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
#include "TemporalEffects.h"
#include "BufferManager.h"
#include "GraphicsCore.h"
#include "CommandContext.h"
#include "SystemTime.h"
#include "PostEffects.h"

#include "CompiledShaders/TemporalBlendCS.h"
#include "CompiledShaders/BoundNeighborhoodCS.h"
#include "CompiledShaders/ResolveTAACS.h"
#include "CompiledShaders/SharpenTAACS.h"

using namespace Graphics;
using namespace Math;
using namespace TemporalEffects;

namespace TemporalEffects
{
    BoolVar EnableTAA("Graphics/AA/TAA/Enable", false);
    NumVar Sharpness("Graphics/AA/TAA/Sharpness", 0.5f, 0.0f, 1.0f, 0.25f);
    NumVar TemporalMaxLerp("Graphics/AA/TAA/Blend Factor", 1.0f, 0.0f, 1.0f, 0.01f);
    ExpVar TemporalSpeedLimit("Graphics/AA/TAA/Speed Limit", 64.0f, 1.0f, 1024.0f, 1.0f);
    BoolVar TriggerReset("Graphics/AA/TAA/Reset", false);

    RootSignature s_RootSignature;

    ComputePSO s_TemporalBlendCS;
    ComputePSO s_BoundNeighborhoodCS;
    ComputePSO s_SharpenTAACS;
    ComputePSO s_ResolveTAACS;

    uint32_t s_FrameIndex = 0;
    uint32_t s_FrameIndexMod2 = 0;
    float s_JitterX = 0.5f;
    float s_JitterY = 0.5f;
    float s_JitterDeltaX = 0.0f;
    float s_JitterDeltaY = 0.0f;

    void ApplyTemporalAA(ComputeContext& Context);
    void SharpenImage(ComputeContext& Context, ColorBuffer& TemporalColor);
}

void TemporalEffects::Initialize( void )
{
    s_RootSignature.Reset(4, 2);
    s_RootSignature[0].InitAsConstants(0, 4);
    s_RootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10);
    s_RootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 10);
    s_RootSignature[3].InitAsConstantBuffer(1);
    s_RootSignature.InitStaticSampler(0, SamplerLinearBorderDesc);
    s_RootSignature.InitStaticSampler(1, SamplerPointBorderDesc);
    s_RootSignature.Finalize(L"Temporal RS");

#define CreatePSO( ObjName, ShaderByteCode ) \
    ObjName.SetRootSignature(s_RootSignature); \
    ObjName.SetComputeShader(ShaderByteCode, sizeof(ShaderByteCode) ); \
    ObjName.Finalize();

    CreatePSO( s_TemporalBlendCS, g_pTemporalBlendCS );
    CreatePSO( s_BoundNeighborhoodCS, g_pBoundNeighborhoodCS );
    CreatePSO( s_SharpenTAACS, g_pSharpenTAACS );
    CreatePSO( s_ResolveTAACS, g_pResolveTAACS );

#undef CreatePSO
}

void TemporalEffects::Shutdown( void )
{
}

void TemporalEffects::Update( uint64_t FrameIndex )
{
    s_FrameIndex = (uint32_t)FrameIndex;
    s_FrameIndexMod2 = s_FrameIndex % 2;

    if (EnableTAA)// && !DepthOfField::Enable)
    {
        static const float Halton23[8][2] =
        {
            { 0.0f / 8.0f, 0.0f / 9.0f }, { 4.0f / 8.0f, 3.0f / 9.0f },
            { 2.0f / 8.0f, 6.0f / 9.0f }, { 6.0f / 8.0f, 1.0f / 9.0f },
            { 1.0f / 8.0f, 4.0f / 9.0f }, { 5.0f / 8.0f, 7.0f / 9.0f },
            { 3.0f / 8.0f, 2.0f / 9.0f }, { 7.0f / 8.0f, 5.0f / 9.0f }
        };

        const float* Offset = Halton23[s_FrameIndex % 8];

        s_JitterDeltaX = s_JitterX - Offset[0];
        s_JitterDeltaY = s_JitterY - Offset[1];
        s_JitterX = Offset[0];
        s_JitterY = Offset[1];
    }
    else
    {
        s_JitterDeltaX = s_JitterX - 0.5f;
        s_JitterDeltaY = s_JitterY - 0.5f;
        s_JitterX = 0.5f;
        s_JitterY = 0.5f;
    }

}

uint32_t TemporalEffects::GetFrameIndexMod2( void )
{
    return s_FrameIndexMod2;
}

void TemporalEffects::GetJitterOffset( float& JitterX, float& JitterY )
{
    JitterX = s_JitterX;
    JitterY = s_JitterY;
}

void TemporalEffects::ClearHistory( CommandContext& Context )
{
    GraphicsContext& gfxContext = Context.GetGraphicsContext();

    if (EnableTAA)
    {
        gfxContext.TransitionResource(g_TemporalColor[0], D3D12_RESOURCE_STATE_RENDER_TARGET);
        gfxContext.TransitionResource(g_TemporalColor[1], D3D12_RESOURCE_STATE_RENDER_TARGET, true);
        gfxContext.ClearColor(g_TemporalColor[0]);
        gfxContext.ClearColor(g_TemporalColor[1]);
    }
}

void TemporalEffects::ResolveImage( CommandContext& BaseContext )
{
    ScopedTimer _prof(L"Temporal Resolve", BaseContext);

    ComputeContext& Context = BaseContext.GetComputeContext();

    static bool s_EnableTAA = false;

    if (EnableTAA != s_EnableTAA || TriggerReset)
    {
        ClearHistory(Context);
        s_EnableTAA = EnableTAA;
        TriggerReset = false;
    }

    uint32_t Src = s_FrameIndexMod2;
    uint32_t Dst = Src ^ 1;

    if (EnableTAA)
    {
        ApplyTemporalAA(Context);
        SharpenImage(Context, g_TemporalColor[Dst]);
    }
}

void TemporalEffects::ApplyTemporalAA(ComputeContext& Context)
{
    ScopedTimer _prof(L"Resolve Image", Context);

    uint32_t Src = s_FrameIndexMod2;
    uint32_t Dst = Src ^ 1;

    Context.SetRootSignature(s_RootSignature);
    Context.SetPipelineState(s_TemporalBlendCS);

    __declspec(align(16)) struct ConstantBuffer
    {
        float RcpBufferDim[2];
        float TemporalBlendFactor;
        float RcpSeedLimiter;
        float CombinedJitter[2];
    };
    ConstantBuffer cbv = {
        1.0f / g_SceneColorBuffer.GetWidth(), 1.0f / g_SceneColorBuffer.GetHeight(),
        (float)TemporalMaxLerp, 1.0f / TemporalSpeedLimit,
        s_JitterDeltaX, s_JitterDeltaY
    };

    Context.SetDynamicConstantBufferView(3, sizeof(cbv), &cbv);

    Context.TransitionResource(g_VelocityBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_TemporalColor[Src], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_TemporalColor[Dst], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_LinearDepth[Src], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_LinearDepth[Dst], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.SetDynamicDescriptor(1, 0, g_VelocityBuffer.GetSRV());
    Context.SetDynamicDescriptor(1, 1, g_SceneColorBuffer.GetSRV());
    Context.SetDynamicDescriptor(1, 2, g_TemporalColor[Src].GetSRV());
    Context.SetDynamicDescriptor(1, 3, g_LinearDepth[Src].GetSRV());
    Context.SetDynamicDescriptor(1, 4, g_LinearDepth[Dst].GetSRV());
    Context.SetDynamicDescriptor(2, 0, g_TemporalColor[Dst].GetUAV());

    Context.Dispatch2D(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight(), 16, 8);
}

void TemporalEffects::SharpenImage(ComputeContext& Context, ColorBuffer& TemporalColor)
{
    ScopedTimer _prof(L"Sharpen or Copy Image", Context);

    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(TemporalColor, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    Context.SetPipelineState(Sharpness >= 0.001f ? s_SharpenTAACS : s_ResolveTAACS);
    Context.SetConstants(0, 1.0f + Sharpness, 0.25f * Sharpness);
    Context.SetDynamicDescriptor(1, 0, TemporalColor.GetSRV());
    Context.SetDynamicDescriptor(2, 0, g_SceneColorBuffer.GetUAV());
    Context.Dispatch2D(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());
}
