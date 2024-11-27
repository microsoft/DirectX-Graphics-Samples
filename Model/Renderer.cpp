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

// Shadow Buffer VisDebug Rendering
#include "../Model/CompiledShaders/FullscreenQuadVS.h"
#include "../Model/CompiledShaders/VisShadowBufferPS.h"

// SDF Debug Rendering
#include "../Model/CompiledShaders/SDFDebugRayMarchPS.h"

// JFA
#include "../Model/CompiledShaders/JFA3DCS.h"

#include <algorithm>

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
    GraphicsPSO m_DefaultVoxelPSO(L"Renderer: SDFGI Default Voxel PSO"); // Not finalized. Used as a template. 

    DescriptorHandle m_CommonTextures;

    // SDFGI: Voxel
    std::vector<GraphicsPSO> sm_VoxelPSOs;
    DescriptorHandle m_SDFGIVoxelTextures; 
    Texture m_VoxelAlbedo;
    Texture m_VoxelVoronoiInput;

    // SDFGI: 3D JFA
    RootSignature m_JFA3DRS;
    ComputePSO m_JFA3DPSO;

    DescriptorHeap m_JFA3DTextureHeap;
    DescriptorHandle m_JFA3DRWTextures_0;
    DescriptorHandle m_JFA3DRWTextures_1;
    Texture m_FinalSDFOutput;
    Texture m_IntermediateSDFOutput;

    JFAGlobalConstants m_jfaGlobals;

    // SDFGI: SDF Ray March Debug
    DescriptorHeap m_SDFRayMarchTextureHeap; 
    DescriptorHandle m_SDFTextures;
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
    D3D12_DESCRIPTOR_RANGE srvRange;
    srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRange.NumDescriptors = 1;
    srvRange.BaseShaderRegister = 21;
    srvRange.RegisterSpace = 0;
    srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    m_RootSig[kSDFGIIrradianceAtlasSRV].InitAsDescriptorTable(1, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig[kSDFGIIrradianceAtlasSRV].SetTableRange(0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 21, 1);
    // m_RootSig[kSDFGIIrradianceAtlasSRV].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 21, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig[kSDFGIDepthAtlasSRV].InitAsDescriptorTable(1, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig[kSDFGIDepthAtlasSRV].SetTableRange(0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 22, 1);
    m_RootSig[kSDFGICBV].InitAsConstantBuffer(2, D3D12_SHADER_VISIBILITY_PIXEL);

    // For Voxel PSO's
    m_RootSig[kSDFGICommonCBV].InitAsConstantBuffer(3, D3D12_SHADER_VISIBILITY_ALL);
    m_RootSig[kSDFGIVoxelUAVs].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 2, D3D12_SHADER_VISIBILITY_PIXEL);
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

    {
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
    }

    g_SSAOFullScreenID = g_SSAOFullScreen.GetVersionID();
    g_ShadowBufferID = g_ShadowBuffer.GetVersionID();

    // SDFGI Initialization
    InitializeVoxel(); 
    InitializeJFA();
    InitializeRayMarchDebug(); 

    s_Initialized = true;
}


void Renderer::ClearVoxelTextures(GraphicsContext& gfxContext)
{
    gfxContext.TransitionResource(m_VoxelAlbedo, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    gfxContext.TransitionResource(m_VoxelVoronoiInput, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    gfxContext.FlushResourceBarriers();

    gfxContext.ClearUAV(m_VoxelAlbedo); 
    gfxContext.ClearUAV(m_VoxelVoronoiInput);

    gfxContext.TransitionResource(m_VoxelAlbedo, D3D12_RESOURCE_STATE_GENERIC_READ);
    gfxContext.TransitionResource(m_VoxelVoronoiInput, D3D12_RESOURCE_STATE_GENERIC_READ);

    gfxContext.FlushResourceBarriers();
}

void Renderer::ClearSDFTextures(GraphicsContext& gfxContext) {
    gfxContext.TransitionResource(m_FinalSDFOutput, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    gfxContext.TransitionResource(m_IntermediateSDFOutput, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    gfxContext.FlushResourceBarriers();

    gfxContext.ClearUAV(m_FinalSDFOutput);
    gfxContext.ClearUAV(m_IntermediateSDFOutput);

    gfxContext.TransitionResource(m_FinalSDFOutput, D3D12_RESOURCE_STATE_GENERIC_READ);
    gfxContext.TransitionResource(m_IntermediateSDFOutput, D3D12_RESOURCE_STATE_GENERIC_READ);

    gfxContext.FlushResourceBarriers();
}

void Renderer::InitializeVoxel(void)
{
    // This is stupid, but we only do this so that sm_VoxelPSOs is the same size as sm_PSOs, 
    // because we want to index both of these arrays the same way. 
    for (int i = 0; i < 8; ++i) {
        sm_VoxelPSOs.push_back(m_DefaultVoxelPSO);
    }

    // SDFGI Voxel PSO, default PSO but we remove depth culling
    m_DefaultVoxelPSO = m_DefaultPSO;

    D3D12_RASTERIZER_DESC rasterizerDesc = Graphics::RasterizerDefault;
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    m_DefaultVoxelPSO.SetRasterizerState(rasterizerDesc);

    m_DefaultVoxelPSO.SetBlendState(BlendDisable);
    m_DefaultVoxelPSO.SetDepthStencilState(DepthStateDisabled);
    m_DefaultVoxelPSO.SetRenderTargetFormats(1, &g_SceneColorBuffer.GetFormat(), DXGI_FORMAT_UNKNOWN);

    // SDFGI: Create Voxel UAV Textures
    constexpr size_t size = SDF_TEXTURE_RESOLUTION * SDF_TEXTURE_RESOLUTION * SDF_TEXTURE_RESOLUTION;
    uint32_t* init = new uint32_t[size];
    std::fill(init, init + size, 0x0);
    m_VoxelAlbedo.Create3D(
        4, SDF_TEXTURE_RESOLUTION, SDF_TEXTURE_RESOLUTION, SDF_TEXTURE_RESOLUTION, DXGI_FORMAT_R8G8B8A8_UNORM, init, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, L"Voxel Albedo"
    );
    m_VoxelVoronoiInput.Create3D(
        4, SDF_TEXTURE_RESOLUTION, SDF_TEXTURE_RESOLUTION, SDF_TEXTURE_RESOLUTION, DXGI_FORMAT_R16G16B16A16_UINT, init, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, L"Voxel Voronoi Input"
    );
    delete[] init;

    // SDFGI: Allocate a descriptor table for the Voxel 3D Texture UAVs
    m_SDFGIVoxelTextures = s_TextureHeap.Alloc(2);

    {
        uint32_t DestCount = 2;
        uint32_t SourceCounts[] = { 1, 1 };

        D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
        {
            m_VoxelAlbedo.GetUAV(),
            m_VoxelVoronoiInput.GetUAV()
        };

        g_Device->CopyDescriptors(1, &m_SDFGIVoxelTextures, &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
}

void Renderer::InitializeJFA(void)
{
    //JFA3D Initialization
    {
        //RS
        {
            m_JFA3DRS.Reset(2, 1);
            //2 (root params)
            // 0: 3 UAVS (input texture, final sdf texture, intermediate texture)
            m_JFA3DRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, /*register*/ 0, /*count*/ 3);
            // 2: CBV for extra info: ivec2 uResolution and uint uStepSize
            m_JFA3DRS[1].InitAsConstantBuffer(0);

            //1 (texture sampler(s)) (only 1 in this case)
            m_JFA3DRS.InitStaticSampler(0, SamplerLinearClampDesc, D3D12_SHADER_VISIBILITY_ALL);
            m_JFA3DRS.Finalize(L"JFA3DRootSig", D3D12_ROOT_SIGNATURE_FLAG_NONE); // No flags required for CS
        }
        //PSO
        {
            m_JFA3DPSO.SetRootSignature(m_JFA3DRS);
            m_JFA3DPSO.SetComputeShader(g_pJFA3DCS, sizeof(g_pJFA3DCS));
            m_JFA3DPSO.Finalize();
        }
        //Resource Initialization
        {
            constexpr size_t size = SDF_TEXTURE_RESOLUTION * SDF_TEXTURE_RESOLUTION * SDF_TEXTURE_RESOLUTION;
            uint32_t* init = new uint32_t[size];
            std::fill(init, init + size, 0x0);
            m_FinalSDFOutput.Create3D(
                4, SDF_TEXTURE_RESOLUTION, SDF_TEXTURE_RESOLUTION, SDF_TEXTURE_RESOLUTION, DXGI_FORMAT_R32_FLOAT, init, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, L"SDF Output"
            );
            m_IntermediateSDFOutput.Create3D(
                4, SDF_TEXTURE_RESOLUTION, SDF_TEXTURE_RESOLUTION, SDF_TEXTURE_RESOLUTION, DXGI_FORMAT_R16G16B16A16_UINT, init, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, L"Intermediate JFA Output"
            );
            delete[] init;
        }
        {
            m_JFA3DTextureHeap.Create(L"3D JFA Descriptors", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 16);

            // SRV: m_VoxelVoronoiInput
            m_JFA3DRWTextures_0 = m_JFA3DTextureHeap.Alloc(3);
            {
                uint32_t DestCount = 3;
                uint32_t SourceCounts[] = { 1, 1, 1 };

                D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
                {
                    m_VoxelVoronoiInput.GetUAV(),
                    m_FinalSDFOutput.GetUAV(),
                    m_IntermediateSDFOutput.GetUAV()
                };

                g_Device->CopyDescriptors(1, &m_JFA3DRWTextures_0, &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            }

            m_JFA3DRWTextures_1 = m_JFA3DTextureHeap.Alloc(3);
            {
                uint32_t DestCount = 3;
                uint32_t SourceCounts[] = { 1, 1, 1 };

                D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
                {
                    m_IntermediateSDFOutput.GetUAV(),
                    m_FinalSDFOutput.GetUAV(),
                    m_VoxelVoronoiInput.GetUAV()
                };

                g_Device->CopyDescriptors(1, &m_JFA3DRWTextures_1, &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            }
        }
        //Resource Initialization
    }
    //JFA3D Initialization

    m_jfaGlobals.gridResolution[0] = (float)SDF_TEXTURE_RESOLUTION;
    m_jfaGlobals.gridResolution[1] = (float)SDF_TEXTURE_RESOLUTION;
    m_jfaGlobals.gridResolution[2] = (float)SDF_TEXTURE_RESOLUTION;
}

void Renderer::InitializeRayMarchDebug() {
    m_SDFRayMarchTextureHeap.Create(L"SDF Ray March 3D Tex Descriptors", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2);

    // SRV: m_VoxelVoronoiInput
    m_SDFTextures = m_SDFRayMarchTextureHeap.Alloc(2);
    {
        uint32_t DestCount = 2;
        uint32_t SourceCounts[] = { 1, 1 };

        D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
        {
            m_VoxelAlbedo.GetUAV(),
            m_FinalSDFOutput.GetUAV()
        };

        g_Device->CopyDescriptors(1, &m_SDFTextures, &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
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

// TODO: Create duplicates of all hlsl shaders with the "Default" Prefix with the added `#define SDFGI_VOXEL_PASS 1` preprocessor
void Renderer::CreateVoxelPSO(uint16_t psoFlags) {
    using namespace PSOFlags;

    GraphicsPSO ColorPSO = m_DefaultVoxelPSO;

    uint16_t Requirements = kHasPosition | kHasNormal;
    ASSERT((psoFlags & Requirements) == Requirements);

    std::vector<D3D12_INPUT_ELEMENT_DESC> vertexLayout;
    if (psoFlags & kHasPosition)
        vertexLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT });
    if (psoFlags & kHasNormal)
        vertexLayout.push_back({ "NORMAL",   0, DXGI_FORMAT_R10G10B10A2_UNORM,  0, D3D12_APPEND_ALIGNED_ELEMENT });
    if (psoFlags & kHasTangent)
        vertexLayout.push_back({ "TANGENT",  0, DXGI_FORMAT_R10G10B10A2_UNORM,  0, D3D12_APPEND_ALIGNED_ELEMENT });
    if (psoFlags & kHasUV0)
        vertexLayout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R16G16_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT });
    else
        vertexLayout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R16G16_FLOAT,       1, D3D12_APPEND_ALIGNED_ELEMENT });
    if (psoFlags & kHasUV1)
        vertexLayout.push_back({ "TEXCOORD", 1, DXGI_FORMAT_R16G16_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT });
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
  /*  for (uint32_t i = 0; i < sm_VoxelPSOs.size(); ++i)
    {
        if (ColorPSO.GetPipelineStateObject() == sm_VoxelPSOs[i].GetPipelineStateObject())
        {
            return;
        }
    }*/

    // If not found, keep the new one, and return its index
    sm_VoxelPSOs.push_back(ColorPSO);

    // The returned PSO index has read-write depth.  The index+1 tests for equal depth.
    //ColorPSO.SetDepthStencilState(DepthStateTestEqual);
    //ColorPSO.Finalize();
    sm_VoxelPSOs.push_back(ColorPSO);

    ASSERT(sm_VoxelPSOs.size() <= 256, "Ran out of room for unique PSOs");

    return;
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

    // Create associated voxelPSO
    CreateVoxelPSO(psoFlags); 

    ASSERT(sm_PSOs.size() == sm_VoxelPSOs.size());

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

void Renderer::ComputeSDF(ComputeContext& context)
{
    //1. Set up context
    {
        context.SetPipelineState(m_JFA3DPSO);
        context.SetRootSignature(m_JFA3DRS);
        context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_JFA3DTextureHeap.GetHeapPointer());

        context.TransitionResource(m_VoxelVoronoiInput, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        context.TransitionResource(m_FinalSDFOutput, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        context.TransitionResource(m_IntermediateSDFOutput, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        context.FlushResourceBarriers(); 

        bool swap = false;
        constexpr uint32_t stepSizeInit = SDF_TEXTURE_RESOLUTION / 2; 
        constexpr uint32_t dispatchSize = SDF_TEXTURE_RESOLUTION / 8; 
        for (uint32_t i = stepSizeInit; i >= 1; i /= 2) {
            //0. Globals CBV
            {
                m_jfaGlobals.stepSize = i;
                context.SetDynamicConstantBufferView(kJFACBV, sizeof(JFAGlobalConstants), &m_jfaGlobals);
            }
            //1. Descriptor Table with UAVs!
            {
                if (!swap) {
                    context.SetDescriptorTable(kJFATextures, m_JFA3DRWTextures_0); //original order
                }
                else {
                    context.SetDescriptorTable(kJFATextures, m_JFA3DRWTextures_1); //swapped order
                }
            }
            //2. Dispatch
            context.Dispatch(dispatchSize, dispatchSize, dispatchSize);
            //3. Update swap bool
            swap = !swap;
        }
        context.TransitionResource(m_FinalSDFOutput, D3D12_RESOURCE_STATE_GENERIC_READ);
    }
}

void Renderer::RayMarchSDF(GraphicsContext& gfxContext, const Math::Camera& cam, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor)
{
    // Init Constant Buffer
    static RayMarchGlobalConstants constants{}; 
    constants.InvViewProjMatrix = Math::Invert(cam.GetViewProjMatrix()); 
    constants.CameraPos = cam.GetPosition();
    constants.xmin = -2000;
    constants.xmax = 2000;
    constants.ymin = -2000;
    constants.ymax = 2000;
    constants.zmin = -2000;
    constants.zmax = 2000;
    constants.sdfResolution = SDF_TEXTURE_RESOLUTION;

    // Init Root Sig
    RootSignature RayMarchRS;
    {
        RayMarchRS.Reset(2, 0);

        // Root Parameters
        RayMarchRS[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
        RayMarchRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 2, D3D12_SHADER_VISIBILITY_PIXEL);
        RayMarchRS.Finalize(L"Ray March Root Sig", D3D12_ROOT_SIGNATURE_FLAG_NONE);
    }

    GraphicsPSO SDFRayMarchPSO(L"Ray March FullScreen PSO");
    {
        DXGI_FORMAT ColorFormat = g_SceneColorBuffer.GetFormat();
        DXGI_FORMAT DepthFormat = g_SceneDepthBuffer.GetFormat();
        DXGI_FORMAT formats[1] = { ColorFormat };

        SDFRayMarchPSO.SetRootSignature(RayMarchRS);
        D3D12_RASTERIZER_DESC rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
        SDFRayMarchPSO.SetRasterizerState(rasterizerDesc);
        SDFRayMarchPSO.SetBlendState(BlendDisable);

        D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
        {
            depthStencilDesc.DepthEnable = FALSE;        // Disable depth testing
            depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;  // Depth writes are not needed
            depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS; // No depth comparisons
            depthStencilDesc.StencilEnable = FALSE;      // Disable stencil testing
            depthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
            depthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        }
        SDFRayMarchPSO.SetDepthStencilState(depthStencilDesc);
        SDFRayMarchPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
        SDFRayMarchPSO.SetRenderTargetFormats(1, formats, DXGI_FORMAT_UNKNOWN);
        SDFRayMarchPSO.SetVertexShader(g_pFullScreenQuadVS, sizeof(g_pFullScreenQuadVS));
        SDFRayMarchPSO.SetPixelShader(g_pSDFDebugRayMarchPS, sizeof(g_pSDFDebugRayMarchPS));
        SDFRayMarchPSO.Finalize();
    }

    // Render Call
    {
        gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
        gfxContext.ClearColor(g_SceneColorBuffer);

        gfxContext.SetRootSignature(RayMarchRS);
        gfxContext.SetPipelineState(SDFRayMarchPSO);

        gfxContext.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_SDFRayMarchTextureHeap.GetHeapPointer());

        gfxContext.SetDynamicConstantBufferView(0, sizeof(RayMarchGlobalConstants), &constants);
        gfxContext.SetDescriptorTable(1, m_SDFTextures);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvs[]{ g_SceneColorBuffer.GetRTV() };
        gfxContext.SetRenderTargets(ARRAYSIZE(rtvs), rtvs);
        gfxContext.SetViewportAndScissor(viewport, scissor);

        gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        gfxContext.DrawIndexed(6, 0, 0);
    }
}

void Renderer::DrawShadowBuffer(GraphicsContext& gfxContext, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor)
{
    RootSignature VisShadowBufferRS;
    //1. Create the root signature with 1 Descriptor Table for Depth Texture and 1 SRV Sampler for Accessing Depth Texture
    {
        VisShadowBufferRS.Reset(1, 1);

        // 1 Root Parameter: Descriptor Table
        VisShadowBufferRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
        // 1 Sampler for Pixel Shader
        SamplerDesc DepthSamplerDesc;
        DepthSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        DepthSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        DepthSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        DepthSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        DepthSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

        VisShadowBufferRS.InitStaticSampler(0, DepthSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);

        VisShadowBufferRS.Finalize(L"RootSig", D3D12_ROOT_SIGNATURE_FLAG_NONE);
    }

    GraphicsPSO DepthToQuadPSO(L"FullScreenPSO");
    {
        DXGI_FORMAT ColorFormat = g_SceneColorBuffer.GetFormat();
        DXGI_FORMAT DepthFormat = g_SceneDepthBuffer.GetFormat();
        DXGI_FORMAT formats[1] = { ColorFormat };

        DepthToQuadPSO.SetRootSignature(VisShadowBufferRS);
        D3D12_RASTERIZER_DESC rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
        DepthToQuadPSO.SetRasterizerState(rasterizerDesc);
        DepthToQuadPSO.SetBlendState(BlendDisable);

        D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
        {
            depthStencilDesc.DepthEnable = FALSE;        // Disable depth testing
            depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;  // Depth writes are not needed
            depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS; // No depth comparisons

            depthStencilDesc.StencilEnable = FALSE;      // Disable stencil testing
            depthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
            depthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        }
        DepthToQuadPSO.SetDepthStencilState(depthStencilDesc);
        DepthToQuadPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
        DepthToQuadPSO.SetRenderTargetFormats(1, formats, DXGI_FORMAT_UNKNOWN);

        //DepthToQuadPSO.Set

        DepthToQuadPSO.SetVertexShader(g_pFullScreenQuadVS, sizeof(g_pFullScreenQuadVS));
        DepthToQuadPSO.SetPixelShader(g_pVisShadowBufferPS, sizeof(g_pVisShadowBufferPS));
        DepthToQuadPSO.Finalize();
    }

    //Render Call
    {

        gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
        gfxContext.ClearColor(g_SceneColorBuffer);

        gfxContext.SetRootSignature(VisShadowBufferRS);
        gfxContext.SetPipelineState(DepthToQuadPSO);
        gfxContext.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Renderer::s_TextureHeap.GetHeapPointer());
        D3D12_GPU_DESCRIPTOR_HANDLE baseHandle = Renderer::s_TextureHeap.GetHeapPointer()->GetGPUDescriptorHandleForHeapStart();
        D3D12_GPU_DESCRIPTOR_HANDLE shadowHandle;
        unsigned int shadowMapIndexInHeap = 3; //This is just hardcoded by MiniEngine lol
        shadowHandle.ptr = baseHandle.ptr + shadowMapIndexInHeap * g_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        gfxContext.SetDescriptorTable(0, shadowHandle);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvs[]{ g_SceneColorBuffer.GetRTV() };
        gfxContext.SetRenderTargets(ARRAYSIZE(rtvs), rtvs);
        gfxContext.SetViewportAndScissor(viewport, scissor);

        gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        gfxContext.DrawIndexed(6, 0, 0);
    }
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
    GlobalConstants& globals,
    bool UseSDFGI,
    SDFGI::SDFGIManager* mp_SDFGIManager)
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

    static SDFGIGlobalConstants voxelPassOff{};
    voxelPassOff.voxelPass = 0; 
    context.SetDynamicConstantBufferView(kSDFGICommonCBV, sizeof(SDFGIGlobalConstants), &voxelPassOff);


  if (UseSDFGI) {
      __declspec(align(16)) struct SDFGIConstants {
          Vector3 GridSize;                       // 16

          Vector3 ProbeSpacing;                   // 16

          Vector3 SceneMinBounds;                 // 16

          unsigned int ProbeAtlasBlockResolution; // 4
          unsigned int GutterSize;                // 4
          float AtlasWidth;                       // 4
          float AtlasHeight;                      // 4

          bool UseAtlas;                          // 4
          float Pad0;                             // 4
          float Pad1;                             // 4
          float Pad2;                             // 4
      } sdfgiConstants;

      context.SetDescriptorTable(Renderer::kSDFGIIrradianceAtlasSRV, mp_SDFGIManager->GetIrradianceAtlasDescriptorHandle());
      context.SetDescriptorTable(Renderer::kSDFGIDepthAtlasSRV, mp_SDFGIManager->GetDepthAtlasDescriptorHandle());
      SDFGI::SDFGIProbeData sdfgiProbeData = mp_SDFGIManager->GetProbeData();
      sdfgiConstants.GridSize = sdfgiProbeData.GridSize;
      sdfgiConstants.ProbeSpacing = sdfgiProbeData.ProbeSpacing;
      sdfgiConstants.SceneMinBounds = sdfgiProbeData.SceneMinBounds;
      sdfgiConstants.ProbeAtlasBlockResolution = sdfgiProbeData.ProbeAtlasBlockResolution;
      sdfgiConstants.GutterSize = sdfgiProbeData.GutterSize;
      sdfgiConstants.AtlasWidth = sdfgiProbeData.AtlasWidth;
      sdfgiConstants.AtlasHeight = sdfgiProbeData.AtlasHeight;
      sdfgiConstants.UseAtlas = true;
      context.SetDynamicConstantBufferView(Renderer::kSDFGICBV, sizeof(sdfgiConstants), &sdfgiConstants);
  }

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

void MeshSorter::RenderVoxels(DrawPass pass, GraphicsContext& context, GlobalConstants& globals, 
    SDFGIGlobalConstants& SDFGIglobals)
{
    Renderer::UpdateGlobalDescriptors();

    context.SetRootSignature(m_RootSig);
    context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, s_TextureHeap.GetHeapPointer());
    context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, s_SamplerHeap.GetHeapPointer());

    // Set common textures
    context.SetDescriptorTable(kCommonSRVs, m_CommonTextures);
    // SDFGI: Set Voxel UAVS 
    context.SetDescriptorTable(kSDFGIVoxelUAVs, m_SDFGIVoxelTextures);

    // Set common shader constants
    globals.ViewProjMatrix = m_Camera->GetViewProjMatrix();
    globals.CameraPos = m_Camera->GetPosition();
    globals.IBLRange = s_SpecularIBLRange - s_SpecularIBLBias;
    globals.IBLBias = s_SpecularIBLBias;
    context.SetDynamicConstantBufferView(kCommonCBV, sizeof(GlobalConstants), &globals);
    context.SetDynamicConstantBufferView(kSDFGICommonCBV, sizeof(SDFGIGlobalConstants), &SDFGIglobals);

    for (; m_CurrentPass <= pass; m_CurrentPass = (DrawPass)(m_CurrentPass + 1))
    {
        const uint32_t passCount = m_PassCounts[m_CurrentPass];
        if (passCount == 0)
            continue;

        switch (m_CurrentPass)
        {
        case kOpaque:
            context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
            context.SetRenderTarget(g_SceneColorBuffer.GetRTV());
            break;
        case kTransparent:
            context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
            context.SetRenderTarget(g_SceneColorBuffer.GetRTV());
            break;
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
            context.SetPipelineState(sm_VoxelPSOs[key.psoIdx]);

            context.SetVertexBuffer(0, { object.bufferPtr + mesh.vbOffset, mesh.vbSize, mesh.vbStride });

            context.SetIndexBuffer({ object.bufferPtr + mesh.ibOffset, mesh.ibSize, (DXGI_FORMAT)mesh.ibFormat });

            for (uint32_t i = 0; i < mesh.numDraws; ++i)
                context.DrawIndexed(mesh.draw[i].primCount, mesh.draw[i].startIndex, mesh.draw[i].baseVertex);

            ++m_CurrentDraw;
        }
    }
}
