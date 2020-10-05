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
#include "D3D12PipelineStateCache.h"

const float D3D12PipelineStateCache::IntermediateClearColor[4] = { 0.0f, 0.2f, 0.3f, 1.0f };

D3D12PipelineStateCache::D3D12PipelineStateCache(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_psoLibrary(FrameCount, RootParameterUberShaderCB),
    m_frameIndex(0),
    m_swapChainEvent(0),
    m_drawIndex(0),
    m_maxDrawsPerFrame(256),
    m_dynamicCB(sizeof(DrawConstantBuffer), m_maxDrawsPerFrame, FrameCount),
    m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
    m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_rtvDescriptorSize(0),
    m_srvDescriptorSize(0),
    m_fenceValues{}
{
    memset(m_enabledEffects, true, sizeof(m_enabledEffects));

    ThrowIfFailed(DXGIDeclareAdapterRemovalSupport());
}

void D3D12PipelineStateCache::OnInit()
{
    UpdateWindowTextPso();
    m_camera.Init({ 0.0f, 0.0f, 5.0f });
    m_camera.SetMoveSpeed(1.0f);

    m_projectionMatrix = m_camera.GetProjectionMatrix(0.8f, m_aspectRatio);

    LoadPipeline();
    LoadAssets();
}

// Load the rendering pipeline dependencies.
void D3D12PipelineStateCache::LoadPipeline()
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

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
    NAME_D3D12_OBJECT(m_commandQueue);

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

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
    m_swapChainEvent = m_swapChain->GetFrameLatencyWaitableObject();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount + 1;    // A descriptor for each frame + 1 intermediate render target.
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        // Describe and create a shader resource view (SRV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = 1;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));
        NAME_D3D12_OBJECT(m_srvHeap);

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        m_srvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create RTVs and a command allocator for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);

            NAME_D3D12_OBJECT_INDEXED(m_renderTargets, n);

            ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n])));
        }

        D3D12_RESOURCE_DESC renderTargetDesc = m_renderTargets[0]->GetDesc();

        D3D12_CLEAR_VALUE clearValue = {};
        memcpy(clearValue.Color, IntermediateClearColor, sizeof(IntermediateClearColor));
        clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        // Create an intermediate render target that is the same dimensions as the swap chain.
        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &renderTargetDesc,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            &clearValue,
            IID_PPV_ARGS(&m_intermediateRenderTarget)));

        NAME_D3D12_OBJECT(m_intermediateRenderTarget);

        m_device->CreateRenderTargetView(m_intermediateRenderTarget.Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);

        // Create a SRV of the intermediate render target.
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = renderTargetDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart());
        m_device->CreateShaderResourceView(m_intermediateRenderTarget.Get(), &srvDesc, srvHandle);
    }
}

void D3D12PipelineStateCache::LoadAssets()
{
    // Create the root signature.
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        // We don't modify the SRV in the command list after SetGraphicsRootDescriptorTable
        // is executed on the GPU so we can use the default range behavior:
        // D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE
        CD3DX12_DESCRIPTOR_RANGE1 ranges[RootParametersCount];
        ranges[RootParameterSRV].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

        CD3DX12_ROOT_PARAMETER1 rootParameters[RootParametersCount];
        rootParameters[RootParameterUberShaderCB].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[RootParameterCB].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[RootParameterSRV].InitAsDescriptorTable(1, &ranges[RootParameterSRV], D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = 9999.0f;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
        NAME_D3D12_OBJECT(m_rootSignature);
    }

    // Create the command list.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
    NAME_D3D12_OBJECT(m_commandList);

    // Note: ComPtr's are CPU objects but this resource needs to stay in scope until
    // the command list that references it has finished executing on the GPU.
    // We will flush the GPU at the end of this method to ensure the resource is not
    // prematurely destroyed.
    ComPtr<ID3D12Resource> vertexIndexBufferUpload;

    // Vertex and Index Buffer.
    {
        const VertexPositionColor cubeVertices[] = {
            { { -1.0f, 1.0f, -1.0f, 1.0f }, { GetRandomColor(),GetRandomColor(), GetRandomColor() } },    // Back Top Left
            { { 1.0f, 1.0f, -1.0f, 1.0f }, { GetRandomColor(), GetRandomColor(), GetRandomColor() } },    // Back Top Right
            { { 1.0f, 1.0f, 1.0f, 1.0f }, { GetRandomColor(), GetRandomColor(), GetRandomColor() } },    // Front Top Right
            { { -1.0f, 1.0f, 1.0f, 1.0f }, { GetRandomColor(), GetRandomColor(), GetRandomColor() } },    // Front Top Left

            { { -1.0f, -1.0f, -1.0f, 1.0f }, { GetRandomColor(),GetRandomColor(), GetRandomColor() } },    // Back Bottom Left
            { { 1.0f, -1.0f, -1.0f, 1.0f }, { GetRandomColor(),GetRandomColor(), GetRandomColor() } },    // Back Bottom Right
            { { 1.0f, -1.0f, 1.0f, 1.0f }, { GetRandomColor(),GetRandomColor(), GetRandomColor() } },    // Front Bottom Right
            { { -1.0f, -1.0f, 1.0f, 1.0f }, { GetRandomColor(),GetRandomColor(), GetRandomColor() } },    // Front Bottom Left
        };

        const UINT cubeIndices[] =
        {
            0, 1, 3,
            1, 2, 3,

            3, 2, 7,
            6, 7, 2,

            2, 1, 6,
            5, 6, 1,

            1, 0, 5,
            4, 5, 0,

            0, 3, 4,
            7, 4, 3,

            7, 6, 4,
            5, 4, 6,
        };

        static const VertexPositionUV quadVertices[] =
        {
            { { -1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },    // Bottom Left
            { { -1.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },    // Top Left
            { { 1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },    // Bottom Right
            { { 1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },        // Top Right
        };

        const UINT vertexIndexBufferSize = sizeof(cubeIndices) + sizeof(cubeVertices) + sizeof(quadVertices);

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(vertexIndexBufferSize),
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_vertexIndexBuffer)));

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(vertexIndexBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&vertexIndexBufferUpload)));

        NAME_D3D12_OBJECT(m_vertexIndexBuffer);

        UINT8* mappedUploadHeap = nullptr;
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(vertexIndexBufferUpload->Map(0, &readRange, reinterpret_cast<void**>(&mappedUploadHeap)));

        // Fill in part of the upload heap with our index and vertex data.
        UINT8* heapLocation = static_cast<UINT8*>(mappedUploadHeap);
        memcpy(heapLocation, cubeVertices, sizeof(cubeVertices));
        heapLocation += sizeof(cubeVertices);
        memcpy(heapLocation, cubeIndices, sizeof(cubeIndices));
        heapLocation += sizeof(cubeIndices);
        memcpy(heapLocation, quadVertices, sizeof(quadVertices));

        // Pack the vertices and indices into their destination by copying from the upload heap.
        m_commandList->CopyBufferRegion(m_vertexIndexBuffer.Get(), 0, vertexIndexBufferUpload.Get(), 0, vertexIndexBufferSize);
        m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexIndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_INDEX_BUFFER));

        // Create the index and vertex buffer views.
        m_cubeVbv.BufferLocation = m_vertexIndexBuffer.Get()->GetGPUVirtualAddress();
        m_cubeVbv.SizeInBytes = sizeof(cubeVertices);
        m_cubeVbv.StrideInBytes = sizeof(VertexPositionColor);

        m_cubeIbv.BufferLocation = m_cubeVbv.BufferLocation + sizeof(cubeVertices);
        m_cubeIbv.SizeInBytes = sizeof(cubeIndices);
        m_cubeIbv.Format = DXGI_FORMAT_R32_UINT;

        m_quadVbv.BufferLocation = m_cubeIbv.BufferLocation + sizeof(cubeIndices);
        m_quadVbv.SizeInBytes = sizeof(quadVertices);
        m_quadVbv.StrideInBytes = sizeof(VertexPositionUV);
    }

    // Create the constant buffer.
    m_dynamicCB.Init(m_device.Get());

    // Close the command list and execute it to begin the vertex/index buffer copy
    // into the default heap.
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(m_device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
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

    m_psoLibrary.Build(m_device.Get(), m_rootSignature.Get());
    UpdateWindowTextPso();
}

// Update frame-based values.
void D3D12PipelineStateCache::OnUpdate()
{
    // Wait for the previous Present to complete.
    WaitForSingleObjectEx(m_swapChainEvent, 100, FALSE);

    m_timer.Tick(NULL);
    m_camera.Update(static_cast<float>(m_timer.GetElapsedSeconds()));
}

// Render the scene.
void D3D12PipelineStateCache::OnRender()
{
    try
    {
        PIXBeginEvent(m_commandQueue.Get(), 0, L"Render");

        // Record all the commands we need to render the scene into the command list.
        PopulateCommandList();

        // Execute the command list.
        ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        PIXEndEvent(m_commandQueue.Get());

        // Present the frame.
        ThrowIfFailed(m_swapChain->Present(1, 0));

        m_drawIndex = 0;
        m_psoLibrary.EndFrame();

        MoveToNextFrame();
    }
    catch (HrException& e)
    {
        if (e.Error() == DXGI_ERROR_DEVICE_REMOVED || e.Error() == DXGI_ERROR_DEVICE_RESET)
        {
            RestoreD3DResources();
        }
        else
        {
            throw;
        }
    }
}

// Release sample's D3D objects.
void D3D12PipelineStateCache::ReleaseD3DResources()
{
    m_fence.Reset();
    ResetComPtrArray(&m_renderTargets);
    m_commandQueue.Reset();
    m_swapChain.Reset();
    m_device.Reset();
}

// Tears down D3D resources and reinitializes them.
void D3D12PipelineStateCache::RestoreD3DResources()
{
    // Give GPU a chance to finish its execution in progress.
    try
    {
        WaitForGpu();
    }
    catch (HrException&)
    {
        // Do nothing, currently attached adapter is unresponsive.
    }
    ReleaseD3DResources();
    OnInit();
}


void D3D12PipelineStateCache::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForGpu();

    CloseHandle(m_fenceEvent);
}

void D3D12PipelineStateCache::OnKeyDown(UINT8 key)
{
    m_camera.OnKeyDown(key);
}

void D3D12PipelineStateCache::OnKeyUp(UINT8 key)
{
    m_camera.OnKeyUp(key);
    
    switch (key)
    {
    case 'C':
        WaitForGpu();
        m_psoLibrary.ClearPSOCache();
        m_psoLibrary.Build(m_device.Get(), m_rootSignature.Get());
        break;

    case 'U':
        m_psoLibrary.ToggleUberShader();
        break;

    case 'L':
        m_psoLibrary.ToggleDiskLibrary();
        break;

    case 'M':
        m_psoLibrary.SwitchPSOCachingMechanism();
        break;

    case '1':
        ToggleEffect(PostBlit);
        break;

    case '2':
        ToggleEffect(PostInvert);
        break;

    case '3':
        ToggleEffect(PostGrayScale);
        break;

    case '4':
        ToggleEffect(PostEdgeDetect);
        break;

    case '5':
        ToggleEffect(PostBlur);
        break;

    case '6':
        ToggleEffect(PostWarp);
        break;

    case '7':
        ToggleEffect(PostPixelate);
        break;

    case '8':
        ToggleEffect(PostDistort);
        break;

    case '9':
        ToggleEffect(PostWave);
        break;

    default:
        break;
    }

    UpdateWindowTextPso();
}

// Fill the command list with all the render commands and dependent state.
void D3D12PipelineStateCache::PopulateCommandList()
{
    // Command list allocators can only be reset when the associated
    // command lists have finished execution on the GPU; apps should use
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_commandAllocators[m_frameIndex]->Reset());

    // However, when ExecuteCommandList() is called on a particular command
    // list, that command list can then be reset at any time and must be before
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr));

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE intermediateRtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), FrameCount, m_rtvDescriptorSize);
    CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(m_srvHeap->GetGPUDescriptorHandleForHeapStart());

    m_commandList->OMSetRenderTargets(1, &intermediateRtvHandle, FALSE, nullptr);

    // Record commands.
    m_commandList->ClearRenderTargetView(intermediateRtvHandle, IntermediateClearColor, 0, nullptr);

    // Draw the scene as normal into the intermediate buffer.
    PIXBeginEvent(m_commandList.Get(), 0, L"Draw cube");
    {
        static float rot = 0.0f;
        DrawConstantBuffer* drawCB = (DrawConstantBuffer*)m_dynamicCB.GetMappedMemory(m_drawIndex, m_frameIndex);
        drawCB->worldViewProjection = XMMatrixTranspose(XMMatrixRotationY(rot) * XMMatrixRotationX(-rot) * m_camera.GetViewMatrix() * m_projectionMatrix);

        rot += 0.01f;

        m_commandList->IASetVertexBuffers(0, 1, &m_cubeVbv);
        m_commandList->IASetIndexBuffer(&m_cubeIbv);
        m_psoLibrary.SetPipelineState(m_device.Get(), m_rootSignature.Get(), m_commandList.Get(), BaseNormal3DRender, m_frameIndex);

        m_commandList->SetGraphicsRootConstantBufferView(RootParameterCB, m_dynamicCB.GetGpuVirtualAddress(m_drawIndex, m_frameIndex));
        m_commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
        m_drawIndex++;
    }
    PIXEndEvent(m_commandList.Get());

    // Set up the state for a fullscreen quad.
    m_commandList->IASetVertexBuffers(0, 1, &m_quadVbv);
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // Indicate that the back buffer will be used as a render target and the
    // intermediate render target will be used as a SRV.
    D3D12_RESOURCE_BARRIER barriers[] = {
        CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET),
        CD3DX12_RESOURCE_BARRIER::Transition(m_intermediateRenderTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
    };

    m_commandList->ResourceBarrier(_countof(barriers), barriers);
    m_commandList->SetGraphicsRootDescriptorTable(RootParameterSRV, m_srvHeap->GetGPUDescriptorHandleForHeapStart());

    const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, black, 0, nullptr);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Draw some quads using the rendered scene with some effect shaders.
    PIXBeginEvent(m_commandList.Get(), 0, L"Post-processing");
    {
        UINT quadCount = 0;
        static const UINT quadsX = 3;
        static const UINT quadsY = 3;

        // Cycle through all of the effects.
        for (UINT i = PostBlit; i < EffectPipelineTypeCount; i++)
        {
            if (m_enabledEffects[i])
            {
                CD3DX12_VIEWPORT viewport(
                    (quadCount % quadsX) * (m_viewport.Width / quadsX),
                    (quadCount / quadsY) * (m_viewport.Height / quadsY),
                    m_viewport.Width / quadsX,
                    m_viewport.Height / quadsY);

                PIXBeginEvent(m_commandList.Get(), 0, g_cEffectNames[i]);
                m_commandList->RSSetViewports(1, &viewport);
                m_psoLibrary.SetPipelineState(m_device.Get(), m_rootSignature.Get(), m_commandList.Get(), static_cast<EffectPipelineType>(i), m_frameIndex);
                m_commandList->DrawInstanced(4, 1, 0, 0);
                PIXEndEvent(m_commandList.Get());
            }

            quadCount++;
        }
    }
    PIXEndEvent(m_commandList.Get());

    // Revert resource states back to original values.
    barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    m_commandList->ResourceBarrier(_countof(barriers), barriers);

    ThrowIfFailed(m_commandList->Close());
}

void D3D12PipelineStateCache::ToggleEffect(EffectPipelineType type)
{
    if (m_enabledEffects[type])
    {
        // Wait until all frames using the shader are rendered before destroying
        // the shader.
        WaitForGpu();

        m_psoLibrary.DestroyShader(type);
    }

    m_enabledEffects[type] = !m_enabledEffects[type];
}

void D3D12PipelineStateCache::UpdateWindowTextPso()
{
    std::wstringstream stringStream;
    stringStream << L"  [Use Uber Shader: ";
    stringStream << (m_psoLibrary.UberShadersEnabled() ? L"true]" : L"false]");
    stringStream << L"   [Use DiskCache: ";

    if (m_psoLibrary.DiskCacheEnabled())
    {
        stringStream << L"true, ";
        switch (m_psoLibrary.GetPSOCachingMechanism())
        {
        case PSOCachingMechanism::CachedBlobs:
                stringStream << L"CachedBlobs]";
                break;
        case PSOCachingMechanism::PipelineLibraries:
            stringStream << L"PipelineLibraries]";
            break;
        }
    }
    else
    {
        stringStream <<  L"false]";
    }

    SetCustomWindowText(stringStream.str().c_str());
}

// Wait for pending GPU work to complete.
void D3D12PipelineStateCache::WaitForGpu()
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
void D3D12PipelineStateCache::MoveToNextFrame()
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
