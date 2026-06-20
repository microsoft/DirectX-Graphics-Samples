#pragma once

#include "../DXSampleHelper.h"
#include "SimpleDescriptorHeapAllocator.h"

#include <DirectXMath.h>
#include <utility>
#include <vector>
#include <wrl.h>

namespace Engine
{

constexpr bool kUseGpuProceduralEnvMap = true;

struct HdrImage
{
    UINT width = 0;
    UINT height = 0;
    std::vector<DirectX::XMFLOAT3> pixels;
};

bool TryLoadHdrImage(const wchar_t* hdrPath, HdrImage& image);

enum class EnvironmentSource
{
    AssetHdr = 0,
    ProceduralStudio,
    ProceduralSun,
    ProceduralColorPanels,
    ProceduralHorizon,
};

struct ProceduralEnvironmentSettings
{
    EnvironmentSource source = EnvironmentSource::AssetHdr;
    DirectX::XMFLOAT3 skyColor = {0.42f, 0.56f, 0.72f};
    DirectX::XMFLOAT3 groundColor = {0.18f, 0.17f, 0.15f};
    DirectX::XMFLOAT3 lightColor = {1.0f, 0.96f, 0.86f};
    DirectX::XMFLOAT3 lightDirection = {0.35f, 0.75f, 0.25f};
    float backgroundIntensity = 0.6f;
    float lightIntensity = 6.0f;
    float lightSize = 0.12f;
    float fillIntensity = 0.12f;
    float colorPanelIntensity = 1.5f;
    float horizonSharpness = 0.08f;
};

class EnvironmentMap
{
public:
    bool TryCreateFromHdrEquirectangular(ID3D12Device* device,
                                         ID3D12GraphicsCommandList* commandList,
                                         SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                                         const HdrImage& image,
                                         UINT outputSize,
                                         bool createDiffuseIrradiance,
                                         ComPtr<ID3D12Resource>& uploadHeap);

    bool TryCreateSpecularPrefilterFromHdrEquirectangular(ID3D12Device* device,
                                                          ID3D12GraphicsCommandList* commandList,
                                                          SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                                                          const HdrImage& image,
                                                          UINT outputSize,
                                                          UINT mipCount,
                                                          ComPtr<ID3D12Resource>& uploadHeap);

    void CreateFromDdsOrProceduralFallback(ID3D12Device* device,
                                           ID3D12GraphicsCommandList* commandList,
                                           SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                                           const wchar_t* ddsPath,
                                           ComPtr<ID3D12Resource>& uploadHeap);

    void CreateProceduralFallback(ID3D12Device* device,
                                  ID3D12GraphicsCommandList* commandList,
                                  SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                                  ComPtr<ID3D12Resource>& uploadHeap);

    void CreateProcedural(ID3D12Device* device,
                          ID3D12GraphicsCommandList* commandList,
                          SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                          const ProceduralEnvironmentSettings& settings,
                          UINT outputSize,
                          bool createDiffuseIrradiance,
                          ComPtr<ID3D12Resource>& uploadHeap);

    void CreateSpecularPrefilterProcedural(ID3D12Device* device,
                                           ID3D12GraphicsCommandList* commandList,
                                           SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                                           const ProceduralEnvironmentSettings& settings,
                                           UINT outputSize,
                                           UINT mipCount,
                                           ComPtr<ID3D12Resource>& uploadHeap);

    void Release(SimpleDescriptorHeapAllocator& descriptorHeapAllocator);

    void Attach(ComPtr<ID3D12Resource> resource, DescriptorHeapHandle srv)
    {
        m_resource = std::move(resource);
        m_srv = srv;
    }

    void Detach(ComPtr<ID3D12Resource>& resource, DescriptorHeapHandle& srv)
    {
        resource = std::move(m_resource);
        srv = m_srv;
        m_srv = {};
    }

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
