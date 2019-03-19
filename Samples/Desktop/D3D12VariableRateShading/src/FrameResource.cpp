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
#include "SquidRoom.h"
#include "DXSampleHelper.h"

using namespace SceneEnums;

FrameResource::FrameResource(ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue)
    : m_gpuTimer(pDevice, pCommandQueue, 3)
{
    ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
    NAME_D3D12_OBJECT(m_commandAllocator);

    for (UINT i = 0; i < NumContexts; i++)
    {
        // Create command list allocators for worker threads. One per open command list.
        ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_contextCommandAllocators[i])));
        NAME_D3D12_OBJECT_INDEXED(m_contextCommandAllocators, i);
    }

    // Create constant buffers.
    {
        ThrowIfFailed(CreateConstantBuffer(pDevice, sizeof(SceneConstantBuffer), &m_constantBuffers[RenderPass::Shadow], nullptr, D3D12_RESOURCE_STATE_GENERIC_READ));
        NAME_D3D12_OBJECT(m_constantBuffers[RenderPass::Shadow]);
        
        // Scene render constant buffer.
        ThrowIfFailed(CreateConstantBuffer(pDevice, sizeof(SceneConstantBuffer), &m_constantBuffers[RenderPass::Scene], nullptr, D3D12_RESOURCE_STATE_GENERIC_READ));
        NAME_D3D12_OBJECT(m_constantBuffers[RenderPass::Scene]);

        // Postprocess pass constant buffer.
        ThrowIfFailed(CreateConstantBuffer(pDevice, sizeof(PostprocessConstantBuffer), &m_constantBuffers[RenderPass::Postprocess], nullptr, D3D12_RESOURCE_STATE_GENERIC_READ));
        NAME_D3D12_OBJECT(m_constantBuffers[RenderPass::Postprocess]);

        // Map the constant buffers and cache their heap pointers.
        // We don't unmap this until the app closes. Keeping buffer mapped for the lifetime of the resource is okay.
        const CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(m_constantBuffers[RenderPass::Shadow]->Map(0, &readRange, reinterpret_cast<void**>(&m_pConstantBuffersWO[RenderPass::Shadow])));
        ThrowIfFailed(m_constantBuffers[RenderPass::Scene]->Map(0, &readRange, reinterpret_cast<void**>(&m_pConstantBuffersWO[RenderPass::Scene])));
        ThrowIfFailed(m_constantBuffers[RenderPass::Postprocess]->Map(0, &readRange, reinterpret_cast<void**>(&m_pConstantBuffersWO[RenderPass::Postprocess])));
    }
}

FrameResource::~FrameResource()
{
}

void FrameResource::InitFrame()
{
    // Reset the main thread command allocator.
    m_commandAllocator->Reset();

    // Reset the worker command allocators.
    for (int i = 0; i < NumContexts; i++)
    {
        ThrowIfFailed(m_contextCommandAllocators[i]->Reset());
    }
}

void FrameResource::BeginFrame(ID3D12GraphicsCommandList* pCommandList)
{
    m_gpuTimer.BeginFrame(pCommandList);
    }

void FrameResource::EndFrame(ID3D12GraphicsCommandList* pCommandList)
{
    m_gpuTimer.EndFrame(pCommandList);
}