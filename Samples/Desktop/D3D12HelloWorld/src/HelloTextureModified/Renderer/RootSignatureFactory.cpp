#include "stdafx.h"

#include "RootSignatureFactory.h"

#include "RootSignatureLayout.h"

namespace Engine
{
namespace
{

struct DescriptorRanges
{
    CD3DX12_DESCRIPTOR_RANGE1 textureSrvs;
    CD3DX12_DESCRIPTOR_RANGE1 instanceSrv;
    CD3DX12_DESCRIPTOR_RANGE1 materialSrv;
    CD3DX12_DESCRIPTOR_RANGE1 gbufferAndDepthSrvs;
    CD3DX12_DESCRIPTOR_RANGE1 environmentMapSrv;
    CD3DX12_DESCRIPTOR_RANGE1 toneMapSceneColorSrv;
    CD3DX12_DESCRIPTOR_RANGE1 cameraCbv;
    CD3DX12_DESCRIPTOR_RANGE1 lightCbv;
};

D3D_ROOT_SIGNATURE_VERSION GetHighestRootSignatureVersion(ID3D12Device* device)
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

    // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned
    // will not be greater than this.
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    return featureData.HighestVersion;
}

DescriptorRanges CreateDescriptorRanges(UINT textureSrvCount, UINT gbufferSrvCount)
{
    DescriptorRanges ranges = {};

    // t0 - t(TextureCount-1) : Texture SRVs: space 0 : 0 - (textureSrvCount-1)
    ranges.textureSrvs.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                            textureSrvCount,
                            RootSignatureLayout::kBaseRegister,
                            RootSignatureLayout::kTextureSrvSpace,
                            D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

    // t0 : SRV structured buffer: space1 : 0
    ranges.instanceSrv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                            1,
                            RootSignatureLayout::kBaseRegister,
                            RootSignatureLayout::kInstanceSrvSpace,
                            D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

    // t0 : SRV structured buffer: space2 : 0
    ranges.materialSrv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                            1,
                            RootSignatureLayout::kBaseRegister,
                            RootSignatureLayout::kMaterialSrvSpace,
                            D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

    // t0 - t4 : GBuffer SRVs, t5 : depth SRV, space 3
    ranges.gbufferAndDepthSrvs.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                                    gbufferSrvCount,
                                    RootSignatureLayout::kBaseRegister,
                                    RootSignatureLayout::kGBufferSrvSpace,
                                    D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

    // t0 - t3 : Environment cubemap SRV + diffuse irradiance cubemap SRV + specular prefilter cubemap SRV + BRDF LUT, space 5
    ranges.environmentMapSrv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                                  4,
                                  RootSignatureLayout::kBaseRegister,
                                  RootSignatureLayout::kEnvironmentMapSrvSpace,
                                  D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE |
                                      D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

    // t0 : HDR scene color SRV, space 4
    ranges.toneMapSceneColorSrv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                                     1,
                                     RootSignatureLayout::kBaseRegister,
                                     RootSignatureLayout::kToneMapSceneColorSrvSpace,
                                     D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

    ranges.cameraCbv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                          1,
                          RootSignatureLayout::kCameraCbvRegister,
                          RootSignatureLayout::kCameraCbvSpace,
                          D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

    ranges.lightCbv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                         1,
                         RootSignatureLayout::kLightCbvRegister,
                         RootSignatureLayout::kLightCbvSpace,
                         D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

    return ranges;
}

void CreateRootParameters(const DescriptorRanges& ranges,
                          CD3DX12_ROOT_PARAMETER1 (&rootParameters)[RootSignatureLayout::Count])
{
    rootParameters[RootSignatureLayout::TextureTable].InitAsDescriptorTable(
        1, &ranges.textureSrvs, D3D12_SHADER_VISIBILITY_PIXEL); // Texture SRVs
    rootParameters[RootSignatureLayout::InstanceSrv].InitAsDescriptorTable(
        1, &ranges.instanceSrv, D3D12_SHADER_VISIBILITY_ALL); // Structured buffer SRV (Instance data)
    rootParameters[RootSignatureLayout::MaterialSrv].InitAsDescriptorTable(
        1, &ranges.materialSrv, D3D12_SHADER_VISIBILITY_ALL); // Structured buffer SRV (Material data)
    rootParameters[RootSignatureLayout::CameraConstants].InitAsDescriptorTable(
        1, &ranges.cameraCbv, D3D12_SHADER_VISIBILITY_ALL); // Camera constants
    rootParameters[RootSignatureLayout::GBufferSrvBase].InitAsDescriptorTable(
        1, &ranges.gbufferAndDepthSrvs, D3D12_SHADER_VISIBILITY_PIXEL); // GBuffer and depth SRVs
    rootParameters[RootSignatureLayout::EnvironmentMap].InitAsDescriptorTable(
        1, &ranges.environmentMapSrv, D3D12_SHADER_VISIBILITY_PIXEL); // Environment cubemap
    rootParameters[RootSignatureLayout::LightConstants].InitAsDescriptorTable(
        1, &ranges.lightCbv, D3D12_SHADER_VISIBILITY_PIXEL); // Light constants
    rootParameters[RootSignatureLayout::GBufferDebugConstants].InitAsConstants(
        RootSignatureLayout::kGBufferDebugConstantsCount,
        RootSignatureLayout::kGBufferDebugConstantsRegister,
        RootSignatureLayout::kGBufferDebugConstantsSpace,
        D3D12_SHADER_VISIBILITY_PIXEL); // GBuffer debug target
    rootParameters[RootSignatureLayout::ToneMapSceneColor].InitAsDescriptorTable(
        1, &ranges.toneMapSceneColorSrv, D3D12_SHADER_VISIBILITY_PIXEL); // ToneMap HDR scene color
    rootParameters[RootSignatureLayout::ToneMapConstants].InitAsConstants(
        RootSignatureLayout::kToneMapConstantsCount,
        RootSignatureLayout::kToneMapConstantsRegister,
        RootSignatureLayout::kToneMapConstantsSpace,
        D3D12_SHADER_VISIBILITY_PIXEL); // ToneMap constants
}

D3D12_STATIC_SAMPLER_DESC CreateStaticSampler()
{
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 1;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = RootSignatureLayout::kStaticSamplerRegister;
    sampler.RegisterSpace = RootSignatureLayout::kStaticSamplerSpace;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    return sampler;
}

} // namespace

void CreateRootSignature(ID3D12Device* device,
                         UINT textureSrvCount,
                         UINT gbufferSrvCount,
                         Microsoft::WRL::ComPtr<ID3D12RootSignature>& rootSignature)
{
    const D3D_ROOT_SIGNATURE_VERSION rootSignatureVersion = GetHighestRootSignatureVersion(device);

    const DescriptorRanges ranges = CreateDescriptorRanges(textureSrvCount, gbufferSrvCount);
    CD3DX12_ROOT_PARAMETER1 rootParameters[RootSignatureLayout::Count];
    CreateRootParameters(ranges, rootParameters);

    const D3D12_STATIC_SAMPLER_DESC sampler = CreateStaticSampler();

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(_countof(rootParameters),
                               rootParameters,
                               1,
                               &sampler,
                               D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, rootSignatureVersion, &signature, &error));
    ThrowIfFailed(device->CreateRootSignature(
        0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
}

} // namespace Engine
