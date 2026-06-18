#pragma once

#include <chrono>
#include "DXSampleHelper.h"

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

namespace MyDx12Util
{

inline void CreateUploadBuffer(ID3D12Device* device, size_t size, ComPtr<ID3D12Resource>& ppResource)
{
    ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                  D3D12_HEAP_FLAG_NONE,
                                                  &CD3DX12_RESOURCE_DESC::Buffer(size),
                                                  D3D12_RESOURCE_STATE_GENERIC_READ,
                                                  nullptr,
                                                  IID_PPV_ARGS(&ppResource)));
}

class ScopedTimer
{
    const char* m_name;
    std::chrono::steady_clock::time_point m_start;

public:
    explicit ScopedTimer(const char* name)
        : m_name(name), m_start(std::chrono::steady_clock::now())
    {
    }

    ~ScopedTimer()
    {
        auto end = std::chrono::steady_clock::now();
        float ms = std::chrono::duration<float, std::milli>(end - m_start).count();
        char buf[256] = {};
        sprintf_s(buf, "[Timer] %s: %.2f ms\n", m_name, ms);
        OutputDebugStringA(buf);
    }
};

class GpuTimestampPair
{
    ComPtr<ID3D12QueryHeap> m_queryHeap;
    ComPtr<ID3D12Resource> m_readback;

public:
    void Init(ID3D12Device* device)
    {
        D3D12_QUERY_HEAP_DESC desc = {};
        desc.Count = 2;
        desc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
        ThrowIfFailed(device->CreateQueryHeap(&desc, IID_PPV_ARGS(&m_queryHeap)));

        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT64) * 2),
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_readback)));
    }

    void Begin(ID3D12GraphicsCommandList* cmdList)
    {
        cmdList->EndQuery(m_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0);
    }

    void End(ID3D12GraphicsCommandList* cmdList)
    {
        cmdList->EndQuery(m_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 1);
        cmdList->ResolveQueryData(m_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, 2, m_readback.Get(), 0);
    }

    float ReadMs(ID3D12CommandQueue* queue)
    {
        D3D12_RANGE readRange = {0, sizeof(UINT64) * 2};
        UINT64* data = nullptr;
        ThrowIfFailed(m_readback->Map(0, &readRange, reinterpret_cast<void**>(&data)));

        UINT64 freq = 0;
        queue->GetTimestampFrequency(&freq);

        float ms = 0.0f;
        if (freq > 0 && data[1] > data[0])
        {
            ms = (static_cast<float>(data[1] - data[0]) / static_cast<float>(freq)) * 1000.0f;
        }

        m_readback->Unmap(0, nullptr);
        return ms;
    }
};

} // namespace MyDx12Util

inline void DebugPrint(const char* fmt, ...)
{
    char buf[1024];

    va_list args;
    va_start(args, fmt);
    vsprintf_s(buf, fmt, args);
    va_end(args);

    OutputDebugStringA(buf);
}
#define DBG_PRINT(fmt, ...) DebugPrint("[%s:%d] " fmt, __FILE__, __LINE__, __VA_ARGS__)
