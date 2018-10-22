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

#include "../stdafx.h"
#include "PerformanceTimers.h"
#include "GpuKernels.h"
#include "CompiledShaders\ReduceSumCS.hlsl.h"

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

	void ReduceSum::Initialize(ID3D12Device* device)
	{
		// Create root signature.
		{
			using namespace RootSignature::ReduceSum;

			CD3DX12_DESCRIPTOR_RANGE ranges[2]; // Perfomance TIP: Order from most frequent to least frequent.
			ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);  // 1 input texture
			ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);  // 1 output texture

			CD3DX12_ROOT_PARAMETER rootParameters[Slot::Count];
			rootParameters[Slot::Input].InitAsDescriptorTable(1, &ranges[0]);
			rootParameters[Slot::Output].InitAsDescriptorTable(1, &ranges[1]);

			CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
			SerializeAndCreateRootSignature(device, rootSignatureDesc, &m_rootSignature, L"Compute root signature: ReduceSum");
		}

		// Create compute pipeline state.
		{
			D3D12_COMPUTE_PIPELINE_STATE_DESC descComputePSO = {};
			descComputePSO.pRootSignature = m_rootSignature.Get();
			descComputePSO.CS = CD3DX12_SHADER_BYTECODE((void *)g_pReduceSumCS, ARRAYSIZE(g_pReduceSumCS));

			ThrowIfFailed(device->CreateComputePipelineState(&descComputePSO, IID_PPV_ARGS(&m_pipelineStateObject)));
			m_pipelineStateObject->SetName(L"Pipeline state object: ReduceSum");
		}
	}

	void ReduceSum::CreateInputResourceSizeDependentResources(
		ID3D12Device* device,
		DescriptorHeap* descriptorHeap,
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

			m_csReduceSumOutputs.resize(numIterations);
			for (UINT i = 0; i < numIterations; i++)
			{
				width = max(1, CeilDivide(width, ReduceSumCS::ThreadGroup::Width));
				height = max(1, CeilDivide(height, ReduceSumCS::ThreadGroup::Height));

				m_csReduceSumOutputs[i].rwFlags = ResourceRWFlags::AllowWrite | ResourceRWFlags::AllowRead;
				CreateRenderTargetResource(device, DXGI_FORMAT_R32_UINT, width, height, descriptorHeap,
					&m_csReduceSumOutputs[i], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"UAV texture: ReduceSum intermediate output");
			}

			// ToDo should we allocate FrameCount + 1 in GPUTImeras we're depending on Present to stall?
			UINT bufferSize = sizeof(ResultType);
			m_readbackResources.resize(numInvocationsPerFrame);
			for (UINT i = 0; i < m_readbackResources.size(); i++)
				for (auto& readbackResource : m_readbackResources)
				{
					wstringstream wResourceName;
					wResourceName << L"Readback buffer - ReduceSum output [" << i << L"]";
					AllocateReadBackBuffer(device, frameCount * bufferSize, &m_readbackResources[i], D3D12_RESOURCE_STATE_COPY_DEST, wResourceName.str().c_str());
				}
		}
	}

	void ReduceSum::Execute(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* commandList,
		ID3D12DescriptorHeap* descriptorHeap, 
		UINT frameIndex,
		const D3D12_GPU_DESCRIPTOR_HANDLE& inputResourceHandle,
		UINT invocationIndex,
		ResultType* resultSum)
	{
		using namespace RootSignature::ReduceSum;
		
		PIXBeginEvent(commandList, 0, L"CalculateNumCameraRayHits");

		// Set pipeline state.
		{
			commandList->SetDescriptorHeaps(1, &descriptorHeap);
			commandList->SetComputeRootSignature(m_rootSignature.Get());
			commandList->SetComputeRootDescriptorTable(Slot::Output, m_csReduceSumOutputs[0].gpuDescriptorWriteAccess);
			commandList->SetPipelineState(m_pipelineStateObject.Get());
		}

		//
		// Iterative sum reduce [m_width, m_height] to [1,1]
		//
		SIZE_T readBackBaseOffset = frameIndex * sizeof(ResultType);
		{
			// First iteration reads from input resource.		
			commandList->SetComputeRootDescriptorTable(Slot::Input, inputResourceHandle);
			commandList->SetComputeRootDescriptorTable(Slot::Output, m_csReduceSumOutputs[0].gpuDescriptorWriteAccess);

			for (UINT i = 0; i < m_csReduceSumOutputs.size(); i++)
			{
				auto& outputResourceDesc = m_csReduceSumOutputs[i].resource.Get()->GetDesc();

				// Each group writes out a single summed result accross group threads.
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
			auto& destDesc = m_readbackResources[invocationIndex]->GetDesc();
			auto& srcDesc = m_csReduceSumOutputs.back().resource.Get()->GetDesc();
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT bufferFootprint = {};
			bufferFootprint.Offset = 0;
			bufferFootprint.Footprint.Width = static_cast<UINT>(destDesc.Width / sizeof(ResultType));
			ToDo check width is FrameCount;
			bufferFootprint.Footprint.Height = 1;
			bufferFootprint.Footprint.Depth = 1;
			bufferFootprint.Footprint.RowPitch = Align(static_cast<UINT>(destDesc.Width) * sizeof(ResultType), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
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
		// To avoid stalling on CPU until GPU is done, grab the data from finished frame FrameCount ago.
		// This is fine for the informational purposes of using the value for UI display only.
		UINT* mappedData = nullptr;
		CD3DX12_RANGE readRange(readBackBaseOffset, readBackBaseOffset + sizeof(ResultType));
		ThrowIfFailed(m_readbackResources[invocationIndex]->Map(0, &readRange, reinterpret_cast<void**>(&mappedData)));
		*resultSum = *mappedData;
		m_readbackResources[invocationIndex]->Unmap(0, &CD3DX12_RANGE(0, 0));

		PIXEndEvent(commandList);
	}
}