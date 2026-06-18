#pragma once

#include "../DXSampleHelper.h"
#include "SimpleDescriptorHeapAllocator.h"

namespace Engine
{

class BrdfLut
{
public:
    void Create(ID3D12Device* device,
                ID3D12GraphicsCommandList* commandList,
                SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                UINT size,
                ComPtr<ID3D12Resource>& uploadHeap);

    void Release(SimpleDescriptorHeapAllocator& descriptorHeapAllocator);

    DescriptorHeapHandle Srv() const
    {
        return m_srv;
    }

    ID3D12Resource* Resource() const
    {
        return m_resource.Get();
    }

private:
    ComPtr<ID3D12Resource> m_resource;
    DescriptorHeapHandle m_srv;
};

} // namespace Engine
