//*********************************************************
//
// Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// Copyright (c) 2026 Microsoft Corporation. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <cstdio>

using Microsoft::WRL::ComPtr;

extern "C"
{
    __declspec(dllexport) extern const UINT D3D12SDKVersion = D3D12_PREVIEW_SDK_VERSION;
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

int main()
{
    const UUID previewFeatures[] = {D3D12ExperimentalShaderModels, D3D12RaytracingPreview};
    const HRESULT enableResult =
        D3D12EnableExperimentalFeatures(_countof(previewFeatures), previewFeatures, nullptr, nullptr);
    std::printf("Enable preview features: 0x%08lX\n", static_cast<unsigned long>(enableResult));
    if (FAILED(enableResult))
        return 1;

    ComPtr<IDXGIFactory6> factory;
    HRESULT result = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
    if (FAILED(result))
    {
        std::printf("CreateDXGIFactory2 failed: 0x%08lX\n", static_cast<unsigned long>(result));
        return 1;
    }

    ComPtr<ID3D12Device> device;
    bool foundHardware = false;
    for (UINT index = 0;; ++index)
    {
        ComPtr<IDXGIAdapter1> adapter;
        result = factory->EnumAdapterByGpuPreference(index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                                                     IID_PPV_ARGS(&adapter));
        if (result == DXGI_ERROR_NOT_FOUND)
            break;
        if (FAILED(result))
        {
            std::printf("EnumAdapterByGpuPreference failed: 0x%08lX\n", static_cast<unsigned long>(result));
            return 1;
        }
        DXGI_ADAPTER_DESC1 description{};
        result = adapter->GetDesc1(&description);
        if (FAILED(result))
        {
            std::printf("GetDesc1 failed: 0x%08lX\n", static_cast<unsigned long>(result));
            return 1;
        }
        if (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            continue;
        foundHardware = true;
        std::printf("Adapter %u: %ls\n", index, description.Description);

        ComPtr<ID3D12Device> candidate;
        result = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&candidate));
        if (FAILED(result))
        {
            std::printf("D3D12CreateDevice failed: 0x%08lX\n", static_cast<unsigned long>(result));
            continue;
        }

        ComPtr<ID3D12Device16> capabilities;
        result = candidate.As(&capabilities);
        std::printf("ID3D12Device16: 0x%08lX\n", static_cast<unsigned long>(result));
        if (FAILED(result))
            continue;
        D3D12_FEATURE_DATA_D3D12_OPTIONS_RAYTRACING2 raytracing2{};
        result = capabilities->CheckFeatureSupport1(D3D12_FEATURE_D3D12_OPTIONS_RAYTRACING2, &raytracing2,
                                                    sizeof(raytracing2), D3D12_PREVIEW_SDK_VERSION);
        std::printf("Clustered geometry / PTLAS query: 0x%08lX, ClustersAndPTLASSupported=%d\n", static_cast<unsigned long>(result),
                    raytracing2.ClustersAndPTLASSupported);
        if (FAILED(result) || !raytracing2.ClustersAndPTLASSupported)
            continue;

        device = candidate;
        std::printf("Selected adapter %u: %ls\n", index, description.Description);
        break;
    }
    if (!device)
    {
        std::puts(foundHardware ? "No compatible partitioned TLAS adapter found" : "No hardware adapter found");
        return foundHardware ? 2 : 1;
    }

    D3D12_COMMAND_QUEUE_DESC queueDescription{};
    queueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ComPtr<ID3D12CommandQueue> queue;
    result = device->CreateCommandQueue(&queueDescription, IID_PPV_ARGS(&queue));
    if (FAILED(result))
        return 2;

    ComPtr<ID3D12CommandAllocator> allocator;
    result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));
    if (FAILED(result))
        return 2;

    ComPtr<ID3D12GraphicsCommandList4> commandList;
    result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr,
                                       IID_PPV_ARGS(&commandList));
    if (FAILED(result))
        return 2;

    ComPtr<ID3D12CommandListRaytracing2> raytracingCommandList;
    result = commandList.As(&raytracingCommandList);
    std::printf("ID3D12CommandListRaytracing2: 0x%08lX\n", static_cast<unsigned long>(result));
    return SUCCEEDED(result) ? 0 : 2;
}
