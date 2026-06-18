#include "stdafx.h"

#include "EnvironmentMap.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <rpcndr.h>
#include <malloc.h>
#include <combaseapi.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <fileapi.h>
#include <Windows.h>
#include <wrl\client.h>
#include "..\DXSampleHelper.h"
#include "..\MyDx12Utils.h"
#include "SimpleDescriptorHeapAllocator.h"
#include <d3d12.h>
#include <d3dx12_barriers.h>
#include <d3dx12_core.h>
#include <d3dx12_resource_helpers.h>
#include <dxgiformat.h>
#include <debugapi.h>
#include <cstdio>

namespace Engine
{

namespace
{
static constexpr UINT kEnvironmentMapSize = 64;
static constexpr UINT kEnvironmentMapFaceCount = 6;
static constexpr UINT kDdsMagic = 0x20534444;
static constexpr UINT kDdsFourCcDx10 = 0x30315844;
static constexpr UINT kDdsResourceMiscTextureCube = 0x4;
static constexpr UINT kDdsDimensionTexture2D = 3;

using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;
using DirectX::PackedVector::XMHALF4;

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

XMFLOAT3 Add(const XMFLOAT3& a, const XMFLOAT3& b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

XMFLOAT3 Multiply(const XMFLOAT3& a, float scale)
{
    return {a.x * scale, a.y * scale, a.z * scale};
}

XMFLOAT3 Cross(const XMFLOAT3& a, const XMFLOAT3& b)
{
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

float Dot(const XMFLOAT3& a, const XMFLOAT3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

XMFLOAT3 Lerp(const XMFLOAT3& a, const XMFLOAT3& b, float t)
{
    return {Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t)};
}

float SmoothStep(float edge0, float edge1, float x)
{
    const float t = Clamp01((x - edge0) / (edge1 - edge0));
    return t * t * (3.0f - 2.0f * t);
}

std::string ReadLine(const std::string& text, size_t& pos)
{
    if (pos >= text.size())
    {
        return {};
    }

    const size_t lineEnd = text.find('\n', pos);
    const size_t end = lineEnd == std::string::npos ? text.size() : lineEnd;
    const size_t length = end > pos && text[end - 1] == '\r' ? end - pos - 1 : end - pos;
    std::string line = text.substr(pos, length);
    pos = lineEnd == std::string::npos ? text.size() : lineEnd + 1;
    return line;
}

bool ParseHdrResolutionLine(const std::string& line, UINT& width, UINT& height)
{
    char ySign = 0;
    char yAxis = 0;
    char xSign = 0;
    char xAxis = 0;
    std::istringstream stream(line);
    if (!(stream >> ySign >> yAxis >> height >> xSign >> xAxis >> width))
    {
        return false;
    }

    return (yAxis == 'Y' || yAxis == 'y') && (xAxis == 'X' || xAxis == 'x') && (ySign == '+' || ySign == '-') &&
           (xSign == '+' || xSign == '-');
}

} // namespace

bool TryLoadHdrImage(const wchar_t* hdrPath, HdrImage& image)
{
    if (hdrPath == nullptr || hdrPath[0] == L'\0' || GetFileAttributesW(hdrPath) == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    byte* hdrDataRaw = nullptr;
    UINT hdrDataSize = 0;
    if (FAILED(ReadDataFromFile(hdrPath, &hdrDataRaw, &hdrDataSize)))
    {
        return false;
    }

    std::unique_ptr<byte, decltype(&std::free)> hdrData(hdrDataRaw, std::free);
    std::string file(reinterpret_cast<const char*>(hdrData.get()), hdrDataSize);
    size_t pos = 0;

    const std::string signature = ReadLine(file, pos);
    if (signature != "#?RADIANCE" && signature != "#?RGBE")
    {
        return false;
    }

    bool formatOk = false;
    while (pos < file.size())
    {
        const std::string line = ReadLine(file, pos);
        if (line.empty())
        {
            break;
        }
        if (line.rfind("FORMAT=32-bit_rle_rgbe", 0) == 0)
        {
            formatOk = true;
        }
    }

    if (!formatOk)
    {
        return false;
    }

    std::string resolutionLine;
    while (pos < file.size())
    {
        resolutionLine = ReadLine(file, pos);
        if (!resolutionLine.empty())
        {
            break;
        }
    }

    if (resolutionLine.empty())
    {
        return false;
    }

    if (!ParseHdrResolutionLine(resolutionLine, image.width, image.height) || image.width == 0 || image.height == 0)
    {
        return false;
    }

    image.pixels.assign(static_cast<size_t>(image.width) * static_cast<size_t>(image.height), {});

    const byte* current = reinterpret_cast<const byte*>(file.data()) + pos;
    const byte* end = reinterpret_cast<const byte*>(file.data()) + file.size();

    std::vector<UINT8> scanline(static_cast<size_t>(image.width) * 4);
    for (UINT y = 0; y < image.height; ++y)
    {
        if (current + 4 > end)
        {
            return false;
        }

        const UINT width = (static_cast<UINT>(current[2]) << 8u) | static_cast<UINT>(current[3]);
        if (current[0] != 2 || current[1] != 2 || width != image.width)
        {
            return false;
        }
        current += 4;

        for (UINT channel = 0; channel < 4; ++channel)
        {
            UINT x = 0;
            while (x < image.width)
            {
                if (current >= end)
                {
                    return false;
                }

                const UINT8 count = *current++;
                if (count > 128)
                {
                    const UINT runLength = count - 128;
                    if (current >= end || x + runLength > image.width)
                    {
                        return false;
                    }

                    const UINT8 value = *current++;
                    for (UINT i = 0; i < runLength; ++i)
                    {
                        scanline[channel * image.width + x + i] = value;
                    }
                    x += runLength;
                }
                else
                {
                    if (current + count > end || x + count > image.width)
                    {
                        return false;
                    }

                    for (UINT i = 0; i < count; ++i)
                    {
                        scanline[channel * image.width + x + i] = current[i];
                    }
                    current += count;
                    x += count;
                }
            }
        }

        for (UINT x = 0; x < image.width; ++x)
        {
            const UINT8 r = scanline[0 * image.width + x];
            const UINT8 g = scanline[1 * image.width + x];
            const UINT8 b = scanline[2 * image.width + x];
            const UINT8 e = scanline[3 * image.width + x];

            XMFLOAT3 color = {};
            if (e != 0)
            {
                const float scale = std::ldexp(1.0f, static_cast<int>(e) - (128 + 8));
                color = {static_cast<float>(r) * scale, static_cast<float>(g) * scale, static_cast<float>(b) * scale};
            }

            image.pixels[static_cast<size_t>(y) * image.width + x] = color;
        }
    }

    return true;
}

namespace
{

float RadicalInverseVdC(UINT bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return static_cast<float>(bits) * 2.3283064365386963e-10f;
}

XMFLOAT2 Hammersley(UINT i, UINT n)
{
    return {static_cast<float>(i) / static_cast<float>(n), RadicalInverseVdC(i)};
}

XMFLOAT3 Reflect(const XMFLOAT3& incident, const XMFLOAT3& normal)
{
    const float scale = 2.0f * Dot(incident, normal);
    return {incident.x - scale * normal.x, incident.y - scale * normal.y, incident.z - scale * normal.z};
}

XMFLOAT3 SampleEquirectangular(const HdrImage& image, XMFLOAT3 direction)
{
    direction = Normalize(direction);
    const float pi = 3.1415926535f;
    float u = std::atan2(direction.z, direction.x) / (2.0f * pi) + 0.5f;
    float v = std::acos((std::max)(-1.0f, (std::min)(1.0f, direction.y))) / pi;

    u = u - std::floor(u);
    v = Clamp01(v);

    const float x = u * static_cast<float>(image.width - 1);
    const float y = v * static_cast<float>(image.height - 1);
    const UINT x0 = static_cast<UINT>(x);
    const UINT y0 = static_cast<UINT>(y);
    const UINT x1 = (std::min)(x0 + 1, image.width - 1);
    const UINT y1 = (std::min)(y0 + 1, image.height - 1);
    const float tx = x - static_cast<float>(x0);
    const float ty = y - static_cast<float>(y0);

    auto sample = [&image](UINT sx, UINT sy) -> XMFLOAT3
    { return image.pixels[static_cast<size_t>(sy) * image.width + sx]; };

    const XMFLOAT3 c00 = sample(x0, y0);
    const XMFLOAT3 c10 = sample(x1, y0);
    const XMFLOAT3 c01 = sample(x0, y1);
    const XMFLOAT3 c11 = sample(x1, y1);

    const XMFLOAT3 cx0 = {c00.x + (c10.x - c00.x) * tx, c00.y + (c10.y - c00.y) * tx, c00.z + (c10.z - c00.z) * tx};
    const XMFLOAT3 cx1 = {c01.x + (c11.x - c01.x) * tx, c01.y + (c11.y - c01.y) * tx, c01.z + (c11.z - c01.z) * tx};
    return {cx0.x + (cx1.x - cx0.x) * ty, cx0.y + (cx1.y - cx0.y) * ty, cx0.z + (cx1.z - cx0.z) * ty};
}

XMFLOAT3 SampleEnvironment(const HdrImage& image, XMFLOAT3 direction)
{
    return SampleEquirectangular(image, direction);
}

void BuildTangentFrame(const XMFLOAT3& normal, XMFLOAT3& tangent, XMFLOAT3& bitangent)
{
    const XMFLOAT3 up = std::abs(normal.y) < 0.999f ? XMFLOAT3{0.0f, 1.0f, 0.0f} : XMFLOAT3{1.0f, 0.0f, 0.0f};
    tangent = Normalize(Cross(up, normal));
    bitangent = Cross(normal, tangent);
}

XMFLOAT3 ToWorld(const XMFLOAT3& tangent, const XMFLOAT3& bitangent, const XMFLOAT3& normal, const XMFLOAT3& local)
{
    return {tangent.x * local.x + bitangent.x * local.y + normal.x * local.z,
            tangent.y * local.x + bitangent.y * local.y + normal.y * local.z,
            tangent.z * local.x + bitangent.z * local.y + normal.z * local.z};
}

XMFLOAT3 ImportanceSampleGGX(const XMFLOAT2& xi,
                             float roughness,
                             const XMFLOAT3& tangent,
                             const XMFLOAT3& bitangent,
                             const XMFLOAT3& normal)
{
    static constexpr float kPi = 3.1415926535f;
    const float a = roughness * roughness;
    const float phi = 2.0f * kPi * xi.x;
    const float cosTheta = std::sqrt((1.0f - xi.y) / (1.0f + (a * a - 1.0f) * xi.y));
    const float sinTheta = std::sqrt((std::max)(0.0f, 1.0f - cosTheta * cosTheta));
    const XMFLOAT3 local = {std::cos(phi) * sinTheta, std::sin(phi) * sinTheta, cosTheta};
    return Normalize(ToWorld(tangent, bitangent, normal, local));
}

XMFLOAT3 ComputeDiffuseIrradiance(const HdrImage& image, const XMFLOAT3& normal)
{
    static constexpr UINT kSampleCount = 64;
    static constexpr float kPi = 3.1415926535f;

    XMFLOAT3 tangent = {};
    XMFLOAT3 bitangent = {};
    BuildTangentFrame(normal, tangent, bitangent);

    XMFLOAT3 irradiance = {0.0f, 0.0f, 0.0f};
    for (UINT i = 0; i < kSampleCount; ++i)
    {
        const XMFLOAT2 xi = Hammersley(i, kSampleCount);
        const float phi = 2.0f * kPi * xi.x;
        const float cosTheta = std::sqrt(1.0f - xi.y);
        const float sinTheta = std::sqrt(xi.y);
        const XMFLOAT3 local = {std::cos(phi) * sinTheta, std::sin(phi) * sinTheta, cosTheta};
        irradiance = Add(irradiance, SampleEnvironment(image, ToWorld(tangent, bitangent, normal, local)));
    }

    return Multiply(irradiance, kPi / static_cast<float>(kSampleCount));
}

XMFLOAT3 ComputeSpecularPrefilter(const HdrImage& image, const XMFLOAT3& reflectionDir, float roughness)
{
    static constexpr UINT kSampleCount = 64;

    if (roughness <= 0.001f)
    {
        return SampleEnvironment(image, reflectionDir);
    }

    XMFLOAT3 tangent = {};
    XMFLOAT3 bitangent = {};
    BuildTangentFrame(reflectionDir, tangent, bitangent);

    const XMFLOAT3 viewDir = reflectionDir;
    XMFLOAT3 prefiltered = {0.0f, 0.0f, 0.0f};
    float totalWeight = 0.0f;
    for (UINT i = 0; i < kSampleCount; ++i)
    {
        const XMFLOAT2 xi = Hammersley(i, kSampleCount);
        const XMFLOAT3 halfVector = ImportanceSampleGGX(xi, roughness, tangent, bitangent, reflectionDir);
        const XMFLOAT3 lightDir = Normalize(Reflect({-viewDir.x, -viewDir.y, -viewDir.z}, halfVector));
        const float ndotl = Clamp01(Dot(reflectionDir, lightDir));
        if (ndotl > 0.0f)
        {
            prefiltered = Add(prefiltered, Multiply(SampleEnvironment(image, lightDir), ndotl));
            totalWeight += ndotl;
        }
    }

    return totalWeight > 0.0f ? Multiply(prefiltered, 1.0f / totalWeight) : SampleEnvironment(image, reflectionDir);
}

XMFLOAT3 SampleProceduralEnvironment(const ProceduralEnvironmentSettings& settings, XMFLOAT3 direction)
{
    direction = Normalize(direction);
    const XMFLOAT3 lightDirection = Normalize(settings.lightDirection);
    const float skyBlend = Clamp01(direction.y * 0.5f + 0.5f);
    const float horizonWidth = (std::max)(0.01f, settings.horizonSharpness);
    const float horizonBlend = SmoothStep(-horizonWidth, horizonWidth, direction.y);
    const XMFLOAT3 baseHorizon = Lerp(settings.groundColor, settings.skyColor, horizonBlend);
    const XMFLOAT3 baseSky = Lerp(settings.groundColor, settings.skyColor, skyBlend);
    const float lightDot = Clamp01(Dot(direction, lightDirection));
    const float lightSize = (std::max)(0.01f, settings.lightSize);
    const float lightSpot = std::pow(lightDot, 1.0f / lightSize);

    switch (settings.source)
    {
        case EnvironmentSource::ProceduralSun:
        {
            const XMFLOAT3 background = Multiply(baseSky, settings.backgroundIntensity);
            const XMFLOAT3 sun = Multiply(settings.lightColor, lightSpot * settings.lightIntensity);
            return Add(background, sun);
        }
        case EnvironmentSource::ProceduralColorPanels:
        {
            XMFLOAT3 panelColor = {};
            const float ax = std::abs(direction.x);
            const float ay = std::abs(direction.y);
            const float az = std::abs(direction.z);
            if (ax >= ay && ax >= az)
            {
                panelColor = direction.x > 0.0f ? XMFLOAT3{1.0f, 0.12f, 0.08f} : XMFLOAT3{0.05f, 0.55f, 1.0f};
            }
            else if (ay >= ax && ay >= az)
            {
                panelColor = direction.y > 0.0f ? XMFLOAT3{1.0f, 1.0f, 1.0f} : XMFLOAT3{0.18f, 0.15f, 0.12f};
            }
            else
            {
                panelColor = direction.z > 0.0f ? XMFLOAT3{0.10f, 1.0f, 0.22f} : XMFLOAT3{1.0f, 0.82f, 0.08f};
            }

            const XMFLOAT3 background = Multiply(baseHorizon, settings.fillIntensity);
            return Add(background, Multiply(panelColor, settings.colorPanelIntensity));
        }
        case EnvironmentSource::ProceduralHorizon:
        {
            const float horizonLine = 1.0f - SmoothStep(0.0f, horizonWidth, std::abs(direction.y));
            const XMFLOAT3 horizonLight = Multiply(settings.lightColor, horizonLine * settings.lightIntensity * 0.25f);
            return Add(Multiply(baseHorizon, settings.backgroundIntensity), horizonLight);
        }
        case EnvironmentSource::ProceduralStudio:
        case EnvironmentSource::AssetHdr:
        default:
        {
            const XMFLOAT3 background = Multiply(baseHorizon, settings.backgroundIntensity);
            const XMFLOAT3 softbox = Multiply(settings.lightColor, lightSpot * settings.lightIntensity);
            const XMFLOAT3 fill = Multiply(settings.skyColor, settings.fillIntensity * skyBlend);
            return Add(Add(background, softbox), fill);
        }
    }
}

XMFLOAT3 ComputeDiffuseIrradiance(const ProceduralEnvironmentSettings& settings, const XMFLOAT3& normal)
{
    static constexpr UINT kSampleCount = 64;
    static constexpr float kPi = 3.1415926535f;

    XMFLOAT3 tangent = {};
    XMFLOAT3 bitangent = {};
    BuildTangentFrame(normal, tangent, bitangent);

    XMFLOAT3 irradiance = {0.0f, 0.0f, 0.0f};
    for (UINT i = 0; i < kSampleCount; ++i)
    {
        const XMFLOAT2 xi = Hammersley(i, kSampleCount);
        const float phi = 2.0f * kPi * xi.x;
        const float cosTheta = std::sqrt(1.0f - xi.y);
        const float sinTheta = std::sqrt(xi.y);
        const XMFLOAT3 local = {std::cos(phi) * sinTheta, std::sin(phi) * sinTheta, cosTheta};
        irradiance = Add(irradiance, SampleProceduralEnvironment(settings, ToWorld(tangent, bitangent, normal, local)));
    }

    return Multiply(irradiance, kPi / static_cast<float>(kSampleCount));
}

XMFLOAT3 ComputeSpecularPrefilter(const ProceduralEnvironmentSettings& settings,
                                  const XMFLOAT3& reflectionDir,
                                  float roughness)
{
    static constexpr UINT kSampleCount = 64;

    if (roughness <= 0.001f)
    {
        return SampleProceduralEnvironment(settings, reflectionDir);
    }

    XMFLOAT3 tangent = {};
    XMFLOAT3 bitangent = {};
    BuildTangentFrame(reflectionDir, tangent, bitangent);

    const XMFLOAT3 viewDir = reflectionDir;
    XMFLOAT3 prefiltered = {0.0f, 0.0f, 0.0f};
    float totalWeight = 0.0f;
    for (UINT i = 0; i < kSampleCount; ++i)
    {
        const XMFLOAT2 xi = Hammersley(i, kSampleCount);
        const XMFLOAT3 halfVector = ImportanceSampleGGX(xi, roughness, tangent, bitangent, reflectionDir);
        const XMFLOAT3 lightDir = Normalize(Reflect({-viewDir.x, -viewDir.y, -viewDir.z}, halfVector));
        const float ndotl = Clamp01(Dot(reflectionDir, lightDir));
        if (ndotl > 0.0f)
        {
            prefiltered = Add(prefiltered, Multiply(SampleProceduralEnvironment(settings, lightDir), ndotl));
            totalWeight += ndotl;
        }
    }

    return totalWeight > 0.0f ? Multiply(prefiltered, 1.0f / totalWeight)
                              : SampleProceduralEnvironment(settings, reflectionDir);
}

DescriptorHeapHandle AllocateTextureCubeSRV(ID3D12Device* device,
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

bool CreateCubeFromHdrImage(ID3D12Device* device,
                            ID3D12GraphicsCommandList* commandList,
                            SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                            const HdrImage& image,
                            UINT outputSize,
                            bool createDiffuseIrradiance,
                            ComPtr<ID3D12Resource>& uploadHeap,
                            ComPtr<ID3D12Resource>& resource,
                            DescriptorHeapHandle& srv)
{
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    textureDesc.Width = outputSize;
    textureDesc.Height = outputSize;
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
                                                  IID_PPV_ARGS(&resource)));

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(resource.Get(), 0, kEnvironmentMapFaceCount);
    ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                  D3D12_HEAP_FLAG_NONE,
                                                  &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                                                  D3D12_RESOURCE_STATE_GENERIC_READ,
                                                  nullptr,
                                                  IID_PPV_ARGS(&uploadHeap)));

    std::vector<std::vector<XMHALF4>> facePixels(kEnvironmentMapFaceCount);
    std::vector<D3D12_SUBRESOURCE_DATA> subresources(kEnvironmentMapFaceCount);

    for (UINT face = 0; face < kEnvironmentMapFaceCount; ++face)
    {
        facePixels[face].resize(static_cast<size_t>(outputSize) * outputSize);
        for (UINT y = 0; y < outputSize; ++y)
        {
            for (UINT x = 0; x < outputSize; ++x)
            {
                const float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(outputSize) * 2.0f - 1.0f;
                const float v = 1.0f - (static_cast<float>(y) + 0.5f) / static_cast<float>(outputSize) * 2.0f;
                const XMFLOAT3 direction = Normalize(DirectionForCubeFace(face, u, v));
                const XMFLOAT3 color = createDiffuseIrradiance ? ComputeDiffuseIrradiance(image, direction)
                                                               : SampleEnvironment(image, direction);
                facePixels[face][static_cast<size_t>(y) * outputSize + x] = XMHALF4(color.x, color.y, color.z, 1.0f);
            }
        }

        subresources[face].pData = facePixels[face].data();
        subresources[face].RowPitch = outputSize * sizeof(XMHALF4);
        subresources[face].SlicePitch = subresources[face].RowPitch * outputSize;
    }

    UpdateSubresources(
        commandList, resource.Get(), uploadHeap.Get(), 0, 0, kEnvironmentMapFaceCount, subresources.data());

    commandList->ResourceBarrier(1,
                                 &CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(),
                                                                       D3D12_RESOURCE_STATE_COPY_DEST,
                                                                       D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    srv = AllocateTextureCubeSRV(device, descriptorHeapAllocator, resource.Get());
    return true;
}

bool CreateCubeFromProceduralEnvironment(ID3D12Device* device,
                                         ID3D12GraphicsCommandList* commandList,
                                         SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                                         const ProceduralEnvironmentSettings& settings,
                                         UINT outputSize,
                                         bool createDiffuseIrradiance,
                                         ComPtr<ID3D12Resource>& uploadHeap,
                                         ComPtr<ID3D12Resource>& resource,
                                         DescriptorHeapHandle& srv)
{
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    textureDesc.Width = outputSize;
    textureDesc.Height = outputSize;
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
                                                  IID_PPV_ARGS(&resource)));

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(resource.Get(), 0, kEnvironmentMapFaceCount);
    ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                  D3D12_HEAP_FLAG_NONE,
                                                  &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                                                  D3D12_RESOURCE_STATE_GENERIC_READ,
                                                  nullptr,
                                                  IID_PPV_ARGS(&uploadHeap)));

    std::vector<std::vector<XMHALF4>> facePixels(kEnvironmentMapFaceCount);
    std::vector<D3D12_SUBRESOURCE_DATA> subresources(kEnvironmentMapFaceCount);

    for (UINT face = 0; face < kEnvironmentMapFaceCount; ++face)
    {
        facePixels[face].resize(static_cast<size_t>(outputSize) * outputSize);
        for (UINT y = 0; y < outputSize; ++y)
        {
            for (UINT x = 0; x < outputSize; ++x)
            {
                const float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(outputSize) * 2.0f - 1.0f;
                const float v = 1.0f - (static_cast<float>(y) + 0.5f) / static_cast<float>(outputSize) * 2.0f;
                const XMFLOAT3 direction = Normalize(DirectionForCubeFace(face, u, v));
                const XMFLOAT3 color = createDiffuseIrradiance ? ComputeDiffuseIrradiance(settings, direction)
                                                               : SampleProceduralEnvironment(settings, direction);
                facePixels[face][static_cast<size_t>(y) * outputSize + x] = XMHALF4(color.x, color.y, color.z, 1.0f);
            }
        }

        subresources[face].pData = facePixels[face].data();
        subresources[face].RowPitch = outputSize * sizeof(XMHALF4);
        subresources[face].SlicePitch = subresources[face].RowPitch * outputSize;
    }

    UpdateSubresources(
        commandList, resource.Get(), uploadHeap.Get(), 0, 0, kEnvironmentMapFaceCount, subresources.data());

    commandList->ResourceBarrier(1,
                                 &CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(),
                                                                       D3D12_RESOURCE_STATE_COPY_DEST,
                                                                       D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    srv = AllocateTextureCubeSRV(device, descriptorHeapAllocator, resource.Get());
    return true;
}

bool CreateSpecularPrefilterCubeFromHdrImage(ID3D12Device* device,
                                             ID3D12GraphicsCommandList* commandList,
                                             SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                                             const HdrImage& image,
                                             UINT outputSize,
                                             UINT mipCount,
                                             ComPtr<ID3D12Resource>& uploadHeap,
                                             ComPtr<ID3D12Resource>& resource,
                                             DescriptorHeapHandle& srv)
{
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = static_cast<UINT16>(mipCount);
    textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    textureDesc.Width = outputSize;
    textureDesc.Height = outputSize;
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
                                                  IID_PPV_ARGS(&resource)));

    const UINT subresourceCount = kEnvironmentMapFaceCount * mipCount;
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(resource.Get(), 0, subresourceCount);
    ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                  D3D12_HEAP_FLAG_NONE,
                                                  &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                                                  D3D12_RESOURCE_STATE_GENERIC_READ,
                                                  nullptr,
                                                  IID_PPV_ARGS(&uploadHeap)));

    std::vector<std::vector<XMHALF4>> subresourcePixels(subresourceCount);
    std::vector<D3D12_SUBRESOURCE_DATA> subresources(subresourceCount);

    for (UINT face = 0; face < kEnvironmentMapFaceCount; ++face)
    {
        for (UINT mip = 0; mip < mipCount; ++mip)
        {
            const UINT mipSize = (std::max)(1u, outputSize >> mip);
            const float roughness = mipCount > 1 ? static_cast<float>(mip) / static_cast<float>(mipCount - 1) : 0.0f;
            const UINT subresourceIndex = face * mipCount + mip;
            subresourcePixels[subresourceIndex].resize(static_cast<size_t>(mipSize) * mipSize);

            for (UINT y = 0; y < mipSize; ++y)
            {
                for (UINT x = 0; x < mipSize; ++x)
                {
                    const float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(mipSize) * 2.0f - 1.0f;
                    const float v = 1.0f - (static_cast<float>(y) + 0.5f) / static_cast<float>(mipSize) * 2.0f;
                    const XMFLOAT3 direction = Normalize(DirectionForCubeFace(face, u, v));
                    const XMFLOAT3 color = ComputeSpecularPrefilter(image, direction, roughness);
                    subresourcePixels[subresourceIndex][static_cast<size_t>(y) * mipSize + x] =
                        XMHALF4(color.x, color.y, color.z, 1.0f);
                }
            }

            subresources[subresourceIndex].pData = subresourcePixels[subresourceIndex].data();
            subresources[subresourceIndex].RowPitch = mipSize * sizeof(XMHALF4);
            subresources[subresourceIndex].SlicePitch = subresources[subresourceIndex].RowPitch * mipSize;
        }
    }

    UpdateSubresources(commandList, resource.Get(), uploadHeap.Get(), 0, 0, subresourceCount, subresources.data());

    commandList->ResourceBarrier(1,
                                 &CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(),
                                                                       D3D12_RESOURCE_STATE_COPY_DEST,
                                                                       D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    srv = AllocateTextureCubeSRV(device, descriptorHeapAllocator, resource.Get());
    return true;
}

bool CreateSpecularPrefilterCubeFromProceduralEnvironment(ID3D12Device* device,
                                                          ID3D12GraphicsCommandList* commandList,
                                                          SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                                                          const ProceduralEnvironmentSettings& settings,
                                                          UINT outputSize,
                                                          UINT mipCount,
                                                          ComPtr<ID3D12Resource>& uploadHeap,
                                                          ComPtr<ID3D12Resource>& resource,
                                                          DescriptorHeapHandle& srv)
{
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = static_cast<UINT16>(mipCount);
    textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    textureDesc.Width = outputSize;
    textureDesc.Height = outputSize;
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
                                                  IID_PPV_ARGS(&resource)));

    const UINT subresourceCount = kEnvironmentMapFaceCount * mipCount;
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(resource.Get(), 0, subresourceCount);
    ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                  D3D12_HEAP_FLAG_NONE,
                                                  &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                                                  D3D12_RESOURCE_STATE_GENERIC_READ,
                                                  nullptr,
                                                  IID_PPV_ARGS(&uploadHeap)));

    std::vector<std::vector<XMHALF4>> subresourcePixels(subresourceCount);
    std::vector<D3D12_SUBRESOURCE_DATA> subresources(subresourceCount);

    for (UINT face = 0; face < kEnvironmentMapFaceCount; ++face)
    {
        for (UINT mip = 0; mip < mipCount; ++mip)
        {
            const UINT mipSize = (std::max)(1u, outputSize >> mip);
            const float roughness = mipCount > 1 ? static_cast<float>(mip) / static_cast<float>(mipCount - 1) : 0.0f;
            const UINT subresourceIndex = face * mipCount + mip;
            subresourcePixels[subresourceIndex].resize(static_cast<size_t>(mipSize) * mipSize);

            for (UINT y = 0; y < mipSize; ++y)
            {
                for (UINT x = 0; x < mipSize; ++x)
                {
                    const float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(mipSize) * 2.0f - 1.0f;
                    const float v = 1.0f - (static_cast<float>(y) + 0.5f) / static_cast<float>(mipSize) * 2.0f;
                    const XMFLOAT3 direction = Normalize(DirectionForCubeFace(face, u, v));
                    const XMFLOAT3 color = ComputeSpecularPrefilter(settings, direction, roughness);
                    subresourcePixels[subresourceIndex][static_cast<size_t>(y) * mipSize + x] =
                        XMHALF4(color.x, color.y, color.z, 1.0f);
                }
            }

            subresources[subresourceIndex].pData = subresourcePixels[subresourceIndex].data();
            subresources[subresourceIndex].RowPitch = mipSize * sizeof(XMHALF4);
            subresources[subresourceIndex].SlicePitch = subresources[subresourceIndex].RowPitch * mipSize;
        }
    }

    UpdateSubresources(commandList, resource.Get(), uploadHeap.Get(), 0, 0, subresourceCount, subresources.data());

    commandList->ResourceBarrier(1,
                                 &CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(),
                                                                       D3D12_RESOURCE_STATE_COPY_DEST,
                                                                       D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    srv = AllocateTextureCubeSRV(device, descriptorHeapAllocator, resource.Get());
    return true;
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

bool EnvironmentMap::TryCreateFromHdrEquirectangular(ID3D12Device* device,
                                                     ID3D12GraphicsCommandList* commandList,
                                                     SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                                                     const HdrImage& image,
                                                     UINT outputSize,
                                                     bool createDiffuseIrradiance,
                                                     ComPtr<ID3D12Resource>& uploadHeap)
{
    if (image.width == 0 || image.height == 0 || image.pixels.empty())
    {
        return false;
    }

    return CreateCubeFromHdrImage(device,
                                  commandList,
                                  descriptorHeapAllocator,
                                  image,
                                  outputSize,
                                  createDiffuseIrradiance,
                                  uploadHeap,
                                  m_resource,
                                  m_srv);
}

bool EnvironmentMap::TryCreateSpecularPrefilterFromHdrEquirectangular(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* commandList,
    SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
    const HdrImage& image,
    UINT outputSize,
    UINT mipCount,
    ComPtr<ID3D12Resource>& uploadHeap)
{
    if (image.width == 0 || image.height == 0 || image.pixels.empty())
    {
        return false;
    }

    return CreateSpecularPrefilterCubeFromHdrImage(device,
                                                   commandList,
                                                   descriptorHeapAllocator,
                                                   image,
                                                   outputSize,
                                                   mipCount,
                                                   uploadHeap,
                                                   m_resource,
                                                   m_srv);
}

void EnvironmentMap::CreateProcedural(ID3D12Device* device,
                                      ID3D12GraphicsCommandList* commandList,
                                      SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                                      const ProceduralEnvironmentSettings& settings,
                                      UINT outputSize,
                                      bool createDiffuseIrradiance,
                                      ComPtr<ID3D12Resource>& uploadHeap)
{
    CreateCubeFromProceduralEnvironment(device,
                                        commandList,
                                        descriptorHeapAllocator,
                                        settings,
                                        outputSize,
                                        createDiffuseIrradiance,
                                        uploadHeap,
                                        m_resource,
                                        m_srv);
}

void EnvironmentMap::CreateSpecularPrefilterProcedural(ID3D12Device* device,
                                                       ID3D12GraphicsCommandList* commandList,
                                                       SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                                                       const ProceduralEnvironmentSettings& settings,
                                                       UINT outputSize,
                                                       UINT mipCount,
                                                       ComPtr<ID3D12Resource>& uploadHeap)
{
    CreateSpecularPrefilterCubeFromProceduralEnvironment(device,
                                                         commandList,
                                                         descriptorHeapAllocator,
                                                         settings,
                                                         outputSize,
                                                         mipCount,
                                                         uploadHeap,
                                                         m_resource,
                                                         m_srv);
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
    ProceduralEnvironmentSettings settings = {};
    settings.source = EnvironmentSource::ProceduralSun;
    CreateProcedural(device, commandList, descriptorHeapAllocator, settings, kEnvironmentMapSize, false, uploadHeap);
}

void EnvironmentMap::Release(SimpleDescriptorHeapAllocator& descriptorHeapAllocator)
{
    if (m_srv.IsValid())
    {
        descriptorHeapAllocator.Free(m_srv);
        m_srv = {};
    }

    m_resource.Reset();
}

DescriptorHeapHandle EnvironmentMap::AllocateTextureCubeSRV(ID3D12Device* device,
                                                            SimpleDescriptorHeapAllocator& descriptorHeapAllocator,
                                                            ID3D12Resource* texture)
{
    return ::Engine::AllocateTextureCubeSRV(device, descriptorHeapAllocator, texture);
}

} // namespace Engine
