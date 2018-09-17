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
#include "PerformanceTimers.h"

#include "DirectXHelpers.h"

#include <exception>
#include <stdexcept>

using namespace DirectX;
using namespace DX;

using Microsoft::WRL::ComPtr;

namespace
{
    inline float lerp(float a, float b, float f)
    {
        return (1.f - f) * a + f * b;
    }

    inline float UpdateRunningAverage(float avg, float value)
    {
        return lerp(value, avg, 0.95f);
    }

    inline void DebugWarnings(uint32_t timerid, uint64_t start, uint64_t end)
    {
#if defined(_DEBUG)
        if (!start && end > 0)
        {
            char buff[128] = {};
            sprintf_s(buff, "ERROR: Timer %u stopped but not started\n", timerid);
            OutputDebugStringA(buff);
        }
        else if (start > 0 && !end)
        {
            char buff[128] = {};
            sprintf_s(buff, "ERROR: Timer %u started but not stopped\n", timerid);
            OutputDebugStringA(buff);
        }
#else
        UNREFERENCED_PARAMETER(timerid);
        UNREFERENCED_PARAMETER(start);
        UNREFERENCED_PARAMETER(end);
#endif
    }
};

//======================================================================================
// CPUTimer
//======================================================================================

CPUTimer::CPUTimer() :
    m_cpuFreqInv(1.f),
    m_start{},
    m_end{},
    m_avg{}
{
    LARGE_INTEGER cpuFreq;
    if (!QueryPerformanceFrequency(&cpuFreq))
    {
        throw std::exception("QueryPerformanceFrequency");
    }

    m_cpuFreqInv = 1000.0 / double(cpuFreq.QuadPart);
}

void CPUTimer::Start(uint32_t timerid)
{
    if (timerid >= c_maxTimers)
        throw std::out_of_range("Timer ID out of range");

    if (!QueryPerformanceCounter(&m_start[timerid]))
    {
        throw std::exception("QueryPerformanceCounter");
    }
}

void CPUTimer::Stop(uint32_t timerid)
{
    if (timerid >= c_maxTimers)
        throw std::out_of_range("Timer ID out of range");

    if (!QueryPerformanceCounter(&m_end[timerid]))
    {
        throw std::exception("QueryPerformanceCounter");
    }
}

void CPUTimer::Update()
{
    for (uint32_t j = 0; j < c_maxTimers; ++j)
    {
        uint64_t start = m_start[j].QuadPart;
        uint64_t end = m_end[j].QuadPart;

        DebugWarnings(j, start, end);

        float value = float(double(end - start) * m_cpuFreqInv);
        m_avg[j] = UpdateRunningAverage(m_avg[j], value);
    }
}

void CPUTimer::Reset()
{
    memset(m_avg, 0, sizeof(m_avg));
}

double CPUTimer::GetElapsedMS(uint32_t timerid) const
{
    if (timerid >= c_maxTimers)
        return 0.0;

    uint64_t start = m_start[timerid].QuadPart;
    uint64_t end = m_end[timerid].QuadPart;

    return double(end - start) * m_cpuFreqInv;
}


//======================================================================================
// GPUTimer (DirectX 12)
//======================================================================================

void GPUTimer::BeginFrame(_In_ ID3D12GraphicsCommandList* commandList)
{
    UNREFERENCED_PARAMETER(commandList);
}

void GPUTimer::EndFrame(_In_ ID3D12GraphicsCommandList* commandList)
{
    commandList->ResolveQueryData(m_heap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, c_timerSlots, m_buffer.Get(), 0);
    
    // Grab read-back data for the queries
    D3D12_RANGE dataRange =
    {
        0,
        c_timerSlots * sizeof(UINT64),
    };

    UINT64* timingData;
    ThrowIfFailed(m_buffer->Map(0, &dataRange, reinterpret_cast<void**>(&timingData)));
    memcpy(m_timing, timingData, sizeof(UINT64) * c_timerSlots);
    m_buffer->Unmap(0, nullptr);

    for (uint32_t j = 0; j < c_maxTimers; ++j)
    {
        UINT64 start = m_timing[j * 2];
        UINT64 end = m_timing[j * 2 + 1];

        DebugWarnings(j, start, end);

        float value = float(double(end - start) * m_gpuFreqInv);
        m_avg[j] = UpdateRunningAverage(m_avg[j], value);
    }
}

void GPUTimer::Start(_In_ ID3D12GraphicsCommandList* commandList, uint32_t timerid)
{
    if (timerid >= c_maxTimers)
        throw std::out_of_range("Timer ID out of range");

    commandList->EndQuery(m_heap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, timerid * 2);
}

void GPUTimer::Stop(_In_ ID3D12GraphicsCommandList* commandList, uint32_t timerid)
{
    if (timerid >= c_maxTimers)
        throw std::out_of_range("Timer ID out of range");

    commandList->EndQuery(m_heap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, timerid * 2 + 1);
}

void GPUTimer::Reset()
{
    memset(m_avg, 0, sizeof(m_avg));
}

double GPUTimer::GetElapsedMS(uint32_t timerid) const
{
    if (timerid >= c_maxTimers)
        return 0.0;
 
    UINT64 start = m_timing[timerid * 2];
    UINT64 end = m_timing[timerid * 2 + 1];

    if (end < start)
        return 0.0;

    return double(end - start) * m_gpuFreqInv;
}

void GPUTimer::ReleaseDevice()
{
    m_heap.Reset();
    m_buffer.Reset();
}

void GPUTimer::RestoreDevice(_In_ ID3D12Device* device, _In_ ID3D12CommandQueue* commandQueue)
{
    assert(device != 0 && commandQueue != 0);

#if defined(_DEBUG) || defined(PROFILE)
    if (FAILED(device->SetStablePowerState(TRUE)))
    {
        OutputDebugStringA("WARNING: Unable to set a stable power state.\n        profiling data may be inaccurate due to power management!\n");
    }
#endif

    UINT64 gpuFreq;
    ThrowIfFailed(commandQueue->GetTimestampFrequency(&gpuFreq));
    m_gpuFreqInv = 1000.0 / double(gpuFreq);

    D3D12_QUERY_HEAP_DESC desc = {};
    desc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
    desc.Count = c_timerSlots;
    ThrowIfFailed(device->CreateQueryHeap(&desc, IID_GRAPHICS_PPV_ARGS(m_heap.ReleaseAndGetAddressOf())));
    m_heap->SetName(L"GPUTimer");

    auto readBack = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);

    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(c_timerSlots * sizeof(UINT64));
    ThrowIfFailed(device->CreateCommittedResource(
        &readBack,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_GRAPHICS_PPV_ARGS(m_buffer.ReleaseAndGetAddressOf()))
    );
    m_buffer->SetName(L"GPUTimer");
}

