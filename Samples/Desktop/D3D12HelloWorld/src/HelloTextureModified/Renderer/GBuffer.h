#pragma once

#include "../DXSampleHelper.h"
#include "ResolvedRenderTargets.h"
#include "SceneGeometryPass.h"
#include "SimpleDescriptorHeapAllocator.h"

namespace Engine
{

struct GBuffer
{
    static constexpr UINT kCount = 5;

    enum Target : UINT
    {
        Albedo = 0,
        Normal = 1,
        Material = 2,
        MotionVector = 3,
        PBRParams = 4,
    };

    ComPtr<ID3D12Resource> resources[kCount];

    DXGI_FORMAT formats[kCount] = {
        DXGI_FORMAT_R8G8B8A8_UNORM,     // Albedo
        DXGI_FORMAT_R16G16B16A16_FLOAT, // Normal
        DXGI_FORMAT_R32_UINT,           // Material
        DXGI_FORMAT_R16G16_FLOAT,       // Motion Vector
        DXGI_FORMAT_R8G8B8A8_UNORM,     // PBR Params (Metallic, Roughness, Occlusion, Emissive)
    };

    D3D12_CLEAR_VALUE clearValues[kCount] = {
        {DXGI_FORMAT_R8G8B8A8_UNORM, {0.0f, 0.0f, 0.0f, 1.0f}},
        {DXGI_FORMAT_R16G16B16A16_FLOAT, {0.5f, 0.5f, 1.0f, 1.0f}},
        {DXGI_FORMAT_R32_UINT, {0.0f, 0.0f, 0.0f, 0.0f}},
        {DXGI_FORMAT_R16G16_FLOAT, {0.0f, 0.0f, 0.0f, 0.0f}},
        {DXGI_FORMAT_R8G8B8A8_UNORM, {0.0f, 0.0f, 0.0f, 0.0f}},
    };

    UINT rtvIndex[kCount] = {};
    DescriptorHeapHandle srvHandles[kCount];

    void CreateResources(ID3D12Device* device, UINT width, UINT height);
    void CreateRTVs(ID3D12Device* device,
                    ID3D12DescriptorHeap* rtvHeap,
                    UINT rtvBaseIndex,
                    UINT rtvDescriptorSize);
    void CreateSRVs(ID3D12Device* device, SimpleDescriptorHeapAllocator& descriptorHeapAllocator);
    D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(ID3D12DescriptorHeap* rtvHeap, UINT rtvDescriptorSize, UINT index) const;
};

struct GBufferPassDesc
{
    ResolvedRenderTargets renderTargets;
    const D3D12_CLEAR_VALUE* clearValues = nullptr;
    SceneGeometryDrawDesc geometryDraw = {};
};

void RecordGBufferPass(ID3D12GraphicsCommandList* commandList, const GBufferPassDesc& passDesc);
void RecordGBufferDebugPass(ID3D12GraphicsCommandList* commandList);

} // namespace Engine
