//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "EngineProfiling.h"
#include "GpuKernels.h"
#include "DirectXRaytracingHelper.h"
#include "D3D12RaytracingRealTimeDenoisedAmbientOcclusion.h"
#include "CompiledShaders\ReduceSumUintCS.hlsl.h"
#include "CompiledShaders\ReduceSumFloatCS.hlsl.h"
#include "CompiledShaders\CalculatePartialDerivativesViaCentralDifferencesCS.hlsl.h"
#include "CompiledShaders\DownsampleGBufferDataBilateralFilter2x2CS.hlsl.h"
#include "CompiledShaders\UpsampleBilateralFilter2x2FloatCS.hlsl.h"
#include "CompiledShaders\UpsampleBilateralFilter2x2UintCS.hlsl.h"
#include "CompiledShaders\UpsampleBilateralFilter2x2Float2CS.hlsl.h"
#include "CompiledShaders\GenerateGrassStrawsCS.hlsl.h"

using namespace std;

namespace GpuKernels
{
	namespace RootSignature {
		namespace ReduceSum {
			namespace Slot {
				enum Enum {
					Output = 0,
					Input,
					Count
				};
			}
		}
	}

	void ReduceSum::Initialize(ID3D12Device5* device, Type type)
	{
        m_resultType = type;

		// Create root signature.
		{
			using namespace RootSignature::ReduceSum;

			CD3DX12_DESCRIPTOR_RANGE ranges[Slot::Count];
			ranges[Slot::Input].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);  // 1 input texture
			ranges[Slot::Output].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 1 output texture

			CD3DX12_ROOT_PARAMETER rootParameters[Slot::Count];
			rootParameters[Slot::Input].InitAsDescriptorTable(1, &ranges[Slot::Input]);
			rootParameters[Slot::Output].InitAsDescriptorTable(1, &ranges[Slot::Output]);

			CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
			SerializeAndCreateRootSignature(device, rootSignatureDesc, &m_rootSignature, L"Compute root signature: ReduceSum");
		}

        // Create compute pipeline state.
        {
            D3D12_COMPUTE_PIPELINE_STATE_DESC descComputePSO = {};
            descComputePSO.pRootSignature = m_rootSignature.Get();
            
            switch (m_resultType)
            {
            case Uint:
                descComputePSO.CS = CD3DX12_SHADER_BYTECODE(static_cast<const void*>(g_pReduceSumUintCS), ARRAYSIZE(g_pReduceSumUintCS));
                break;
            case Float:
                descComputePSO.CS = CD3DX12_SHADER_BYTECODE(static_cast<const void*>(g_pReduceSumFloatCS), ARRAYSIZE(g_pReduceSumFloatCS));
                break;
            }

            ThrowIfFailed(device->CreateComputePipelineState(&descComputePSO, IID_PPV_ARGS(&m_pipelineStateObject)));
            m_pipelineStateObject->SetName(L"Pipeline state object: ReduceSum");
        }
	}

	void ReduceSum::CreateInputResourceSizeDependentResources(
		ID3D12Device5* device,
		DX::DescriptorHeap* descriptorHeap,
		UINT frameCount,
		UINT width,
		UINT height,
		UINT numInvocationsPerFrame)
	{
		// Create shader resources
		{
			width = CeilDivide(width, ReduceSumCS::ThreadGroup::NumElementsToLoadPerThread);
			
			// Number of reduce iterations to bring [width, height] down to [1, 1]
			UINT numIterations = max(
				CeilLogWithBase(width, ReduceSumCS::ThreadGroup::Width),
				CeilLogWithBase(height, ReduceSumCS::ThreadGroup::Height));

            DXGI_FORMAT format;
            switch (m_resultType)
            {
            case Uint: format = DXGI_FORMAT_R32_UINT; break;
            case Float: format = DXGI_FORMAT_R32_FLOAT; break;
            }

			m_csReduceSumOutputs.resize(numIterations);
			for (UINT i = 0; i < numIterations; i++)
			{
				width = max(1u, CeilDivide(width, ReduceSumCS::ThreadGroup::Width));
				height = max(1u, CeilDivide(height, ReduceSumCS::ThreadGroup::Height));

				CreateRenderTargetResource(device, format, width, height, descriptorHeap,
					&m_csReduceSumOutputs[i], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"UAV texture: ReduceSum intermediate output");
			}

            switch (m_resultType)
            {
            case Uint: m_resultSize = sizeof(UINT); break;
            case Float: m_resultSize = sizeof(float); break;
                break;
            }
			m_readbackResources.resize(numInvocationsPerFrame);
			for (UINT i = 0; i < m_readbackResources.size(); i++)
				for (auto& readbackResource : m_readbackResources)
				{
					wstringstream wResourceName;
					wResourceName << L"Readback buffer - ReduceSum output [" << i << L"]";
					AllocateReadBackBuffer(device, frameCount * m_resultSize, &m_readbackResources[i], D3D12_RESOURCE_STATE_COPY_DEST, wResourceName.str().c_str());
				}
		}
	}

	void ReduceSum::Run(
		ID3D12GraphicsCommandList4* commandList,
		ID3D12DescriptorHeap* descriptorHeap, 
		UINT frameIndex,
        D3D12_GPU_DESCRIPTOR_HANDLE inputResourceHandle,
		void* resultSum,
        UINT invocationIndex)   // per frame invocation index)
	{
		using namespace RootSignature::ReduceSum;
		
        ScopedTimer _prof(L"ReduceSum", commandList);

		// Set pipeline state.
		{
			commandList->SetDescriptorHeaps(1, &descriptorHeap);
			commandList->SetComputeRootSignature(m_rootSignature.Get());
			commandList->SetPipelineState(m_pipelineStateObject.Get());
		}

		//
		// Iterative sum reduce [width, height] to [1,1]
		//
		SIZE_T readBackBaseOffset = frameIndex * m_resultSize;
		{
			// First iteration reads from input resource.		
			commandList->SetComputeRootDescriptorTable(Slot::Input, inputResourceHandle);
			commandList->SetComputeRootDescriptorTable(Slot::Output, m_csReduceSumOutputs[0].gpuDescriptorWriteAccess);

			for (UINT i = 0; i < m_csReduceSumOutputs.size(); i++)
			{
				auto outputResourceDesc = m_csReduceSumOutputs[i].resource.Get()->GetDesc();

				// Each group writes out a single summed result across group threads.
				XMUINT2 groupSize(static_cast<UINT>(outputResourceDesc.Width), static_cast<UINT>(outputResourceDesc.Height));

				// Dispatch.
				commandList->Dispatch(groupSize.x, groupSize.y, 1);

				// Set the output resource as input in the next iteration. 
				if (i < m_csReduceSumOutputs.size() - 1)
				{
					commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_csReduceSumOutputs[i].resource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
					commandList->SetComputeRootDescriptorTable(Slot::Input, m_csReduceSumOutputs[i].gpuDescriptorReadAccess);
					commandList->SetComputeRootDescriptorTable(Slot::Output, m_csReduceSumOutputs[i + 1].gpuDescriptorWriteAccess);
				}
				else  // We're done, prepare the last output for copy to readback.
				{
					commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_csReduceSumOutputs.back().resource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
				}
			}

			// Copy the sum result to the readback buffer.
			auto destDesc = m_readbackResources[invocationIndex]->GetDesc();
			auto srcDesc = m_csReduceSumOutputs.back().resource.Get()->GetDesc();
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT bufferFootprint = {};
			bufferFootprint.Offset = 0;
			bufferFootprint.Footprint.Width = static_cast<UINT>(destDesc.Width / m_resultSize);
			bufferFootprint.Footprint.Height = 1;
			bufferFootprint.Footprint.Depth = 1;
			bufferFootprint.Footprint.RowPitch = Align(static_cast<UINT>(destDesc.Width) * m_resultSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
			bufferFootprint.Footprint.Format = srcDesc.Format;
			CD3DX12_TEXTURE_COPY_LOCATION copyDest(m_readbackResources[invocationIndex].Get(), bufferFootprint);
			CD3DX12_TEXTURE_COPY_LOCATION copySrc(m_csReduceSumOutputs.back().resource.Get(), 0);
			commandList->CopyTextureRegion(&copyDest, frameIndex, 0, 0, &copySrc, nullptr);

			// Transition the intermediate output resources back.
			{
				std::vector<D3D12_RESOURCE_BARRIER> barriers;
				barriers.resize(m_csReduceSumOutputs.size());
				for (UINT i = 0; i < m_csReduceSumOutputs.size() - 1; i++)
				{
					barriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(m_csReduceSumOutputs[i].resource.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				}
				barriers[m_csReduceSumOutputs.size() - 1] = CD3DX12_RESOURCE_BARRIER::Transition(m_csReduceSumOutputs.back().resource.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

				commandList->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
			}
		}

		// Performance optimization.
		// To avoid stalling CPU until GPU is done, grab the data from a finished frame FrameCount ago.
		// This is fine for informational purposes such as using the value for UI display.
		UINT* mappedData = nullptr;
		CD3DX12_RANGE readRange(readBackBaseOffset, readBackBaseOffset + m_resultSize);
		ThrowIfFailed(m_readbackResources[invocationIndex]->Map(0, &readRange, reinterpret_cast<void**>(&mappedData)));
		memcpy(resultSum, mappedData, m_resultSize);
		m_readbackResources[invocationIndex]->Unmap(0, &CD3DX12_RANGE(0, 0));
	}


    namespace RootSignature {
        namespace DownsampleGBufferDataBilateralFilter {
            namespace Slot {
                enum Enum {
                    OutputNormal = 0,
                    OutputPosition,
                    OutputPartialDistanceDerivative,
                    OutputMotionVector,
                    OutputPrevFrameHitPosition,
                    OutputDepth,
                    OutputSurfaceAlbedo,
                    InputNormal,
                    InputPosition,
                    InputPartialDistanceDerivative,
                    InputMotionVector,
                    InputPrevFrameHitPosition,
                    InputDepth,
                    InputSurfaceAlbedo,
                    ConstantBuffer,
                    Count
                };
            }
        }
    }

    void DownsampleGBufferDataBilateralFilter::Initialize(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame)
    {
        // Create root signature.
        {
            using namespace RootSignature::DownsampleGBufferDataBilateralFilter;

            CD3DX12_DESCRIPTOR_RANGE ranges[Slot::Count]; 
            ranges[Slot::InputNormal].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);  // 1 input normal texture
            ranges[Slot::InputPosition].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);  // 1 input position texture
            ranges[Slot::InputPartialDistanceDerivative].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);  // 1 input partial distance derivative
            ranges[Slot::OutputNormal].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);  // 1 output normal texture
            ranges[Slot::OutputPosition].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);  // 1 output position texture
            ranges[Slot::OutputPartialDistanceDerivative].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 4);  // 1 output partial distance derivative
            ranges[Slot::InputDepth].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);  // 1 input depth
            ranges[Slot::OutputDepth].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 5);  // 1 output depth
            ranges[Slot::InputMotionVector].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6);  // 1 input motion vector
            ranges[Slot::OutputMotionVector].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 6);  // 1 output motion vector
            ranges[Slot::InputPrevFrameHitPosition].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7);  // 1 input previous frame hit position
            ranges[Slot::OutputPrevFrameHitPosition].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 7);  // 1 output previous frame hit position
            ranges[Slot::InputSurfaceAlbedo].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8);
            ranges[Slot::OutputSurfaceAlbedo].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 8);
        
            CD3DX12_ROOT_PARAMETER rootParameters[Slot::Count];
            rootParameters[Slot::InputNormal].InitAsDescriptorTable(1, &ranges[Slot::InputNormal]);
            rootParameters[Slot::InputPosition].InitAsDescriptorTable(1, &ranges[Slot::InputPosition]);
            rootParameters[Slot::InputPartialDistanceDerivative].InitAsDescriptorTable(1, &ranges[Slot::InputPartialDistanceDerivative]);
            rootParameters[Slot::OutputNormal].InitAsDescriptorTable(1, &ranges[Slot::OutputNormal]);
            rootParameters[Slot::OutputPosition].InitAsDescriptorTable(1, &ranges[Slot::OutputPosition]);
            rootParameters[Slot::OutputPartialDistanceDerivative].InitAsDescriptorTable(1, &ranges[Slot::OutputPartialDistanceDerivative]);
            rootParameters[Slot::InputDepth].InitAsDescriptorTable(1, &ranges[Slot::InputDepth]);
            rootParameters[Slot::OutputDepth].InitAsDescriptorTable(1, &ranges[Slot::OutputDepth]);
            rootParameters[Slot::InputMotionVector].InitAsDescriptorTable(1, &ranges[Slot::InputMotionVector]);
            rootParameters[Slot::OutputMotionVector].InitAsDescriptorTable(1, &ranges[Slot::OutputMotionVector]);
            rootParameters[Slot::InputPrevFrameHitPosition].InitAsDescriptorTable(1, &ranges[Slot::InputPrevFrameHitPosition]);
            rootParameters[Slot::OutputPrevFrameHitPosition].InitAsDescriptorTable(1, &ranges[Slot::OutputPrevFrameHitPosition]);
            rootParameters[Slot::InputSurfaceAlbedo].InitAsDescriptorTable(1, &ranges[Slot::InputSurfaceAlbedo]);
            rootParameters[Slot::OutputSurfaceAlbedo].InitAsDescriptorTable(1, &ranges[Slot::OutputSurfaceAlbedo]);
            rootParameters[Slot::ConstantBuffer].InitAsConstantBufferView(0);

            CD3DX12_STATIC_SAMPLER_DESC staticSamplers[] = {
                CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP) };

            CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters, ARRAYSIZE(staticSamplers), staticSamplers);
            SerializeAndCreateRootSignature(device, rootSignatureDesc, &m_rootSignature, L"Compute root signature: DownsampleGBufferDataBilateralFilter");
        }

        // Create compute pipeline state.
        {
            D3D12_COMPUTE_PIPELINE_STATE_DESC descComputePSO = {};
            descComputePSO.pRootSignature = m_rootSignature.Get();
            descComputePSO.CS = CD3DX12_SHADER_BYTECODE(static_cast<const void*>(g_pDownsampleGBufferDataBilateralFilter2x2CS), ARRAYSIZE(g_pDownsampleGBufferDataBilateralFilter2x2CS));

            ThrowIfFailed(device->CreateComputePipelineState(&descComputePSO, IID_PPV_ARGS(&m_pipelineStateObject)));
            m_pipelineStateObject->SetName(L"Pipeline state object: DownsampleGBufferDataBilateralFilter");
        }

        // Create shader resources
        {
            m_CB.Create(device, frameCount * numCallsPerFrame, L"Constant Buffer: DownsampleGBufferDataBilateralFilter");
        }
    }

    // Downsamples input resource.
    // width, height - dimensions of the input resource.
    void DownsampleGBufferDataBilateralFilter::Run(
        ID3D12GraphicsCommandList4* commandList,
        UINT width,
        UINT height,
        ID3D12DescriptorHeap* descriptorHeap,
        D3D12_GPU_DESCRIPTOR_HANDLE inputNormalResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE inputPositionResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE inputPartialDistanceDerivativesResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE inputMotionVectorResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE inputPrevFrameHitPositionResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE inputDepthResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE inputSurfaceAlbedoResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE outputNormalResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE outputPositionResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE outputPartialDistanceDerivativesResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE outputMotionVectorResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE outputPrevFrameHitPositionResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE outputDepthResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE outputSurfaceAlbedoResourceHandle)
    {
        using namespace RootSignature::DownsampleGBufferDataBilateralFilter;
        using namespace DefaultComputeShaderParams;

        ScopedTimer _prof(L"DownsampleGBufferDataBilateralFilter", commandList);

        m_CB->textureDim = XMUINT2(width, height);
        m_CB->invTextureDim = XMFLOAT2(1.f / width, 1.f / height);
        m_CBinstanceID = (m_CBinstanceID + 1) % m_CB.NumInstances();
        m_CB.CopyStagingToGpu(m_CBinstanceID);

        // Set pipeline state.
        {
            commandList->SetDescriptorHeaps(1, &descriptorHeap);
            commandList->SetComputeRootSignature(m_rootSignature.Get());
            commandList->SetComputeRootDescriptorTable(Slot::InputNormal, inputNormalResourceHandle);
            commandList->SetComputeRootDescriptorTable(Slot::InputPosition, inputPositionResourceHandle);
            commandList->SetComputeRootDescriptorTable(Slot::InputPartialDistanceDerivative, inputPartialDistanceDerivativesResourceHandle);
            commandList->SetComputeRootDescriptorTable(Slot::InputMotionVector, inputMotionVectorResourceHandle);
            commandList->SetComputeRootDescriptorTable(Slot::InputPrevFrameHitPosition, inputPrevFrameHitPositionResourceHandle);
            commandList->SetComputeRootDescriptorTable(Slot::InputDepth, inputDepthResourceHandle);
            commandList->SetComputeRootDescriptorTable(Slot::InputSurfaceAlbedo, inputSurfaceAlbedoResourceHandle);
            commandList->SetComputeRootDescriptorTable(Slot::OutputNormal, outputNormalResourceHandle);
            commandList->SetComputeRootDescriptorTable(Slot::OutputPosition, outputPositionResourceHandle);
            commandList->SetComputeRootDescriptorTable(Slot::OutputPartialDistanceDerivative, outputPartialDistanceDerivativesResourceHandle);
            commandList->SetComputeRootDescriptorTable(Slot::OutputMotionVector, outputMotionVectorResourceHandle);
            commandList->SetComputeRootDescriptorTable(Slot::OutputPrevFrameHitPosition, outputPrevFrameHitPositionResourceHandle);
            commandList->SetComputeRootDescriptorTable(Slot::OutputDepth, outputDepthResourceHandle);
            commandList->SetComputeRootDescriptorTable(Slot::OutputSurfaceAlbedo, outputSurfaceAlbedoResourceHandle);
            commandList->SetComputeRootConstantBufferView(Slot::ConstantBuffer, m_CB.GpuVirtualAddress(m_CBinstanceID));
            commandList->SetPipelineState(m_pipelineStateObject.Get());
        }

        XMUINT2 groupSize(CeilDivide((width + 1) / 2 + 1, ThreadGroup::Width), CeilDivide((height + 1) / 2 + 1, ThreadGroup::Height));

        // Dispatch.
        commandList->Dispatch(groupSize.x, groupSize.y, 1);
    }

    namespace RootSignature {
        namespace UpsampleBilateralFilter {
            namespace Slot {
                enum Enum {
                    Output = 0,
                    Input,
                    InputLowResNormal,
                    InputHiResNormal,
                    InputHiResPartialDistanceDerivative,
                    Debug1,
                    Debug2,
                    ConstantBuffer,
                    Count
                };
            }
        }
    }

    void UpsampleBilateralFilter::Initialize(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame)
    {
        // Create root signature.
        {
            using namespace RootSignature::UpsampleBilateralFilter;

            CD3DX12_DESCRIPTOR_RANGE ranges[Slot::Count];
            ranges[Slot::Input].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);  // 1 input texture
            ranges[Slot::InputLowResNormal].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);  // 1 input normal low res texture
            ranges[Slot::InputHiResNormal].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);  // 1 input normal high res texture
            ranges[Slot::InputHiResPartialDistanceDerivative].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);  // 1 input partial distance derivative texture
            ranges[Slot::Output].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 1 output texture
            ranges[Slot::Debug1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1); 
            ranges[Slot::Debug2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);

            CD3DX12_ROOT_PARAMETER rootParameters[Slot::Count];
            rootParameters[Slot::Input].InitAsDescriptorTable(1, &ranges[Slot::Input]);
            rootParameters[Slot::InputLowResNormal].InitAsDescriptorTable(1, &ranges[Slot::InputLowResNormal]);
            rootParameters[Slot::InputHiResNormal].InitAsDescriptorTable(1, &ranges[Slot::InputHiResNormal]);
            rootParameters[Slot::InputHiResPartialDistanceDerivative].InitAsDescriptorTable(1, &ranges[Slot::InputHiResPartialDistanceDerivative]);
            rootParameters[Slot::Output].InitAsDescriptorTable(1, &ranges[Slot::Output]);
            rootParameters[Slot::Debug1].InitAsDescriptorTable(1, &ranges[Slot::Debug1]);
            rootParameters[Slot::Debug2].InitAsDescriptorTable(1, &ranges[Slot::Debug2]);
            rootParameters[Slot::ConstantBuffer].InitAsConstantBufferView(0);

            CD3DX12_STATIC_SAMPLER_DESC staticSamplers[] = {
                CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP) };

            CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters, ARRAYSIZE(staticSamplers), staticSamplers);
            SerializeAndCreateRootSignature(device, rootSignatureDesc, &m_rootSignature, L"Compute root signature: UpsampleBilateralFilter");
        }

        // Create compute pipeline state.
        {
            D3D12_COMPUTE_PIPELINE_STATE_DESC descComputePSO = {};
            descComputePSO.pRootSignature = m_rootSignature.Get();

            for (UINT i = 0; i < FilterType::Count; i++)
            {
                switch (i)
                {
                case Filter2x2FloatR:
                    descComputePSO.CS = CD3DX12_SHADER_BYTECODE(static_cast<const void*>(g_pUpsampleBilateralFilter2x2FloatCS), ARRAYSIZE(g_pUpsampleBilateralFilter2x2FloatCS));
                    break;
                case Filter2x2UintR:
                    descComputePSO.CS = CD3DX12_SHADER_BYTECODE(static_cast<const void*>(g_pUpsampleBilateralFilter2x2UintCS), ARRAYSIZE(g_pUpsampleBilateralFilter2x2UintCS));
                    break;
                case Filter2x2FloatRG:
                    descComputePSO.CS = CD3DX12_SHADER_BYTECODE(static_cast<const void*>(g_pUpsampleBilateralFilter2x2Float2CS), ARRAYSIZE(g_pUpsampleBilateralFilter2x2Float2CS));
                    break;
                }

                ThrowIfFailed(device->CreateComputePipelineState(&descComputePSO, IID_PPV_ARGS(&m_pipelineStateObjects[i])));
                m_pipelineStateObjects[i]->SetName(L"Pipeline state object: UpsampleBilateralFilter");
            }
        }

        // Create shader resources
        {
            m_CB.Create(device, frameCount * numCallsPerFrame, L"Constant Buffer: GaussianFilter");
        }
    }

    // Resamples input resource.
    // width, height - dimensions of the output resource.
    void UpsampleBilateralFilter::Run(
        ID3D12GraphicsCommandList4* commandList,
        UINT width,
        UINT height,
        FilterType type,
        ID3D12DescriptorHeap* descriptorHeap,
        D3D12_GPU_DESCRIPTOR_HANDLE inputResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE inputLowResNormalResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE inputHiResNormalResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE inputHiResPartialDistanceDerivativeResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE outputResourceHandle)
    {
        using namespace RootSignature::UpsampleBilateralFilter;
        using namespace DefaultComputeShaderParams;

        ScopedTimer _prof(L"UpsampleBilateralFilter", commandList);

        // Each shader execution processes 2x2 hiRes pixels
        XMUINT2 lowResDim = XMUINT2(CeilDivide(width, 2), CeilDivide(height, 2));

        m_CB->invHiResTextureDim = XMFLOAT2(1.f / width, 1.f / height);
        m_CB->invLowResTextureDim = XMFLOAT2(1.f / lowResDim.x, 1.f / lowResDim.y);
        m_CBinstanceID = (m_CBinstanceID + 1) % m_CB.NumInstances();
        m_CB.CopyStagingToGpu(m_CBinstanceID);

        // Set pipeline state.
        {
            commandList->SetDescriptorHeaps(1, &descriptorHeap);
            commandList->SetComputeRootSignature(m_rootSignature.Get());
            commandList->SetComputeRootDescriptorTable(Slot::Input, inputResourceHandle);
            commandList->SetComputeRootDescriptorTable(Slot::InputLowResNormal, inputLowResNormalResourceHandle);
            commandList->SetComputeRootDescriptorTable(Slot::InputHiResNormal, inputHiResNormalResourceHandle);
            commandList->SetComputeRootDescriptorTable(Slot::InputHiResPartialDistanceDerivative, inputHiResPartialDistanceDerivativeResourceHandle);
            commandList->SetComputeRootDescriptorTable(Slot::Output, outputResourceHandle);
            commandList->SetComputeRootConstantBufferView(Slot::ConstantBuffer, m_CB.GpuVirtualAddress(m_CBinstanceID));

            GpuResource* debugResources = Sample::g_debugOutput;
            commandList->SetComputeRootDescriptorTable(Slot::Debug1, debugResources[0].gpuDescriptorWriteAccess);
            commandList->SetComputeRootDescriptorTable(Slot::Debug2, debugResources[1].gpuDescriptorWriteAccess);

            commandList->SetPipelineState(m_pipelineStateObjects[type].Get());
        }

        // Start from -1,-1 pixel to account for high-res pixel border around low-res pixel border.
        XMUINT2 groupSize(CeilDivide(lowResDim.x + 1, ThreadGroup::Width), CeilDivide(lowResDim.y + 1, ThreadGroup::Height));

        // Dispatch.
        commandList->Dispatch(groupSize.x, groupSize.y, 1);
    }

    namespace RootSignature {
        namespace CalculatePartialDerivatives {
            namespace Slot {
                enum Enum {
                    Output = 0,
                    Input,
                    ConstantBuffer,
                    Count
                };
            }
        }
    }

    void CalculatePartialDerivatives::Initialize(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame)
    {
        // Create root signature.
        {
            using namespace RootSignature::CalculatePartialDerivatives;

            CD3DX12_DESCRIPTOR_RANGE ranges[Slot::Count];
            ranges[Slot::Input].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);  // input values
            ranges[Slot::Output].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // output filtered values

            CD3DX12_ROOT_PARAMETER rootParameters[Slot::Count];
            rootParameters[Slot::Input].InitAsDescriptorTable(1, &ranges[Slot::Input]);
            rootParameters[Slot::Output].InitAsDescriptorTable(1, &ranges[Slot::Output]);
            rootParameters[Slot::ConstantBuffer].InitAsConstantBufferView(0);

            CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
            SerializeAndCreateRootSignature(device, rootSignatureDesc, &m_rootSignature, L"Compute root signature: CalculatePartialDerivatives");
        }

        // Create compute pipeline state.
        {
            D3D12_COMPUTE_PIPELINE_STATE_DESC descComputePSO = {};
            descComputePSO.pRootSignature = m_rootSignature.Get();
            descComputePSO.CS = CD3DX12_SHADER_BYTECODE(static_cast<const void*>(g_pCalculatePartialDerivativesViaCentralDifferencesCS), ARRAYSIZE(g_pCalculatePartialDerivativesViaCentralDifferencesCS));
            ThrowIfFailed(device->CreateComputePipelineState(&descComputePSO, IID_PPV_ARGS(&m_pipelineStateObject)));
            m_pipelineStateObject->SetName(L"Pipeline state object: CalculatePartialDerivatives");
        }

        // Create shader resources.
        {
            m_CB.Create(device, frameCount * numCallsPerFrame, L"Constant Buffer: CalculatePartialDerivatives");
        }
    }

    // Expects, and returns, outputResource in D3D12_RESOURCE_STATE_UNORDERED_ACCESS state.
    void CalculatePartialDerivatives::Run(
        ID3D12GraphicsCommandList4* commandList,
        ID3D12DescriptorHeap* descriptorHeap,
        UINT width,
        UINT height,
        D3D12_GPU_DESCRIPTOR_HANDLE inputValuesResourceHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE outputResourceHandle)
    {
        using namespace RootSignature::CalculatePartialDerivatives;
        using namespace DefaultComputeShaderParams;

        ScopedTimer _prof(L"CalculatePartialDerivatives", commandList);

        // Set pipeline state.
        {
            commandList->SetDescriptorHeaps(1, &descriptorHeap);
            commandList->SetComputeRootSignature(m_rootSignature.Get());
            commandList->SetPipelineState(m_pipelineStateObject.Get());
            commandList->SetComputeRootDescriptorTable(Slot::Input, inputValuesResourceHandle);
            commandList->SetComputeRootDescriptorTable(Slot::Output, outputResourceHandle);
        }

        // Update the Constant Buffer.
        {
            m_CB->textureDim = XMUINT2(width, height);
            m_CBinstanceID = (m_CBinstanceID + 1) % m_CB.NumInstances();
            m_CB.CopyStagingToGpu(m_CBinstanceID);
            commandList->SetComputeRootConstantBufferView(Slot::ConstantBuffer, m_CB.GpuVirtualAddress(m_CBinstanceID));
        }

        // Dispatch.
        {
            XMUINT2 groupSize(CeilDivide(width, ThreadGroup::Width), CeilDivide(height, ThreadGroup::Height));
            commandList->Dispatch(groupSize.x, groupSize.y, 1);
        }
    }

    namespace RootSignature {
        namespace GenerateGrassPatch {
            namespace Slot {
                enum Enum {
                    OutputVB = 0,
                    InputWindMap,
                    ConstantBuffer,
                    Count
                };
            }
        }
    }

    void GenerateGrassPatch::Initialize(
        ID3D12Device5* device, 
        const wchar_t* windTexturePath,
        DX::DescriptorHeap* descriptorHeap,
        ResourceUploadBatch* resourceUpload, 
        UINT frameCount, 
        UINT numCallsPerFrame)
    {
        // Create root signature.
        {
            using namespace RootSignature::GenerateGrassPatch;

            CD3DX12_DESCRIPTOR_RANGE ranges[Slot::Count];
            ranges[Slot::InputWindMap].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);  // input wind texture
            ranges[Slot::OutputVB].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // output vertex buffer 

            CD3DX12_ROOT_PARAMETER rootParameters[Slot::Count];
            rootParameters[Slot::InputWindMap].InitAsDescriptorTable(1, &ranges[Slot::InputWindMap]);
            rootParameters[Slot::OutputVB].InitAsDescriptorTable(1, &ranges[Slot::OutputVB]);
            rootParameters[Slot::ConstantBuffer].InitAsConstantBufferView(0);

            CD3DX12_STATIC_SAMPLER_DESC staticWrapLinearSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

            CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters, 1, &staticWrapLinearSampler);
            SerializeAndCreateRootSignature(device, rootSignatureDesc, &m_rootSignature, L"Compute root signature: GenerateGrassPatch");
        }

        // Create compute pipeline state.
        {
            D3D12_COMPUTE_PIPELINE_STATE_DESC descComputePSO = {};
            descComputePSO.pRootSignature = m_rootSignature.Get();
            descComputePSO.CS = CD3DX12_SHADER_BYTECODE(static_cast<const void*>(g_pGenerateGrassStrawsCS), ARRAYSIZE(g_pGenerateGrassStrawsCS));

            ThrowIfFailed(device->CreateComputePipelineState(&descComputePSO, IID_PPV_ARGS(&m_pipelineStateObject)));
            m_pipelineStateObject->SetName(L"Pipeline state object: GenerateGrassPatch");
        }

        // Create shader resources.
        {
            m_CB.Create(device, frameCount * numCallsPerFrame, L"Constant Buffer: GenerateGrassPatch");
        }

        // Load the wind texture.
        {
            LoadWICTexture(device, resourceUpload, windTexturePath, descriptorHeap, &m_windTexture.resource, &m_windTexture.heapIndex, &m_windTexture.cpuDescriptorHandle, &m_windTexture.gpuDescriptorHandle, false);
        };

    }

    // Expects, and returns, outputResource in D3D12_RESOURCE_STATE_UNORDERED_ACCESS state.
    void GenerateGrassPatch::Run(
        ID3D12GraphicsCommandList4* commandList,
        const GenerateGrassStrawsConstantBuffer_AppParams& appParams,
        ID3D12DescriptorHeap* descriptorHeap,
        D3D12_GPU_DESCRIPTOR_HANDLE outputVertexBufferResourceHandle)
    {
        using namespace RootSignature::GenerateGrassPatch;
        using namespace DefaultComputeShaderParams;
        
        ScopedTimer _prof(L"Generate Grass Patch", commandList);

        // Set pipeline state.
        {
            commandList->SetDescriptorHeaps(1, &descriptorHeap);
            commandList->SetComputeRootSignature(m_rootSignature.Get());
            commandList->SetPipelineState(m_pipelineStateObject.Get());
            commandList->SetComputeRootDescriptorTable(Slot::InputWindMap, m_windTexture.gpuDescriptorHandle);
            commandList->SetComputeRootDescriptorTable(Slot::OutputVB, outputVertexBufferResourceHandle);
        }

        // Update the Constant Buffer.
        m_CB->p = appParams;
        m_CB->invActivePatchDim = XMFLOAT2(1.f / appParams.activePatchDim.x, 1.f / appParams.activePatchDim.y);
        m_CB->p = appParams;
        m_CBinstanceID = (m_CBinstanceID + 1) % m_CB.NumInstances();
        m_CB.CopyStagingToGpu(m_CBinstanceID);
        commandList->SetComputeRootConstantBufferView(Slot::ConstantBuffer, m_CB.GpuVirtualAddress(m_CBinstanceID));

        // Dispatch.
        XMUINT2 dim = appParams.maxPatchDim;
        XMUINT2 groupSize(CeilDivide(dim.x, ThreadGroup::Width), CeilDivide(dim.y, ThreadGroup::Height));
        commandList->Dispatch(groupSize.x, groupSize.y, 1);
    }
}