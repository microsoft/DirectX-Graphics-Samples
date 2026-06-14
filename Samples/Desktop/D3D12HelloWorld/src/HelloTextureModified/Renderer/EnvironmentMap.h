#pragma once

#include "../DXSampleHelper.h"
#include "SimpleDescriptorHeapAllocator.h"

#include <wrl.h>

namespace Engine
{

class EnvironmentMap
{
public:
    void CreateFromDdsOrProceduralFallback(ID3D12Device* device,
                                           ID3D12GraphicsCommandList* commandList,
                                           SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                                           const wchar_t* ddsPath,
                                           ComPtr<ID3D12Resource>& uploadHeap);

    void CreateProceduralFallback(ID3D12Device* device,
                                  ID3D12GraphicsCommandList* commandList,
                                  SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                                  ComPtr<ID3D12Resource>& uploadHeap);

    DescriptorHeapHandle Srv() const
    {
        return m_srv;
    }

    ID3D12Resource* Resource() const
    {
        return m_resource.Get();
    }

private:
    bool TryCreateFromDds(ID3D12Device* device,
                          ID3D12GraphicsCommandList* commandList,
                          SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                          const wchar_t* ddsPath,
                          ComPtr<ID3D12Resource>& uploadHeap);

    DescriptorHeapHandle AllocateTextureCubeSRV(ID3D12Device* device,
                                                SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                                                ID3D12Resource* texture);

    ComPtr<ID3D12Resource> m_resource;
    DescriptorHeapHandle m_srv;
};

} // namespace Engine
