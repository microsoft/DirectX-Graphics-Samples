#include "stdafx.h"

#include "EnvironmentMap.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <vector>

namespace Engine
{

namespace
{
static constexpr UINT kEnvironmentMapSize = 64;
static constexpr UINT kEnvironmentMapFaceCount = 6;
static constexpr UINT kTexturePixelSize = 4;
static constexpr UINT kDdsMagic = 0x20534444;
static constexpr UINT kDdsFourCcDx10 = 0x30315844;
static constexpr UINT kDdsResourceMiscTextureCube = 0x4;
static constexpr UINT kDdsDimensionTexture2D = 3;

using DirectX::XMFLOAT3;

struct DdsPixelFormat
{
    UINT size;
    UINT flags;
    UINT fourCC;
    UINT rgbBitCount;
    UINT rBitMask;
    UINT gBitMask;
    UINT bBitMask;
    UINT aBitMask;
};

struct DdsHeader
{
    UINT size;
    UINT flags;
    UINT height;
    UINT width;
    UINT pitchOrLinearSize;
    UINT depth;
    UINT mipMapCount;
    UINT reserved1[11];
    DdsPixelFormat pixelFormat;
    UINT caps;
    UINT caps2;
    UINT caps3;
    UINT caps4;
    UINT reserved2;
};

struct DdsHeaderDx10
{
    DXGI_FORMAT dxgiFormat;
    UINT resourceDimension;
    UINT miscFlag;
    UINT arraySize;
    UINT miscFlags2;
};

bool GetBlockCompressedLayout(DXGI_FORMAT format, UINT width, UINT height, UINT& rowPitch, UINT& numRows)
{
    UINT bytesPerBlock = 0;
    switch (format)
    {
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
            bytesPerBlock = 8;
            break;
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            bytesPerBlock = 16;
            break;
        default:
            return false;
    }

    const UINT blocksWide = (std::max)(1u, (width + 3u) / 4u);
    const UINT blocksHigh = (std::max)(1u, (height + 3u) / 4u);
    rowPitch = blocksWide * bytesPerBlock;
    numRows = blocksHigh;
    return true;
}

XMFLOAT3 Normalize(XMFLOAT3 v)
{
    const float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return {v.x / length, v.y / length, v.z / length};
}

XMFLOAT3 DirectionForCubeFace(UINT face, float u, float v)
{
    switch (face)
    {
        case 0:
            return {1.0f, v, -u};
        case 1:
            return {-1.0f, v, u};
        case 2:
            return {u, 1.0f, -v};
        case 3:
            return {u, -1.0f, v};
        case 4:
            return {u, v, 1.0f};
        default:
            return {-u, v, -1.0f};
    }
}

float Clamp01(float value)
{
    if (value < 0.0f)
    {
        return 0.0f;
    }
    if (value > 1.0f)
    {
        return 1.0f;
    }
    return value;
}

void WriteColor(UINT8* dst, XMFLOAT3 color)
{
    dst[0] = static_cast<UINT8>(Clamp01(color.x) * 255.0f);
    dst[1] = static_cast<UINT8>(Clamp01(color.y) * 255.0f);
    dst[2] = static_cast<UINT8>(Clamp01(color.z) * 255.0f);
    dst[3] = 255;
}
} // namespace

void EnvironmentMap::CreateFromDdsOrProceduralFallback(ID3D12Device* device,
                                                       ID3D12GraphicsCommandList* commandList,
                                                       SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                                                       const wchar_t* ddsPath,
                                                       ComPtr<ID3D12Resource>& uploadHeap)
{
    if (TryCreateFromDds(device, commandList, descriptorHeapAllocator, ddsPath, uploadHeap))
    {
        return;
    }

    CreateProceduralFallback(device, commandList, descriptorHeapAllocator, uploadHeap);
}

bool EnvironmentMap::TryCreateFromDds(ID3D12Device* device,
                                      ID3D12GraphicsCommandList* commandList,
                                      SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                                      const wchar_t* ddsPath,
                                      ComPtr<ID3D12Resource>& uploadHeap)
{
    if (ddsPath == nullptr || ddsPath[0] == L'\0' || GetFileAttributesW(ddsPath) == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    byte* ddsDataRaw = nullptr;
    UINT ddsDataSize = 0;
    if (FAILED(ReadDataFromFile(ddsPath, &ddsDataRaw, &ddsDataSize)))
    {
        return false;
    }

    std::unique_ptr<byte, decltype(&std::free)> ddsData(ddsDataRaw, std::free);
    if (ddsDataSize < sizeof(UINT) + sizeof(DdsHeader) + sizeof(DdsHeaderDx10))
    {
        return false;
    }

    const UINT magic = *reinterpret_cast<const UINT*>(ddsData.get());
    if (magic != kDdsMagic)
    {
        return false;
    }

    const DdsHeader* header = reinterpret_cast<const DdsHeader*>(ddsData.get() + sizeof(UINT));
    if (header->size != sizeof(DdsHeader) || header->pixelFormat.size != sizeof(DdsPixelFormat) ||
        header->pixelFormat.fourCC != kDdsFourCcDx10)
    {
        return false;
    }

    const DdsHeaderDx10* dx10Header =
        reinterpret_cast<const DdsHeaderDx10*>(ddsData.get() + sizeof(UINT) + sizeof(DdsHeader));
    if (dx10Header->resourceDimension != kDdsDimensionTexture2D ||
        (dx10Header->miscFlag & kDdsResourceMiscTextureCube) == 0 || dx10Header->arraySize == 0)
    {
        return false;
    }

    const UINT mipCount = (std::max)(1u, header->mipMapCount);
    const UINT cubeFaceCount = dx10Header->arraySize * 6u;
    const UINT subresourceCount = cubeFaceCount * mipCount;
    std::vector<D3D12_SUBRESOURCE_DATA> subresources(subresourceCount);

    const byte* textureData = ddsData.get() + sizeof(UINT) + sizeof(DdsHeader) + sizeof(DdsHeaderDx10);
    const byte* textureDataEnd = ddsData.get() + ddsDataSize;
    const byte* current = textureData;

    UINT subresourceIndex = 0;
    for (UINT arrayIndex = 0; arrayIndex < cubeFaceCount; ++arrayIndex)
    {
        for (UINT mip = 0; mip < mipCount; ++mip)
        {
            const UINT mipWidth = (std::max)(1u, header->width >> mip);
            const UINT mipHeight = (std::max)(1u, header->height >> mip);
            UINT rowPitch = 0;
            UINT numRows = 0;
            if (!GetBlockCompressedLayout(dx10Header->dxgiFormat, mipWidth, mipHeight, rowPitch, numRows))
            {
                return false;
            }

            const UINT slicePitch = rowPitch * numRows;
            if (current + slicePitch > textureDataEnd)
            {
                return false;
            }

            subresources[subresourceIndex].pData = current;
            subresources[subresourceIndex].RowPitch = rowPitch;
            subresources[subresourceIndex].SlicePitch = slicePitch;
            current += slicePitch;
            ++subresourceIndex;
        }
    }

    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = static_cast<UINT16>(mipCount);
    textureDesc.Format = dx10Header->dxgiFormat;
    textureDesc.Width = header->width;
    textureDesc.Height = header->height;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = static_cast<UINT16>(cubeFaceCount);
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                  D3D12_HEAP_FLAG_NONE,
                                                  &textureDesc,
                                                  D3D12_RESOURCE_STATE_COPY_DEST,
                                                  nullptr,
                                                  IID_PPV_ARGS(&m_resource)));

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_resource.Get(), 0, subresourceCount);
    ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                  D3D12_HEAP_FLAG_NONE,
                                                  &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                                                  D3D12_RESOURCE_STATE_GENERIC_READ,
                                                  nullptr,
                                                  IID_PPV_ARGS(&uploadHeap)));

    UpdateSubresources(commandList, m_resource.Get(), uploadHeap.Get(), 0, 0, subresourceCount, subresources.data());

    commandList->ResourceBarrier(1,
                                 &CD3DX12_RESOURCE_BARRIER::Transition(m_resource.Get(),
                                                                       D3D12_RESOURCE_STATE_COPY_DEST,
                                                                       D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    m_srv = AllocateTextureCubeSRV(device, descriptorHeapAllocator, m_resource.Get());
    DBG_PRINT("Loaded environment DDS: %ls\n", ddsPath);
    return true;
}

void EnvironmentMap::CreateProceduralFallback(ID3D12Device* device,
                                              ID3D12GraphicsCommandList* commandList,
                                              SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                                              ComPtr<ID3D12Resource>& uploadHeap)
{
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width = kEnvironmentMapSize;
    textureDesc.Height = kEnvironmentMapSize;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = kEnvironmentMapFaceCount;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                  D3D12_HEAP_FLAG_NONE,
                                                  &textureDesc,
                                                  D3D12_RESOURCE_STATE_COPY_DEST,
                                                  nullptr,
                                                  IID_PPV_ARGS(&m_resource)));

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_resource.Get(), 0, kEnvironmentMapFaceCount);
    ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                  D3D12_HEAP_FLAG_NONE,
                                                  &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                                                  D3D12_RESOURCE_STATE_GENERIC_READ,
                                                  nullptr,
                                                  IID_PPV_ARGS(&uploadHeap)));

    std::vector<std::vector<UINT8>> facePixels(kEnvironmentMapFaceCount);
    std::vector<D3D12_SUBRESOURCE_DATA> subresources(kEnvironmentMapFaceCount);
    const XMFLOAT3 sunDirection = Normalize({0.35f, 0.75f, 0.25f});

    for (UINT face = 0; face < kEnvironmentMapFaceCount; ++face)
    {
        facePixels[face].resize(kEnvironmentMapSize * kEnvironmentMapSize * kTexturePixelSize);
        for (UINT y = 0; y < kEnvironmentMapSize; ++y)
        {
            for (UINT x = 0; x < kEnvironmentMapSize; ++x)
            {
                const float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(kEnvironmentMapSize) * 2.0f - 1.0f;
                const float v = 1.0f - (static_cast<float>(y) + 0.5f) / static_cast<float>(kEnvironmentMapSize) * 2.0f;
                const XMFLOAT3 direction = Normalize(DirectionForCubeFace(face, u, v));
                const float skyBlend = Clamp01(direction.y * 0.5f + 0.5f);
                const float sunDot =
                    Clamp01(direction.x * sunDirection.x + direction.y * sunDirection.y + direction.z * sunDirection.z);
                const float sun = std::pow(sunDot, 96.0f);
                XMFLOAT3 color = {
                    0.03f + 0.38f * skyBlend + 0.65f * sun,
                    0.04f + 0.50f * skyBlend + 0.58f * sun,
                    0.05f + 0.72f * skyBlend + 0.42f * sun,
                };
                if (direction.y < -0.05f)
                {
                    const float groundBlend = Clamp01((-direction.y - 0.05f) / 0.95f);
                    color = {0.06f + 0.08f * groundBlend, 0.055f + 0.08f * groundBlend, 0.045f + 0.05f * groundBlend};
                }

                WriteColor(&facePixels[face][(y * kEnvironmentMapSize + x) * kTexturePixelSize], color);
            }
        }

        subresources[face].pData = facePixels[face].data();
        subresources[face].RowPitch = kEnvironmentMapSize * kTexturePixelSize;
        subresources[face].SlicePitch = subresources[face].RowPitch * kEnvironmentMapSize;
    }

    UpdateSubresources(commandList, m_resource.Get(), uploadHeap.Get(), 0, 0, kEnvironmentMapFaceCount, subresources.data());

    commandList->ResourceBarrier(1,
                                 &CD3DX12_RESOURCE_BARRIER::Transition(m_resource.Get(),
                                                                       D3D12_RESOURCE_STATE_COPY_DEST,
                                                                       D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    m_srv = AllocateTextureCubeSRV(device, descriptorHeapAllocator, m_resource.Get());
}

DescriptorHeapHandle EnvironmentMap::AllocateTextureCubeSRV(ID3D12Device* device,
                                                            SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                                                            ID3D12Resource* texture)
{
    DescriptorHeapHandle handle = descriptorHeapAllocator.AllocWithHandle();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texture->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.TextureCube.MipLevels = texture->GetDesc().MipLevels;
    device->CreateShaderResourceView(texture, &srvDesc, handle.cpu);

    return handle;
}

} // namespace Engine
