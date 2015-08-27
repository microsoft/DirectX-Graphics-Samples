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
#include "FXAA.h"
#include "GraphicsCore.h"
#include "BufferManager.h"
#include "CommandContext.h"

#include "CompiledShaders/FXAAPass1_RGB_CS.h"
#include "CompiledShaders/FXAAPass1_Luma_CS.h"
#include "CompiledShaders/FXAAResolveWorkQueueCS.h"
#include "CompiledShaders/FXAAPass2HCS.h"
#include "CompiledShaders/FXAAPass2VCS.h"
#include "CompiledShaders/FXAAPass2HDebugCS.h"
#include "CompiledShaders/FXAAPass2VDebugCS.h"

using namespace Graphics;

namespace FXAA
{
	RootSignature	RootSig;
	ComputePSO		Pass1HdrCS;
	ComputePSO		Pass1LdrCS;
	ComputePSO		ResolveWorkCS;
	ComputePSO		Pass2HCS;
	ComputePSO		Pass2VCS;
	ComputePSO		Pass2HDebugCS;
	ComputePSO		Pass2VDebugCS;
	GpuBuffer		IndirectParameters;

	BoolVar Enable("Graphics/FXAA/Enable", true);
	BoolVar DebugDraw("Graphics/FXAA/Debug", false);

	// With a properly encoded luma buffer, [0.25 = "low", 0.2 = "medium", 0.15 = "high", 0.1 = "ultra"]
	NumVar ContrastThreshold("Graphics/FXAA/Contrast Threshold", 0.175f, 0.05f, 0.5f, 0.025f);

	// Controls how much to blur isolated pixels that have little-to-no edge length.
	NumVar SubpixelRemoval("Graphics/FXAA/Subpixel Removal", 0.50f, 0.0f, 1.0f, 0.25f);

	// This is for testing the performance of computing luma on the fly rather than reusing
	// the luma buffer output of tone mapping.
	BoolVar ForceOffPreComputedLuma("Graphics/FXAA/Always Recompute Log-Luma", false);
}

void FXAA::Initialize( void )
{
	RootSig.Reset(2, 2);
	RootSig.InitStaticSampler(0, SamplerLinearClampDesc);
	RootSig.InitStaticSampler(1, SamplerPointBorderDesc);
	RootSig[0].InitAsConstants(0, 4);
	RootSig[1].InitAsDescriptorTable(2);
	RootSig[1].SetTableRange(0, D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 4);
	RootSig[1].SetTableRange(1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 3);
	RootSig.Finalize();

#define CreatePSO( ObjName, ShaderByteCode ) \
	ObjName.SetRootSignature(RootSig); \
	ObjName.SetComputeShader(ShaderByteCode, sizeof(ShaderByteCode) ); \
	ObjName.Finalize();

	CreatePSO(Pass1LdrCS, g_pFXAAPass1_RGB_CS);		// Use RGB and recompute log-luma; pre-computed luma is unavailable
	CreatePSO(Pass1HdrCS, g_pFXAAPass1_Luma_CS);		// Use pre-computed luma
	CreatePSO(ResolveWorkCS, g_pFXAAResolveWorkQueueCS);
	CreatePSO(Pass2HCS, g_pFXAAPass2HCS);
	CreatePSO(Pass2VCS, g_pFXAAPass2VCS);
	CreatePSO(Pass2HDebugCS, g_pFXAAPass2HDebugCS);
	CreatePSO(Pass2VDebugCS, g_pFXAAPass2VDebugCS);
#undef CreatePSO

	__declspec(align(16)) const uint32_t initArgs[6] = { 0, 1, 1, 0, 1, 1 };
	IndirectParameters.Create(L"FXAA Indirect Parameters", kIndirectArgs, 2, sizeof(D3D12_DISPATCH_ARGUMENTS), initArgs);
}

void FXAA::Shutdown(void)
{
	IndirectParameters.Destroy();
}

void FXAA::Render( ComputeContext& Context, bool bUsePreComputedLuma )
{
	ScopedTimer _prof(L"FXAA", Context);

	if (ForceOffPreComputedLuma)
		bUsePreComputedLuma = false;

	Context.SetRootSignature(RootSig);
	Context.SetConstants(0, 1.0f / g_SceneColorBuffer.GetWidth(), 1.0f / g_SceneColorBuffer.GetHeight(), (float)ContrastThreshold, (float)SubpixelRemoval);

	{
		ScopedTimer _prof(L"Pass 1", Context);

		// Begin by analysing the luminance buffer and setting aside high-contrast pixels in
		// work queues to be processed later.  There are horizontal edge and vertical edge work
		// queues so that the shader logic is simpler for each type of edge.


		g_FXAAWorkQueueH.SetCounterValue(Context, 0);
		g_FXAAWorkQueueV.SetCounterValue(Context, 0);

		Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		Context.TransitionResource(g_FXAAWorkQueueH, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		Context.TransitionResource(g_FXAAWorkQueueV, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		D3D12_CPU_DESCRIPTOR_HANDLE Pass1UAVs[3] = { g_FXAAWorkQueueH.GetUAV(), g_FXAAWorkQueueV.GetUAV(), g_LumaBuffer.GetUAV() };
		Context.SetDynamicDescriptors(1, 0, _countof(Pass1UAVs), Pass1UAVs);
		D3D12_CPU_DESCRIPTOR_HANDLE Pass1SRVs[2] = { g_SceneColorBuffer.GetSRV(), g_LumaBuffer.GetSRV() };
		Context.SetDynamicDescriptors(1, 4, _countof(Pass1SRVs), Pass1SRVs);

		if (bUsePreComputedLuma)
		{
			Context.SetPipelineState(Pass1HdrCS);
			Context.TransitionResource(g_LumaBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}
		else
		{
			Context.SetPipelineState(Pass1LdrCS);
			Context.TransitionResource(g_LumaBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}

		Context.Dispatch2D(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());
	}

	{
		ScopedTimer _prof(L"Pass 2", Context);

		D3D12_CPU_DESCRIPTOR_HANDLE Pass2UAVs[4] = { g_FXAAWorkQueueH.GetUAV(), g_FXAAWorkQueueV.GetUAV(), IndirectParameters.GetUAV(), g_SceneColorBuffer.GetUAV() };
		Context.SetDynamicDescriptors(1, 0, _countof(Pass2UAVs), Pass2UAVs);
		D3D12_CPU_DESCRIPTOR_HANDLE Pass2SRVs[3] = { g_LumaBuffer.GetSRV(), g_FXAAWorkQueueH.GetSRV(), g_FXAAWorkQueueV.GetSRV() };
		Context.SetDynamicDescriptors(1, 4, _countof(Pass2SRVs), Pass2SRVs);

		// The next phase involves converting the work queues to DispatchIndirect parameters.
		// The queues are also padded out to 64 elements to simplify the final consume logic.
		Context.TransitionResource(IndirectParameters, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		Context.SetPipelineState(ResolveWorkCS);
		Context.Dispatch(1, 1, 1);

		Context.TransitionResource(IndirectParameters, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
		Context.TransitionResource(g_FXAAWorkQueueH, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		Context.TransitionResource(g_FXAAWorkQueueV, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		// The final phase involves processing pixels on the work queues and writing them
		// back into the color buffer.  Because the two source pixels required for linearly
		// blending are held in the work queue, this does not require also sampling from
		// the target color buffer (i.e. no read/modify/write, just write.)

		Context.SetPipelineState(DebugDraw ? Pass2HDebugCS : Pass2HCS);
		Context.DispatchIndirect( IndirectParameters, 0 );

		Context.SetPipelineState(DebugDraw ? Pass2VDebugCS : Pass2VCS);
		Context.DispatchIndirect( IndirectParameters, 12 );
	}
}