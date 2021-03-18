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
#include "D3D12MeshletCull.h"

#include "Shared.h"

namespace
{
    const wchar_t* c_ampShaderFilename   = L"MeshletAS.cso";
    const wchar_t* c_meshShaderFilename  = L"MeshletMS.cso";
    const wchar_t* c_pixelShaderFilename = L"MeshletPS.cso";

    const wchar_t* c_modelFilenames[] = 
    { 
        L"..\\Assets\\Dragon_LOD0.bin", 
        L"..\\Assets\\Camera.bin" 
    };

    struct ObjectDefinition
    {
        uint32_t ModelIndex;
        XMFLOAT3 Position;
        XMFLOAT3 Rotation;
        float    Scale;

        bool     Cull;
        bool     DrawMeshlets;
    };

    const ObjectDefinition c_sceneDefinition[] =
    {
        { 0, {}, {}, 0.2f, true, true },  // View Model
        { 1, {}, {}, 1.0f, false, false } // Debug Camera - this transform gets overwritten every frame by our controller.
    };

    enum NamedObjectIndex
    {
        ViewModel = 0,
        DebugCamera
    };

#pragma warning (disable : 4061)
    uint32_t FormatSize(DXGI_FORMAT f)
    {
        switch (f)
        {
        case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16;
        case DXGI_FORMAT_R32G32B32_FLOAT: return 12;
        case DXGI_FORMAT_R32G32_FLOAT: return 8;
        case DXGI_FORMAT_R32_FLOAT: return 4;
        default: assert(false);
        }
        return 0;
    }
#pragma warning (default : 4061)

    uint32_t ComputeSemanticByteOffset(const D3D12_INPUT_LAYOUT_DESC& desc, const char* name)
    {
        uint32_t offset = 0;
        for (uint32_t i = 0; i < desc.NumElements; ++i)
        {
            auto& decl = desc.pInputElementDescs[i];

            if (std::strcmp(decl.SemanticName, name) == 0)
            {
                if (decl.AlignedByteOffset != ~0u)
                {
                    offset = decl.AlignedByteOffset;
                }

                return offset;
            }

            offset += FormatSize(decl.Format);
        }

        return uint32_t(-1);
    }

    /////
    // Ray casting intersection tests for determining meshlet picking.

    bool RayIntersectSphere(FXMVECTOR o, FXMVECTOR d, FXMVECTOR s)
    {
        XMVECTOR l = o - s;
        XMVECTOR r = XMVectorSplatW(s);
        XMVECTOR a = XMVector3Dot(d, d);
        XMVECTOR b = 2.0 * XMVector3Dot(l, d);
        XMVECTOR c = XMVector3Dot(l, l) - (r * r);

        XMVECTOR disc = b * b - 4 * a * c;
        return !XMVector4Less(disc, g_XMZero);
    }

    XMVECTOR RayIntersectTriangle(FXMVECTOR o, FXMVECTOR d, FXMVECTOR p0, GXMVECTOR p1, HXMVECTOR p2)
    {
        XMVECTOR edge1, edge2, h, s, q;
        XMVECTOR a, f, u, v;
        edge1 = p1 - p0;
        edge2 = p2 - p0;

        h = XMVector3Cross(d, edge2);
        a = XMVector3Dot(edge1, h);
        if (XMVector4Less(XMVectorAbs(a), g_XMEpsilon))
            return g_XMQNaN;

        f = g_XMOne / a;
        s = o - p0;
        u = f * XMVector3Dot(s, h);
        if (XMVector4Less(u, g_XMZero) || XMVector4Greater(u, g_XMOne))
            return g_XMQNaN;

        q = XMVector3Cross(s, edge1);
        v = f * XMVector3Dot(d, q);
        if (XMVector4Less(v, g_XMZero) || XMVector4Greater(u + v, g_XMOne))
            return g_XMQNaN;

        // At this stage we can compute t to find out where the intersection point is on the line.
        XMVECTOR t = f * XMVector3Dot(edge2, q);
        if (XMVector4Greater(t, g_XMEpsilon) && XMVector4Less(t, XMVectorReciprocal(g_XMEpsilon))) // ray intersection
        {
            return t;
        }

        return g_XMQNaN; // This means that there is a line intersection but not a ray intersection.
    }

    inline uint32_t DivRoundUp(uint32_t num, uint32_t den) { return (num + den - 1) / den; }
}


D3D12MeshletCull::D3D12MeshletCull(UINT width, UINT height, std::wstring name)
    : DXSample(width, height, name)
    , m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))
    , m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height))
    , m_rtvDescriptorSize(0)
    , m_dsvDescriptorSize(0)
    , m_constantsData(nullptr)
    , m_fovy(XM_PI / 3)
    , m_frameIndex(0)
    , m_frameCounter(0)
    , m_fenceEvent{}
    , m_fenceValues{}
    , m_highlightedIndex(uint32_t(-1))
    , m_selectedIndex(uint32_t(-1))
    , m_drawMeshlets(true)
{ }

void D3D12MeshletCull::OnInit()
{
    m_mainCam.Init({ 0, 15, 40 });
    m_mainCam.SetMoveSpeed(25.0f);

    m_debugCam.Init({ 0.0f, 10, 21 });
    m_debugCam.SetMoveSpeed(25.0f);

    LoadPipeline();
    LoadAssets();
}

// Load the rendering pipeline dependencies.
void D3D12MeshletCull::LoadPipeline()
{
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    if (m_useWarpDevice)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

        ThrowIfFailed(D3D12CreateDevice(
            warpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
            ));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter, true);

        ThrowIfFailed(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
            ));
    }

    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_5 };
    if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
        || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_5))
    {
        OutputDebugStringA("ERROR: Shader Model 6.5 is not supported\n");
        throw std::exception("Shader Model 6.5 is not supported");
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS7 features = {};
    if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &features, sizeof(features)))
        || (features.MeshShaderTier == D3D12_MESH_SHADER_TIER_NOT_SUPPORTED))
    {
        OutputDebugStringA("ERROR: Mesh Shaders aren't supported!\n");
        throw std::exception("Mesh Shaders aren't supported!");
    }

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        Win32Application::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
        ));

    // This sample does not support fullscreen transitions.
    ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain.As(&m_swapChain));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

        m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV and a command allocator for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);

            ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n])));
        }
    }

    // Create the depth stencil view.
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
        depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
        depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

        D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
        depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
        depthOptimizedClearValue.DepthStencil.Stencil = 0;

        const CD3DX12_HEAP_PROPERTIES depthStencilHeapProps(D3D12_HEAP_TYPE_DEFAULT);
        const CD3DX12_RESOURCE_DESC depthStencilTextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

        ThrowIfFailed(m_device->CreateCommittedResource(
            &depthStencilHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &depthStencilTextureDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depthOptimizedClearValue,
            IID_PPV_ARGS(&m_depthStencil)
        ));

        NAME_D3D12_OBJECT(m_depthStencil);

        m_device->CreateDepthStencilView(m_depthStencil.Get(), &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    }

    // Create the constant buffer.
    {
        const UINT64 constantBufferSize = sizeof(Constants) * FrameCount;

        const CD3DX12_HEAP_PROPERTIES constantBufferHeapProps(D3D12_HEAP_TYPE_UPLOAD);
        const CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);

        ThrowIfFailed(m_device->CreateCommittedResource(
            &constantBufferHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &constantBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_constantBuffer)));

        // Describe and create a constant buffer view.
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = constantBufferSize;

        // Map and initialize the constant buffer. We don't unmap this until the
        // app closes. Keeping things mapped for the lifetime of the resource is okay.
        ThrowIfFailed(m_constantBuffer->Map(0, nullptr, &m_constantsData));
    }
}

// Load the sample assets.
void D3D12MeshletCull::LoadAssets()
{
    // Load shaders, root signature, and create the the pipeline state.
    {
        struct 
        { 
            byte* data; 
            uint32_t size; 
        } ampShader, meshShader, pixelShader;

        ReadDataFromFile(GetAssetFullPath(c_ampShaderFilename).c_str(), &ampShader.data, &ampShader.size);
        ReadDataFromFile(GetAssetFullPath(c_meshShaderFilename).c_str(), &meshShader.data, &meshShader.size);
        ReadDataFromFile(GetAssetFullPath(c_pixelShaderFilename).c_str(), &pixelShader.data, &pixelShader.size);

        // Pull root signature from the precompiled mesh shader.
        ThrowIfFailed(m_device->CreateRootSignature(0, ampShader.data, ampShader.size, IID_PPV_ARGS(&m_rootSignature)));

        // Disable culling so we can see the backside of geometry through the culled mesh.
        CD3DX12_RASTERIZER_DESC rasterDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        rasterDesc.CullMode = D3D12_CULL_MODE_NONE;

        D3DX12_MESH_SHADER_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature        = m_rootSignature.Get();
        psoDesc.AS                    = { ampShader.data, ampShader.size };
        psoDesc.MS                    = { meshShader.data, meshShader.size };
        psoDesc.PS                    = { pixelShader.data, pixelShader.size };
        psoDesc.NumRenderTargets      = 1;
        psoDesc.RTVFormats[0]         = m_renderTargets[0]->GetDesc().Format;
        psoDesc.DSVFormat             = m_depthStencil->GetDesc().Format;
        psoDesc.RasterizerState       = rasterDesc;
        psoDesc.BlendState            = CD3DX12_BLEND_DESC(D3D12_DEFAULT);         // Opaque
        psoDesc.DepthStencilState     = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // Less-equal depth test w/ writes; no stencil
        psoDesc.SampleMask            = UINT_MAX;
        psoDesc.SampleDesc            = DefaultSampleDesc();

        auto psoStream = CD3DX12_PIPELINE_MESH_STATE_STREAM(psoDesc);

        D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
        streamDesc.SizeInBytes                   = sizeof(psoStream);
        streamDesc.pPipelineStateSubobjectStream = &psoStream;

        ThrowIfFailed(m_device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_pipelineState)));
    }

    // Create the command list.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

    // Command lists are created in the recording state, but there is nothing
    // to record yet. The main loop expects it to be closed, so close it now.
    ThrowIfFailed(m_commandList->Close());

    // Load the objects from our scene definition
    m_objects.resize(_countof(c_sceneDefinition));

    for (uint32_t i = 0; i < static_cast<uint32_t>(m_objects.size()); ++i)
    {
        auto& obj = m_objects[i];
        auto& def = c_sceneDefinition[i];

        // Copy over the render flags
        obj.Flags |= def.Cull ? CULL_FLAG : 0;
        obj.Flags |= def.DrawMeshlets ? MESHLET_FLAG : 0;

        // Convert the transform definition to a matrix
        XMMATRIX world = XMMatrixAffineTransformation(
            XMVectorReplicate(def.Scale),
            g_XMZero,
            XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&def.Rotation)),
            XMLoadFloat3(&def.Position)
        );
        XMStoreFloat4x4(&obj.World, world);

        // Load and upload model resources to the GPU
        // Just use the D3D12_COMMAND_LIST_TYPE_DIRECT queue since it's a one-and-done operation. 
        // For per-frame uploads consider using the D3D12_COMMAND_LIST_TYPE_COPY command queue.
        obj.Model.LoadFromFile(c_modelFilenames[def.ModelIndex]);
        obj.Model.UploadGpuResources(m_device.Get(), m_commandQueue.Get(), m_commandAllocators[m_frameIndex].Get(), m_commandList.Get());

        const CD3DX12_HEAP_PROPERTIES instanceBufferHeapProps(D3D12_HEAP_TYPE_UPLOAD);
        const CD3DX12_RESOURCE_DESC instanceBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(Instance) * 2);

        // Create the per-object instance data buffer
        ThrowIfFailed(m_device->CreateCommittedResource(
            &instanceBufferHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &instanceBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&obj.InstanceResource)
        ));

        ThrowIfFailed(obj.InstanceResource->Map(0, nullptr, &obj.InstanceData));
    }

#ifdef _DEBUG
    // Mesh shader file expects a certain vertex layout; assert our mesh conforms to that layout.
    const D3D12_INPUT_ELEMENT_DESC c_elementDescs[2] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 1 },
    };

    for (auto& obj : m_objects)
    {
        for (auto& mesh : obj.Model)
        {
            assert(mesh.LayoutDesc.NumElements == 2);

            for (uint32_t i = 0; i < _countof(c_elementDescs); ++i)
                assert(std::memcmp(&mesh.LayoutElems[i], &c_elementDescs[i], sizeof(D3D12_INPUT_ELEMENT_DESC)) == 0);
        }
    }
#endif
    
    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValues[m_frameIndex]++;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        WaitForGpu();
    }

    // Initialize our debug visualization shader effects
    m_frustumDraw.CreateDeviceResources(m_device.Get(), m_renderTargets[0]->GetDesc().Format, m_depthStencil->GetDesc().Format);
    m_cullDataDraw.CreateDeviceResources(m_device.Get(), m_renderTargets[0]->GetDesc().Format, m_depthStencil->GetDesc().Format);
}

// Update frame-based values.
void D3D12MeshletCull::OnUpdate()
{
    m_timer.Tick(NULL);

    if (m_frameCounter++ % 30 == 0)
    {
        // Update window text with FPS value.
        wchar_t fps[64];
        swprintf_s(fps, L"%ufps", m_timer.GetFramesPerSecond());
        SetCustomWindowText(fps);
    }

    m_mainCam.Update(static_cast<float>(m_timer.GetElapsedSeconds()));
    m_debugCam.Update(static_cast<float>(m_timer.GetElapsedSeconds()));

    {
        XMMATRIX view = m_mainCam.GetViewMatrix();
        XMMATRIX proj = m_mainCam.GetProjectionMatrix(m_fovy, m_aspectRatio);
        XMMATRIX viewInv = XMMatrixInverse(nullptr, view);

        XMVECTOR scale, rot, viewPosition;
        XMMatrixDecompose(&scale, &rot, &viewPosition, viewInv);

        // Calculate the debug camera's properties to extract plane data.
        XMMATRIX cullWorld = XMMatrixInverse(nullptr, m_debugCam.GetViewMatrix());
        XMMATRIX cullView = m_debugCam.GetViewMatrix();
        XMMATRIX cullProj = m_debugCam.GetProjectionMatrix(m_fovy, m_aspectRatio, 1, 300.0f);

        XMVECTOR cullScale, cullRot, cullPos;
        XMMatrixDecompose(&cullScale, &cullRot, &cullPos, cullWorld);

        // Extract the planes from the debug camera view-projection matrix.
        XMMATRIX vp = XMMatrixTranspose(cullView * cullProj);
        XMVECTOR planes[6] =
        {
            XMPlaneNormalize(vp.r[3] + vp.r[0]), // Left
            XMPlaneNormalize(vp.r[3] - vp.r[0]), // Right
            XMPlaneNormalize(vp.r[3] + vp.r[1]), // Bottom
            XMPlaneNormalize(vp.r[3] - vp.r[1]), // Top
            XMPlaneNormalize(vp.r[2]),           // Near
            XMPlaneNormalize(vp.r[3] - vp.r[2]), // Far
        };

        // Set constant data to be read by the shaders.
        auto& constants = *(reinterpret_cast<Constants*>(m_constantsData) + m_frameIndex);

        constants.HighlightedIndex = m_highlightedIndex;
        constants.SelectedIndex = m_selectedIndex;
        constants.DrawMeshlets = m_drawMeshlets;

        XMStoreFloat4x4(&constants.View, XMMatrixTranspose(view));
        XMStoreFloat4x4(&constants.ViewProj, XMMatrixTranspose(view * proj));
        XMStoreFloat3(&constants.ViewPosition, viewPosition);
        XMStoreFloat3(&constants.CullViewPosition, cullPos);

        for (uint32_t i = 0; i < _countof(planes); ++i)
        {
            XMStoreFloat4(&constants.Planes[i], planes[i]);
        }

        // Update the planes for the Frustum drawer.
        m_frustumDraw.Update(view * proj, planes);

        // Copy the camera controller's world matrix to the scene object for rendering.
        XMStoreFloat4x4(&m_objects[DebugCamera].World, cullWorld);
    }

    // Update the scene objects with potentially modified data.
    for (auto& obj : m_objects)
    {
        XMMATRIX world = XMLoadFloat4x4(&obj.World);

        XMVECTOR scale, rot, pos;
        XMMatrixDecompose(&scale, &rot, &pos, world);

        auto& instance = *(reinterpret_cast<Instance*>(obj.InstanceData) + m_frameIndex);
        XMStoreFloat4x4(&instance.World, XMMatrixTranspose(world));
        XMStoreFloat4x4(&instance.WorldInvTrans, XMMatrixTranspose(XMMatrixInverse(nullptr, XMMatrixTranspose(world))));
        instance.Scale = XMVectorGetX(scale);
        instance.Flags = obj.Flags;
    }

    // Do ray intersection routine of the view model if in meshlet viewing mode.
    if (m_drawMeshlets)
    {
        Pick();
    }
}

// Render the scene.
void D3D12MeshletCull::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));

    MoveToNextFrame();
}

void D3D12MeshletCull::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForGpu();

    CloseHandle(m_fenceEvent);
}

void D3D12MeshletCull::OnKeyDown(UINT8 key)
{
    switch (key)
    {
    case VK_SPACE:
        m_drawMeshlets = !m_drawMeshlets;
        m_selectedIndex = uint32_t(-1);
        m_highlightedIndex = uint32_t(-1);
        break;

    case VK_TAB:
        m_selectedIndex = m_highlightedIndex;
        break;

    case VK_SHIFT:
        m_moveCam = true;
        break;
    }

    if (!m_moveCam)
    {
        m_mainCam.OnKeyDown(key);
    }
    else
    {
        m_debugCam.OnKeyDown(key);
    }
}

void D3D12MeshletCull::OnKeyUp(UINT8 key)
{
    switch (key)
    {
    case VK_SHIFT:
        m_moveCam = false;
        break;
    }

    m_mainCam.OnKeyUp(key);
    m_debugCam.OnKeyUp(key);
}

void D3D12MeshletCull::PopulateCommandList()
{
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_commandAllocators[m_frameIndex]->Reset());

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get()));

    // Set necessary state.
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    const auto toRenderTargetBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandList->ResourceBarrier(1, &toRenderTargetBarrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Set the root signature and bind resources
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress() + sizeof(Constants) * m_frameIndex);

    // Draw all scene objects with the same pipeline state.
    for (auto& obj : m_objects)
    {
        // Bind instance data
        m_commandList->SetGraphicsRootConstantBufferView(2, obj.InstanceResource->GetGPUVirtualAddress() + sizeof(Instance) * m_frameIndex);

        for (auto& mesh : obj.Model)
        {
            // Bind meshlet data
            m_commandList->SetGraphicsRootConstantBufferView(1, mesh.MeshInfoResource->GetGPUVirtualAddress());
            m_commandList->SetGraphicsRootShaderResourceView(3, mesh.VertexResources[0]->GetGPUVirtualAddress());
            m_commandList->SetGraphicsRootShaderResourceView(4, mesh.MeshletResource->GetGPUVirtualAddress());
            m_commandList->SetGraphicsRootShaderResourceView(5, mesh.UniqueVertexIndexResource->GetGPUVirtualAddress());
            m_commandList->SetGraphicsRootShaderResourceView(6, mesh.PrimitiveIndexResource->GetGPUVirtualAddress());
            m_commandList->SetGraphicsRootShaderResourceView(7, mesh.CullDataResource->GetGPUVirtualAddress());

            const uint32_t meshletCount = static_cast<uint32_t>(mesh.Meshlets.size());

            // Dispatch a single thread for each meshlet.
            m_commandList->DispatchMesh(DivRoundUp(meshletCount, AS_GROUP_SIZE), 1, 1);
        }
    }

    // Draw the frustum bounds of the culling camera.
    m_frustumDraw.Draw(m_commandList.Get());

    // Culling data visualization - only in meshlet viewing mode and have a meshlet selected.
    if (m_drawMeshlets && m_selectedIndex != uint32_t(-1))
    {
        auto& obj = m_objects[ViewModel];

        XMMATRIX world = XMLoadFloat4x4(&obj.World);
        XMMATRIX view = m_mainCam.GetViewMatrix();
        XMMATRIX proj = m_mainCam.GetProjectionMatrix(m_fovy, m_aspectRatio);

        m_cullDataDraw.SetConstants(world, view, proj, DirectX::Colors::Yellow);
        m_cullDataDraw.Draw(m_commandList.Get(), obj.Model.GetMesh(0), m_selectedIndex, 1);
    }

    // Indicate that the back buffer will now be used to present.
    const auto toPresentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_commandList->ResourceBarrier(1, &toPresentBarrier);

    ThrowIfFailed(m_commandList->Close());
}

// Wait for pending GPU work to complete.
void D3D12MeshletCull::WaitForGpu()
{
    // Schedule a Signal command in the queue.
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));

    // Wait until the fence has been processed.
    ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

    // Increment the fence value for the current frame.
    m_fenceValues[m_frameIndex]++;
}

// Prepare to render the next frame.
void D3D12MeshletCull::MoveToNextFrame()
{
    // Schedule a Signal command in the queue.
    const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

    // Update the frame index.
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
    }

    // Set the fence value for the next frame.
    m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}

XMVECTOR D3D12MeshletCull::GetSamplePoint() const
{
    POINT point;
    if (!GetCursorPos(&point))
        return g_XMZero;

    if (!ScreenToClient(Win32Application::GetHwnd(), &point))
        return g_XMZero;

    // Get a sample point to query the scene for meshlet intersections for picking
    return XMVectorSet(float(point.x), float(point.y), 1, 1);
}

void D3D12MeshletCull::Pick()
{
    m_highlightedIndex = uint32_t(-1);

    // Cache the object and its mesh (using only the first mesh of the model)
    auto& obj = m_objects[ViewModel];
    auto& mesh = obj.Model.GetMesh(0);

    // Grab the world, view, & proj matrices
    XMMATRIX world = XMLoadFloat4x4(&obj.World);
    XMMATRIX view = m_mainCam.GetViewMatrix();
    XMMATRIX proj = m_mainCam.GetProjectionMatrix(m_fovy, m_aspectRatio);

    // Determine the ray cast location in world space (sampling done in pixel space)
    XMVECTOR sampleSS = GetSamplePoint();
    XMVECTOR sampleWS = XMVector3Unproject(sampleSS, 0, 0, float(m_width), float(m_height), 0, 1, proj, view, XMMatrixIdentity());

    XMVECTOR viewPosWS = XMVector3Transform(g_XMZero, XMMatrixInverse(nullptr, view));
    XMVECTOR viewDirWS = XMVector3Normalize(sampleWS - viewPosWS);

    // Grab the vertex positions array
    const uint8_t* vbMem = mesh.Vertices[0].data();
    uint32_t stride = mesh.VertexStrides[0];
    uint32_t offset = ComputeSemanticByteOffset(mesh.LayoutDesc, "POSITION");
    assert(offset != uint32_t(-1));

    // Transform ray into object space for intersection tests
    XMMATRIX invWorld = XMMatrixInverse(nullptr, world);
    XMVECTOR dir = XMVector3Normalize(XMVector3TransformNormal(viewDirWS, invWorld));
    XMVECTOR org = XMVector3TransformCoord(viewPosWS, invWorld);

    XMVECTOR minT = g_XMFltMax;

    for (uint32_t i = 0; i < static_cast<uint32_t>(mesh.Meshlets.size()); ++i)
    {
        auto& meshlet = mesh.Meshlets[i];
        auto& cull = mesh.CullingData[i];

        // Quick narrow-phase test against the meshlet's sphere bounds.
        if (!RayIntersectSphere(org, dir, XMLoadFloat4(&cull.BoundingSphere)))
        {
            continue;
        }

        // Test each triangle of the meshlet.
        for (uint32_t j = 0; j < meshlet.PrimCount; ++j)
        {
            uint32_t i0, i1, i2;
            mesh.GetPrimitive(meshlet.PrimOffset + j, i0, i1, i2);

            uint32_t v0 = mesh.GetVertexIndex(meshlet.VertOffset + i0);
            uint32_t v1 = mesh.GetVertexIndex(meshlet.VertOffset + i1);
            uint32_t v2 = mesh.GetVertexIndex(meshlet.VertOffset + i2);

            XMVECTOR p0 = XMLoadFloat3((XMFLOAT3*)(vbMem + v0 * stride + offset));
            XMVECTOR p1 = XMLoadFloat3((XMFLOAT3*)(vbMem + v1 * stride + offset));
            XMVECTOR p2 = XMLoadFloat3((XMFLOAT3*)(vbMem + v2 * stride + offset));

            XMVECTOR t = RayIntersectTriangle(org, dir, p0, p1, p2);
            if (XMVector4Less(t, minT))
            {
                minT = t;
                m_highlightedIndex = i;
            }
        }
    }
}
