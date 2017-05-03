//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#include "pch.h"
#include "SSAO.h"
#include "BufferManager.h"
#include "GraphicsCore.h"
#include "CommandContext.h"
#include "Camera.h"
#include "TemporalEffects.h"

#include "CompiledShaders/AoPrepareDepthBuffers1CS.h"
#include "CompiledShaders/AoPrepareDepthBuffers2CS.h"
#include "CompiledShaders/LinearizeDepthCS.h"
#include "CompiledShaders/DebugSSAOCS.h"
#include "CompiledShaders/AoRender1CS.h"
#include "CompiledShaders/AoRender2CS.h"
#include "CompiledShaders/AoBlurUpsampleBlendOutCS.h"
#include "CompiledShaders/AoBlurUpsamplePreMinBlendOutCS.h"
#include "CompiledShaders/AoBlurUpsampleCS.h"
#include "CompiledShaders/AoBlurUpsamplePreMinCS.h"

using namespace Graphics;
using namespace Math;

namespace SSAO
{
    BoolVar Enable("Graphics/SSAO/Enable", true);
    BoolVar DebugDraw("Graphics/SSAO/Debug Draw", false);
    BoolVar AsyncCompute("Graphics/SSAO/Async Compute", false);
    BoolVar ComputeLinearZ("Graphics/SSAO/Always Linearize Z", true);

    // High quality (and better) is barely a noticeable improvement when modulated properly with ambient light.
    // However, in the debug view the quality improvement is very apparent.
    enum QualityLevel { kSsaoQualityVeryLow, kSsaoQualityLow, kSsaoQualityMedium, kSsaoQualityHigh, kSsaoQualityVeryHigh, kNumSsaoQualitySettings };
    const char* QualityLabels[kNumSsaoQualitySettings] = { "Very Low", "Low", "Medium", "High", "Very High" };
    EnumVar g_QualityLevel("Graphics/SSAO/Quality Level", kSsaoQualityHigh, kNumSsaoQualitySettings, QualityLabels);

    // This is necessary to filter out pixel shimmer due to bilateral upsampling with too much lost resolution.  High
    // frequency detail can sometimes not be reconstructed, and the noise filter fills in the missing pixels with the
    // result of the higher resolution SSAO.
    NumVar g_NoiseFilterTolerance("Graphics/SSAO/Noise Filter Threshold (log10)", -3.0f, -8.0f, 0.0f, 0.25f);
    NumVar g_BlurTolerance("Graphics/SSAO/Blur Tolerance (log10)", -5.0f, -8.0f, -1.0f, 0.25f);
    NumVar g_UpsampleTolerance("Graphics/SSAO/Upsample Tolerance (log10)", -7.0f, -12.0f, -1.0f, 0.5f);

    // Controls how aggressive to fade off samples that occlude spheres but by so much as to be unreliable.
    // This is what gives objects a dark halo around them when placed in front of a wall.  If you want to
    // fade off the halo, boost your rejection falloff.  The tradeoff is that it reduces overall AO.
    NumVar RejectionFalloff("Graphics/SSAO/Rejection Falloff (rcp)", 2.5f, 1.0f, 10.0f, 0.5f);

    // The effect normally marks anything that's 50% occluded or less as "fully unoccluded".  This throws away
    // half of our result.  Accentuation gives more range to the effect, but it will darken all AO values in the
    // process.  It will also cause "under occluded" geometry to appear to be highlighted.  If your ambient light
    // is determined by the surface normal (such as with IBL), you might not want this side effect.
    NumVar Accentuation("Graphics/SSAO/Accentuation", 0.1f, 0.0f, 1.0f, 0.1f);

    IntVar HierarchyDepth("Graphics/SSAO/Hierarchy Depth", 3, 1, 4);
}

namespace
{
    RootSignature s_RootSignature;
    ComputePSO s_DepthPrepare1CS;
    ComputePSO s_DepthPrepare2CS;
    ComputePSO s_Render1CS;
    ComputePSO s_Render2CS;
    ComputePSO s_BlurUpsampleBlend[2];	// Blend the upsampled result with the next higher resolution
    ComputePSO s_BlurUpsampleFinal[2];	// Don't blend the result, just upsample it
    ComputePSO s_LinearizeDepthCS;
    ComputePSO s_DebugSSAOCS;

    float SampleThickness[12];	// Pre-computed sample thicknesses
}

void SSAO::Initialize( void )
{
    s_RootSignature.Reset(5, 2);
    s_RootSignature.InitStaticSampler(0, SamplerLinearClampDesc);
    s_RootSignature.InitStaticSampler(1, SamplerLinearBorderDesc);
    s_RootSignature[0].InitAsConstants(0, 4);
    s_RootSignature[1].InitAsConstantBuffer(1);
    s_RootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 5);
    s_RootSignature[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 5);
    s_RootSignature[4].InitAsBufferSRV(5);
    s_RootSignature.Finalize(L"SSAO");

#define CreatePSO( ObjName, ShaderByteCode ) \
    ObjName.SetRootSignature(s_RootSignature); \
    ObjName.SetComputeShader(ShaderByteCode, sizeof(ShaderByteCode) ); \
    ObjName.Finalize();

    CreatePSO( s_DepthPrepare1CS, g_pAoPrepareDepthBuffers1CS );
    CreatePSO( s_DepthPrepare2CS, g_pAoPrepareDepthBuffers2CS );
    CreatePSO( s_LinearizeDepthCS,  g_pLinearizeDepthCS );
    CreatePSO( s_DebugSSAOCS, g_pDebugSSAOCS );
    CreatePSO( s_Render1CS, g_pAoRender1CS );
    CreatePSO( s_Render2CS, g_pAoRender2CS );

    CreatePSO( s_BlurUpsampleBlend[0], g_pAoBlurUpsampleBlendOutCS );
    CreatePSO( s_BlurUpsampleBlend[1], g_pAoBlurUpsamplePreMinBlendOutCS );
    CreatePSO( s_BlurUpsampleFinal[0], g_pAoBlurUpsampleCS );
    CreatePSO( s_BlurUpsampleFinal[1], g_pAoBlurUpsamplePreMinCS );

    SampleThickness[ 0] = sqrt(1.0f - 0.2f * 0.2f);
    SampleThickness[ 1] = sqrt(1.0f - 0.4f * 0.4f);
    SampleThickness[ 2] = sqrt(1.0f - 0.6f * 0.6f);
    SampleThickness[ 3] = sqrt(1.0f - 0.8f * 0.8f);
    SampleThickness[ 4] = sqrt(1.0f - 0.2f * 0.2f - 0.2f * 0.2f);
    SampleThickness[ 5] = sqrt(1.0f - 0.2f * 0.2f - 0.4f * 0.4f);
    SampleThickness[ 6] = sqrt(1.0f - 0.2f * 0.2f - 0.6f * 0.6f);
    SampleThickness[ 7] = sqrt(1.0f - 0.2f * 0.2f - 0.8f * 0.8f);
    SampleThickness[ 8] = sqrt(1.0f - 0.4f * 0.4f - 0.4f * 0.4f);
    SampleThickness[ 9] = sqrt(1.0f - 0.4f * 0.4f - 0.6f * 0.6f);
    SampleThickness[10] = sqrt(1.0f - 0.4f * 0.4f - 0.8f * 0.8f);
    SampleThickness[11] = sqrt(1.0f - 0.6f * 0.6f - 0.6f * 0.6f);
}

void SSAO::Shutdown(void)
{
}

namespace SSAO
{
    void ComputeAO( ComputeContext& Context, ColorBuffer& Destination, ColorBuffer& DepthBuffer, const float TanHalfFovH )
    {
        size_t BufferWidth = DepthBuffer.GetWidth();
        size_t BufferHeight = DepthBuffer.GetHeight();
        size_t ArrayCount = DepthBuffer.GetDepth();

        // Here we compute multipliers that convert the center depth value into (the reciprocal of)
        // sphere thicknesses at each sample location.  This assumes a maximum sample radius of 5
        // units, but since a sphere has no thickness at its extent, we don't need to sample that far
        // out.  Only samples whole integer offsets with distance less than 25 are used.  This means
        // that there is no sample at (3, 4) because its distance is exactly 25 (and has a thickness of 0.)

        // The shaders are set up to sample a circular region within a 5-pixel radius.
        const float ScreenspaceDiameter = 10.0f;

        // SphereDiameter = CenterDepth * ThicknessMultiplier.  This will compute the thickness of a sphere centered
        // at a specific depth.  The ellipsoid scale can stretch a sphere into an ellipsoid, which changes the
        // characteristics of the AO.
        // TanHalfFovH:  Radius of sphere in depth units if its center lies at Z = 1
        // ScreenspaceDiameter:  Diameter of sample sphere in pixel units
        // ScreenspaceDiameter / BufferWidth:  Ratio of the screen width that the sphere actually covers
        // Note about the "2.0f * ":  Diameter = 2 * Radius
        float ThicknessMultiplier = 2.0f * TanHalfFovH * ScreenspaceDiameter / BufferWidth;

        if (ArrayCount == 1)
            ThicknessMultiplier *= 2.0f;

        // This will transform a depth value from [0, thickness] to [0, 1].
        float InverseRangeFactor = 1.0f / ThicknessMultiplier;

        __declspec(align(16)) float SsaoCB[28];

        // The thicknesses are smaller for all off-center samples of the sphere.  Compute thicknesses relative
        // to the center sample.
        SsaoCB[ 0] = InverseRangeFactor / SampleThickness[ 0];
        SsaoCB[ 1] = InverseRangeFactor / SampleThickness[ 1];
        SsaoCB[ 2] = InverseRangeFactor / SampleThickness[ 2];
        SsaoCB[ 3] = InverseRangeFactor / SampleThickness[ 3];
        SsaoCB[ 4] = InverseRangeFactor / SampleThickness[ 4];
        SsaoCB[ 5] = InverseRangeFactor / SampleThickness[ 5];
        SsaoCB[ 6] = InverseRangeFactor / SampleThickness[ 6];
        SsaoCB[ 7] = InverseRangeFactor / SampleThickness[ 7];
        SsaoCB[ 8] = InverseRangeFactor / SampleThickness[ 8];
        SsaoCB[ 9] = InverseRangeFactor / SampleThickness[ 9];
        SsaoCB[10] = InverseRangeFactor / SampleThickness[10];
        SsaoCB[11] = InverseRangeFactor / SampleThickness[11];

        // These are the weights that are multiplied against the samples because not all samples are
        // equally important.  The farther the sample is from the center location, the less they matter.
        // We use the thickness of the sphere to determine the weight.  The scalars in front are the number
        // of samples with this weight because we sum the samples together before multiplying by the weight,
        // so as an aggregate all of those samples matter more.  After generating this table, the weights
        // are normalized.
        SsaoCB[12] = 4.0f * SampleThickness[ 0];	// Axial
        SsaoCB[13] = 4.0f * SampleThickness[ 1];	// Axial
        SsaoCB[14] = 4.0f * SampleThickness[ 2];	// Axial
        SsaoCB[15] = 4.0f * SampleThickness[ 3];	// Axial
        SsaoCB[16] = 4.0f * SampleThickness[ 4];	// Diagonal
        SsaoCB[17] = 8.0f * SampleThickness[ 5];	// L-shaped
        SsaoCB[18] = 8.0f * SampleThickness[ 6];	// L-shaped
        SsaoCB[19] = 8.0f * SampleThickness[ 7];	// L-shaped
        SsaoCB[20] = 4.0f * SampleThickness[ 8];	// Diagonal
        SsaoCB[21] = 8.0f * SampleThickness[ 9];	// L-shaped
        SsaoCB[22] = 8.0f * SampleThickness[10];	// L-shaped
        SsaoCB[23] = 4.0f * SampleThickness[11];	// Diagonal

//#define SAMPLE_EXHAUSTIVELY

        // If we aren't using all of the samples, delete their weights before we normalize.
    #ifndef SAMPLE_EXHAUSTIVELY
        SsaoCB[12] = 0.0f;
        SsaoCB[14] = 0.0f;
        SsaoCB[17] = 0.0f;
        SsaoCB[19] = 0.0f;
        SsaoCB[21] = 0.0f;
    #endif

        // Normalize the weights by dividing by the sum of all weights
        float totalWeight = 0.0f;
        for (int i = 12; i < 24; ++i)
            totalWeight += SsaoCB[i];
        for (int i = 12; i < 24; ++i)
            SsaoCB[i] /= totalWeight;

        SsaoCB[24] = 1.0f / BufferWidth;
        SsaoCB[25] = 1.0f / BufferHeight;
        SsaoCB[26] = 1.0f / -RejectionFalloff;
        SsaoCB[27] = 1.0f / (1.0f + Accentuation);

        Context.SetDynamicConstantBufferView(1, sizeof(SsaoCB), SsaoCB);
        Context.SetDynamicDescriptor(2, 0, Destination.GetUAV());
        Context.SetDynamicDescriptor(3, 0, DepthBuffer.GetSRV() );

        if (ArrayCount == 1)
            Context.Dispatch2D(BufferWidth, BufferHeight, 16, 16);
        else
            Context.Dispatch3D(BufferWidth, BufferHeight, ArrayCount, 8, 8, 1);
    }
    
    void BlurAndUpsample( ComputeContext& Context,
        ColorBuffer& Destination, ColorBuffer& HiResDepth, ColorBuffer& LoResDepth,
        ColorBuffer* InterleavedAO, ColorBuffer* HighQualityAO, ColorBuffer* HiResAO
    )
    {
        size_t LoWidth  = LoResDepth.GetWidth();
        size_t LoHeight = LoResDepth.GetHeight();
        size_t HiWidth  = HiResDepth.GetWidth();
        size_t HiHeight = HiResDepth.GetHeight();

        ComputePSO* shader = nullptr;
        if (HiResAO == nullptr)
        {
            shader = &s_BlurUpsampleFinal[HighQualityAO == nullptr ? 0 : 1];
        }
        else
        {
            shader = &s_BlurUpsampleBlend[HighQualityAO == nullptr ? 0 : 1];
        }
        Context.SetPipelineState(*shader);

        float kBlurTolerance = 1.0f - powf(10.0f, g_BlurTolerance) * 1920.0f / (float)LoWidth;
        kBlurTolerance *= kBlurTolerance;
        float kUpsampleTolerance = powf(10.0f, g_UpsampleTolerance);
        float kNoiseFilterWeight = 1.0f / (powf(10.0f, g_NoiseFilterTolerance) + kUpsampleTolerance);

        __declspec(align(16)) float cbData[] = {
            1.0f / LoWidth, 1.0f / LoHeight, 1.0f / HiWidth, 1.0f / HiHeight, 
            kNoiseFilterWeight, 1920.0f / (float)LoWidth, kBlurTolerance, kUpsampleTolerance
        };
        Context.SetDynamicConstantBufferView(1, sizeof(cbData), cbData);

        Context.TransitionResource(Destination, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(LoResDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(HiResDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.SetDynamicDescriptor(2, 0, Destination.GetUAV());
        Context.SetDynamicDescriptor(3, 0, LoResDepth.GetSRV());
        Context.SetDynamicDescriptor(3, 1, HiResDepth.GetSRV());
        if (InterleavedAO != nullptr)
        {
            Context.TransitionResource(*InterleavedAO, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            Context.SetDynamicDescriptor(3, 2, InterleavedAO->GetSRV());
        }
        if (HighQualityAO != nullptr)
        {
            Context.TransitionResource(*HighQualityAO, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            Context.SetDynamicDescriptor(3, 3, HighQualityAO->GetSRV());
        }
        if (HiResAO != nullptr)
        {
            Context.TransitionResource(*HiResAO, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            Context.SetDynamicDescriptor(3, 4, HiResAO->GetSRV());
        }

        Context.Dispatch2D(HiWidth+2, HiHeight+2, 16, 16);
    }
}

void SSAO::Render( GraphicsContext& GfxContext, const Camera& camera )
{
    const float* pProjMat = reinterpret_cast<const float*>(&camera.GetProjMatrix());
    Render(GfxContext, pProjMat, camera.GetNearClip(), camera.GetFarClip() );
}

void SSAO::Render( GraphicsContext& GfxContext, const float* ProjMat, float NearClipDist, float FarClipDist )
{
    uint32_t FrameIndex = TemporalEffects::GetFrameIndexMod2();

    ColorBuffer& LinearDepth = g_LinearDepth[FrameIndex];

    const float zMagic = (FarClipDist - NearClipDist) / NearClipDist;

    if (!Enable)
    {
        ScopedTimer _prof(L"Generate SSAO", GfxContext);

        GfxContext.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
        GfxContext.ClearColor(g_SSAOFullScreen);
        GfxContext.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        if (!ComputeLinearZ)
            return;

        ComputeContext& Context = GfxContext.GetComputeContext();
        Context.SetRootSignature(s_RootSignature);

        Context.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.SetConstants(0, zMagic);
        Context.SetDynamicDescriptor(3, 0, g_SceneDepthBuffer.GetDepthSRV());

        Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.SetDynamicDescriptors(2, 0, 1, &LinearDepth.GetUAV());
        Context.SetPipelineState(s_LinearizeDepthCS);
        Context.Dispatch2D(LinearDepth.GetWidth(), LinearDepth.GetHeight(), 16, 16);

        if (DebugDraw)
        {
            Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            Context.SetDynamicDescriptors(2, 0, 1, &g_SceneColorBuffer.GetUAV());
            Context.SetDynamicDescriptors(3, 0, 1, &LinearDepth.GetSRV() );
            Context.SetPipelineState(s_DebugSSAOCS);
            Context.Dispatch2D(g_SSAOFullScreen.GetWidth(), g_SSAOFullScreen.GetHeight());
        }

        return;
    }

    GfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    GfxContext.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    if (AsyncCompute)
    {
        // Flush the ZPrePass and wait for it on the compute queue
        g_CommandManager.GetComputeQueue().StallForFence(GfxContext.Flush());
    }
    else
    {
        EngineProfiling::BeginBlock(L"Generate SSAO", &GfxContext);
    }

    ComputeContext& Context = AsyncCompute ? ComputeContext::Begin(L"Async SSAO", true) : GfxContext.GetComputeContext();
    Context.SetRootSignature(s_RootSignature);

    { ScopedTimer _prof(L"Decompress and downsample", Context);

    // Phase 1:  Decompress, linearize, downsample, and deinterleave the depth buffer
    Context.SetConstants(0, zMagic);
    Context.SetDynamicDescriptor(3, 0, g_SceneDepthBuffer.GetDepthSRV() );

    Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_DepthDownsize1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_DepthTiled1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_DepthDownsize2, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_DepthTiled2, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    D3D12_CPU_DESCRIPTOR_HANDLE DownsizeUAVs[5] = { LinearDepth.GetUAV(), g_DepthDownsize1.GetUAV(), g_DepthTiled1.GetUAV(),
        g_DepthDownsize2.GetUAV(), g_DepthTiled2.GetUAV() };
    Context.SetDynamicDescriptors(2, 0, 5, DownsizeUAVs);

    Context.SetPipelineState(s_DepthPrepare1CS);
    Context.Dispatch2D(g_DepthTiled2.GetWidth() * 8, g_DepthTiled2.GetHeight() * 8);

    if (HierarchyDepth > 2)
    {
        Context.SetConstants(0, 1.0f / g_DepthDownsize2.GetWidth(), 1.0f / g_DepthDownsize2.GetHeight());
        Context.TransitionResource(g_DepthDownsize2, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_DepthDownsize3, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(g_DepthTiled3, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(g_DepthDownsize4, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(g_DepthTiled4, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        D3D12_CPU_DESCRIPTOR_HANDLE DownsizeAgainUAVs[4] = { g_DepthDownsize3.GetUAV(), g_DepthTiled3.GetUAV(), g_DepthDownsize4.GetUAV(), g_DepthTiled4.GetUAV() };
        Context.SetDynamicDescriptors(2, 0, 4, DownsizeAgainUAVs);
        Context.SetDynamicDescriptors(3, 0, 1, &g_DepthDownsize2.GetSRV() );
        Context.SetPipelineState(s_DepthPrepare2CS);
        Context.Dispatch2D(g_DepthTiled4.GetWidth() * 8, g_DepthTiled4.GetHeight() * 8);
    }

    } // End decompress
    { ScopedTimer _prof(L"Analyze depth volumes", Context);

    // Load first element of projection matrix which is the cotangent of the horizontal FOV divided by 2.
    const float FovTangent = 1.0f / ProjMat[0];

    Context.TransitionResource(g_AOMerged1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_AOMerged2, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_AOMerged3, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_AOMerged4, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_AOHighQuality1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_AOHighQuality2, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_AOHighQuality3, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_AOHighQuality4, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_DepthTiled1, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_DepthTiled2, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_DepthTiled3, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_DepthTiled4, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_DepthDownsize1, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_DepthDownsize2, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_DepthDownsize3, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_DepthDownsize4, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    // Phase 2:  Render SSAO for each sub-tile
    if (HierarchyDepth > 3)
    {
        Context.SetPipelineState( s_Render1CS );
        ComputeAO( Context, g_AOMerged4, g_DepthTiled4, FovTangent );
        if (g_QualityLevel >= kSsaoQualityLow)
        {
            Context.SetPipelineState( s_Render2CS );
            ComputeAO( Context, g_AOHighQuality4, g_DepthDownsize4, FovTangent );
        }
    }
    if (HierarchyDepth > 2)
    {
        Context.SetPipelineState( s_Render1CS );
        ComputeAO( Context, g_AOMerged3, g_DepthTiled3, FovTangent );
        if (g_QualityLevel >= kSsaoQualityMedium)  
        {
            Context.SetPipelineState( s_Render2CS );
            ComputeAO( Context, g_AOHighQuality3, g_DepthDownsize3, FovTangent );
        }
    }
    if (HierarchyDepth > 1)
    {
        Context.SetPipelineState( s_Render1CS );
        ComputeAO( Context, g_AOMerged2, g_DepthTiled2, FovTangent );
        if (g_QualityLevel >= kSsaoQualityHigh)	   
        {
            Context.SetPipelineState( s_Render2CS );
            ComputeAO( Context, g_AOHighQuality2, g_DepthDownsize2, FovTangent );
        }
    }
    {
        Context.SetPipelineState( s_Render1CS );
        ComputeAO( Context, g_AOMerged1, g_DepthTiled1, FovTangent );
        if (g_QualityLevel >= kSsaoQualityVeryHigh)
        {
            Context.SetPipelineState( s_Render2CS );
            ComputeAO( Context, g_AOHighQuality1, g_DepthDownsize1, FovTangent );
        }
    }

    } // End analyze
    {  ScopedTimer _prof(L"Blur and upsample", Context);

    // Phase 4:  Iteratively blur and upsample, combining each result

    ColorBuffer* NextSRV = &g_AOMerged4;


    // 120 x 68 -> 240 x 135
    if (HierarchyDepth > 3)
    {
        BlurAndUpsample( Context, g_AOSmooth3, g_DepthDownsize3, g_DepthDownsize4, NextSRV,
            g_QualityLevel >= kSsaoQualityLow ? &g_AOHighQuality4 : nullptr, &g_AOMerged3 );

        NextSRV = &g_AOSmooth3;
    }
    else
        NextSRV = &g_AOMerged3;


    // 240 x 135 -> 480 x 270
    if (HierarchyDepth > 2)
    {
        BlurAndUpsample( Context, g_AOSmooth2, g_DepthDownsize2, g_DepthDownsize3, NextSRV,
            g_QualityLevel >= kSsaoQualityMedium ? &g_AOHighQuality3 : nullptr, &g_AOMerged2 );

        NextSRV = &g_AOSmooth2;
    }
    else
        NextSRV = &g_AOMerged2;

    // 480 x 270 -> 960 x 540
    if (HierarchyDepth > 1)
    {
        BlurAndUpsample( Context, g_AOSmooth1, g_DepthDownsize1, g_DepthDownsize2, NextSRV,
            g_QualityLevel >= kSsaoQualityHigh ? &g_AOHighQuality2 : nullptr, &g_AOMerged1 );

        NextSRV = &g_AOSmooth1;
    }
    else
        NextSRV = &g_AOMerged1;

    // 960 x 540 -> 1920 x 1080
    BlurAndUpsample( Context, g_SSAOFullScreen, LinearDepth, g_DepthDownsize1, NextSRV,
        g_QualityLevel >= kSsaoQualityVeryHigh ? &g_AOHighQuality1 : nullptr, nullptr );

    } // End blur and upsample

    if (AsyncCompute)
        Context.Finish();
    else
        EngineProfiling::EndBlock(&GfxContext);

    if (DebugDraw)
    {
        if (AsyncCompute)
        {
            g_CommandManager.GetGraphicsQueue().StallForProducer(
                g_CommandManager.GetComputeQueue());
        }

        ComputeContext& CC = GfxContext.GetComputeContext();
        CC.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        CC.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        CC.SetRootSignature(s_RootSignature);
        CC.SetPipelineState(s_DebugSSAOCS);
        CC.SetDynamicDescriptors(2, 0, 1, &g_SceneColorBuffer.GetUAV());
        CC.SetDynamicDescriptors(3, 0, 1, &g_SSAOFullScreen.GetSRV());
        CC.Dispatch2D(g_SSAOFullScreen.GetWidth(), g_SSAOFullScreen.GetHeight());
    }
}
