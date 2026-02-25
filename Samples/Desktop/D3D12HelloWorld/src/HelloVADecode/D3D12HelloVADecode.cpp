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
#include "D3D12HelloVADecode.h"
#include "DecodeParamBuffer.h"
#include <va/va.h>
#include <va/va_win32.h>

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 618; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\"; }

D3D12HelloVADecode::D3D12HelloVADecode(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name)
{
}

void D3D12HelloVADecode::OnInit()
{
    LoadD3D12Pipeline();
    LoadAssets();
    LoadVAPipeline();
}

void D3D12HelloVADecode::DestroyVAProc()
{
    VAStatus va_status = vaDestroyConfig(m_vaDisplay, m_VAProcConfigId);
    ThrowIfFailed(va_status, "vaDestroyConfig");

    va_status = vaDestroyContext(m_vaDisplay, m_vaColorConvCtx);
    ThrowIfFailed(va_status, "vaDestroyContext");

    va_status = vaDestroyBuffer(m_vaDisplay, m_vaColorConvBuf);
    ThrowIfFailed(va_status, "vaDestroyBuffer");
}

void D3D12HelloVADecode::DestroyVADec()
{
    VAStatus va_status = vaDestroyConfig(m_vaDisplay, m_VADecConfigId);
    ThrowIfFailed(va_status, "vaDestroyConfig");

    va_status = vaDestroyContext(m_vaDisplay, m_VADecContextId);
    ThrowIfFailed(va_status, "vaDestroyContext");

    for (UINT i = 0; i < _countof(m_VADecPipelineBufferId); i++) {
        vaDestroyBuffer(m_vaDisplay, m_VADecPipelineBufferId[i]);
        ThrowIfFailed(va_status, "vaDestroyBuffer");
    }
}

void D3D12HelloVADecode::DestroyVA()
{
    DestroyVAProc();
    DestroyVADec();

    VAStatus va_status = vaDestroySurfaces(m_vaDisplay, m_VARenderTargets, FrameCount);
    ThrowIfFailed(va_status, "vaDestroySurfaces");

    va_status = vaDestroySurfaces(m_vaDisplay, &m_VASurfaceNV12, 1);
    ThrowIfFailed(va_status, "vaDestroySurfaces");

    vaTerminate(m_vaDisplay);
    ThrowIfFailed(va_status, "vaTerminate");
}

void D3D12HelloVADecode::CreateVASurfaces()
{
    VASurfaceAttrib createSurfacesAttribList[2] = {
        {
            VASurfaceAttribPixelFormat,
            VA_SURFACE_ATTRIB_SETTABLE,
            {
                VAGenericValueTypeInteger,
                VA_FOURCC_NV12,
            },
        },
        {
            VASurfaceAttribMemoryType,
            VA_SURFACE_ATTRIB_SETTABLE,
            {
                VAGenericValueTypeInteger,
                VA_SURFACE_ATTRIB_MEM_TYPE_VA,
            },
        },
    };

    VAStatus va_status = vaCreateSurfaces(
        m_vaDisplay,
        VA_RT_FORMAT_YUV420,
        GetWidth(),
        GetHeight(),
        &m_VASurfaceNV12,
        1,
        createSurfacesAttribList,
        _countof(createSurfacesAttribList));
    ThrowIfFailed(va_status, "vaCreateSurfaces");
}

void D3D12HelloVADecode::LoadVAPipeline()
{    
    InitVADisplay();
    EnsureVAProcSupport();
    EnsureVADecSupport();
    CreateVASurfaces();
    ImportRenderTargetsToVA();
    InitVAProcContext();
    InitVADecContext();    
}

void D3D12HelloVADecode::InitVADisplay()
{
    DXGI_ADAPTER_DESC desc = {};
    ThrowIfFailed(m_adapter->GetDesc(&desc));
    m_vaDisplay = vaGetDisplayWin32(&desc.AdapterLuid);
    assert(m_vaDisplay);

    int major_ver, minor_ver;
    VAStatus va_status = vaInitialize(m_vaDisplay, &major_ver, &minor_ver);
    ThrowIfFailed(va_status, "vaInitialize");
}

void D3D12HelloVADecode::EnsureVADecSupport() {
    int num_entrypoints = vaMaxNumEntrypoints(m_vaDisplay);
    std::vector<VAEntrypoint> entrypoints(num_entrypoints);
    VAStatus va_status = vaQueryConfigEntrypoints(
        m_vaDisplay,
        VAProfileH264Main,
        entrypoints.data(),
        &num_entrypoints);
    ThrowIfFailed(va_status, "vaQueryConfigEntrypoints for VAProfileH264Main");

    bool supportsH264Dec = false;
    for (int32_t i = 0; !supportsH264Dec && i < num_entrypoints; i++) {
        if (entrypoints[i] == VAEntrypointVLD)
            supportsH264Dec = true;
    }

    if (!supportsH264Dec) {
        ThrowIfFailed(VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT, "VAEntrypointVLD not supported for VAProfileH264Main.");
    }
}

void D3D12HelloVADecode::EnsureVAProcSupport() {
    int num_entrypoints = vaMaxNumEntrypoints(m_vaDisplay);
    std::vector<VAEntrypoint> entrypoints(num_entrypoints);
    VAStatus va_status = vaQueryConfigEntrypoints(
        m_vaDisplay,
        VAProfileNone,
        entrypoints.data(),
        &num_entrypoints);
    ThrowIfFailed(va_status, "vaQueryConfigEntrypoints for VAProfileNone");

    bool supportsVideoProcessing = false;
    for (int32_t i = 0; !supportsVideoProcessing && i < num_entrypoints; i++) {
        if (entrypoints[i] == VAEntrypointVideoProc)
            supportsVideoProcessing = true;
    }

    if (!supportsVideoProcessing) {
        ThrowIfFailed(VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT, "VAEntrypointVideoProc not supported.");
    }

    // Check VPBlit support for format DXGI_FORMAT_R8G8B8A8_NV12 -> DXGI_FORMAT_R8G8B8A8_UNORM
    D3D12_FEATURE_DATA_VIDEO_PROCESS_SUPPORT dx12ProcCaps =
    {
        0, // NodeIndex
        { GetWidth(), GetHeight(), { DXGI_FORMAT_NV12, DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709 }},
        D3D12_VIDEO_FIELD_TYPE_NONE,
        D3D12_VIDEO_FRAME_STEREO_FORMAT_NONE,
        { 30, 1 },
        { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709},
        D3D12_VIDEO_FRAME_STEREO_FORMAT_NONE,
        { 30, 1 },
    };

    ComPtr<ID3D12VideoDevice> spVideoDevice;
    ThrowIfFailed(m_device->QueryInterface(IID_PPV_ARGS(spVideoDevice.GetAddressOf())));
    ThrowIfFailed(spVideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_PROCESS_SUPPORT, &dx12ProcCaps, sizeof(dx12ProcCaps)));
    if ((dx12ProcCaps.SupportFlags & D3D12_VIDEO_PROCESS_SUPPORT_FLAG_SUPPORTED) == 0) {
        ThrowIfFailed(VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT, "VAEntrypointVideoProc not supported for conversion DXGI_FORMAT_NV12 to DXGI_FORMAT_R8G8B8A8_UNORM.");
    }
}

void D3D12HelloVADecode::InitVAProcContext()
{
    VAStatus va_status = vaCreateConfig(
        m_vaDisplay,
        VAProfileNone,
        VAEntrypointVideoProc,
        nullptr,
        0,
        &m_VAProcConfigId);
    ThrowIfFailed(va_status, "vaCreateConfig");

    va_status = vaCreateContext(
        m_vaDisplay,
        m_VAProcConfigId,
        GetWidth(),
        GetHeight(),
        VA_PROGRESSIVE,
        m_VARenderTargets,
        FrameCount,
        &m_vaColorConvCtx);
    ThrowIfFailed(va_status, "vaCreateContext");
        
    va_status = vaCreateBuffer(
        m_vaDisplay,
        m_vaColorConvCtx,
        VAProcPipelineParameterBufferType,
        sizeof(VAProcPipelineParameterBuffer),
        1,
        NULL,
        &m_vaColorConvBuf);
    ThrowIfFailed(va_status, "vaCreateBuffer");
}

void D3D12HelloVADecode::ImportRenderTargetsToVA()
{
    VASurfaceAttrib createSurfacesAttribList[3] = {
        {
            VASurfaceAttribPixelFormat,
            VA_SURFACE_ATTRIB_SETTABLE,
            {
                VAGenericValueTypeInteger,
                // Based on the default render target
                // format DXGI_FORMAT_R8G8B8A8_UNORM
                VA_FOURCC_RGBA,
            },
        },
        {
            VASurfaceAttribMemoryType,
            VA_SURFACE_ATTRIB_SETTABLE,
            {
                VAGenericValueTypeInteger,
                // Using NTHandles for interop is the safe way of sharing resources between the 
                // VADisplay D3D12 device and the app/sample D3D12 independent devices
                VA_SURFACE_ATTRIB_MEM_TYPE_NTHANDLE,
            },
        },
        {
            VASurfaceAttribExternalBufferDescriptor,
            VA_SURFACE_ATTRIB_SETTABLE,
            {
                VAGenericValueTypePointer,
                // createSurfacesAttribList[2].value.value.p is set in code below
                0,
            },
        },
    };

    // The value here is an array of num_surfaces pointers to HANDLE so
    // each handle can be associated with the corresponding output surface
    // in the call to vaCreateSurfaces
    HANDLE renderTargets[FrameCount];
    for (size_t i = 0; i < FrameCount; i++)
    {
        HRESULT hr = m_device->CreateSharedHandle(m_renderTargets[i].Get(),
            nullptr,
            GENERIC_ALL,
            nullptr,
            &renderTargets[i]);
        ThrowIfFailed(hr);
    }
    createSurfacesAttribList[2].value.value.p = renderTargets;

    // Creates VASurface objects by importing
    // handles of existing D3D12 resources
    VAStatus va_status = vaCreateSurfaces(
        m_vaDisplay,
        VA_RT_FORMAT_RGB32, 
        GetWidth(),
        GetHeight(),
        m_VARenderTargets,
        FrameCount,
        createSurfacesAttribList,
        _countof(createSurfacesAttribList));
    ThrowIfFailed(va_status, "vaCreateSurfaces");
}

// Load the rendering pipeline dependencies.
void D3D12HelloVADecode::LoadD3D12Pipeline()
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
        ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&m_adapter)));
        ThrowIfFailed(D3D12CreateDevice(
            m_adapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
            ));
    }
    else
    {
        GetHardwareAdapter(factory.Get(), &m_adapter, true);
        ThrowIfFailed(D3D12CreateDevice(
            m_adapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
            ));        
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
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
}

// Load the sample assets.
void D3D12HelloVADecode::LoadAssets()
{
    // Create the command list.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));

    // Command lists are created in the recording state, but there is nothing
    // to record yet. The main loop expects it to be closed, so close it now.
    ThrowIfFailed(m_commandList->Close());

    // Create synchronization objects.
    {
        ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValue = 1;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }
    }
}

// Update frame-based values.
void D3D12HelloVADecode::OnUpdate()
{
}

// Render the scene.
void D3D12HelloVADecode::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    // In this case, clear the render target with a predefined color
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Before calling PerformVAWorkload, we must ensure the following:
    //  1. The D3D12 resources to be used must be in D3D12_RESOURCE_STATE_COMMON state
    //      * PopulateCommandList is already transitioning the resource to D3D12_RESOURCE_STATE_PRESENT
    //          which happens to also match the definition of D3D12_RESOURCE_STATE_COMMON
    //  2. The D3D12 resources must not have any pending GPU operations
    //      * Call WaitForPreviousFrame below for this end, to wait for the ExecuteCommandLists below
    //          that clears this render target with a predefined solid color.

    WaitForPreviousFrame();

    // Perform the VA workload on the current render target
    // The VA driver internally manages any other state transitions and it is expected that
    // PerformVAWorkload calls vaSyncSurface, which ensures the affected resources are
    // back in COMMON state and all the GPU work flushed and finished on them
    // Currently only m_VARenderTargets[m_frameIndex] is used in the VA workload,
    // transition it back to present mode for the call below.

    PerformVAWorkload();

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));

    WaitForPreviousFrame();
}

void D3D12HelloVADecode::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForPreviousFrame();

    DestroyVA();

    CloseHandle(m_fenceEvent);
}

void D3D12HelloVADecode::PopulateCommandList()
{
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_commandAllocator->Reset());

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

    // Indicate that the back buffer will be used as a render target.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());
}

void D3D12HelloVADecode::WaitForPreviousFrame()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    // Signal and increment the fence value.
    const UINT64 fence = m_fenceValue;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
    m_fenceValue++;

    // Wait until the previous frame is finished.
    if (m_fence->GetCompletedValue() < fence)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}


void D3D12HelloVADecode::InitVADecContext()
{
    VAStatus va_status = vaCreateConfig(
        m_vaDisplay,
        VAProfileH264Main,
        VAEntrypointVLD,
        nullptr,
        0,
        &m_VADecConfigId);
    ThrowIfFailed(va_status, "vaCreateConfig");

    va_status = vaCreateContext(
        m_vaDisplay,
        m_VADecConfigId,
        GetWidth(),
        GetHeight(),
        VA_PROGRESSIVE,
        m_VARenderTargets,
        FrameCount,
        &m_VADecContextId);
    ThrowIfFailed(va_status, "vaCreateContext");

    va_status = vaCreateBuffer(
        m_vaDisplay,
        m_VADecContextId,
        VAPictureParameterBufferType,
        sizeof(VAPictureParameterBufferH264),
        1,
        (void*) g_PicParams_AVC,
        &m_VADecPipelineBufferId[VA_H264DEC_BUFFER_INDEX_PIC]);
    ThrowIfFailed(va_status, "vaCreateBuffer");

    va_status = vaCreateBuffer(
        m_vaDisplay,
        m_VADecContextId,
        VAIQMatrixBufferType,
        sizeof(VAIQMatrixBufferH264),
        1,
        (void*) g_Qmatrix_AVC,
        &m_VADecPipelineBufferId[VA_H264DEC_BUFFER_INDEX_QMATRIX]);
    ThrowIfFailed(va_status, "vaCreateBuffer");

    va_status = vaCreateBuffer(
        m_vaDisplay,
        m_VADecContextId,
        VASliceDataBufferType,
        sizeof(g_Bitstream_AVC),
        1,
        (void*)g_Bitstream_AVC,
        &m_VADecPipelineBufferId[VA_H264DEC_BUFFER_INDEX_COMPRESSED_BIT]);
    ThrowIfFailed(va_status, "vaCreateBuffer");

    va_status = vaCreateBuffer(
        m_vaDisplay,
        m_VADecContextId,
        VASliceParameterBufferType,
        sizeof(VASliceParameterBufferH264),
        1,
        (void*) g_SlcParams_AVC,
        &m_VADecPipelineBufferId[VA_H264DEC_BUFFER_INDEX_SLICE]);
    ThrowIfFailed(va_status, "vaCreateBuffer");
}

void D3D12HelloVADecode::PerformVADecodeFrame(VASurfaceID dst_surface)
{
    VAStatus va_status;
    va_status = vaBeginPicture(m_vaDisplay, m_VADecContextId, dst_surface);
    ThrowIfFailed(va_status, "vaBeginPicture");
    
    vaRenderPicture(m_vaDisplay, m_VADecContextId, m_VADecPipelineBufferId, _countof(m_VADecPipelineBufferId));

    va_status = vaEndPicture(m_vaDisplay, m_VADecContextId);
    ThrowIfFailed(va_status, "vaEndPicture");

    // Wait for completion on GPU for the indicated VASurface
    va_status = vaSyncSurface(m_vaDisplay, dst_surface);
    ThrowIfFailed(va_status, "vaSyncSurface");
}

void D3D12HelloVADecode::PerformVAWorkload()
{
    // Decode H264 into a NV12 surface
    PerformVADecodeFrame(m_VASurfaceNV12);

    // Color convert NV12 into the RGBA render target
    PerformVABlit(m_vaColorConvCtx, m_vaColorConvBuf, &m_VASurfaceNV12, 1, NULL, NULL, m_VARenderTargets[m_frameIndex], 1.0f);
}

void D3D12HelloVADecode::PerformVABlit(
    VAContextID context,
    VABufferID buffer,
    VASurfaceID* pInSurfaces,
    UINT inSurfacesCount,
    VARectangle* pSrcRegions,
    VARectangle* pDstRegions,
    VASurfaceID dstSurface,
    float alpha)
{
    assert(inSurfacesCount == 1);

    VAStatus va_status;
    va_status = vaBeginPicture(m_vaDisplay, context, dstSurface);
    ThrowIfFailed(va_status, "vaBeginPicture");

    for (size_t i = 0; i < inSurfacesCount; i++)
    {
        VAProcPipelineParameterBuffer* pipeline_param;
        va_status = vaMapBuffer(m_vaDisplay, buffer, (void**)&pipeline_param);
        memset(pipeline_param, 0, sizeof(VAProcPipelineParameterBuffer));
        ThrowIfFailed(va_status, "vaMapBuffer");
        pipeline_param->surface = pInSurfaces[i];
        if (pSrcRegions)
            pipeline_param->surface_region = &pSrcRegions[i];
        if (pDstRegions)
            pipeline_param->output_region = &pDstRegions[i];

        // Check the VA platform can perform global alpha
        // blend using the queried capabilities previously
        VABlendState blend;
        if (m_ProcPipelineCaps.blend_flags & VA_BLEND_GLOBAL_ALPHA)
        {
            memset(&blend, 0, sizeof(VABlendState));
            blend.flags = VA_BLEND_GLOBAL_ALPHA;
            blend.global_alpha = alpha;
            pipeline_param->blend_state = &blend;
        }

        va_status = vaUnmapBuffer(m_vaDisplay, buffer);
        ThrowIfFailed(va_status, "vaUnMapBuffer");

        // Apply VPBlit
        vaRenderPicture(m_vaDisplay, context, &buffer, 1);
    }
    va_status = vaEndPicture(m_vaDisplay, context);
    ThrowIfFailed(va_status, "vaEndPicture");

    // Wait for completion on GPU for the indicated VASurface
    va_status = vaSyncSurface(m_vaDisplay, dstSurface);
    ThrowIfFailed(va_status, "vaSyncSurface");
}
