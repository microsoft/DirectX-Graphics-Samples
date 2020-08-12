//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// ShadowsFogScatteringSquidScene
// Renders a scene with a squid and illumination from a point light. A postprocess pass 
// applies a fog effect and an analytical in-scattering model. 
// Uses multithreading to draw scene geometry.
// Limitations\future work: 
// - shadow for a point light is generated for one face only - i.e. same as for a spotlight.
// - no volumetric shadows

#pragma once

#include "ShadowsFogScatteringSquidScene.h"

namespace SampleAssets
{
    namespace Glass
    {
        static const UINT TextureOffset = 128;    // Offset in the file where the data starts.

        static const wchar_t* TextureFileNames[] =
        {
            L"glass_xii_diffuse.dds",
            L"glass_xii_normal.dds",
        };

        static const TextureResource Textures[] =
        {
            { 1024,   1024,   1,       DXGI_FORMAT_BC1_UNORM,{ { 0, 524288, 2048 }, } }, // Glass diffuse dds
            { 1024,   1024,   1,       DXGI_FORMAT_BC1_UNORM,{ { 0, 524288, 2048 }, } }, // Glass normal dds
        };

        static const DrawParameters Draws[] =
        {
            { 0,   1,  -1,        0,    4,        0 },
        };

        struct Vertex
        {
            XMFLOAT3 position;
            XMFLOAT3 normal;
            XMFLOAT2 texcoord;
            XMFLOAT3 tangent;
        };

        const float zPosition = -150.0f;
        static void GetVertexData(std::vector<Vertex>& vertexData)
        {
            const float x = -150.0f;
            const float y = 50.0f;
            const float z = zPosition;
            const float scale = 1.1f;
            const float width = 266.66f * scale;
            const float height = 150.0f * scale;

            vertexData.empty();
            vertexData.resize(4);

            vertexData[0].position = { x, y, z };
            vertexData[1].position = { x, y + height, z };
            vertexData[2].position = { x + width, y, z };
            vertexData[3].position = { x + width, y + height, z };

            vertexData[0].texcoord = { 1.0f, 0.0f };
            vertexData[1].texcoord = { 1.0f, 1.0f };
            vertexData[2].texcoord = { 0.0f, 0.0f };
            vertexData[3].texcoord = { 0.0f, 1.0f };

            vertexData[0].normal = { 0.0f, 0.0f, 1.0f };
            vertexData[1].normal = { 0.0f, 0.0f, 1.0f };
            vertexData[2].normal = { 0.0f, 0.0f, 1.0f };
            vertexData[3].normal = { 0.0f, 0.0f, 1.0f };

            vertexData[0].tangent = { 1.0f, 1.0f, 0.0f };
            vertexData[1].tangent = { 1.0f, 1.0f, 0.0f };
            vertexData[2].tangent = { 1.0f, 1.0f, 0.0f };
            vertexData[3].tangent = { 1.0f, 1.0f, 0.0f };
        }
    }
}

namespace VRSSceneEnums
{
    namespace Timestamp {
        enum Value { RefractionPass = SceneEnums::Timestamp::Count, Count };
    }
}

struct GlassConstantBuffer
{
    float refractionScale;
    bool flipNormal;
};

class VariableRateShadingScene : public ShadowsFogScatteringSquidScene
{
public:
    VariableRateShadingScene(UINT frameCount, DXSample* pSample);
    virtual ~VariableRateShadingScene();

    virtual void Initialize(ID3D12Device* pDevice, ID3D12CommandQueue* pDirectCommandQueue, ID3D12GraphicsCommandList* pCommandList, UINT frameIndex) override;
    virtual void LoadSizeDependentResources(ID3D12Device* pDevice, ComPtr<ID3D12Resource>* ppRenderTargets, UINT width, UINT height) override;
    virtual void ReleaseSizeDependentResources() override;

    float m_refractionScale;
    D3D12_VARIABLE_SHADING_RATE_TIER m_shadingRateTier;
    D3D12_SHADING_RATE m_refractionShadingRate;
    D3D12_SHADING_RATE m_sceneShadingRate;
    D3D12_SHADING_RATE m_postprocessShadingRate;

    float GetRefractionPassGPUTimeInMs() const;

protected:
    // D3D objects.
    ComPtr<ID3D12PipelineState> m_refractionPassPipelineState;
    ComPtr<ID3D12PipelineState> m_glassPipelineStates[SceneEnums::RenderPass::Count];
    ComPtr<ID3D12Resource> m_refractionTexture;
    ComPtr<ID3D12Resource> m_glassTextures[_countof(SampleAssets::Glass::Textures)];
    ComPtr<ID3D12Resource> m_glassTextureUploads[_countof(SampleAssets::Glass::Textures)];
    ComPtr<ID3D12Resource> m_glassVertexBuffer;
    ComPtr<ID3D12Resource> m_glassVertexBufferUpload;
    D3D12_VERTEX_BUFFER_VIEW m_glassVertexBufferView;

    // Heap objects.
    D3D12_CPU_DESCRIPTOR_HANDLE m_refractionMapCpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE m_shadowMapRefractionMapSrvGpuHandle;

    // Frame resources.
    GlassConstantBuffer m_glassConstantBuffer; // Shadow copy.

    virtual void UpdateConstantBuffers() override; // Updates the shadow copies of the constant buffers.
    virtual void CommitConstantBuffers() override; // Commits the shadows copies of the constant buffers to GPU-visible memory for the current frame.

    void RefractionPass(ID3D12GraphicsCommandList* pCommandList);
    virtual void ScenePass(ID3D12GraphicsCommandList* pCommandList, int threadIndex) override;
    virtual void PostprocessPass(ID3D12GraphicsCommandList* pCommandList) override;
    virtual void MidFrame() override;
    virtual void CreateRootSignatures(ID3D12Device* pDevice) override;
    virtual void CreatePipelineStates(ID3D12Device* pDevice) override;
    virtual void CreateFrameResources(ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue) override;
    virtual void CreateAssetResources(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) override;

    virtual UINT GetNumRtvDescriptors() const override
    {
        // Original number of descriptors + refraction map.
        return ShadowsFogScatteringSquidScene::GetNumRtvDescriptors() + 1;
    }

    virtual UINT GetNumCbvSrvUavDescriptors() const override
    {
        // Original number of descriptors + glass textures + shadow map (again) + refraction map.
        return ShadowsFogScatteringSquidScene::GetNumCbvSrvUavDescriptors() + _countof(SampleAssets::Glass::Textures) + 1 + 1;
    }
};