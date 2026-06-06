#include "stdafx.h"

#include "GBuffer.h"

#include "FullscreenTriangle.h"

#include <pix3.h>

namespace Engine
{

void GBuffer::CreateResources(ID3D12Device* device, UINT width, UINT height)
{
    for (UINT i = 0; i < GBuffer::kCount; ++i)
    {
        resources[i].Reset();

        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width = width;
        desc.Height = height;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = formats[i];
        desc.SampleDesc.Count = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                      D3D12_HEAP_FLAG_NONE,
                                                      &desc,
                                                      D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                      &clearValues[i],
                                                      IID_PPV_ARGS(&resources[i])));
    }
}

void GBuffer::CreateRTVs(ID3D12Device* device,
                         ID3D12DescriptorHeap* rtvHeap,
                         UINT rtvBaseIndex,
                         UINT rtvDescriptorSize)
{
    for (UINT i = 0; i < GBuffer::kCount; ++i)
    {
        rtvIndex[i] = rtvBaseIndex + i;

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
            rtvHeap->GetCPUDescriptorHandleForHeapStart(), rtvIndex[i], rtvDescriptorSize);
        device->CreateRenderTargetView(resources[i].Get(), nullptr, rtvHandle);
    }
}

void GBuffer::CreateSRVs(ID3D12Device* device, SimpleDescriptorHeapAllocator& descriptorHeapAllocator)
{
    for (UINT i = 0; i < GBuffer::kCount; ++i)
    {
        if (srvHandles[i].Index == UINT_MAX)
        {
            srvHandles[i] = descriptorHeapAllocator.AllocWithHandle();
        }

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = formats[i];
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels = 1;

        device->CreateShaderResourceView(resources[i].Get(), &srvDesc, srvHandles[i].cpu);
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE GBuffer::GetRTV(ID3D12DescriptorHeap* rtvHeap, UINT rtvDescriptorSize, UINT index) const
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE h(rtvHeap->GetCPUDescriptorHandleForHeapStart());
    h.Offset(rtvIndex[index], rtvDescriptorSize);
    return h;
}

void RecordGBufferPass(ID3D12GraphicsCommandList* commandList, const GBufferPassDesc& passDesc)
{
    PIXBeginEvent(commandList, 0, L"GBufferPass");
    assert(passDesc.renderTargets.rtvs.size() == GBuffer::kCount);
    assert(passDesc.clearValues != nullptr);

    for (UINT i = 0; i < GBuffer::kCount; ++i)
    {
        commandList->ClearRenderTargetView(passDesc.renderTargets.rtvs[i], passDesc.clearValues[i].Color, 0, nullptr);
    }

    RecordSceneGeometryDraw(commandList, passDesc.geometryDraw);

    PIXEndEvent(commandList);
}

void RecordGBufferDebugPass(ID3D12GraphicsCommandList* commandList)
{
    PIXBeginEvent(commandList, 0, L"GBufferDebugPass");

    DrawFullscreenTriangle(commandList);

    PIXEndEvent(commandList);
}

} // namespace Engine
