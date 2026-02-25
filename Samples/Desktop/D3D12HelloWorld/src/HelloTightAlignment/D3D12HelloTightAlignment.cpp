// D3D12HelloTightAlignment.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <iostream>
#include <atlbase.h>
#include <vector>
#include <initguid.h>
#include "dxcapi.h"
#include "d3d12.h"
#include "d3dx12.h"
#include <dxgi1_6.h>

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 618; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\"; }
extern "C" { __declspec(dllexport) extern const char* WarpPath = u8".\\WARP\\"; }

// use a warp device instead of a hardware device
bool g_useWarpDevice = false;

#pragma region helper_and_setup
//=================================================================================================================================
// Helper / setup code, not specific to work graphs
// Look for "Start of interesting code" further below or just collapse this region.
//=================================================================================================================================
#define PRINT(text) std::cout << (char*)text << "\n" << std::flush; 
#define VERIFY_SUCCEEDED(hr) {HRESULT hrLocal = hr; if(FAILED(hrLocal)) {PRINT("Error at: " << __FILE__ << ", line: " << __LINE__ << ", HRESULT: 0x" << std::hex << hrLocal); throw E_FAIL;} }

struct D3DContext
{
    CComPtr<ID3D12Device14> spDevice;
    CComPtr<IDXGIAdapter3> spAdapter;
    bool bIsHeapTier1;
};

//=================================================================================================================================
void InitDeviceAndContext(D3DContext& D3D, bool useWarp)
{
    D3D.spDevice.Release();
    D3D.spAdapter.Release();

    CComPtr<ID3D12Debug1> pDebug;
    VERIFY_SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug)));
    pDebug->EnableDebugLayer();

    D3D_FEATURE_LEVEL FL = D3D_FEATURE_LEVEL_11_0;
    CComPtr<ID3D12Device> spDevice;
    CComPtr<IDXGIFactory4> factory;
    VERIFY_SUCCEEDED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));

    if (useWarp)
    {
        VERIFY_SUCCEEDED(factory->EnumWarpAdapter(IID_PPV_ARGS(&D3D.spAdapter)));
        VERIFY_SUCCEEDED(D3D12CreateDevice(D3D.spAdapter, FL, IID_PPV_ARGS(&spDevice)));
    }
    else
    {
        VERIFY_SUCCEEDED(D3D12CreateDevice(NULL, FL, IID_PPV_ARGS(&spDevice)));
        LUID luid = spDevice->GetAdapterLuid();
        factory->EnumAdapterByLuid(luid, IID_PPV_ARGS(&D3D.spAdapter));
    }
    D3D.spDevice = spDevice;
    D3D.bIsHeapTier1 = false;
}

template <UINT _Size>
LPSTR FormatMemoryUsage(UINT64 usage, CHAR(&result)[_Size])
{
    const UINT64 mb = 1 << 20;
    const UINT64 kb = 1 << 10;
    if (usage > mb)
    {
        sprintf_s(result, "%.2f MB", static_cast<float>(usage) / mb);
    }
    else if (usage > kb)
    {
        sprintf_s(result, "%.2f KB", static_cast<float>(usage) / kb);
    }
    else
    {
        sprintf_s(result, "%I64d B", usage);
    }
    return result;
}

void PrintGPUMemoryUsage(D3DContext& D3D, UINT64 baselineUsage = 0)
{
    DXGI_QUERY_VIDEO_MEMORY_INFO memoryInfo;
    D3D.spAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memoryInfo);

    char text[100];
    char usageString[20];
    UINT64 usage = memoryInfo.CurrentUsage - baselineUsage;
    if (baselineUsage > memoryInfo.CurrentUsage)
    {
        PRINT(" ## Error - baseline usage was greater than current usage");
        return;
    }
    sprintf_s(text, "  Memory Used: %s", FormatMemoryUsage(usage, usageString));
    PRINT(text);
}

#pragma endregion

template<UINT32 numBuffers>
void CreateManyPlacedBuffers(D3DContext& D3D, const CD3DX12_RESOURCE_DESC1& rDesc)
{
    DXGI_QUERY_VIDEO_MEMORY_INFO baslineMemoryInfo;
    D3D.spAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &baslineMemoryInfo);

    D3D12_RESOURCE_ALLOCATION_INFO1 infos[numBuffers];
    D3D12_RESOURCE_DESC1 pResourceDescs[numBuffers];
    std::fill_n(pResourceDescs, numBuffers, rDesc);
    D3D12_RESOURCE_ALLOCATION_INFO aggregateInfo = D3D.spDevice->GetResourceAllocationInfo2(0, numBuffers, pResourceDescs, infos);

    CComPtr<ID3D12Heap> pHeap;
    CComPtr<ID3D12Resource> buffers[numBuffers];
    CD3DX12_HEAP_DESC heapDesc = CD3DX12_HEAP_DESC(aggregateInfo.SizeInBytes, D3D12_HEAP_TYPE_DEFAULT);
    heapDesc.Flags = D3D.bIsHeapTier1 ? D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS : D3D12_HEAP_FLAG_NONE;
    VERIFY_SUCCEEDED(D3D.spDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&pHeap)));
    for (UINT32 i = 0; i < numBuffers; i++)
    {
        VERIFY_SUCCEEDED(D3D.spDevice->CreatePlacedResource1(pHeap, infos[i].Offset, &rDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffers[i])));
    }
    PrintGPUMemoryUsage(D3D, baslineMemoryInfo.CurrentUsage);
}

template<UINT32 numBuffers>
void ComparePlacedBufferMemoryFootprint(D3DContext& D3D)
{
    PRINT("Comparing memory footprint for " << numBuffers << " placed buffers...\n");
    CD3DX12_RESOURCE_DESC1 bufferDesc;
    //Beyond the obvious benefits for small resources, you can also see savings for large buffers that go beyond the 64KiB mark
    UINT bufferSizes[] = { 1, 16, 256, 4096, 32000, 65540, 80000 };
    for (UINT bufferSize : bufferSizes)
    {
        PRINT("Size: " << bufferSize << " B");
        PRINT("  Classic buffer alignment:");
        bufferDesc = CD3DX12_RESOURCE_DESC1::Buffer(bufferSize);
        CreateManyPlacedBuffers<numBuffers>(D3D, bufferDesc);

        PRINT("  Tight buffer alignment:");
        bufferDesc = CD3DX12_RESOURCE_DESC1::Buffer(bufferSize, D3D12_RESOURCE_FLAG_USE_TIGHT_ALIGNMENT);
        CreateManyPlacedBuffers<numBuffers>(D3D, bufferDesc);

        PRINT("----------");
    }
    PRINT("====================");
}

template<UINT32 numBuffers>
void CreateManyCommittedBuffers(D3DContext& D3D, const CD3DX12_RESOURCE_DESC1& rDesc)
{
    DXGI_QUERY_VIDEO_MEMORY_INFO baslineMemoryInfo;
    D3D.spAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &baslineMemoryInfo);

    CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    CComPtr<ID3D12Resource> buffers[numBuffers];
    for (UINT32 i = 0; i < numBuffers; i++)
    {
        VERIFY_SUCCEEDED(D3D.spDevice->CreateCommittedResource2(&heapProperties, D3D12_HEAP_FLAG_NONE, &rDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, nullptr, IID_PPV_ARGS(&buffers[i])));
    }

    PrintGPUMemoryUsage(D3D, baslineMemoryInfo.CurrentUsage);
}

template<UINT32 numBuffers>
void CompareCommittedBufferMemoryFootprint(D3DContext& D3D)
{
    PRINT("Comparing memory footprint for " << numBuffers << " committed buffers...\n");
    CD3DX12_RESOURCE_DESC1 bufferDesc;
    
    UINT bufferSizes[] = { 1, 16, 256, 4096, 65536, 65540, 80000 };
    for (UINT bufferSize : bufferSizes)
    {
        PRINT("Size: " << bufferSize << " B");
        PRINT("  Classic buffer alignment:");
        bufferDesc = CD3DX12_RESOURCE_DESC1::Buffer(bufferSize);
        CreateManyCommittedBuffers<numBuffers>(D3D, bufferDesc);

        PRINT("  Tight buffer alignment:");
        bufferDesc = CD3DX12_RESOURCE_DESC1::Buffer(bufferSize, D3D12_RESOURCE_FLAG_USE_TIGHT_ALIGNMENT);
        CreateManyCommittedBuffers<numBuffers>(D3D, bufferDesc);

        PRINT("----------");
    }
    PRINT("====================");
}

int main()
{
    try
    {
        PRINT("\n" <<
            "==================================================================================\n" <<
            " Hello Tight Alignment\n" <<
            "==================================================================================");
        D3DContext D3D;
        InitDeviceAndContext(D3D, g_useWarpDevice);
        bool warpFallback = false;

        CD3DX12FeatureSupport featureSupport;
        featureSupport.Init(D3D.spDevice);
        if (featureSupport.TightAlignmentSupportTier() == D3D12_TIGHT_ALIGNMENT_TIER_NOT_SUPPORTED && !g_useWarpDevice)
        {
            PRINT("Tight alignment is not supported on this driver. Retrying with Warp...");
            InitDeviceAndContext(D3D, true);
            featureSupport.Init(D3D.spDevice);
            warpFallback = true;
        }
        if (featureSupport.TightAlignmentSupportTier() == D3D12_TIGHT_ALIGNMENT_TIER_NOT_SUPPORTED)
        {
            PRINT("Tight alignment is not supported on this driver or the currently installed version of Warp. Aborting.");
            return 0;
        }
        DXGI_ADAPTER_DESC desc;
        VERIFY_SUCCEEDED(D3D.spAdapter->GetDesc(&desc));
        std::wcout << L"Running sample on " << desc.Description << L"\n\n" << std::flush;

        D3D12_FEATURE_DATA_D3D12_OPTIONS options{};
        VERIFY_SUCCEEDED(D3D.spDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options)));
        D3D.bIsHeapTier1 = options.ResourceHeapTier == D3D12_RESOURCE_HEAP_TIER_1;

        // Depending on the vendor, you may actually see benefits for placed resources even with only 1 resource in the heap (smaller heap allocation).
        ComparePlacedBufferMemoryFootprint<1>(D3D);     // No benefit due to the heap allocations being made in 64KiB chuncks
        ComparePlacedBufferMemoryFootprint<8>(D3D);     // Can already see some benefit being realized
        ComparePlacedBufferMemoryFootprint<4096>(D3D);  // drastic savings in most cases

        PRINT("\n========================================\n========================================\n");

        // Depending on the vendor, there may be no difference here since they now receive a hint that the heap is being implicitly created for a single resource.
        CompareCommittedBufferMemoryFootprint<1>(D3D);
        CompareCommittedBufferMemoryFootprint<8>(D3D);
        CompareCommittedBufferMemoryFootprint<4096>(D3D);
    }
    catch (HRESULT)
    {
        PRINT("Aborting.");
        return -1;
    }
    return 0;
    
}
