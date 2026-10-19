//*********************************************************
//
// Copyright (c) 2024-2025, NVIDIA CORPORATION. All rights reserved.
// Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// Scene construction, domino orientation and seed velocities adapt the public
// vk_partitioned_tlas/src/partitioned_tlas_scene.cpp implementation.
// See THIRD_PARTY_NOTICES.md for source revisions and acknowledgments.

#include "DXSampleHelper.h"
#include "D3D12RaytracingPartitionedTLAS.h"
#include "Win32Application.h"
#include "ShaderCompiler.h"
#include "PtlasShaderBindings.h"
#include <windowsx.h>
#include <d3dcompiler.h>
#include <d3dx12.h>
#include <pix3.h>
#include <ScreenGrab.h>
#include <shellapi.h>
#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cwchar>
#include <stdexcept>
#include <string_view>
#include "scene.hpp"
#include "SceneMeshes.h"

namespace
{
void SetDebugName(ID3D12Object* object, const wchar_t* label)
{
    ThrowIfFailed(object->SetName(label), "SetName");
}

ComPtr<ID3D12RootSignature> SerializeAndCreateRootSignature(ID3D12Device* device,
                                                            const D3D12_ROOT_SIGNATURE_DESC& description)
{
    ComPtr<ID3DBlob> blob;
    ComPtr<ID3DBlob> errors;
    const HRESULT result = D3D12SerializeRootSignature(&description, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &errors);
    if (FAILED(result) && errors)
        throw std::runtime_error(static_cast<const char*>(errors->GetBufferPointer()));
    ThrowIfFailed(result, "D3D12SerializeRootSignature");
    ComPtr<ID3D12RootSignature> root;
    ThrowIfFailed(device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&root)),
                  "CreateRootSignature");
    return root;
}

std::filesystem::path GetExecutableDirectory()
{
    wchar_t executable[MAX_PATH]{};
    if (!GetModuleFileNameW(nullptr, executable, MAX_PATH))
        throw std::runtime_error("GetModuleFileNameW failed");
    return std::filesystem::path(executable).parent_path();
}

UINT64 AlignUp(UINT64 value, UINT64 alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

UINT WriteDebugMessages(ID3D12InfoQueue* infoQueue, std::ostream& log)
{
    UINT errorCount = 0;
    const UINT64 count = infoQueue->GetNumStoredMessages();
    for (UINT64 index = 0; index < count; ++index)
    {
        SIZE_T bytes = 0;
        ThrowIfFailed(infoQueue->GetMessage(index, nullptr, &bytes), "GetMessage(size)");
        std::vector<BYTE> storage(bytes);
        auto* message = reinterpret_cast<D3D12_MESSAGE*>(storage.data());
        ThrowIfFailed(infoQueue->GetMessage(index, message, &bytes), "GetMessage(data)");
        log << static_cast<int>(message->Severity) << ',' << message->ID << ',' << message->pDescription << '\n';
        if (message->Severity == D3D12_MESSAGE_SEVERITY_ERROR || message->Severity == D3D12_MESSAGE_SEVERITY_CORRUPTION)
            ++errorCount;
    }
    log << "messages=" << count << ", errors=" << errorCount << '\n';
    return errorCount;
}

} // namespace

void D3D12RaytracingPartitionedTLAS::ParseCommandLineArgs()
{
    int count = 0;
    LPWSTR* arguments = CommandLineToArgvW(GetCommandLineW(), &count);
    if (!arguments)
        throw std::runtime_error("CommandLineToArgvW failed");
    for (int index = 1; index < count; ++index)
    {
        const std::wstring_view argument(arguments[index]);
        if (argument == L"--frames" && index + 1 < count)
            m_frameLimit = static_cast<UINT64>(std::stoull(arguments[++index]));
        else if (argument == L"--test-device-loss-frame" && index + 1 < count)
            m_testDeviceLossFrame = std::stoull(arguments[++index]);
        else if (argument == L"--mode" && index + 1 < count)
        {
            const unsigned mode = static_cast<unsigned>(std::stoul(arguments[++index]));
            if (mode < 1 || mode > 3)
                throw std::runtime_error("--mode must be 1, 2 or 3");
            m_updateMode = mode - 1;
        }
        else if (argument == L"--csv" && index + 1 < count)
        {
            m_csv.open(std::filesystem::path(arguments[++index]));
            if (!m_csv)
                throw std::runtime_error("Could not open CSV output");
            m_csv << "frame,as,mode,hint,writes,gpu_animation_ms,gpu_as_ms,gpu_update_ms,gpu_trace_ms,gpu_composite_ms,"
                     "gpu_total_ms\n";
        }
        else if (argument == L"--state-dump" && index + 1 < count)
            m_stateDumpPath = std::filesystem::path(arguments[++index]);
        else if (argument == L"--screenshot" && index + 1 < count)
            m_screenshotPath = std::filesystem::path(arguments[++index]);
        else if (argument == L"--image" && index + 1 < count)
            m_imagePath = std::filesystem::path(arguments[++index]);
        else if (argument == L"--tlas")
            m_usePtlas = false;
        else if (argument == L"--ptlas")
            m_usePtlas = true;
        else if (argument == L"--no-partition-colors")
            m_showPartitionColors = false;
        else if (argument == L"--partition-colors")
            m_showPartitionColors = true;
        else if (argument == L"--paused")
            m_paused = true;
        else if (argument == L"--self-topple-count" && index + 1 < count)
            m_selfToppleCount = static_cast<UINT>(std::stoul(arguments[++index]));
        else if (argument == L"--mark-all")
            m_markAllInPartition = true;
        else if (argument == L"--no-vsync")
            m_vsync = false;
        else if (argument == L"--vsync")
            m_vsync = true;
        else if (argument == L"--debug")
            m_debug = true;
        else if ((argument == L"--adapter" || argument == L"-forceAdapter") && index + 1 < count)
            m_adapterIndex = static_cast<UINT>(std::stoul(arguments[++index]));
        else if (argument == L"--reload-shaders")
            m_reloadShadersRequested = true;
        else if (argument == L"--ptlas-hint" && index + 1 < count)
        {
            const std::wstring_view hint(arguments[++index]);
            if (hint == L"none")
                m_ptlasHint = D3D12_RTAS_PARTITIONED_TLAS_FLAG_NONE;
            else if (hint == L"build")
                m_ptlasHint = D3D12_RTAS_PARTITIONED_TLAS_FLAG_FAST_BUILD;
            else if (hint == L"trace")
                m_ptlasHint = D3D12_RTAS_PARTITIONED_TLAS_FLAG_FAST_TRACE;
            else
                throw std::runtime_error("--ptlas-hint must be none, build or trace");
        }
        else if (argument == L"--axis" && index + 1 < count)
            m_dominoesPerAxis = static_cast<UINT>(std::stoul(arguments[++index]));
        else if (argument == L"--partitions" && index + 1 < count)
            m_partitionCountPerAxis = static_cast<UINT>(std::stoul(arguments[++index]));
        else if (argument == L"--scene-size" && index + 1 < count)
            m_sceneSize = std::stof(arguments[++index]);
        else if (argument == L"--static-count" && index + 1 < count)
            m_staticCount = static_cast<UINT>(std::stoul(arguments[++index]));
        else if (argument == L"--width" && index + 1 < count)
            m_width = static_cast<UINT>(std::stoul(arguments[++index]));
        else if (argument == L"--height" && index + 1 < count)
            m_height = static_cast<UINT>(std::stoul(arguments[++index]));
        else if (argument == L"--render-scale" && index + 1 < count)
            m_renderScale = static_cast<UINT>(std::stoul(arguments[++index]));
        else if (argument == L"--pick" && index + 2 < count)
        {
            m_pickX = std::stoi(arguments[++index]);
            m_pickY = std::stoi(arguments[++index]);
            m_pickRequested = true;
        }
        else if (argument == L"--full")
        {
            m_fullScene = true;
            m_partitionCountPerAxis = 50;
            m_staticCount = UINT_MAX;
        }
        else
            throw std::runtime_error("Unknown or incomplete command line option");
    }
    LocalFree(arguments);
#ifdef _DEBUG
    m_debug = true;
#endif
    if (m_selfToppleCount == UINT_MAX)
        m_selfToppleCount = m_fullScene ? 2000u : 1u;
    if (m_dominoesPerAxis < 1 || m_dominoesPerAxis > 1000 || m_partitionCountPerAxis < 1 ||
        (!m_fullScene && m_partitionCountPerAxis > m_dominoesPerAxis) ||
        (m_staticCount != UINT_MAX && m_staticCount > 2000000) ||
        (m_fullScene &&
         (!std::isfinite(m_sceneSize) || m_sceneSize < 1.5f * m_partitionCountPerAxis || m_sceneSize > 1000.0f)) ||
        m_selfToppleCount > 1000000 || m_width < 320 || m_width > 3840 || m_height < 240 || m_height > 2160 ||
        m_renderScale < 1 || m_renderScale > 2)
        throw std::runtime_error("Invalid --axis or --partitions value");
    if (!m_fullScene)
        m_dominoCount = m_dominoesPerAxis * m_dominoesPerAxis;
    m_staticCountLimit = m_fullScene ? m_staticCount : UINT_MAX;
    m_modeDistance = m_fullScene ? m_sceneSize / 10.0f : std::max(22.0f, m_dominoesPerAxis * 1.3f / 10.0f);
    m_totalInstanceCount = m_fullScene ? 0 : m_dominoCount + m_staticCount;
    m_partitionCount = m_partitionCountPerAxis * m_partitionCountPerAxis;
    const UINT maximumAxisSpan = (m_dominoesPerAxis + m_partitionCountPerAxis - 1) / m_partitionCountPerAxis;
    m_maxInstancesPerPartition = maximumAxisSpan * maximumAxisSpan;
}

void D3D12RaytracingPartitionedTLAS::Run(HINSTANCE instance, int showCommand)
{
    m_window = Win32Application::Create(instance, WindowProc, this, m_width, m_height);
    OnInit();
    ShowWindow(m_window, showCommand);
    Win32Application::Run([this] { OnRender(); }, [this] { return m_minimized; });
    if (m_windowException)
        std::rethrow_exception(m_windowException);
    OnDestroy();
}

void D3D12RaytracingPartitionedTLAS::OnInit()
{
    m_deviceResources.RegisterDeviceNotify(this);
    m_deviceResources.CreateDevice(m_debug, m_adapterIndex);
    m_deviceResources.CreateSwapChain(m_window, m_width, m_height);
    OnDeviceRestored();
}

void D3D12RaytracingPartitionedTLAS::OnDeviceRestored()
{
    m_device = m_deviceResources.GetDevice();
    m_commandList = m_deviceResources.GetCommandList();
    m_dxr2Device = m_deviceResources.GetRaytracingDevice();
    m_dxr2CommandList = m_deviceResources.GetRaytracingCommandList();
    CreateWindowSizeDependentResources();
    CreateDeviceDependentResources();
    m_deviceReady = true;
    m_keyboardButtons.Reset();
    // A key held during reconstruction is not a new button press.
    m_keyboardButtons.Update(m_keyboard.GetState());
    m_lastCameraTime = {};
    UpdateTitle();
}

void D3D12RaytracingPartitionedTLAS::CreateDeviceDependentResources()
{
    BuildGeometryAndBLAS();
    CreateSceneAndTopLevels();
    if (m_shaderBytecodes.raytracing.empty())
        m_shaderBytecodes = LoadSampleShaders(GetExecutableDirectory());
    CreateRaytracingPipelineStateObject(m_shaderBytecodes);
    CreateCompositingPipeline(m_shaderBytecodes);
    CreateAnimationPipelines(m_shaderBytecodes);
    CreateAnimationResources();
    CreateFrameConstants();
    CreateTimingQueries();
    m_ui.CreateDeviceResources(m_device.Get(), m_deviceResources.GetCommandQueue(),
                               GetExecutableDirectory() / L"SegoeUI_18.spritefont");
    m_frames[0].allocator = m_deviceResources.GetCommandAllocator();
    ThrowIfFailed(
        m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_frames[1].allocator)),
        "CreateCommandAllocator(frame 1)");
    ThrowIfFailed(m_commandList->Close(), "Close(initial command list)");
}

void D3D12RaytracingPartitionedTLAS::OnDestroy()
{
    m_deviceResources.WaitForGpu();
    for (UINT offset = 0; offset < kFrameCount; ++offset)
    {
        FrameContext& frame = m_frames[(m_frameSlot + offset) % kFrameCount];
        if (frame.pending)
            CollectFrame(frame);
    }
    if (m_stateDumpPending)
        WriteStateDump();
    if (m_debug)
        InspectDebugMessages();
}

void D3D12RaytracingPartitionedTLAS::OnUpdate()
{
    UpdateInput();
    DrawInterface();
    UpdateCamera();
    UpdateAnimation();
}

D3D12RaytracingPartitionedTLAS::~D3D12RaytracingPartitionedTLAS()
{
    m_deviceResources.WaitForGpuNoThrow();
    m_ui.ReleaseDeviceResources();
    if (IsWindow(m_window))
    {
        SetWindowLongPtrW(m_window, GWLP_USERDATA, 0);
        DestroyWindow(m_window);
    }
}

LRESULT CALLBACK D3D12RaytracingPartitionedTLAS::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    D3D12RaytracingPartitionedTLAS* app =
        reinterpret_cast<D3D12RaytracingPartitionedTLAS*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    try
    {
        DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
        switch (message)
        {
        case WM_SYSKEYDOWN:
            if (wParam == VK_RETURN && (lParam & (1 << 29)) && !(lParam & (1 << 30)))
            {
                Win32Application::ToggleFullscreen(window);
                return 0;
            }
            break;
        case WM_NCCREATE: {
            auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
            SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create->lpCreateParams));
            break;
        }
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE)
            {
                DestroyWindow(window);
                return 0;
            }
            break;
        case WM_LBUTTONDOWN:
            if (app && !app->m_ui.IsEditing())
            {
                app->m_draggingCamera = true;
                app->m_lastMouseX = GET_X_LPARAM(lParam);
                app->m_lastMouseY = GET_Y_LPARAM(lParam);
                SetCapture(window);
                return 0;
            }
            break;
        case WM_LBUTTONUP:
            if (app && app->m_draggingCamera)
            {
                app->m_draggingCamera = false;
                ReleaseCapture();
                return 0;
            }
            break;
        case WM_MOUSEMOVE:
            if (app && app->m_draggingCamera)
            {
                const int x = GET_X_LPARAM(lParam);
                const int y = GET_Y_LPARAM(lParam);
                app->m_cameraYaw += (x - app->m_lastMouseX) * 0.005f;
                app->m_cameraPitch = std::clamp(app->m_cameraPitch - (y - app->m_lastMouseY) * 0.005f, -1.5f, 1.5f);
                app->m_lastMouseX = x;
                app->m_lastMouseY = y;
                return 0;
            }
            break;
        case WM_RBUTTONDOWN:
            if (app && !app->m_ui.IsEditing())
            {
                app->m_pickX = GET_X_LPARAM(lParam);
                app->m_pickY = GET_Y_LPARAM(lParam);
                app->m_pickRequested = true;
                return 0;
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            if (app)
            {
                app->OnSizeChanged(LOWORD(lParam), HIWORD(lParam), wParam == SIZE_MINIMIZED);
                return 0;
            }
            break;
        default:
            break;
        }
        return DefWindowProcW(window, message, wParam, lParam);
    }
    catch (const HrException& error)
    {
        if (app && app->m_deviceResources.IsDeviceLost(error.Error()))
            app->m_deviceRecoveryRequested = true;
        else
        {
            if (app)
                app->m_windowException = std::current_exception();
            PostQuitMessage(EXIT_FAILURE);
        }
        return 0;
    }
    catch (...)
    {
        // Do not unwind a C++ exception through the Windows callback boundary.
        if (app)
            app->m_windowException = std::current_exception();
        PostQuitMessage(EXIT_FAILURE);
        return 0;
    }
}

void D3D12RaytracingPartitionedTLAS::OnSizeChanged(UINT width, UINT height, bool minimized)
{
    m_minimized = minimized || width == 0 || height == 0;
    if (m_minimized)
        return;
    if (width == m_width && height == m_height)
        return;
    m_width = width;
    m_height = height;
    if (!m_deviceReady)
        return;
    m_deviceResources.ResizeSwapChain(width, height);
    for (FrameContext& frame : m_frames)
        if (frame.pending)
            CollectFrame(frame);
    CreateWindowSizeDependentResources();
    m_pickRequested = false;
}

ComPtr<ID3D12Resource> D3D12RaytracingPartitionedTLAS::CreateBuffer(UINT64 bytes, D3D12_HEAP_TYPE heapType,
                                                                    D3D12_RESOURCE_FLAGS flags,
                                                                    D3D12_RESOURCE_STATES state)
{
    const CD3DX12_HEAP_PROPERTIES heap(heapType);
    const auto description = CD3DX12_RESOURCE_DESC::Buffer(std::max<UINT64>(bytes, 256), flags);
    ComPtr<ID3D12Resource> resource;
    // Ordinary default-heap buffers begin in COMMON; their first use promotes
    // them to the requested state. Acceleration structures need their AS state.
    const auto creationState =
        heapType == D3D12_HEAP_TYPE_DEFAULT && state != D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE
            ? D3D12_RESOURCE_STATE_COMMON
            : state;
    ThrowIfFailed(m_device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &description, creationState, nullptr,
                                                    IID_PPV_ARGS(&resource)),
                  "CreateCommittedResource(buffer)");
    return resource;
}

template <typename T> ComPtr<ID3D12Resource> D3D12RaytracingPartitionedTLAS::Upload(const std::vector<T>& values)
{
    ComPtr<ID3D12Resource> resource = CreateBuffer(values.size() * sizeof(T), D3D12_HEAP_TYPE_UPLOAD,
                                                   D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);
    void* destination = nullptr;
    ThrowIfFailed(resource->Map(0, nullptr, &destination), "Map(upload)");
    std::memcpy(destination, values.data(), values.size() * sizeof(T));
    resource->Unmap(0, nullptr);
    return resource;
}

void D3D12RaytracingPartitionedTLAS::CreateWindowSizeDependentResources()
{
    D3D12_HEAP_PROPERTIES heap{};
    heap.Type = D3D12_HEAP_TYPE_DEFAULT;
    D3D12_RESOURCE_DESC description{};
    description.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    description.Width = m_width * m_renderScale;
    description.Height = m_height * m_renderScale;
    description.DepthOrArraySize = 1;
    description.MipLevels = 1;
    description.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    description.SampleDesc.Count = 1;
    description.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    ID3D12Resource* images[3]{};
    for (UINT index = 0; index < 3; ++index)
    {
        ThrowIfFailed(m_device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &description,
                                                        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr,
                                                        IID_PPV_ARGS(&m_traceImages[index])),
                      "CreateCommittedResource(trace image)");
        wchar_t label[32];
        swprintf_s(label, L"Trace image %u", index);
        SetDebugName(m_traceImages[index].Get(), label);
        images[index] = m_traceImages[index].Get();
    }
    description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    if (m_renderScale > 1)
        ThrowIfFailed(m_device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &description,
                                                        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr,
                                                        IID_PPV_ARGS(&m_renderOutput)),
                      "CreateCommittedResource(render output)");
    if (m_renderOutput)
        SetDebugName(m_renderOutput.Get(), L"Render output");
    description.Width = m_width;
    description.Height = m_height;
    ThrowIfFailed(m_device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &description,
                                                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr,
                                                    IID_PPV_ARGS(&m_output)),
                  "CreateCommittedResource(output)");
    SetDebugName(m_output.Get(), L"Presentation output");
    if (m_renderScale == 1)
        m_renderOutput = m_output;

    D3D12_DESCRIPTOR_HEAP_DESC heapDescription{};
    heapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDescription.NumDescriptors = 5;
    heapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&heapDescription, IID_PPV_ARGS(&m_descriptors)),
                  "CreateDescriptorHeap");
    const UINT stride = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_UNORDERED_ACCESS_VIEW_DESC view{};
    view.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    view.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    auto handle = m_descriptors->GetCPUDescriptorHandleForHeapStart();
    for (UINT index = 0; index < 3; ++index)
    {
        m_device->CreateUnorderedAccessView(images[index], nullptr, &view, handle);
        handle.ptr += stride;
    }
    view.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    m_device->CreateUnorderedAccessView(m_renderOutput.Get(), nullptr, &view, handle);
    handle.ptr += stride;
    m_device->CreateUnorderedAccessView(m_output.Get(), nullptr, &view, handle);
}

void D3D12RaytracingPartitionedTLAS::BuildGeometryAndBLAS()
{
    const auto dynamicMesh = scene_meshes::cube();
    m_vertices = Upload(dynamicMesh.positions);
    m_indices = Upload(dynamicMesh.indices);

    D3D12_RAYTRACING_GEOMETRY_DESC geometry{};
    geometry.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    geometry.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
    geometry.Triangles.VertexBuffer.StartAddress = m_vertices->GetGPUVirtualAddress();
    geometry.Triangles.VertexBuffer.StrideInBytes = sizeof(Vector3);
    geometry.Triangles.VertexCount = static_cast<UINT>(dynamicMesh.positions.size());
    geometry.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    geometry.Triangles.IndexBuffer = m_indices->GetGPUVirtualAddress();
    geometry.Triangles.IndexCount = static_cast<UINT>(dynamicMesh.indices.size());
    geometry.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    inputs.NumDescs = 1;
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.pGeometryDescs = &geometry;
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO sizes{};
    m_device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &sizes);
    if (!sizes.ResultDataMaxSizeInBytes || !sizes.ScratchDataSizeInBytes)
        throw std::runtime_error("BLAS prebuild returned zero size");
    m_blas =
        CreateBuffer(sizes.ResultDataMaxSizeInBytes, D3D12_HEAP_TYPE_DEFAULT,
                     D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE,
                     D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
    SetDebugName(m_blas.Get(), L"Domino BLAS");
    auto scratch = CreateBuffer(sizes.ScratchDataSizeInBytes, D3D12_HEAP_TYPE_DEFAULT,
                                D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build{};
    build.Inputs = inputs;
    build.DestAccelerationStructureData = m_blas->GetGPUVirtualAddress();
    build.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();
    m_commandList->BuildRaytracingAccelerationStructure(&build, 0, nullptr);
    auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(m_blas.Get());
    m_commandList->ResourceBarrier(1, &barrier);
    ExecuteAndWait();

    auto buildShape = [&](UINT kind, const scene_meshes::Mesh& mesh) {
        auto vertexBuffer = Upload(mesh.positions);
        auto indexBuffer = Upload(mesh.indices);
        D3D12_RAYTRACING_GEOMETRY_DESC shapeGeometry{};
        shapeGeometry.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
        shapeGeometry.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
        shapeGeometry.Triangles.VertexBuffer.StartAddress = vertexBuffer->GetGPUVirtualAddress();
        shapeGeometry.Triangles.VertexBuffer.StrideInBytes = sizeof(Vector3);
        shapeGeometry.Triangles.VertexCount = static_cast<UINT>(mesh.positions.size());
        shapeGeometry.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        shapeGeometry.Triangles.IndexBuffer = indexBuffer->GetGPUVirtualAddress();
        shapeGeometry.Triangles.IndexCount = static_cast<UINT>(mesh.indices.size());
        shapeGeometry.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS shapeInputs{};
        shapeInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        shapeInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
        shapeInputs.NumDescs = 1;
        shapeInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        shapeInputs.pGeometryDescs = &shapeGeometry;
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO shapeSizes{};
        m_device->GetRaytracingAccelerationStructurePrebuildInfo(&shapeInputs, &shapeSizes);
        if (!shapeSizes.ResultDataMaxSizeInBytes || !shapeSizes.ScratchDataSizeInBytes)
            throw std::runtime_error("Static BLAS prebuild returned zero size");
        m_staticBlas[kind] = CreateBuffer(shapeSizes.ResultDataMaxSizeInBytes, D3D12_HEAP_TYPE_DEFAULT,
                                          D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS |
                                              D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE,
                                          D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
        wchar_t label[32];
        swprintf_s(label, L"Static BLAS %u", kind);
        SetDebugName(m_staticBlas[kind].Get(), label);
        auto shapeScratch =
            CreateBuffer(shapeSizes.ScratchDataSizeInBytes, D3D12_HEAP_TYPE_DEFAULT,
                         D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC shapeBuild{};
        shapeBuild.Inputs = shapeInputs;
        shapeBuild.DestAccelerationStructureData = m_staticBlas[kind]->GetGPUVirtualAddress();
        shapeBuild.ScratchAccelerationStructureData = shapeScratch->GetGPUVirtualAddress();
        m_commandList->BuildRaytracingAccelerationStructure(&shapeBuild, 0, nullptr);
        auto shapeBarrier = CD3DX12_RESOURCE_BARRIER::UAV(m_staticBlas[kind].Get());
        m_commandList->ResourceBarrier(1, &shapeBarrier);
        ExecuteAndWait();
    };

    const float partitionSize = 2.0f * m_sceneSize / m_partitionCountPerAxis;
    float tileSize = 3.0f;
    const float count = std::floor(partitionSize / tileSize);
    tileSize += ((partitionSize / tileSize - count) * tileSize) / count;
    const auto planeMesh = scene_meshes::plane(partitionSize, partitionSize);
    const auto icoMesh = scene_meshes::icosahedron();
    const auto tileMesh = scene_meshes::cube(tileSize * 0.98f, 0.1f, tileSize * 0.98f);
    const auto tetraMesh = scene_meshes::tetrahedron();
    buildShape(0, planeMesh);
    buildShape(1, icoMesh);
    buildShape(2, tileMesh);
    buildShape(3, tetraMesh);

    // Store three float3s (36 bytes) per triangle: shading normal and two edges.
    // ClosestHit computes the geometric normal from the edges for its grouping key.
    // Mesh order, triangle counts, and starts below match raytracing.hlsl offsets:
    // domino (12, 0), plane (200, 12), icosahedron (20, 212), tile (12, 232),
    // tetrahedron (4, 244). Update both sides when changing topology/subdivisions.
    std::vector<Vector3> triangleData;
    auto appendNormals = [&](const scene_meshes::Mesh& mesh) {
        for (size_t triangle = 0; triangle < mesh.indices.size(); triangle += 3)
        {
            triangleData.push_back(mesh.normals[mesh.indices[triangle]]);
            const Vector3 a = mesh.positions[mesh.indices[triangle]];
            const Vector3 b = mesh.positions[mesh.indices[triangle + 1]];
            const Vector3 c = mesh.positions[mesh.indices[triangle + 2]];
            triangleData.push_back(b - a);
            triangleData.push_back(c - a);
        }
    };
    appendNormals(dynamicMesh);
    appendNormals(planeMesh);
    appendNormals(icoMesh);
    appendNormals(tileMesh);
    appendNormals(tetraMesh);
    m_triangleData = Upload(triangleData);
}

void D3D12RaytracingPartitionedTLAS::CreateSceneAndTopLevels()
{
    std::vector<std::pair<Matrix, UINT>> staticObjects;
    std::vector<Vector3> dominoPositions;
    if (m_fullScene)
    {
        dominoPositions = scene::generateDominoPositions(m_sceneSize);
        m_dominoCount = static_cast<UINT>(dominoPositions.size());
        staticObjects.reserve(1216683);
        m_staticCount = scene::generateStaticObjects(
            dominoPositions, m_sceneSize, m_partitionCountPerAxis, m_staticCount,
            [&](const Matrix& matrix, UINT kind) { staticObjects.emplace_back(matrix, kind); });
        m_totalInstanceCount = m_dominoCount + m_staticCount;
        if (m_dominoCount < 2)
            throw std::runtime_error("Hilbert scene did not generate dominoes");
    }
    std::vector<D3D12_RAYTRACING_INSTANCE_DESC> tlasInstances(m_totalInstanceCount);
    std::vector<D3D12_RTAS_PARTITIONED_TLAS_OPERATION_WRITE_INSTANCE_ARGS> ptlasInstances(m_totalInstanceCount);
    std::vector<UINT> partitionSizes(m_partitionCount);
    for (UINT index = 0; index < m_dominoCount; ++index)
    {
        const UINT x = m_fullScene ? 0 : index % m_dominoesPerAxis;
        const UINT z = m_fullScene ? 0 : index / m_dominoesPerAxis;
        const float worldX =
            m_fullScene ? dominoPositions[index].x : (static_cast<float>(x) - (m_dominoesPerAxis - 1) * 0.5f) * 1.3f;
        const float worldZ =
            m_fullScene ? dominoPositions[index].y : (static_cast<float>(z) - (m_dominoesPerAxis - 1) * 0.5f) * 1.3f;
        float transform[3][4]{};
        if (m_fullScene)
        {
            const Vector3 derivative = index == 0 ? dominoPositions[1] - dominoPositions[0]
                                                  : dominoPositions[index] - dominoPositions[index - 1];
            float heading = 0.0f;
            if (derivative.x != 0.0f || derivative.y != 0.0f)
            {
                const float derivativeScale = float(static_cast<UINT>(m_sceneSize * 4.0f) - 1) / float(m_dominoCount);
                const Vector2 direction =
                    scene::Normalize(Vector2(derivative.x / derivativeScale, derivative.y / derivativeScale));
                heading = std::acos(direction.y);
                if (direction.x < 0.0f)
                    heading = -heading;
                heading += DirectX::XM_PI * 0.5f;
            }
            // Expand a yaw quaternion into the instance transform.
            const float sy = std::sin(heading * 0.5f);
            const float cy = std::cos(heading * 0.5f);
            const Matrix matrix = scene::InstanceTransform(Vector3(worldX, 0.5f, worldZ), Vector3(0.1f, 1.0f, 0.5f),
                                                           2.0f * (cy * sy), 1.0f - 2.0f * (sy * sy));
            for (UINT row = 0; row < 3; ++row)
                for (UINT column = 0; column < 4; ++column)
                    transform[row][column] = matrix.m[column][row];
        }
        else
        {
            const float gridTransform[3][4] = {{-0.35f, 0, 0, worldX}, {0, 2.0f, 0, 1.0f}, {0, 0, -0.18f, worldZ}};
            std::memcpy(transform, gridTransform, sizeof(transform));
        }
        auto& regular = tlasInstances[index];
        std::memcpy(regular.Transform, transform, sizeof(transform));
        // Domino IDs directly index physics and partition buffers in ClosestHit.
        // They must fit below bit 23, which marks static objects.
        regular.InstanceID = index;
        regular.InstanceMask = 0xff;
        regular.AccelerationStructure = m_blas->GetGPUVirtualAddress();

        auto& partitioned = ptlasInstances[index];
        std::memcpy(partitioned.Transform, transform, sizeof(transform));
        partitioned.InstanceID = index;
        partitioned.InstanceMask = 0xff;
        partitioned.InstanceIndex = index;
        if (m_fullScene)
        {
            const UINT partitionX =
                std::clamp(static_cast<int>((worldX + m_sceneSize) * m_partitionCountPerAxis / (2.0f * m_sceneSize)), 0,
                           static_cast<int>(m_partitionCountPerAxis) - 1);
            const UINT partitionZ =
                std::clamp(static_cast<int>((worldZ + m_sceneSize) * m_partitionCountPerAxis / (2.0f * m_sceneSize)), 0,
                           static_cast<int>(m_partitionCountPerAxis) - 1);
            partitioned.PartitionIndex = partitionX + m_partitionCountPerAxis * partitionZ;
        }
        else
        {
            partitioned.PartitionIndex = x * m_partitionCountPerAxis / m_dominoesPerAxis +
                                         m_partitionCountPerAxis * (z * m_partitionCountPerAxis / m_dominoesPerAxis);
        }
        partitioned.AccelerationStructure = m_blas->GetGPUVirtualAddress();
        ++partitionSizes[partitioned.PartitionIndex];
    }

    if (m_staticCount)
    {
        UINT staticIndex = 0;
        auto addStatic = [&](const Matrix& matrix, UINT kind, UINT partition) {
            const UINT index = m_dominoCount + staticIndex;
            if (index >= m_totalInstanceCount || partition >= m_partitionCount)
                throw std::runtime_error("Static scene generator exceeded PTLAS capacity");
            float transform[3][4];
            for (UINT row = 0; row < 3; ++row)
                for (UINT column = 0; column < 4; ++column)
                    transform[row][column] = matrix.m[column][row];
            auto& regular = tlasInstances[index];
            std::memcpy(regular.Transform, transform, sizeof(transform));
            // ClosestHit decodes this 24-bit application ID: bit 23 marks static,
            // bits 22:21 select plane/icosahedron/tile/tetrahedron, bits 20:0 hold
            // staticIndex. Keep staticIndex below 2^21 so it cannot overlap kind.
            // The PTLAS InstanceIndex below remains the unencoded stable slot.
            regular.InstanceID = 0x800000u | (kind << 21) | staticIndex;
            regular.InstanceMask = 0xff;
            regular.AccelerationStructure = m_staticBlas[kind]->GetGPUVirtualAddress();
            auto& partitioned = ptlasInstances[index];
            std::memcpy(partitioned.Transform, transform, sizeof(transform));
            partitioned.InstanceID = regular.InstanceID;
            partitioned.InstanceMask = 0xff;
            partitioned.InstanceIndex = index;
            partitioned.PartitionIndex = partition;
            partitioned.AccelerationStructure = m_staticBlas[kind]->GetGPUVirtualAddress();
            ++partitionSizes[partition];
            ++staticIndex;
        };
        if (m_fullScene)
        {
            for (const auto& [matrix, kind] : staticObjects)
            {
                const float worldX = matrix._41;
                const float worldZ = matrix._43;
                const UINT px = std::clamp(
                    static_cast<int>((worldX + m_sceneSize) * m_partitionCountPerAxis / (2.0f * m_sceneSize)), 0,
                    static_cast<int>(m_partitionCountPerAxis) - 1);
                const UINT pz = std::clamp(
                    static_cast<int>((worldZ + m_sceneSize) * m_partitionCountPerAxis / (2.0f * m_sceneSize)), 0,
                    static_cast<int>(m_partitionCountPerAxis) - 1);
                addStatic(matrix, kind, px + m_partitionCountPerAxis * pz);
            }
        }
        else
        {
            const UINT staticAxis = static_cast<UINT>(std::ceil(std::sqrt(static_cast<double>(m_staticCount))));
            const float spacing = static_cast<float>(m_dominoesPerAxis) * 1.3f / staticAxis;
            for (UINT index = 0; index < m_staticCount; ++index)
            {
                const UINT x = index % staticAxis;
                const UINT z = index / staticAxis;
                const Vector3 position{(static_cast<float>(x) - (staticAxis - 1) * 0.5f) * spacing, -0.2f,
                                       (static_cast<float>(z) - (staticAxis - 1) * 0.5f) * spacing};
                const UINT partition = x * m_partitionCountPerAxis / staticAxis +
                                       m_partitionCountPerAxis * (z * m_partitionCountPerAxis / staticAxis);
                addStatic(scene::objectTransform(position, Vector3(spacing * 0.92f, 0.15f, spacing * 0.92f)), 2,
                          partition);
            }
        }
        if (staticIndex != m_staticCount)
            throw std::runtime_error("Static scene generator produced incorrect count");
    }
    if (m_fullScene)
    {
        char* environmentPath = nullptr;
        size_t environmentLength = 0;
        if (_dupenv_s(&environmentPath, &environmentLength, "PTLAS_SCENE_DUMP") != 0)
            throw std::runtime_error("Could not read PTLAS_SCENE_DUMP");
        const std::unique_ptr<char, decltype(&std::free)> dumpPath(environmentPath, &std::free);
        if (dumpPath)
        {
            std::ofstream dump(dumpPath.get(), std::ios::binary);
            const UINT counts[2] = {m_staticCount, m_dominoCount};
            dump.write(reinterpret_cast<const char*>(counts), sizeof(counts));
            auto writeRecord = [&](UINT mesh, const float(&transform)[3][4]) {
                dump.write(reinterpret_cast<const char*>(&mesh), sizeof(mesh));
                dump.write(reinterpret_cast<const char*>(transform), sizeof(transform));
            };
            for (UINT index = m_dominoCount; index < m_totalInstanceCount; ++index)
                writeRecord(1u + ((tlasInstances[index].InstanceID >> 21) & 3u), tlasInstances[index].Transform);
            for (UINT index = 0; index < m_dominoCount; ++index)
                writeRecord(0u, tlasInstances[index].Transform);
        }
    }
    m_maxInstancesPerPartition = *std::max_element(partitionSizes.begin(), partitionSizes.end());
    if (!m_stateDumpPath.empty())
        m_stateDumpReadback = CreateBuffer(UINT64(m_dominoCount) * sizeof(Motion), D3D12_HEAP_TYPE_READBACK,
                                           D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);
    m_sceneSizeInput = m_sceneSize;
    m_partitionInput = static_cast<int>(m_partitionCountPerAxis);
    m_seedInput = static_cast<int>(m_selfToppleCount);
    m_dominoAxisInput = static_cast<int>(m_dominoesPerAxis);
    m_previewDominoCount = m_dominoCount;

    UploadSeedVelocities(tlasInstances);
    CreateRegularTLAS(tlasInstances);
    CreatePartitionedTLAS(ptlasInstances);
    ExecuteAndWait();
    m_ptlasOperations.Reset(); // Initial build input is no longer needed.
}

void D3D12RaytracingPartitionedTLAS::UploadSeedVelocities(const std::vector<D3D12_RAYTRACING_INSTANCE_DESC>& instances)
{
    static_assert(sizeof(Vector3) == 12);
    const UINT interval = std::max(1u, m_dominoCount / std::max(1u, m_selfToppleCount));
    const UINT seedCount = m_fullScene ? std::min(m_selfToppleCount, (m_dominoCount + interval - 1) / interval) : 1u;
    std::vector<Vector3> velocities(std::max(1u, seedCount), Vector3(0.0f));
    auto position = [&](UINT index) {
        const auto& transform = instances[index].Transform;
        return Vector3(transform[0][3], transform[1][3], transform[2][3]);
    };
    for (UINT seed = 0; seed < seedCount && m_dominoCount >= 2; ++seed)
    {
        const UINT index = m_fullScene ? seed * interval : m_dominoCount - 1;
        const Vector3 orientation = index + 1 < m_dominoCount
                                        ? scene::Normalize(position(index + 1) - position(index))
                                        : -scene::Normalize(position(index) - position(index - 1));
        const Vector3 axis = scene::Normalize(scene::Cross(Vector3(0, 1, 0), orientation));
        velocities[seed] = -axis * 5.5f;
    }
    m_seedAngularVelocities = Upload(velocities);
}

void D3D12RaytracingPartitionedTLAS::CreateRegularTLAS(const std::vector<D3D12_RAYTRACING_INSTANCE_DESC>& tlasInstances)
{
    m_originalTlasUpload = Upload(tlasInstances);
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlasInputs{};
    tlasInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    tlasInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE |
                       D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
    tlasInputs.NumDescs = m_totalInstanceCount;
    tlasInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    tlasInputs.InstanceDescs = m_originalTlasUpload->GetGPUVirtualAddress();
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO tlasSizes{};
    m_device->GetRaytracingAccelerationStructurePrebuildInfo(&tlasInputs, &tlasSizes);
    if (!tlasSizes.ResultDataMaxSizeInBytes || !tlasSizes.ScratchDataSizeInBytes)
        throw std::runtime_error("TLAS prebuild returned zero size");
    m_tlas =
        CreateBuffer(tlasSizes.ResultDataMaxSizeInBytes, D3D12_HEAP_TYPE_DEFAULT,
                     D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE,
                     D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
    SetDebugName(m_tlas.Get(), L"Regular TLAS");
    m_tlasScratch = CreateBuffer(std::max(tlasSizes.ScratchDataSizeInBytes, tlasSizes.UpdateScratchDataSizeInBytes),
                                 D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
                                 D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    SetDebugName(m_tlasScratch.Get(), L"Regular TLAS scratch");
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build{};
    build.Inputs = tlasInputs;
    build.DestAccelerationStructureData = m_tlas->GetGPUVirtualAddress();
    build.ScratchAccelerationStructureData = m_tlasScratch->GetGPUVirtualAddress();
    m_commandList->BuildRaytracingAccelerationStructure(&build, 0, nullptr);
    auto tlasBarrier = CD3DX12_RESOURCE_BARRIER::UAV(m_tlas.Get());
    m_commandList->ResourceBarrier(1, &tlasBarrier);
}

void D3D12RaytracingPartitionedTLAS::CreatePartitionedTLAS(
    const std::vector<D3D12_RTAS_PARTITIONED_TLAS_OPERATION_WRITE_INSTANCE_ARGS>& ptlasInstances)
{
    m_ptlasInstances = Upload(ptlasInstances);
    // Capacity stays fixed across updates. A dynamic instance can be in its
    // original partition or the global partition, so both maxima cover it.
    m_ptlasInputs.Flags = m_ptlasHint;
    m_ptlasInputs.InstanceCount = m_totalInstanceCount;
    m_ptlasInputs.MaxInstancePerPartitionCount = m_maxInstancesPerPartition;
    m_ptlasInputs.PartitionCount = m_partitionCount;
    m_ptlasInputs.MaxInstanceInGlobalPartitionCount = m_dominoCount;
    D3D12_RTAS_OPERATION_INPUTS ptlasOperationInputs{};
    ptlasOperationInputs.Type = D3D12_RTAS_OPERATION_TYPE_PARTITIONED_TLAS;
    ptlasOperationInputs.pPartitionedTLASInputsDesc = &m_ptlasInputs;
    D3D12_RTAS_OPERATION_PREBUILD_INFO ptlasSizes{};
    m_dxr2Device->GetRTASOperationPrebuildInfo(&ptlasOperationInputs, &ptlasSizes);
    if (!ptlasSizes.ResultDataMaxSizeInBytes || !ptlasSizes.ScratchDataSizeInBytes)
        throw std::runtime_error("PTLAS prebuild returned zero size");
    m_ptlas =
        CreateBuffer(ptlasSizes.ResultDataMaxSizeInBytes, D3D12_HEAP_TYPE_DEFAULT,
                     D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE,
                     D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
    SetDebugName(m_ptlas.Get(), L"Partitioned TLAS");
    m_ptlasScratch = CreateBuffer(ptlasSizes.ScratchDataSizeInBytes, D3D12_HEAP_TYPE_DEFAULT,
                                  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    SetDebugName(m_ptlasScratch.Get(), L"Partitioned TLAS scratch");
    // The initial build writes every instance into the persistent PTLAS.
    D3D12_RTAS_PARTITIONED_TLAS_OPERATION operation{};
    operation.Type = D3D12_RTAS_PARTITIONED_TLAS_OPERATION_TYPE_WRITE_INSTANCE;
    operation.ArgCount = m_totalInstanceCount;
    operation.ArgData.StartAddress = m_ptlasInstances->GetGPUVirtualAddress();
    operation.ArgData.StrideInBytes = sizeof(D3D12_RTAS_PARTITIONED_TLAS_OPERATION_WRITE_INSTANCE_ARGS);
    m_ptlasOperations = Upload(std::vector{operation});
    // The outer count is one WRITE_INSTANCE operation. Its ArgCount is the
    // number of instance records, and is generated by the GPU after this build.
    m_ptlasOperationCount = Upload(std::vector<UINT>{1});
    D3D12_RTAS_PARTITIONED_TLAS_OPERATION_DATA ptlasData{};
    ptlasData.DestAccelerationStructureData = m_ptlas->GetGPUVirtualAddress();
    ptlasData.ScratchAccelerationStructureData = m_ptlasScratch->GetGPUVirtualAddress();
    ptlasData.IndirectPartitionedTlasOpCount = m_ptlasOperationCount->GetGPUVirtualAddress();
    ptlasData.IndirectPartitionedTlasOps = m_ptlasOperations->GetGPUVirtualAddress();
    D3D12_RTAS_OPERATION_DESC ptlasBuild{};
    ptlasBuild.Inputs = ptlasOperationInputs;
    ptlasBuild.pPartitionedTlasOperationData = &ptlasData;
    m_dxr2CommandList->ExecuteIndirectRTASOperations(1, &ptlasBuild, D3D12_EXECUTE_INDIRECT_RTAS_OPERATIONS_FLAG_NONE);
    auto ptlasBarrier = CD3DX12_RESOURCE_BARRIER::UAV(m_ptlas.Get());
    m_commandList->ResourceBarrier(1, &ptlasBarrier);
}

void D3D12RaytracingPartitionedTLAS::CreateAnimationPipelines(const ShaderBytecodes& shaders)
{
    CD3DX12_ROOT_PARAMETER parameters[AnimationRoot::Count]{};
    for (UINT index = 0; index < 4; ++index)
        parameters[index].InitAsShaderResourceView(index);
    for (UINT index = 0; index < 6; ++index)
        parameters[index + AnimationRoot::CurrentRegular].InitAsUnorderedAccessView(index);
    parameters[AnimationRoot::Constants].InitAsConstants(sizeof(AnimationConstants) / sizeof(UINT), 0);
    parameters[AnimationRoot::SeedVelocities].InitAsShaderResourceView(4);
    const CD3DX12_ROOT_SIGNATURE_DESC rootDescription(_countof(parameters), parameters);
    m_pipelines.animationRoot = SerializeAndCreateRootSignature(m_device.Get(), rootDescription);
    const auto& dxil = shaders.animation;
    D3D12_COMPUTE_PIPELINE_STATE_DESC pipeline{};
    pipeline.pRootSignature = m_pipelines.animationRoot.Get();
    pipeline.CS = {dxil.data(), dxil.size()};
    ThrowIfFailed(m_device->CreateComputePipelineState(&pipeline, IID_PPV_ARGS(&m_pipelines.animation)),
                  "CreateComputePipelineState(animation)");
    SetDebugName(m_pipelines.animation.Get(), L"Instance animation");
    const auto& physicsDxil = shaders.physics;
    pipeline.CS = {physicsDxil.data(), physicsDxil.size()};
    ThrowIfFailed(m_device->CreateComputePipelineState(&pipeline, IID_PPV_ARGS(&m_pipelines.physics)),
                  "CreateComputePipelineState(physics)");
    SetDebugName(m_pipelines.physics.Get(), L"Domino physics");
    const auto& markDxil = shaders.markPartitions;
    pipeline.CS = {markDxil.data(), markDxil.size()};
    ThrowIfFailed(m_device->CreateComputePipelineState(&pipeline, IID_PPV_ARGS(&m_pipelines.markPartitions)),
                  "CreateComputePipelineState(mark partitions)");
    SetDebugName(m_pipelines.markPartitions.Get(), L"Mark active partitions");
}

void D3D12RaytracingPartitionedTLAS::CreateAnimationResources()
{
    m_gpuTlasInstances =
        CreateBuffer(m_totalInstanceCount * sizeof(D3D12_RAYTRACING_INSTANCE_DESC), D3D12_HEAP_TYPE_DEFAULT,
                     D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    m_gpuOriginalRegular = CreateBuffer(m_dominoCount * sizeof(D3D12_RAYTRACING_INSTANCE_DESC), D3D12_HEAP_TYPE_DEFAULT,
                                        D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);
    m_gpuOriginalPartitioned =
        CreateBuffer(m_dominoCount * sizeof(D3D12_RTAS_PARTITIONED_TLAS_OPERATION_WRITE_INSTANCE_ARGS),
                     D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);
    m_gpuPtlasWrites = CreateBuffer(m_dominoCount * sizeof(D3D12_RTAS_PARTITIONED_TLAS_OPERATION_WRITE_INSTANCE_ARGS),
                                    D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
                                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    // Only ArgCount changes per frame; the GPU shader appends WRITE_INSTANCE
    // records and atomically increments it. A transform change needs WRITE,
    // while UPDATE_INSTANCE only changes the BLAS address or hit-group index.
    D3D12_RTAS_PARTITIONED_TLAS_OPERATION emptyWrites{};
    emptyWrites.Type = D3D12_RTAS_PARTITIONED_TLAS_OPERATION_TYPE_WRITE_INSTANCE;
    emptyWrites.ArgData.StartAddress = m_gpuPtlasWrites->GetGPUVirtualAddress();
    emptyWrites.ArgData.StrideInBytes = sizeof(D3D12_RTAS_PARTITIONED_TLAS_OPERATION_WRITE_INSTANCE_ARGS);
    m_ptlasZeroOperation = Upload(std::vector{emptyWrites});
    m_gpuPtlasOperation = CreateBuffer(sizeof(D3D12_RTAS_PARTITIONED_TLAS_OPERATION), D3D12_HEAP_TYPE_DEFAULT,
                                       D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);
    m_instancePartitions =
        CreateBuffer(m_dominoCount * sizeof(UINT), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
                     D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    for (auto& state : m_physicsState)
        state = CreateBuffer(m_dominoCount * sizeof(Motion), D3D12_HEAP_TYPE_DEFAULT,
                             D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    m_partitionActivity = CreateBuffer(m_partitionCount * sizeof(UINT), D3D12_HEAP_TYPE_DEFAULT,
                                       D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);
    SetDebugName(m_gpuTlasInstances.Get(), L"Regular TLAS instances");
    SetDebugName(m_gpuOriginalRegular.Get(), L"Original regular instances");
    SetDebugName(m_gpuOriginalPartitioned.Get(), L"Original partitioned instances");
    SetDebugName(m_gpuPtlasWrites.Get(), L"PTLAS instance writes");
    SetDebugName(m_gpuPtlasOperation.Get(), L"PTLAS write operation");
    SetDebugName(m_instancePartitions.Get(), L"Instance partition IDs");
    SetDebugName(m_physicsState[0].Get(), L"Physics state 0");
    SetDebugName(m_physicsState[1].Get(), L"Physics state 1");
    SetDebugName(m_partitionActivity.Get(), L"Partition activity");
    m_partitionActivityZero = Upload(std::vector<UINT>(m_partitionCount, 0));
    auto toCopy = CD3DX12_RESOURCE_BARRIER::Transition(m_gpuTlasInstances.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                                       D3D12_RESOURCE_STATE_COPY_DEST);
    m_commandList->ResourceBarrier(1, &toCopy);
    m_commandList->CopyBufferRegion(m_gpuTlasInstances.Get(), 0, m_originalTlasUpload.Get(), 0,
                                    m_totalInstanceCount * sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
    m_commandList->CopyBufferRegion(m_gpuOriginalRegular.Get(), 0, m_originalTlasUpload.Get(), 0,
                                    m_dominoCount * sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
    m_commandList->CopyBufferRegion(m_gpuOriginalPartitioned.Get(), 0, m_ptlasInstances.Get(), 0,
                                    m_dominoCount * sizeof(D3D12_RTAS_PARTITIONED_TLAS_OPERATION_WRITE_INSTANCE_ARGS));
    auto toUav = CD3DX12_RESOURCE_BARRIER::Transition(m_gpuTlasInstances.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
                                                      D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    m_commandList->ResourceBarrier(1, &toUav);
    D3D12_RESOURCE_BARRIER sourceBarriers[2] = {
        CD3DX12_RESOURCE_BARRIER::Transition(m_gpuOriginalRegular.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
                                             D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
        CD3DX12_RESOURCE_BARRIER::Transition(m_gpuOriginalPartitioned.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
                                             D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)};
    m_commandList->ResourceBarrier(2, sourceBarriers);
    m_commandList->CopyBufferRegion(m_partitionActivity.Get(), 0, m_partitionActivityZero.Get(), 0,
                                    m_partitionCount * sizeof(UINT));
    auto partitionHistoryBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_partitionActivity.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    m_commandList->ResourceBarrier(1, &partitionHistoryBarrier);
    m_partitionActivityState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    auto physicsInputBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_physicsState[0].Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    m_commandList->ResourceBarrier(1, &physicsInputBarrier);
    ExecuteAndWait();
    m_originalTlasUpload.Reset();
    m_ptlasInstances.Reset();
    m_partitionActivityZero.Reset();
}

void D3D12RaytracingPartitionedTLAS::CreateRaytracingPipelineStateObject(const ShaderBytecodes& shaders)
{
    CD3DX12_DESCRIPTOR_RANGE outputRange;
    outputRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 3, 0);
    CD3DX12_ROOT_PARAMETER parameters[RayRoot::Count]{};
    parameters[RayRoot::Scene].InitAsShaderResourceView(0);
    parameters[RayRoot::Images].InitAsDescriptorTable(1, &outputRange);
    parameters[RayRoot::Frame].InitAsConstantBufferView(0);
    for (UINT slot = RayRoot::InstancePartitions; slot <= RayRoot::TriangleData; ++slot)
        parameters[slot].InitAsShaderResourceView(slot - RayRoot::Frame);
    parameters[RayRoot::PickResult].InitAsUnorderedAccessView(3);
    const CD3DX12_ROOT_SIGNATURE_DESC rootDescription(_countof(parameters), parameters);
    m_pipelines.raytracingRoot = SerializeAndCreateRootSignature(m_device.Get(), rootDescription);

    const auto& dxil = shaders.raytracing;

    CD3DX12_STATE_OBJECT_DESC stateObject(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);
    auto* library = stateObject.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    const D3D12_SHADER_BYTECODE libraryCode{dxil.data(), dxil.size()};
    library->SetDXILLibrary(&libraryCode);
    library->DefineExport(L"RayGen");
    library->DefineExport(L"Miss");
    library->DefineExport(L"ClosestHit");
    auto* hitGroup = stateObject.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    hitGroup->SetHitGroupExport(L"HitGroup");
    hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
    hitGroup->SetClosestHitShaderImport(L"ClosestHit");
    auto* shaderConfig = stateObject.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    shaderConfig->Config(sizeof(RayPayload), sizeof(float) * 2);
    auto* pipelineConfig = stateObject.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
    pipelineConfig->Config(10); // Maximum depth of the reflection/refraction path.
    auto* root = stateObject.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
    root->SetRootSignature(m_pipelines.raytracingRoot.Get());
    ThrowIfFailed(m_device->CreateStateObject(stateObject, IID_PPV_ARGS(&m_pipelines.raytracing)), "CreateStateObject");
    SetDebugName(m_pipelines.raytracing.Get(), L"Raytracing pipeline");
    ComPtr<ID3D12StateObjectProperties> properties;
    ThrowIfFailed(m_pipelines.raytracing.As(&properties), "QueryInterface(ID3D12StateObjectProperties)");

    constexpr UINT64 tableStride = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
    m_pipelines.shaderTable = CreateBuffer(tableStride * 3, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_FLAG_NONE,
                                           D3D12_RESOURCE_STATE_GENERIC_READ);
    SetDebugName(m_pipelines.shaderTable.Get(), L"Shader table");
    BYTE* table = nullptr;
    ThrowIfFailed(m_pipelines.shaderTable->Map(0, nullptr, reinterpret_cast<void**>(&table)), "Map(shader table)");
    std::memset(table, 0, tableStride * 3);
    const wchar_t* names[3] = {L"RayGen", L"Miss", L"HitGroup"};
    for (UINT index = 0; index < 3; ++index)
    {
        const void* identifier = properties->GetShaderIdentifier(names[index]);
        if (!identifier)
            throw std::runtime_error("Missing DXR shader identifier");
        std::memcpy(table + tableStride * index, identifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
    }
    m_pipelines.shaderTable->Unmap(0, nullptr);
}

void D3D12RaytracingPartitionedTLAS::CreateCompositingPipeline(const ShaderBytecodes& shaders)
{
    CD3DX12_DESCRIPTOR_RANGE imageRange;
    imageRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 5, 0);
    CD3DX12_ROOT_PARAMETER parameters[CompositeRoot::Count]{};
    parameters[CompositeRoot::Images].InitAsDescriptorTable(1, &imageRange);
    parameters[CompositeRoot::Pass].InitAsConstants(1, 0);
    const CD3DX12_ROOT_SIGNATURE_DESC description(_countof(parameters), parameters);
    m_pipelines.compositingRoot = SerializeAndCreateRootSignature(m_device.Get(), description);

    const auto& dxil = shaders.compositing;
    D3D12_COMPUTE_PIPELINE_STATE_DESC pipeline{};
    pipeline.pRootSignature = m_pipelines.compositingRoot.Get();
    pipeline.CS = {dxil.data(), dxil.size()};
    ThrowIfFailed(m_device->CreateComputePipelineState(&pipeline, IID_PPV_ARGS(&m_pipelines.compositing)),
                  "CreateComputePipelineState(compositing)");
    SetDebugName(m_pipelines.compositing.Get(), L"Compositing");
}

void D3D12RaytracingPartitionedTLAS::CreateFrameConstants()
{
    for (FrameContext& frame : m_frames)
        frame.constants =
            CreateBuffer(AlignUp(sizeof(FrameConstants), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT),
                         D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);
    m_pickResult = CreateBuffer(sizeof(UINT), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
                                D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    for (FrameContext& frame : m_frames)
        frame.pickedInstance = CreateBuffer(sizeof(UINT), D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_FLAG_NONE,
                                            D3D12_RESOURCE_STATE_COPY_DEST);
    if (!m_cameraInitialized)
    {
        m_cameraEye = m_fullScene ? Vector3(-20.0f, 10.0f, 15.0f) : Vector3(0.0f, 16.0f, -28.0f);
        const Vector3 target = m_fullScene ? Vector3(-15.0f, 7.0f, 7.0f) : Vector3(0, 0, 0);
        const Vector3 forward = scene::Normalize(target - m_cameraEye);
        m_cameraYaw = std::atan2(forward.x, forward.z);
        m_cameraPitch = std::asin(forward.y);
        m_cameraInitialized = true;
    }
    UpdateCamera();
}

void D3D12RaytracingPartitionedTLAS::UpdateCamera()
{
    const auto now = std::chrono::steady_clock::now();
    float elapsed = 0.0f;
    if (m_lastCameraTime.time_since_epoch().count() != 0)
        elapsed = std::min(std::chrono::duration<float>(now - m_lastCameraTime).count(), 0.05f);
    if (elapsed > 0.0f)
        m_fps = m_fps * 0.9f + 0.1f / elapsed;
    m_lastCameraTime = now;
    const Vector3 forward{std::sin(m_cameraYaw) * std::cos(m_cameraPitch), std::sin(m_cameraPitch),
                          std::cos(m_cameraYaw) * std::cos(m_cameraPitch)};
    const Vector3 right = scene::Normalize(scene::Cross(forward, Vector3(0, 1, 0)));
    if (GetForegroundWindow() == m_window && !m_ui.IsEditing())
    {
        const auto keys = m_keyboard.GetState();
        const float speed = (m_fullScene ? 20.0f : 10.0f) * ((keys.LeftShift || keys.RightShift) ? 5.0f : 1.0f);
        const float distance = speed * elapsed;
        if (keys.W)
            m_cameraEye += forward * distance;
        if (keys.S)
            m_cameraEye -= forward * distance;
        if (keys.D)
            m_cameraEye += right * distance;
        if (keys.A)
            m_cameraEye -= right * distance;
        if (keys.E)
            m_cameraEye.y += distance;
        if (keys.Q)
            m_cameraEye.y -= distance;
    }
    const Matrix view = Matrix::CreateLookAt(m_cameraEye, m_cameraEye + forward, Vector3::UnitY);
    const Matrix projection = Matrix::CreatePerspectiveFieldOfView(
        DirectX::XMConvertToRadians(m_cameraFov), static_cast<float>(m_width) / m_height, 0.1f, 10000.0f);
    m_viewProjection = view * projection;
    m_pickThisFrame = m_pickRequested && m_pickX >= 0 && m_pickY >= 0 && m_pickX < static_cast<int>(m_width) &&
                      m_pickY < static_cast<int>(m_height);
    // DirectXMath stores row-vector matrices. HLSL's column-major declaration
    // reads these bytes as the transposed inverse for mul(matrix, clipPosition).
    FrameConstants constants{m_viewProjection.Invert(),
                             Vector4(m_cameraEye.x, m_cameraEye.y, m_cameraEye.z, 1.0f),
                             m_partitionCountPerAxis,
                             static_cast<UINT>(m_simulationFrame),
                             m_showPartitionColors && m_usePtlas ? 1u : 0u,
                             m_fullScene ? m_sceneSize : m_dominoesPerAxis * 1.3f * 0.5f,
                             Vector4(std::cos(m_sunElevation) * std::sin(m_sunAzimuth), std::sin(m_sunElevation),
                                     std::cos(m_sunElevation) * std::cos(m_sunAzimuth), 0.0f),
                             Vector4(m_metallic, m_roughness, m_intensity, static_cast<float>(m_maxRayDepth)),
                             UINT(m_pickX * int(m_renderScale) + int(m_renderScale / 2)),
                             UINT(m_pickY * int(m_renderScale) + int(m_renderScale / 2)),
                             m_pickThisFrame ? 1u : 0u,
                             m_staticCount};
    void* destination = nullptr;
    auto& constantsBuffer = m_frames[m_frameSlot].constants;
    ThrowIfFailed(constantsBuffer->Map(0, nullptr, &destination), "Map(frame constants)");
    std::memcpy(destination, &constants, sizeof(constants));
    constantsBuffer->Unmap(0, nullptr);
}

void D3D12RaytracingPartitionedTLAS::UpdateInput()
{
    const auto keys = m_keyboard.GetState();
    m_keyboardButtons.Update(keys);
    m_ui.BeginFrame(m_keyboardButtons, keys.LeftShift || keys.RightShift);
    if (m_keyboardButtons.IsKeyPressed(DirectX::Keyboard::F12))
    {
        if (m_screenshotPath.empty())
            m_screenshotPath = L"screenshot.dds";
        m_screenshotRequested = true;
    }
    if (m_ui.IsEditing())
        return;
    using K = DirectX::Keyboard;
    if (m_keyboardButtons.IsKeyPressed(K::P) || m_keyboardButtons.IsKeyPressed(K::Space))
        m_usePtlas = !m_usePtlas;
    if (m_keyboardButtons.IsKeyPressed(K::D1))
        m_updateMode = 0;
    if (m_keyboardButtons.IsKeyPressed(K::D2))
        m_updateMode = 1;
    if (m_keyboardButtons.IsKeyPressed(K::D3))
        m_updateMode = 2;
    if (m_keyboardButtons.IsKeyPressed(K::F))
        m_paused = !m_paused;
    if (m_keyboardButtons.IsKeyPressed(K::N))
        m_step = true;
    if (m_keyboardButtons.IsKeyPressed(K::R))
        m_reset = true;
    if (m_keyboardButtons.IsKeyPressed(K::T))
        m_tlasRefit = !m_tlasRefit;
    if (m_keyboardButtons.IsKeyPressed(K::V))
        m_vsync = !m_vsync;
    if (m_keyboardButtons.IsKeyPressed(K::C))
        m_showPartitionColors = !m_showPartitionColors;
    if (m_keyboardButtons.IsKeyPressed(K::F5))
        m_reloadShadersRequested = true;
}

void D3D12RaytracingPartitionedTLAS::RegenerateScene()
{
    m_regenerateSceneRequested = false;
    m_deviceResources.WaitForGpu();
    for (UINT offset = 0; offset < kFrameCount; ++offset)
    {
        auto& frame = m_frames[(m_frameSlot + offset) % kFrameCount];
        if (frame.pending)
            CollectFrame(frame);
    }
    m_sceneSize = m_sceneSizeInput;
    m_partitionCountPerAxis = UINT(m_partitionInput);
    m_partitionCount = m_partitionCountPerAxis * m_partitionCountPerAxis;
    m_dominoesPerAxis = UINT(m_dominoAxisInput);
    m_selfToppleCount = UINT(m_seedInput);
    if (m_fullScene)
        m_staticCount = m_staticCountLimit;
    else
    {
        m_dominoCount = m_dominoesPerAxis * m_dominoesPerAxis;
        m_totalInstanceCount = m_dominoCount + m_staticCount;
    }
    // Reuse the device-loss resource lifecycle, with a completed GPU fence and
    // the same device. Rebuilds run between frames, never inside a UI draw.
    OnDeviceLost();
    m_deviceResources.BeginInitialization();
    OnDeviceRestored();
}

void D3D12RaytracingPartitionedTLAS::DrawInterface()
{
    wchar_t text[256];
    m_ui.Text(L"D3D12: Partitioned Top-Level Acceleration Structures");
    swprintf_s(text, L"%s | mode %u | %u dominoes, %u static | %u x %u partitions", m_usePtlas ? L"PTLAS" : L"TLAS",
               m_updateMode + 1, m_dominoCount, m_staticCount, m_partitionCountPerAxis, m_partitionCountPerAxis);
    m_ui.Text(text);
    swprintf_s(text, L"GPU: animation %.3f | AS %.3f | trace %.3f | composite %.3f | total %.3f ms", m_gpuAnimationMs,
               m_gpuAsMs, m_gpuTraceMs, m_gpuCompositeMs, m_gpuTotalMs);
    m_ui.Text(text);
    swprintf_s(text, L"%u instance writes | %.0f FPS | %s | F1 help | F2 settings", m_lastChangedCount, m_fps,
               m_paused ? L"paused" : L"running");
    m_ui.Text(text);
    if (m_ui.ShowHelp())
    {
        m_ui.Text(L"P/Space: PTLAS/TLAS | 1-3: update mode | C: partition colors | V: VSync");
        m_ui.Text(L"F: pause | N: single step | R: reset | T: TLAS refit | F5: reload shaders");
        m_ui.Text(L"WASD/QE: move | Shift: faster | left drag: look | right click: topple");
        m_ui.Text(L"Alt+Enter: fullscreen | Esc: quit | F2: edit settings");
    }
    if (!m_shaderReloadError.empty())
        m_ui.Text(L"Shader reload failed. Old pipelines active; see dxr2_shader_reload.log.");
    if (!m_ui.IsEditing())
        return;
    m_ui.Text(L"Tab: next page | Up/Down: select | Left/Right: change | Shift: x10 | Enter: action");
    switch (m_ui.GetPage())
    {
    case 0: {
        m_ui.Text(L"PTLAS and animation (1/4)");
        m_ui.Toggle(L"PTLAS active", m_usePtlas);
        int mode = int(m_updateMode) + 1;
        m_ui.Integer(L"Mode (1 local / 2 global / 3 near-far)", mode, 1, 3);
        m_updateMode = UINT(mode - 1);
        m_ui.Toggle(L"Mark all dynamic in active partitions", m_markAllInPartition);
        m_ui.Float(L"Near/far distance", m_modeDistance, 1, m_fullScene ? m_sceneSize : 100, 1);
        m_ui.Toggle(L"Partition colors", m_showPartitionColors);
        m_ui.Toggle(L"Pause", m_paused);
        m_ui.Toggle(L"VSync", m_vsync);
        m_ui.Toggle(L"Refit regular TLAS", m_tlasRefit);
        if (m_ui.Button(L"Single step"))
            m_step = true;
        if (m_ui.Button(L"Reset animation"))
            m_reset = true;
        m_ui.Integer(L"Topple domino index", m_toppleInput, 0, int(m_dominoCount) - 1);
        if (m_ui.Button(L"Topple selected domino"))
        {
            m_toppleIndex = UINT(m_toppleInput);
            m_manualTopple = true;
            m_hasManualTopple = true;
            m_paused = false; // Match right-click toppling: consume the request in a physics step.
        }
        if (m_hasManualTopple)
            m_ui.Text(L"Last toppled: " + std::to_wstring(m_toppleIndex));
        const double mb = 1024.0 * 1024.0;
        swprintf_s(text, L"TLAS %.2f MB / scratch %.2f | PTLAS %.2f MB / scratch %.2f", m_tlas->GetDesc().Width / mb,
                   m_tlasScratch->GetDesc().Width / mb, m_ptlas->GetDesc().Width / mb,
                   m_ptlasScratch->GetDesc().Width / mb);
        m_ui.Text(text);
        break;
    }
    case 1: {
        m_ui.Text(L"Scene (2/4) - apply to restart the scene");
        const float previousSize = m_sceneSizeInput;
        const int previousAxis = m_dominoAxisInput;
        m_ui.Integer(L"Partitions per axis", m_partitionInput, 1,
                     m_fullScene ? int(m_sceneSizeInput / 1.5f) : m_dominoAxisInput);
        if (m_fullScene)
            m_ui.Float(L"Scene size", m_sceneSizeInput, 1.5f * m_partitionInput, 1000, 1);
        else
            m_ui.Integer(L"Dominoes per axis", m_dominoAxisInput, m_partitionInput, 1000);
        m_ui.Integer(L"Self-toppling dominoes", m_seedInput, 0, 1000000);
        if (previousSize != m_sceneSizeInput || previousAxis != m_dominoAxisInput)
            m_previewDominoCount = m_fullScene ? UINT(scene::generateDominoPositions(m_sceneSizeInput).size())
                                               : UINT(m_dominoAxisInput * m_dominoAxisInput);
        m_ui.Text(L"New domino count: " + std::to_wstring(m_previewDominoCount));
        if (m_ui.Button(L"Regenerate scene"))
            m_regenerateSceneRequested = true;
        if (m_ui.Button(L"Reload shaders"))
            m_reloadShadersRequested = true;
        m_ui.Text(L"PTLAS allocation hint: " +
                  std::wstring(m_ptlasHint == D3D12_RTAS_PARTITIONED_TLAS_FLAG_FAST_BUILD   ? L"fast build"
                               : m_ptlasHint == D3D12_RTAS_PARTITIONED_TLAS_FLAG_FAST_TRACE ? L"fast trace"
                                                                                            : L"none"));
        break;
    }
    case 2:
        m_ui.Text(L"Camera (3/4)");
        m_ui.Float(L"Position X", m_cameraEye.x, -10000, 10000, 1);
        m_ui.Float(L"Position Y", m_cameraEye.y, -10000, 10000, 1);
        m_ui.Float(L"Position Z", m_cameraEye.z, -10000, 10000, 1);
        m_ui.Float(L"Yaw (radians)", m_cameraYaw, -DirectX::XM_PI, DirectX::XM_PI, 0.02f);
        m_ui.Float(L"Pitch (radians)", m_cameraPitch, -1.55f, 1.55f, 0.02f);
        m_ui.Float(L"Field of view (degrees)", m_cameraFov, 20, 100, 1);
        break;
    case 3:
        m_ui.Text(L"Lighting (4/4)");
        m_ui.Float(L"Sun azimuth (radians)", m_sunAzimuth, -DirectX::XM_PI, DirectX::XM_PI, 0.02f);
        m_ui.Float(L"Sun elevation (radians)", m_sunElevation, 0.02f, 1.55f, 0.02f);
        m_ui.Float(L"Metallic", m_metallic, 0, 1, 0.05f);
        m_ui.Float(L"Roughness", m_roughness, 0.05f, 1, 0.05f);
        m_ui.Float(L"Intensity", m_intensity, 0, 20, 0.25f);
        m_ui.Integer(L"Ray depth", m_maxRayDepth, 2, 10);
        break;
    }
}

void D3D12RaytracingPartitionedTLAS::CreateTimingQueries()
{
    D3D12_QUERY_HEAP_DESC description{};
    description.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
    description.Count = 6 * kFrameCount;
    ThrowIfFailed(m_device->CreateQueryHeap(&description, IID_PPV_ARGS(&m_timingQueries)), "CreateQueryHeap");
    for (FrameContext& frame : m_frames)
    {
        frame.timings = CreateBuffer(6 * sizeof(UINT64), D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_FLAG_NONE,
                                     D3D12_RESOURCE_STATE_COPY_DEST);
        frame.operationCount = CreateBuffer(sizeof(D3D12_RTAS_PARTITIONED_TLAS_OPERATION), D3D12_HEAP_TYPE_READBACK,
                                            D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);
    }
    ThrowIfFailed(m_deviceResources.GetCommandQueue()->GetTimestampFrequency(&m_timestampFrequency),
                  "GetTimestampFrequency");
}

void D3D12RaytracingPartitionedTLAS::InspectDebugMessages()
{
    std::ofstream log("dxr2_debug.log");
    const UINT errorCount = WriteDebugMessages(m_deviceResources.GetInfoQueue(), log);
    if (errorCount)
        throw std::runtime_error("D3D12 debug layer reported errors; see dxr2_debug.log");
}

void D3D12RaytracingPartitionedTLAS::UpdateAnimation()
{
    m_advancePhysics = !m_paused || m_step;
    m_step = false;
    if (m_reset)
    {
        m_reset = false;
        m_manualTopple = false;
        m_hasManualTopple = false;
        m_physicsResetPending = true;
        m_ptlasNeedsSync = true;
        m_tlasNeedsSync = true;
    }
}

void D3D12RaytracingPartitionedTLAS::GenerateInstanceUpdates()
{
    // Simulate writes the next physics buffer, MarkPartitions reads that result
    // to update spatial activity, and Animate emits descriptors plus membership.
    // The t2 binding is changed from previous to current physics between passes.
    if (m_instancePartitionState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
    {
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_instancePartitions.Get(), m_instancePartitionState,
                                                            D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        m_commandList->ResourceBarrier(1, &barrier);
        m_instancePartitionState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }
    if (m_usePtlas)
    {
        if (m_gpuOperationState != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_gpuPtlasOperation.Get(), m_gpuOperationState,
                                                                D3D12_RESOURCE_STATE_COPY_DEST);
            m_commandList->ResourceBarrier(1, &barrier);
        }
        m_commandList->CopyBufferRegion(m_gpuPtlasOperation.Get(), 0, m_ptlasZeroOperation.Get(), 0,
                                        sizeof(D3D12_RTAS_PARTITIONED_TLAS_OPERATION));
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_gpuPtlasOperation.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
                                                            D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        m_commandList->ResourceBarrier(1, &barrier);
        m_gpuOperationState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        if (m_gpuPtlasWritesState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
        {
            barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_gpuPtlasWrites.Get(), m_gpuPtlasWritesState,
                                                           D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            m_commandList->ResourceBarrier(1, &barrier);
            m_gpuPtlasWritesState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        }
        m_tlasNeedsSync = true;
    }
    else
    {
        if (m_gpuTlasInstancesState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
        {
            auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_gpuTlasInstances.Get(), m_gpuTlasInstancesState,
                                                                D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            m_commandList->ResourceBarrier(1, &barrier);
            m_gpuTlasInstancesState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        }
        m_ptlasNeedsSync = true;
    }

    if (m_partitionActivityState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
    {
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_partitionActivity.Get(), m_partitionActivityState,
                                                            D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        m_commandList->ResourceBarrier(1, &barrier);
    }
    m_partitionActivityState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

    // Five substeps at 0.008 seconds each are taken per running/single-step frame.
    // Reset initializes both physics buffers without advancing them this frame.
    AnimationConstants constants{m_updateMode,
                                 m_usePtlas ? 1u : 0u,
                                 m_usePtlas && m_ptlasNeedsSync ? 1u : 0u,
                                 m_dominoCount,
                                 m_fullScene ? 1u : 0u,
                                 m_partitionCountPerAxis,
                                 m_cameraEye.x,
                                 m_cameraEye.z,
                                 m_modeDistance,
                                 m_fullScene ? m_sceneSize : m_dominoesPerAxis * 1.3f * 0.5f,
                                 m_manualTopple ? 1u : 0u,
                                 m_toppleIndex,
                                 m_cameraEye.y,
                                 static_cast<UINT>(m_simulationFrame),
                                 m_physicsResetPending ? 1u : 0u,
                                 m_advancePhysics && !m_physicsResetPending ? 1u : 0u,
                                 m_selfToppleCount,
                                 m_markAllInPartition ? 1u : 0u,
                                 0.008f};
    const UINT nextPhysicsIndex = 1u - m_physicsCurrentIndex;
    m_commandList->SetComputeRootSignature(m_pipelines.animationRoot.Get());
    m_commandList->SetPipelineState(m_pipelines.physics.Get());
    m_commandList->SetComputeRootShaderResourceView(AnimationRoot::OriginalRegular,
                                                    m_gpuOriginalRegular->GetGPUVirtualAddress());
    m_commandList->SetComputeRootShaderResourceView(AnimationRoot::OriginalPartitioned,
                                                    m_gpuOriginalPartitioned->GetGPUVirtualAddress());
    m_commandList->SetComputeRootShaderResourceView(AnimationRoot::PhysicsPrevious,
                                                    m_physicsState[m_physicsCurrentIndex]->GetGPUVirtualAddress());
    m_commandList->SetComputeRootShaderResourceView(AnimationRoot::SeedVelocities,
                                                    m_seedAngularVelocities->GetGPUVirtualAddress());
    m_commandList->SetComputeRootUnorderedAccessView(AnimationRoot::PhysicsCurrent,
                                                     m_physicsState[nextPhysicsIndex]->GetGPUVirtualAddress());
    m_commandList->SetComputeRoot32BitConstants(AnimationRoot::Constants, sizeof(constants) / sizeof(UINT), &constants,
                                                0);
    PIXBeginEvent(m_commandList.Get(), PIX_COLOR_INDEX(5), "Physics");
    m_commandList->Dispatch((m_dominoCount + 255) / 256, 1, 1);
    PIXEndEvent(m_commandList.Get());
    if (m_physicsResetPending)
    {
        D3D12_RESOURCE_BARRIER copies[2] = {
            CD3DX12_RESOURCE_BARRIER::Transition(m_physicsState[nextPhysicsIndex].Get(),
                                                 D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                                 D3D12_RESOURCE_STATE_COPY_SOURCE),
            CD3DX12_RESOURCE_BARRIER::Transition(m_physicsState[m_physicsCurrentIndex].Get(),
                                                 D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                                                 D3D12_RESOURCE_STATE_COPY_DEST)};
        m_commandList->ResourceBarrier(2, copies);
        m_commandList->CopyBufferRegion(m_physicsState[m_physicsCurrentIndex].Get(), 0,
                                        m_physicsState[nextPhysicsIndex].Get(), 0,
                                        UINT64(m_dominoCount) * sizeof(Motion));
        D3D12_RESOURCE_BARRIER ready[2] = {
            CD3DX12_RESOURCE_BARRIER::Transition(m_physicsState[nextPhysicsIndex].Get(),
                                                 D3D12_RESOURCE_STATE_COPY_SOURCE,
                                                 D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
            CD3DX12_RESOURCE_BARRIER::Transition(m_physicsState[m_physicsCurrentIndex].Get(),
                                                 D3D12_RESOURCE_STATE_COPY_DEST,
                                                 D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)};
        m_commandList->ResourceBarrier(2, ready);
    }
    else
    {
        auto physicsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_physicsState[nextPhysicsIndex].Get(),
                                                                   D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                                                   D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        m_commandList->ResourceBarrier(1, &physicsBarrier);
    }
    m_commandList->SetPipelineState(m_pipelines.markPartitions.Get());
    m_commandList->SetComputeRootShaderResourceView(AnimationRoot::OriginalPartitioned,
                                                    m_gpuOriginalPartitioned->GetGPUVirtualAddress());
    m_commandList->SetComputeRootShaderResourceView(AnimationRoot::PhysicsPrevious,
                                                    m_physicsState[nextPhysicsIndex]->GetGPUVirtualAddress());
    m_commandList->SetComputeRootUnorderedAccessView(AnimationRoot::PartitionActivityOutput,
                                                     m_partitionActivity->GetGPUVirtualAddress());
    PIXBeginEvent(m_commandList.Get(), PIX_COLOR_INDEX(6), "Mark partitions");
    m_commandList->Dispatch((m_dominoCount + 255) / 256, 1, 1);
    PIXEndEvent(m_commandList.Get());
    auto activityBarrier =
        CD3DX12_RESOURCE_BARRIER::Transition(m_partitionActivity.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                             D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    m_commandList->ResourceBarrier(1, &activityBarrier);
    m_partitionActivityState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    m_commandList->SetPipelineState(m_pipelines.animation.Get());
    m_commandList->SetComputeRootShaderResourceView(AnimationRoot::PartitionActivity,
                                                    m_partitionActivity->GetGPUVirtualAddress());
    m_commandList->SetComputeRootUnorderedAccessView(AnimationRoot::CurrentRegular,
                                                     m_gpuTlasInstances->GetGPUVirtualAddress());
    m_commandList->SetComputeRootUnorderedAccessView(AnimationRoot::DynamicWrites,
                                                     m_gpuPtlasWrites->GetGPUVirtualAddress());
    m_commandList->SetComputeRootUnorderedAccessView(AnimationRoot::Operation,
                                                     m_gpuPtlasOperation->GetGPUVirtualAddress());
    m_commandList->SetComputeRootUnorderedAccessView(AnimationRoot::InstancePartitions,
                                                     m_instancePartitions->GetGPUVirtualAddress());
    PIXBeginEvent(m_commandList.Get(), PIX_COLOR_INDEX(7), "Generate instance writes");
    m_commandList->Dispatch((m_dominoCount + 255) / 256, 1, 1);
    PIXEndEvent(m_commandList.Get());
    auto physicsInputBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_physicsState[m_physicsCurrentIndex].Get(),
                                                                    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                                                                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    m_commandList->ResourceBarrier(1, &physicsInputBarrier);
    m_physicsCurrentIndex = nextPhysicsIndex;
    m_physicsResetPending = false;
    m_manualTopple = false;
    if (m_usePtlas)
    {
        // ExecuteIndirectRTASOperations reads these buffers as shader resources,
        // not as D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT. The transitions also
        // make the preceding UAV writes visible to the AS update.
        D3D12_RESOURCE_BARRIER barriers[3] = {
            CD3DX12_RESOURCE_BARRIER::UAV(m_instancePartitions.Get()),
            CD3DX12_RESOURCE_BARRIER::Transition(m_gpuPtlasWrites.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                                 D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
            CD3DX12_RESOURCE_BARRIER::Transition(m_gpuPtlasOperation.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                                 D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)};
        m_commandList->ResourceBarrier(3, barriers);
        m_gpuPtlasWritesState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        m_gpuOperationState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        m_ptlasNeedsSync = false;
    }
    else
    {
        auto barrier =
            CD3DX12_RESOURCE_BARRIER::Transition(m_gpuTlasInstances.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                                 D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        m_commandList->ResourceBarrier(1, &barrier);
        m_gpuTlasInstancesState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    }
}

void D3D12RaytracingPartitionedTLAS::UpdateAccelerationStructure()
{
    if (m_usePtlas)
    {
        D3D12_RTAS_OPERATION_INPUTS operationInputs{};
        operationInputs.Type = D3D12_RTAS_OPERATION_TYPE_PARTITIONED_TLAS;
        operationInputs.pPartitionedTLASInputsDesc = &m_ptlasInputs;
        D3D12_RTAS_PARTITIONED_TLAS_OPERATION_DATA data{};
        // Source and destination are the same allocation: unchanged partitions
        // are retained, and only partitions touched by the sparse writes rebuild.
        data.SourceAccelerationStructureData = m_ptlas->GetGPUVirtualAddress();
        data.DestAccelerationStructureData = m_ptlas->GetGPUVirtualAddress();
        data.ScratchAccelerationStructureData = m_ptlasScratch->GetGPUVirtualAddress();
        data.IndirectPartitionedTlasOpCount = m_ptlasOperationCount->GetGPUVirtualAddress();
        data.IndirectPartitionedTlasOps = m_gpuPtlasOperation->GetGPUVirtualAddress();
        D3D12_RTAS_OPERATION_DESC build{};
        build.Inputs = operationInputs;
        build.pPartitionedTlasOperationData = &data;
        m_dxr2CommandList->ExecuteIndirectRTASOperations(1, &build, D3D12_EXECUTE_INDIRECT_RTAS_OPERATIONS_FLAG_NONE);
        auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(m_ptlas.Get());
        m_commandList->ResourceBarrier(1, &barrier);
    }
    else
    {
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build{};
        build.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
        build.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE |
                             D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
        if (m_tlasRefit && !m_tlasNeedsSync)
        {
            build.Inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
            build.SourceAccelerationStructureData = m_tlas->GetGPUVirtualAddress();
        }
        build.Inputs.NumDescs = m_totalInstanceCount;
        build.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        build.Inputs.InstanceDescs = m_gpuTlasInstances->GetGPUVirtualAddress();
        build.DestAccelerationStructureData = m_tlas->GetGPUVirtualAddress();
        build.ScratchAccelerationStructureData = m_tlasScratch->GetGPUVirtualAddress();
        m_commandList->BuildRaytracingAccelerationStructure(&build, 0, nullptr);
        auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(m_tlas.Get());
        m_commandList->ResourceBarrier(1, &barrier);
        m_tlasNeedsSync = false;
    }
}

void D3D12RaytracingPartitionedTLAS::ReloadShaders()
{
    // Compilation and pipeline validation can fail independently. Preserve all
    // live roots, PSOs, and the shader table until the complete set is ready.
    const PipelineResources previous = m_pipelines;
    try
    {
        auto shaders = CompileSampleShaders(GetExecutableDirectory());
        CreateRaytracingPipelineStateObject(shaders);
        CreateCompositingPipeline(shaders);
        CreateAnimationPipelines(shaders);
        m_shaderBytecodes = std::move(shaders);
    }
    catch (...)
    {
        m_pipelines = previous;
        throw;
    }
}

void D3D12RaytracingPartitionedTLAS::CollectFrame(FrameContext& frame)
{
    const UINT64* stamps = nullptr;
    D3D12_RANGE timingRange{0, 6 * sizeof(UINT64)};
    ThrowIfFailed(frame.timings->Map(0, &timingRange, reinterpret_cast<void**>(const_cast<UINT64**>(&stamps))),
                  "Map(timestamp readback)");
    // RenderFrame defines these timestamp boundaries. The AS interval includes
    // the PTLAS statistics copy/transition; the total also includes gaps between
    // named stages. Readback mapping here happens after this frame's fence.
    m_gpuAnimationMs = 1000.0 * double(stamps[1] - stamps[0]) / double(m_timestampFrequency);
    m_gpuAsMs = 1000.0 * double(stamps[2] - stamps[1]) / double(m_timestampFrequency);
    m_gpuUpdateMs = 1000.0 * double(stamps[2] - stamps[0]) / double(m_timestampFrequency);
    m_gpuTraceMs = 1000.0 * double(stamps[4] - stamps[3]) / double(m_timestampFrequency);
    m_gpuCompositeMs = 1000.0 * double(stamps[5] - stamps[4]) / double(m_timestampFrequency);
    m_gpuTotalMs = 1000.0 * double(stamps[5] - stamps[0]) / double(m_timestampFrequency);
    D3D12_RANGE writtenRange{0, 0};
    frame.timings->Unmap(0, &writtenRange);

    m_lastChangedCount = 0;
    if (frame.ptlas)
    {
        const BYTE* operationBytes = nullptr;
        D3D12_RANGE operationRange{0, sizeof(D3D12_RTAS_PARTITIONED_TLAS_OPERATION)};
        ThrowIfFailed(frame.operationCount->Map(0, &operationRange,
                                                reinterpret_cast<void**>(const_cast<BYTE**>(&operationBytes))),
                      "Map(GPU operation readback)");
        std::memcpy(&m_lastChangedCount, operationBytes + offsetof(D3D12_RTAS_PARTITIONED_TLAS_OPERATION, ArgCount),
                    sizeof(m_lastChangedCount));
        frame.operationCount->Unmap(0, &writtenRange);
    }
    if (frame.pickPending)
    {
        const UINT* hit = nullptr;
        D3D12_RANGE pickRange{0, sizeof(UINT)};
        ThrowIfFailed(frame.pickedInstance->Map(0, &pickRange, reinterpret_cast<void**>(const_cast<UINT**>(&hit))),
                      "Map(pick result)");
        if (*hit < m_dominoCount)
        {
            m_toppleIndex = *hit;
            m_manualTopple = true;
            m_hasManualTopple = true;
            m_paused = false;
        }
        frame.pickedInstance->Unmap(0, &writtenRange);
    }
    if (m_csv)
        m_csv << frame.number << ',' << (frame.ptlas ? "ptlas" : "tlas") << ',' << (frame.mode + 1) << ','
              << (frame.hint == D3D12_RTAS_PARTITIONED_TLAS_FLAG_FAST_BUILD   ? "build"
                  : frame.hint == D3D12_RTAS_PARTITIONED_TLAS_FLAG_FAST_TRACE ? "trace"
                                                                              : "none")
              << ',' << m_lastChangedCount << ',' << m_gpuAnimationMs << ',' << m_gpuAsMs << ',' << m_gpuUpdateMs << ','
              << m_gpuTraceMs << ',' << m_gpuCompositeMs << ',' << m_gpuTotalMs << '\n';
    frame.pending = false;
    if (frame.number % 30 == 0)
        UpdateTitle();
}

void D3D12RaytracingPartitionedTLAS::BeginFrame()
{
    FrameContext& frame = m_frames[m_frameSlot];
    m_deviceResources.WaitForFence(frame.fenceValue);
    if (frame.pending)
        CollectFrame(frame);
    ThrowIfFailed(frame.allocator->Reset(), "Reset(frame allocator)");
    ThrowIfFailed(m_commandList->Reset(frame.allocator.Get(), nullptr), "Reset(frame command list)");
}

void D3D12RaytracingPartitionedTLAS::WriteStateDump()
{
    const void* state = nullptr;
    D3D12_RANGE range{0, SIZE_T(m_dominoCount) * sizeof(Motion)};
    ThrowIfFailed(m_stateDumpReadback->Map(0, &range, const_cast<void**>(&state)), "Map(state dump)");
    std::ofstream dump(m_stateDumpPath, std::ios::binary);
    dump.write(static_cast<const char*>(state), static_cast<std::streamsize>(range.End));
    m_stateDumpReadback->Unmap(0, nullptr);
    m_stateDumpPending = false;
}

void D3D12RaytracingPartitionedTLAS::OnRender()
{
    if (m_deviceRecoveryRequested)
    {
        RecoverDevice();
        return;
    }
    try
    {
        if (m_regenerateSceneRequested)
            RegenerateScene();
        if (m_frameNumber == m_testDeviceLossFrame)
        {
            m_testDeviceLossFrame = UINT64_MAX;
            if (m_debug)
                InspectDebugMessages();
            // Removes only this application's D3D12 device; no system TDR.
            m_device->RemoveDevice();
        }
        RenderFrame();
    }
    catch (const HrException& error)
    {
        if (!m_deviceResources.IsDeviceLost(error.Error()))
            throw;
        RecoverDevice();
    }
    catch (const std::exception&)
    {
        // DirectXTK has its own exception type. Recover only when the actual
        // D3D12 device was removed; other toolkit failures remain fatal.
        if (!m_deviceResources.IsDeviceLost(S_OK))
            throw;
        RecoverDevice();
    }
}

void D3D12RaytracingPartitionedTLAS::RecoverDevice()
{
    std::ofstream log("device_recovery.log", std::ios::app);
    log << "Device lost at submitted frame " << m_frameNumber << ", reason 0x" << std::hex
        << static_cast<unsigned long>(m_device->GetDeviceRemovedReason()) << std::dec << '\n';
    if (auto* infoQueue = m_deviceResources.GetInfoQueue())
    {
        try
        {
            WriteDebugMessages(infoQueue, log);
        }
        catch (const std::exception& error)
        {
            log << "Could not read removed-device diagnostics: " << error.what() << '\n';
        }
    }
    m_deviceRecoveryRequested = false;
    m_deviceResources.HandleDeviceLost();
    log << "Device restored; PTLAS rebuilt and simulation restarted\n";
}

void D3D12RaytracingPartitionedTLAS::RenderFrame()
{
    ReloadShadersIfRequested();
    BeginFrame();
    FrameContext& frame = m_frames[m_frameSlot];
    const UINT queryBase = m_frameSlot * 6;
    OnUpdate();
    // Timestamp intervals reported by CollectFrame:
    // 0..1 animation + optional state-dump copy; 1..2 AS update, its barrier,
    // and the PTLAS operation-count copy/transition; 3..4 DispatchRays;
    // 4..5 optional pick readback + compositing. Total is 0..5, including 2..3
    // raytracing setup. Backbuffer copy, HUD, image capture, and present follow.
    // Thus a zero-write AS measurement still includes sample bookkeeping, and
    // regular TLAS measurements omit the PTLAS-only count copy. The AS interval
    // measures the whole stage; isolating the API call needs narrower timestamps.
    m_commandList->EndQuery(m_timingQueries.Get(), D3D12_QUERY_TYPE_TIMESTAMP, queryBase);
    PIXBeginEvent(m_commandList.Get(), PIX_COLOR_INDEX(1), "Animation and instance writes");
    GenerateInstanceUpdates();
    PIXEndEvent(m_commandList.Get());
    CaptureStateDump();
    m_commandList->EndQuery(m_timingQueries.Get(), D3D12_QUERY_TYPE_TIMESTAMP, queryBase + 1);
    PIXBeginEvent(m_commandList.Get(), PIX_COLOR_INDEX(2), "Acceleration structure update");
    UpdateAccelerationStructure();
    PIXEndEvent(m_commandList.Get());
    ReadbackPtlasWriteCount(frame);
    m_commandList->EndQuery(m_timingQueries.Get(), D3D12_QUERY_TYPE_TIMESTAMP, queryBase + 2);
    DoRaytracing(frame, queryBase);
    Composite();
    m_commandList->EndQuery(m_timingQueries.Get(), D3D12_QUERY_TYPE_TIMESTAMP, queryBase + 5);
    m_commandList->ResolveQueryData(m_timingQueries.Get(), D3D12_QUERY_TYPE_TIMESTAMP, queryBase, 6,
                                    frame.timings.Get(), 0);

    CopyRaytracingOutputToBackbuffer();
    RenderUI();
    PresentFrame(frame);
}

void D3D12RaytracingPartitionedTLAS::ReloadShadersIfRequested()
{
    if (m_reloadShadersRequested)
    {
        m_reloadShadersRequested = false;
        m_deviceResources.WaitForGpu();
        try
        {
            ReloadShaders();
            m_shaderReloadError.clear();
            std::ofstream("dxr2_shader_reload.log") << "reload successful\n";
        }
        catch (const std::exception& error)
        {
            m_shaderReloadError = error.what();
            std::ofstream("dxr2_shader_reload.log") << m_shaderReloadError << '\n';
        }
    }
}

void D3D12RaytracingPartitionedTLAS::CaptureStateDump()
{
    const bool dumpState = !m_stateDumpPath.empty() && m_frameLimit && m_frameNumber + 1 == m_frameLimit;
    if (dumpState)
    {
        auto toCopy = CD3DX12_RESOURCE_BARRIER::Transition(m_physicsState[m_physicsCurrentIndex].Get(),
                                                           D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                                                           D3D12_RESOURCE_STATE_COPY_SOURCE);
        m_commandList->ResourceBarrier(1, &toCopy);
        m_commandList->CopyBufferRegion(m_stateDumpReadback.Get(), 0, m_physicsState[m_physicsCurrentIndex].Get(), 0,
                                        UINT64(m_dominoCount) * sizeof(Motion));
        m_stateDumpPending = true;
        auto fromCopy = CD3DX12_RESOURCE_BARRIER::Transition(m_physicsState[m_physicsCurrentIndex].Get(),
                                                             D3D12_RESOURCE_STATE_COPY_SOURCE,
                                                             D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        m_commandList->ResourceBarrier(1, &fromCopy);
    }
}

void D3D12RaytracingPartitionedTLAS::ReadbackPtlasWriteCount(FrameContext& frame)
{
    // Statistics only: the CPU reads this after the frame fence. It never gates
    // the indirect update, which consumes the GPU's ArgCount directly.
    if (m_usePtlas)
    {
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_gpuPtlasOperation.Get(),
                                                            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                                                            D3D12_RESOURCE_STATE_COPY_SOURCE);
        m_commandList->ResourceBarrier(1, &barrier);
        m_commandList->CopyBufferRegion(frame.operationCount.Get(), 0, m_gpuPtlasOperation.Get(), 0,
                                        sizeof(D3D12_RTAS_PARTITIONED_TLAS_OPERATION));
        m_gpuOperationState = D3D12_RESOURCE_STATE_COPY_SOURCE;
    }
}

void D3D12RaytracingPartitionedTLAS::DoRaytracing(FrameContext& frame, UINT queryBase)
{
    if (m_instancePartitionState != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
    {
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_instancePartitions.Get(), m_instancePartitionState,
                                                            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        m_commandList->ResourceBarrier(1, &barrier);
        m_instancePartitionState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    }
    if (m_partitionActivityState != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
    {
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_partitionActivity.Get(), m_partitionActivityState,
                                                            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        m_commandList->ResourceBarrier(1, &barrier);
        m_partitionActivityState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    }
    ID3D12DescriptorHeap* heaps[] = {m_descriptors.Get()};
    m_commandList->SetDescriptorHeaps(1, heaps);
    m_commandList->SetComputeRootSignature(m_pipelines.raytracingRoot.Get());
    m_commandList->SetComputeRootShaderResourceView(RayRoot::Scene, m_usePtlas ? m_ptlas->GetGPUVirtualAddress()
                                                                               : m_tlas->GetGPUVirtualAddress());
    m_commandList->SetComputeRootDescriptorTable(RayRoot::Images, m_descriptors->GetGPUDescriptorHandleForHeapStart());
    m_commandList->SetComputeRootConstantBufferView(RayRoot::Frame, frame.constants->GetGPUVirtualAddress());
    m_commandList->SetComputeRootShaderResourceView(RayRoot::InstancePartitions,
                                                    m_instancePartitions->GetGPUVirtualAddress());
    m_commandList->SetComputeRootShaderResourceView(RayRoot::PartitionActivity,
                                                    m_partitionActivity->GetGPUVirtualAddress());
    m_commandList->SetComputeRootShaderResourceView(RayRoot::PhysicsState,
                                                    m_physicsState[m_physicsCurrentIndex]->GetGPUVirtualAddress());
    m_commandList->SetComputeRootShaderResourceView(RayRoot::TriangleData, m_triangleData->GetGPUVirtualAddress());
    m_commandList->SetComputeRootUnorderedAccessView(RayRoot::PickResult, m_pickResult->GetGPUVirtualAddress());
    m_commandList->SetPipelineState1(m_pipelines.raytracing.Get());
    constexpr UINT64 tableStride = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
    const D3D12_GPU_VIRTUAL_ADDRESS tableStart = m_pipelines.shaderTable->GetGPUVirtualAddress();
    D3D12_DISPATCH_RAYS_DESC dispatch{};
    dispatch.RayGenerationShaderRecord = {tableStart, tableStride};
    dispatch.MissShaderTable = {tableStart + tableStride, tableStride, tableStride};
    dispatch.HitGroupTable = {tableStart + tableStride * 2, tableStride, tableStride};
    dispatch.Width = m_width * m_renderScale;
    dispatch.Height = m_height * m_renderScale;
    dispatch.Depth = 1;
    m_commandList->EndQuery(m_timingQueries.Get(), D3D12_QUERY_TYPE_TIMESTAMP, queryBase + 3);
    PIXBeginEvent(m_commandList.Get(), PIX_COLOR_INDEX(3), "Raytracing");
    m_commandList->DispatchRays(&dispatch);
    PIXEndEvent(m_commandList.Get());
    m_commandList->EndQuery(m_timingQueries.Get(), D3D12_QUERY_TYPE_TIMESTAMP, queryBase + 4);
    if (m_pickThisFrame)
    {
        auto toCopy = CD3DX12_RESOURCE_BARRIER::Transition(m_pickResult.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                                           D3D12_RESOURCE_STATE_COPY_SOURCE);
        m_commandList->ResourceBarrier(1, &toCopy);
        m_commandList->CopyBufferRegion(frame.pickedInstance.Get(), 0, m_pickResult.Get(), 0, sizeof(UINT));
        auto fromCopy = CD3DX12_RESOURCE_BARRIER::Transition(m_pickResult.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE,
                                                             D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        m_commandList->ResourceBarrier(1, &fromCopy);
    }
}

void D3D12RaytracingPartitionedTLAS::Composite()
{
    D3D12_RESOURCE_BARRIER imageBarriers[3] = {CD3DX12_RESOURCE_BARRIER::UAV(m_traceImages[0].Get()),
                                               CD3DX12_RESOURCE_BARRIER::UAV(m_traceImages[1].Get()),
                                               CD3DX12_RESOURCE_BARRIER::UAV(m_traceImages[2].Get())};
    m_commandList->ResourceBarrier(3, imageBarriers);
    m_commandList->SetComputeRootSignature(m_pipelines.compositingRoot.Get());
    m_commandList->SetComputeRootDescriptorTable(CompositeRoot::Images,
                                                 m_descriptors->GetGPUDescriptorHandleForHeapStart());
    m_commandList->SetPipelineState(m_pipelines.compositing.Get());
    PIXBeginEvent(m_commandList.Get(), PIX_COLOR_INDEX(4), "Compositing");
    for (UINT pass = 0; pass < 4; ++pass)
    {
        m_commandList->SetComputeRoot32BitConstant(CompositeRoot::Pass, pass, 0);
        m_commandList->Dispatch((m_width * m_renderScale + 15) / 16, (m_height * m_renderScale + 15) / 16, 1);
        if (pass < 3)
        {
            D3D12_RESOURCE_BARRIER passBarriers[2] = {CD3DX12_RESOURCE_BARRIER::UAV(m_traceImages[0].Get()),
                                                      CD3DX12_RESOURCE_BARRIER::UAV(m_traceImages[1].Get())};
            m_commandList->ResourceBarrier(2, passBarriers);
        }
    }
    if (m_renderScale == 2)
    {
        auto rendered = CD3DX12_RESOURCE_BARRIER::UAV(m_renderOutput.Get());
        m_commandList->ResourceBarrier(1, &rendered);
        m_commandList->SetComputeRoot32BitConstant(CompositeRoot::Pass, 4, 0);
        m_commandList->Dispatch((m_width + 15) / 16, (m_height + 15) / 16, 1);
    }
    PIXEndEvent(m_commandList.Get());
}

void D3D12RaytracingPartitionedTLAS::CopyRaytracingOutputToBackbuffer()
{
    ID3D12Resource* backBuffer = m_deviceResources.GetRenderTarget();
    const auto outputBarrier = CD3DX12_RESOURCE_BARRIER::UAV(m_output.Get());
    m_commandList->ResourceBarrier(1, &outputBarrier);
    D3D12_RESOURCE_BARRIER toCopy[2] = {
        CD3DX12_RESOURCE_BARRIER::Transition(m_output.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                             D3D12_RESOURCE_STATE_COPY_SOURCE),
        CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST)};
    m_commandList->ResourceBarrier(2, toCopy);
    m_commandList->CopyResource(backBuffer, m_output.Get());
    D3D12_RESOURCE_BARRIER fromCopy[2] = {
        CD3DX12_RESOURCE_BARRIER::Transition(m_output.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE,
                                             D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
        CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_COPY_DEST,
                                             D3D12_RESOURCE_STATE_RENDER_TARGET)};
    m_commandList->ResourceBarrier(2, fromCopy);
}

void D3D12RaytracingPartitionedTLAS::RenderUI()
{
    ID3D12Resource* backBuffer = m_deviceResources.GetRenderTarget();
    const auto rtv = m_deviceResources.GetRenderTargetView();
    m_commandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
    m_ui.Render(m_commandList.Get(), m_width, m_height);
    auto toPresent = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                          D3D12_RESOURCE_STATE_PRESENT);
    m_commandList->ResourceBarrier(1, &toPresent);
}

void D3D12RaytracingPartitionedTLAS::PresentFrame(FrameContext& frame)
{
    ThrowIfFailed(m_commandList->Close(), "Close(render command list)");
    ID3D12CommandList* lists[] = {m_commandList.Get()};
    m_deviceResources.GetCommandQueue()->ExecuteCommandLists(1, lists);
    m_ui.Commit(m_deviceResources.GetCommandQueue());
    if (!m_imagePath.empty() && m_frameLimit && m_frameNumber + 1 == m_frameLimit)
    {
        // Capture the composited image at internal resolution, without UI or
        // viewport scaling, for image inspection and repeatable captures.
        ThrowIfFailed(DirectX::SaveDDSTextureToFile(m_deviceResources.GetCommandQueue(), m_renderOutput.Get(),
                                                    m_imagePath.c_str(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
                      "SaveDDSTextureToFile(rendered image)");
    }
    if (m_screenshotRequested || (!m_screenshotPath.empty() && m_frameLimit && m_frameNumber + 1 == m_frameLimit))
    {
        // Optional capture waits for this frame. Normal rendering stays asynchronous.
        ThrowIfFailed(DirectX::SaveDDSTextureToFile(m_deviceResources.GetCommandQueue(),
                                                    m_deviceResources.GetRenderTarget(), m_screenshotPath.c_str(),
                                                    D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_PRESENT),
                      "SaveDDSTextureToFile");
        m_screenshotRequested = false;
    }
    m_deviceResources.Present(m_vsync);
    frame.number = m_frameNumber + 1;
    frame.mode = m_updateMode;
    frame.hint = m_ptlasHint;
    frame.ptlas = m_usePtlas;
    frame.pickPending = m_pickThisFrame;
    frame.pending = true;
    frame.fenceValue = m_deviceResources.Signal();
    m_pickRequested = false;
    ++m_frameNumber;
    // This clock advances on paused frames too: contact ages, activity colors,
    // and shader effects use rendered-frame ticks. Reset-motion keeps the clock;
    // device recovery or scene regeneration restarts it in OnDeviceLost.
    ++m_simulationFrame;
    m_frameSlot = static_cast<UINT>(m_frameNumber % kFrameCount);
    if (m_frameLimit && m_frameNumber >= m_frameLimit)
        PostQuitMessage(0);
}

void D3D12RaytracingPartitionedTLAS::ExecuteAndWait()
{
    m_deviceResources.ExecuteAndWait();
}

void D3D12RaytracingPartitionedTLAS::UpdateTitle()
{
    const wchar_t* names[] = {L"update partition", L"move to global", L"near update / far global"};
    wchar_t title[512];
    const float fps = m_fps;
    swprintf_s(title, L"DXR 2 PTLAS | %.0f FPS | GPU %u %s | %s | %s | %s | %u writes | update %.2f ms | trace %.2f ms",
               fps, m_deviceResources.GetAdapterIndex(), m_deviceResources.GetAdapterDescription().c_str(),
               m_usePtlas ? L"PTLAS" : L"TLAS", names[m_updateMode], m_paused ? L"paused" : L"running",
               m_lastChangedCount, m_gpuUpdateMs, m_gpuTraceMs);
    SetWindowTextW(m_window, title);
}

void D3D12RaytracingPartitionedTLAS::OnDeviceLost()
{
    m_deviceReady = false;
    m_ui.ReleaseDeviceResources();
    m_frames = {};
    m_pipelines = {};
    m_traceImages = {};
    m_staticBlas = {};
    m_physicsState = {};
    m_device.Reset();
    m_commandList.Reset();
    m_dxr2Device.Reset();
    m_dxr2CommandList.Reset();
    m_output.Reset();
    m_renderOutput.Reset();
    m_descriptors.Reset();
    m_vertices.Reset();
    m_indices.Reset();
    m_triangleData.Reset();
    m_pickResult.Reset();
    m_blas.Reset();
    m_tlas.Reset();
    m_ptlas.Reset();
    m_tlasScratch.Reset();
    m_ptlasScratch.Reset();
    m_originalTlasUpload.Reset();
    m_ptlasInstances.Reset();
    m_gpuTlasInstances.Reset();
    m_gpuOriginalRegular.Reset();
    m_gpuOriginalPartitioned.Reset();
    m_seedAngularVelocities.Reset();
    m_gpuPtlasWrites.Reset();
    m_gpuPtlasOperation.Reset();
    m_instancePartitions.Reset();
    m_partitionActivity.Reset();
    m_partitionActivityZero.Reset();
    m_ptlasOperations.Reset();
    m_ptlasZeroOperation.Reset();
    m_ptlasOperationCount.Reset();
    m_timingQueries.Reset();
    m_stateDumpReadback.Reset();
    m_frameSlot = 0;
    m_simulationFrame = 0;
    m_physicsCurrentIndex = 0;
    m_physicsResetPending = true;
    m_tlasNeedsSync = m_ptlasNeedsSync = true;
    m_partitionActivityState = D3D12_RESOURCE_STATE_COPY_DEST;
    m_instancePartitionState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    m_gpuTlasInstancesState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    m_gpuPtlasWritesState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    m_gpuOperationState = D3D12_RESOURCE_STATE_COPY_DEST;
    m_stateDumpPending = false;
    m_pickRequested = m_pickThisFrame = m_manualTopple = m_hasManualTopple = false;
    m_lastChangedCount = 0;
    m_gpuUpdateMs = m_gpuAnimationMs = m_gpuAsMs = m_gpuTraceMs = m_gpuCompositeMs = m_gpuTotalMs = 0;
}
