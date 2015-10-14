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

#include "CompiledShaders/CameraMotionBlurPrePassCS.h"
#include "CompiledShaders/CameraMotionBlurPrePassLinearZCS.h"
#include "CompiledShaders/MotionBlurPrePassCS.h"
#include "CompiledShaders/MotionBlurFinalPassCS.h"
#include "CompiledShaders/MotionBlurFinalPassTemporalCS.h"
#include "CompiledShaders/CameraVelocityCS.h"
#include "CompiledShaders/TemporalBlendCS.h"

using namespace Graphics;
using namespace Math;

namespace MotionBlur
{
	BoolVar Enable("Graphics/Temporal AA/Motion Blur", false);
	BoolVar TemporalAA("Graphics/Temporal AA/Blend Previous Frame", false);
	NumVar TemporalMaxLerp("Graphics/Temporal AA/Maximum Blend Factor", 0.75f, 0.0f, 1.0f, 0.05f);

	RootSignature s_RootSignature;
	ComputePSO s_CameraMotionBlurPrePassCS[2];
	ComputePSO s_MotionBlurPrePassCS;
	ComputePSO s_MotionBlurFinalPassCS[2];

	ComputePSO s_CameraVelocityCS;

	// Disabled blur but with temporal anti-aliasing that requires a velocity buffer
	ComputePSO s_TemporalBlendCS;
}

void MotionBlur::Initialize( void )
{
	s_RootSignature.Reset(4, 1);
	s_RootSignature.InitStaticSampler(0, SamplerLinearBorderDesc);
	s_RootSignature[0].InitAsConstants(0, 3);
	s_RootSignature[1].InitAsConstantBuffer(1);
	s_RootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 2);
	s_RootSignature[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 4);
	s_RootSignature.Finalize();

#define CreatePSO( ObjName, ShaderByteCode ) \
	ObjName.SetRootSignature(s_RootSignature); \
	ObjName.SetComputeShader(ShaderByteCode, sizeof(ShaderByteCode) ); \
	ObjName.Finalize();

	CreatePSO( s_CameraMotionBlurPrePassCS[0], g_pCameraMotionBlurPrePassCS );
	CreatePSO( s_CameraMotionBlurPrePassCS[1], g_pCameraMotionBlurPrePassLinearZCS );
	CreatePSO( s_MotionBlurPrePassCS, g_pMotionBlurPrePassCS );
	CreatePSO( s_MotionBlurFinalPassCS[0], g_pMotionBlurFinalPassCS );
	CreatePSO( s_MotionBlurFinalPassCS[1], g_pMotionBlurFinalPassTemporalCS );
	CreatePSO( s_CameraVelocityCS, g_pCameraVelocityCS );
	CreatePSO( s_TemporalBlendCS, g_pTemporalBlendCS);

#undef CreatePSO
}

void MotionBlur::Shutdown( void )
{
}

// Linear Z ends up being faster since we haven't officially decompressed the depth buffer.  You 
// would think that it might be slower to use linear Z because we have to convert it back to
// hyperbolic Z for the reprojection.  Nevertheless, the reduced bandwidth and decompress eliminate
// make Linear Z the better choice.  (The choice also lets you evict the depth buffer from ESRAM.)

void MotionBlur::RenderCameraBlur( CommandContext& BaseContext, const Camera& camera, bool UseLinearZ )
{
	RenderCameraBlur(BaseContext, camera.GetReprojectionMatrix(), camera.GetNearClip(), camera.GetFarClip(), UseLinearZ);
}

void MotionBlur::RenderCameraBlur( CommandContext& BaseContext, const Matrix4& reprojectionMatrix, float nearClip, float farClip, bool UseLinearZ)
{
	ScopedTimer _prof(L"MotionBlur", BaseContext);

	ComputeContext& Context = BaseContext.GetComputeContext();

	Context.SetRootSignature(s_RootSignature);

	float RcpHalfDimX = 2.0f / g_SceneColorBuffer.GetWidth();
	float RcpHalfDimY = 2.0f / g_SceneColorBuffer.GetHeight();
	float RcpZMagic = nearClip / (farClip - nearClip);

	Matrix4 preMult = Matrix4(
		Vector4( RcpHalfDimX, 0.0f, 0.0f, 0.0f ),
		Vector4( 0.0f, -RcpHalfDimY, 0.0f, 0.0f),
		Vector4( 0.0f, 0.0f, UseLinearZ ? RcpZMagic : 1.0f, 0.0f ),
		Vector4( -1.0f, 1.0f, UseLinearZ ? -RcpZMagic: 0.0f, 1.0f )
	);

	Matrix4 postMult = Matrix4(
		Vector4( 1.0f / RcpHalfDimX, 0.0f, 0.0f, 0.0f ),
		Vector4( 0.0f, -1.0f / RcpHalfDimY, 0.0f, 0.0f ),
		Vector4( 0.0f, 0.0f, 1.0f, 0.0f ),
		Vector4( 1.0f / RcpHalfDimX, 1.0f / RcpHalfDimY, 0.0f, 1.0f ) );

	struct PrePassCB
	{
		Matrix4 CurToPrevXForm;
		float RcpBufferWidth;
		float RcpBufferHeight;
		float MaxTemporalBlend;
	} params;

	params.CurToPrevXForm = postMult * reprojectionMatrix * preMult;
	params.RcpBufferWidth = 1.0f / g_SceneColorBuffer.GetWidth();
	params.RcpBufferHeight = 1.0f / g_SceneColorBuffer.GetHeight();
	params.MaxTemporalBlend = TemporalMaxLerp;
	Context.SetDynamicConstantBufferView(1, sizeof(PrePassCB), &params);

	Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	uint32_t thisFrame = Graphics::GetFrameCount() & 1;
	uint32_t lastFrame = thisFrame ^ 1;

	if (!TemporalAA)
	{
		BaseContext.GetGraphicsContext().ClearColor(g_TemporalBuffer[thisFrame]);
		BaseContext.GetGraphicsContext().ClearColor(g_TemporalBuffer[lastFrame]);
	}

	if (!Enable)
	{
		if (TemporalAA)
		{
			Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			Context.TransitionResource(g_LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			Context.TransitionResource(g_TemporalBuffer[lastFrame], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			Context.TransitionResource(g_TemporalBuffer[thisFrame], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			D3D12_CPU_DESCRIPTOR_HANDLE SRVs[] = { g_SceneColorBuffer.GetSRV(), g_LinearDepth.GetSRV(), g_TemporalBuffer[lastFrame].GetSRV() };
			D3D12_CPU_DESCRIPTOR_HANDLE Pass2UAVs[] = { g_SceneColorBuffer.GetUAV(), g_TemporalBuffer[thisFrame].GetUAV() };

			Context.SetDynamicDescriptors(2, 0, 2, Pass2UAVs);
			Context.SetDynamicDescriptors(3, 0, 3, SRVs);
			Context.SetPipelineState(s_TemporalBlendCS);
			Context.Dispatch2D(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());
		}
		return;
	}

	Context.TransitionResource(g_MotionPrepBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	Context.TransitionResource(g_VelocityBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	D3D12_CPU_DESCRIPTOR_HANDLE Pass1UAVs[] = { g_MotionPrepBuffer.GetUAV(), g_VelocityBuffer.GetUAV() };
	Context.SetDynamicDescriptors(2, 0, 2, Pass1UAVs);
	D3D12_CPU_DESCRIPTOR_HANDLE Pass1SRVs[2] = { g_SceneColorBuffer.GetSRV() };
	if (UseLinearZ)
	{
		Context.TransitionResource(g_LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		Pass1SRVs[1] = g_LinearDepth.GetSRV();
	}
	else
	{
		Context.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		Pass1SRVs[1] = g_SceneDepthBuffer.GetDepthSRV();
	}
	Context.SetDynamicDescriptors(3, 0, 2, Pass1SRVs);

	Context.SetPipelineState(s_CameraMotionBlurPrePassCS[UseLinearZ ? 1 : 0]);
	Context.Dispatch2D(g_MotionPrepBuffer.GetWidth(), g_MotionPrepBuffer.GetHeight());

	Context.SetConstants(0, 1.0f / g_SceneColorBuffer.GetWidth(), 1.0f / g_SceneColorBuffer.GetHeight(), (float)TemporalMaxLerp );

	Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	Context.TransitionResource(g_VelocityBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	Context.TransitionResource(g_MotionPrepBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	Context.SetDynamicDescriptor(2, 0, g_SceneColorBuffer.GetUAV());
	Context.SetDynamicDescriptor(3, 0, g_SceneColorBuffer.GetSRV());
	Context.SetDynamicDescriptor(3, 1, g_VelocityBuffer.GetSRV());
	Context.SetDynamicDescriptor(3, 2, g_MotionPrepBuffer.GetSRV());

	if (TemporalAA)
	{
		Context.TransitionResource(g_TemporalBuffer[thisFrame], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		Context.TransitionResource(g_TemporalBuffer[lastFrame], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		Context.SetDynamicDescriptor(2, 1, g_TemporalBuffer[thisFrame].GetUAV());
		Context.SetDynamicDescriptor(3, 3, g_TemporalBuffer[lastFrame].GetSRV());
		Context.SetPipelineState(s_MotionBlurFinalPassCS[1]);
		Context.Dispatch2D(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());
	}
	else
	{
		Context.SetPipelineState(s_MotionBlurFinalPassCS[0]);
		Context.Dispatch2D(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());
	}
}

void MotionBlur::RenderObjectBlur( CommandContext& BaseContext, ColorBuffer& velocityBuffer )
{
	ScopedTimer _prof(L"MotionBlur", BaseContext);

	ComputeContext& Context = BaseContext.GetComputeContext();

	Context.SetRootSignature(s_RootSignature);

	uint32_t thisFrame = Graphics::GetFrameCount() & 1;
	uint32_t lastFrame = thisFrame ^ 1;

	if (!TemporalAA)
		BaseContext.GetGraphicsContext().ClearColor(g_TemporalBuffer[thisFrame]);

	if (!Enable)
		return;

	Context.TransitionResource(g_MotionPrepBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	Context.TransitionResource(velocityBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	D3D12_CPU_DESCRIPTOR_HANDLE Pass1SRVs[] = { g_SceneColorBuffer.GetSRV(), velocityBuffer.GetSRV() };
	Context.SetDynamicDescriptors(2, 0, 1, &g_MotionPrepBuffer.GetUAV());
	Context.SetDynamicDescriptors(3, 0, 2, Pass1SRVs);

	Context.SetPipelineState(s_MotionBlurPrePassCS);
	Context.Dispatch2D(g_MotionPrepBuffer.GetWidth(), g_MotionPrepBuffer.GetHeight());

	Context.SetConstants(0, 1.0f / g_SceneColorBuffer.GetWidth(), 1.0f / g_SceneColorBuffer.GetHeight(), (float)TemporalMaxLerp );

	Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	Context.TransitionResource(velocityBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	Context.TransitionResource(g_MotionPrepBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	D3D12_CPU_DESCRIPTOR_HANDLE Pass2UAVs[] = { g_SceneColorBuffer.GetUAV(), g_TemporalBuffer[thisFrame].GetUAV() };
	D3D12_CPU_DESCRIPTOR_HANDLE Pass2SRVs[] = { g_SceneColorBuffer.GetSRV(), velocityBuffer.GetSRV(), g_MotionPrepBuffer.GetSRV(), g_TemporalBuffer[lastFrame].GetSRV() };

	if (TemporalAA)
	{
		Context.TransitionResource(g_TemporalBuffer[thisFrame], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		Context.TransitionResource(g_TemporalBuffer[lastFrame], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		Context.SetDynamicDescriptors(2, 0, 2, Pass2UAVs);
		Context.SetDynamicDescriptors(3, 0, 4, Pass2SRVs);
		Context.SetPipelineState(s_MotionBlurFinalPassCS[1]);
		Context.Dispatch2D(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());
	}
	else
	{
		Context.SetDynamicDescriptors(2, 0, 1, Pass2UAVs);
		Context.SetDynamicDescriptors(3, 0, 3, Pass2SRVs);
		Context.SetPipelineState(s_MotionBlurFinalPassCS[0]);
		Context.Dispatch2D(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());
	}
}
