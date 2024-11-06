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
// Author:   James Stanard
//

#include "Renderer.h"
#include "Model.h"
#include "TextureManager.h"
#include "ConstantBuffers.h"
#include "LightManager.h"
#include "../Core/RootSignature.h"
#include "../Core/PipelineState.h"
#include "../Core/GraphicsCommon.h"
#include "../Core/BufferManager.h"
#include "../Core/ShadowCamera.h"

#include "CompiledShaders/DefaultVS.h"
#include "CompiledShaders/DefaultSkinVS.h"
#include "CompiledShaders/DefaultPS.h"
#include "CompiledShaders/DefaultNoUV1VS.h"
#include "CompiledShaders/DefaultNoUV1SkinVS.h"
#include "CompiledShaders/DefaultNoUV1PS.h"
#include "CompiledShaders/DefaultNoTangentVS.h"
#include "CompiledShaders/DefaultNoTangentSkinVS.h"
#include "CompiledShaders/DefaultNoTangentPS.h"
#include "CompiledShaders/DefaultNoTangentNoUV1VS.h"
#include "CompiledShaders/DefaultNoTangentNoUV1SkinVS.h"
#include "CompiledShaders/DefaultNoTangentNoUV1PS.h"
#include "CompiledShaders/DepthOnlyVS.h"
#include "CompiledShaders/DepthOnlySkinVS.h"
#include "CompiledShaders/CutoutDepthVS.h"
#include "CompiledShaders/CutoutDepthSkinVS.h"
#include "CompiledShaders/CutoutDepthPS.h"
#include "CompiledShaders/SkyboxVS.h"
#include "CompiledShaders/SkyboxPS.h"

#pragma warning(disable:4319) // '~': zero extending 'uint32_t' to 'uint64_t' of greater size

using namespace Math;
using namespace Graphics;
using namespace Renderer;

namespace Renderer
{
    BoolVar SeparateZPass("Renderer/Separate Z Pass", true);

    bool s_Initialized = false;

    DescriptorHeap s_TextureHeap;
    DescriptorHeap s_SamplerHeap;
    std::vector<GraphicsPSO> sm_PSOs;

    TextureRef s_RadianceCubeMap;
    TextureRef s_IrradianceCubeMap;
    float s_SpecularIBLRange;
    float s_SpecularIBLBias;
    uint32_t g_SSAOFullScreenID;
    uint32_t g_ShadowBufferID;

    RootSignature m_RootSig;
    GraphicsPSO m_SkyboxPSO(L"Renderer: Skybox PSO");
    GraphicsPSO m_DefaultPSO(L"Renderer: Default PSO"); // Not finalized.  Used as a template.

    DescriptorHandle m_CommonTextures;
}

void Renderer::Initialize(void)
{
    if (s_Initialized)
        return;

    SamplerDesc DefaultSamplerDesc;
    DefaultSamplerDesc.MaxAnisotropy = 8;

    SamplerDesc CubeMapSamplerDesc = DefaultSamplerDesc;
    //CubeMapSamplerDesc.MaxLOD = 6.0f;

    m_RootSig.Reset(kNumRootBindings, 3);
    m_RootSig.InitStaticSampler(10, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig.InitStaticSampler(11, SamplerShadowDesc, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig.InitStaticSampler(12, CubeMapSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig[kMeshConstants].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
    m_RootSig[kMaterialConstants].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig[kMaterialSRVs].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig[kMaterialSamplers].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0, 10, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig[kCommonSRVs].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 10, 10, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig[kCommonCBV].InitAsConstantBuffer(1);
    m_RootSig[kSkinMatrices].InitAsBufferSRV(20, D3D12_SHADER_VISIBILITY_VERTEX);
    m_RootSig.Finalize(L"RootSig", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    DXGI_FORMAT ColorFormat = g_SceneColorBuffer.GetFormat();
    DXGI_FORMAT DepthFormat = g_SceneDepthBuffer.GetFormat();

    D3D12_INPUT_ELEMENT_DESC posOnly[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_INPUT_ELEMENT_DESC posAndUV[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R16G16_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_INPUT_ELEMENT_DESC skinPos[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R16G16B16A16_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT", 0, DXGI_FORMAT_R16G16B16A16_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_INPUT_ELEMENT_DESC skinPosAndUV[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R16G16_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R16G16B16A16_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT", 0, DXGI_FORMAT_R16G16B16A16_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    ASSERT(sm_PSOs.size() == 0);

    // Depth Only PSOs

    GraphicsPSO DepthOnlyPSO(L"Renderer: Depth Only PSO");
    DepthOnlyPSO.SetRootSignature(m_RootSig);
    DepthOnlyPSO.SetRasterizerState(RasterizerDefault);
    DepthOnlyPSO.SetBlendState(BlendDisable);
    DepthOnlyPSO.SetDepthStencilState(DepthStateReadWrite);
    DepthOnlyPSO.SetInputLayout(_countof(posOnly), posOnly);
    DepthOnlyPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    DepthOnlyPSO.SetRenderTargetFormats(0, nullptr, DepthFormat);
    DepthOnlyPSO.SetVertexShader(g_pDepthOnlyVS, sizeof(g_pDepthOnlyVS));
    DepthOnlyPSO.Finalize();
    sm_PSOs.push_back(DepthOnlyPSO);

    GraphicsPSO CutoutDepthPSO(L"Renderer: Cutout Depth PSO");
    CutoutDepthPSO = DepthOnlyPSO;
    CutoutDepthPSO.SetInputLayout(_countof(posAndUV), posAndUV);
    CutoutDepthPSO.SetRasterizerState(RasterizerTwoSided);
    CutoutDepthPSO.SetVertexShader(g_pCutoutDepthVS, sizeof(g_pCutoutDepthVS));
    CutoutDepthPSO.SetPixelShader(g_pCutoutDepthPS, sizeof(g_pCutoutDepthPS));
    CutoutDepthPSO.Finalize();
    sm_PSOs.push_back(CutoutDepthPSO);

    GraphicsPSO SkinDepthOnlyPSO = DepthOnlyPSO;
    SkinDepthOnlyPSO.SetInputLayout(_countof(skinPos), skinPos);
    SkinDepthOnlyPSO.SetVertexShader(g_pDepthOnlySkinVS, sizeof(g_pDepthOnlySkinVS));
    SkinDepthOnlyPSO.Finalize();
    sm_PSOs.push_back(SkinDepthOnlyPSO);

    GraphicsPSO SkinCutoutDepthPSO = CutoutDepthPSO;
    SkinCutoutDepthPSO.SetInputLayout(_countof(skinPosAndUV), skinPosAndUV);
    SkinCutoutDepthPSO.SetVertexShader(g_pCutoutDepthSkinVS, sizeof(g_pCutoutDepthSkinVS));
    SkinCutoutDepthPSO.Finalize();
    sm_PSOs.push_back(SkinCutoutDepthPSO);

    ASSERT(sm_PSOs.size() == 4);

    // Shadow PSOs

    DepthOnlyPSO.SetRasterizerState(RasterizerShadow);
    DepthOnlyPSO.SetRenderTargetFormats(0, nullptr, g_ShadowBuffer.GetFormat());
    DepthOnlyPSO.Finalize();
    sm_PSOs.push_back(DepthOnlyPSO);

    CutoutDepthPSO.SetRasterizerState(RasterizerShadowTwoSided);
    CutoutDepthPSO.SetRenderTargetFormats(0, nullptr, g_ShadowBuffer.GetFormat());
    CutoutDepthPSO.Finalize();
    sm_PSOs.push_back(CutoutDepthPSO);

    SkinDepthOnlyPSO.SetRasterizerState(RasterizerShadow);
    SkinDepthOnlyPSO.SetRenderTargetFormats(0, nullptr, g_ShadowBuffer.GetFormat());
    SkinDepthOnlyPSO.Finalize();
    sm_PSOs.push_back(SkinDepthOnlyPSO);

    SkinCutoutDepthPSO.SetRasterizerState(RasterizerShadowTwoSided);
    SkinCutoutDepthPSO.SetRenderTargetFormats(0, nullptr, g_ShadowBuffer.GetFormat());
    SkinCutoutDepthPSO.Finalize();
    sm_PSOs.push_back(SkinCutoutDepthPSO);

    ASSERT(sm_PSOs.size() == 8);

    // Default PSO

    m_DefaultPSO.SetRootSignature(m_RootSig);
    m_DefaultPSO.SetRasterizerState(RasterizerDefault);
    m_DefaultPSO.SetBlendState(BlendDisable);
    m_DefaultPSO.SetDepthStencilState(DepthStateReadWrite);
    m_DefaultPSO.SetInputLayout(0, nullptr);
    m_DefaultPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    m_DefaultPSO.SetRenderTargetFormats(1, &ColorFormat, DepthFormat);
    m_DefaultPSO.SetVertexShader(g_pDefaultVS, sizeof(g_pDefaultVS));
    m_DefaultPSO.SetPixelShader(g_pDefaultPS, sizeof(g_pDefaultPS));

    // Skybox PSO

    m_SkyboxPSO = m_DefaultPSO;
    m_SkyboxPSO.SetDepthStencilState(DepthStateReadOnly);
    m_SkyboxPSO.SetInputLayout(0, nullptr);
    m_SkyboxPSO.SetVertexShader(g_pSkyboxVS, sizeof(g_pSkyboxVS));
    m_SkyboxPSO.SetPixelShader(g_pSkyboxPS, sizeof(g_pSkyboxPS));
    m_SkyboxPSO.Finalize();

    TextureManager::Initialize(L"");

    s_TextureHeap.Create(L"Scene Texture Descriptors", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096);

    // Maybe only need 2 for wrap vs. clamp?  Currently we allocate 1 for 1 with textures
    s_SamplerHeap.Create(L"Scene Sampler Descriptors", D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 2048);

    Lighting::InitializeResources();

    // Allocate a descriptor table for the common textures
    m_CommonTextures = s_TextureHeap.Alloc(8);

    uint32_t DestCount = 8;
    uint32_t SourceCounts[] = { 1, 1, 1, 1, 1, 1, 1, 1 };

    D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
    {
        GetDefaultTexture(kBlackCubeMap),
        GetDefaultTexture(kBlackCubeMap),
        g_SSAOFullScreen.GetSRV(),
        g_ShadowBuffer.GetSRV(),
        Lighting::m_LightBuffer.GetSRV(),
        Lighting::m_LightShadowArray.GetSRV(),
        Lighting::m_LightGrid.GetSRV(),
        Lighting::m_LightGridBitMask.GetSRV(),
    };

    g_Device->CopyDescriptors(1, &m_CommonTextures, &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    g_SSAOFullScreenID = g_SSAOFullScreen.GetVersionID();
    g_ShadowBufferID = g_ShadowBuffer.GetVersionID();

    s_Initialized = true;
}

void Renderer::UpdateGlobalDescriptors(void)
{
    if (g_SSAOFullScreenID == g_SSAOFullScreen.GetVersionID() &&
        g_ShadowBufferID == g_ShadowBuffer.GetVersionID())
    {
        return;
    }

    uint32_t DestCount = 2;
    uint32_t SourceCounts[] = { 1, 1 };

    D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
    {
        g_SSAOFullScreen.GetSRV(),
        g_ShadowBuffer.GetSRV(),
    };

    DescriptorHandle dest = m_CommonTextures + 2 * s_TextureHeap.GetDescriptorSize();

    g_Device->CopyDescriptors(1, &dest, &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    g_SSAOFullScreenID = g_SSAOFullScreen.GetVersionID();
    g_ShadowBufferID = g_ShadowBuffer.GetVersionID();

}

void Renderer::SetIBLTextures(TextureRef diffuseIBL, TextureRef specularIBL)
{
    s_RadianceCubeMap = specularIBL;
    s_IrradianceCubeMap = diffuseIBL;

    s_SpecularIBLRange = 0.0f;
    if (s_RadianceCubeMap.IsValid())
    {
        ID3D12Resource* texRes = const_cast<ID3D12Resource*>(s_RadianceCubeMap.Get()->GetResource());
        const D3D12_RESOURCE_DESC& texDesc = texRes->GetDesc();
        s_SpecularIBLRange = Max(0.0f, (float)texDesc.MipLevels - 1);
        s_SpecularIBLBias = Min(s_SpecularIBLBias, s_SpecularIBLRange);
    }

    uint32_t DestCount = 2;
    uint32_t SourceCounts[] = { 1, 1 };

    D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
    {
        specularIBL.IsValid() ? specularIBL.GetSRV() : GetDefaultTexture(kBlackCubeMap),
        diffuseIBL.IsValid() ? diffuseIBL.GetSRV() : GetDefaultTexture(kBlackCubeMap)
    };

    g_Device->CopyDescriptors(1, &m_CommonTextures, &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void Renderer::SetIBLBias(float LODBias)
{
    s_SpecularIBLBias = Min(LODBias, s_SpecularIBLRange);
}

void Renderer::Shutdown(void)
{
    s_RadianceCubeMap = nullptr;
    s_IrradianceCubeMap = nullptr;
    TextureManager::Shutdown();
    s_TextureHeap.Destroy();
    s_SamplerHeap.Destroy();
}

uint8_t Renderer::GetPSO(uint16_t psoFlags)
{
    using namespace PSOFlags;

    GraphicsPSO ColorPSO = m_DefaultPSO;

    uint16_t Requirements = kHasPosition | kHasNormal;
    ASSERT((psoFlags & Requirements) == Requirements);

    std::vector<D3D12_INPUT_ELEMENT_DESC> vertexLayout;
    if (psoFlags & kHasPosition)
        vertexLayout.push_back({"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT});
    if (psoFlags & kHasNormal)
        vertexLayout.push_back({"NORMAL",   0, DXGI_FORMAT_R10G10B10A2_UNORM,  0, D3D12_APPEND_ALIGNED_ELEMENT});
    if (psoFlags & kHasTangent)
        vertexLayout.push_back({"TANGENT",  0, DXGI_FORMAT_R10G10B10A2_UNORM,  0, D3D12_APPEND_ALIGNED_ELEMENT});
    if (psoFlags & kHasUV0)
        vertexLayout.push_back({"TEXCOORD", 0, DXGI_FORMAT_R16G16_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT});
    else
        vertexLayout.push_back({"TEXCOORD", 0, DXGI_FORMAT_R16G16_FLOAT,       1, D3D12_APPEND_ALIGNED_ELEMENT});
    if (psoFlags & kHasUV1)
        vertexLayout.push_back({"TEXCOORD", 1, DXGI_FORMAT_R16G16_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT});
    if (psoFlags & kHasSkin)
    {
        vertexLayout.push_back({ "BLENDINDICES", 0, DXGI_FORMAT_R16G16B16A16_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
        vertexLayout.push_back({ "BLENDWEIGHT", 0, DXGI_FORMAT_R16G16B16A16_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
    }

    ColorPSO.SetInputLayout((uint32_t)vertexLayout.size(), vertexLayout.data());

    if (psoFlags & kHasSkin)
    {
        if (psoFlags & kHasTangent)
        {
            if (psoFlags & kHasUV1)
            {
                ColorPSO.SetVertexShader(g_pDefaultSkinVS, sizeof(g_pDefaultSkinVS));
                ColorPSO.SetPixelShader(g_pDefaultPS, sizeof(g_pDefaultPS));
            }
            else
            {
                ColorPSO.SetVertexShader(g_pDefaultNoUV1SkinVS, sizeof(g_pDefaultNoUV1SkinVS));
                ColorPSO.SetPixelShader(g_pDefaultNoUV1PS, sizeof(g_pDefaultNoUV1PS));
            }
        }
        else
        {
            if (psoFlags & kHasUV1)
            {
                ColorPSO.SetVertexShader(g_pDefaultNoTangentSkinVS, sizeof(g_pDefaultNoTangentSkinVS));
                ColorPSO.SetPixelShader(g_pDefaultNoTangentPS, sizeof(g_pDefaultNoTangentPS));
            }
            else
            {
                ColorPSO.SetVertexShader(g_pDefaultNoTangentNoUV1SkinVS, sizeof(g_pDefaultNoTangentNoUV1SkinVS));
                ColorPSO.SetPixelShader(g_pDefaultNoTangentNoUV1PS, sizeof(g_pDefaultNoTangentNoUV1PS));
            }
        }
    }
    else
    {
        if (psoFlags & kHasTangent)
        {
            if (psoFlags & kHasUV1)
            {
                ColorPSO.SetVertexShader(g_pDefaultVS, sizeof(g_pDefaultVS));
                ColorPSO.SetPixelShader(g_pDefaultPS, sizeof(g_pDefaultPS));
            }
            else
            {
                ColorPSO.SetVertexShader(g_pDefaultNoUV1VS, sizeof(g_pDefaultNoUV1VS));
                ColorPSO.SetPixelShader(g_pDefaultNoUV1PS, sizeof(g_pDefaultNoUV1PS));
            }
        }
        else
        {
            if (psoFlags & kHasUV1)
            {
                ColorPSO.SetVertexShader(g_pDefaultNoTangentVS, sizeof(g_pDefaultNoTangentVS));
                ColorPSO.SetPixelShader(g_pDefaultNoTangentPS, sizeof(g_pDefaultNoTangentPS));
            }
            else
            {
                ColorPSO.SetVertexShader(g_pDefaultNoTangentNoUV1VS, sizeof(g_pDefaultNoTangentNoUV1VS));
                ColorPSO.SetPixelShader(g_pDefaultNoTangentNoUV1PS, sizeof(g_pDefaultNoTangentNoUV1PS));
            }
        }
    }

    if (psoFlags & kAlphaBlend)
    {
        ColorPSO.SetBlendState(BlendPreMultiplied);
        ColorPSO.SetDepthStencilState(DepthStateReadOnly);
    }
    if (psoFlags & kTwoSided)
    {
        ColorPSO.SetRasterizerState(RasterizerTwoSided);
    }
    ColorPSO.Finalize();

    // Look for an existing PSO
    for (uint32_t i = 0; i < sm_PSOs.size(); ++i)
    {
        if (ColorPSO.GetPipelineStateObject() == sm_PSOs[i].GetPipelineStateObject())
        {
            return (uint8_t)i;
        }
    }

    // If not found, keep the new one, and return its index
    sm_PSOs.push_back(ColorPSO);

    // The returned PSO index has read-write depth.  The index+1 tests for equal depth.
    ColorPSO.SetDepthStencilState(DepthStateTestEqual);
    ColorPSO.Finalize();
#ifdef DEBUG
    for (uint32_t i = 0; i < sm_PSOs.size(); ++i)
        ASSERT(ColorPSO.GetPipelineStateObject() != sm_PSOs[i].GetPipelineStateObject());
#endif
    sm_PSOs.push_back(ColorPSO);

    ASSERT(sm_PSOs.size() <= 256, "Ran out of room for unique PSOs");

    return (uint8_t)sm_PSOs.size() - 2;
}

void Renderer::DrawSkybox( GraphicsContext& gfxContext, const Camera& Camera, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor )
{
    ScopedTimer _prof(L"Draw Skybox", gfxContext);

    __declspec(align(16)) struct SkyboxVSCB
    {
        Matrix4 ProjInverse;
        Matrix3 ViewInverse;
    } skyVSCB;
    skyVSCB.ProjInverse = Invert(Camera.GetProjMatrix());
    skyVSCB.ViewInverse = Invert(Camera.GetViewMatrix()).Get3x3();

    __declspec(align(16)) struct SkyboxPSCB
    {
        float TextureLevel;
    } skyPSCB;
    skyPSCB.TextureLevel = s_SpecularIBLBias;

    gfxContext.SetRootSignature(m_RootSig);
    gfxContext.SetPipelineState(m_SkyboxPSO);

    gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
    gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
    gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV(), g_SceneDepthBuffer.GetDSV_DepthReadOnly());
    gfxContext.SetViewportAndScissor(viewport, scissor);

    gfxContext.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, s_TextureHeap.GetHeapPointer());
    gfxContext.SetDynamicConstantBufferView(kMeshConstants, sizeof(SkyboxVSCB), &skyVSCB);
    gfxContext.SetDynamicConstantBufferView(kMaterialConstants, sizeof(SkyboxPSCB), &skyPSCB);
    gfxContext.SetDescriptorTable(kCommonSRVs, m_CommonTextures);
    gfxContext.Draw(3);
}

void MeshSorter::AddMesh( const Mesh& mesh, float distance,
    D3D12_GPU_VIRTUAL_ADDRESS meshCBV,
    D3D12_GPU_VIRTUAL_ADDRESS materialCBV,
    D3D12_GPU_VIRTUAL_ADDRESS bufferPtr,
    const Joint* skeleton)
{
    SortKey key;
    key.value = m_SortObjects.size();

	bool alphaBlend = (mesh.psoFlags & PSOFlags::kAlphaBlend) == PSOFlags::kAlphaBlend;
    bool alphaTest = (mesh.psoFlags & PSOFlags::kAlphaTest) == PSOFlags::kAlphaTest;
    bool skinned = (mesh.psoFlags & PSOFlags::kHasSkin) == PSOFlags::kHasSkin;
    uint64_t depthPSO = (skinned ? 2 : 0) + (alphaTest ? 1 : 0);

    union float_or_int { float f; uint32_t u; } dist;
    dist.f = Max(distance, 0.0f);

	if (m_BatchType == kShadows)
	{
		if (alphaBlend)
			return;

		key.passID = kZPass;
		key.psoIdx = depthPSO + 4;
        key.key = dist.u;
		m_SortKeys.push_back(key.value);
		m_PassCounts[kZPass]++;
	}
    else if (mesh.psoFlags & PSOFlags::kAlphaBlend)
    {
        key.passID = kTransparent;
        key.psoIdx = mesh.pso;
        key.key = ~dist.u;
        m_SortKeys.push_back(key.value);
        m_PassCounts[kTransparent]++;
    }
    else if (SeparateZPass || alphaTest)
    {
        key.passID = kZPass;
        key.psoIdx = depthPSO;
        key.key = dist.u;
        m_SortKeys.push_back(key.value);
        m_PassCounts[kZPass]++;

        key.passID = kOpaque;
        key.psoIdx = mesh.pso + 1;
        key.key = dist.u;
        m_SortKeys.push_back(key.value);
        m_PassCounts[kOpaque]++;
    }
    else
    {
        key.passID = kOpaque;
        key.psoIdx = mesh.pso;
        key.key = dist.u;
        m_SortKeys.push_back(key.value);
        m_PassCounts[kOpaque]++;
    }

    SortObject object = { &mesh, skeleton, meshCBV, materialCBV, bufferPtr };
    m_SortObjects.push_back(object);
}

void MeshSorter::Sort()
{
    struct { bool operator()(uint64_t a, uint64_t b) const { return a < b; } } Cmp;
    std::sort(m_SortKeys.begin(), m_SortKeys.end(), Cmp);
}

void MeshSorter::RenderMeshes(
    DrawPass pass,
    GraphicsContext& context,
    GlobalConstants& globals)
{
	ASSERT(m_DSV != nullptr);

    Renderer::UpdateGlobalDescriptors();

    context.SetRootSignature(m_RootSig);
    context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, s_TextureHeap.GetHeapPointer());
    context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, s_SamplerHeap.GetHeapPointer());

    // Set common textures
    context.SetDescriptorTable(kCommonSRVs, m_CommonTextures);

    // Set common shader constants
	globals.ViewProjMatrix = m_Camera->GetViewProjMatrix();
	globals.CameraPos = m_Camera->GetPosition();
    globals.IBLRange = s_SpecularIBLRange - s_SpecularIBLBias;
    globals.IBLBias = s_SpecularIBLBias;
	context.SetDynamicConstantBufferView(kCommonCBV, sizeof(GlobalConstants), &globals);

	if (m_BatchType == kShadows)
	{
		context.TransitionResource(*m_DSV, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
		context.ClearDepth(*m_DSV);
		context.SetDepthStencilTarget(m_DSV->GetDSV());

		if (m_Viewport.Width == 0)
		{
			m_Viewport.TopLeftX = 0.0f;
			m_Viewport.TopLeftY = 0.0f;
			m_Viewport.Width = (float)m_DSV->GetWidth();
			m_Viewport.Height = (float)m_DSV->GetHeight();
			m_Viewport.MaxDepth = 1.0f;
			m_Viewport.MinDepth = 0.0f;

			m_Scissor.left = 1;
			m_Scissor.right = m_DSV->GetWidth() - 2;
			m_Scissor.top = 1;
			m_Scissor.bottom = m_DSV->GetHeight() - 2;
		}
	}
	else
	{
		for (uint32_t i = 0; i < m_NumRTVs; ++i)
		{
			ASSERT(m_RTV[i] != nullptr);
			ASSERT(m_DSV->GetWidth() == m_RTV[i]->GetWidth());
			ASSERT(m_DSV->GetHeight() == m_RTV[i]->GetHeight());
		}

		if (m_Viewport.Width == 0)
		{
			m_Viewport.TopLeftX = 0.0f;
			m_Viewport.TopLeftY = 0.0f;
			m_Viewport.Width = (float)m_DSV->GetWidth();
			m_Viewport.Height = (float)m_DSV->GetHeight();
			m_Viewport.MaxDepth = 1.0f;
			m_Viewport.MinDepth = 0.0f;

			m_Scissor.left = 0;
			m_Scissor.right = m_DSV->GetWidth();
			m_Scissor.top = 0;
			m_Scissor.bottom = m_DSV->GetWidth();
		}
	}

    for ( ; m_CurrentPass <= pass; m_CurrentPass = (DrawPass)(m_CurrentPass + 1))
    {
        const uint32_t passCount = m_PassCounts[m_CurrentPass];
        if (passCount == 0)
            continue;

		if (m_BatchType == kDefault)
		{
			switch (m_CurrentPass)
			{
			case kZPass:
				context.TransitionResource(*m_DSV, D3D12_RESOURCE_STATE_DEPTH_WRITE);
				context.SetDepthStencilTarget(m_DSV->GetDSV());
				break;
			case kOpaque:
				if (SeparateZPass)
				{
					context.TransitionResource(*m_DSV, D3D12_RESOURCE_STATE_DEPTH_READ);
					context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
					context.SetRenderTarget(g_SceneColorBuffer.GetRTV(), m_DSV->GetDSV_DepthReadOnly());
				}
				else
				{
					context.TransitionResource(*m_DSV, D3D12_RESOURCE_STATE_DEPTH_WRITE);
					context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
					context.SetRenderTarget(g_SceneColorBuffer.GetRTV(), m_DSV->GetDSV());
				}
				break;
			case kTransparent:
				context.TransitionResource(*m_DSV, D3D12_RESOURCE_STATE_DEPTH_READ);
				context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
				context.SetRenderTarget(g_SceneColorBuffer.GetRTV(), m_DSV->GetDSV_DepthReadOnly());
				break;
			}
		}

        context.SetViewportAndScissor(m_Viewport, m_Scissor);
        context.FlushResourceBarriers();

        const uint32_t lastDraw = m_CurrentDraw + passCount;

        while (m_CurrentDraw < lastDraw)
        {
            SortKey key;
            key.value = m_SortKeys[m_CurrentDraw];
            const SortObject& object = m_SortObjects[key.objectIdx];
            const Mesh& mesh = *object.mesh;

            context.SetConstantBuffer(kMeshConstants, object.meshCBV);
            context.SetConstantBuffer(kMaterialConstants, object.materialCBV);
            context.SetDescriptorTable(kMaterialSRVs, s_TextureHeap[mesh.srvTable]);
            context.SetDescriptorTable(kMaterialSamplers, s_SamplerHeap[mesh.samplerTable]);
            if (mesh.numJoints > 0)
            {
                ASSERT(object.skeleton != nullptr, "Unspecified joint matrix array");
                context.SetDynamicSRV(kSkinMatrices, sizeof(Joint) * mesh.numJoints, object.skeleton + mesh.startJoint);
            }
            context.SetPipelineState(sm_PSOs[key.psoIdx]);

            if (m_CurrentPass == kZPass)
            {
                bool alphaTest = (mesh.psoFlags & PSOFlags::kAlphaTest) == PSOFlags::kAlphaTest;
                uint32_t stride = alphaTest ? 16u : 12u;
                if (mesh.numJoints > 0)
                    stride += 16;
                context.SetVertexBuffer(0, {object.bufferPtr + mesh.vbDepthOffset, mesh.vbDepthSize, stride});
            }
            else
            {
                context.SetVertexBuffer(0, {object.bufferPtr + mesh.vbOffset, mesh.vbSize, mesh.vbStride});
            }

            context.SetIndexBuffer({object.bufferPtr + mesh.ibOffset, mesh.ibSize, (DXGI_FORMAT)mesh.ibFormat});

            for (uint32_t i = 0; i < mesh.numDraws; ++i)
                context.DrawIndexed(mesh.draw[i].primCount, mesh.draw[i].startIndex, mesh.draw[i].baseVertex);

            ++m_CurrentDraw;
        }
    }

	if (m_BatchType == kShadows)
	{
		context.TransitionResource(*m_DSV, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
}
