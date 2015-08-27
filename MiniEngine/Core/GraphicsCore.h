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

#pragma once

#include "PipelineState.h"
#include "DescriptorHeap.h"
#include "RootSignature.h"

class ColorBuffer;
class Sampler;
class GraphicsPSO;
class CommandContext;
class CommandListManager;
class CommandSignature;

namespace Graphics
{
#ifndef RELEASE
	extern const GUID WKPDID_D3DDebugObjectName;
#endif

	using namespace Microsoft::WRL;

    void Initialize( uint32_t width, uint32_t height );
	void Terminate( void );
	void Shutdown( void );

	void Present( void );

	extern uint32_t							g_windowWidth;
	extern uint32_t							g_windowHeight;

	extern ID3D12Device*					g_Device;
	extern CommandListManager				g_CommandManager;

	extern D3D_FEATURE_LEVEL				g_D3DFeatureLevel;

	extern D3D12_SAMPLER_DESC				SamplerLinearWrapDesc;
	extern D3D12_SAMPLER_DESC				SamplerAnisoWrapDesc;
	extern D3D12_SAMPLER_DESC				SamplerShadowDesc;
	extern D3D12_SAMPLER_DESC				SamplerLinearClampDesc;
	extern D3D12_SAMPLER_DESC				SamplerVolumeWrapDesc;
	extern D3D12_SAMPLER_DESC				SamplerPointClampDesc;
	extern D3D12_SAMPLER_DESC				SamplerPointBorderDesc;
	extern D3D12_SAMPLER_DESC				SamplerLinearBorderDesc;

	extern Sampler							SamplerLinearWrap;
	extern Sampler							SamplerAnisoWrap;
	extern Sampler							SamplerShadow;
	extern Sampler							SamplerLinearClamp;
	extern Sampler							SamplerVolumeWrap;
	extern Sampler							SamplerPointClamp;
	extern Sampler							SamplerPointBorder;
	extern Sampler							SamplerLinearBorder;

	extern D3D12_RASTERIZER_DESC			RasterizerDefault;
	extern D3D12_RASTERIZER_DESC			RasterizerDefaultCW;
	extern D3D12_RASTERIZER_DESC			RasterizerTwoSided;
	extern D3D12_RASTERIZER_DESC			RasterizerShadow;
	extern D3D12_RASTERIZER_DESC			RasterizerShadowCW;

	extern D3D12_BLEND_DESC					BlendNoColorWrite;		// XXX
	extern D3D12_BLEND_DESC					BlendDisable;			// 1, 0
	extern D3D12_BLEND_DESC					BlendPreMultiplied;		// 1, 1-SrcA
	extern D3D12_BLEND_DESC					BlendTraditional;		// SrcA, 1-SrcA
	extern D3D12_BLEND_DESC					BlendAdditive;			// 1, 1
	extern D3D12_BLEND_DESC					BlendTraditionalAdditive;// SrcA, 1

	extern D3D12_DEPTH_STENCIL_DESC			DepthStateDisabled;
	extern D3D12_DEPTH_STENCIL_DESC			DepthStateReadWrite;
	extern D3D12_DEPTH_STENCIL_DESC			DepthStateReadOnly;
	extern D3D12_DEPTH_STENCIL_DESC			DepthStateReadOnlyReversed;
	extern D3D12_DEPTH_STENCIL_DESC			DepthStateTestEqual;

	extern CommandSignature					DispatchIndirectCommandSignature;
	extern CommandSignature					DrawIndirectCommandSignature;

	extern DescriptorAllocator				g_DescriptorAllocator[];
	inline D3D12_CPU_DESCRIPTOR_HANDLE		AllocateDescriptor( D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1 )
	{
		return g_DescriptorAllocator[Type].Allocate(Count);
	}

}
