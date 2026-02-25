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
// Author:  James Stanard 
//

#include "pch.h"
#include "GraphicsCore.h"
#include "GameCore.h"
#include "BufferManager.h"
#include "GpuTimeManager.h"
#include "PostEffects.h"
#include "SSAO.h"
#include "TextRenderer.h"
#include "ColorBuffer.h"
#include "SystemTime.h"
#include "SamplerManager.h"
#include "DescriptorHeap.h"
#include "CommandContext.h"
#include "CommandListManager.h"
#include "RootSignature.h"
#include "CommandSignature.h"
#include "ParticleEffectManager.h"
#include "GraphRenderer.h"
#include "TemporalEffects.h"
#include "Display.h"

#pragma comment(lib, "d3d12.lib") 

#ifdef _GAMING_DESKTOP
    #include <winreg.h>		// To read the registry
#endif

using namespace Math;

namespace Graphics
{
#ifndef RELEASE
    const GUID WKPDID_D3DDebugObjectName = { 0x429b8c22,0x9188,0x4b0c, { 0x87,0x42,0xac,0xb0,0xbf,0x85,0xc2,0x00 }};
#endif

    bool g_bTypedUAVLoadSupport_R11G11B10_FLOAT = false;
    bool g_bTypedUAVLoadSupport_R16G16B16A16_FLOAT = false;

    ID3D12Device* g_Device = nullptr;
    CommandListManager g_CommandManager;
    ContextManager g_ContextManager;

    D3D_FEATURE_LEVEL g_D3DFeatureLevel = D3D_FEATURE_LEVEL_11_0;

    DescriptorAllocator g_DescriptorAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
    {
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        D3D12_DESCRIPTOR_HEAP_TYPE_DSV
    };

    static const uint32_t vendorID_Nvidia   = 0x10DE;
    static const uint32_t vendorID_AMD      = 0x1002;
    static const uint32_t vendorID_Intel    = 0x8086;

    uint32_t GetDesiredGPUVendor()
    {
        uint32_t desiredVendor = 0;

        std::wstring vendorVal;
        if (CommandLineArgs::GetString(L"vendor", vendorVal))
        {
            // Convert to lower case
            std::transform(vendorVal.begin(), vendorVal.end(), vendorVal.begin(), std::towlower);

            if (vendorVal.find(L"amd") != std::wstring::npos)
            {
                desiredVendor = vendorID_AMD;
            }
            else if (vendorVal.find(L"nvidia") != std::wstring::npos || vendorVal.find(L"nvd") != std::wstring::npos ||
                vendorVal.find(L"nvda") != std::wstring::npos || vendorVal.find(L"nv") != std::wstring::npos)
            {
                desiredVendor = vendorID_Nvidia;
            }
            else if (vendorVal.find(L"intel") != std::wstring::npos || vendorVal.find(L"intc") != std::wstring::npos)
            {
                desiredVendor = vendorID_Intel;
            }
        }

        return desiredVendor;
    }

    const wchar_t* GPUVendorToString(uint32_t vendorID)
    {
        switch (vendorID)
        {
        case vendorID_Nvidia:
            return L"Nvidia";
        case vendorID_AMD:
            return L"AMD";
        case vendorID_Intel:
            return L"Intel";
        default:
            return L"Unknown";
            break;
        }
    }

    uint32_t GetVendorIdFromDevice(ID3D12Device* pDevice)
    {
        LUID luid = pDevice->GetAdapterLuid();

        // Obtain the DXGI factory
        Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
        ASSERT_SUCCEEDED(CreateDXGIFactory2(0, MY_IID_PPV_ARGS(&dxgiFactory)));

        Microsoft::WRL::ComPtr<IDXGIAdapter1> pAdapter;

        if (SUCCEEDED(dxgiFactory->EnumAdapterByLuid(luid, MY_IID_PPV_ARGS(&pAdapter))))
        {
            DXGI_ADAPTER_DESC1 desc;
            if(SUCCEEDED(pAdapter->GetDesc1(&desc)))
            {
                return desc.VendorId;
            }
        }

        return 0;
    }

    bool IsDeviceNvidia(ID3D12Device* pDevice)
    {
        return GetVendorIdFromDevice(pDevice) == vendorID_Nvidia;
    }

    bool IsDeviceAMD(ID3D12Device* pDevice)
    {
        return GetVendorIdFromDevice(pDevice) == vendorID_AMD;
    }

    bool IsDeviceIntel(ID3D12Device* pDevice)
    {
        return GetVendorIdFromDevice(pDevice) == vendorID_Intel;
    }

	// Check adapter support for DirectX Raytracing.
	bool IsDirectXRaytracingSupported(ID3D12Device* testDevice)
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupport = {};

        if (FAILED(testDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureSupport, sizeof(featureSupport))))
            return false;

        return featureSupport.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
	}
}

// Initialize the DirectX resources required to run.
void Graphics::Initialize(bool RequireDXRSupport)
{
    Microsoft::WRL::ComPtr<ID3D12Device> pDevice;

    uint32_t useDebugLayers = 0;
    CommandLineArgs::GetInteger(L"debug", useDebugLayers);
#if _DEBUG
    // Default to true for debug builds
    useDebugLayers = 1;
#endif

    DWORD dxgiFactoryFlags = 0;

    if (useDebugLayers)
    {
        Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface;
        if (SUCCEEDED(D3D12GetDebugInterface(MY_IID_PPV_ARGS(&debugInterface))))
        {
            debugInterface->EnableDebugLayer();

            uint32_t useGPUBasedValidation = 0;
            CommandLineArgs::GetInteger(L"gpu_debug", useGPUBasedValidation);
            if (useGPUBasedValidation)
            {
                Microsoft::WRL::ComPtr<ID3D12Debug1> debugInterface1;
                if (SUCCEEDED((debugInterface->QueryInterface(MY_IID_PPV_ARGS(&debugInterface1)))))
                {
                    debugInterface1->SetEnableGPUBasedValidation(true);
                }
            }
        }
        else
        {
            Utility::Print("WARNING:  Unable to enable D3D12 debug validation layer\n");
        }

#if _DEBUG
        ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
        {
            dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

            DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
            {
                80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
            };
            DXGI_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
        }
#endif
    }

    // Obtain the DXGI factory
    Microsoft::WRL::ComPtr<IDXGIFactory6> dxgiFactory;
    ASSERT_SUCCEEDED(CreateDXGIFactory2(dxgiFactoryFlags, MY_IID_PPV_ARGS(&dxgiFactory)));

    // Create the D3D graphics device
    Microsoft::WRL::ComPtr<IDXGIAdapter1> pAdapter;

    uint32_t bUseWarpDriver = false;
    CommandLineArgs::GetInteger(L"warp", bUseWarpDriver);

    uint32_t desiredVendor = GetDesiredGPUVendor();

    if (desiredVendor)
    {
        Utility::Printf(L"Looking for a %s GPU\n", GPUVendorToString(desiredVendor));
    }

    // Temporary workaround because SetStablePowerState() is crashing
    D3D12EnableExperimentalFeatures(0, nullptr, nullptr, nullptr);

    if (!bUseWarpDriver)
    {
        SIZE_T MaxSize = 0;

        for (uint32_t Idx = 0; DXGI_ERROR_NOT_FOUND != dxgiFactory->EnumAdapters1(Idx, &pAdapter); ++Idx)
        {
            DXGI_ADAPTER_DESC1 desc;
            pAdapter->GetDesc1(&desc);

            // Is a software adapter?
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                continue;

            // Is this the desired vendor desired?
            if (desiredVendor != 0 && desiredVendor != desc.VendorId)
                continue;

            // Can create a D3D12 device?
            if (FAILED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, MY_IID_PPV_ARGS(&pDevice))))
                continue;

            // Does support DXR if required?
            if (RequireDXRSupport && !IsDirectXRaytracingSupported(pDevice.Get()))
                continue;

            // By default, search for the adapter with the most memory because that's usually the dGPU.
            if (desc.DedicatedVideoMemory < MaxSize)
                continue;

            MaxSize = desc.DedicatedVideoMemory;

            if (g_Device != nullptr)
                g_Device->Release();

            g_Device = pDevice.Detach();

            Utility::Printf(L"Selected GPU:  %s (%u MB)\n", desc.Description, desc.DedicatedVideoMemory >> 20);
        }
    }

    if (RequireDXRSupport && !g_Device)
    {
        Utility::Printf("Unable to find a DXR-capable device. Halting.\n");
        __debugbreak();
    }

    if (g_Device == nullptr)
    {
        if (bUseWarpDriver)
            Utility::Print("WARP software adapter requested.  Initializing...\n");
        else
            Utility::Print("Failed to find a hardware adapter.  Falling back to WARP.\n");
        ASSERT_SUCCEEDED(dxgiFactory->EnumWarpAdapter(MY_IID_PPV_ARGS(&pAdapter)));
        ASSERT_SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, MY_IID_PPV_ARGS(&pDevice)));
        g_Device = pDevice.Detach();
    }
#ifndef RELEASE
    else
    {
        bool DeveloperModeEnabled = false;

        // Look in the Windows Registry to determine if Developer Mode is enabled
        HKEY hKey;
        LSTATUS result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\AppModelUnlock", 0, KEY_READ, &hKey);
        if (result == ERROR_SUCCESS)
        {
            DWORD keyValue, keySize = sizeof(DWORD);
            result = RegQueryValueEx(hKey, L"AllowDevelopmentWithoutDevLicense", 0, NULL, (byte*)&keyValue, &keySize);
            if (result == ERROR_SUCCESS && keyValue == 1)
                DeveloperModeEnabled = true;
            RegCloseKey(hKey);
        }

        WARN_ONCE_IF_NOT(DeveloperModeEnabled, "Enable Developer Mode on Windows 10 to get consistent profiling results");

        // Prevent the GPU from overclocking or underclocking to get consistent timings
        if (DeveloperModeEnabled)
            g_Device->SetStablePowerState(TRUE);
    }
#endif	

#if _DEBUG
    ID3D12InfoQueue* pInfoQueue = nullptr;
    if (SUCCEEDED(g_Device->QueryInterface(MY_IID_PPV_ARGS(&pInfoQueue))))
    {
        // Suppress whole categories of messages
        //D3D12_MESSAGE_CATEGORY Categories[] = {};

        // Suppress messages based on their severity level
        D3D12_MESSAGE_SEVERITY Severities[] = 
        {
            D3D12_MESSAGE_SEVERITY_INFO
        };

        // Suppress individual messages by their ID
        D3D12_MESSAGE_ID DenyIds[] =
        {
            // This occurs when there are uninitialized descriptors in a descriptor table, even when a
            // shader does not access the missing descriptors.  I find this is common when switching
            // shader permutations and not wanting to change much code to reorder resources.
            D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,

            // Triggered when a shader does not export all color components of a render target, such as
            // when only writing RGB to an R10G10B10A2 buffer, ignoring alpha.
            D3D12_MESSAGE_ID_CREATEGRAPHICSPIPELINESTATE_PS_OUTPUT_RT_OUTPUT_MISMATCH,

            // This occurs when a descriptor table is unbound even when a shader does not access the missing
            // descriptors.  This is common with a root signature shared between disparate shaders that
            // don't all need the same types of resources.
            D3D12_MESSAGE_ID_COMMAND_LIST_DESCRIPTOR_TABLE_NOT_SET,

            // RESOURCE_BARRIER_DUPLICATE_SUBRESOURCE_TRANSITIONS
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_DUPLICATE_SUBRESOURCE_TRANSITIONS,

            // Suppress errors from calling ResolveQueryData with timestamps that weren't requested on a given frame.
            D3D12_MESSAGE_ID_RESOLVE_QUERY_INVALID_QUERY_STATE,

            // Ignoring InitialState D3D12_RESOURCE_STATE_COPY_DEST. Buffers are effectively created in state D3D12_RESOURCE_STATE_COMMON.
            D3D12_MESSAGE_ID_CREATERESOURCE_STATE_IGNORED,
        };

        D3D12_INFO_QUEUE_FILTER NewFilter = {};
        //NewFilter.DenyList.NumCategories = _countof(Categories);
        //NewFilter.DenyList.pCategoryList = Categories;
        NewFilter.DenyList.NumSeverities = _countof(Severities);
        NewFilter.DenyList.pSeverityList = Severities;
        NewFilter.DenyList.NumIDs = _countof(DenyIds);
        NewFilter.DenyList.pIDList = DenyIds;

        pInfoQueue->PushStorageFilter(&NewFilter);
        pInfoQueue->Release();
    }
#endif

    // We like to do read-modify-write operations on UAVs during post processing.  To support that, we
    // need to either have the hardware do typed UAV loads of R11G11B10_FLOAT or we need to manually
    // decode an R32_UINT representation of the same buffer.  This code determines if we get the hardware
    // load support.
    D3D12_FEATURE_DATA_D3D12_OPTIONS FeatureData = {};
    if (SUCCEEDED(g_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &FeatureData, sizeof(FeatureData))))
    {
        if (FeatureData.TypedUAVLoadAdditionalFormats)
        {
            D3D12_FEATURE_DATA_FORMAT_SUPPORT Support =
            {
                DXGI_FORMAT_R11G11B10_FLOAT, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE
            };

            if (SUCCEEDED(g_Device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &Support, sizeof(Support))) &&
                (Support.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
            {
                g_bTypedUAVLoadSupport_R11G11B10_FLOAT = true;
            }

            Support.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

            if (SUCCEEDED(g_Device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &Support, sizeof(Support))) &&
                (Support.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
            {
                g_bTypedUAVLoadSupport_R16G16B16A16_FLOAT = true;
            }
        }
    }

    g_CommandManager.Create(g_Device);

    // Common state was moved to GraphicsCommon.*
    InitializeCommonState();

    Display::Initialize();

    GpuTimeManager::Initialize(4096);
    TemporalEffects::Initialize();
    PostEffects::Initialize();
    SSAO::Initialize();
    TextRenderer::Initialize();
    GraphRenderer::Initialize();
    ParticleEffectManager::Initialize(3840, 2160);
}

void Graphics::Shutdown( void )
{
    g_CommandManager.IdleGPU();

    CommandContext::DestroyAllContexts();
    g_CommandManager.Shutdown();
    GpuTimeManager::Shutdown();
    PSO::DestroyAll();
    RootSignature::DestroyAll();
    DescriptorAllocator::DestroyAll();

    DestroyCommonState();
    DestroyRenderingBuffers();
    TemporalEffects::Shutdown();
    PostEffects::Shutdown();
    SSAO::Shutdown();
    TextRenderer::Shutdown();
    GraphRenderer::Shutdown();
    ParticleEffectManager::Shutdown();
    Display::Shutdown();

#if defined(_GAMING_DESKTOP) && defined(_DEBUG)
    ID3D12DebugDevice* debugInterface;
    if (SUCCEEDED(g_Device->QueryInterface(&debugInterface)))
    {
        debugInterface->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
        debugInterface->Release();
    }
#endif

    if (g_Device != nullptr)
    {
        g_Device->Release();
        g_Device = nullptr;
    }
}
