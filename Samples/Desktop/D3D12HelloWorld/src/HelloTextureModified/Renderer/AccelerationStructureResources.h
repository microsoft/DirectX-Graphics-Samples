#pragma once

#include "../DXSampleHelper.h"
#include "../Scene/Scene.h"
#include "SimpleDescriptorHeapAllocator.h"

namespace Engine
{

struct AccelerationStructureResources
{
    ComPtr<ID3D12Resource> blas;
    ComPtr<ID3D12Resource> blasScratch;
    ComPtr<ID3D12Resource> tlas;
    ComPtr<ID3D12Resource> tlasScratch;
    DescriptorAllocation tlasSrv;

    void Build(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* commandList,
        ID3D12Resource* vertexBuffer,
        ID3D12Resource* indexBuffer,
        UINT vertexCountPerInstance,
        UINT indexCountPerInstance,
        bool usesIndexedDraw,
        const InstanceData* instances,
        UINT instanceCount,
        ID3D12Resource* tlasInstanceBuffer,
        SimpleDescriptorHeapAllocator& descriptorHeapAllocator);

    void RebuildTlas(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* commandList,
        const InstanceData* instances,
        UINT instanceCount,
        ID3D12Resource* tlasInstanceBuffer);

    void Release();
};

} // namespace Engine
