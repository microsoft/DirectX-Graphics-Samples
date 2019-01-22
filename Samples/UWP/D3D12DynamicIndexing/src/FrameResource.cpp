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
#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* pDevice, UINT cityRowCount, UINT cityColumnCount, UINT cityMaterialCount, float citySpacingInterval) :
	m_fenceValue(0),
	m_cityRowCount(cityRowCount),
	m_cityColumnCount(cityColumnCount),
	m_cityMaterialCount(cityMaterialCount)
{
	m_modelMatrices.resize(m_cityRowCount * m_cityColumnCount);

	// The command allocator is used by the main sample class when 
	// resetting the command list in the main update loop. Each frame 
	// resource needs a command allocator because command allocators 
	// cannot be reused until the GPU is done executing the commands 
	// associated with it.
	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(&m_bundleAllocator)));

	// Create an upload heap for the constant buffers.
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(SceneConstantBuffer) * m_cityRowCount * m_cityColumnCount),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_cbvUploadHeap)));

	// Map the constant buffers. Note that unlike D3D11, the resource 
	// does not need to be unmapped for use by the GPU. In this sample, 
	// the resource stays 'permenantly' mapped to avoid overhead with 
	// mapping/unmapping each frame.
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_cbvUploadHeap->Map(0, &readRange, reinterpret_cast<void**>(&m_pConstantBuffers)));

	// Update all of the model matrices once; our cities don't move so 
	// we don't need to do this ever again.
	SetCityPositions(citySpacingInterval, -citySpacingInterval);
}

FrameResource::~FrameResource()
{
	m_cbvUploadHeap->Unmap(0, nullptr);
	m_pConstantBuffers = nullptr;
}

void FrameResource::InitBundle(ID3D12Device* pDevice, ID3D12PipelineState* pPso,
	UINT frameResourceIndex, UINT numIndices, D3D12_INDEX_BUFFER_VIEW* pIndexBufferViewDesc, D3D12_VERTEX_BUFFER_VIEW* pVertexBufferViewDesc,
	ID3D12DescriptorHeap* pCbvSrvDescriptorHeap, UINT cbvSrvDescriptorSize, ID3D12DescriptorHeap* pSamplerDescriptorHeap, ID3D12RootSignature* pRootSignature)
{
	ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, m_bundleAllocator.Get(), pPso, IID_PPV_ARGS(&m_bundle)));
	NAME_D3D12_OBJECT(m_bundle);

	PopulateCommandList(m_bundle.Get(), pPso, frameResourceIndex, numIndices, pIndexBufferViewDesc,
		pVertexBufferViewDesc, pCbvSrvDescriptorHeap, cbvSrvDescriptorSize, pSamplerDescriptorHeap, pRootSignature);

	ThrowIfFailed(m_bundle->Close());
}

void FrameResource::SetCityPositions(FLOAT intervalX, FLOAT intervalZ)
{
	for (UINT i = 0; i < m_cityRowCount; i++)
	{
		FLOAT cityOffsetZ = i * intervalZ;
		for (UINT j = 0; j < m_cityColumnCount; j++)
		{
			FLOAT cityOffsetX = j * intervalX;

			// The y position is based off of the city's row and column 
			// position to prevent z-fighting.
			XMStoreFloat4x4(&m_modelMatrices[i * m_cityColumnCount + j], XMMatrixTranslation(cityOffsetX, 0.02f * (i * m_cityColumnCount + j), cityOffsetZ));
		}
	}
}

void FrameResource::PopulateCommandList(ID3D12GraphicsCommandList* pCommandList, ID3D12PipelineState* pPso,
	UINT frameResourceIndex, UINT numIndices, D3D12_INDEX_BUFFER_VIEW* pIndexBufferViewDesc, D3D12_VERTEX_BUFFER_VIEW* pVertexBufferViewDesc,
	ID3D12DescriptorHeap* pCbvSrvDescriptorHeap, UINT cbvSrvDescriptorSize, ID3D12DescriptorHeap* pSamplerDescriptorHeap, ID3D12RootSignature* pRootSignature)
{
	// If the root signature matches the root signature of the caller, then
	// bindings are inherited, otherwise the bind space is reset.
	pCommandList->SetGraphicsRootSignature(pRootSignature);

	ID3D12DescriptorHeap* ppHeaps[] = { pCbvSrvDescriptorHeap, pSamplerDescriptorHeap };
	pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCommandList->IASetIndexBuffer(pIndexBufferViewDesc);
	pCommandList->IASetVertexBuffers(0, 1, pVertexBufferViewDesc);
	pCommandList->SetGraphicsRootDescriptorTable(0, pCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	pCommandList->SetGraphicsRootDescriptorTable(1, pSamplerDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	// Calculate the descriptor offset due to multiple frame resources.
	// (m_cityMaterialCount + 1) SRVs + how many CBVs we have currently.
	UINT frameResourceDescriptorOffset = (m_cityMaterialCount + 1) + (frameResourceIndex * m_cityRowCount * m_cityColumnCount);
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvHandle(pCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), frameResourceDescriptorOffset, cbvSrvDescriptorSize);

	PIXBeginEvent(pCommandList, 0, L"Draw cities");
	for (UINT i = 0; i < m_cityRowCount; i++)
	{
		for (UINT j = 0; j < m_cityColumnCount; j++)
		{
			pCommandList->SetPipelineState(pPso);

			// Set the city's root constant for dynamically indexing into the material array.
			pCommandList->SetGraphicsRoot32BitConstant(3, (i * m_cityColumnCount) + j, 0);

			// Set this city's CBV table and move to the next descriptor.
			pCommandList->SetGraphicsRootDescriptorTable(2, cbvSrvHandle);
			cbvSrvHandle.Offset(cbvSrvDescriptorSize);

			pCommandList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);
		}
	}
	PIXEndEvent(pCommandList);
}

void XM_CALLCONV FrameResource::UpdateConstantBuffers(FXMMATRIX view, CXMMATRIX projection)
{
	XMMATRIX model;
	XMFLOAT4X4 mvp;

	for (UINT i = 0; i < m_cityRowCount; i++)
	{
		for (UINT j = 0; j < m_cityColumnCount; j++)
		{
			model = XMLoadFloat4x4(&m_modelMatrices[i * m_cityColumnCount + j]);

			// Compute the model-view-projection matrix.
			XMStoreFloat4x4(&mvp, XMMatrixTranspose(model * view * projection));

			// Copy this matrix into the appropriate location in the upload heap subresource.
			memcpy(&m_pConstantBuffers[i * m_cityColumnCount + j], &mvp, sizeof(mvp));
		}
	}
}
