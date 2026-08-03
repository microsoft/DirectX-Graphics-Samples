//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
//
//*********************************************************

// ============================================================================
// OverlayUI.cpp
//
// On-screen text-overlay rendering + delta-color baseline tracking, factored
// out of D3D12RaytracingClusteredGeometry.cpp so the main file can focus on
// CORE acceleration-structure management and rendering.
//
// All methods here are still members of D3D12RaytracingClusteredGeometry --
// they're declared in the class header and just have their definitions live
// in a separate translation unit.  No public API changes.
//
// Contains:
//   * StashOverlayStatsAsPrev / RefreshOverlayStatsCurrent /
//     CaptureOverlayStatsSnapshot -- delta-color baseline tracking
//   * CreateUIFont -- DirectXTK SpriteBatch + SpriteFont init
//   * RenderUI    -- the overlay-text drawing pass
// ============================================================================

#include "stdafx.h"
#include "D3D12RaytracingClusteredGeometry.h"

using namespace std;
using namespace DX;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

// ---- StashOverlayStatsAsPrev -------------------------------------------
// =====================================================================================
// Walk all the live buffers + m_clasMemStats and copy the displayed numbers
// into m_overlayStats so the per-frame overlay can read from a snapshot rather
// than poking GetDesc().Width on every frame.  Called from
// RebuildStaticAccelerationStructures (after both static and animated paths
// have finished).  Per-frame timing values are *not* captured here - they're
// snapped by Tick() a few frames later once the ring buffer has refilled.
// =====================================================================================
// =====================================================================================
// Stash the CURRENT overlay snapshot into m_overlayStatsPrev (the delta-colour
// baseline).  Must be called BEFORE any of the build code mutates m_overlayStats
// for a config-change rebuild -- otherwise the rebuild path would stash an
// already-half-updated snapshot as prev, killing the delta colouring.
//
// Specifically: MeasureAnimatedClasBytesOneShot runs INSIDE BuildAnimatedObjectSetup
// and writes s.animatedPerFrameClasActualBytes mid-rebuild.  If we stash s -> prev
// AFTER that write (the old combined-Capture* behaviour), prev gets the NEW
// actual-CLAS value and the delta is always zero -> the per-frame-CLAS-actual
// number never lights up green/red even though it changed visibly on screen.
// Splitting stash + refresh fixes this: the rebuild path stashes prev at the
// top (s is still the OLD state at that point) and the refresh runs at the
// bottom (s gets the NEW state to display).
// =====================================================================================
void D3D12RaytracingClusteredGeometry::StashOverlayStatsAsPrev()
{
    // 'First call' is detected via m_overlayStatsHasPrev rather than any
    // sentinel field value -- see CaptureOverlayStatsSnapshot's call-site
    // comment for why a field-based heuristic is wrong across mode toggles.
    if (!m_overlayStatsHasPrev) return;
    const auto& s = m_overlayStats;
    m_overlayStatsPrev = s;
    if (s.geometryMode == (int)GeometryMode::Clusters)
    {
        m_overlayStatsLastInCluster    = s;
        m_overlayStatsHasLastInCluster = true;
    }
    else
    {
        m_overlayStatsLastInTrad       = s;
        m_overlayStatsHasLastInTrad    = true;
    }
}


// ---- RefreshOverlayStatsCurrent -------------------------------------------
// =====================================================================================
// Snapshot m_overlayStats from the current live GPU resource state.  Re-reads
// every displayed value EXCEPT animatedPerFrameClasActualBytes (which is set by
// MeasureAnimatedClasBytesOneShot at end-of-build and persists across calls).
// Also arms the 5s delta-colour fade and the "recalculating..." per-frame
// settle flag.  Does NOT stash prev -- that must already have been done by
// either CaptureOverlayStatsSnapshot (one-shot callers) or by an earlier
// StashOverlayStatsAsPrev (rebuild callers).
// =====================================================================================
void D3D12RaytracingClusteredGeometry::RefreshOverlayStatsCurrent()
{
    auto sizeOf = [](const Microsoft::WRL::ComPtr<ID3D12Resource>& r) -> UINT64 {
        return r ? r->GetDesc().Width : 0ull;
    };
    auto& s = m_overlayStats;
    const auto now    = std::chrono::steady_clock::now();
    const bool isInit = !m_overlayStatsHasPrev;
    m_overlayStatsHasPrev = true;  // sticky after first refresh

    // Static
    s.staticClasAllocBytes   = m_totalClasBytes;
    s.staticClasActualBytes  = m_clasMemStats.sumActualBytes;
    s.staticClasScratchBytes = m_clasMemStats.scratchBytesPhase1;
    UINT64 blasSum = 0;
    if (m_clusterBlasPoolBuffer)
        blasSum += m_clusterBlasPoolBuffer->GetDesc().Width;
    s.staticBlasTotalBytes   = blasSum;
    s.staticBlasScratchBytes = sizeOf(m_blasScratchBuffer);
    s.staticClusterInputBytes = sizeOf(m_clusterInputBuffer);

    // Animated  (animatedPerFrameClasActualBytes is set by
    // MeasureAnimatedClasBytesOneShot and we leave it alone here).
    if (m_animatedObjectEnabled)
    {
        const auto& a = m_animatedObject;
        s.animatedTemplateBytes            = sizeOf(a.templateResultBuffer);
        s.animatedTemplateScratchBytes     = sizeOf(a.templateScratchBuffer);
        s.animatedTemplateInputBytes       = sizeOf(a.templateInputBuffer);
        s.animatedRestPositionsBytes       = sizeOf(a.restPositionsBuffer);
        s.animatedPerFrameClasAllocBytes   = sizeOf(a.perFrameClasResultBuffer);
        s.animatedPerFrameClasScratchBytes = sizeOf(a.perFrameClasScratchBuffer);
        s.animatedBlasBytes                = sizeOf(a.blasStorage);
        s.animatedBlasScratchBytes         = sizeOf(a.blasScratchBuffer);
        // Traditional-mode animated BLAS state.  Independent of cluster-
        // path fields above so the overlay can show both side-by-side on
        // a [T] toggle without one mode's values bleeding into the other.
        s.animatedTradBlasBytes            = a.tradBlasResultBytes;
        s.animatedTradScratchBytes         = a.tradBlasScratchBytes;
        s.animatedTradIbBytes              = sizeOf(a.tradIndexBuffer);
        s.animatedTradModeIsRefit          = (m_traditionalAnimMode == TraditionalAnimMode::Refit) ? 1 : 0;
        // Phase-2 anim clones: per-clone BLAS pool + scratch.  Mode-
        // exclusive -- only one of these is non-zero at a time depending
        // on m_geometryMode.
        s.animClonesBlasPoolBytes        = sizeOf(m_animClonesBlasPool);
        s.animClonesBlasScratchBytes     = sizeOf(m_animClonesBlasScratchBuffer);
        s.animClonesTradBlasPoolBytes    = sizeOf(m_animClonesTradBlasPool);
        s.animClonesTradBlasScratchBytes = sizeOf(m_animClonesTradBlasScratch);
        // Count of clones that actually got their own per-clone BLAS slot
        // (the rest share source's BLAS via the TLAS fallback).  In cluster
        // mode all clones are pooled; in trad mode it's capped at
        // kTradPerClonePoolMax via BuildAnimatedClonesTradSetup.
        s.animClonesPooledCount =
            (m_geometryMode == GeometryMode::Clusters)
                ? (m_animClonesBlasPool ? (UINT)m_animatedClones.size() : 0u)
                : (UINT)std::count_if(m_animatedClones.begin(), m_animatedClones.end(),
                                      [](const AnimatedCloneInstance& c) { return c.blasGPUVA != 0; });
    }
    else
    {
        s.animatedTemplateBytes = s.animatedTemplateScratchBytes =
        s.animatedTemplateInputBytes = s.animatedRestPositionsBytes =
        s.animatedPerFrameClasAllocBytes = s.animatedPerFrameClasScratchBytes =
        s.animatedBlasBytes = s.animatedBlasScratchBytes =
        s.animatedPerFrameClasActualBytes = 0;
        s.animatedTradBlasBytes = s.animatedTradScratchBytes = s.animatedTradIbBytes = 0;
        s.animatedTradModeIsRefit = 0;
        s.animClonesBlasPoolBytes = s.animClonesBlasScratchBytes = 0;
        s.animClonesTradBlasPoolBytes = s.animClonesTradBlasScratchBytes = 0;
        s.animClonesPooledCount = 0;
    }

    s.tlasBytes            = sizeOf(m_tlasBuffer);
    s.totalClusterCount    = m_totalClusterCount;
    s.totalTriangleCount   = m_totalTriangleCount;

    // Traditional path totals.  Aggregate per-object tradBlasStorage sizes
    // so the overlay can show "BLAS X MB" apples-to-apples vs the cluster
    // path's CLAS+BLAS total.  Build-time wall-clock is captured by
    // BuildTraditionalStaticAS itself.
    s.geometryMode                = (int)m_geometryMode;
    {
        // For Implicit alloc: alloc == actual.  For Compact alloc: alloc
        // is the worst-case prebuild size we used during the build (now
        // freed!), actual is the compacted size that obj.tradBlasStorage
        // currently holds.  m_traditionalStaticTotalResultBytes /
        // ...ActualBytes are populated by BuildTraditionalStaticAS using
        // the values captured DURING the build (so they're correct for
        // Compact even after the worst-case temp is dropped).
        s.traditionalBlasTotalBytes   = m_traditionalStaticTotalResultBytes;
        s.traditionalBlasActualBytes  = m_traditionalStaticTotalActualBytes;
        s.traditionalBlasScratchBytes = m_traditionalStaticTotalScratchBytes;
        UINT64 tradVbSum = 0, tradIbSum = 0;
        for (const auto& obj : m_objects)
        {
            if (obj.tradVertexBuffer) tradVbSum += obj.tradVertexBuffer->GetDesc().Width;
            if (obj.tradIndexBuffer)  tradIbSum += obj.tradIndexBuffer->GetDesc().Width;
        }
        // Add pool-owned sizes too -- when per-obj VB/IB live in the
        // shared trad VB/IB pools (m_tradVertexPool / m_tradIndexPool)
        // their ComPtrs above are null, so we'd otherwise undercount.
        if (m_tradVertexPool)  tradVbSum += m_tradVertexPool->GetDesc().Width;
        if (m_tradIndexPool)   tradIbSum += m_tradIndexPool->GetDesc().Width;
        s.traditionalVbBytes          = tradVbSum;
        s.traditionalIbBytes          = tradIbSum;
        s.traditionalBuildMs          = m_traditionalStaticBuildMs;
    }

    // Reset the per-frame snap accumulator so the first post-toggle snap
    // is built only from new-mode samples.  Don't touch m_overlayStats /
    // pfTimingValid -- we keep displaying the previous value during the
    // ~0.3 s settle until the new snap fires.  See the long comment on
    // m_pfSnapAccum in the header for the full mechanism.
    m_pfSnapSkipFramesLeft   = (INT)kPerFrameRingSlots;
    m_pfSnapSamplesCollected = 0;
    m_pfSnapAccum            = PfSnapAccum{};
    // Mark per-frame timings as settling (skipped on init -- no settle to
    // wait for on the first capture, just display whatever the first
    // post-init snap window produces).  The per-frame snap-window-complete
    // branch in DoRender clears this once the first post-toggle window
    // averages settle into m_overlayStats.  While set, the overlay's
    // PER-FRAME section prints "recalculating..." in place of the timing
    // numbers (which would otherwise show the OLD mode's stale numbers
    // for the ~1 s the rolling window takes to refill, leaving the user
    // wondering "did the toggle do anything?").
    if (!isInit)
        m_pfTimingSettlingAfterToggle = true;

    // Refresh the delta-colour fade window: 5 seconds from NOW.  Each toggle
    // installs a fresh window (and a fresh prev above), so the user sees
    // immediate red/green vs the immediately previous toggle, fading back to
    // subtle 5 s after the LAST toggle.  Skipped on init -- there's nothing
    // meaningful to compare against yet, so we leave deltaUntil at min() so
    // the first real toggle's colours show without waiting for the fake-init
    // window to drain.
    if (!isInit)
        m_overlayStatsDeltaUntil = now + std::chrono::seconds(5);
}


// ---- CaptureOverlayStatsSnapshot -------------------------------------------
// =====================================================================================
// Convenience wrapper: stash prev, then refresh current.  Use this from any
// caller whose code path does NOT mutate m_overlayStats between the prev-stash
// and the refresh.  Rebuild paths (RebuildStaticAccelerationStructures) MUST
// instead bracket the build with StashOverlayStatsAsPrev() at the top and
// RefreshOverlayStatsCurrent() at the bottom -- otherwise the
// MeasureAnimatedClasBytesOneShot write inside the build steals prev.
// =====================================================================================
void D3D12RaytracingClusteredGeometry::CaptureOverlayStatsSnapshot()
{
    StashOverlayStatsAsPrev();
    RefreshOverlayStatsCurrent();
}


// ---- CreateUIFont -------------------------------------------
// ---------------------------------------------------------------------------------
// DirectXTK SpriteBatch + SpriteFont setup.  Creates the GraphicsMemory ring
// allocator that DirectXTK uses for per-frame upload data, builds a sprite
// batch PSO that matches the back-buffer + depth-buffer format pair, and
// loads SegoeUI_24.spritefont (deployed by the .vcxproj alongside the exe).
// The 24 pt atlas is exactly 2x the visual target -- combined with kScale=0.5
// below, that gives a clean 2:1 bilinear downsample (every destination pixel
// = perfect average of its 2x2 source texels).  Any other ratio (e.g. 48 pt /
// kScale=0.265 we briefly tried) drops source texels under bilinear filtering
// and aliases on thin glyph strokes.
//
// The font texture SRV is placed into slot 1 of our shader-visible descriptor
// heap (slot 0 = the raytracing-output UAV).  AllocateDescriptor() hands
// these out monotonically; the heap was bumped to 8 slots so we have plenty
// of room for the font + future overlays.
//
// MUST be called AFTER CreateDescriptorHeapAndRaytracingOutput so that the
// shared descriptor heap exists when we ask it for slot 1.
// ---------------------------------------------------------------------------------
void D3D12RaytracingClusteredGeometry::CreateUIFont()
{
    using namespace DirectX;
    auto device = m_deviceResources->GetD3DDevice();

    m_graphicsMemory = std::make_unique<GraphicsMemory>(device);

    // SpriteBatch PSO needs to match the back-buffer format pair (RTV format
    // + DSV format).  Depth buffer is present on this sample's DeviceResources
    // but the overlay itself disables depth read/write via SpriteBatch's
    // default state -- the format just has to match the bound DSV slot at
    // draw time, even if we render with no depth.
    ResourceUploadBatch resourceUpload(device);
    resourceUpload.Begin();
    {
        RenderTargetState rtState(m_deviceResources->GetBackBufferFormat(),
                                  m_deviceResources->GetDepthBufferFormat());
        SpriteBatchPipelineStateDescription pd(rtState);
        m_spriteBatch = std::make_unique<SpriteBatch>(device, resourceUpload, pd);
    }
    auto uploadFinished = resourceUpload.End(m_deviceResources->GetCommandQueue());
    uploadFinished.wait();

    // Reserve slot 1 of our shared descriptor heap for the font texture SRV.
    // SpriteFont's constructor takes the CPU + GPU descriptor handles where
    // it should write the SRV.
    D3D12_CPU_DESCRIPTOR_HANDLE fontCpu;
    UINT fontSlot = AllocateDescriptor(&fontCpu);
    D3D12_GPU_DESCRIPTOR_HANDLE fontGpu = CD3DX12_GPU_DESCRIPTOR_HANDLE(
        m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), fontSlot, m_descriptorSize);

    {
        ResourceUploadBatch fontUpload(device);
        fontUpload.Begin();
        m_uiFont = std::make_unique<SpriteFont>(device, fontUpload,
            L"SegoeUI_24.spritefont",
            fontCpu, fontGpu);
        // Defensive: if we ever try to render a character that isn't in the
        // sprite font (e.g. a non-ASCII codepoint baked into a log message
        // by mistake), SpriteFont::DrawString throws std::runtime_error which
        // propagates out of WndProc -> STATUS_FATAL_USER_CALLBACK_EXCEPTION.
        // Setting a default glyph turns that crash into a visible '?' instead.
        m_uiFont->SetDefaultCharacter(L'?');
        auto finished = fontUpload.End(m_deviceResources->GetCommandQueue());
        finished.wait();
    }
    SampleLog::LogF(L"[ui] SpriteFont loaded (descriptor heap slot %u); "
                    L"line spacing = %.1f px\n",
                    fontSlot, m_uiFont->GetLineSpacing());

    // 1x1 white texture used by the overlay backing-rect pass below
    // (see RenderUI's drawSeg/draw lambdas).  Created via the same
    // ResourceUploadBatch pattern as the spritefont, dropped into the
    // next descriptor heap slot, GPU handle stashed for SpriteBatch::Draw.
    {
        ResourceUploadBatch upload(device);
        upload.Begin();

        D3D12_RESOURCE_DESC texDesc = {};
        texDesc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texDesc.Width              = 1;
        texDesc.Height             = 1;
        texDesc.DepthOrArraySize   = 1;
        texDesc.MipLevels          = 1;
        texDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count   = 1;
        texDesc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;

        auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        ThrowIfFailed(device->CreateCommittedResource(
            &defaultHeap, D3D12_HEAP_FLAG_NONE, &texDesc,
            D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
            IID_PPV_ARGS(&m_overlayPanelTexture)));
        m_overlayPanelTexture->SetName(L"Overlay 1x1 white (per-line dark backing source)");

        // One white pixel: R=255 G=255 B=255 A=255.  SpriteBatch tints
        // it with the dark+alpha colour we want when drawing the rect.
        const uint8_t whitePixel[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
        D3D12_SUBRESOURCE_DATA sub = {};
        sub.pData      = whitePixel;
        sub.RowPitch   = sizeof(whitePixel);
        sub.SlicePitch = sizeof(whitePixel);

        upload.Upload(m_overlayPanelTexture.Get(), 0, &sub, 1);
        upload.Transition(m_overlayPanelTexture.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        auto done = upload.End(m_deviceResources->GetCommandQueue());
        done.wait();

        // SRV in the next free heap slot; SpriteBatch::Draw consumes a
        // GPU handle.
        D3D12_CPU_DESCRIPTOR_HANDLE cpu;
        UINT slot = AllocateDescriptor(&cpu);
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels     = 1;
        device->CreateShaderResourceView(m_overlayPanelTexture.Get(), &srvDesc, cpu);
        m_overlayPanelTextureGpu = CD3DX12_GPU_DESCRIPTOR_HANDLE(
            m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), slot, m_descriptorSize);
        SampleLog::LogF(L"[ui] overlay-panel 1x1 texture (descriptor slot %u)\n", slot);
    }
}


// ---- RenderUI -------------------------------------------
// ---------------------------------------------------------------------------------
// Per-frame overlay text.  Called from DoRender() AFTER the raytraced output
// has been copied into the back buffer and the back buffer is transitioned
// back to RENDER_TARGET state.  The sprite batch rebinds the back-buffer RTV
// (no depth attachment -- text is depth-disabled by default) and draws our
// stats block + key bindings on top of the ray-traced image.
//
// Layout:
//   line 0   adapter name (white, larger weight via DrawString w/ 1.1x scale)
//   line 1   BLAS / CLAS / triangle counts
//   line 2   CLAS memory: total + avg/cluster + scratch
//   line 3   active vertex format
//   line 4   active CLAS alloc mode
//   line 5   per-frame rebuild timing (anim + TLAS)
//   line 6   FPS
//   line 7+  key-binding hints, with the hotkey character coloured yellow
//
// Pixel coordinates are top-left origin; we leave a 24-px inset from the
// window's top-left corner.
// ---------------------------------------------------------------------------------
void D3D12RaytracingClusteredGeometry::RenderUI()
{
    using namespace DirectX;
    if (!m_spriteBatch || !m_uiFont) return;

    auto commandList = m_deviceResources->GetCommandList();
    auto viewport    = m_deviceResources->GetScreenViewport();
    auto scissor     = m_deviceResources->GetScissorRect();

    // Bind the back buffer as render target (no depth -- text doesn't write Z).
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_deviceResources->GetRenderTargetView();
    commandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissor);

    // Re-bind the shader-visible descriptor heap so the SpriteBatch shader
    // can sample the font texture SRV from slot 1.  (Compute path bound it
    // earlier; OMSetRenderTargets doesn't disturb it but the SetDescriptorHeaps
    // call is documented to be safe to repeat per-pass and PIX captures look
    // cleaner with the explicit bind.)
    ID3D12DescriptorHeap* heaps[] = { m_descriptorHeap.Get() };
    commandList->SetDescriptorHeaps(_countof(heaps), heaps);

    m_spriteBatch->SetViewport(viewport);
    m_spriteBatch->Begin(commandList);

    // Adaptive scale: target 0.625x (looks great at 4K) but if that
    // would overflow the back buffer (typically on small windows like
    // 1280x720, or wide-but-short ones like 2560x400) drop just enough
    // to fit.  Math: pick the most restrictive constraint of width
    // and height, clamp to the target.
    //   availPxW   = bb_width  - leftMargin - rightMargin - kCol2Gap
    //   availPxH   = bb_height - topMargin  - bottomMargin
    //   fitScaleW  = availPxW / m_overlayContentUnscaledWidth
    //   fitScaleH  = availPxH / m_overlayContentUnscaledHeight
    //   kScale     = min(target, fitScaleW, fitScaleH)
    // Both Content*Unscaled* members come from the PREVIOUS frame's
    // measurement (max-monotonic), so they converge in 1 frame and
    // auto-handle window-resize / mode-toggle changes.  Initial
    // values are calibrated in the header so the first frame on
    // common small windows (1280x720, 1024x768) doesn't flash overflow
    // before the measurement-feedback loop catches up.
    //
    // Why not pre-measure THIS frame's strings: would require
    // refactoring all ~700 lines of overlay formatting into a
    // build-strings-then-draw-strings split.  Frame-late approach is
    // ~30 lines of bookkeeping and visually indistinguishable except
    // on the FIRST frame after a state change that widens/lengthens
    // content.
    constexpr float kTargetScale  = 0.625f;
    constexpr float kLeftMargin   = 24.0f;
    constexpr float kRightMargin  = 8.0f;
    constexpr float kTopMargin    = 18.0f;
    constexpr float kBottomMargin = 8.0f;
    constexpr float kCol2Gap      = 24.0f;
    const float     bbWidth       = (float)m_deviceResources->GetScreenViewport().Width;
    const float     bbHeight      = (float)m_deviceResources->GetScreenViewport().Height;
    const float     availPxW      = bbWidth  - kLeftMargin - kRightMargin  - kCol2Gap;
    const float     availPxH      = bbHeight - kTopMargin  - kBottomMargin;
    // Independent width- and height-fit constraints; whichever bites
    // first wins.  Width-fit covers narrow windows (small bbWidth /
    // long key descriptions); height-fit covers wide-but-short
    // windows (e.g. 2560x400) where the keys column would otherwise
    // run off the bottom of the back buffer.
    const float     fitScaleW     = (availPxW > 0.0f && m_overlayContentUnscaledWidth  > 0.0f)
                                        ? availPxW / m_overlayContentUnscaledWidth
                                        : kTargetScale;
    const float     fitScaleH     = (availPxH > 0.0f && m_overlayContentUnscaledHeight > 0.0f)
                                        ? availPxH / m_overlayContentUnscaledHeight
                                        : kTargetScale;
    const float    kScale  = std::min({kTargetScale, fitScaleW, fitScaleH});
    const float    kLineH  = m_uiFont->GetLineSpacing() * kScale;
    XMFLOAT2       pos{ kLeftMargin, kTopMargin };
    const XMFLOAT2 kOrigin { 0.0f, 0.0f };
    // Body text colours.  White on this scene's sky/hex/floor palette --
    // kSubtle is the "no change" colour for stat rows (delta-coloured numbers
    // tint away from this to green/red when they move).
    const XMVECTOR kWhite  = XMVectorSet(1.00f, 1.00f, 1.00f, 1);
    const XMVECTOR kSubtle = XMVectorSet(0.92f, 0.94f, 0.97f, 1);
    const XMVECTOR kAccent = XMVectorSet(0.20f, 0.95f, 1.00f, 1);   // bright cyan for section headers (replaces lavender -- pops harder against the dark backing rect AND against the warm checkerboard floor)
    const XMVECTOR kHotkey = XMVectorSet(1.00f, 0.90f, 0.15f, 1);   // bright yellow for the hotkey char
    // Delta-colouring: every per-config-change number compares to its
    // previous-snapshot value and tints itself.
    //   green = went down (smaller = "better" for memory/time)
    //   red   = went up   ("worse")
    //   subtle (unchanged) = no prev or no change
    const XMVECTOR kGreen  = XMVectorSet(0.40f, 1.00f, 0.40f, 1);   // bright green delta
    const XMVECTOR kRed    = XMVectorSet(1.00f, 0.45f, 0.45f, 1);   // bright red delta

    // Local helpers -- under each text segment, draw a dark backing
    // rect first (alpha-blended), then the body text "faux-bold" -- two
    // DrawString passes at the same position offset by +1 px horizontally,
    // so glyph strokes appear 1 px wider on their right edge.  That
    // simulates a bold weight without needing a separate Bold spritefont
    // asset, which would require running MakeSpriteFont and shipping
    // a second .spritefont file.  Net visual: each stroke is 1 pixel
    // heavier -> noticeably easier to read against the rect's AA dim.
    //
    // No drop shadow -- it stamped extra dark pixels around each glyph
    // edge which compounded the AA-dim problem the rect already had.
    // Faux-bold thickens the BODY which is rendered at full alpha, so
    // there's no AA mixing penalty for the heavier weight.
    //
    // Padding strategy: line ENDS get kLinePad horizontal pad via the
    // deferred-`pending` rect slot; interior segment joins get zero
    // pad so they don't double-darken.  See same-line-aware logic below.
    constexpr float kLinePad = 4.0f;
    constexpr float kBoldOff = 1.0f;   // faux-bold horizontal stroke widening
    const XMVECTOR  kBacking = XMVectorSet(0.0f, 0.0f, 0.0f, 0.20f);

    auto measureX = [&](const wchar_t* s) {
        return XMVectorGetX(m_uiFont->MeasureString(s, /*ignoreWhitespace*/false)) * kScale;
    };

    struct PendingRect { float x, y, w, h; bool valid; } pending = {0, 0, 0, 0, false};

    // Frame-local tracking for the next-frame fit-scale calculation.
    // Both right- and bottom-edge tracking; converted to unscaled
    // atlas units at end of RenderUI before being max()'d into the
    // persistent members.
    //  - thisFrameCol1RightPx: max right edge while trackCol1==true.
    //    Drives col 2's x-anchor for NEXT frame, and feeds the col1
    //    width portion of the total.
    //  - thisFrameAnyRightPx: max right edge of ANY rect this frame
    //    (cols 1 AND 2).  Combined with col2's x-anchor, gives col 2's
    //    content width.
    //  - thisFrameAnyBottomPx: max bottom edge of ANY rect this frame.
    //    Drives the height-fit clamp for NEXT frame.
    float thisFrameCol1RightPx  = 0.0f;
    float thisFrameAnyRightPx   = 0.0f;
    float thisFrameAnyBottomPx  = 0.0f;
    bool  trackCol1             = true;

    auto emitBacking = [&](float x, float y, float w, float h) {
        if (w <= 0.0f || h <= 0.0f) return;
        thisFrameAnyRightPx  = std::max(thisFrameAnyRightPx,  x + w);
        thisFrameAnyBottomPx = std::max(thisFrameAnyBottomPx, y + h);
        if (trackCol1)
            thisFrameCol1RightPx = std::max(thisFrameCol1RightPx, x + w);
        RECT r = {
            (LONG)std::floor(x),
            (LONG)std::floor(y),
            (LONG)std::floor(x + w),
            (LONG)std::floor(y + h)
        };
        m_spriteBatch->Draw(m_overlayPanelTextureGpu, XMUINT2(1, 1), r, kBacking);
    };

    // Single-rect-per-row strategy:  `pending` accumulates the bounding
    // rect of EVERY segment drawn on the current row.  Each call to
    // addToPending(x,y,w,h) either (a) starts a new row by flushing the
    // previous row's rect, or (b) GROWS the current row's rect to cover
    // the new segment.  The row's rect is only emitted to the back
    // buffer when we move to a new row OR at end-of-frame.
    //
    // Why this matters: emitting one rect per segment caused 1-pixel
    // vertical dark seams at segment boundaries.  Each segment rect
    // gets floor()'d at both ends so adjacent rects share a 1-pixel
    // column where the alpha (0.20) composites twice -> visible dark
    // bar at every "label | value | label" join along a row.  One
    // single rect per row eliminates the issue entirely -- there's
    // simply no boundary to double-darken.
    auto flushPending = [&]() {
        if (!pending.valid) return;
        emitBacking(pending.x, pending.y, pending.w + kLinePad, pending.h);
        pending.valid = false;
    };

    auto addToPending = [&](float x, float y, float w, float h) {
        if (!pending.valid || pending.y != y)
        {
            // New row -- flush old row's rect, start a fresh one with
            // the left-pad baked into x.  Width includes +kBoldOff so
            // the +1 px faux-bold pass stays inside the rect.
            if (pending.valid) flushPending();
            pending = { x - kLinePad, y, w + kLinePad + kBoldOff, h, true };
        }
        else
        {
            // Same row -- extend pending right edge to cover this
            // segment, keep height = max so a mixed-height row (e.g.
            // the Adapter line uses 1.10x scale) draws a rect tall
            // enough for the tallest glyph on it.
            const float newRight = std::max(pending.x + pending.w,
                                            x + w + kBoldOff);
            pending.w = newRight - pending.x;
            pending.h = std::max(pending.h, h);
        }
    };

    // Local helper -- DrawString with the global scale factor baked in.
    // Two body passes for faux-bold: same string, same colour, +1px
    // horizontal offset on the second pass.
    auto draw = [&](const wchar_t* s, XMFLOAT2 p, FXMVECTOR colour, float relScale = 1.0f) {
        const float w = measureX(s) * relScale;
        const float h = kLineH * relScale;
        addToPending(p.x, p.y, w, h);
        m_uiFont->DrawString(m_spriteBatch.get(), s, p, colour,
                             /*rotation*/0.0f, kOrigin, kScale * relScale);
        XMFLOAT2 boldPos{ p.x + kBoldOff, p.y };
        m_uiFont->DrawString(m_spriteBatch.get(), s, boldPos, colour,
                             /*rotation*/0.0f, kOrigin, kScale * relScale);
    };

    // Segment draw: writes `text` at the cursor and advances cursor.x.
    // Defers its rect into the row-level `pending`; faux-bold body pass
    // like `draw`.
    auto drawSeg = [&](const wchar_t* text, XMFLOAT2& cursor, FXMVECTOR colour) {
        const float w = measureX(text);
        addToPending(cursor.x, cursor.y, w, kLineH);
        m_uiFont->DrawString(m_spriteBatch.get(), text, cursor, colour,
                             /*rotation*/0.0f, kOrigin, kScale);
        XMFLOAT2 boldPos{ cursor.x + kBoldOff, cursor.y };
        m_uiFont->DrawString(m_spriteBatch.get(), text, boldPos, colour,
                             /*rotation*/0.0f, kOrigin, kScale);
        cursor.x += w;
    };
    // Delta-colour pickers.  Return green/red/subtle based on cur vs prev.
    // Two gates:
    //   1. m_overlayStatsHasPrev -- no baseline yet (first capture).
    //   2. m_overlayStatsDeltaUntil -- the 5s post-toggle window during which
    //      colours are shown.  After it expires we fall back to subtle so the
    //      screen doesn't permanently glow red/green long after the user
    //      finished iterating.
    // Plus a NOISE THRESHOLD: tiny changes (rounding drift, EMA wobble) get
    // suppressed -- only a |cur - prev| / |prev| >= 2 % relative change
    // triggers a colour, so the screen doesn't flash for sub-MB / sub-us
    // differences that don't matter.  2 % was chosen so that going 11.00 KB/cl
    // -> 11.21 KB/cl still reads as "same"; 11.00 -> 11.23 lights up.
    constexpr double kDeltaPctThreshold = 0.02;
    const bool deltaActive = m_overlayStatsHasPrev
        && (std::chrono::steady_clock::now() < m_overlayStatsDeltaUntil);
    auto deltaColour = [&](double cur, double prev) -> XMVECTOR {
        if (!deltaActive) return kSubtle;
        const double base = std::max(std::abs(prev), 1e-9);
        const double rel  = std::abs(cur - prev) / base;
        if (rel < kDeltaPctThreshold) return kSubtle;
        return (cur < prev) ? kGreen : kRed;
    };
    auto deltaColourU = [&](UINT64 cur, UINT64 prev) -> XMVECTOR {
        if (!deltaActive) return kSubtle;
        const double base = std::max((double)prev, 1.0);
        const double diff = (cur > prev) ? (double)(cur - prev) : (double)(prev - cur);
        if (diff / base < kDeltaPctThreshold) return kSubtle;
        return (cur < prev) ? kGreen : kRed;
    };
    // Convenience: print a value into a small buffer + return a wchar_t* so
    // drawSeg can consume it inline.  Each fmt* call is the SAME format string
    // so cur and prev are formatted identically (avoids "0.49 vs 0.495"
    // false-equal artifacts when the difference is below the displayed precision).
    auto fmt2 = [](wchar_t* dst, size_t cch, const wchar_t* f, double v) {
        swprintf_s(dst, cch, f, v); return dst;
    };
    // Smart size formatter: prints "X.XX MB" when value rounds to >= 0.01 MB
    // at the displayed precision, otherwise falls back to "X.XX KB" so a
    // sub-10KB allocation (e.g. TLAS at low N, materials buffer) doesn't
    // disappear as "0.00 MB".  Caller passes the raw byte count so we always
    // pick the right unit for the actual value (delta colouring still uses
    // MB pairs separately so cross-unit transitions colour correctly).
    auto fmtSize = [](wchar_t* dst, size_t cch, UINT64 bytes) {
        const double mb = bytes / (1024.0 * 1024.0);
        if (mb >= 0.01) swprintf_s(dst, cch, L"%.2f MB", mb);
        else            swprintf_s(dst, cch, L"%.2f KB", bytes / 1024.0);
        return dst;
    };
    // Same smart unit picker but taking the already-summed MB value (for
    // multi-term sums like total scratch = template + per-frame + BLAS).
    auto fmtMb = [](wchar_t* dst, size_t cch, double mb) {
        if (mb >= 0.01) swprintf_s(dst, cch, L"%.2f MB", mb);
        else            swprintf_s(dst, cch, L"%.2f KB", mb * 1024.0);
        return dst;
    };
    wchar_t fnum[64];   // scratch for formatted numbers

    wchar_t buf[256];

    // Line 0: adapter name (slightly bigger than the body lines).
    swprintf_s(buf, L"Adapter: %s", m_deviceResources->GetAdapterDescription());
    draw(buf, pos, kWhite, /*relScale*/1.10f);
    pos.y += kLineH * 1.6f;

    // Triangle-count short form.
    auto formatTris = [](UINT n, wchar_t* out, size_t cch) {
        if      (n >= 1000000u) swprintf_s(out, cch, L"%.1fM", n / 1.0e6);
        else if (n >= 1000u)    swprintf_s(out, cch, L"%.1fK", n / 1.0e3);
        else                    swprintf_s(out, cch, L"%u",    n);
    };
    // ----- All numeric stats below read from m_overlayStats (snapshotted on
    //       config change) so the per-frame overhead of this overlay is just
    //       a few swprintf_s calls + the SpriteBatch draws.  Only FPS is
    //       computed live.  See OverlayStats / CaptureOverlayStatsSnapshot
    //       in D3D12RaytracingClusteredGeometry.h/.cpp for the snap logic.
    //
    // Layout convention:
    //   - Blue (kAccent)  = section headers + top-level counts.
    //   - White (kSubtle) = stat rows underneath, indented two spaces.
    // That's the only colour-meaning rule.  Don't sprinkle blue on data rows.
    const auto& s = m_overlayStats;
    const float kSectionGap = kLineH * 0.4f;

    wchar_t trisBuf[16];
    formatTris(s.totalTriangleCount, trisBuf, _countof(trisBuf));
    const UINT animClusters = m_animatedObjectEnabled ? m_animatedObject.clusterCount : 0u;
    // Add anim clones' own BLASes when phase-2 pool exists.
    const UINT animCloneBlasCount =
        ((m_geometryMode == GeometryMode::Clusters && m_animClonesBlasPool) ||
         (m_geometryMode == GeometryMode::Traditional && m_animClonesTradBlasPool))
            ? (UINT)m_animatedClones.size()
            : 0u;
    const UINT sceneBlasCount    = (UINT)m_objects.size()
                                 + (m_animatedObjectEnabled ? 1u : 0u)
                                 + animCloneBlasCount;
    const UINT sceneClusterCount = s.totalClusterCount + animClusters;
    const bool isTraditional = (s.geometryMode != (int)GeometryMode::Clusters);
    draw(L"SCENE:", pos, kAccent);
    // Column 2 (keys section) will start at this same Y, anchored at
    // m_overlayCol1MaxRight (from last frame) + a small gap.  Captured
    // here -- right before we draw the first column's first section --
    const float col2StartY = pos.y;
    pos.y += kLineH;
    if (isTraditional)
        swprintf_s(buf, L"  %u BLASes  /  %s tris   (traditional BLAS)",
                   sceneBlasCount, trisBuf);
    else
        swprintf_s(buf, L"  %u BLASes  /  %u CLASes  /  %s tris   (clustered BLAS)",
                   sceneBlasCount, sceneClusterCount, trisBuf);
    draw(buf, pos, kSubtle);
    pos.y += kLineH + kSectionGap;

    // section subtotal at the bottom of each STATIC arm can show an
    // apples-to-apples comparison against the OTHER mode's last value
    // (cluster: CLAS+BLAS vs trad: BLAS).
    auto sectionStaticTotalBytes = [](const OverlayStats& x) -> UINT64 {
        return (x.geometryMode == (int)GeometryMode::Clusters)
            ? (x.staticClasAllocBytes + x.staticBlasTotalBytes)
            : x.traditionalBlasActualBytes;
    };
    draw(L"STATIC:", pos, kAccent);
    pos.y += kLineH;
    if (isTraditional)
    {
        // Same-mode prev: a [T] cluster->trad toggle leaves
        // m_overlayStatsPrev holding cluster numbers, but trad stats
        // (BLAS bytes, scratch, etc.) don't exist in cluster mode.
        // Use the last-seen-IN-TRAD snapshot for deltas so colours
        // mean "what changed within trad path" -- not "what changed
        // because the unrelated cluster path was the previous mode".
        const auto& p = m_overlayStatsHasLastInTrad ? m_overlayStatsLastInTrad
                                                    : m_overlayStatsPrev;
        const double allocMb    = s.traditionalBlasTotalBytes     / (1024.0 * 1024.0);
        const double prevAllocMb= p.traditionalBlasTotalBytes     / (1024.0 * 1024.0);
        const double actualMb   = s.traditionalBlasActualBytes    / (1024.0 * 1024.0);
        const double prevActMb  = p.traditionalBlasActualBytes    / (1024.0 * 1024.0);
        const double scratchMb  = s.traditionalBlasScratchBytes / (1024.0 * 1024.0);
        const double prevScrMb  = p.traditionalBlasScratchBytes / (1024.0 * 1024.0);
        const double inputsMb   = (s.traditionalVbBytes + s.traditionalIbBytes) / (1024.0 * 1024.0);
        const double prevInpMb  = (p.traditionalVbBytes + p.traditionalIbBytes) / (1024.0 * 1024.0);
        // Per-geometry-desc average -- one geom desc per cluster, so this
        // is the apples-to-apples comparison vs the cluster panel's
        // "KB/cl" line (both divide by the same denominator -- the total
        // static cluster count).
        const double avgKb      = (s.totalClusterCount > 0)
            ? (double)s.traditionalBlasActualBytes / (double)s.totalClusterCount / 1024.0 : 0.0;
        const double prevAvgKb  = (p.totalClusterCount > 0 && p.traditionalBlasActualBytes > 0)
            ? (double)p.traditionalBlasActualBytes / (double)p.totalClusterCount / 1024.0 : 0.0;

        XMFLOAT2 c = pos;
        drawSeg(L"  BLASes ", c, kSubtle);
        drawSeg(fmtSize(fnum, _countof(fnum), s.staticClasAllocBytes), c, deltaColour(allocMb, prevAllocMb));
            drawSeg(L" alloc  (", c, kSubtle);
        drawSeg(fmtSize(fnum, _countof(fnum), s.staticClasActualBytes), c, deltaColour(actualMb, prevActMb));
            drawSeg(L" actual, avg ", c, kSubtle);
        drawSeg(fmt2(fnum, _countof(fnum), L"%.2f", avgKb), c, deltaColour(avgKb, prevAvgKb));
        drawSeg(L" KB/geom)", c, kSubtle);
        pos.y += kLineH;
        // Section subtotal (cross-mode delta vs m_overlayStatsPrev -- so a
        // [T] toggle paints this red/green with the cluster-vs-trad
        // comparison of total static resident memory).  Trad path = BLAS
        // actual only.  Label suffix tells the user exactly which
        // buffers contribute; scratch and inputs are below, outside the
        // total since they're workspace + source-data, not BVH.
        const double tradTotalMb = sectionStaticTotalBytes(s) / (1024.0 * 1024.0);
        const double prevTotalMb = sectionStaticTotalBytes(m_overlayStatsPrev) / (1024.0 * 1024.0);
        c = pos;
        drawSeg(L"  total ", c, kSubtle);
        drawSeg(fmtMb(fnum, _countof(fnum), tradTotalMb), c, deltaColour(tradTotalMb, prevTotalMb));
            drawSeg(L"  (BLASes)", c, kSubtle);
        pos.y += kLineH;
        c = pos;
        drawSeg(L"  scratch ", c, kSubtle);
        drawSeg(fmtSize(fnum, _countof(fnum), s.traditionalBlasScratchBytes), c, deltaColour(scratchMb, prevScrMb));
            drawSeg(L"    inputs (vb+ib) ", c, kSubtle);
        drawSeg(fmtSize(fnum, _countof(fnum), s.staticClusterInputBytes), c, deltaColour(inputsMb, prevInpMb));
        pos.y += kLineH;
    }
    else
    {
    // NOTE on precision-vs-memory: staticClasAllocBytes is the RESULT BUFFER
    // ALLOCATION, not the bytes the driver actually emitted into it.  In
    // Implicit-dest mode the buffer is sized for worst-case (max possible
    // per-cluster BVH leaf) and is *insensitive* to PositionTruncateBitCount
    // / compressed1 bits-per-component.  In GetSizes / Compact modes the
    // buffer is sized after a GetSizes probe returns per-cluster actual
    // sizes, so the slider visibly shrinks/grows it.  Either way we show
    // staticClasActualBytes alongside so the precision effect is visible.
        // Same-mode prev: CLAS bytes are cluster-mode-only -- use the
        // last cluster-mode snapshot so a [T] toggle's delta reflects
        // intra-cluster changes, not garbage vs trad.
        const auto& p = m_overlayStatsHasLastInCluster ? m_overlayStatsLastInCluster
                                                       : m_overlayStatsPrev;
        const double allocMb     = s.staticClasAllocBytes   / (1024.0 * 1024.0);
        const double prevAllocMb = p.staticClasAllocBytes   / (1024.0 * 1024.0);
        const double actualMb    = s.staticClasActualBytes  / (1024.0 * 1024.0);
        const double prevActMb   = p.staticClasActualBytes  / (1024.0 * 1024.0);
        const double scratchMb   = (s.staticClasScratchBytes + s.staticBlasScratchBytes) / (1024.0 * 1024.0);
        const double prevScrMb   = (p.staticClasScratchBytes + p.staticBlasScratchBytes) / (1024.0 * 1024.0);
        const double avgKb       = (s.totalClusterCount > 0)
            ? (double)s.staticClasAllocBytes / (double)s.totalClusterCount / 1024.0 : 0.0;
        const double prevAvgKb   = (p.totalClusterCount > 0)
            ? (double)p.staticClasAllocBytes / (double)p.totalClusterCount / 1024.0 : 0.0;
        const double blasMb      = s.staticBlasTotalBytes   / (1024.0 * 1024.0);
        const double prevBlasMb  = p.staticBlasTotalBytes   / (1024.0 * 1024.0);
        const double inputsMb    = s.staticClusterInputBytes / (1024.0 * 1024.0);
        const double prevInpMb   = p.staticClusterInputBytes / (1024.0 * 1024.0);

        XMFLOAT2 c = pos;
        drawSeg(L"  CLASes ", c, kSubtle);
        drawSeg(fmtSize(fnum, _countof(fnum), s.staticClasAllocBytes), c, deltaColour(allocMb, prevAllocMb));
            drawSeg(L" alloc  (", c, kSubtle);
        drawSeg(fmtSize(fnum, _countof(fnum), s.staticClasActualBytes), c, deltaColour(actualMb, prevActMb));
            drawSeg(L" actual, avg ", c, kSubtle);
        drawSeg(fmt2(fnum, _countof(fnum), L"%.2f", avgKb), c, deltaColour(avgKb, prevAvgKb));
        drawSeg(L" KB/cl)", c, kSubtle);
        pos.y += kLineH;

        // BLAS line: cluster path's per-object BLAS storage (sum over all
        // static objects).  The cluster path holds BOTH a CLAS pool AND
        // per-object BLAS-from-CLAS so the static-memory comparison vs
        // trad (which has just BLAS) needs both lines visible.
        c = pos;
        drawSeg(L"  BLASes ", c, kSubtle);
        drawSeg(fmtSize(fnum, _countof(fnum), s.staticBlasTotalBytes), c, deltaColour(blasMb, prevBlasMb));
        pos.y += kLineH;

        // Section subtotal (cross-mode delta vs m_overlayStatsPrev -- so a
        // [T] toggle paints this red/green with the cluster-vs-trad
        // comparison of total static resident memory).  Cluster path =
        // CLAS alloc + BLAS.  Label suffix tells the user exactly which
        // buffers contribute -- scratch and inputs are below, outside
        // the total since they're workspace + source-data, not BVH.
        const double clTotalMb   = sectionStaticTotalBytes(s) / (1024.0 * 1024.0);
        const double prevTotalMb = sectionStaticTotalBytes(m_overlayStatsPrev) / (1024.0 * 1024.0);
        c = pos;
        drawSeg(L"  total ", c, kSubtle);
        drawSeg(fmtMb(fnum, _countof(fnum), clTotalMb), c, deltaColour(clTotalMb, prevTotalMb));
            drawSeg(L"  (CLASes + BLASes)", c, kSubtle);
        pos.y += kLineH;
        // Scratch + inputs: outside the BVH total above (scratch is
        // workspace the driver re-uses across builds; inputs are the
        // source vertex+index data the build reads from).  Same line
        // because they're conceptually the "supporting" memory.
        c = pos;
        drawSeg(L"  scratch ", c, kSubtle);
        drawSeg(fmtSize(fnum, _countof(fnum), s.traditionalBlasScratchBytes), c, deltaColour(scratchMb, prevScrMb));
            drawSeg(L"    inputs ", c, kSubtle);
        drawSeg(fmtSize(fnum, _countof(fnum), s.staticClusterInputBytes), c, deltaColour(inputsMb, prevInpMb));
        pos.y += kLineH;
    }
    pos.y += kSectionGap;

    // ----- ANIMATED section -----
    auto sectionAnimatedTotalBytes = [](const OverlayStats& x) -> UINT64 {
        // Resident BVH memory for the animated source AND every anim clone.
        // Cluster path: templates + per-frame CLAS + source BLAS + per-clone BLAS pool.
        // Trad   path: source DXR1 BLAS + per-clone DXR1 BLAS pool.
        return (x.geometryMode == (int)GeometryMode::Clusters)
            ? (x.animatedTemplateBytes + x.animatedPerFrameClasAllocBytes
               + x.animatedBlasBytes   + x.animClonesBlasPoolBytes)
            : (x.animatedTradBlasBytes + x.animClonesTradBlasPoolBytes);
    };
    if (m_animatedObjectEnabled)
    {
        const auto& a = m_animatedObject;
        const bool isTradMode = (s.geometryMode != (int)GeometryMode::Clusters);
        // Same-mode prev: animated stats are mode-specific (cluster
        // uses templates+CLAS+BLAS, trad uses one DXR1 BLAS), so a
        // [T] cross-mode toggle's delta should compare against the
        // last snapshot taken in the SAME mode, not the immediate prev
        // (which is the other mode).  See m_overlayStatsLastIn*.
        const auto& p = isTradMode
            ? (m_overlayStatsHasLastInTrad    ? m_overlayStatsLastInTrad    : m_overlayStatsPrev)
            : (m_overlayStatsHasLastInCluster ? m_overlayStatsLastInCluster : m_overlayStatsPrev);

        draw(L"ANIMATED:", pos, kAccent);
        pos.y += kLineH;
        if (isTradMode)
        {
            swprintf_s(buf, L"  %u clusters / %u verts / %u tris   (per-frame DXR1 %s)",
                       a.clusterCount, a.totalVertexCount, a.mesh.totalTriangles,
                       s.animatedTradModeIsRefit ? L"refit" : L"rebuild");
        }
        else
        {
            swprintf_s(buf, L"  %u clusters / %u verts / %u tris   (template + per-frame template instantiate path)",
                       a.clusterCount, a.totalVertexCount, a.mesh.totalTriangles);
        }
        draw(buf, pos, kSubtle); pos.y += kLineH;

        if (isTradMode)
        {
            // Traditional-mode animated stats: BLAS (resident BVH) plus
            // total + scratch/inputs breakout below.  No templates / no
            // per-frame CLAS in this mode.  perFrameVertexBuffer is
            // both written by AnimateBall.cs AND read by the BLAS
            // build, so it's a working buffer not a pure input -- the
            // "input" line here counts the flat IB + the rest-positions
            // buffer (the immutable input that AnimateBall.cs reads).
            const double aBlasMb   = s.animatedTradBlasBytes    / (1024.0 * 1024.0);
            const double prevABlas = p.animatedTradBlasBytes    / (1024.0 * 1024.0);
            const double aScrMb    = s.animatedTradScratchBytes / (1024.0 * 1024.0);
            const double prevAScr  = p.animatedTradScratchBytes / (1024.0 * 1024.0);
            const double aInputsMb = (s.animatedTradIbBytes + s.animatedRestPositionsBytes) / (1024.0 * 1024.0);
            const double prevAInp  = (p.animatedTradIbBytes + p.animatedRestPositionsBytes) / (1024.0 * 1024.0);
            // Phase-2 anim-clones trad pool (1 DXR1 BLAS per pooled clone,
            // rebuilt each frame).  Counted as resident memory; the
            // scratch is shared across the per-clone builds.
            const double cBlasMb   = s.animClonesTradBlasPoolBytes    / (1024.0 * 1024.0);
            const double prevCBlas = p.animClonesTradBlasPoolBytes    / (1024.0 * 1024.0);
            const double cScrMb    = s.animClonesTradBlasScratchBytes / (1024.0 * 1024.0);
            const double prevCScr  = p.animClonesTradBlasScratchBytes / (1024.0 * 1024.0);

            XMFLOAT2 c = pos;
            drawSeg(L"  BLAS ", c, kSubtle);
            drawSeg(fmtSize(fnum, _countof(fnum), s.animatedBlasBytes), c, deltaColour(aBlasMb, prevABlas));
            drawSeg(L"  (source)", c, kSubtle);
            pos.y += kLineH;
            if (s.animClonesPooledCount > 0 || p.animClonesPooledCount > 0)
            {
                c = pos;
                wchar_t poolLabel[64];
                swprintf_s(poolLabel, L"  clones pool ");
                drawSeg(poolLabel, c, kSubtle);
                drawSeg(fmt2(fnum, _countof(fnum), L"%.2f", cBlasMb), c, deltaColour(cBlasMb, prevCBlas));
                wchar_t poolCount[64];
                swprintf_s(poolCount, L" MB  (%u pooled clones; rest share source's BLAS)",
                           s.animClonesPooledCount);
                drawSeg(poolCount, c, kSubtle);
                pos.y += kLineH;
            }
            // Cross-mode section subtotal + label.  Trad's resident
            // animated memory = source BLAS + per-clone BLAS pool (when
            // phase-2 is active).  Scratch broken out below.
            const double tradAnimTotalMb = sectionAnimatedTotalBytes(s) / (1024.0 * 1024.0);
            const double prevAnimTotalMb = sectionAnimatedTotalBytes(m_overlayStatsPrev) / (1024.0 * 1024.0);
            c = pos;
            drawSeg(L"  total ", c, kSubtle);
            drawSeg(fmtMb(fnum, _countof(fnum), tradAnimTotalMb), c, deltaColour(tradAnimTotalMb, prevAnimTotalMb));
            drawSeg(L"  (source BLAS + clones pool)", c, kSubtle);
            pos.y += kLineH;
            c = pos;
            drawSeg(L"  scratch ", c, kSubtle);
            drawSeg(fmtMb(fnum, _countof(fnum), aScrMb + cScrMb), c, deltaColour(aScrMb + cScrMb, prevAScr + prevCScr));
            drawSeg(L"    inputs (ib + rest) ", c, kSubtle);
            drawSeg(fmtMb(fnum, _countof(fnum), aInputsMb), c, deltaColour(aInputsMb, prevAInp));
            pos.y += kLineH;
            pos.y += kSectionGap;
        }
        else
        {
            const double tplMb     = s.animatedTemplateBytes            / (1024.0 * 1024.0);
            const double prevTplMb = p.animatedTemplateBytes            / (1024.0 * 1024.0);
            const double pfMb      = s.animatedPerFrameClasAllocBytes   / (1024.0 * 1024.0);
            const double prevPfMb  = p.animatedPerFrameClasAllocBytes   / (1024.0 * 1024.0);
            const double pfActMb   = s.animatedPerFrameClasActualBytes  / (1024.0 * 1024.0);
            const double prevPfAct = p.animatedPerFrameClasActualBytes  / (1024.0 * 1024.0);
            const double aBlasMb   = s.animatedBlasBytes                / (1024.0 * 1024.0);
            const double prevABlas = p.animatedBlasBytes                / (1024.0 * 1024.0);
            const double cPoolMb   = s.animClonesBlasPoolBytes          / (1024.0 * 1024.0);
            const double prevCPool = p.animClonesBlasPoolBytes          / (1024.0 * 1024.0);
            const double cScrMb    = s.animClonesBlasScratchBytes       / (1024.0 * 1024.0);
            const double prevCScr  = p.animClonesBlasScratchBytes       / (1024.0 * 1024.0);
            // Scratch = template build + per-frame CLAS + BLAS-from-CLAS
            // scratch summed (all three are workspace re-used across
            // builds, all three are resident).  Also include the anim-
            // clones BLAS-from-CLAS scratch when phase 2 is active.
            const double scrMb     = (s.animatedTemplateScratchBytes
                                    + s.animatedPerFrameClasScratchBytes
                                    + s.animatedBlasScratchBytes
                                    + s.animClonesBlasScratchBytes) / (1024.0 * 1024.0);
            const double prevScrMb = (p.animatedTemplateScratchBytes
                                    + p.animatedPerFrameClasScratchBytes
                                    + p.animatedBlasScratchBytes
                                    + p.animClonesBlasScratchBytes) / (1024.0 * 1024.0);
            // Inputs = templateInputBuffer (hint vertex+index source for
            // BUILD_CLUSTER_TEMPLATES) + restPositionsBuffer (read every
            // frame by AnimateBall.cs).  Both immutable post-init.
            const double inputsMb  = (s.animatedTemplateInputBytes
                                    + s.animatedRestPositionsBytes) / (1024.0 * 1024.0);
            const double prevInpMb = (p.animatedTemplateInputBytes
                                    + p.animatedRestPositionsBytes) / (1024.0 * 1024.0);

            XMFLOAT2 c = pos;
            drawSeg(L"  templates ", c, kSubtle);
            drawSeg(fmtSize(fnum, _countof(fnum), s.animatedTemplateBytes), c, deltaColour(tplMb, prevTplMb));
            pos.y += kLineH;

            c = pos;
            drawSeg(L"  per-frame CLASes ", c, kSubtle);
            drawSeg(fmtSize(fnum, _countof(fnum), s.animatedPerFrameClasAllocBytes), c, deltaColour(pfMb, prevPfMb));
            drawSeg(L" alloc  (", c, kSubtle);
            drawSeg(fmtSize(fnum, _countof(fnum), s.animatedPerFrameClasActualBytes), c, deltaColour(pfActMb, prevPfAct));
            drawSeg(L" actual)", c, kSubtle);
            pos.y += kLineH;

            c = pos;
            drawSeg(L"  BLAS ", c, kSubtle);
            drawSeg(fmtSize(fnum, _countof(fnum), s.animatedBlasBytes), c, deltaColour(aBlasMb, prevABlas));
            drawSeg(L"  (source)", c, kSubtle);
            pos.y += kLineH;
            if (s.animClonesPooledCount > 0 || p.animClonesPooledCount > 0)
            {
                c = pos;
                drawSeg(L"  clones pool ", c, kSubtle);
                drawSeg(fmt2(fnum, _countof(fnum), L"%.2f", cPoolMb), c, deltaColour(cPoolMb, prevCPool));
                wchar_t poolCount[64];
                swprintf_s(poolCount, L" MB  (%u per-clone BLAS slots)", s.animClonesPooledCount);
                drawSeg(poolCount, c, kSubtle);
                pos.y += kLineH;
            }
            // Cross-mode section subtotal + label.  Cluster path's
            // resident animated memory = templates + per-frame CLAS
            // alloc + source BLAS + per-clone BLAS pool.  Scratch
            // and inputs broken out below.
            const double clAnimTotalMb   = sectionAnimatedTotalBytes(s) / (1024.0 * 1024.0);
            const double prevAnimTotalMb = sectionAnimatedTotalBytes(m_overlayStatsPrev) / (1024.0 * 1024.0);
            c = pos;
            drawSeg(L"  total ", c, kSubtle);
            drawSeg(fmtMb(fnum, _countof(fnum), clAnimTotalMb), c, deltaColour(clAnimTotalMb, prevAnimTotalMb));
            drawSeg(L"  (templates + CLASes + source BLAS + clones pool)", c, kSubtle);
            pos.y += kLineH;
            c = pos;
            drawSeg(L"  scratch ", c, kSubtle);
            drawSeg(fmtMb(fnum, _countof(fnum), scrMb), c, deltaColour(scrMb, prevScrMb));
            drawSeg(L"    inputs (hint+rest) ", c, kSubtle);
            drawSeg(fmtSize(fnum, _countof(fnum), s.staticClusterInputBytes), c, deltaColour(inputsMb, prevInpMb));
            pos.y += kLineH;
            pos.y += kSectionGap;
        }
    }

    // ----- TOTAL section -----
    {
        const auto& p = m_overlayStatsPrev;
        // animatedTotal = cluster-path templates + per-frame CLAS + source BLAS +
        //                 per-clone BLAS pool
        //                 OR trad-path source BLAS + per-clone BLAS pool
        //                 (mode-exclusive: only one mode's fields are non-zero).
        const UINT64 animatedTotal     = s.animatedTemplateBytes
                                       + s.animatedPerFrameClasAllocBytes
                                       + s.animatedBlasBytes
                                       + s.animatedTradBlasBytes
                                       + s.animClonesBlasPoolBytes
                                       + s.animClonesTradBlasPoolBytes;
        const UINT64 prevAnimatedTotal = p.animatedTemplateBytes
                                       + p.animatedPerFrameClasAllocBytes
                                       + p.animatedBlasBytes
                                       + p.animatedTradBlasBytes
                                       + p.animClonesBlasPoolBytes
                                       + p.animClonesTradBlasPoolBytes;
        // TOTAL line shows the bytes currently RESIDENT in GPU memory.
        // For Compact traditional, the worst-case temp buffer is freed
        // after the build, so the resident total uses the compacted
        // (actual) size, not the original prebuild alloc.
        const UINT64 staticTotal       = isTraditional
            ? s.traditionalBlasActualBytes
            : (s.staticClasAllocBytes + s.staticBlasTotalBytes);
        const UINT64 prevStaticTotal   = (p.geometryMode != (int)GeometryMode::Clusters)
            ? p.traditionalBlasActualBytes
            : (p.staticClasAllocBytes + p.staticBlasTotalBytes);
        const UINT64 grandTotal     = staticTotal + animatedTotal + s.tlasBytes;
        const UINT64 prevGrandTotal = prevStaticTotal + prevAnimatedTotal + p.tlasBytes;

        draw(L"TOTAL:", pos, kAccent);
        pos.y += kLineH;

        const double grandMb = grandTotal / (1024.0 * 1024.0);
        const double statMb  = staticTotal / (1024.0 * 1024.0);
        const double animMb  = animatedTotal / (1024.0 * 1024.0);
        const double tlasMb  = s.tlasBytes / (1024.0 * 1024.0);
        XMFLOAT2 c = pos;
        drawSeg(L"  AS memory ", c, kSubtle);
        drawSeg(fmtSize(fnum, _countof(fnum), grandTotal), c, deltaColourU(grandTotal, prevGrandTotal));
            drawSeg(L"   (static ", c, kSubtle);
        drawSeg(fmtSize(fnum, _countof(fnum), staticTotal), c, deltaColourU(staticTotal, prevStaticTotal));
        if (animatedTotal > 0 || prevAnimatedTotal > 0)
        {
            drawSeg(L" + animated ", c, kSubtle);
            drawSeg(fmtSize(fnum, _countof(fnum), animatedTotal), c, deltaColourU(animatedTotal, prevAnimatedTotal));
        }
        drawSeg(L" + TLAS ", c, kSubtle);
        drawSeg(fmtSize(fnum, _countof(fnum), s.tlasBytes), c, deltaColourU(s.tlasBytes, p.tlasBytes));
            drawSeg(L")", c, kSubtle);
        pos.y += kLineH;
        pos.y += kSectionGap;
    }

    // ----- PER-FRAME section -----
    // Times are snapped from the EMA a few frames after each config change so
    // the displayed value belongs to the current config (not a smear of pre +
    // post-rebuild samples).  Splitting INSTANTIATE vs BLAS vs TLAS is what
    // makes the precision / vertex-format sweep meaningful: changing
    // precision should move ONLY the INSTANTIATE column.
    if (m_animatedObjectEnabled)
    {
        const auto& p = m_overlayStatsPrev;
        draw(L"PER-FRAME:", pos, kAccent);
        pos.y += kLineH;
        // During the post-toggle settle (~kPerFrameRingSlots +
        // m_pfSnapTargetCount frames), the rolling EMA window is still
        // refilling with new-mode samples, and m_overlayStats.pf* still
        // holds the OLD mode's averages.  Showing those would mislead
        // the user into thinking the toggle had no effect on per-frame
        // cost; print "recalculating..." instead so the lag is explicit.
        // (N/M) progress is most useful on WARP where each sample is a
        // full multi-second frame -- watching it tick from 0/5 to 5/5
        // confirms the app is still alive and gives a rough ETA.
        if (m_pfTimingSettlingAfterToggle)
        {
            wchar_t pbuf[64];
            swprintf_s(pbuf, L"  recalculating... (%d/%d)",
                       (int)m_pfSnapSamplesCollected,
                       (int)m_pfSnapTargetCount);
            draw(pbuf, pos, kSubtle);
            pos.y += kLineH;
        }
        else if (s.pfTimingValid)
        {
            const double totalMs     = s.pfInstantiateMs + s.pfBlasRebuildMs + s.pfTlasRebuildMs;
            const double prevTotalMs = p.pfInstantiateMs + p.pfBlasRebuildMs + p.pfTlasRebuildMs;
            // Only colour-compare the per-frame timings if prev had valid
            // timing too (else we'd compare against a stale 0.0 every time).
            // pfPickV takes an explicit prevValid so callers can pass a
            // different prev's validity bit (cross-mode prev pX vs
            // same-mode prev p have independently-valid timing flags).
            auto pfPickV = [&](double cur, double prev, bool prevValid) -> XMVECTOR {
                if (!deltaActive || !prevValid) return kSubtle;
                const double base = std::max(std::abs(prev), 1e-9);
                const double rel  = std::abs(cur - prev) / base;
                if (rel < kDeltaPctThreshold) return kSubtle;
                return (cur < prev) ? kGreen : kRed;
            };
            auto pfPick = [&](double cur, double prev) -> XMVECTOR {
                return pfPickV(cur, prev, p.pfTimingValid);
            };
            // Auto-picked unit so the user always knows what scale they're
            // looking at -- "0.073" is ambiguous (ms? us? s?), "73 us" /
            // "0.073 ms" / "1.5 s" are not.  Switch us<->ms at the 1 ms mark
            // and ms<->s at the 1000 ms mark, both round numbers.
            //
            // NOTE: we deliberately use the ASCII "us" rather than the U+00B5
            // micro-sign because MakeSpriteFont's default character range only
            // covers ASCII 32-126 -- the spritefont file we ship has NO glyph
            // for the micro-sign and SpriteFont::DrawString throws std::runtime_error
            // ("character not in the font") when it sees one, which propagates
            // out of WndProc as STATUS_FATAL_USER_CALLBACK_EXCEPTION (0xC000041D).
            // Regenerating the spritefont with /CharacterRegion would also work
            // but ASCII "us" is universally readable and avoids the asset rebuild.
            auto fmtTime = [](wchar_t* dst, size_t cch, double ms) -> wchar_t* {
                if (ms >= 1000.0)      swprintf_s(dst, cch, L"%.3f s",  ms / 1000.0);
                else if (ms >= 1.0)    swprintf_s(dst, cch, L"%.3f ms", ms);
                else                   swprintf_s(dst, cch, L"%.1f us", ms * 1000.0);
                return dst;
            };

            // The "(...)" sub-time labels differ by mode (and by [F] toggle
            // in trad mode):
            //
            //   Slot 0..1 (first sub-time):
            //     Cluster: pfTimestampBase+0..+1 brackets ONLY
            //              INSTANTIATE_CLUSTER_TEMPLATES (the compute
            //              deform runs untimed in the prologue)
            //              -> "template instantiate"
            //     Trad:    same slot brackets the AnimateBall.cs compute
            //              deform (there are no templates in trad mode
            //              at all) -> "compute deform"
            //
            //   Slot 2..3 (BLAS sub-time):
            //     Cluster: BUILD_BLAS_FROM_CLAS -> "BLAS from CLAS"
            //     Trad:    BuildRaytracingAccelerationStructure on
            //              the animated DXR1 BLAS, with mode depending
            //              on [F] toggle -> "BLAS refit" or "BLAS rebuild"
            //
            // The user's intuitive cross-mode comparison ("template
            // instantiate is morally equivalent to a refit") is then
            // possible by reading the cluster line's first sub-time
            // against the trad line's BLAS-refit sub-time -- both are
            // "incremental work to refresh a pre-built BVH from new
            // vertex positions".
            //
            // Delta-colour prev pointer SELECTION per sub-time:
            //   Slot 0..1 (instantiate vs deform): use SAME-mode prev
            //     ('p' above).  Cross-mode coloring here would compare
            //     two genuinely-different ops timed in the same slot,
            //     which is misleading -- "deform got 5us cheaper than
            //     instantiate" is meaningless.
            //   Slot 2..3 (BLAS), slot 4..5 (TLAS), and the rebuild
            //   TOTAL: use IMMEDIATE prev (m_overlayStatsPrev).  In
            //     same-mode frames this IS the same-mode prev so
            //     behaviour matches today; right after a [T] toggle
            //     the immediate prev is the OTHER mode's last snapshot
            //     and the colour now shows the cross-mode delta the
            //     user actually wants to see ("did BLAS work get
            //     faster/slower when I switched modes?").  BLAS work
            //     is morally comparable cross-mode (both produce the
            //     final animated BVH from inputs); TLAS is the same op
            //     in both modes; the total approximates "all per-frame
            //     AS work" in either mode (cluster total is missing
            //     the deform, but the deform is small enough that the
            //     comparison stays useful).
            const bool   isClusterMode = (m_geometryMode == GeometryMode::Clusters);
            const wchar_t* pfSubLabel  = isClusterMode
                ? L"   (template instantiate "
                : L"   (compute deform ";
            const wchar_t* pfBlasLabel = isClusterMode
                ? L"  +  BLAS from CLAS "
                : (s.animatedTradModeIsRefit ? L"  +  BLAS refit " : L"  +  BLAS rebuild ");

            // Cross-mode-aware prev for the comparable sub-times.  Note
            // we recompute prevTotalMs from this prev so the TOTAL
            // colour is consistent with the sum of its sub-times'
            // colours.
            const auto&  pX            = m_overlayStatsPrev;
            const double prevTotalMsX  = pX.pfInstantiateMs + pX.pfBlasRebuildMs + pX.pfTlasRebuildMs;
            const bool   pXValid       = pX.pfTimingValid;

            XMFLOAT2 c = pos;
            drawSeg(L"  rebuild ", c, kSubtle);
            drawSeg(fmtTime(fnum, _countof(fnum), totalMs), c, pfPickV(totalMs, prevTotalMsX, pXValid));
            drawSeg(pfSubLabel, c, kSubtle);
            // First sub-time uses SAME-mode prev 'p' (see comment above).
            drawSeg(fmtTime(fnum, _countof(fnum), s.pfInstantiateMs), c, pfPick(s.pfInstantiateMs, p.pfInstantiateMs));
            drawSeg(pfBlasLabel, c, kSubtle);
            drawSeg(fmtTime(fnum, _countof(fnum), s.pfBlasRebuildMs), c, pfPickV(s.pfBlasRebuildMs, pX.pfBlasRebuildMs, pXValid));
            drawSeg(L"  +  TLAS ", c, kSubtle);
            drawSeg(fmtTime(fnum, _countof(fnum), s.pfTlasRebuildMs), c, pfPickV(s.pfTlasRebuildMs, pX.pfTlasRebuildMs, pXValid));
            drawSeg(L")", c, kSubtle);
            pos.y += kLineH;

            // Per-frame STATIC-AS rebuild line.  CLUSTER-MODE-ONLY: the [R]
            // toggle is gated on cluster mode (see OnKeyDown), and the
            // per-frame static rebuild path itself is gated on cluster
            // mode in OnRender.  m_staticRebuildMode is sticky across [T]
            // toggles though, so without an explicit mode-check here the
            // line (including the "[CLAS rebuild requires Implicit alloc;
            // press [A]]" hint) would still render in trad mode -- both
            // misleading (the rebuild isn't actually running) and stale
            // (the [A] hint refers to CLAS, which doesn't exist in trad).
            if (m_staticRebuildMode != StaticRebuildMode::None &&
                m_geometryMode == GeometryMode::Clusters)
            {
                const bool clasActive =
                    (m_staticRebuildMode == StaticRebuildMode::ClasAndBlas) &&
                    (m_clasAllocMode == ClasAllocMode::Implicit);
                const double staticTotal     = (clasActive ? s.pfStaticClasMs : 0.0)
                                             + s.pfStaticBlasMs;
                const double prevStaticTotal = (clasActive ? p.pfStaticClasMs : 0.0)
                                             + p.pfStaticBlasMs;
                XMFLOAT2 c2 = pos;
                drawSeg(L"  static rebuild ", c2, kSubtle);
                drawSeg(fmtTime(fnum, _countof(fnum), staticTotal), c2,
                        pfPick(staticTotal, prevStaticTotal));
                drawSeg(L"   (", c2, kSubtle);
                if (clasActive)
                {
                    drawSeg(L"CLASes ", c2, kSubtle);
                    drawSeg(fmtTime(fnum, _countof(fnum), s.pfStaticClasMs), c2,
                            pfPick(s.pfStaticClasMs, p.pfStaticClasMs));
                    drawSeg(L"  +  ", c2, kSubtle);
                }
                drawSeg(L"BLASes ", c2, kSubtle);
                drawSeg(fmtTime(fnum, _countof(fnum), s.pfStaticBlasMs), c2,
                        pfPick(s.pfStaticBlasMs, p.pfStaticBlasMs));
                drawSeg(L")", c2, kSubtle);
                if (m_staticRebuildMode == StaticRebuildMode::ClasAndBlas &&
                    m_clasAllocMode != ClasAllocMode::Implicit)
                {
                    drawSeg(L"   [CLAS rebuild requires Implicit alloc; press [A]]",
                            c2, kHotkey);
                }
                pos.y += kLineH;
            }
        }
        else
        {
            draw(L"  measuring...", pos, kSubtle);
            pos.y += kLineH;
        }
        pos.y += kSectionGap;
    }

    // ----- FPS section.  The ONLY number that's truly live - we don't
    //       snapshot it because the whole point of FPS is the instantaneous
    //       value the user can watch fluctuate.  Section header in blue +
    //       indented value in white, matching the convention above.
    //
    //       Both displayed values (FPS and ms/frame) are derived from the
    //       SAME wall-clock measurement (rolling average of the last N
    //       frame deltas; N=m_frameTimeWindow, auto-resolved per adapter
    //       in OnInit -- 60 on HW, 3 on WARP).  Reasons we don't use
    //       m_timer.GetElapsedSeconds() and m_timer.GetFramesPerSecond()
    //       any more:
    //         1. StepTimer caps elapsed at 100 ms (m_qpcMaxDelta) so on
    //            anything slower than 10 fps the ms reads "100.0" forever
    //            -- doesn't match the integer fps the same StepTimer
    //            shows (which uses unclamped delta into its 1-second
    //            sliding window).  Two values from the same struct
    //            disagreed by 30x on WARP.
    //         2. StepTimer's fps is integer, rounding sub-1-fps to 0 or
    //            1 -- ambiguous in exactly the WARP case we'd care.
    //       Both fixed by 1) measuring real frame-to-frame wall clock
    //       (no clamp) and 2) deriving fps as 1.0/seconds with
    //       sub-10-fps shown to 1-2 decimals so 0.05 / 0.3 / 5.4 fps
    //       are readable values, not rounded mush.  Time unit auto-
    //       switches to seconds at >= 1000 ms (i.e. fps <= 1) so WARP's
    //       many-seconds-per-frame reads naturally.
    draw(L"FPS:", pos, kAccent);
    pos.y += kLineH;
    const double secPerFrame = (m_frameTimeRingCount > 0)
                               ? (m_frameTimeRingSum / m_frameTimeRingCount)
                               : 0.0;
    const double fps         = (secPerFrame > 0.0) ? (1.0 / secPerFrame) : 0.0;
    const double msPerFrame  = secPerFrame * 1000.0;
    // FPS format: 2 decimals below 1 fps (so 0.05 fps doesn't round to
    // "0.0"), 1 decimal between 1 and 10 fps, integer above.
    wchar_t fpsBuf[32];
    if (fps < 1.0)
        swprintf_s(fpsBuf, L"%.2f", fps);
    else if (fps < 10.0)
        swprintf_s(fpsBuf, L"%.1f", fps);
    else
        swprintf_s(fpsBuf, L"%u", (unsigned)(fps + 0.5));
    if (msPerFrame >= 1000.0)
        swprintf_s(buf, L"  %ls   (%.2f s/frame)", fpsBuf, secPerFrame);
    else
        swprintf_s(buf, L"  %ls   (%.1f ms/frame)", fpsBuf, msPerFrame);
    draw(buf, pos, kSubtle);
    pos.y += kLineH * 1.4f;

    // ----- Interactive controls.  Key prefix coloured amber, label + current
    // value in white -- each line owns its own value so there's no need to
    // hunt up the screen for "what is it set to right now?".  The "(-/+)"
    // hint is omitted because ',' and '.' / '[' and ']' are visually paired
    // keys -- you can tell from the prefix which way each one moves.
    //
    // Column 2: anchored at (m_overlayCol1MaxRightUnscaled * kScale +
    // leftMargin + kCol2Gap).  Using the UNSCALED last-frame value
    // means col 2 reflows correctly when kScale changes (e.g. on
    // window resize), while still being monotonic-max (in atlas units)
    // so the keys don't jump leftward as numbers shrink.  Aligned with
    // the top of column 1 (the SCENE row) so the vertical space the
    // keys used to take below FPS is now free for the scene.
    // Tracking flipped off so the keys' own backing rects don't
    // contribute to the col 1 width tracker for next frame.
    flushPending();   // close out any pending col1 rect
    trackCol1 = false;
    pos = XMFLOAT2(kLeftMargin + m_overlayCol1MaxRightUnscaled * kScale + kCol2Gap,
                   col2StartY);
    auto drawKeyLine = [&](const wchar_t* keyPrefix, const wchar_t* tail) {
        XMFLOAT2 p = pos;
        draw(keyPrefix, p, kHotkey);
        p.x += measureX(keyPrefix);
        draw(tail, p, kWhite);
        pos.y += kLineH;
    };

    wchar_t kbuf[256];

    // ----- [T] geometry mode (always shown; locked-out hint if HW can't do clusters) -----
    if (m_clustersAndPtlasSupported)
        swprintf_s(kbuf, L"   geometry path:    %s", GeometryModeName());
    else
        swprintf_s(kbuf, L"   geometry path:    %s  (locked -- clusters not supported)",
                   GeometryModeName());
    drawKeyLine(L"[T]", kbuf);

    if (IsTraditional())
    {
        // [A] cycles traditional BLAS alloc mode (compact <-> implicit).
        swprintf_s(kbuf, L"   BLAS alloc mode:  %s", TraditionalAllocModeName());
        drawKeyLine(L"[A]", kbuf);

        // [F] is only meaningful in traditional mode (animated BLAS
        // update strategy).  Hide in cluster mode.
        swprintf_s(kbuf, L"   anim BLAS update: %s   (per-frame rebuild vs ALLOW_UPDATE refit)",
                   TraditionalAnimModeName());
        drawKeyLine(L"[F]", kbuf);
    }
    else
    {
        // Cluster-only knobs.  Hidden in traditional mode -- they don't
        // apply (no CLAS, no per-cluster precision).
        swprintf_s(kbuf, L"   CLAS alloc mode:  %s", ClasAllocModeName());
        drawKeyLine(L"[A]", kbuf);

        swprintf_s(kbuf, L"   per-frame static-AS rebuild:  %s", StaticRebuildModeName());
        drawKeyLine(L"[R]", kbuf);

        swprintf_s(kbuf, L"   vertex format:    %s",
                   m_vertexMode == VertexMode::Float32_3 ? L"FLOAT32_3" : L"COMPRESSED1");
        drawKeyLine(L"[V]", kbuf);

        if (m_vertexMode == VertexMode::Float32_3)
            swprintf_s(kbuf, L"   vertex precision: %u bits/component   (32-bit float, PositionTruncateBitCount=%u)",
                       32u - m_positionTruncateBits, m_positionTruncateBits);
        else
            swprintf_s(kbuf, L"   vertex precision: %u bits/component   (shared exponent)",
                       m_compressedBitsPerComponent);
        drawKeyLine(L"[ ]", kbuf);
    }

    // [B] BVH-build-flag preference -- applied uniformly to BOTH trad
    // (DXR1) and cluster (DXR2) AS builds via BuildFlagModeRtas() /
    // BuildFlagModeDxr1().  Toggling rebuilds the static AS once
    // (unless per-frame static rebuild is already on).
    swprintf_s(kbuf, L"   BVH build flag:   %s", BuildFlagModeName());
    drawKeyLine(L"[B]", kbuf);

    // Workload-scaling [N] toggle.  Always shown so the user knows the
    // hotkey exists even when N=0.  Reports the count + breakdown
    // (static clones + animated clones share-source-BLAS) + LOD state.
    //
    // Mode-aware label so it's clear these are NOT cheap TLAS-instance
    // copies of a shared BLAS -- each STATIC clone gets its own full
    // CLAS array + BLAS (in cluster mode) or its own DXR1 BLAS (in
    // trad mode), so the toggle progressively stresses the
    // CLAS+BLAS+templates paths rather than just the TLAS row count.
    // ANIMATED clones currently SHARE the source's per-frame BLAS
    // (phase 1) -- the breakdown count flags them separately so the
    // reader can mentally subtract them when reasoning about the
    // unique-BLAS budget.
    if (m_extraInstancesMode == ExtraInstancesMode::None)
    {
        const wchar_t* label = (m_geometryMode == GeometryMode::Clusters)
            ? L"   extra unique BLAS+CLAS: %s"
            : L"   extra unique BLASes:    %s";
        swprintf_s(kbuf, label, ExtraInstancesModeName());
    }
    else
    {
        const UINT N_static_clones = (UINT)(m_objects.size() - m_sourceObjectCount);
        const UINT N_anim_clones   = (UINT)m_animatedClones.size();
        const wchar_t* fmt = (m_geometryMode == GeometryMode::Clusters)
            ? L"   extra unique BLAS+CLAS: %s   (%u static + %u animated [own BLAS, shared CLAS])   LOD reduces with distance"
            : L"   extra unique BLASes:    %s   (%u static + %u animated [own BLAS, per-clone DXR1 rebuild])   LOD reduces with distance";
        swprintf_s(kbuf, fmt,
                   ExtraInstancesModeName(), N_static_clones, N_anim_clones);
    }
    drawKeyLine(L"[N]", kbuf);

    swprintf_s(kbuf, L"   ray bounces:      reflection %u   refraction %u",
               ReflectionBounces(), RefractionBounces());
    drawKeyLine(L", .", kbuf);

    // [M] animation + [Space] camera pause -- two independent toggles
    // (decoupled in the pause-fix refactor).  Side-by-side at the bottom of
    // the key list.  [Space] is the absolute bottom because it's the most
    // common interactive press (freezing the camera to study a frame); [M]
    // sits just above it for the secondary "freeze the wobble" control.
    swprintf_s(kbuf, L"   animation:        %s", m_animPaused ? L"PAUSED" : L"playing");
    drawKeyLine(L"[M]", kbuf);

    swprintf_s(kbuf, L"   camera pan:       %s", m_cameraPaused ? L"PAUSED" : L"playing");
    drawKeyLine(L"space", kbuf);

    // Flush any pending drawSeg whose right-pad we haven't decided yet
    // -- it's the last segment of the overlay, so it gets the right-pad.
    flushPending();

    // Convert this frame's max right edges (pixels) back to unscaled
    // atlas units, then max() into the persistent members for next
    // frame's fit-scale + col2 anchor computation.
    //   col1 content width (unscaled) = (col1_right_px - leftMargin) / kScale
    //   col2 starts at: leftMargin + col1_unscaled_last_frame * kScale + gap
    //                 = pos_x_we_used_for_col2_start
    //   col2 content width (unscaled) = (any_right_px - col2_start_x) / kScale
    //   total content width (unscaled) = col1_unscaled + col2_unscaled
    // Both members are MAX-monotonic during the session so kScale only
    // ever decreases (overflow protection) -- if a wider line appears,
    // we shrink to fit and stay shrunk.  Resetting requires app restart
    // (acceptable -- the alternative is oscillation between scales).
    if (kScale > 0.0f) {
        const float col1UnscaledThisFrame = std::max(0.0f,
            (thisFrameCol1RightPx - kLeftMargin) / kScale);
        const float col2StartXPx          = kLeftMargin
                                          + m_overlayCol1MaxRightUnscaled * kScale
                                          + kCol2Gap;
        const float col2UnscaledThisFrame = std::max(0.0f,
            (thisFrameAnyRightPx - col2StartXPx) / kScale);
        // Height counterpart: any rect's bottom minus topMargin gives
        // the overlay's content height in pixels at current scale;
        // divide by kScale for atlas units.
        const float heightUnscaledThisFrame = std::max(0.0f,
            (thisFrameAnyBottomPx - kTopMargin) / kScale);
        m_overlayCol1MaxRightUnscaled  = std::max(m_overlayCol1MaxRightUnscaled,
                                                  col1UnscaledThisFrame);
        m_overlayContentUnscaledWidth  = std::max(m_overlayContentUnscaledWidth,
                                                  col1UnscaledThisFrame + col2UnscaledThisFrame);
        m_overlayContentUnscaledHeight = std::max(m_overlayContentUnscaledHeight,
                                                  heightUnscaledThisFrame);
    }
    m_spriteBatch->End();
}
