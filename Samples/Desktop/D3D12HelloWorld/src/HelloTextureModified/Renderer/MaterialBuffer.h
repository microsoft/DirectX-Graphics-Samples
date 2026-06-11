#pragma once

#include "../DXSampleHelper.h"
#include "../MyDx12Utils.h"
#include "Material.h"
#include "SimpleDescriptorHeapAllocator.h"

#include <vector>

namespace Engine
{

class MaterialBuffer
{
public:
    void Create(ID3D12Device* device,
                SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                const std::vector<Material>& materials);
    void Update(const std::vector<Material>& materials);

    DescriptorHeapHandle Srv() const
    {
        return m_srv;
    }

private:
    ComPtr<ID3D12Resource> m_buffer;
    DescriptorHeapHandle m_srv;
};

} // namespace Engine
