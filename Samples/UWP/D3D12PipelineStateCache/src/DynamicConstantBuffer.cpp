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
#include "DynamicConstantBuffer.h"

static UINT Align(UINT location, UINT align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)
{
	return (location + (align - 1)) & ~(align - 1);
}

DynamicConstantBuffer::DynamicConstantBuffer(UINT constantSize, UINT maxDrawsPerFrame, UINT frameCount) :
	m_alignedPerDrawConstantBufferSize(Align(constantSize)),	// Constant buffers must be aligned for hardware requirements.
	m_maxDrawsPerFrame(maxDrawsPerFrame),
	m_frameCount(frameCount),
	m_constantBuffer(nullptr)
{
	m_perFrameConstantBufferSize = m_alignedPerDrawConstantBufferSize * m_maxDrawsPerFrame;
}

DynamicConstantBuffer::~DynamicConstantBuffer()
{
	m_constantBuffer->Unmap(0, nullptr);
}

void DynamicConstantBuffer::Init(ID3D12Device* pDevice)
{
	const UINT bufferSize = m_perFrameConstantBufferSize * m_frameCount;

	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_constantBuffer)
		));

	NAME_D3D12_OBJECT(m_constantBuffer);

	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pMappedConstantBuffer)));
}

void* DynamicConstantBuffer::GetMappedMemory(UINT drawIndex, UINT frameIndex)
{
	assert(drawIndex < m_maxDrawsPerFrame);
	UINT constantBufferOffset = (frameIndex * m_perFrameConstantBufferSize) + (drawIndex * m_alignedPerDrawConstantBufferSize);

	UINT8* temp = reinterpret_cast<UINT8*>(m_pMappedConstantBuffer);
	temp += constantBufferOffset;

	return temp;
}

D3D12_GPU_VIRTUAL_ADDRESS DynamicConstantBuffer::GetGpuVirtualAddress(UINT drawIndex, UINT frameIndex)
{
	UINT constantBufferOffset = (frameIndex * m_perFrameConstantBufferSize) + (drawIndex * m_alignedPerDrawConstantBufferSize);
	return m_constantBuffer->GetGPUVirtualAddress() + constantBufferOffset;
}
