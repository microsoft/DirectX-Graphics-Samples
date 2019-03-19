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

#pragma once

#include "VariableRateShadingScene.h"
#include "FrameResource.h"

using namespace DirectX;
using namespace Microsoft::WRL;

class VariableRateShadingFrameResource : public FrameResource
{
public:
    ComPtr<ID3D12Resource> m_glassConstantBuffer;
    void* m_pGlassConstantBufferWO; // WRITE-ONLY pointer to the constant buffer.

public:
    VariableRateShadingFrameResource(ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue)
        : FrameResource(pDevice, pCommandQueue)
        , m_pGlassConstantBufferWO(nullptr)
    {
        // Create the glass constant buffer.
        {
            ThrowIfFailed(CreateConstantBuffer(pDevice, sizeof(GlassConstantBuffer), &m_glassConstantBuffer, nullptr, D3D12_RESOURCE_STATE_GENERIC_READ));
            NAME_D3D12_OBJECT(m_glassConstantBuffer);

            // Map the constant buffers and cache their heap pointers.
            // We don't unmap this until the app closes. Keeping buffer mapped for the lifetime of the resource is okay.
            const CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
            ThrowIfFailed(m_glassConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pGlassConstantBufferWO)));
        }
    }

    virtual ~VariableRateShadingFrameResource()
    {
    }

    inline D3D12_GPU_VIRTUAL_ADDRESS GetGlassConstantBufferGPUVirtualAddress() const
    {
        return m_glassConstantBuffer->GetGPUVirtualAddress();
    }
};