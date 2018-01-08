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
#include "DepthOfField.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "CommandContext.h"
#include "BufferManager.h"

#include "CompiledShaders/DoFPass1CS.h"
#include "CompiledShaders/DoFTilePassCS.h"
#include "CompiledShaders/DoFTilePassFixupCS.h"
#include "CompiledShaders/DoFPreFilterCS.h"
#include "CompiledShaders/DoFPreFilterFastCS.h"
#include "CompiledShaders/DoFPreFilterFixupCS.h"
#include "CompiledShaders/DoFPass2CS.h"
#include "CompiledShaders/DoFPass2FastCS.h"
#include "CompiledShaders/DoFPass2FixupCS.h"
#include "CompiledShaders/DoFPass2DebugCS.h"
#include "CompiledShaders/DoFMedianFilterCS.h"
#include "CompiledShaders/DoFMedianFilterSepAlphaCS.h"
#include "CompiledShaders/DoFMedianFilterFixupCS.h"
#include "CompiledShaders/DoFCombineCS.h"
#include "CompiledShaders/DoFCombine2CS.h"
#include "CompiledShaders/DoFCombineFastCS.h"
#include "CompiledShaders/DoFCombineFast2CS.h"
#include "CompiledShaders/DoFDebugRedCS.h"
#include "CompiledShaders/DoFDebugGreenCS.h"
#include "CompiledShaders/DoFDebugBlueCS.h"

using namespace Graphics;

namespace DepthOfField
{
    BoolVar Enable("Graphics/Depth of Field/Enable", false);
    BoolVar EnablePreFilter("Graphics/Depth of Field/PreFilter", true);
    BoolVar MedianFilter("Graphics/Depth of Field/Median Filter", true);
    BoolVar MedianAlpha("Graphics/Depth of Field/Median Alpha", false);
    NumVar FocalDepth("Graphics/Depth of Field/Focal Center", 0.1f, 0.0f, 1.0f, 0.01f);
    NumVar FocalRange("Graphics/Depth of Field/Focal Radius", 0.1f, 0.0f, 1.0f, 0.01f);
    NumVar ForegroundRange("Graphics/Depth of Field/FG Range", 100.0f, 10.0f, 1000.0f, 10.0f);
    NumVar AntiSparkleWeight("Graphics/Depth of Field/AntiSparkle", 1.0f, 0.0f, 10.0f, 1.0f);
    const char* DebugLabels[] = { "Off", "Foreground", "Background", "FG Alpha", "CoC" };
    EnumVar DebugMode("Graphics/Depth of Field/Debug Mode", 0, _countof(DebugLabels), DebugLabels);
    BoolVar DebugTiles("Graphics/Depth of Field/Debug Tiles", false);
    BoolVar ForceSlow("Graphics/Depth of Field/Force Slow Path", false);
    BoolVar ForceFast("Graphics/Depth of Field/Force Fast Path", false);

    RootSignature s_RootSignature;

    ComputePSO s_DoFPass1CS;				// Responsible for classifying tiles (1st pass)
    ComputePSO s_DoFTilePassCS;				// Disperses tile info to its neighbors (3x3)
    ComputePSO s_DoFTilePassFixupCS;		// Searches for straggler tiles to "fixup"

    ComputePSO s_DoFPreFilterCS;			// Full pre-filter with variable focus
    ComputePSO s_DoFPreFilterFastCS;		// Pre-filter assuming near-constant focus
    ComputePSO s_DoFPreFilterFixupCS;		// Pass through colors for completely in focus tile

    ComputePSO s_DoFPass2CS;				// Perform full CoC convolution pass
    ComputePSO s_DoFPass2FastCS;			// Perform color-only convolution for near-constant focus
    ComputePSO s_DoFPass2FixupCS;			// Pass through colors again
    ComputePSO s_DoFPass2DebugCS;			// Full pass 2 shader with options for debugging

    ComputePSO s_DoFMedianFilterCS;			// 3x3 median filter to reduce fireflies
    ComputePSO s_DoFMedianFilterSepAlphaCS;	// 3x3 median filter to reduce fireflies (separate filter on alpha)
    ComputePSO s_DoFMedianFilterFixupCS;	// Pass through without performing median

    ComputePSO s_DoFCombineCS;				// Combine DoF blurred buffer with focused color buffer
    ComputePSO s_DoFCombineFastCS;			// Upsample DoF blurred buffer
    ComputePSO s_DoFDebugRedCS;				// Output red to entire tile for debugging
    ComputePSO s_DoFDebugGreenCS;			// Output green to entire tile for debugging
    ComputePSO s_DoFDebugBlueCS;			// Output blue to entire tile for debugging

    IndirectArgsBuffer s_IndirectParameters;
}

void DepthOfField::Initialize( void )
{
    s_RootSignature.Reset(4, 3);
    s_RootSignature.InitStaticSampler(0, SamplerPointBorderDesc);
    s_RootSignature.InitStaticSampler(1, SamplerPointClampDesc);
    s_RootSignature.InitStaticSampler(2, SamplerLinearClampDesc);
    s_RootSignature[0].InitAsConstantBuffer(0);
    s_RootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 6);
    s_RootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 3);
    s_RootSignature[3].InitAsConstants(1, 1);
    s_RootSignature.Finalize(L"Depth of Field");

#define CreatePSO( ObjName, ShaderByteCode ) \
    ObjName.SetRootSignature(s_RootSignature); \
    ObjName.SetComputeShader(ShaderByteCode, sizeof(ShaderByteCode) ); \
    ObjName.Finalize();

    CreatePSO( s_DoFPass1CS, g_pDoFPass1CS);
    CreatePSO( s_DoFTilePassCS, g_pDoFTilePassCS);
    CreatePSO( s_DoFTilePassFixupCS, g_pDoFTilePassFixupCS);
    CreatePSO( s_DoFPreFilterCS, g_pDoFPreFilterCS);
    CreatePSO( s_DoFPreFilterFastCS, g_pDoFPreFilterFastCS);
    CreatePSO( s_DoFPreFilterFixupCS, g_pDoFPreFilterFixupCS);
    CreatePSO( s_DoFPass2CS, g_pDoFPass2CS);
    CreatePSO( s_DoFPass2FastCS, g_pDoFPass2FastCS);
    CreatePSO( s_DoFPass2FixupCS, g_pDoFPass2FixupCS);
    CreatePSO( s_DoFPass2DebugCS, g_pDoFPass2DebugCS);
    CreatePSO( s_DoFMedianFilterCS, g_pDoFMedianFilterCS );
    CreatePSO( s_DoFMedianFilterSepAlphaCS, g_pDoFMedianFilterSepAlphaCS );
    CreatePSO( s_DoFMedianFilterFixupCS, g_pDoFMedianFilterFixupCS );
    if (g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
    {
        CreatePSO(s_DoFCombineCS, g_pDoFCombine2CS);
        CreatePSO(s_DoFCombineFastCS, g_pDoFCombineFast2CS);
    }
    else
    {
        CreatePSO(s_DoFCombineCS, g_pDoFCombineCS);
        CreatePSO(s_DoFCombineFastCS, g_pDoFCombineFastCS);
    }
    CreatePSO( s_DoFDebugRedCS, g_pDoFDebugRedCS );
    CreatePSO( s_DoFDebugGreenCS, g_pDoFDebugGreenCS );
    CreatePSO( s_DoFDebugBlueCS, g_pDoFDebugBlueCS );

#undef CreatePSO

    __declspec(align(16)) const uint32_t initArgs[9] = { 0, 1, 1, 0, 1, 1, 0, 1, 1 };
    s_IndirectParameters.Create(L"DoF Indirect Parameters", 3, sizeof(D3D12_DISPATCH_ARGUMENTS), initArgs);
}

void DepthOfField::Shutdown( void )
{
    s_IndirectParameters.Destroy();
}

void DepthOfField::Render( CommandContext& BaseContext, float /*NearClipDist*/, float FarClipDist )
{
    ScopedTimer _prof(L"Depth of Field", BaseContext);

    if (!g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
    {
        WARN_ONCE_IF(!g_bTypedUAVLoadSupport_R11G11B10_FLOAT, "Unable to perform final pass of DoF without support for R11G11B10F UAV loads");
        Enable = false;
    }

    ComputeContext& Context = BaseContext.GetComputeContext();
    Context.SetRootSignature(s_RootSignature);

    ColorBuffer& LinearDepth = g_LinearDepth[ Graphics::GetFrameCount() % 2 ];

    uint32_t BufferWidth = (uint32_t)LinearDepth.GetWidth();
    uint32_t BufferHeight = (uint32_t)LinearDepth.GetHeight();
    uint32_t TiledWidth = (uint32_t)g_DoFTileClass[0].GetWidth();
    uint32_t TiledHeight = (uint32_t)g_DoFTileClass[0].GetHeight();

    __declspec(align(16)) struct DoFConstantBuffer
    {
        float FocalCenter, FocalSpread;
        float FocalMinZ, FocalMaxZ;
        float RcpBufferWidth, RcpBufferHeight;
        uint32_t BufferWidth, BufferHeight;
        int32_t HalfWidth, HalfHeight;
        uint32_t TiledWidth, TiledHeight;
        float RcpTiledWidth, RcpTiledHeight;
        uint32_t DebugState, DisablePreFilter;
        float FGRange, RcpFGRange, AntiSparkleFilterStrength;
    };
    DoFConstantBuffer cbuffer =
    {
        (float)FocalDepth, 1.0f / (float)FocalRange,
        (float)FocalDepth - (float)FocalRange, (float)FocalDepth + (float)FocalRange,
        1.0f / BufferWidth, 1.0f / BufferHeight,
        BufferWidth, BufferHeight,
        (int32_t)Math::DivideByMultiple(BufferWidth, 2), (int32_t)Math::DivideByMultiple(BufferHeight, 2),
        TiledWidth, TiledHeight,
        1.0f / TiledWidth, 1.0f / TiledHeight,
        (uint32_t)DebugMode, EnablePreFilter ? 0u : 1u,
        ForegroundRange / FarClipDist, FarClipDist / ForegroundRange, (float)AntiSparkleWeight
    };
    Context.SetDynamicConstantBufferView(0, sizeof(cbuffer), &cbuffer);

    {
        ScopedTimer _prof2(L"DoF Tiling", Context);

        // Initial pass to discover max CoC and closest depth in 16x16 tiles
        Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_DoFTileClass[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.SetPipelineState(s_DoFPass1CS);
        Context.SetDynamicDescriptor(1, 0, LinearDepth.GetSRV());
        Context.SetDynamicDescriptor(2, 0, g_DoFTileClass[0].GetUAV());
        Context.Dispatch2D(BufferWidth, BufferHeight, 16, 16);

        Context.ResetCounter(g_DoFWorkQueue);
        Context.ResetCounter(g_DoFFastQueue);
        Context.ResetCounter(g_DoFFixupQueue);

        // 3x3 filter to spread max CoC and closest depth to neighboring tiles
        Context.TransitionResource(g_DoFTileClass[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_DoFTileClass[1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(g_DoFWorkQueue, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(g_DoFFastQueue, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.SetPipelineState(s_DoFTilePassCS);
        Context.SetDynamicDescriptor(1, 0, g_DoFTileClass[0].GetSRV());
        Context.SetDynamicDescriptor(2, 0, g_DoFTileClass[1].GetUAV());
        Context.SetDynamicDescriptor(2, 1, g_DoFWorkQueue.GetUAV());
        Context.SetDynamicDescriptor(2, 2, g_DoFFastQueue.GetUAV());
        Context.Dispatch2D(TiledWidth, TiledHeight);

        Context.TransitionResource(g_DoFTileClass[1], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_DoFFixupQueue, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.SetPipelineState(s_DoFTilePassFixupCS);
        Context.SetDynamicDescriptor(1, 0, g_DoFTileClass[1].GetSRV());
        Context.SetDynamicDescriptor(2, 0, g_DoFFixupQueue.GetUAV());
        Context.Dispatch2D(TiledWidth, TiledHeight);

        Context.TransitionResource(g_DoFWorkQueue, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.CopyCounter(s_IndirectParameters, 0, g_DoFWorkQueue);

        Context.TransitionResource(g_DoFFastQueue, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.CopyCounter(s_IndirectParameters, 12, g_DoFFastQueue);

        Context.TransitionResource(g_DoFFixupQueue, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.CopyCounter(s_IndirectParameters, 24, g_DoFFixupQueue);

        Context.TransitionResource(s_IndirectParameters, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
    }

    {
        ScopedTimer _prof2(L"DoF PreFilter", Context);

        if (ForceFast && !DebugMode)
            Context.SetPipelineState(s_DoFPreFilterFastCS);
        else
            Context.SetPipelineState(s_DoFPreFilterCS);
        Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_DoFPresortBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(g_DoFPrefilter, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.SetDynamicDescriptor(1, 0, LinearDepth.GetSRV());
        Context.SetDynamicDescriptor(1, 1, g_DoFTileClass[1].GetSRV());
        Context.SetDynamicDescriptor(1, 2, g_SceneColorBuffer.GetSRV());
        Context.SetDynamicDescriptor(1, 3, g_DoFWorkQueue.GetSRV());
        Context.SetDynamicDescriptor(2, 0, g_DoFPresortBuffer.GetUAV());
        Context.SetDynamicDescriptor(2, 1, g_DoFPrefilter.GetUAV());
        Context.DispatchIndirect(s_IndirectParameters, 0);

        if (!ForceSlow && !DebugMode)
            Context.SetPipelineState(s_DoFPreFilterFastCS);
        Context.SetDynamicDescriptor(1, 3, g_DoFFastQueue.GetSRV());
        Context.DispatchIndirect(s_IndirectParameters, 12);

        Context.SetPipelineState(s_DoFPreFilterFixupCS);
        Context.SetDynamicDescriptor(1, 3, g_DoFFixupQueue.GetSRV());
        Context.DispatchIndirect(s_IndirectParameters, 24);
    }

    {
        ScopedTimer _prof2(L"DoF Main Pass", Context);

        Context.TransitionResource(g_DoFPrefilter, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_DoFBlurColor[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(g_DoFBlurAlpha[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        if (ForceFast && !DebugMode)
            Context.SetPipelineState(s_DoFPass2FastCS);
        else
            Context.SetPipelineState(DebugMode > 0 ? s_DoFPass2DebugCS : s_DoFPass2CS);
        Context.SetDynamicDescriptor(1, 0, g_DoFPrefilter.GetSRV());
        Context.SetDynamicDescriptor(1, 1, g_DoFPresortBuffer.GetSRV());
        Context.SetDynamicDescriptor(1, 2, g_DoFTileClass[1].GetSRV());
        Context.SetDynamicDescriptor(1, 3, g_DoFWorkQueue.GetSRV());
        Context.SetDynamicDescriptor(2, 0, g_DoFBlurColor[0].GetUAV());
        Context.SetDynamicDescriptor(2, 1, g_DoFBlurAlpha[0].GetUAV());
        Context.DispatchIndirect(s_IndirectParameters, 0);

        if (!ForceSlow && !DebugMode)
            Context.SetPipelineState(s_DoFPass2FastCS);
        Context.SetDynamicDescriptor(1, 3, g_DoFFastQueue.GetSRV());
        Context.DispatchIndirect(s_IndirectParameters, 12);

        Context.SetPipelineState(s_DoFPass2FixupCS);
        Context.SetDynamicDescriptor(1, 3, g_DoFFixupQueue.GetSRV());
        Context.DispatchIndirect(s_IndirectParameters, 24);
    }

    {
        ScopedTimer _prof2(L"DoF Median Pass", Context);
        Context.TransitionResource(g_DoFBlurColor[0], D3D12_RESOURCE_STATE_GENERIC_READ);
        Context.TransitionResource(g_DoFBlurAlpha[0], D3D12_RESOURCE_STATE_GENERIC_READ);

        if (MedianFilter)
        {
            Context.TransitionResource(g_DoFBlurColor[1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            Context.TransitionResource(g_DoFBlurAlpha[1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            Context.SetPipelineState(MedianAlpha ? s_DoFMedianFilterSepAlphaCS : s_DoFMedianFilterCS);
            Context.SetDynamicDescriptor(1, 0, g_DoFBlurColor[0].GetSRV());
            Context.SetDynamicDescriptor(1, 1, g_DoFBlurAlpha[0].GetSRV());
            Context.SetDynamicDescriptor(1, 2, g_DoFWorkQueue.GetSRV());
            Context.SetDynamicDescriptor(2, 0, g_DoFBlurColor[1].GetUAV());
            Context.SetDynamicDescriptor(2, 1, g_DoFBlurAlpha[1].GetUAV());
            Context.DispatchIndirect(s_IndirectParameters, 0);

            Context.SetDynamicDescriptor(1, 2, g_DoFFastQueue.GetSRV());
            Context.DispatchIndirect(s_IndirectParameters, 12);

            Context.SetPipelineState(s_DoFMedianFilterFixupCS);
            Context.SetDynamicDescriptor(1, 2, g_DoFFixupQueue.GetSRV());
            Context.DispatchIndirect(s_IndirectParameters, 24);

            Context.TransitionResource(g_DoFBlurColor[1], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            Context.TransitionResource(g_DoFBlurAlpha[1], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        }
    }

    {
        ScopedTimer _prof2(L"DoF Final Combine", Context);
        Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        if (DebugTiles)
        {
            Context.SetPipelineState(s_DoFDebugRedCS);
            Context.SetDynamicDescriptor(1, 5, g_DoFWorkQueue.GetSRV());
            Context.SetDynamicDescriptor(2, 0, g_SceneColorBuffer.GetUAV());
            Context.DispatchIndirect(s_IndirectParameters, 0);

            Context.SetPipelineState(s_DoFDebugGreenCS);
            Context.SetDynamicDescriptor(1, 5, g_DoFFastQueue.GetSRV());
            Context.DispatchIndirect(s_IndirectParameters, 12);

            Context.SetPipelineState(s_DoFDebugBlueCS);
            Context.SetDynamicDescriptor(1, 5, g_DoFFixupQueue.GetSRV());
            Context.DispatchIndirect(s_IndirectParameters, 24);
        }
        else
        {
            Context.SetPipelineState(s_DoFCombineCS);
            Context.SetDynamicDescriptor(1, 0, g_DoFBlurColor[MedianFilter ? 1 : 0].GetSRV());
            Context.SetDynamicDescriptor(1, 1, g_DoFBlurAlpha[MedianFilter ? 1 : 0].GetSRV());
            Context.SetDynamicDescriptor(1, 2, g_DoFTileClass[1].GetSRV());
            Context.SetDynamicDescriptor(1, 3, LinearDepth.GetSRV());
            Context.SetDynamicDescriptor(1, 4, g_DoFWorkQueue.GetSRV());
            Context.SetDynamicDescriptor(2, 0, g_SceneColorBuffer.GetUAV());
            Context.DispatchIndirect(s_IndirectParameters, 0);

            Context.SetPipelineState(s_DoFCombineFastCS);
            Context.SetDynamicDescriptor(1, 4, g_DoFFastQueue.GetSRV());
            Context.DispatchIndirect(s_IndirectParameters, 12);
        }

        Context.InsertUAVBarrier(g_SceneColorBuffer);
    }
}
