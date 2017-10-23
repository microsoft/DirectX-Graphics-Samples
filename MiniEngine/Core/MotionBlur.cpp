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
#include "MotionBlur.h"
#include "Camera.h"
#include "BufferManager.h"
#include "GraphicsCore.h"
#include "CommandContext.h"
#include "Camera.h"
#include "PostEffects.h"
#include "SystemTime.h"

#include "CompiledShaders/ScreenQuadVS.h"
#include "CompiledShaders/CameraMotionBlurPrePassCS.h"
#include "CompiledShaders/CameraMotionBlurPrePassLinearZCS.h"
#include "CompiledShaders/MotionBlurPrePassCS.h"
#include "CompiledShaders/MotionBlurFinalPassCS.h"
#include "CompiledShaders/MotionBlurFinalPassPS.h"
#include "CompiledShaders/CameraVelocityCS.h"
#include "CompiledShaders/TemporalBlendCS.h"
#include "CompiledShaders/BoundNeighborhoodCS.h"

using namespace Graphics;
using namespace Math;

namespace MotionBlur
{
    BoolVar Enable("Graphics/Motion Blur/Enable", false);

    RootSignature s_RootSignature;
    ComputePSO s_CameraMotionBlurPrePassCS[2];
    ComputePSO s_MotionBlurPrePassCS;
    ComputePSO s_MotionBlurFinalPassCS;
    GraphicsPSO s_MotionBlurFinalPassPS;
    ComputePSO s_CameraVelocityCS[2];
}

void MotionBlur::Initialize( void )
{
    s_RootSignature.Reset(4, 1);
    s_RootSignature.InitStaticSampler(0, SamplerLinearBorderDesc);
    s_RootSignature[0].InitAsConstants(0, 4);
    s_RootSignature[1].InitAsConstantBuffer(1);
    s_RootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 8);
    s_RootSignature[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 8);
    s_RootSignature.Finalize(L"Motion Blur");

#define CreatePSO( ObjName, ShaderByteCode ) \
    ObjName.SetRootSignature(s_RootSignature); \
    ObjName.SetComputeShader(ShaderByteCode, sizeof(ShaderByteCode) ); \
    ObjName.Finalize();

    if (g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
    {
        CreatePSO(s_MotionBlurFinalPassCS, g_pMotionBlurFinalPassCS);
    }
    else
    {
        s_MotionBlurFinalPassPS.SetRootSignature(s_RootSignature);
        s_MotionBlurFinalPassPS.SetRasterizerState( RasterizerTwoSided );
        s_MotionBlurFinalPassPS.SetBlendState( BlendPreMultiplied );
        s_MotionBlurFinalPassPS.SetDepthStencilState( DepthStateDisabled );
        s_MotionBlurFinalPassPS.SetSampleMask(0xFFFFFFFF);
        s_MotionBlurFinalPassPS.SetInputLayout(0, nullptr);
        s_MotionBlurFinalPassPS.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
        s_MotionBlurFinalPassPS.SetVertexShader( g_pScreenQuadVS, sizeof(g_pScreenQuadVS) );
        s_MotionBlurFinalPassPS.SetPixelShader( g_pMotionBlurFinalPassPS, sizeof(g_pMotionBlurFinalPassPS) );
        s_MotionBlurFinalPassPS.SetRenderTargetFormat(g_SceneColorBuffer.GetFormat(), DXGI_FORMAT_UNKNOWN);
        s_MotionBlurFinalPassPS.Finalize();

    }
    CreatePSO( s_CameraMotionBlurPrePassCS[0], g_pCameraMotionBlurPrePassCS );
    CreatePSO( s_CameraMotionBlurPrePassCS[1], g_pCameraMotionBlurPrePassLinearZCS );
    CreatePSO( s_MotionBlurPrePassCS, g_pMotionBlurPrePassCS );
    CreatePSO( s_CameraVelocityCS[0], g_pCameraVelocityCS );
    CreatePSO( s_CameraVelocityCS[1], g_pCameraVelocityCS );

#undef CreatePSO
}

void MotionBlur::Shutdown( void )
{
}

// Linear Z ends up being faster since we haven't officially decompressed the depth buffer.  You 
// would think that it might be slower to use linear Z because we have to convert it back to
// hyperbolic Z for the reprojection.  Nevertheless, the reduced bandwidth and decompress eliminate
// make Linear Z the better choice.  (The choice also lets you evict the depth buffer from ESRAM.)

void MotionBlur::GenerateCameraVelocityBuffer( CommandContext& BaseContext, const Camera& camera, bool UseLinearZ )
{
    GenerateCameraVelocityBuffer(BaseContext, camera.GetReprojectionMatrix(), camera.GetNearClip(), camera.GetFarClip(), UseLinearZ);
}

void MotionBlur::GenerateCameraVelocityBuffer( CommandContext& BaseContext, const Matrix4& reprojectionMatrix, float nearClip, float farClip, bool UseLinearZ)
{
    ScopedTimer _prof(L"Generate Camera Velocity", BaseContext);

    ComputeContext& Context = BaseContext.GetComputeContext();

    Context.SetRootSignature(s_RootSignature);

    uint32_t Width = g_SceneColorBuffer.GetWidth();
    uint32_t Height = g_SceneColorBuffer.GetHeight();

    float RcpHalfDimX = 2.0f / Width;
    float RcpHalfDimY = 2.0f / Height;
    float RcpZMagic = nearClip / (farClip - nearClip);

    Matrix4 preMult = Matrix4(
        Vector4( RcpHalfDimX, 0.0f, 0.0f, 0.0f ),
        Vector4( 0.0f, -RcpHalfDimY, 0.0f, 0.0f),
        Vector4( 0.0f, 0.0f, UseLinearZ ? RcpZMagic : 1.0f, 0.0f ),
        Vector4( -1.0f, 1.0f, UseLinearZ ? -RcpZMagic : 0.0f, 1.0f )
    );

    Matrix4 postMult = Matrix4(
        Vector4( 1.0f / RcpHalfDimX, 0.0f, 0.0f, 0.0f ),
        Vector4( 0.0f, -1.0f / RcpHalfDimY, 0.0f, 0.0f ),
        Vector4( 0.0f, 0.0f, 1.0f, 0.0f ),
        Vector4( 1.0f / RcpHalfDimX, 1.0f / RcpHalfDimY, 0.0f, 1.0f ) );


    Matrix4 CurToPrevXForm = postMult * reprojectionMatrix * preMult;

    Context.SetDynamicConstantBufferView(1, sizeof(CurToPrevXForm), &CurToPrevXForm);
    Context.TransitionResource(g_VelocityBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    ColorBuffer& LinearDepth = g_LinearDepth[ Graphics::GetFrameCount() % 2 ];
    if (UseLinearZ)
        Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    else
        Context.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    Context.SetPipelineState(s_CameraVelocityCS[UseLinearZ ? 1 : 0]);
    Context.SetDynamicDescriptor(3, 0, UseLinearZ ? LinearDepth.GetSRV() : g_SceneDepthBuffer.GetDepthSRV());
    Context.SetDynamicDescriptor(2, 0, g_VelocityBuffer.GetUAV());
    Context.Dispatch2D(Width, Height);
}


void MotionBlur::RenderCameraBlur( CommandContext& BaseContext, const Camera& camera, bool UseLinearZ )
{
    RenderCameraBlur(BaseContext, camera.GetReprojectionMatrix(), camera.GetNearClip(), camera.GetFarClip(), UseLinearZ);
}

void MotionBlur::RenderCameraBlur( CommandContext& BaseContext, const Matrix4& reprojectionMatrix, float nearClip, float farClip, bool UseLinearZ)
{
    ScopedTimer _prof(L"MotionBlur", BaseContext);

    if (!Enable)
        return;

    ComputeContext& Context = BaseContext.GetComputeContext();

    Context.SetRootSignature(s_RootSignature);

    uint32_t Width = g_SceneColorBuffer.GetWidth();
    uint32_t Height = g_SceneColorBuffer.GetHeight();

    float RcpHalfDimX = 2.0f / Width;
    float RcpHalfDimY = 2.0f / Height;
    float RcpZMagic = nearClip / (farClip - nearClip);

    Matrix4 preMult = Matrix4(
        Vector4( RcpHalfDimX, 0.0f, 0.0f, 0.0f ),
        Vector4( 0.0f, -RcpHalfDimY, 0.0f, 0.0f),
        Vector4( 0.0f, 0.0f, UseLinearZ ? RcpZMagic : 1.0f, 0.0f ),
        Vector4( -1.0f, 1.0f, UseLinearZ ? -RcpZMagic : 0.0f, 1.0f )
    );

    Matrix4 postMult = Matrix4(
        Vector4( 1.0f / RcpHalfDimX, 0.0f, 0.0f, 0.0f ),
        Vector4( 0.0f, -1.0f / RcpHalfDimY, 0.0f, 0.0f ),
        Vector4( 0.0f, 0.0f, 1.0f, 0.0f ),
        Vector4( 1.0f / RcpHalfDimX, 1.0f / RcpHalfDimY, 0.0f, 1.0f ) );

    Matrix4 CurToPrevXForm = postMult * reprojectionMatrix * preMult;

    Context.SetDynamicConstantBufferView(1, sizeof(CurToPrevXForm), &CurToPrevXForm);

    ColorBuffer& LinearDepth = g_LinearDepth[ Graphics::GetFrameCount() % 2 ];
    if (UseLinearZ)
        Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    else
        Context.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    if (Enable)
    {
        Context.TransitionResource(g_VelocityBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(g_MotionPrepBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        Context.SetPipelineState(s_CameraMotionBlurPrePassCS[UseLinearZ ? 1 : 0]);
        Context.SetDynamicDescriptor(3, 0, g_SceneColorBuffer.GetSRV());
        Context.SetDynamicDescriptor(3, 1, UseLinearZ ? LinearDepth.GetSRV() : g_SceneDepthBuffer.GetDepthSRV());
        Context.SetDynamicDescriptor(2, 0, g_MotionPrepBuffer.GetUAV());
        Context.SetDynamicDescriptor(2, 1, g_VelocityBuffer.GetUAV());
        Context.Dispatch2D(g_MotionPrepBuffer.GetWidth(), g_MotionPrepBuffer.GetHeight());

        if (g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
        {
            Context.SetPipelineState(s_MotionBlurFinalPassCS);
            Context.SetConstants(0, 1.0f / Width, 1.0f / Height);

            Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            Context.TransitionResource(g_VelocityBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            Context.TransitionResource(g_MotionPrepBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            Context.SetDynamicDescriptor(2, 0, g_SceneColorBuffer.GetUAV());
            Context.SetDynamicDescriptor(3, 0, g_VelocityBuffer.GetSRV());
            Context.SetDynamicDescriptor(3, 1, g_MotionPrepBuffer.GetSRV());

            Context.Dispatch2D(Width, Height);

            Context.InsertUAVBarrier(g_SceneColorBuffer);
        }
        else
        {
            GraphicsContext& GrContext = BaseContext.GetGraphicsContext();
            GrContext.SetRootSignature(s_RootSignature);
            GrContext.SetPipelineState(s_MotionBlurFinalPassPS);
            GrContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
            GrContext.TransitionResource(g_VelocityBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            GrContext.TransitionResource(g_MotionPrepBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            GrContext.SetDynamicDescriptor(3, 0, g_VelocityBuffer.GetSRV());
            GrContext.SetDynamicDescriptor(3, 1, g_MotionPrepBuffer.GetSRV());
            GrContext.SetConstants(0, 1.0f / Width, 1.0f / Height);
            GrContext.SetRenderTarget(g_SceneColorBuffer.GetRTV());
            GrContext.SetViewportAndScissor(0, 0, Width, Height);
            GrContext.Draw(3);
        }
    }
    else
    {
        Context.SetPipelineState(s_CameraVelocityCS[UseLinearZ ? 1 : 0]);
        Context.SetDynamicDescriptor(3, 0, UseLinearZ ? LinearDepth.GetSRV() : g_SceneDepthBuffer.GetDepthSRV());
        Context.SetDynamicDescriptor(2, 0, g_VelocityBuffer.GetUAV());
        Context.Dispatch2D(Width, Height);
    }
}

void MotionBlur::RenderObjectBlur( CommandContext& BaseContext, ColorBuffer& velocityBuffer )
{
    ScopedTimer _prof(L"MotionBlur", BaseContext);

    if (!Enable)
        return;

    uint32_t Width = g_SceneColorBuffer.GetWidth();
    uint32_t Height = g_SceneColorBuffer.GetHeight();

    ComputeContext& Context = BaseContext.GetComputeContext();

    Context.SetRootSignature(s_RootSignature);

    Context.TransitionResource(g_MotionPrepBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(velocityBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    Context.SetDynamicDescriptor(2, 0, g_MotionPrepBuffer.GetUAV());
    Context.SetDynamicDescriptor(3, 0, g_SceneColorBuffer.GetSRV());
    Context.SetDynamicDescriptor(3, 1, velocityBuffer.GetSRV());

    Context.SetPipelineState(s_MotionBlurPrePassCS);
    Context.Dispatch2D(g_MotionPrepBuffer.GetWidth(), g_MotionPrepBuffer.GetHeight());

    if (g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
    {
        Context.SetPipelineState(s_MotionBlurFinalPassCS);

        Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(velocityBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_MotionPrepBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        Context.SetDynamicDescriptor(2, 0, g_SceneColorBuffer.GetUAV());
        Context.SetDynamicDescriptor(3, 0, velocityBuffer.GetSRV());
        Context.SetDynamicDescriptor(3, 1, g_MotionPrepBuffer.GetSRV());
        Context.SetConstants(0, 1.0f / Width, 1.0f / Height);

        Context.Dispatch2D(Width, Height);

        Context.InsertUAVBarrier(g_SceneColorBuffer);
    }
    else
    {
        GraphicsContext& GrContext = BaseContext.GetGraphicsContext();
        GrContext.SetRootSignature(s_RootSignature);
        GrContext.SetPipelineState(s_MotionBlurFinalPassPS);

        GrContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
        GrContext.TransitionResource(velocityBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        GrContext.TransitionResource(g_MotionPrepBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        GrContext.SetDynamicDescriptor(3, 0, velocityBuffer.GetSRV());
        GrContext.SetDynamicDescriptor(3, 1, g_MotionPrepBuffer.GetSRV());
        GrContext.SetConstants(0, 1.0f / Width, 1.0f / Height);
        GrContext.SetRenderTarget(g_SceneColorBuffer.GetRTV());
        GrContext.SetViewportAndScissor(0, 0, Width, Height);

        GrContext.Draw(3);
    }
}

