//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
//
//*********************************************************

// ============================================================================
// InputHandling.cpp
//
// CLI argument parsing and keyboard input handling, factored out of the
// main D3D12RaytracingClusteredGeometry.cpp so that file can focus on CORE
// acceleration-structure management + rendering.
//
// Contains:
//   * ParseCommandLineArgs -- CLI options for screenshots, benchmarks,
//     geometry/build modes, scheduled actions, and deterministic captures.
//   * OnKeyDown            -- hotkey routing for Space, [M], [A], [V], [R],
//     [T], [F], [N], [B], "," ".", "[" "]" runtime toggles.  Each hotkey
//     either mutates a piece of state and triggers a
//     RebuildStaticAccelerationStructures, or flips a per-frame flag.
//
// All methods here are still members of D3D12RaytracingClusteredGeometry.
// ============================================================================

#include "stdafx.h"
#include "D3D12RaytracingClusteredGeometry.h"

using namespace std;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

// ---- ParseCommandLineArgs -------------------------------------------
void D3D12RaytracingClusteredGeometry::ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc)
{
    DXSample::ParseCommandLineArgs(argv, argc);
    for (int i = 1; i < argc; i++)
    {
        if (_wcsicmp(argv[i], L"--screenshot") == 0 && i + 2 < argc)
        {
            m_screenshotFrame = _wtoi(argv[i + 1]);
            m_screenshotPath  = argv[i + 2];
            i += 2;
        }
        else if (_wcsicmp(argv[i], L"--screenshot-at") == 0 && i + 2 < argc)
        {
            // --screenshot-at <seconds> <path> : jump m_animSeconds to <seconds>
            // on frame 0 and capture once the swap chain has warmed up. Useful
            // for verifying the time-based camera orbit at known angles
            // (orbit period is 30s).
            m_screenshotAtSeconds = _wtof(argv[i + 1]);
            m_screenshotPath      = argv[i + 2];
            i += 2;
        }
        else if (_wcsicmp(argv[i], L"--vertex-format") == 0 && i + 1 < argc)
        {
            if      (_wcsicmp(argv[i+1], L"float")      == 0) m_vertexMode = VertexMode::Float32_3;
            else if (_wcsicmp(argv[i+1], L"compressed") == 0) m_vertexMode = VertexMode::Compressed1;
            i += 1;
        }
        else if (_wcsicmp(argv[i], L"--clas-alloc") == 0 && i + 1 < argc)
        {
            // CLAS memory-allocation strategy. See ClasAllocMode in the header for
            // the contract of each mode; the [CLAS mem] log line at init time
            // reports the stats that change between them.
            if      (_wcsicmp(argv[i+1], L"implicit")  == 0) m_clasAllocMode = ClasAllocMode::Implicit;
            else if (_wcsicmp(argv[i+1], L"get-sizes") == 0) m_clasAllocMode = ClasAllocMode::GetSizes;
            else if (_wcsicmp(argv[i+1], L"compact")   == 0) m_clasAllocMode = ClasAllocMode::Compact;
            i += 1;
        }
        else if (_wcsicmp(argv[i], L"--position-truncate") == 0 && i + 1 < argc)
        {
            // FLOAT32_3 mode only - clamp the per-vertex position mantissa to
            // 23-N bits by zeroing the low N. Range 0 (full precision, default)
            // to 22 (only sign+exponent kept). Sweet spot for sub-mm scenes is
            // 8-12. Ignored under --vertex-format compressed.
            int n = _wtoi(argv[i+1]);
            if (n < 0)  n = 0;
            if (n > 22) n = 22;
            m_positionTruncateBits = (UINT)n;
            i += 1;
        }
        else if (_wcsicmp(argv[i], L"--geometry-mode") == 0 && i + 1 < argc)
        {
            // [T] runtime toggle, set from CLI for headless / scripted runs.
            //   clusters    - DXR2 cluster-based BLAS (default)
            //   traditional - classic DXR1 per-object monolithic BLAS
            if      (_wcsicmp(argv[i+1], L"clusters")    == 0) m_geometryMode = GeometryMode::Clusters;
            else if (_wcsicmp(argv[i+1], L"traditional") == 0) m_geometryMode = GeometryMode::Traditional;
            i += 1;
        }
        else if (_wcsicmp(argv[i], L"--build-flags") == 0 && i + 1 < argc)
        {
            // [B] runtime toggle, set from CLI for headless / scripted runs.
            //   none       - D3D12_..._FLAG_NONE   (driver default, typically biased to FAST_TRACE)
            //   fast-build - PREFER_FAST_BUILD / FAST_BUILD
            //   fast-trace - PREFER_FAST_TRACE / FAST_TRACE (default)
            if      (_wcsicmp(argv[i+1], L"none")       == 0) m_buildFlagMode = BuildFlagMode::None;
            else if (_wcsicmp(argv[i+1], L"fast-build") == 0) m_buildFlagMode = BuildFlagMode::FastBuild;
            else if (_wcsicmp(argv[i+1], L"fast-trace") == 0) m_buildFlagMode = BuildFlagMode::FastTrace;
            i += 1;
        }
        else if (_wcsicmp(argv[i], L"--trad-alloc") == 0 && i + 1 < argc)
        {
            // [A] runtime toggle in traditional mode, set from CLI.  Mirrors
            // the cluster path's --clas-alloc but two-mode instead of three.
            if      (_wcsicmp(argv[i+1], L"implicit") == 0) m_traditionalAllocMode = TraditionalAllocMode::Implicit;
            else if (_wcsicmp(argv[i+1], L"compact")  == 0) m_traditionalAllocMode = TraditionalAllocMode::Compact;
            i += 1;
        }
        else if (_wcsicmp(argv[i], L"--trad-anim") == 0 && i + 1 < argc)
        {
            // [F] runtime toggle in traditional mode (per-frame animated BLAS).
            //   rebuild - full BuildRaytracingAccelerationStructure each frame
            //             (FAST_TRACE quality; fresh topology every frame --
            //             apples-to-apples comparison with the cluster path's
            //             INSTANTIATE + BUILD_BLAS_FROM_CLAS pipeline).
            //   refit   - same Build call with PERFORM_UPDATE flag (reuses
            //             topology; ~4-10x cheaper but quality drifts as
            //             vertices wobble away from the cached topology).
            //             No equivalent in the cluster path -- INSTANTIATE
            //             always re-encodes leaves and BUILD_BLAS_FROM_CLAS
            //             always re-tops the BLAS.
            if      (_wcsicmp(argv[i+1], L"rebuild") == 0) m_traditionalAnimMode = TraditionalAnimMode::Rebuild;
            else if (_wcsicmp(argv[i+1], L"refit")   == 0) m_traditionalAnimMode = TraditionalAnimMode::Refit;
            i += 1;
        }
        else if (_wcsicmp(argv[i], L"--rebuild-mode") == 0 && i + 1 < argc)
        {
            // Per-frame static-AS rebuild mode = [R] runtime toggle, set from CLI
            // for headless / scripted measurement runs.
            //   none      - off (default)
            //   blas      - re-run BUILD_BLAS_FROM_CLAS every frame
            //   clas-blas - re-run static CLAS + BLAS every frame (CLAS only
            //               actually runs in --clas-alloc implicit)
            if      (_wcsicmp(argv[i+1], L"none")      == 0) m_staticRebuildMode = StaticRebuildMode::None;
            else if (_wcsicmp(argv[i+1], L"blas")      == 0) m_staticRebuildMode = StaticRebuildMode::BlasOnly;
            else if (_wcsicmp(argv[i+1], L"clas-blas") == 0) m_staticRebuildMode = StaticRebuildMode::ClasAndBlas;
            i += 1;
        }
        else if (_wcsicmp(argv[i], L"--log-pf-every") == 0 && i + 1 < argc)
        {
            // Every N frames, dump the per-frame timing EMA values (animated
            // INSTANTIATE / animated BLAS / TLAS / static BLAS / static CLAS)
            // to the SampleLog so a wrapper script can scrape them.  0 disables.
            int n = _wtoi(argv[i+1]);
            m_logPfEveryFrames = (UINT)std::max(0, n);
            i += 1;
        }
        else if (_wcsicmp(argv[i], L"--log-raw-every") == 0 && i + 1 < argc)
        {
            // Every N frames, dump the RAW (un-smoothed) per-frame timestamp
            // deltas, not the EMA.  Useful for catching transients the EMA
            // smooths away.  0 disables.
            int n = _wtoi(argv[i+1]);
            m_logRawPfEveryFrames = (UINT)std::max(0, n);
            i += 1;
        }
        else if (_wcsicmp(argv[i], L"--exit-after-frames") == 0 && i + 1 < argc)
        {
            // Quit cleanly after rendering N frames (post-init).  Headless
            // measurement helper -- pair with --log-pf-every to capture a
            // settled timing run and exit.
            int n = _wtoi(argv[i+1]);
            m_exitAfterFrames = (UINT)std::max(0, n);
            i += 1;
        }
        else if (_wcsicmp(argv[i], L"--extra-instances") == 0 && i + 1 < argc)
        {
            // [N] runtime toggle, set from CLI for headless / scripted runs.
            // Accepts 0 / 100 / 1000 / 10000 (or 1k / 10k as a shorthand).
            // Initial scene build picks up m_extraInstancesMode in BuildScene
            // -> RegenerateWorkloadCloneInstances, so the clones are present
            // from frame 1 -- no scheduled-action gymnastics needed.  This is
            // distinct from --at <frame>:extra-NNN which fires AFTER init
            // and incurs a per-frame stall.
            const wchar_t* v = argv[i+1];
            if      (_wcsicmp(v, L"0")     == 0)  m_extraInstancesMode = ExtraInstancesMode::None;
            else if (_wcsicmp(v, L"100")   == 0)  m_extraInstancesMode = ExtraInstancesMode::Hundred;
            else if (_wcsicmp(v, L"1000")  == 0 ||
                     _wcsicmp(v, L"1k")    == 0)  m_extraInstancesMode = ExtraInstancesMode::Thousand;
            else if (_wcsicmp(v, L"10000") == 0 ||
                     _wcsicmp(v, L"10k")   == 0)  m_extraInstancesMode = ExtraInstancesMode::TenThousand;
            i += 1;
        }
        else if (_wcsicmp(argv[i], L"--bench-seconds") == 0 && i + 1 < argc)
        {
            // Wall-clock benchmark mode.  After init + first rendered frame,
            // measure for N seconds, then write JSON to --bench-out and
            // PostQuitMessage.  Wall-clock instead of frame count because
            // FPS varies 1000x between HW (~120 fps) and WARP (~0.1 fps);
            // a frame-count exit would either misfire on slow adapters or
            // take hours.  Typical use: --bench-seconds 5 --bench-out result.json.
            // The first ~2 s of the window naturally double as warmup
            // (frame-time rolling avg + per-frame EMA need to settle before
            // the snapshot reflects steady state).
            m_benchSeconds = _wtof(argv[i+1]);
            if (m_benchSeconds < 0.0) m_benchSeconds = 0.0;
            i += 1;
        }
        else if (_wcsicmp(argv[i], L"--bench-out") == 0 && i + 1 < argc)
        {
            // Output path for the benchmark JSON snapshot.  Required pairing
            // with --bench-seconds; without it the snapshot is never written.
            m_benchOutPath = argv[i+1];
            i += 1;
        }
        else if (_wcsicmp(argv[i], L"--camera-paused") == 0)
        {
            // Lock the camera at the current m_cameraSeconds value (=0 by
            // default; combine with --camera-seconds N to pick a specific
            // orbit angle).  Equivalent to hitting [Space] interactively.
            // Bench-mode-friendly: per-frame cost stops depending on which
            // glass surfaces are currently overlapping in screen space, so
            // FPS measurements stabilise.  Animation still runs unless
            // --anim-paused is also passed.
            m_cameraPaused = true;
        }
        else if (_wcsicmp(argv[i], L"--anim-paused") == 0)
        {
            // Freeze the animated-ball wobble (m_animSeconds stops advancing).
            // Equivalent to hitting [M] interactively.  Per-frame INSTANTIATE /
            // BLAS-from-CLAS still happen but with frozen vertex positions.
            m_animPaused = true;
        }
        else if (_wcsicmp(argv[i], L"--camera-seconds") == 0 && i + 1 < argc)
        {
            // Set the initial camera orbit time (in seconds along the 30 s
            // yaw / 19 s dolly Lissajous).  Combine with --camera-paused
            // to lock at a specific viewpoint.  Same time value the
            // [Space] freezer sees; useful for picking a viewpoint that
            // shows lots of scene at large clone counts (try ~7.5 for
            // quarter-orbit side view, ~15 for halfway-around).
            m_cameraSeconds = _wtof(argv[i+1]);
            i += 1;
        }
        else if (_wcsicmp(argv[i], L"--at") == 0 && i + 1 < argc)
        {
            // Schedule an action to fire at a specific frame.  Format:
            //   --at <frame>:<action>
            // where action is one of:
            //   alloc-implicit / alloc-getsizes / alloc-compact   (=> [A] press)
            //   rebuild-none / rebuild-blas / rebuild-clas-blas  (=> [R] press)
            //   log    -- snapshot all 5 per-frame timing values to SampleLog
            //   exit   -- post WM_QUIT
            // Multiple --at args allowed; executed in order at OnRender time.
            // Frame indices are 0-based and reference m_framesRendered AFTER init.
            std::wstring spec = argv[i+1];
            auto colon = spec.find(L':');
            if (colon != std::wstring::npos)
            {
                ScheduledAction a;
                a.frame  = (UINT)_wtoi(spec.substr(0, colon).c_str());
                a.action = spec.substr(colon + 1);
                m_scheduledActions.push_back(std::move(a));
            }
            i += 1;
        }
        else if (_wcsicmp(argv[i], L"--compressed-bits") == 0 && i + 1 < argc)
        {
            // COMPRESSED1 mode only - bits per component for the shared-exponent
            // quantizer. Range 1 (extreme - 1 bit each axis) to 16 (max).
            // Same value the [/] runtime slider drives.  Ignored under
            // --vertex-format float.
            int n = _wtoi(argv[i+1]);
            if (n < 1)  n = 1;
            if (n > 16) n = 16;
            m_compressedBitsPerComponent = (UINT)n;
            i += 1;
        }
        else if (_wcsicmp(argv[i], L"--aa-samples") == 0 && i + 1 < argc)
        {
            // Anti-aliasing samples per pixel.  The raygen shader traces N
            // jittered primary rays per pixel and averages.  N must be 1,
            // 2, or 4 (clamped to nearest valid).  Default is 4.
            int n = _wtoi(argv[i+1]);
            if (n <= 1) n = 1;
            else if (n <= 2) n = 2;
            else n = 4;
            m_aaSamplesPerPixel = (UINT)n;
            i += 1;
        }
        else if (_wcsicmp(argv[i], L"--cluster-tint") == 0 && i + 1 < argc)
        {
            // Cluster-rainbow tint blend in [0..1].  0 = pure material
            // colour; 1 = original "cluster rainbow dominates everything"
            // look.  Default 0.3 leaves a visible hint of cluster
            // boundaries without overpowering the material palette.
            float t = (float)_wtof(argv[i+1]);
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            m_clusterTint = t;
            i += 1;
        }
    }
}


// ---- OnKeyDown -------------------------------------------
void D3D12RaytracingClusteredGeometry::OnKeyDown(UINT8 key)
{
    // ----- Space / M = camera pan + animation toggle.  Decoupled in the
    //   pause-fix refactor: pre-decoupling Space toggled m_animSeconds which
    //   drove BOTH camera orbit and per-vertex animation, so freezing one
    //   froze the other.  Now Space toggles ONLY camera (m_cameraSeconds),
    //   M toggles ONLY animation (m_animSeconds).  Useful for the bench
    //   script: --camera-paused locks viewpoint -> stable per-frame cost,
    //   while animation keeps running so per-frame anim AS rebuilds are
    //   still measured.  Interactive use: hit Space to study a frame
    //   without losing animation, hit M to freeze the wobble without
    //   stopping the orbit.
    if (key == VK_SPACE)
    {
        m_cameraPaused = !m_cameraPaused;
        SampleLog::LogF(L"[input] camera pan %s\n", m_cameraPaused ? L"PAUSED" : L"resumed");
    }
    else if (key == 'M' || key == 'm')
    {
        m_animPaused = !m_animPaused;
        SampleLog::LogF(L"[input] animation %s\n", m_animPaused ? L"PAUSED" : L"resumed");
    }
    else if (key == 'A' || key == 'a')
    {
        // [A] cycles the alloc strategy for whichever path is active:
        //   Clustered BLAS: CLAS alloc mode (Implicit -> GetSizes -> Compact -> ...)
        //   Traditional BLAS: BLAS alloc mode (Compact <-> Implicit)
        // Either triggers a full GPU flush + rebuild of the static AS pipeline.
        if (m_geometryMode == GeometryMode::Clusters)
        {
            switch (m_clasAllocMode)
            {
            case ClasAllocMode::Implicit: m_clasAllocMode = ClasAllocMode::GetSizes; break;
            case ClasAllocMode::GetSizes: m_clasAllocMode = ClasAllocMode::Compact;  break;
            case ClasAllocMode::Compact:  m_clasAllocMode = ClasAllocMode::Implicit; break;
            }
            SampleLog::LogF(L"[input] CLAS alloc mode -> %s\n", ClasAllocModeName());
        }
        else
        {
            m_traditionalAllocMode = (m_traditionalAllocMode == TraditionalAllocMode::Compact)
                                     ? TraditionalAllocMode::Implicit
                                     : TraditionalAllocMode::Compact;
            SampleLog::LogF(L"[input] traditional BLAS alloc mode -> %s\n", TraditionalAllocModeName());
        }
        RebuildStaticAccelerationStructures(L"alloc-mode toggle");
    }
    else if (key == 'V' || key == 'v')
    {
        // Cycle vertex format: FLOAT32_3 <-> COMPRESSED1.  Both paths are
        // validated on the current experimental WARP.
        m_vertexMode = (m_vertexMode == VertexMode::Float32_3)
                     ? VertexMode::Compressed1
                     : VertexMode::Float32_3;
        SampleLog::LogF(L"[input] vertex format -> %s\n",
                        m_vertexMode == VertexMode::Compressed1 ? L"COMPRESSED1" : L"FLOAT32_3");
        RebuildStaticAccelerationStructures(L"vertex-format toggle");
    }
    else if (key == 'R' || key == 'r')
    {
        // Cycle [R] static-rebuild mode: None -> BlasOnly -> ClasAndBlas -> ...
        // Per-frame work only; doesn't touch buffers (no GPU flush, no
        // RebuildStaticAccelerationStructures).  The next OnRender picks
        // up the new mode via m_staticRebuildMode and emits the
        // corresponding RTAS ops.  Snap overlay stats so the new mode's
        // per-frame timing values get armed for capture.
        // [R] is cluster-mode-only -- no-op in traditional mode.
        if (m_geometryMode != GeometryMode::Clusters) return;
        switch (m_staticRebuildMode)
        {
        case StaticRebuildMode::None:        m_staticRebuildMode = StaticRebuildMode::BlasOnly;    break;
        case StaticRebuildMode::BlasOnly:    m_staticRebuildMode = StaticRebuildMode::ClasAndBlas; break;
        case StaticRebuildMode::ClasAndBlas: m_staticRebuildMode = StaticRebuildMode::None;        break;
        }
        SampleLog::LogF(L"[input] static rebuild mode -> %s\n", StaticRebuildModeName());
        CaptureOverlayStatsSnapshot();
    }
    else if (key == 'T' || key == 't')
    {
        // Cycle [T] geometry mode: Clusters <-> Traditional.  Triggers a
        // full static-AS rebuild (heavy, like [A]).  Locked off if the
        // adapter doesn't support clusters (we're already pinned to
        // Traditional and stay there).
        if (!m_clustersAndPtlasSupported) return;
        m_geometryMode = (m_geometryMode == GeometryMode::Clusters)
                         ? GeometryMode::Traditional
                         : GeometryMode::Clusters;
        SampleLog::LogF(L"[input] geometry mode -> %s\n", GeometryModeName());
        RebuildStaticAccelerationStructures(L"geometry-mode toggle");
    }
    else if (key == 'F' || key == 'f')
    {
        // [F] toggles animated-BLAS update strategy in traditional mode.
        // No effect in cluster mode (the cluster INSTANTIATE+BLAS-from-CLAS
        // pipeline doesn't have a refit/rebuild dichotomy).  No GPU
        // rebuild needed; the next per-frame animated update picks up the
        // new flag.
        if (m_geometryMode == GeometryMode::Clusters) return;
        m_traditionalAnimMode = (m_traditionalAnimMode == TraditionalAnimMode::Rebuild)
                                ? TraditionalAnimMode::Refit
                                : TraditionalAnimMode::Rebuild;
        SampleLog::LogF(L"[input] traditional animated mode -> %s\n", TraditionalAnimModeName());
        CaptureOverlayStatsSnapshot();
    }
    else if (key == 'N' || key == 'n')
    {
        // [N] cycles workload scaling: extra cloned TLAS instances
        // spiraling out from the floor.  Each clone gets its OWN full
        // CLAS + BLAS set (no GVA sharing); animated clones add their
        // own per-frame INSTANTIATE_CLUSTER_TEMPLATES + BUILD_BLAS_FROM_CLAS
        // work.  Triggers a full RebuildStaticAccelerationStructures
        // which clones+builds all extras and rebuilds the TLAS.  Toggle
        // delay scales with count -- ~1s at 10K on a 4090.
        switch (m_extraInstancesMode)
        {
        case ExtraInstancesMode::None:        m_extraInstancesMode = ExtraInstancesMode::Hundred;     break;
        case ExtraInstancesMode::Hundred:     m_extraInstancesMode = ExtraInstancesMode::Thousand;    break;
        case ExtraInstancesMode::Thousand:    m_extraInstancesMode = ExtraInstancesMode::TenThousand; break;
        case ExtraInstancesMode::TenThousand: m_extraInstancesMode = ExtraInstancesMode::None;        break;
        }
        SampleLog::LogF(L"[input] extra instances -> %s\n", ExtraInstancesModeName());
        RebuildStaticAccelerationStructures(L"extra-instances toggle");
    }
    else if (key == 'B' || key == 'b')
    {
        // [B] cycles BVH-build-flag preference: NONE -> FAST_BUILD ->
        // FAST_TRACE -> ...  Applies to BOTH trad (DXR1) and cluster
        // (DXR2) paths uniformly via BuildFlagModeRtas() /
        // BuildFlagModeDxr1() helpers.  Re-trigger rebuild ONLY when
        // static-rebuild mode is None -- per-frame rebuild modes
        // (BlasOnly / ClasAndBlas) will pick up the new flag next
        // frame on their own.
        switch (m_buildFlagMode)
        {
        case BuildFlagMode::None:      m_buildFlagMode = BuildFlagMode::FastBuild; break;
        case BuildFlagMode::FastBuild: m_buildFlagMode = BuildFlagMode::FastTrace; break;
        case BuildFlagMode::FastTrace: m_buildFlagMode = BuildFlagMode::None;      break;
        }
        SampleLog::LogF(L"[input] BVH build flag -> %s\n", BuildFlagModeName());
        if (m_staticRebuildMode == StaticRebuildMode::None)
            RebuildStaticAccelerationStructures(L"build-flag toggle");
        else
            CaptureOverlayStatsSnapshot();
    }
    // ----- ',' / '.' = bounce-depth slider.  See m_bounceSlider in the
    //   header for the canonical mapping table.  Slider direction has a
    //   single meaning at every position -- moving '.' bumps the higher
    //   value first (refraction) and once it saturates at 5, starts
    //   bumping the lower one (reflection).  Moving ',' is the mirror
    //   image.  Going up and back down ALWAYS lands at the same (refl,
    //   refr) pair the slider passed through on the way up -- so the
    //   default +2 gap is restored automatically.  No rebuild needed.
    else if (key == VK_OEM_COMMA)            // ','
    {
        if (m_bounceSlider <= kBounceSliderMin) return;
        --m_bounceSlider;
        SampleLog::LogF(L"[input] bounce slider %d -> refl %u  refr %u\n",
                        m_bounceSlider, ReflectionBounces(), RefractionBounces());
    }
    else if (key == VK_OEM_PERIOD)           // '.'
    {
        if (m_bounceSlider >= kBounceSliderMax) return;
        ++m_bounceSlider;
        SampleLog::LogF(L"[input] bounce slider %d -> refl %u  refr %u\n",
                        m_bounceSlider, ReflectionBounces(), RefractionBounces());
    }
    // ----- '[' / ']' = per-cluster precision slider.  Direction is the same
    //   in both modes: '[' -> LESS precision, ']' -> MORE precision.
    //   Internally:
    //     FLOAT32_3   -> m_positionTruncateBits in [0, 23].  '[' increments
    //                    (truncates more bits); ']' decrements (keeps more).
    //                    23 is float32's mantissa width -- truncating more
    //                    than that just zeros the whole mantissa.
    //     COMPRESSED1 -> m_compressedBitsPerComponent in [1, 16].  '['
    //                    decrements; ']' increments.  16 is the per-axis
    //                    cap in the D3D12 COMPRESSED1 encoding; 1 is the
    //                    minimum (0 would divide-by-zero in our encoder).
    //   Either change triggers a full clustered-AS rebuild. Static CLAS are
    //   re-encoded in COMPRESSED1 mode, and the animated templates are rebuilt
    //   with a matching stored format and precision header.
    else if (key == VK_OEM_4 || key == VK_OEM_6)
    {
        const bool wantLess = (key == VK_OEM_4);
        if (m_vertexMode == VertexMode::Float32_3)
        {
            const UINT prev = m_positionTruncateBits;
            if (wantLess)  m_positionTruncateBits = (prev >= 23u) ? prev : prev + 1u;
            else           m_positionTruncateBits = (prev == 0u)  ? 0u  : prev - 1u;
            if (m_positionTruncateBits == prev) return;
            SampleLog::LogF(L"[input] position truncate bits -> %u  (%u bits kept)\n",
                            m_positionTruncateBits, 32u - m_positionTruncateBits);
        }
        else
        {
            const UINT prev = m_compressedBitsPerComponent;
            if (wantLess)  m_compressedBitsPerComponent = (prev <= 1u)  ? 1u  : prev - 1u;
            else           m_compressedBitsPerComponent = (prev >= 16u) ? prev : prev + 1u;
            if (m_compressedBitsPerComponent == prev) return;
            SampleLog::LogF(L"[input] compressed1 bits/component -> %u\n", m_compressedBitsPerComponent);
        }
        RebuildStaticAccelerationStructures(L"precision slider");
    }
}
