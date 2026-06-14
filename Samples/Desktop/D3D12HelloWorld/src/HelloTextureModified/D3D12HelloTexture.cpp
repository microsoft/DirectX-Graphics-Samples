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

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

#include "D3D12HelloTexture.h"

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <DirectXPackedVector.h>
#include <windows.h>

#include "MyDx12Utils.h"
#include "Renderer/DebugDumpReport.h"
#include "Renderer/RootSignatureFactory.h"

#include <pix3.h>

// ImGui
#define IMGUI_IMPL 1

#include <random>

extern "C"
{
    __declspec(dllexport) extern const UINT D3D12SDKVersion = 618;
}
extern "C"
{
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

HelloTextureEngine::HelloTextureEngine(GraphicsDevice& graphicsDevice)
    : m_graphicsDevice(graphicsDevice), m_width(0), m_height(0), m_aspectRatio(1.0f),
      m_previousFrameIndex(0), m_currentFrameIndex(0), m_rtvDescriptorSize(0)
{
    WCHAR assetsPath[512];
    GetAssetsPath(assetsPath, _countof(assetsPath));
    m_assetsPath = assetsPath;

    RegisterPassBindingResolvers();
    RegisterPassConstantsHandlers();
    RegisterResourceResolvers();
}

void HelloTextureEngine::Initialize(UINT width, UINT height)
{
    assert(m_graphicsDevice.Device() != nullptr);
    assert(m_graphicsDevice.HasSwapChain());

    m_width = width;
    m_height = height;
    m_aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
    m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
    m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));

    InitializeFrameResources();
}

void HelloTextureEngine::InitializeFrameResources()
{
    m_prevTime = std::chrono::steady_clock::now();
    LoadPipeline();
    LoadAssets();
    InitImGui();
    InitResourceDefaultStates();
}

void HelloTextureEngine::InitResourceDefaultStates()
{
    m_resourceDefaultStates.clear();
    m_resourceDefaultStates.push_back({kBackBufferResourceName, D3D12_RESOURCE_STATE_PRESENT});
    m_resourceDefaultStates.push_back({kDepthStencilResourceName, D3D12_RESOURCE_STATE_DEPTH_WRITE});
    m_resourceDefaultStates.push_back({kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET});
    for (UINT i = 0; i < Engine::GBuffer::kCount; ++i)
    {
        m_resourceDefaultStates.push_back({kGBufferResourceNames[i], D3D12_RESOURCE_STATE_RENDER_TARGET});
    }
}

std::wstring HelloTextureEngine::GetAssetFullPath(LPCWSTR assetName)
{
    return m_assetsPath + assetName;
}

void HelloTextureEngine::SetDebugUiHandler(DebugUiHandler handler)
{
    m_debugUiHandler = std::move(handler);
}

void HelloTextureEngine::SetUpdateHandler(UpdateHandler handler)
{
    m_updateHandler = std::move(handler);
}

void HelloTextureEngine::SetLightingParams(const LightingParams& params)
{
    m_lightingParams = params;
}

void HelloTextureEngine::SetMaterialParams(UINT materialIndex, const MaterialParams& params)
{
    if (materialIndex >= m_materialData.size())
    {
        return;
    }

    Engine::Material& material = m_materialData[materialIndex];
    material.roughnessFactor = params.roughnessFactor;
    material.metallicFactor = params.metallicFactor;
    material.ambientOcclusionFactor = params.ambientOcclusionFactor;
    material.emissiveScale = params.emissiveScale;
    m_materialBuffer.Update(m_materialData);
}

auto HelloTextureEngine::MakeLightingConstants() const -> LightingConstants
{
    return {
        m_lightingParams.lightDirection,
        m_lightingParams.iblIntensity,
        m_lightingParams.lightColor,
        m_lightingParams.diffuseIntensity,
        {m_backBufferClearColor[0], m_backBufferClearColor[1], m_backBufferClearColor[2], m_backBufferClearColor[3]},
        m_lightingParams.skyboxPreview ? 1.0f : 0.0f,
    };
}

void HelloTextureEngine::SetRenderingPath(RenderingPath renderingPath)
{
    m_renderingPath = renderingPath;
}

void HelloTextureEngine::SetLightingPassDebugGradient(bool enabled)
{
    m_lightingPassDebugGradientEnabled = enabled;
}

void HelloTextureEngine::SetBackBufferClearColor(const std::array<float, 4>& color)
{
    m_backBufferClearColor = color;
}

void HelloTextureEngine::SetScene(const Scene& scene)
{
    m_scene = scene;
}

void HelloTextureEngine::SetDisplayInstanceCount(int count)
{
    m_displayInstanceCount = std::clamp(count, 0, static_cast<int>(kMaxInstanceCount));
}

void HelloTextureEngine::SetToneMapParams(const ToneMapParams& params)
{
    m_toneMapPass.settings.operatorIndex = params.operatorIndex;
    m_toneMapPass.settings.exposure = params.exposure;
    m_toneMapPass.settings.paperWhiteNits = params.paperWhiteNits;
    m_toneMapPass.settings.maxDisplayNits = params.maxDisplayNits;
}

void HelloTextureEngine::SetRenderViewMode(RenderViewMode mode)
{
    m_debugViewSettings.renderViewMode = mode;
}

void HelloTextureEngine::SetRequestHdrDump(bool request)
{
    m_debugViewSettings.requestHdrDump = request;
}

void HelloTextureEngine::UpdateCameraConstantBuffer()
{
    const float aspect = static_cast<float>(m_width) / static_cast<float>(m_height);
    const XMMATRIX rotMat =
        XMMatrixRotationRollPitchYaw(m_scene.camera.rot.x, m_scene.camera.rot.y, m_scene.camera.rot.z);
    const XMMATRIX transMat = XMMatrixTranslation(m_scene.camera.pos.x, m_scene.camera.pos.y, m_scene.camera.pos.z);
    const XMMATRIX view = XMMatrixInverse(nullptr, rotMat * transMat);
    const XMMATRIX projection =
        XMMatrixPerspectiveFovLH(XMConvertToRadians(m_scene.camera.fov), aspect, kCameraNearZ, kCameraFarZ);
    const XMMATRIX viewProjection = XMMatrixMultiply(view, projection);
    XMStoreFloat4x4(&m_constantBufferData.viewProjection, XMMatrixTranspose(viewProjection));
    XMStoreFloat4x4(&m_constantBufferData.invViewProjection,
                    XMMatrixTranspose(XMMatrixInverse(nullptr, viewProjection)));
    m_constantBufferData.cameraPosition = m_scene.camera.pos;
}

// Load the rendering pipeline dependencies.
void HelloTextureEngine::LoadPipeline()
{
    assert(m_graphicsDevice.Device() != nullptr);
    assert(m_graphicsDevice.HasSwapChain());

    UpdateHdr10DisplayMode();
    m_currentFrameIndex = m_graphicsDevice.CurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = kRTVDescriptorCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_graphicsDevice.Device()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        m_rtvDescriptorSize =
            m_graphicsDevice.Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Describe and create a heap for SRV/CBV/UAV
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = kMainHeapDescriptorCount;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_graphicsDevice.Device()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap)));
        // Create a descriptor allocator to manage the descriptors in the heap.
        m_descriptorHeapAllocator.Init(m_graphicsDevice.Device(), m_heap.Get());

        D3D12_DESCRIPTOR_HEAP_DESC imguiHeapDesc = {};
        imguiHeapDesc.NumDescriptors = kHeapDescriptorCount;
        imguiHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        imguiHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_graphicsDevice.Device()->CreateDescriptorHeap(&imguiHeapDesc, IID_PPV_ARGS(&m_imguiHeap)));
        m_ImGuiDescriptorHeapAllocator.Init(m_graphicsDevice.Device(), m_imguiHeap.Get());
    }

    // create render target views (RTVs) for the swap chain back buffers.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < kFrameCount; n++)
        {
            m_graphicsDevice.GetBackBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
            m_graphicsDevice.Device()->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    // Create the depth stencil view.
    {
        CreateDsvHeap();
        RegisterDepthStencil(m_width, m_height);
        RegisterLightPassRenderTarget(m_width, m_height);
    }

    // create command allocators.
    for (UINT n = 0; n < kFrameCount; n++)
    {
        ThrowIfFailed(m_graphicsDevice.Device()->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_frameResources[n].commandAllocator)));
    }

    //
    m_gpuWorkMeter.Init(m_graphicsDevice.Device(),
                        kGpuWorkMeterQueryCount); // Initialize GPU work meter with a maximum of 100 timestamp queries.
}

void HelloTextureEngine::UpdateHdr10DisplayMode()
{
    m_graphicsDevice.EnsureCurrentDxgiFactory();
    m_hdrOutputPolicy.Update(m_graphicsDevice.DxgiFactory(), m_graphicsDevice.SwapChain(), m_graphicsDevice.Hwnd());
}

DescriptorHeapHandle HelloTextureEngine::CreateTextureFromRGBA8(
    const UINT8* pixels, UINT width, UINT height, ComPtr<ID3D12Resource>& texture, ComPtr<ID3D12Resource>& uploadHeap)
{
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    // Create the GPU resource for the texture.
    ThrowIfFailed(m_graphicsDevice.Device()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                                     D3D12_HEAP_FLAG_NONE,
                                                                     &textureDesc,
                                                                     D3D12_RESOURCE_STATE_COPY_DEST,
                                                                     nullptr,
                                                                     IID_PPV_ARGS(&texture)));

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, 1);

    // Create the GPU upload buffer.
    ThrowIfFailed(m_graphicsDevice.Device()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                                     D3D12_HEAP_FLAG_NONE,
                                                                     &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                                                                     D3D12_RESOURCE_STATE_GENERIC_READ,
                                                                     nullptr,
                                                                     IID_PPV_ARGS(&uploadHeap)));

    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = pixels;
    textureData.RowPitch = width * kTexturePixelSize;
    textureData.SlicePitch = textureData.RowPitch * height;

    UpdateSubresources(m_commandList.Get(), texture.Get(), uploadHeap.Get(), 0, 0, 1, &textureData);

    m_commandList->ResourceBarrier(1,
                                   &CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(),
                                                                         D3D12_RESOURCE_STATE_COPY_DEST,
                                                                         D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    return AllocateTextureSRV(texture.Get());
}

void HelloTextureEngine::CreateEnvironmentMapResource(ComPtr<ID3D12Resource>& environmentMapUploadHeap)
{
    m_environmentMap.CreateFromDdsOrProceduralFallback(m_graphicsDevice.Device(),
                                                       m_commandList.Get(),
                                                       m_descriptorHeapAllocator,
                                                       L"Assets\\Environment\\default_environment.dds",
                                                       environmentMapUploadHeap);
}

// Load the sample assets.
void HelloTextureEngine::LoadAssets()
{
    CreateRootSignature();
    CreatePipelineStates();
    CreateGBuffer();
    CreateInitialCommandList();
    CreateSceneGeometryBuffers();

    std::vector<ComPtr<ID3D12Resource>> textureUploadHeap;
    ComPtr<ID3D12Resource> environmentMapUploadHeap;
    CreateSceneTextureResources(textureUploadHeap);
    CreateEnvironmentMapResource(environmentMapUploadHeap);
    PrepareSceneInstanceData();
    CreateSceneMaterialResources();
    CreateInstanceBuffers();
    CreateFrameConstantBuffers();
    ExecuteInitialGpuSetup();
}

void HelloTextureEngine::CreateRootSignature()
{
    Engine::CreateRootSignature(m_graphicsDevice.Device(), kTextureCount, Engine::GBuffer::kCount + 1, m_rootSignature);
}

auto HelloTextureEngine::LoadShaderBytecode(LPCWSTR assetName) -> ShaderBytecode
{
    UINT8* data = nullptr;
    UINT size = 0;
    ThrowIfFailed(ReadDataFromFile(GetAssetFullPath(assetName).c_str(), &data, &size));
    return {data, size};
}

auto HelloTextureEngine::LoadPipelineShaderBytecode() -> PipelineShaderBytecode
{
    PipelineShaderBytecode shaders = {};
    shaders.forward = {LoadShaderBytecode(L"shaders_VSMain.cso"), LoadShaderBytecode(L"shaders_PSMain.cso")};
    shaders.depthPrePass = {LoadShaderBytecode(L"shaders_DepthOnlyVS_VSMain.cso"), {}};
    shaders.gbuffer = {LoadShaderBytecode(L"shaders_GBuffer_VSMain.cso"),
                       LoadShaderBytecode(L"shaders_GBuffer_PSMain.cso")};
    shaders.gbufferDebug = {LoadShaderBytecode(L"shaders_GBufferDebug_VSMain.cso"),
                            LoadShaderBytecode(L"shaders_GBufferDebug_PSMain.cso")};
    shaders.lighting = {LoadShaderBytecode(L"shaders_LightPass_VSMain.cso"),
                        LoadShaderBytecode(L"shaders_LightPass_PSMain.cso")};
    shaders.lightingDebugGradient = {LoadShaderBytecode(L"shaders_LightPassDebugGradient_VSMain.cso"),
                                     LoadShaderBytecode(L"shaders_LightPassDebugGradient_PSMain.cso")};
    shaders.toneMap = {LoadShaderBytecode(L"shaders_ToneMap_VSMain.cso"),
                       LoadShaderBytecode(L"shaders_ToneMap_PSMain.cso")};
    return shaders;
}

void HelloTextureEngine::CreatePipelineStates()
{
    const PipelineShaderBytecode shaders = LoadPipelineShaderBytecode();
    RegisterPipelineStates(shaders);
}

void HelloTextureEngine::RegisterPipelineStates(const PipelineShaderBytecode& shaders)
{
    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

    D3D12_INPUT_ELEMENT_DESC depthLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
    const InputLayoutDefinition meshInputLayout = {inputElementDescs, _countof(inputElementDescs)};
    const InputLayoutDefinition depthInputLayout = {depthLayout, _countof(depthLayout)};

    //
    // Forward Pass PSO
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    RegisterForwardPipeline(
        psoDesc,
        {Pipe::Forward, meshInputLayout, shaders.forward, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D32_FLOAT});

    //
    // GBuffer PSO
    //
    RegisterGBufferPipeline(psoDesc, {Pipe::GBuffer, meshInputLayout, shaders.gbuffer});

    // Light Pass PSO, ToneMap PSO, GBuffer Debug PSO
    RegisterFullscreenPipelines(
        psoDesc,
        {{Pipe::Lighting, shaders.lighting, DXGI_FORMAT_R16G16B16A16_FLOAT},
         {Pipe::LightingDebugGradient, shaders.lightingDebugGradient, DXGI_FORMAT_R16G16B16A16_FLOAT},
         {Pipe::ToneMap, shaders.toneMap, m_backBufferFormat},
         {Pipe::GBufferDebug, shaders.gbufferDebug, DXGI_FORMAT_R16G16B16A16_FLOAT}});

    //
    // Depth PrePass PSO
    //
    RegisterDepthPrePassPipeline(psoDesc, {Pipe::DepthPrePass, depthInputLayout, shaders.depthPrePass});
}

void HelloTextureEngine::CreateInitialCommandList()
{
    ThrowIfFailed(
        m_graphicsDevice.Device()->CreateCommandList(0,
                                                     D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                     m_frameResources[m_currentFrameIndex].commandAllocator.Get(),
                                                     GetPipelineState(PipelineId(Pipe::Forward)),
                                                     IID_PPV_ARGS(&m_commandList)));
}

void HelloTextureEngine::CreateSceneGeometryBuffers()
{
    assert(m_scene.mesh != nullptr);
    const Engine::SceneMesh& mesh = *m_scene.mesh;
    assert(!mesh.vertices.empty());

    m_indexCountPerInstance = static_cast<UINT>(mesh.indices.size());
    m_vertexCountPerInstance = static_cast<UINT>(mesh.vertices.size());
    m_usesIndexedDraw = m_indexCountPerInstance > 0;
    m_sceneHasMaterials = !mesh.materials.empty();
    const UINT vertexBufferSize = static_cast<UINT>(sizeof(Engine::SceneVertex) * mesh.vertices.size());

    // Note: using upload heaps to transfer static data like vert buffers is not
    // recommended. Every time the GPU needs it, the upload heap will be marshalled
    // over. Please read up on Default Heap usage. An upload heap is used here for
    // code simplicity and because there are very few verts to actually transfer.
    MyDx12Util::CreateUploadBuffer(m_graphicsDevice.Device(), vertexBufferSize, m_vertexBuffer);

    // Copy the triangle data to the vertex buffer.
    UINT8* pVertexDataBegin = nullptr;
    CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, mesh.vertices.data(), vertexBufferSize);
    m_vertexBuffer->Unmap(0, nullptr);

    // Initialize the vertex buffer view.
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(Engine::SceneVertex);
    m_vertexBufferView.SizeInBytes = vertexBufferSize;

    if (m_usesIndexedDraw)
    {
        const UINT indexBufferSize = static_cast<UINT>(mesh.indices.size() * sizeof(uint32_t));

        MyDx12Util::CreateUploadBuffer(m_graphicsDevice.Device(), indexBufferSize, m_indexBuffer);

        UINT8* pIndexDataBegin = nullptr;
        ThrowIfFailed(m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
        memcpy(pIndexDataBegin, mesh.indices.data(), indexBufferSize);
        m_indexBuffer->Unmap(0, nullptr);

        m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
        m_indexBufferView.SizeInBytes = indexBufferSize;
    }
}

void HelloTextureEngine::CreateSceneTextureResources(std::vector<ComPtr<ID3D12Resource>>& textureUploadHeap)
{
    assert(m_scene.mesh != nullptr);
    const Engine::SceneMesh& mesh = *m_scene.mesh;

    // Note: ComPtr's are CPU objects but this resource needs to stay in scope until
    // the command list that references it has finished executing on the GPU.
    // We will flush the GPU at the end of this method to ensure the resource is not
    // prematurely destroyed.

    textureUploadHeap.resize(kTextureCount);

    m_texture.resize(kTextureCount);

    std::vector<std::vector<UINT8>> texture(kTextureTypes);

    m_sceneTextureCount = static_cast<UINT>(mesh.textures.size());

    DBG_PRINT("m_sceneTextureCount = %d\n", m_sceneTextureCount);

    for (size_t i = 0; i < kTextureCount; i++)
    {
        bool useSceneTex = i < mesh.textures.size();
        UINT8* pixels = nullptr;
        UINT width, height;

        if (useSceneTex)
        {
            const auto& tex = mesh.textures[i];
            pixels = (UINT8*)tex.pixels.data();
            width = tex.width;
            height = tex.height;
            DBG_PRINT("[%d] sceneTexture :width %d height %d\n", i, tex.width, tex.height);
        }
        else
        {
            texture[i] = GenerateCheckerboardTextureData();
            pixels = &texture[i % kTextureTypes][0];
            width = kTextureWidth;
            height = kTextureHeight;
            DBG_PRINT("[%d] CheckerBoardTexture :width %d height %d\n", i, kTextureWidth, kTextureHeight);
        }

        DescriptorHeapHandle srv = CreateTextureFromRGBA8(pixels, width, height, m_texture[i], textureUploadHeap[i]);
        if (i == 0)
        {
            m_textureTableStart = srv;
        }
        m_texIndex[i] = srv.Index - m_textureTableStart.Index;
        DBG_PRINT("Texture %d SRV index: %d\n", i, m_texIndex[i]);
    }
}

void HelloTextureEngine::PrepareSceneInstanceData()
{
    // Instance data is provided by the application via SetScene().
    // Pre-allocate the buffer for kMaxInstanceCount entries.
    if (m_scene.instances.empty())
    {
        m_scene.instances.resize(kMaxInstanceCount);
    }
}

void HelloTextureEngine::CreateSceneMaterialResources()
{
    assert(m_scene.mesh != nullptr);
    const Engine::SceneMesh& mesh = *m_scene.mesh;

    // Generate the material data.
    m_materialData.clear();
    const auto resolveTextureIndex = [this](int sceneTextureIndex, UINT fallbackIndex) -> UINT
    {
        if (sceneTextureIndex >= 0 && sceneTextureIndex < static_cast<int>(kTextureCount))
        {
            return m_texIndex[sceneTextureIndex];
        }
        return fallbackIndex;
    };

    for (int i = 0; i < Engine::kMaterialCount; i++)
    {
        const UINT fallbackTexIndex = m_texIndex[i % kTextureCount];
        Engine::Material m = {};
        m.albedoTexIndex = fallbackTexIndex;
        m.metallicRoughnessTexIndex = fallbackTexIndex;
        m.emissiveTexIndex = fallbackTexIndex;
        m.occlusionTexIndex = fallbackTexIndex;
        m.normalTexIndex = fallbackTexIndex;
        m.roughnessFactor = 0.2f + 0.6f * static_cast<float>(i % 16) / 15.0f;
        m.metallicFactor = (i % 8 == 0) ? 1.0f : 0.0f;
        m.occlusionStrength = 1.0f;
        m.ambientOcclusionFactor = 1.0f;
        m.emissiveScale = 1.0f;
        m.flags = 0;

        if (m_sceneHasMaterials)
        {
            if (i < static_cast<int>(mesh.materials.size()))
            {
                const auto& gltfMaterial = mesh.materials[i];
                m.albedoTexIndex = resolveTextureIndex(gltfMaterial.albedoTexIndex, fallbackTexIndex);
                m.metallicRoughnessTexIndex =
                    resolveTextureIndex(gltfMaterial.metallicRoughnessTexIndex, fallbackTexIndex);
                m.emissiveTexIndex = resolveTextureIndex(gltfMaterial.emissiveTexIndex, fallbackTexIndex);
                m.occlusionTexIndex = resolveTextureIndex(gltfMaterial.occlusionTexIndex, fallbackTexIndex);
                m.normalTexIndex = resolveTextureIndex(gltfMaterial.normalTexIndex, fallbackTexIndex);
                m.roughnessFactor = gltfMaterial.roughnessFactor;
                m.metallicFactor = gltfMaterial.metallicFactor;
                m.occlusionStrength = gltfMaterial.occlusionStrength;
                m.ambientOcclusionFactor = gltfMaterial.ambientOcclusionFactor;
                m.emissiveScale = gltfMaterial.emissiveScale;
            }
        }

        m_materialData.push_back(m);
    }

    m_materialBuffer.Create(m_graphicsDevice.Device(), m_descriptorHeapAllocator, m_materialData);
}

void HelloTextureEngine::CreateInstanceBuffers()
{
    // Create the instance buffer.
    for (int n = 0; n < kFrameCount; n++)
    {
        const UINT instanceBufferSize = sizeof(InstanceData) * kMaxInstanceCount;

        MyDx12Util::CreateUploadBuffer(
            m_graphicsDevice.Device(), instanceBufferSize, m_frameResources[n].instanceBuffer);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.NumElements = kMaxInstanceCount;
        srvDesc.Buffer.StructureByteStride = sizeof(InstanceData);

        m_frameResources[n].instanceBufferSrv = m_descriptorHeapAllocator.AllocWithHandle();
        m_graphicsDevice.Device()->CreateShaderResourceView(
            m_frameResources[n].instanceBuffer.Get(), &srvDesc, m_frameResources[n].instanceBufferSrv.cpu);

        m_frameResources[n].instanceBuffer->Map(
            0, nullptr, reinterpret_cast<void**>(&m_frameResources[n].pSrvDataBegin));
        memcpy(m_frameResources[n].pSrvDataBegin, m_scene.instances.data(), instanceBufferSize);
        m_frameResources[n].instanceBuffer->Unmap(0, nullptr);
    }
}

void HelloTextureEngine::CreateFrameConstantBuffers()
{
    UpdateCameraConstantBuffer();
    m_constantBufferData.prevViewProjection = m_constantBufferData.viewProjection;

    // Create the per-frame constant buffers.
    for (UINT n = 0; n < kFrameCount; n++)
    {
        CreateConstantBuffer(m_frameResources[n].cameraCB, &m_constantBufferData, sizeof(m_constantBufferData));
        const LightingConstants initialLightingConstants = MakeLightingConstants();
        CreateConstantBuffer(m_frameResources[n].lightCB, &initialLightingConstants, sizeof(initialLightingConstants));
    }
}

void HelloTextureEngine::ExecuteInitialGpuSetup()
{
    // Close the command list and execute it to begin the initial GPU setup.
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = {m_commandList.Get()};
    m_graphicsDevice.ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        m_graphicsDevice.CreateFence(0);
        m_frameResources[m_currentFrameIndex].fenceValue = 1;

        // Wait for the command list to execute; we are reusing the same command
        // list in our main loop but for now, we just want to wait for setup to
        // complete before continuing.
        WaitForGpu();
    }
}

static SimpleDescriptorHeapAllocator* g_allocator = nullptr;

void HelloTextureEngine::InitImGui()
{
#if IMGUI_IMPL > 0
    g_allocator = &m_ImGuiDescriptorHeapAllocator;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(m_graphicsDevice.Hwnd());

    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = m_graphicsDevice.Device();
    init_info.CommandQueue = m_graphicsDevice.CommandQueue();
    init_info.NumFramesInFlight = kFrameCount;
    init_info.RTVFormat = m_backBufferFormat;
    init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
    // Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
    // (current version of the backend will only allocate one descriptor, future versions will need to allocate
    // more)
    init_info.SrvDescriptorHeap = m_imguiHeap.Get();
    init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*,
                                        D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle,
                                        D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle)
    { g_allocator->Alloc(out_cpu_handle, out_gpu_handle); };
    init_info.SrvDescriptorFreeFn =
        [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
    { g_allocator->Free(&cpu_handle, &gpu_handle); };
    ImGui_ImplDX12_Init(&init_info);
#endif
}

void HelloTextureEngine::CreateConstantBuffer(ConstantBufferResource& constantBuffer,
                                              const void* initialData,
                                              UINT sizeInBytes)
{
    assert(sizeInBytes % D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT == 0);

    ThrowIfFailed(m_graphicsDevice.Device()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                                     D3D12_HEAP_FLAG_NONE,
                                                                     &CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes),
                                                                     D3D12_RESOURCE_STATE_GENERIC_READ,
                                                                     nullptr,
                                                                     IID_PPV_ARGS(&constantBuffer.buffer)));

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = constantBuffer.buffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = sizeInBytes;

    constantBuffer.cbv = m_descriptorHeapAllocator.AllocWithHandle();
    m_graphicsDevice.Device()->CreateConstantBufferView(&cbvDesc, constantBuffer.cbv.cpu);

    CD3DX12_RANGE readRange(0, 0);
    ThrowIfFailed(constantBuffer.buffer->Map(0, &readRange, reinterpret_cast<void**>(&constantBuffer.mappedData)));
    memcpy(constantBuffer.mappedData, initialData, sizeInBytes);
}

DescriptorHeapHandle HelloTextureEngine::AllocateTextureSRV(ID3D12Resource* texture)
{
    DescriptorHeapHandle handle = m_descriptorHeapAllocator.AllocWithHandle();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texture->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    m_graphicsDevice.Device()->CreateShaderResourceView(texture, &srvDesc, handle.cpu);

    return handle;
}

// Generate a simple black and white checkerboard texture.
std::vector<UINT8> HelloTextureEngine::GenerateCheckerboardTextureData()
{
    const UINT rowPitch = kTextureWidth * kTexturePixelSize;
    const UINT cellPitch = rowPitch >> 3;       // The width of a cell in the checkboard texture.
    const UINT cellHeight = kTextureWidth >> 3; // The height of a cell in the checkerboard texture.
    const UINT textureSize = rowPitch * kTextureHeight;

    std::vector<UINT8> data(textureSize);
    UINT8* pData = &data[0];

    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 0xFF);
    UINT8 R = static_cast<UINT8>(dist(gen));
    UINT8 G = static_cast<UINT8>(dist(gen));
    UINT8 B = static_cast<UINT8>(dist(gen));

    // DBG_PRINT("R=%d G=%d B=%d\n", R, G, B);

    for (UINT n = 0; n < textureSize; n += kTexturePixelSize)
    {
        UINT x = n % rowPitch;
        UINT y = n / rowPitch;
        UINT i = x / cellPitch;
        UINT j = y / cellHeight;

        if (i % 2 == j % 2)
        {
            pData[n + 0] = 0x00; // R
            pData[n + 1] = 0x00; // G
            pData[n + 2] = 0x00; // B
            pData[n + 3] = 0xff; // A
        }
        else
        {
            pData[n + 0] = R;    // R
            pData[n + 1] = G;    // G
            pData[n + 2] = B;    // B
            pData[n + 3] = 0xff; // A
        }
    }

    return data;
}

void HelloTextureEngine::CreateDsvHeap()
{
    if (m_dsvHeap)
        return;

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ThrowIfFailed(m_graphicsDevice.Device()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
}

void HelloTextureEngine::CreateGBuffer()
{
    m_gbuffer.CreateResources(m_graphicsDevice.Device(), m_width, m_height);
    m_gbuffer.CreateRTVs(m_graphicsDevice.Device(), m_rtvHeap.Get(), kGBufferRTVBaseIndex, m_rtvDescriptorSize);
    m_gbuffer.CreateSRVs(m_graphicsDevice.Device(), m_descriptorHeapAllocator);

    if (m_depthStencilSrv.Index == UINT_MAX)
    {
        m_depthStencilSrv = m_descriptorHeapAllocator.AllocWithHandle();
    }
    assert(m_depthStencilSrv.Index == m_gbuffer.srvHandles[Engine::GBuffer::Albedo].Index + Engine::GBuffer::kCount);

    if (m_lightPassColorSrv.Index == UINT_MAX)
    {
        m_lightPassColorSrv = m_descriptorHeapAllocator.AllocWithHandle();
    }
    assert(m_lightPassColorSrv.Index == m_depthStencilSrv.Index + 1);
}

void HelloTextureEngine::RegisterDepthStencil(UINT width, UINT height)
{
    TransientResource r;

    r.state = TransientResourceState::Initialized;
    r.name = kDepthStencilResourceName;
    r.persistent = true;

    r.desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    r.desc.Width = width;
    r.desc.Height = height;
    r.desc.DepthOrArraySize = 1;
    r.desc.MipLevels = 1;
    r.desc.Format = DXGI_FORMAT_R32_TYPELESS;
    r.desc.SampleDesc.Count = 1;
    r.desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    r.desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    r.clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    r.clearValue.DepthStencil.Depth = 1.0f;
    r.clearValue.DepthStencil.Stencil = 0;

    r.initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

    m_resourceRegistry.RegisterTransientResource(std::move(r));
}

void HelloTextureEngine::RegisterLightPassRenderTarget(UINT width, UINT height)
{
    TransientResource r;

    r.state = TransientResourceState::Initialized;
    r.name = kLightPassRenderTargetResourceName;
    r.persistent = true;

    r.desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    r.desc.Width = width;
    r.desc.Height = height;
    r.desc.DepthOrArraySize = 1;
    r.desc.MipLevels = 1;
    r.desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    r.desc.SampleDesc.Count = 1;
    r.desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    r.desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    r.clearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    r.clearValue.Color[0] = 0.0f;
    r.clearValue.Color[1] = 0.0f;
    r.clearValue.Color[2] = 0.0f;
    r.clearValue.Color[3] = 1.0f;

    r.initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;

    m_resourceRegistry.RegisterTransientResource(std::move(r));
}

void HelloTextureEngine::CreateDepthStencilDescriptors()
{
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    m_graphicsDevice.Device()->CreateDepthStencilView(
        m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    assert(m_depthStencilSrv.Index == m_gbuffer.srvHandles[Engine::GBuffer::Albedo].Index + Engine::GBuffer::kCount);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    m_graphicsDevice.Device()->CreateShaderResourceView(m_depthStencil.Get(), &srvDesc, m_depthStencilSrv.cpu);
}

D3D12_CPU_DESCRIPTOR_HANDLE HelloTextureEngine::GetBackBufferRtv() const
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE h(
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_currentFrameIndex, m_rtvDescriptorSize);
    return h;
}

D3D12_CPU_DESCRIPTOR_HANDLE HelloTextureEngine::GetDepthDsv() const
{
    return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE HelloTextureEngine::GetGBufferRTV(UINT index) const
{
    return m_gbuffer.GetRTV(m_rtvHeap.Get(), m_rtvDescriptorSize, index);
}

D3D12_CPU_DESCRIPTOR_HANDLE HelloTextureEngine::GetLightPassRTV() const
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE h(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    h.Offset(kLightPassRTVIndex, m_rtvDescriptorSize);
    return h;
}

void HelloTextureEngine::RegisterPassBindingResolvers()
{
    m_renderGraphRuntime.Bindings().Clear();

    m_renderGraphRuntime.Bindings().RegisterRtv(m_renderGraphRuntime.RegisterRtv(RtvName::BackBuffer),
                                                [this]() { return GetBackBufferRtv(); });
    m_renderGraphRuntime.Bindings().RegisterRtv(m_renderGraphRuntime.RegisterRtv(RtvName::GBufferAlbedo),
                                                [this]() { return GetGBufferRTV(Engine::GBuffer::Albedo); });
    m_renderGraphRuntime.Bindings().RegisterRtv(m_renderGraphRuntime.RegisterRtv(RtvName::GBufferNormal),
                                                [this]() { return GetGBufferRTV(Engine::GBuffer::Normal); });
    m_renderGraphRuntime.Bindings().RegisterRtv(m_renderGraphRuntime.RegisterRtv(RtvName::GBufferMaterial),
                                                [this]() { return GetGBufferRTV(Engine::GBuffer::Material); });
    m_renderGraphRuntime.Bindings().RegisterRtv(m_renderGraphRuntime.RegisterRtv(RtvName::GBufferMotionVector),
                                                [this]() { return GetGBufferRTV(Engine::GBuffer::MotionVector); });
    m_renderGraphRuntime.Bindings().RegisterRtv(m_renderGraphRuntime.RegisterRtv(RtvName::GBufferPBRParams),
                                                [this]() { return GetGBufferRTV(Engine::GBuffer::PBRParams); });
    m_renderGraphRuntime.Bindings().RegisterRtv(m_renderGraphRuntime.RegisterRtv(RtvName::LightPass),
                                                [this]() { return GetLightPassRTV(); });

    m_renderGraphRuntime.Bindings().RegisterDsv(m_renderGraphRuntime.RegisterDsv(DsvName::Depth),
                                                [this]() { return GetDepthDsv(); });

    m_renderGraphRuntime.Bindings().RegisterDescriptor(m_renderGraphRuntime.RegisterDescriptor(Desc::TextureTable),
                                                       [this]() { return m_textureTableStart.gpu; });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(
        m_renderGraphRuntime.RegisterDescriptor(Desc::InstanceBufferSrv),
        [this]() { return m_frameResources[m_currentFrameIndex].instanceBufferSrv.gpu; });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(m_renderGraphRuntime.RegisterDescriptor(Desc::MaterialBufferSrv),
                                                       [this]() { return m_materialBuffer.Srv().gpu; });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(m_renderGraphRuntime.RegisterDescriptor(Desc::EnvironmentMapSrv),
                                                       [this]() { return m_environmentMap.Srv().gpu; });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(
        m_renderGraphRuntime.RegisterDescriptor(Desc::CameraCbv),
        [this]() { return m_frameResources[m_currentFrameIndex].cameraCB.cbv.gpu; });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(
        m_renderGraphRuntime.RegisterDescriptor(Desc::LightCbv),
        [this]() { return m_frameResources[m_currentFrameIndex].lightCB.cbv.gpu; });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(m_renderGraphRuntime.RegisterDescriptor(Desc::GBufferAlbedoSrv),
                                                       [this]()
                                                       { return m_gbuffer.srvHandles[Engine::GBuffer::Albedo].gpu; });
    m_renderGraphRuntime.Bindings().RegisterDescriptor(
        m_renderGraphRuntime.RegisterDescriptor(Desc::ToneMapSceneColorSrv),
        [this]() { return m_lightPassColorSrv.gpu; });
}

void HelloTextureEngine::RegisterPassConstantsHandlers()
{
    m_renderGraphRuntime.Constants().Clear();
    m_renderGraphRuntime.Constants().Register(
        m_renderGraphRuntime.RegisterConstants(ConstName::ToneMap),
        [this](UINT rootParameterIndex)
        {
            const auto constants = m_toneMapPass.MakeShaderConstants(m_hdrOutputPolicy.settings);
            m_commandList->SetGraphicsRoot32BitConstants(rootParameterIndex, 5, &constants, 0);
        });
    m_renderGraphRuntime.Constants().Register(m_renderGraphRuntime.RegisterConstants(ConstName::GBufferDebugTarget),
                                              [this](UINT rootParameterIndex)
                                              {
                                                  const UINT debugTarget = m_debugViewSettings.GetGBufferDebugTarget();
                                                  m_commandList->SetGraphicsRoot32BitConstants(
                                                      rootParameterIndex, 1, &debugTarget, 0);
                                              });
}

void HelloTextureEngine::RegisterResourceResolvers()
{
    m_renderGraphRuntime.Resources().Clear();
    m_renderGraphRuntime.Resources().RegisterResource(kBackBufferResourceName,
                                                      [this]() { return m_renderTargets[m_currentFrameIndex].Get(); });
    m_renderGraphRuntime.Resources().RegisterResource(kDepthStencilResourceName,
                                                      [this]() { return m_depthStencil.Get(); });
    m_renderGraphRuntime.Resources().RegisterResource(kLightPassRenderTargetResourceName,
                                                      [this]() { return m_lightPassRenderTarget.Get(); });
    for (UINT i = 0; i < Engine::GBuffer::kCount; ++i)
    {
        m_renderGraphRuntime.Resources().RegisterResource(kGBufferResourceNames[i],
                                                          [this, i]() { return m_gbuffer.resources[i].Get(); });
    }
    m_renderGraphRuntime.Resources().SetFallbackResolver([this](const std::string& name)
                                                         { return FindTransientD3DResource(name); });
}

void HelloTextureEngine::CreateDepthStencil(UINT width, UINT height)
{
    // Release if DS exist
    m_depthStencil.Reset();

    // Create Depth Resource
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ThrowIfFailed(m_graphicsDevice.Device()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    ThrowIfFailed(m_graphicsDevice.Device()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                                     D3D12_HEAP_FLAG_NONE,
                                                                     &depthDesc,
                                                                     D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                                     &clearValue,
                                                                     IID_PPV_ARGS(&m_depthStencil)));

    // Create DSV
    CreateDepthStencilDescriptors();
}

// Update frame-based values.
void HelloTextureEngine::UpdateFrame()
{
    PIXBeginEvent(0, L"UpdateFrame");

    if (m_updateHandler)
    {
        m_updateHandler();
    }

    m_frameResources[m_currentFrameIndex].instanceBuffer->Map(
        0, nullptr, reinterpret_cast<void**>(&m_frameResources[m_currentFrameIndex].pSrvDataBegin));
    memcpy(m_frameResources[m_currentFrameIndex].pSrvDataBegin,
           m_scene.instances.data(),
           sizeof(InstanceData) * kMaxInstanceCount);
    m_frameResources[m_currentFrameIndex].instanceBuffer->Unmap(0, nullptr);

    m_constantBufferData.prevViewProjection = m_constantBufferData.viewProjection;
    UpdateCameraConstantBuffer();
    memcpy(
        m_frameResources[m_currentFrameIndex].cameraCB.mappedData, &m_constantBufferData, sizeof(m_constantBufferData));

    PIXEndEvent();
}

void HelloTextureEngine::UpdateImGui()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    if (m_debugUiHandler)
    {
        DebugUiContext context{static_cast<int>(m_currentFrameIndex),
                               m_cpuFrameTime,
                               m_frameResources[m_previousFrameIndex].gpuWorkMeterCheckPoints};
        m_debugUiHandler(context);
    }

    m_toneMapPass.settings.Normalize();
    const LightingConstants lightingConstants = MakeLightingConstants();
    memcpy(m_frameResources[m_currentFrameIndex].lightCB.mappedData, &lightingConstants, sizeof(lightingConstants));

    ImGui::Render();
}

UINT HelloTextureEngine::GetVisibleCubeCount() const
{
    return static_cast<UINT>(m_displayInstanceCount);
}

// Render the scene.
void HelloTextureEngine::RenderFrame()
{
    PIXBeginEvent(0, L"RenderFrame");

    // ImGui frame update
#if IMGUI_IMPL > 0
    UpdateImGui();
#endif

    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = {m_commandList.Get()};
    m_graphicsDevice.ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    if (m_debugViewSettings.hdrDumpPending)
    {
        WaitForGpu();
        PrintDebugDump();
        m_debugViewSettings.requestHdrDump = false;
        m_debugViewSettings.hdrDumpPending = false;
    }

    // Present the frame.
    m_graphicsDevice.Present(1, 0);

    UINT64 submittedFenceValue = MoveToNextFrame();

    // submittedFenceValue marks completion of the command list submitted for this frame.
    MarkPendingTransientResources(submittedFenceValue);

    CollectGarbageTransientResources();

    m_gpuWorkMeter.ReadbackData(m_graphicsDevice.CommandQueue());

    PIXEndEvent();
}

void HelloTextureEngine::RequestResize(UINT width, UINT height)
{
    m_pendingResize = true;
    m_pendingResizeWidth = width;
    m_pendingResizeHeight = height;
}

void HelloTextureEngine::RunFrame()
{
    if (m_pendingResize)
    {
        ApplyResize(m_pendingResizeWidth, m_pendingResizeHeight);
        m_pendingResize = false;
    }

    UpdateHdr10DisplayMode();

    m_workMeter.Start();
    UpdateFrame();
    RenderFrame();
    m_workMeter.End();
    m_cpuFrameTime = m_workMeter.GetCpuFrameTimeMs();
}

void HelloTextureEngine::ApplyResize(UINT width, UINT height)
{
    DBG_PRINT("HelloTextureEngine::ApplyResize() %d %d\n", width, height);
    m_width = width;
    m_height = height;

    if (width == 0 || height == 0)
    {
        return;
    }

    if (!m_graphicsDevice.HasSwapChain())
    {
        return;
    }

    FlushGpu();

    // Clear RTV
    for (UINT n = 0; n < kFrameCount; n++)
    {
        m_renderTargets[n].Reset();
    }

    // GraphicsDevice owns swap chain resizing; the engine rebuilds render resources that depend on its buffers.
    m_graphicsDevice.ResizeSwapChain(kFrameCount, m_width, m_height, m_backBufferFormat, 0);
    m_hdrOutputPolicy.ReapplyColorSpace(m_graphicsDevice.SwapChain());

    // Preserve the previous frame index before taking the resized swap chain index.
    m_previousFrameIndex = m_currentFrameIndex;
    m_currentFrameIndex = m_graphicsDevice.CurrentBackBufferIndex();

    // Re-create render target views (RTVs) for the swap chain back buffers.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < kFrameCount; n++)
        {
            m_graphicsDevice.GetBackBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
            m_graphicsDevice.Device()->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    m_depthStencil.Reset();
    m_lightPassRenderTarget.Reset();
    m_resourceRegistry.UnregisterTransientResource(kDepthStencilResourceName);
    m_resourceRegistry.UnregisterTransientResource(kLightPassRenderTargetResourceName);
    RegisterDepthStencil(m_width, m_height);
    RegisterLightPassRenderTarget(m_width, m_height);
    CreateGBuffer();

    // Camera
    UpdateCameraConstantBuffer();

    // Screen
    m_viewport = CD3DX12_VIEWPORT(
        0.0f, 0.0f, static_cast<FLOAT>(m_width), static_cast<FLOAT>(m_height), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH);
    m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));

    // Imgui
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(static_cast<float>(m_width), static_cast<float>(m_height));
}

void HelloTextureEngine::Shutdown()
{
    DestroyFrameResources();
}

void HelloTextureEngine::DestroyFrameResources()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForGpu();

    m_graphicsDevice.CloseFenceEvent();
}

void HelloTextureEngine::RegisterFullscreenPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                    const FullscreenPipelineDefinition& definition)
{
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = Engine::CreateFullscreenPipelineDesc(baseDesc, definition);
    const PipelineKey key = PipelineId(definition.name);
    ThrowIfFailed(m_renderGraphRuntime.Pipelines().Create(m_graphicsDevice.Device(), key, desc));
}

void HelloTextureEngine::RegisterFullscreenPipelines(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                     std::initializer_list<FullscreenPipelineDefinition> definitions)
{
    for (const FullscreenPipelineDefinition& definition : definitions)
    {
        RegisterFullscreenPipeline(baseDesc, definition);
    }
}

void HelloTextureEngine::RegisterForwardPipeline(D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                 const ForwardPipelineDefinition& definition)
{
    baseDesc = Engine::CreateForwardPipelineDesc(baseDesc, m_rootSignature.Get(), definition);
    const PipelineKey key = PipelineId(definition.name);
    ThrowIfFailed(m_renderGraphRuntime.Pipelines().Create(m_graphicsDevice.Device(), key, baseDesc));
}

void HelloTextureEngine::RegisterGBufferPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                 const GBufferPipelineDefinition& definition)
{
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC desc =
        Engine::CreateGBufferPipelineDesc(baseDesc, definition, m_gbuffer.formats, Engine::GBuffer::kCount);
    const PipelineKey key = PipelineId(definition.name);
    ThrowIfFailed(m_renderGraphRuntime.Pipelines().Create(m_graphicsDevice.Device(), key, desc));
}

void HelloTextureEngine::RegisterDepthPrePassPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                      const DepthPrePassPipelineDefinition& definition)
{
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = Engine::CreateDepthPrePassPipelineDesc(baseDesc, definition);
    const PipelineKey key = PipelineId(definition.name);
    ThrowIfFailed(m_renderGraphRuntime.Pipelines().Create(m_graphicsDevice.Device(), key, desc));
}

void HelloTextureEngine::PopulateCommandList()
{
    PIXBeginEvent(1, L"PopulateCommandList");

    BeginFrame();
    ResetResourceStates();
    BuildRenderPasses();
    ValidateRenderPassGraph();
    AnalyzeResourceLifetimes();
    // DebugPrintLifetimes();
    ExecutePasses();
    EndFrame();

    PIXEndEvent();
}

void HelloTextureEngine::AnalyzeResourceLifetimes()
{
    m_resourceRegistry.AnalyzeLifetimes(m_renderGraphRuntime.Graph().Passes());
}

void HelloTextureEngine::DebugPrintLifetimes()
{
    DBG_PRINT("Resource Lifetimes:\n");
    for (auto& [name, lt] : m_resourceRegistry.lifetimes)
    {
        DBG_PRINT("Resource %s: [%d - %d]\n", name.c_str(), lt.firstPass, lt.lastPass);
    }
}

ID3D12PipelineState* HelloTextureEngine::GetPipelineState(PipelineKey pipeline) const
{
    return m_renderGraphRuntime.Pipelines().Find(pipeline);
}

void HelloTextureEngine::ExecutePasses()
{
    Engine::ResourceTransitionContext resourceTransitions = MakeResourceTransitionContext();
    Engine::ExecuteRenderPassGraph(m_renderGraphRuntime.Graph(),
                                   {m_commandList.Get(),
                                    &m_renderGraphRuntime.Bindings(),
                                    &m_renderGraphRuntime.Pipelines(),
                                    &m_renderGraphRuntime.Constants(),
                                    &resourceTransitions,
                                    [this](int passIndex) { CreateResourcesForPass(passIndex); },
                                    [this](const RenderPass& pass) { ExecutePassOperation(pass); },
                                    [this](int passIndex) { ReleaseResourcesAfterPass(passIndex); }});
}

void HelloTextureEngine::ExecutePassOperation(const RenderPass& pass)
{
    const PassOperationHandler* handler = m_renderGraphRuntime.Operations().Find(pass.operation);
    assert(handler != nullptr && "Unsupported pass operation.");
    if (handler != nullptr)
    {
        const PassOperationHandler handlerFunc = *handler;
        (this->*handlerFunc)(pass);
    }
}

void HelloTextureEngine::CreateResourcesForPass(int passIndex)
{
    const std::vector<std::string> resourceNames =
        m_resourceRegistry.GetResourcesStartingAtPass(passIndex, kBackBufferResourceName);

    for (const std::string& name : resourceNames)
    {
        TransientResource* transientResource = m_resourceRegistry.PrepareTransientResourceForCreate(name);
        if (transientResource == nullptr)
            continue;
        auto& tr = *transientResource;

        CreateCommittedTransientResource(tr);
        m_resourceRegistry.MarkTransientResourceCreated(name);
        BindCreatedTransientResource(name, tr.resource.Get());

        DBG_PRINT("Resource %s created.\n", name.c_str());
    }
}

void HelloTextureEngine::CreateCommittedTransientResource(TransientResource& resource)
{
    ThrowIfFailed(m_graphicsDevice.Device()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                                                                     D3D12_HEAP_FLAG_NONE,
                                                                     &resource.desc,
                                                                     resource.initialState,
                                                                     &resource.clearValue,
                                                                     IID_PPV_ARGS(&resource.resource)));
}

void HelloTextureEngine::BindCreatedTransientResource(const std::string& name, ID3D12Resource* resource)
{
    if (name == kDepthStencilResourceName)
    {
        m_depthStencil = resource;
        CreateDepthStencilDescriptors();
        return;
    }

    if (name == kLightPassRenderTargetResourceName)
    {
        m_lightPassRenderTarget = resource;
        CreateLightPassRenderTargetDescriptors();
        return;
    }

    assert(false && "Unsupported resource in BindCreatedTransientResource()");
}

void HelloTextureEngine::CreateLightPassRenderTargetDescriptors()
{
    m_graphicsDevice.Device()->CreateRenderTargetView(m_lightPassRenderTarget.Get(), nullptr, GetLightPassRTV());

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    m_graphicsDevice.Device()->CreateShaderResourceView(
        m_lightPassRenderTarget.Get(), &srvDesc, m_lightPassColorSrv.cpu);
}

void HelloTextureEngine::ReleaseResourcesAfterPass(int passIndex)
{
    m_resourceRegistry.MarkEndOfLifeResources(passIndex, kBackBufferResourceName);
}

void HelloTextureEngine::MarkPendingTransientResources(UINT64 fenceValue)
{
    m_resourceRegistry.MarkPendingTransientResources(fenceValue);
}

void HelloTextureEngine::CollectGarbageTransientResources()
{
    const UINT64 completed = m_graphicsDevice.CompletedFenceValue();
    const std::vector<std::string> releasedResources = m_resourceRegistry.CollectGarbageTransientResources(completed);

    for (const std::string& name : releasedResources)
    {
        auto transientResource = m_resourceRegistry.transientResources.find(name);
        if (transientResource != m_resourceRegistry.transientResources.end())
        {
            transientResource->second.resource.Reset();
            DBG_PRINT("Resource %s released.\n", name.c_str());
        }

        if (name == kDepthStencilResourceName)
        {
            m_depthStencil.Reset();
        }

        if (name == kLightPassRenderTargetResourceName)
        {
            m_lightPassRenderTarget.Reset();
        }
    }
}

void HelloTextureEngine::ResetResourceStates()
{
    m_resourceRegistry.states.clear();
    for (const auto& usage : m_resourceDefaultStates)
    {
        m_resourceRegistry.SetState(usage.name, usage.state);
    }
}

Engine::ResourceTransitionContext HelloTextureEngine::MakeResourceTransitionContext()
{
    return {m_commandList.Get(),
            [this](const std::string& name) { return m_renderGraphRuntime.Resources().Resolve(name); },
            [this](const std::string& name) { return GetResourceState(name); },
            [this](const std::string& name, D3D12_RESOURCE_STATES state) { SetResourceState(name, state); },
            [](const ResourceUsage& usage)
            { DBG_PRINT("Resource %s is null. Skip transition.\n", usage.name.c_str()); }};
}

void HelloTextureEngine::TransitionPassResources(const RenderPass& pass)
{
    Engine::TransitionPassResources(MakeResourceTransitionContext(), pass);
}

void HelloTextureEngine::TransitionResource(const ResourceUsage& usage)
{
    Engine::TransitionResource(MakeResourceTransitionContext(), usage);
}

ID3D12Resource* HelloTextureEngine::FindTransientD3DResource(const std::string& name) const
{
    auto transientResource = m_resourceRegistry.transientResources.find(name);
    if (transientResource == m_resourceRegistry.transientResources.end())
    {
        return nullptr;
    }

    return transientResource->second.resource.Get();
}

D3D12_RESOURCE_STATES HelloTextureEngine::GetResourceState(const std::string& name) const
{
    return m_resourceRegistry.GetState(name);
}

void HelloTextureEngine::SetResourceState(const std::string& name, D3D12_RESOURCE_STATES state)
{
    m_resourceRegistry.SetState(name, state);
}

void HelloTextureEngine::BeginFrame()
{

    // Command list allocators can only be reset when the associated
    // command lists have finished execution on the GPU; apps should use
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_frameResources[m_currentFrameIndex].commandAllocator->Reset());

    // However, when ExecuteCommandList() is called on a particular command
    // list, that command list can then be reset at any time and must be before
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_frameResources[m_currentFrameIndex].commandAllocator.Get(),
                                       GetPipelineState(PipelineId(Pipe::Forward))));

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    ID3D12DescriptorHeap* ppHeaps[] = {m_heap.Get()};
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    m_gpuWorkMeter.StartGpu(m_commandList.Get(), m_frameResources[m_currentFrameIndex].gpuWorkMeterCheckPoints);
}

void HelloTextureEngine::ExecuteClearPass(const RenderPass& pass)
{
    Engine::RecordClearPass(m_commandList.Get(), ResolveRenderTargets(pass.renderTargets));
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Clear");
}

void HelloTextureEngine::ExecuteDepthPrePass(const RenderPass& pass)
{
    Engine::RecordDepthPrePass(m_commandList.Get(), MakeSceneGeometryDrawDesc());
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Depth Prepass");
}

void HelloTextureEngine::ExecuteGBufferPass(const RenderPass& pass)
{
    assert(pass.renderTargets.rtvs.size() == Engine::GBuffer::kCount);

    Engine::GBufferPassDesc passDesc = {};
    passDesc.renderTargets = ResolveRenderTargets(pass.renderTargets);
    passDesc.clearValues = m_gbuffer.clearValues;
    passDesc.geometryDraw = MakeSceneGeometryDrawDesc();

    Engine::RecordGBufferPass(m_commandList.Get(), passDesc);
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "GBuffer Pass");
}

void HelloTextureEngine::ExecuteForwardPass(const RenderPass& pass)
{
    Engine::ForwardPassDesc passDesc = {};
    passDesc.renderTargets = ResolveRenderTargets(pass.renderTargets);
    passDesc.geometryDraw = MakeSceneGeometryDrawDesc();

    Engine::RecordForwardPass(m_commandList.Get(), passDesc);
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Main Pass");
}

void HelloTextureEngine::ExecuteLightingPass(const RenderPass& pass)
{
    Engine::RecordLightingPass(m_commandList.Get());
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Lighting Pass");
}

void HelloTextureEngine::ExecuteLightingDebugGradientPass(const RenderPass& pass)
{
    Engine::RecordLightingDebugGradientPass(m_commandList.Get());
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "LightPassDebugGradient Pass");
}

void HelloTextureEngine::ExecuteToneMapPass(const RenderPass& pass)
{
    Engine::RecordToneMapPass(m_commandList.Get());
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "ToneMap Pass");
}

void HelloTextureEngine::ExecuteDebugDumpPass(const RenderPass& pass)
{
    if (!m_lightingPassDebugGradientEnabled)
    {
        return;
    }

    RecordDebugDumpPass();
}

void HelloTextureEngine::ExecuteGBufferDebugPass(const RenderPass& pass)
{
    Engine::RecordGBufferDebugPass(m_commandList.Get());
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "GBuffer Debug Pass");
}

void HelloTextureEngine::ExecuteImGuiPass(const RenderPass& pass)
{
    RecordImGuiPass();
}

void HelloTextureEngine::RecordDebugDumpPass()
{
    Engine::RecordDebugDumpCapture(m_commandList.Get(),
                                   m_graphicsDevice.Device(),
                                   m_lightPassRenderTarget.Get(),
                                   m_renderTargets[m_currentFrameIndex].Get(),
                                   m_debugDumpCapture);

    m_debugViewSettings.hdrDumpPending = true;

    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "Debug Dump");
}

void HelloTextureEngine::PrintDebugDump()
{
    if (!m_lightingPassDebugGradientEnabled)
    {
        return;
    }

    if (!m_debugDumpCapture.IsReady())
    {
        return;
    }

    Engine::DebugDumpMappedCapture mappedCapture = {};
    Engine::MapDebugDumpCapture(m_debugDumpCapture, mappedCapture);

    const Engine::DebugDumpReportDesc reportDesc = {
        mappedCapture,
        m_hdrOutputPolicy.settings,
        m_toneMapPass.settings,
    };
    Engine::PrintDebugDumpReport(reportDesc);

    Engine::UnmapDebugDumpCapture(m_debugDumpCapture);
}

auto HelloTextureEngine::MakeSceneGeometryDrawDesc() const -> Engine::SceneGeometryDrawDesc
{
    return {m_vertexBufferView,
            m_indexBufferView,
            m_usesIndexedDraw,
            m_vertexCountPerInstance,
            m_indexCountPerInstance,
            GetVisibleCubeCount()};
}

auto HelloTextureEngine::ResolveRenderTargets(const PassRenderTargetBinding& renderTargets) const
    -> Engine::ResolvedRenderTargets
{
    Engine::ResolvedRenderTargets resolvedRenderTargets = {};
    resolvedRenderTargets.rtvs.reserve(renderTargets.rtvs.size());

    for (RtvKey rtv : renderTargets.rtvs)
    {
        resolvedRenderTargets.rtvs.push_back(m_renderGraphRuntime.Bindings().ResolveRtv(rtv));
    }

    if (renderTargets.dsv)
    {
        resolvedRenderTargets.dsv = m_renderGraphRuntime.Bindings().ResolveDsv(renderTargets.dsv.value());
    }

    resolvedRenderTargets.clearColor = renderTargets.clearColor ? renderTargets.clearColor->data() : nullptr;
    return resolvedRenderTargets;
}

void HelloTextureEngine::RecordImGuiPass()
{
#if IMGUI_IMPL > 0
    {
        m_commandList->RSSetViewports(1, &m_viewport);
        m_commandList->RSSetScissorRects(1, &m_scissorRect);

        ID3D12DescriptorHeap* imguiHeaps[] = {m_imguiHeap.Get()};

        m_commandList->SetDescriptorHeaps(1, imguiHeaps);

        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList.Get());
    }
#endif
    m_gpuWorkMeter.SetCheckPoint(m_commandList.Get(), "ImGUI");
}

void HelloTextureEngine::EndFrame()
{
    m_gpuWorkMeter.EndGpu(m_commandList.Get());

    for (const auto& usage : m_resourceDefaultStates)
    {
        TransitionResource(usage);
    }

    ThrowIfFailed(m_commandList->Close());
}

// Wait for pending GPU work to complete.
void HelloTextureEngine::WaitForGpu()
{
    PIXBeginEvent(3, L"WaitForGpu");

    // Schedule a Signal command in the queue.
    m_graphicsDevice.SignalFence(m_frameResources[m_currentFrameIndex].fenceValue);

    // Wait until the fence has been processed.
    m_graphicsDevice.WaitForFenceValue(m_frameResources[m_currentFrameIndex].fenceValue);

    // Increment the fence value for the current frame.
    m_frameResources[m_currentFrameIndex].fenceValue++;

    PIXEndEvent();
}

void HelloTextureEngine::FlushGpu()
{
    for (UINT n = 0; n < kFrameCount; n++)
    {
        const UINT64 fenceValue = ++m_frameResources[n].fenceValue;

        m_graphicsDevice.SignalFence(fenceValue);
        m_graphicsDevice.WaitForFenceValue(fenceValue);
    }
}

// Prepare to render the next frame.
UINT64 HelloTextureEngine::MoveToNextFrame()
{
    PIXBeginEvent(2, L"MoveToNextFrame");

    // Schedule a Signal command in the queue.
    const UINT64 currentFenceValue = m_frameResources[m_currentFrameIndex].fenceValue;
    m_graphicsDevice.SignalFence(currentFenceValue);

    // Update the frame index.
    m_previousFrameIndex = m_currentFrameIndex;
    m_currentFrameIndex = m_graphicsDevice.CurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    if (m_graphicsDevice.CompletedFenceValue() < m_frameResources[m_currentFrameIndex].fenceValue)
    {
        PIXBeginEvent(4, L"WaitForSingleObjectEx");
        m_graphicsDevice.WaitForFenceValue(m_frameResources[m_currentFrameIndex].fenceValue);
        PIXEndEvent();
    }

    // Set the fence value for the next frame.
    m_frameResources[m_currentFrameIndex].fenceValue = currentFenceValue + 1;

    PIXEndEvent();

    return currentFenceValue;
}
