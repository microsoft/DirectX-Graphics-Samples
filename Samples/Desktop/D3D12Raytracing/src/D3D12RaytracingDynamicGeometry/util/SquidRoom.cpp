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
#include "DXSampleHelper.h"
#include "..\RaytracingSceneDefines.h"
#include "SquidRoom.h"

using namespace SquidRoomAssets;
using namespace std;

	// ToDo move this out as a helper
void SquidRoomAssets::LoadGeometry(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList,
	DescriptorHeap* descriptorHeap,
	LPCWSTR assetPath,
	D3DBuffer* vertexBuffer,
	ID3D12Resource* vertexBufferUpload,
	UINT* vertexBufferHeapIndex,
	D3DBuffer* indexBuffer,
	ID3D12Resource* indexBufferUpload,
	UINT* indexBufferHeapIndex,
	vector<GeometryInstance>* geometryInstances
)
{
	UINT fileSize = 0;
	UINT8* pAssetData = nullptr;

	auto ConvertRHtoLHGeometry = [&]()
	{
		VertexPositionNormalTextureTangent* vertices = reinterpret_cast<VertexPositionNormalTextureTangent*>(pAssetData + SquidRoomAssets::VertexDataOffset);
		UINT nVertices = SquidRoomAssets::VertexDataSize / sizeof(VertexPositionNormalTextureTangent);

		for (UINT i = 0; i < nVertices; i++)
		{
			auto& vertex = vertices[i];
			vertex.position.z = -vertex.position.z;
		}
	};

	auto DeactivateGeometry = [&](UINT geometryID)
	{
		auto& geometryDesc = SquidRoomAssets::Draws[geometryID];
		VertexPositionNormalTextureTangent* geometryVertices = reinterpret_cast<VertexPositionNormalTextureTangent*>(pAssetData + SquidRoomAssets::VertexDataOffset) + geometryDesc.VertexBase;
		UINT* geometryIndices = reinterpret_cast<UINT*>(pAssetData + SquidRoomAssets::IndexDataOffset) + geometryDesc.IndexStart;

		// Deactivate vertices by setting x-coordinate to NaN.
		for (UINT i = 0; i < geometryDesc.IndexCount; i++)
		{
			auto& vertex = geometryVertices[geometryIndices[i]];
			vertex.position.x = nanf("");
		}
	};

	// Load scene assets.
	ThrowIfFailed(ReadDataFromFile(assetPath, &pAssetData, &fileSize));
	ConvertRHtoLHGeometry();

	// Create the index buffer.
	{
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(SquidRoomAssets::IndexDataSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&indexBuffer->resource)));
		indexBuffer->resource->SetName(L"Index buffer: Squid room");

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(SquidRoomAssets::IndexDataSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&indexBufferUpload)));
		indexBufferUpload->SetName(L"Index buffer upload: Squid room");

		// Copy data to the upload heap and then schedule a copy 
		// from the upload heap to the index buffer.
		D3D12_SUBRESOURCE_DATA indexData = {};
		indexData.pData = pAssetData + SquidRoomAssets::IndexDataOffset;
		indexData.RowPitch = SquidRoomAssets::IndexDataSize;
		indexData.SlicePitch = indexData.RowPitch;

		PIXBeginEvent(commandList, 0, L"Copy index buffer data to default resource...");

		UpdateSubresources<1>(commandList, indexBuffer->resource.Get(), indexBufferUpload, 0, 0, 1, &indexData);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer->resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

		PIXEndEvent(commandList);

		UINT numElements = SquidRoomAssets::IndexDataSize / sizeof(UINT);
		UINT elementStride = 0;	// Using raw typeless buffer.
		CreateBufferSRV(indexBuffer, device, numElements, elementStride, descriptorHeap, indexBufferHeapIndex);

	}

	// Create the vertex buffer.
	{
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(SquidRoomAssets::VertexDataSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&vertexBuffer->resource)));
		vertexBuffer->resource->SetName( L"VertexPositionNormalTextureTangent buffer: Squid room");

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(SquidRoomAssets::VertexDataSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&vertexBufferUpload)));
		vertexBufferUpload->SetName(L"VertexPositionNormalTextureTangent buffer upload: Squid room");

		// Copy data to the upload heap and then schedule a copy 
		// from the upload heap to the vertex buffer.
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = pAssetData + SquidRoomAssets::VertexDataOffset;
		vertexData.RowPitch = SquidRoomAssets::VertexDataSize;
		vertexData.SlicePitch = vertexData.RowPitch;

		// Disable side panels on the squid container to make the squid visible.
		{
			const UINT sidePanelsGeometryID = 848;
			DeactivateGeometry(sidePanelsGeometryID);
		}

		PIXBeginEvent(commandList, 0, L"Copy vertex buffer data to default resource...");

		UpdateSubresources<1>(commandList, vertexBuffer->resource.Get(), vertexBufferUpload, 0, 0, 1, &vertexData);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer->resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		PIXEndEvent(commandList);

		UINT numElements = SquidRoomAssets::VertexDataSize / SquidRoomAssets::StandardVertexStride;
		UINT elementStride = SquidRoomAssets::StandardVertexStride;	
		CreateBufferSRV(vertexBuffer, device, numElements, elementStride, descriptorHeap, vertexBufferHeapIndex);
	}


	// ToDo remove
	// Backup primary handles
	struct Handles {
		D3D12_CPU_DESCRIPTOR_HANDLE cpu;
		D3D12_GPU_DESCRIPTOR_HANDLE gpu;
	};
	Handles ib = { indexBuffer->cpuDescriptorHandle, indexBuffer->gpuDescriptorHandle };
	Handles vb = { vertexBuffer->cpuDescriptorHandle, vertexBuffer->gpuDescriptorHandle };

	geometryInstances->resize(ARRAYSIZE(SquidRoomAssets::Draws));
	for (UINT i = 0; i < ARRAYSIZE(SquidRoomAssets::Draws); i++)
	{
		(*geometryInstances)[i].ib.startIndex = SquidRoomAssets::Draws[i].IndexStart;
		(*geometryInstances)[i].ib.count = SquidRoomAssets::Draws[i].IndexCount;

		(*geometryInstances)[i].vb.startIndex = SquidRoomAssets::Draws[i].VertexBase;
		(*geometryInstances)[i].vb.count = SquidRoomAssets::VertexDataSize / SquidRoomAssets::StandardVertexStride - SquidRoomAssets::Draws[i].VertexBase;

		UINT geometryIBHeapIndex = UINT_MAX;
		UINT geometryVBHeapIndex = UINT_MAX;
		CreateBufferSRV(indexBuffer, device, (*geometryInstances)[i].ib.count, sizeof(UINT), descriptorHeap, &geometryIBHeapIndex, (*geometryInstances)[i].ib.startIndex);
		CreateBufferSRV(vertexBuffer, device, (*geometryInstances)[i].vb.count, SquidRoomAssets::StandardVertexStride, descriptorHeap, &geometryVBHeapIndex, (*geometryInstances)[i].vb.startIndex);

		(*geometryInstances)[i].ib.gpuDescriptorHandle = indexBuffer->gpuDescriptorHandle;
		(*geometryInstances)[i].vb.gpuDescriptorHandle = vertexBuffer->gpuDescriptorHandle;
	}

	// Revert
	indexBuffer->cpuDescriptorHandle = ib.cpu;
	indexBuffer->gpuDescriptorHandle = ib.gpu;
	vertexBuffer->cpuDescriptorHandle = vb.cpu;
	vertexBuffer->gpuDescriptorHandle = vb.gpu;

	free(pAssetData);
}