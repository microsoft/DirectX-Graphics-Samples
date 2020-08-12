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

#include "stdafx.h"
#include "VariableRateShadingScene.h"
#include <dxgidebug.h>
#include "VariableRateShadingFrameResource.h"
#include "DXSample.h"

using namespace std;
using namespace SceneEnums;

VariableRateShadingScene::VariableRateShadingScene(UINT frameCount, DXSample* pSample)
    : ShadowsFogScatteringSquidScene(frameCount, pSample)
    , m_refractionScale(0.01f)
    , m_shadingRateTier(D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED)
    , m_refractionShadingRate(D3D12_SHADING_RATE_1X1)
    , m_sceneShadingRate(D3D12_SHADING_RATE_1X1)
    , m_postprocessShadingRate(D3D12_SHADING_RATE_1X1)
{
    m_fogDensity = 0.015f;
}

VariableRateShadingScene::~VariableRateShadingScene()
{
}

void VariableRateShadingScene::Initialize(ID3D12Device* pDevice, ID3D12CommandQueue* pDirectCommandQueue, ID3D12GraphicsCommandList* pCommandList, UINT frameIndex)
{
    // Check that variable rate shading is supported.
    D3D12_FEATURE_DATA_D3D12_OPTIONS6 options = {};
    if (SUCCEEDED(pDevice->CheckFeatureSupport(
        D3D12_FEATURE_D3D12_OPTIONS6,
        &options,
        sizeof(options))))
    {
        m_shadingRateTier = options.VariableShadingRateTier;
    }

    // For now, disable variable rate shading when running under PIX.
    // This is only needed until PIX supports VRS.
    IID graphicsAnalysisID;
    if (SUCCEEDED(IIDFromString(L"{9F251514-9D4D-4902-9D60-18988AB7D4B5}", &graphicsAnalysisID)))
    {
        ComPtr<IUnknown> graphicsAnalysis;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, graphicsAnalysisID, &graphicsAnalysis)))
        {
            // Running under PIX.
            m_shadingRateTier = D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED;
        }
    }

    // Initialize the base scene.
    ShadowsFogScatteringSquidScene::Initialize(pDevice, pDirectCommandQueue, pCommandList, frameIndex);
}

void VariableRateShadingScene::LoadSizeDependentResources(ID3D12Device* pDevice, ComPtr<ID3D12Resource>* ppRenderTargets, UINT width, UINT height)
{
    // Load base resources.
    ShadowsFogScatteringSquidScene::LoadSizeDependentResources(pDevice, ppRenderTargets, width, height);

    // Create the refraction map texture and associated descriptors.
    {
        // Describe and create the refraction map texture.
        CD3DX12_RESOURCE_DESC refractionTexDesc(
            D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            0,
            static_cast<UINT>(width),
            static_cast<UINT>(height),
            1,
            1,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            1,
            0,
            D3D12_TEXTURE_LAYOUT_UNKNOWN,
            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

        ThrowIfFailed(pDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &refractionTexDesc,
            D3D12_RESOURCE_STATE_COMMON,
            &CD3DX12_CLEAR_VALUE(refractionTexDesc.Format, s_clearColor), // Performance tip: Tell the runtime at resource creation the desired clear value.
            IID_PPV_ARGS(&m_refractionTexture)));
        NAME_D3D12_OBJECT(m_refractionTexture);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCpuHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), ShadowsFogScatteringSquidScene::GetNumRtvDescriptors(), m_rtvDescriptorSize);
        pDevice->CreateRenderTargetView(m_refractionTexture.Get(), nullptr, rtvCpuHandle);
        m_refractionMapCpuHandle = rtvCpuHandle;
    }

    // Create shadow + refraction shader resource views (SRVs).
    {
        // Get a handle to the start of the descriptor heap.
        const UINT shadowDescriptorOffset = ShadowsFogScatteringSquidScene::GetNumCbvSrvUavDescriptors() + _countof(SampleAssets::Glass::Textures);
        CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle(m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart(), shadowDescriptorOffset, m_cbvSrvDescriptorSize);
        CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle(m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart(), shadowDescriptorOffset, m_cbvSrvDescriptorSize);

        // Create the shadow depth SRV descriptor again and move to the next descriptor slot.
        // Note we create this descriptor again and cache this offset which is used 
        // when binding both the shadow map + refraction map.
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        pDevice->CreateShaderResourceView(m_depthTextures[DepthGenPass::Shadow].Get(), &srvDesc, cbvSrvCpuHandle);
        m_shadowMapRefractionMapSrvGpuHandle = cbvSrvGpuHandle;
        cbvSrvCpuHandle.Offset(m_cbvSrvDescriptorSize);
        cbvSrvGpuHandle.Offset(m_cbvSrvDescriptorSize);

        // Describe and create a shader resource view (SRV) for the refraction map
        // texture and cache the GPU descriptor handle. This SRV is for sampling
        // the refraction map from our shader. It uses the same texture that we use
        // as a render target during the refraction pass.
        D3D12_SHADER_RESOURCE_VIEW_DESC refractionSrvDesc = {};
        refractionSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        refractionSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        refractionSrvDesc.Texture2D.MipLevels = 1;
        refractionSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        pDevice->CreateShaderResourceView(m_refractionTexture.Get(), &refractionSrvDesc, cbvSrvCpuHandle);
    }
}

void VariableRateShadingScene::ReleaseSizeDependentResources()
{
    // Release base resources.
    ShadowsFogScatteringSquidScene::ReleaseSizeDependentResources();

    m_refractionTexture.Reset();
}

float VariableRateShadingScene::GetRefractionPassGPUTimeInMs() const
{
    return m_pCurrentFrameResource->m_gpuTimer.GetAverageMS(VRSSceneEnums::Timestamp::RefractionPass);
}

void VariableRateShadingScene::UpdateConstantBuffers()
{
    const float glassZ = SampleAssets::Glass::zPosition * GetWorldScale();

    // Update the shadow copies of the base scene's constant buffers.
    {
        ShadowsFogScatteringSquidScene::UpdateConstantBuffers();

        // Set the clip plane for the scene constant buffer.
        // Clip a different side of plane depending on which direction the camera is looking.
        // This is used to limit rasterization during the refraction pass.
        XMFLOAT3 cameraPosition;
        XMStoreFloat3(&cameraPosition, m_camera.mEye);
        const float sign = (glassZ > cameraPosition.z) ? 1.0f : -1.0f; // Which side of the plane to clip.
        m_sceneConstantBuffer.clipPlane = { 0.0f, 0.0f, sign, -sign * glassZ };
    }

    // Update shadow copy of the glass constant buffer.
    {
        m_glassConstantBuffer.refractionScale = m_refractionScale;

        // Flip the glass's normal depending on which side of the glass the light is on.
        const float& lightZ = m_lights[0].position.z;
        m_glassConstantBuffer.flipNormal = (lightZ < glassZ) ? false : true;
    }
}

void VariableRateShadingScene::CommitConstantBuffers()
{
    // Commit the base scene's constant buffers.
    ShadowsFogScatteringSquidScene::CommitConstantBuffers();

    // Commit the glass constant buffer.
    VariableRateShadingFrameResource* pCurrentFrameResource = static_cast<VariableRateShadingFrameResource*>(m_pCurrentFrameResource);
    memcpy(pCurrentFrameResource->m_pGlassConstantBufferWO, &m_glassConstantBuffer, sizeof(m_glassConstantBuffer));
}

void VariableRateShadingScene::CreateRootSignatures(ID3D12Device* pDevice)
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

    // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if (FAILED(pDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    // Create a root signature for the shadow pass.
    {
        CD3DX12_ROOT_PARAMETER1 rootParameters[1]; // Performance tip: Order root parameters from most frequently accessed to least frequently accessed.
        rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX); // 1 frequently changed constant buffer.

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | // Performance tip: Limit root signature access when possible.
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS );

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignatures[RootSignature::ShadowPass])));
        NAME_D3D12_OBJECT(m_rootSignatures[RootSignature::ShadowPass]);
    }

    // Create a root signature for the scene pass.
    {
        CD3DX12_DESCRIPTOR_RANGE1 ranges[3];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 2);
        ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 2, 0);

        CD3DX12_ROOT_PARAMETER1 rootParameters[5]; // Performance tip: Order root parameters from most frequently accessed to least frequently accessed.
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL); // 2 frequently changed diffuse + normal textures - starting in register t0.
        rootParameters[1].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL); // 1 frequently changed constant buffer.
        rootParameters[2].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL); // 1 frequently changed constant buffer.
        rootParameters[3].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL); // 2 infrequently changed shadow + refraction textures - starting in register t2.
        rootParameters[4].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL); // 2 static samplers.

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | // Performance tip: Limit root signature access when possible.
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignatures[RootSignature::ScenePass])));
        NAME_D3D12_OBJECT(m_rootSignatures[RootSignature::ScenePass]);
    }

    // Create a root signature for the post-process pass.
    {
        CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

        CD3DX12_ROOT_PARAMETER1 rootParameters[3];
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL); // 1 depth texture - starting in register t0.
        rootParameters[1].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL); // 1 frequently changed constant buffer.
        rootParameters[2].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL); // 1 static sampler.

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignatures[RootSignature::PostprocessPass])));
        NAME_D3D12_OBJECT(m_rootSignatures[RootSignature::PostprocessPass]);
    }
}

void VariableRateShadingScene::CreatePipelineStates(ID3D12Device* pDevice)
{
    // Create base pipeline states.
    ShadowsFogScatteringSquidScene::CreatePipelineStates(pDevice);

    // QueryInterface for ID3D12Device2, which supports PSO streams.
    // Note: This is expected to work because the sample reqires even newer interfaces for VRS.
    ComPtr<ID3D12Device2> pDevice2;
    ThrowIfFailed(pDevice->QueryInterface(IID_PPV_ARGS(&pDevice2)));

    // Create PSO subobjects.
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
    inputLayoutDesc.pInputElementDescs = SampleAssets::StandardVertexDescription;
    inputLayoutDesc.NumElements = _countof(SampleAssets::StandardVertexDescription);

    CD3DX12_RASTERIZER_DESC rasterizerDepthBias(D3D12_DEFAULT);
    rasterizerDepthBias.DepthBias = 3; // Avoid depth artifacts when filling the scene depth buffer during the refraction pass.

    CD3DX12_RASTERIZER_DESC rasterizerCullModeNone(D3D12_DEFAULT);
    rasterizerCullModeNone.CullMode = D3D12_CULL_MODE_NONE;

    // Pipeline state for rendering the glass during the shadow pass.
    // Render both 'sides' of glass (no culling).
    {
        ComPtr<ID3DBlob> vertexShader;
        vertexShader = CompileShader(m_pSample->GetAssetFullPath(L"ShadowsAndScenePass.hlsl").c_str(), nullptr, "VSMain", "vs_5_0");

        // Describe the PSO stream.
        struct SHADOWMAP_PIPELINE_STATE_STREAM
        {
            CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
            CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
            CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
            CD3DX12_PIPELINE_STATE_STREAM_VS VS;
            CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
            CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER RasterizerState;
        } stream;
        const D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = { sizeof(stream), &stream };

        stream.pRootSignature = CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE(m_rootSignatures[RootSignature::ShadowPass].Get());
        stream.InputLayout = CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT(inputLayoutDesc);
        stream.PrimitiveTopologyType = CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
        stream.VS = CD3DX12_PIPELINE_STATE_STREAM_VS(CD3DX12_SHADER_BYTECODE(vertexShader.Get()));
        stream.DSVFormat = CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT(DXGI_FORMAT_D32_FLOAT);
        stream.RasterizerState = CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER(rasterizerCullModeNone);
        ThrowIfFailed(pDevice2->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_glassPipelineStates[RenderPass::Shadow])));
        NAME_D3D12_OBJECT(m_glassPipelineStates[RenderPass::Shadow]);
    }

    // Pipeline state for rendering the refraction map.
    // Use a clip plane to limit rasterized objects.
    {
        const D3D_SHADER_MACRO macros[] = {
            "WITH_CLIPDISTANCE", "1",
            nullptr, nullptr,
        };

        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;
        vertexShader = CompileShader(m_pSample->GetAssetFullPath(L"ShadowsAndScenePass.hlsl").c_str(), macros, "VSMain", "vs_5_0");
        pixelShader = CompileShader(m_pSample->GetAssetFullPath(L"ShadowsAndScenePass.hlsl").c_str(), macros, "PSMain", "ps_5_0");
    
        // Describe the PSO stream.
        struct REFRACTIONMAP_PIPELINE_STATE_STREAM
        {
            CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
            CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
            CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
            CD3DX12_PIPELINE_STATE_STREAM_VS VS;
            CD3DX12_PIPELINE_STATE_STREAM_PS PS;
            CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
            CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER RasterizerState;
            CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        } stream;
        const D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = { sizeof(stream), &stream };

        D3D12_RT_FORMAT_ARRAY renderTargets = {};
        renderTargets.NumRenderTargets = 1;
        renderTargets.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

        stream.pRootSignature = CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE(m_rootSignatures[RootSignature::ScenePass].Get());
        stream.InputLayout = CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT(inputLayoutDesc);
        stream.PrimitiveTopologyType = CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
        stream.VS = CD3DX12_PIPELINE_STATE_STREAM_VS(CD3DX12_SHADER_BYTECODE(vertexShader.Get()));
        stream.PS = CD3DX12_PIPELINE_STATE_STREAM_PS(CD3DX12_SHADER_BYTECODE(pixelShader.Get()));
        stream.DSVFormat = CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT(DXGI_FORMAT_D32_FLOAT);
        stream.RasterizerState = CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER(rasterizerDepthBias);
        stream.RTVFormats = CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS(renderTargets);
        ThrowIfFailed(pDevice2->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_refractionPassPipelineState)));
        NAME_D3D12_OBJECT(m_refractionPassPipelineState);
    }

    // Pipeline state for rendering the glass during the scene pass.
    // Render both 'sides' of glass (no culling) and samples from the refraction map.
    {
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;
        vertexShader = CompileShader(m_pSample->GetAssetFullPath(L"Glass.hlsl").c_str(), nullptr, "VSMain", "vs_5_0");
        pixelShader = CompileShader(m_pSample->GetAssetFullPath(L"Glass.hlsl").c_str(), nullptr, "PSMain", "ps_5_0");

        // Describe the PSO stream.
        struct SCENE_PIPELINE_STATE_STREAM
        {
            CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
            CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
            CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
            CD3DX12_PIPELINE_STATE_STREAM_VS VS;
            CD3DX12_PIPELINE_STATE_STREAM_PS PS;
            CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
            CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER RasterizerState;
            CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        } stream;
        const D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = { sizeof(stream), &stream };

        D3D12_RT_FORMAT_ARRAY renderTargets = {};
        renderTargets.NumRenderTargets = 1;
        renderTargets.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

        stream.pRootSignature = CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE(m_rootSignatures[RootSignature::ScenePass].Get());
        stream.InputLayout = CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT(inputLayoutDesc);
        stream.PrimitiveTopologyType = CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
        stream.VS = CD3DX12_PIPELINE_STATE_STREAM_VS(CD3DX12_SHADER_BYTECODE(vertexShader.Get()));
        stream.PS = CD3DX12_PIPELINE_STATE_STREAM_PS(CD3DX12_SHADER_BYTECODE(pixelShader.Get()));
        stream.DSVFormat = CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT(DXGI_FORMAT_D32_FLOAT);
        stream.RasterizerState = CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER(rasterizerCullModeNone);
        stream.RTVFormats = CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS(renderTargets);
        ThrowIfFailed(pDevice2->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_glassPipelineStates[RenderPass::Scene])));
        NAME_D3D12_OBJECT(m_glassPipelineStates[RenderPass::Scene]);
    }
}

void VariableRateShadingScene::CreateFrameResources(ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue)
{
    for (UINT i = 0; i < m_frameCount; i++)
    {
        m_frameResources[i] = make_unique<VariableRateShadingFrameResource>(pDevice, pCommandQueue);
    }
}

void VariableRateShadingScene::CreateAssetResources(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList)
{
    // Create base asset resources.
    ShadowsFogScatteringSquidScene::CreateAssetResources(pDevice, pCommandList);

    // Load glass texture data.
    {
        UINT8* pGlassAssetData[_countof(SampleAssets::Glass::TextureFileNames)];
        for (UINT i = 0; i < _countof(pGlassAssetData); i++)
        {
            UINT fileSize = 0;
            UINT dataOffset = 0;
            ThrowIfFailed(ReadDataFromDDSFile(m_pSample->GetAssetFullPath(SampleAssets::Glass::TextureFileNames[i]).c_str(), &pGlassAssetData[i], &dataOffset, &fileSize));
            
            assert(dataOffset == SampleAssets::Glass::TextureOffset);
            pGlassAssetData[i] += SampleAssets::Glass::TextureOffset;
        }

        // Create shader resources.
        {
            // Get a handle to the start of the descriptor heap.
            CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle(m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart(), ShadowsFogScatteringSquidScene::GetNumCbvSrvUavDescriptors(), m_cbvSrvDescriptorSize);
            CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle(m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart(), ShadowsFogScatteringSquidScene::GetNumCbvSrvUavDescriptors(), m_cbvSrvDescriptorSize);

            // Create each texture and SRV descriptor.
            PIXBeginEvent(pCommandList, 0, L"Copy diffuse and normal texture data to glass resources...");
            for (UINT i = 0; i < _countof(SampleAssets::Glass::Textures); i++)
            {
                // Describe and create a Texture2D.
                const SampleAssets::TextureResource &tex = SampleAssets::Glass::Textures[i];
                CD3DX12_RESOURCE_DESC texDesc(
                    D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                    0,
                    tex.Width,
                    tex.Height,
                    1,
                    static_cast<UINT16>(tex.MipLevels),
                    tex.Format,
                    1,
                    0,
                    D3D12_TEXTURE_LAYOUT_UNKNOWN,
                    D3D12_RESOURCE_FLAG_NONE);

                ThrowIfFailed(pDevice->CreateCommittedResource(
                    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                    D3D12_HEAP_FLAG_NONE,
                    &texDesc,
                    D3D12_RESOURCE_STATE_COPY_DEST,
                    nullptr,
                    IID_PPV_ARGS(&m_glassTextures[i])));
                NAME_D3D12_OBJECT_INDEXED(m_glassTextures, i);

                {
                    const UINT subresourceCount = texDesc.DepthOrArraySize * texDesc.MipLevels;
                    UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_glassTextures[i].Get(), 0, subresourceCount);
                    ThrowIfFailed(pDevice->CreateCommittedResource(
                        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                        D3D12_HEAP_FLAG_NONE,
                        &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                        D3D12_RESOURCE_STATE_GENERIC_READ,
                        nullptr,
                        IID_PPV_ARGS(&m_glassTextureUploads[i])));

                    // Copy data to the intermediate upload heap and then schedule a copy
                    // from the upload heap to the Texture2D.
                    D3D12_SUBRESOURCE_DATA textureData = {};
                    textureData.pData = pGlassAssetData[i] + tex.Data->Offset;
                    textureData.RowPitch = tex.Data->Pitch;
                    textureData.SlicePitch = tex.Data->Size;

                    UpdateSubresources(pCommandList, m_glassTextures[i].Get(), m_glassTextureUploads[i].Get(), 0, 0, subresourceCount, &textureData);

                    // Performance tip: You can avoid some resource barriers by relying on resource state promotion and decay.
                    // Resources accessed on a copy queue will decay back to the COMMON after ExecuteCommandLists()
                    // completes on the GPU. Search online for "D3D12 Implicit State Transitions" for more details. 
                }

                // Describe and create an SRV.
                D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                srvDesc.Format = tex.Format;
                srvDesc.Texture2D.MipLevels = tex.MipLevels;
                srvDesc.Texture2D.MostDetailedMip = 0;
                srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
                pDevice->CreateShaderResourceView(m_glassTextures[i].Get(), &srvDesc, cbvSrvCpuHandle);
                cbvSrvCpuHandle.Offset(m_cbvSrvDescriptorSize);
                cbvSrvGpuHandle.Offset(m_cbvSrvDescriptorSize);
            }
            PIXEndEvent(pCommandList);
        }
    }

    // Create the glass vertex buffer.
    {
        // Get vertex data for the glass.
        std::vector<SampleAssets::Glass::Vertex> glassVertexData;
        SampleAssets::Glass::GetVertexData(glassVertexData);

        const UINT glassVertexDataSize = static_cast<UINT>(glassVertexData.size() * sizeof(SampleAssets::Glass::Vertex));

        ThrowIfFailed(pDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(glassVertexDataSize),
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_glassVertexBuffer)));
        NAME_D3D12_OBJECT(m_glassVertexBuffer);

        ThrowIfFailed(pDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(glassVertexDataSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_glassVertexBufferUpload)));

        // Copy data to the upload heap and then schedule a copy 
        // from the upload heap to the vertex buffer.
        D3D12_SUBRESOURCE_DATA vertexData = {};
        vertexData.pData = glassVertexData.data();
        vertexData.RowPitch = glassVertexDataSize;
        vertexData.SlicePitch = vertexData.RowPitch;

        PIXBeginEvent(pCommandList, 0, L"Copy glass vertex buffer data to default resource...");

        UpdateSubresources<1>(pCommandList, m_glassVertexBuffer.Get(), m_glassVertexBufferUpload.Get(), 0, 0, 1, &vertexData);

        // Performance tip: You can avoid some resource barriers by relying on resource state promotion and decay.
        // Resources accessed on a copy queue will decay back to the COMMON after ExecuteCommandLists()
        // completes on the GPU. Search online for "D3D12 Implicit State Transitions" for more details. 

        PIXEndEvent(pCommandList);

        // Initialize the vertex buffer view.
        m_glassVertexBufferView.BufferLocation = m_glassVertexBuffer->GetGPUVirtualAddress();
        m_glassVertexBufferView.SizeInBytes = glassVertexDataSize;
        m_glassVertexBufferView.StrideInBytes = SampleAssets::StandardVertexStride;
    }
}

void VariableRateShadingScene::MidFrame()
{
    ID3D12GraphicsCommandList* pCommandListMid = m_commandLists[CommandListMid].Get();

    // Reset the command list.
    ThrowIfFailed(pCommandListMid->Reset(m_pCurrentFrameResource->m_commandAllocator.Get(), nullptr));

    // Set descriptor heaps.
    ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvHeap.Get(), m_samplerHeap.Get() };
    pCommandListMid->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    // Transition our shadow map to a readable state for refraction and scene rendering.
    // Transition our refraction map to a writable state.
    // Performance tip: Batch resource barriers into as few API calls as possible to minimize the amount of work the GPU does.
    D3D12_RESOURCE_BARRIER barriers[2];
    barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_depthTextures[DepthGenPass::Shadow].Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_refractionTexture.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    pCommandListMid->ResourceBarrier(_countof(barriers), barriers);

    // Clear the refraction map.
    pCommandListMid->ClearRenderTargetView(m_refractionMapCpuHandle, s_clearColor, 0, nullptr);

    PIXBeginEvent(pCommandListMid, 0, L"Refraction pass");
    m_pCurrentFrameResource->m_gpuTimer.Start(pCommandListMid, VRSSceneEnums::Timestamp::RefractionPass);
    RefractionPass(pCommandListMid);
    m_pCurrentFrameResource->m_gpuTimer.Stop(pCommandListMid, VRSSceneEnums::Timestamp::RefractionPass);
    PIXEndEvent(pCommandListMid);

    // Transition refraction map to a readable state for scene rendering.
    pCommandListMid->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_refractionTexture.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    // Close the command list.
    ThrowIfFailed(pCommandListMid->Close());
}

void VariableRateShadingScene::RefractionPass(ID3D12GraphicsCommandList* pCommandList)
{
    //
    // The refraction pass is similar to the scene pass, but has a clip plane enabled and render to an intermediate texture.
    //

    // Set necessary state.
    pCommandList->SetGraphicsRootSignature(m_rootSignatures[RootSignature::ScenePass].Get());
    pCommandList->SetPipelineState(m_refractionPassPipelineState.Get());

    const VariableRateShadingFrameResource* pCurrentFrameResource = static_cast<VariableRateShadingFrameResource*>(m_pCurrentFrameResource);
    pCommandList->SetGraphicsRootConstantBufferView(1, pCurrentFrameResource->GetConstantBufferGPUVirtualAddress(RenderPass::Scene)); // Set scene constant buffer.
    pCommandList->SetGraphicsRootConstantBufferView(2, pCurrentFrameResource->GetGlassConstantBufferGPUVirtualAddress());   // Set glass constant buffer.
    pCommandList->SetGraphicsRootDescriptorTable(3, m_depthSrvGpuHandles[DepthGenPass::Shadow]); // Set the shadow texture as an SRV.
    pCommandList->SetGraphicsRootDescriptorTable(4, m_samplerHeap->GetGPUDescriptorHandleForHeapStart()); // Set samplers.
  
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferViews[VertexBuffer::SceneGeometry]);
    pCommandList->IASetIndexBuffer(&m_indexBufferView);
    pCommandList->RSSetViewports(1, &m_viewport);
    pCommandList->RSSetScissorRects(1, &m_scissorRect);
    pCommandList->OMSetRenderTargets(1, &m_refractionMapCpuHandle, FALSE, &m_depthDsvs[DepthGenPass::Scene]); // Set the refraction map as the target.

    // Set the shading rate.
    if (m_shadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_1)
    {
        reinterpret_cast<ID3D12GraphicsCommandList5*>(pCommandList)->RSSetShadingRate(m_refractionShadingRate, nullptr);
    }

    // Draw.
    const D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvHeapStart = m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart();
    for (int j = 0; j < _countof(SampleAssets::Draws); j++)
    { 
        const SampleAssets::DrawParameters& drawArgs = SampleAssets::Draws[j];

        // Set the diffuse and normal textures for the current object.
        const CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvHandle(cbvSrvHeapStart, NumNullSrvs + _countof(m_depthSrvCpuHandles) + drawArgs.DiffuseTextureIndex, m_cbvSrvDescriptorSize);
        pCommandList->SetGraphicsRootDescriptorTable(0, cbvSrvHandle);

        pCommandList->DrawIndexedInstanced(drawArgs.IndexCount, 1, drawArgs.IndexStart, drawArgs.VertexBase, 0);
    }

    // Transition scene depth to a readable state for post-processing.
    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthTextures[DepthGenPass::Scene].Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    // Set necessary state.
    pCommandList->SetGraphicsRootSignature(m_rootSignatures[RootSignature::PostprocessPass].Get());
    pCommandList->SetPipelineState(m_pipelineStates[RenderPass::Postprocess].Get());

    // Draw.
    DrawInScattering(pCommandList, m_refractionMapCpuHandle);

    // Transition scene depth to a writable state again for scene rendering.
    pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthTextures[DepthGenPass::Scene].Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));
}

void VariableRateShadingScene::ScenePass(ID3D12GraphicsCommandList* pCommandList, int threadIndex)
{
    // Set necessary state.
    pCommandList->SetGraphicsRootSignature(m_rootSignatures[RootSignature::ScenePass].Get());

    const VariableRateShadingFrameResource* pCurrentFrameResource = static_cast<VariableRateShadingFrameResource*>(m_pCurrentFrameResource);
    pCommandList->SetGraphicsRootConstantBufferView(1, pCurrentFrameResource->GetConstantBufferGPUVirtualAddress(RenderPass::Scene)); // Set scene constant buffer.
    pCommandList->SetGraphicsRootConstantBufferView(2, pCurrentFrameResource->GetGlassConstantBufferGPUVirtualAddress()); // Set constant buffer with refraction data.
    pCommandList->SetGraphicsRootDescriptorTable(3, m_shadowMapRefractionMapSrvGpuHandle); // Set the shadow texture and refraction texture as SRVs.
    pCommandList->SetGraphicsRootDescriptorTable(4, m_samplerHeap->GetGPUDescriptorHandleForHeapStart()); // Set samplers.

    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferViews[VertexBuffer::SceneGeometry]);
    pCommandList->IASetIndexBuffer(&m_indexBufferView);
    pCommandList->RSSetViewports(1, &m_viewport);
    pCommandList->RSSetScissorRects(1, &m_scissorRect);
    pCommandList->OMSetRenderTargets(1, &GetCurrentBackBufferRtvCpuHandle(), FALSE, &m_depthDsvs[DepthGenPass::Scene]);

    // Set the shading rate.
    if (m_shadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_1)
    {
        reinterpret_cast<ID3D12GraphicsCommandList5*>(pCommandList)->RSSetShadingRate(m_sceneShadingRate, nullptr);
    }

    // Draw.
    const D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvHeapStart = m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart();
    for (int j = threadIndex; j < _countof(SampleAssets::Draws); j += NumContexts)
    {
        const SampleAssets::DrawParameters& drawArgs = SampleAssets::Draws[j];

        // Set the diffuse and normal textures for the current object.
        const CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvHandle(cbvSrvHeapStart, NumNullSrvs + _countof(m_depthSrvCpuHandles) + drawArgs.DiffuseTextureIndex, m_cbvSrvDescriptorSize);
        pCommandList->SetGraphicsRootDescriptorTable(0, cbvSrvHandle);

        pCommandList->DrawIndexedInstanced(drawArgs.IndexCount, 1, drawArgs.IndexStart, drawArgs.VertexBase, 0);
    }

    // Use the last thread to render the glass pane.
    if (threadIndex == (NumContexts - 1))
    {
        // Set necessary state.
        pCommandList->SetPipelineState(m_glassPipelineStates[RenderPass::Scene].Get());

        pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        pCommandList->IASetVertexBuffers(0, 1, &m_glassVertexBufferView);

        // Draw.
        const UINT glassDescriptorOffset = ShadowsFogScatteringSquidScene::GetNumCbvSrvUavDescriptors();
        for (int j = 0; j < _countof(SampleAssets::Glass::Draws); j++)
        {
            SampleAssets::DrawParameters drawArgs = SampleAssets::Glass::Draws[j];

            // Set the diffuse and normal textures for the current object.
            const CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvHandle(cbvSrvHeapStart, glassDescriptorOffset + drawArgs.DiffuseTextureIndex, m_cbvSrvDescriptorSize);
            pCommandList->SetGraphicsRootDescriptorTable(0, cbvSrvHandle);

            pCommandList->DrawInstanced(4, 1, 0, 0);
        }
    }
}

void VariableRateShadingScene::PostprocessPass(ID3D12GraphicsCommandList* pCommandList)
{
    // Set the shading rate.
    if (m_shadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_1)
    {
        reinterpret_cast<ID3D12GraphicsCommandList5*>(pCommandList)->RSSetShadingRate(m_postprocessShadingRate, nullptr);
    }

    ShadowsFogScatteringSquidScene::PostprocessPass(pCommandList);
}