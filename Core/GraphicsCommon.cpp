//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#include "pch.h"
#include "GraphicsCommon.h"
#include "SamplerManager.h"
#include "CommandSignature.h"
#include "BitonicSort.h"
#include "Texture.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "BufferManager.h"

#include "CompiledShaders/GenerateMipsLinearCS.h"
#include "CompiledShaders/GenerateMipsLinearOddCS.h"
#include "CompiledShaders/GenerateMipsLinearOddXCS.h"
#include "CompiledShaders/GenerateMipsLinearOddYCS.h"
#include "CompiledShaders/GenerateMipsGammaCS.h"
#include "CompiledShaders/GenerateMipsGammaOddCS.h"
#include "CompiledShaders/GenerateMipsGammaOddXCS.h"
#include "CompiledShaders/GenerateMipsGammaOddYCS.h"

#include "CompiledShaders/ScreenQuadCommonVS.h"
#include "CompiledShaders/DownsampleDepthPS.h"

namespace Graphics
{
    SamplerDesc SamplerLinearWrapDesc;
    SamplerDesc SamplerAnisoWrapDesc;
    SamplerDesc SamplerShadowDesc;
    SamplerDesc SamplerLinearClampDesc;
    SamplerDesc SamplerVolumeWrapDesc;
    SamplerDesc SamplerPointClampDesc;
    SamplerDesc SamplerPointBorderDesc;
    SamplerDesc SamplerLinearBorderDesc;

    D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearWrap;
    D3D12_CPU_DESCRIPTOR_HANDLE SamplerAnisoWrap;
    D3D12_CPU_DESCRIPTOR_HANDLE SamplerShadow;
    D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearClamp;
    D3D12_CPU_DESCRIPTOR_HANDLE SamplerVolumeWrap;
    D3D12_CPU_DESCRIPTOR_HANDLE SamplerPointClamp;
    D3D12_CPU_DESCRIPTOR_HANDLE SamplerPointBorder;
    D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearBorder;

    Texture DefaultTextures[kNumDefaultTextures];
    D3D12_CPU_DESCRIPTOR_HANDLE GetDefaultTexture( eDefaultTexture texID )
    {
        ASSERT(texID < kNumDefaultTextures);
        return DefaultTextures[texID].GetSRV();
    }

    D3D12_RASTERIZER_DESC RasterizerDefault;	// Counter-clockwise
    D3D12_RASTERIZER_DESC RasterizerDefaultMsaa;
    D3D12_RASTERIZER_DESC RasterizerDefaultCw;	// Clockwise winding
    D3D12_RASTERIZER_DESC RasterizerDefaultCwMsaa;
    D3D12_RASTERIZER_DESC RasterizerTwoSided;
    D3D12_RASTERIZER_DESC RasterizerTwoSidedMsaa;
    D3D12_RASTERIZER_DESC RasterizerShadow;
    D3D12_RASTERIZER_DESC RasterizerShadowCW;
    D3D12_RASTERIZER_DESC RasterizerShadowTwoSided;

    D3D12_BLEND_DESC BlendNoColorWrite;
    D3D12_BLEND_DESC BlendDisable;
    D3D12_BLEND_DESC BlendPreMultiplied;
    D3D12_BLEND_DESC BlendTraditional;
    D3D12_BLEND_DESC BlendAdditive;
    D3D12_BLEND_DESC BlendTraditionalAdditive;

    D3D12_DEPTH_STENCIL_DESC DepthStateDisabled;
    D3D12_DEPTH_STENCIL_DESC DepthStateReadWrite;
    D3D12_DEPTH_STENCIL_DESC DepthStateReadOnly;
    D3D12_DEPTH_STENCIL_DESC DepthStateReadOnlyReversed;
    D3D12_DEPTH_STENCIL_DESC DepthStateTestEqual;

    CommandSignature DispatchIndirectCommandSignature(1);
    CommandSignature DrawIndirectCommandSignature(1);

    RootSignature g_CommonRS;
    ComputePSO g_GenerateMipsLinearPSO[4] =
    {
        {L"Generate Mips Linear CS"},
        {L"Generate Mips Linear Odd X CS"},
        {L"Generate Mips Linear Odd Y CS"},
        {L"Generate Mips Linear Odd CS"},
    };

    ComputePSO g_GenerateMipsGammaPSO[4] =
    {
        { L"Generate Mips Gamma CS" },
        { L"Generate Mips Gamma Odd X CS" },
        { L"Generate Mips Gamma Odd Y CS" },
        { L"Generate Mips Gamma Odd CS" },
    };

    GraphicsPSO g_DownsampleDepthPSO(L"DownsampleDepth PSO");
}

namespace BitonicSort
{
    void Initialize(void);
    void Shutdown(void);
}

void Graphics::InitializeCommonState(void)
{
    SamplerLinearWrapDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerLinearWrap = SamplerLinearWrapDesc.CreateDescriptor();

    SamplerAnisoWrapDesc.MaxAnisotropy = 4;
    SamplerAnisoWrap = SamplerAnisoWrapDesc.CreateDescriptor();

    SamplerShadowDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    SamplerShadowDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
    SamplerShadowDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
    SamplerShadow = SamplerShadowDesc.CreateDescriptor();

    SamplerLinearClampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerLinearClampDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
    SamplerLinearClamp = SamplerLinearClampDesc.CreateDescriptor();

    SamplerVolumeWrapDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    SamplerVolumeWrap = SamplerVolumeWrapDesc.CreateDescriptor();

    SamplerPointClampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    SamplerPointClampDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
    SamplerPointClamp = SamplerPointClampDesc.CreateDescriptor();

    SamplerLinearBorderDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerLinearBorderDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_BORDER);
    SamplerLinearBorderDesc.SetBorderColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
    SamplerLinearBorder = SamplerLinearBorderDesc.CreateDescriptor();

    SamplerPointBorderDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    SamplerPointBorderDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_BORDER);
    SamplerPointBorderDesc.SetBorderColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
    SamplerPointBorder = SamplerPointBorderDesc.CreateDescriptor();

    uint32_t MagentaPixel = 0xFFFF00FF;
    DefaultTextures[kMagenta2D].Create2D(4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &MagentaPixel);
    uint32_t BlackOpaqueTexel = 0xFF000000;
    DefaultTextures[kBlackOpaque2D].Create2D(4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &BlackOpaqueTexel);
    uint32_t BlackTransparentTexel = 0x00000000;
    DefaultTextures[kBlackTransparent2D].Create2D(4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &BlackTransparentTexel);
    uint32_t WhiteOpaqueTexel = 0xFFFFFFFF;
    DefaultTextures[kWhiteOpaque2D].Create2D(4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &WhiteOpaqueTexel);
    uint32_t WhiteTransparentTexel = 0x00FFFFFF;
    DefaultTextures[kWhiteTransparent2D].Create2D(4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &WhiteTransparentTexel);
    uint32_t FlatNormalTexel = 0x00FF8080;
    DefaultTextures[kDefaultNormalMap].Create2D(4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &FlatNormalTexel);
    uint32_t BlackCubeTexels[6] = {};
    DefaultTextures[kBlackCubeMap].CreateCube(4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, BlackCubeTexels);

    // Default rasterizer states
    RasterizerDefault.FillMode = D3D12_FILL_MODE_SOLID;
    RasterizerDefault.CullMode = D3D12_CULL_MODE_BACK;
    RasterizerDefault.FrontCounterClockwise = TRUE;
    RasterizerDefault.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    RasterizerDefault.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    RasterizerDefault.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    RasterizerDefault.DepthClipEnable = TRUE;
    RasterizerDefault.MultisampleEnable = FALSE;
    RasterizerDefault.AntialiasedLineEnable = FALSE;
    RasterizerDefault.ForcedSampleCount = 0;
    RasterizerDefault.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    RasterizerDefaultMsaa = RasterizerDefault;
    RasterizerDefaultMsaa.MultisampleEnable = TRUE;

    RasterizerDefaultCw = RasterizerDefault;
    RasterizerDefaultCw.FrontCounterClockwise = FALSE;

    RasterizerDefaultCwMsaa = RasterizerDefaultCw;
    RasterizerDefaultCwMsaa.MultisampleEnable = TRUE;

    RasterizerTwoSided = RasterizerDefault;
    RasterizerTwoSided.CullMode = D3D12_CULL_MODE_NONE;

    RasterizerTwoSidedMsaa = RasterizerTwoSided;
    RasterizerTwoSidedMsaa.MultisampleEnable = TRUE;

    // Shadows need their own rasterizer state so we can reverse the winding of faces
    RasterizerShadow = RasterizerDefault;
    //RasterizerShadow.CullMode = D3D12_CULL_FRONT;  // Hacked here rather than fixing the content
    RasterizerShadow.SlopeScaledDepthBias = -1.5f;
    RasterizerShadow.DepthBias = -100;

    RasterizerShadowTwoSided = RasterizerShadow;
    RasterizerShadowTwoSided.CullMode = D3D12_CULL_MODE_NONE;

    RasterizerShadowCW = RasterizerShadow;
    RasterizerShadowCW.FrontCounterClockwise = FALSE;

    DepthStateDisabled.DepthEnable = FALSE;
    DepthStateDisabled.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    DepthStateDisabled.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    DepthStateDisabled.StencilEnable = FALSE;
    DepthStateDisabled.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    DepthStateDisabled.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    DepthStateDisabled.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    DepthStateDisabled.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    DepthStateDisabled.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    DepthStateDisabled.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    DepthStateDisabled.BackFace = DepthStateDisabled.FrontFace;

    DepthStateReadWrite = DepthStateDisabled;
    DepthStateReadWrite.DepthEnable = TRUE;
    DepthStateReadWrite.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    DepthStateReadWrite.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;

    DepthStateReadOnly = DepthStateReadWrite;
    DepthStateReadOnly.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

    DepthStateReadOnlyReversed = DepthStateReadOnly;
    DepthStateReadOnlyReversed.DepthFunc = D3D12_COMPARISON_FUNC_LESS;

    DepthStateTestEqual = DepthStateReadOnly;
    DepthStateTestEqual.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;

    D3D12_BLEND_DESC alphaBlend = {};
    alphaBlend.IndependentBlendEnable = FALSE;
    alphaBlend.RenderTarget[0].BlendEnable = FALSE;
    alphaBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    alphaBlend.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    alphaBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    alphaBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    alphaBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
    alphaBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    alphaBlend.RenderTarget[0].RenderTargetWriteMask = 0;
    BlendNoColorWrite = alphaBlend;

    alphaBlend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    BlendDisable = alphaBlend;

    alphaBlend.RenderTarget[0].BlendEnable = TRUE;
    BlendTraditional = alphaBlend;

    alphaBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    BlendPreMultiplied = alphaBlend;

    alphaBlend.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
    BlendAdditive = alphaBlend;

    alphaBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    BlendTraditionalAdditive = alphaBlend;

    DispatchIndirectCommandSignature[0].Dispatch();
    DispatchIndirectCommandSignature.Finalize();

    DrawIndirectCommandSignature[0].Draw();
    DrawIndirectCommandSignature.Finalize();

    BitonicSort::Initialize();

    g_CommonRS.Reset(4, 3);
    g_CommonRS[0].InitAsConstants(0, 4);
    g_CommonRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10);
    g_CommonRS[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 10);
    g_CommonRS[3].InitAsConstantBuffer(1);
    g_CommonRS.InitStaticSampler(0, SamplerLinearClampDesc);
    g_CommonRS.InitStaticSampler(1, SamplerPointBorderDesc);
    g_CommonRS.InitStaticSampler(2, SamplerLinearBorderDesc);
    g_CommonRS.Finalize(L"GraphicsCommonRS");

#define CreatePSO(ObjName, ShaderByteCode ) \
    ObjName.SetRootSignature(g_CommonRS); \
    ObjName.SetComputeShader(ShaderByteCode, sizeof(ShaderByteCode) ); \
    ObjName.Finalize();

    CreatePSO(g_GenerateMipsLinearPSO[0], g_pGenerateMipsLinearCS);
    CreatePSO(g_GenerateMipsLinearPSO[1], g_pGenerateMipsLinearOddXCS);
    CreatePSO(g_GenerateMipsLinearPSO[2], g_pGenerateMipsLinearOddYCS);
    CreatePSO(g_GenerateMipsLinearPSO[3], g_pGenerateMipsLinearOddCS);
    CreatePSO(g_GenerateMipsGammaPSO[0], g_pGenerateMipsGammaCS);
    CreatePSO(g_GenerateMipsGammaPSO[1], g_pGenerateMipsGammaOddXCS);
    CreatePSO(g_GenerateMipsGammaPSO[2], g_pGenerateMipsGammaOddYCS);
    CreatePSO(g_GenerateMipsGammaPSO[3], g_pGenerateMipsGammaOddCS);

    g_DownsampleDepthPSO.SetRootSignature(g_CommonRS);
    g_DownsampleDepthPSO.SetRasterizerState(RasterizerTwoSided);
    g_DownsampleDepthPSO.SetBlendState(BlendDisable);
    g_DownsampleDepthPSO.SetDepthStencilState(DepthStateReadWrite);
    g_DownsampleDepthPSO.SetSampleMask(0xFFFFFFFF);
    g_DownsampleDepthPSO.SetInputLayout(0, nullptr);
    g_DownsampleDepthPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    g_DownsampleDepthPSO.SetVertexShader(g_pScreenQuadCommonVS, sizeof(g_pScreenQuadCommonVS));
    g_DownsampleDepthPSO.SetPixelShader(g_pDownsampleDepthPS, sizeof(g_pDownsampleDepthPS));
    g_DownsampleDepthPSO.SetDepthTargetFormat(DXGI_FORMAT_D32_FLOAT);
    g_DownsampleDepthPSO.Finalize();
}

void Graphics::DestroyCommonState(void)
{
    for (uint32_t i = 0; i < kNumDefaultTextures; ++i)
        DefaultTextures[i].Destroy();

    DispatchIndirectCommandSignature.Destroy();
    DrawIndirectCommandSignature.Destroy();
    
    BitonicSort::Shutdown();
}
