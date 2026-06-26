#include "stdafx.h"

#include "AccelerationStructureResources.h"

#include <d3dx12_core.h>
#include <d3dx12_resource_helpers.h>
#include <pix3.h>

namespace Engine
{
namespace
{

bool QueryDevice5(ID3D12Device* device, Microsoft::WRL::ComPtr<ID3D12Device5>& device5)
{
    if (device == nullptr)
    {
        return false;
    }

    return SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&device5)));
}

D3D12_RAYTRACING_GEOMETRY_DESC CreateTriangleGeometryDesc(
    ID3D12Resource* vertexBuffer,
    ID3D12Resource* indexBuffer,
    UINT vertexCount,
    UINT indexCount,
    bool usesIndexedDraw)
{
    D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
    geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
    geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    geometryDesc.Triangles.VertexCount = vertexCount;
    geometryDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer->GetGPUVirtualAddress();
    geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(SceneVertex);

    if (usesIndexedDraw && indexBuffer != nullptr && indexCount > 0)
    {
        geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
        geometryDesc.Triangles.IndexCount = indexCount;
        geometryDesc.Triangles.IndexBuffer = indexBuffer->GetGPUVirtualAddress();
    }
    else
    {
        geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_UNKNOWN;
        geometryDesc.Triangles.IndexCount = 0;
    }

    return geometryDesc;
}

D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS CreateBlasInputs(
    const D3D12_RAYTRACING_GEOMETRY_DESC& geometryDesc)
{
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    inputs.NumDescs = 1;
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.pGeometryDescs = &geometryDesc;
    return inputs;
}

D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO GetPrebuildInfo(
    ID3D12Device5* device5,
    const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& inputs)
{
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = {};
    device5->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);
    return prebuildInfo;
}

Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(
    ID3D12Device* device,
    UINT64 sizeInBytes,
    D3D12_HEAP_TYPE heapType,
    D3D12_RESOURCE_STATES initialState,
    D3D12_RESOURCE_FLAGS flags,
    const wchar_t* name)
{
    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    CD3DX12_HEAP_PROPERTIES heapProperties(heapType);
    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes, flags);
    ThrowIfFailed(device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        initialState,
        nullptr,
        IID_PPV_ARGS(&resource)));
    resource->SetName(name);
    return resource;
}

void AddUavBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource)
{
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.UAV.pResource = resource;
    commandList->ResourceBarrier(1, &barrier);
}

void FillTlasInstanceDescs(
    D3D12_RAYTRACING_INSTANCE_DESC* outDescs,
    ID3D12Resource* blas,
    const InstanceData* instances,
    UINT instanceCount)
{
    const D3D12_GPU_VIRTUAL_ADDRESS blasAddress = blas->GetGPUVirtualAddress();

    for (UINT i = 0; i < instanceCount; ++i)
    {
        D3D12_RAYTRACING_INSTANCE_DESC& desc = outDescs[i];
        ZeroMemory(&desc, sizeof(desc));

        desc.InstanceID = i;
        desc.InstanceMask = 0xFF;
        desc.InstanceContributionToHitGroupIndex = 0;
        desc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
        desc.AccelerationStructure = blasAddress;

        // InstanceData::world is stored as XMMatrixTranspose(M) where M = S*R*T (row-major).
        // XMMatrixTranspose rearranges M._ij to XMFLOAT4X4._ji:
        //   world._11=M._11 world._12=M._21 world._13=M._31 world._14=M._41  (translation x)
        //   world._21=M._12 world._22=M._22 world._23=M._32 world._24=M._42  (translation y)
        //   world._31=M._13 world._32=M._23 world._33=M._33 world._34=M._43  (translation z)
        //   world._41=M._14 world._42=M._24 world._43=M._34 world._44=M._44
        // TLAS expects a row-major 3x4 object-to-world transform:
        //   row 0: M._11 M._12 M._13 tx   row 1: M._21 M._22 M._23 ty   row 2: M._31 M._32 M._33 tz
        const float* src = reinterpret_cast<const float*>(&instances[i].world);
        desc.Transform[0][0] = src[0];
        desc.Transform[0][1] = src[4];
        desc.Transform[0][2] = src[8];
        desc.Transform[0][3] = src[3];
        desc.Transform[1][0] = src[1];
        desc.Transform[1][1] = src[5];
        desc.Transform[1][2] = src[9];
        desc.Transform[1][3] = src[7];
        desc.Transform[2][0] = src[2];
        desc.Transform[2][1] = src[6];
        desc.Transform[2][2] = src[10];
        desc.Transform[2][3] = src[11];
    }
}

void CreateTlasSrv(
    ID3D12Device* device,
    ID3D12Resource* tlas,
    DescriptorAllocation& tlasSrv,
    SimpleDescriptorHeapAllocator& descriptorHeapAllocator)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.RaytracingAccelerationStructure.Location = tlas->GetGPUVirtualAddress();

    tlasSrv = descriptorHeapAllocator.Allocate();
    device->CreateShaderResourceView(nullptr, &srvDesc, tlasSrv.Cpu());
}

} // namespace

void AccelerationStructureResources::Build(
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
    SimpleDescriptorHeapAllocator& descriptorHeapAllocator)
{
    PIXBeginEvent(commandList, 0, L"BuildAccelerationStructures");

    Microsoft::WRL::ComPtr<ID3D12Device5> device5;
    if (!QueryDevice5(device, device5))
    {
        PIXEndEvent(commandList);
        return;
    }

    if (vertexBuffer == nullptr || instanceCount == 0 || vertexCountPerInstance == 0)
    {
        PIXEndEvent(commandList);
        return;
    }

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> commandList4;
    ThrowIfFailed(commandList->QueryInterface(IID_PPV_ARGS(&commandList4)));

    const D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = CreateTriangleGeometryDesc(
        vertexBuffer,
        indexBuffer,
        vertexCountPerInstance,
        indexCountPerInstance,
        usesIndexedDraw);
    const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS blasInputs = CreateBlasInputs(geometryDesc);
    const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO blasPrebuildInfo =
        GetPrebuildInfo(device5.Get(), blasInputs);

    blasScratch = CreateBufferResource(
        device,
        blasPrebuildInfo.ScratchDataSizeInBytes,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        L"BLAS Scratch");
    blas = CreateBufferResource(
        device,
        blasPrebuildInfo.ResultDataMaxSizeInBytes,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        L"BLAS");

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC blasBuildDesc = {};
    blasBuildDesc.Inputs = blasInputs;
    blasBuildDesc.DestAccelerationStructureData = blas->GetGPUVirtualAddress();
    blasBuildDesc.ScratchAccelerationStructureData = blasScratch->GetGPUVirtualAddress();
    commandList4->BuildRaytracingAccelerationStructure(&blasBuildDesc, 0, nullptr);
    AddUavBarrier(commandList, blas.Get());

    UINT8* mappedData = nullptr;
    CD3DX12_RANGE readRange(0, 0);
    ThrowIfFailed(tlasInstanceBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedData)));
    FillTlasInstanceDescs(
        reinterpret_cast<D3D12_RAYTRACING_INSTANCE_DESC*>(mappedData),
        blas.Get(),
        instances,
        instanceCount);
    tlasInstanceBuffer->Unmap(0, nullptr);

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlasInputs = {};
    tlasInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    tlasInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    tlasInputs.NumDescs = instanceCount;
    tlasInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    tlasInputs.InstanceDescs = tlasInstanceBuffer->GetGPUVirtualAddress();

    const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO tlasPrebuildInfo =
        GetPrebuildInfo(device5.Get(), tlasInputs);
    tlasScratch = CreateBufferResource(
        device,
        tlasPrebuildInfo.ScratchDataSizeInBytes,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        L"TLAS Scratch");
    tlas = CreateBufferResource(
        device,
        tlasPrebuildInfo.ResultDataMaxSizeInBytes,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        L"TLAS");

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasBuildDesc = {};
    tlasBuildDesc.Inputs = tlasInputs;
    tlasBuildDesc.DestAccelerationStructureData = tlas->GetGPUVirtualAddress();
    tlasBuildDesc.ScratchAccelerationStructureData = tlasScratch->GetGPUVirtualAddress();
    commandList4->BuildRaytracingAccelerationStructure(&tlasBuildDesc, 0, nullptr);
    AddUavBarrier(commandList, tlas.Get());

    CreateTlasSrv(device, tlas.Get(), tlasSrv, descriptorHeapAllocator);

    PIXEndEvent(commandList);
}

void AccelerationStructureResources::RebuildTlas(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* commandList,
    const InstanceData* instances,
    UINT instanceCount,
    ID3D12Resource* tlasInstanceBuffer)
{
    if (blas == nullptr || tlas == nullptr || tlasScratch == nullptr || tlasInstanceBuffer == nullptr ||
        instanceCount == 0)
    {
        return;
    }

    Microsoft::WRL::ComPtr<ID3D12Device5> device5;
    if (!QueryDevice5(device, device5))
    {
        return;
    }

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> commandList4;
    ThrowIfFailed(commandList->QueryInterface(IID_PPV_ARGS(&commandList4)));

    UINT8* mappedData = nullptr;
    CD3DX12_RANGE readRange(0, 0);
    ThrowIfFailed(tlasInstanceBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedData)));
    FillTlasInstanceDescs(
        reinterpret_cast<D3D12_RAYTRACING_INSTANCE_DESC*>(mappedData),
        blas.Get(),
        instances,
        instanceCount);
    tlasInstanceBuffer->Unmap(0, nullptr);

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlasInputs = {};
    tlasInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    tlasInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    tlasInputs.NumDescs = instanceCount;
    tlasInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    tlasInputs.InstanceDescs = tlasInstanceBuffer->GetGPUVirtualAddress();

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasBuildDesc = {};
    tlasBuildDesc.Inputs = tlasInputs;
    tlasBuildDesc.DestAccelerationStructureData = tlas->GetGPUVirtualAddress();
    tlasBuildDesc.ScratchAccelerationStructureData = tlasScratch->GetGPUVirtualAddress();
    commandList4->BuildRaytracingAccelerationStructure(&tlasBuildDesc, 0, nullptr);
    AddUavBarrier(commandList, tlas.Get());

    // tlasSrv is stable because tlas resource is reused; no need to recreate SRV.
}

void AccelerationStructureResources::Release()
{
    tlasSrv.Reset();
    tlas.Reset();
    tlasScratch.Reset();
    blas.Reset();
    blasScratch.Reset();
}

} // namespace Engine
