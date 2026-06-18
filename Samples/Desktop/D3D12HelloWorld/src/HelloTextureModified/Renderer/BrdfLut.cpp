#include "stdafx.h"

#include "BrdfLut.h"

#include <DirectXPackedVector.h>
#include <algorithm>
#include <cmath>
#include <vector>

namespace Engine
{
namespace
{

using DirectX::PackedVector::XMHALF2;

float RadicalInverseVdC(UINT bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return static_cast<float>(bits) * 2.3283064365386963e-10f;
}

DirectX::XMFLOAT2 Hammersley(UINT i, UINT n)
{
    return {static_cast<float>(i) / static_cast<float>(n), RadicalInverseVdC(i)};
}

DirectX::XMFLOAT3 Normalize(const DirectX::XMFLOAT3& v)
{
    const float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return {v.x / length, v.y / length, v.z / length};
}

float Dot(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

DirectX::XMFLOAT3 ImportanceSampleGGX(const DirectX::XMFLOAT2& xi, float roughness)
{
    static constexpr float kPi = 3.1415926535f;
    const float a = roughness * roughness;
    const float phi = 2.0f * kPi * xi.x;
    const float cosTheta = std::sqrt((1.0f - xi.y) / (1.0f + (a * a - 1.0f) * xi.y));
    const float sinTheta = std::sqrt((std::max)(0.0f, 1.0f - cosTheta * cosTheta));

    return {std::cos(phi) * sinTheta, std::sin(phi) * sinTheta, cosTheta};
}

float GeometrySchlickGGX(float ndotv, float roughness)
{
    const float a = roughness;
    const float k = (a * a) / 2.0f;
    return ndotv / (ndotv * (1.0f - k) + k);
}

float GeometrySmith(float ndotv, float ndotl, float roughness)
{
    return GeometrySchlickGGX(ndotv, roughness) * GeometrySchlickGGX(ndotl, roughness);
}

DirectX::XMFLOAT2 IntegrateBrdf(float ndotv, float roughness)
{
    static constexpr UINT kSampleCount = 1024;

    const float sinTheta = std::sqrt((std::max)(0.0f, 1.0f - ndotv * ndotv));
    const DirectX::XMFLOAT3 viewDir = {sinTheta, 0.0f, ndotv};
    float scale = 0.0f;
    float bias = 0.0f;

    for (UINT i = 0; i < kSampleCount; ++i)
    {
        const DirectX::XMFLOAT2 xi = Hammersley(i, kSampleCount);
        const DirectX::XMFLOAT3 halfVector = ImportanceSampleGGX(xi, roughness);
        const float vdoth = (std::max)(Dot(viewDir, halfVector), 0.0f);
        DirectX::XMFLOAT3 lightDir = {
            2.0f * vdoth * halfVector.x - viewDir.x,
            2.0f * vdoth * halfVector.y - viewDir.y,
            2.0f * vdoth * halfVector.z - viewDir.z};
        lightDir = Normalize(lightDir);

        const float ndotl = (std::max)(lightDir.z, 0.0f);
        const float ndoth = (std::max)(halfVector.z, 0.0f);
        if (ndotl > 0.0f)
        {
            const float geometry = GeometrySmith(ndotv, ndotl, roughness);
            const float geometryVisible = geometry * vdoth / ((std::max)(ndoth, 0.0001f) * (std::max)(ndotv, 0.0001f));
            const float fresnel = std::pow(1.0f - vdoth, 5.0f);
            scale += (1.0f - fresnel) * geometryVisible;
            bias += fresnel * geometryVisible;
        }
    }

    const float invSampleCount = 1.0f / static_cast<float>(kSampleCount);
    return {scale * invSampleCount, bias * invSampleCount};
}

} // namespace

void BrdfLut::Create(ID3D12Device* device,
                     ID3D12GraphicsCommandList* commandList,
                     SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                     UINT size,
                     ComPtr<ID3D12Resource>& uploadHeap)
{
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
    textureDesc.Width = size;
    textureDesc.Height = size;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                  D3D12_HEAP_FLAG_NONE,
                                                  &textureDesc,
                                                  D3D12_RESOURCE_STATE_COPY_DEST,
                                                  nullptr,
                                                  IID_PPV_ARGS(&m_resource)));

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_resource.Get(), 0, 1);
    ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                  D3D12_HEAP_FLAG_NONE,
                                                  &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                                                  D3D12_RESOURCE_STATE_GENERIC_READ,
                                                  nullptr,
                                                  IID_PPV_ARGS(&uploadHeap)));

    std::vector<XMHALF2> pixels(static_cast<size_t>(size) * size);
    for (UINT y = 0; y < size; ++y)
    {
        const float roughness = (static_cast<float>(y) + 0.5f) / static_cast<float>(size);
        for (UINT x = 0; x < size; ++x)
        {
            const float ndotv = (static_cast<float>(x) + 0.5f) / static_cast<float>(size);
            const DirectX::XMFLOAT2 brdf = IntegrateBrdf(ndotv, roughness);
            pixels[static_cast<size_t>(y) * size + x] = XMHALF2(brdf.x, brdf.y);
        }
    }

    D3D12_SUBRESOURCE_DATA subresource = {};
    subresource.pData = pixels.data();
    subresource.RowPitch = size * sizeof(XMHALF2);
    subresource.SlicePitch = subresource.RowPitch * size;

    UpdateSubresources(commandList, m_resource.Get(), uploadHeap.Get(), 0, 0, 1, &subresource);
    commandList->ResourceBarrier(1,
                                 &CD3DX12_RESOURCE_BARRIER::Transition(m_resource.Get(),
                                                                       D3D12_RESOURCE_STATE_COPY_DEST,
                                                                       D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    if (!m_srv.IsValid())
    {
        m_srv = descriptorHeapAllocator.AllocWithHandle();
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    device->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_srv.cpu);
}

void BrdfLut::Release(SimpleDescriptorHeapAllocator& descriptorHeapAllocator)
{
    m_resource.Reset();
    if (m_srv.IsValid())
    {
        descriptorHeapAllocator.Free(m_srv);
        m_srv = {};
    }
}

} // namespace Engine
