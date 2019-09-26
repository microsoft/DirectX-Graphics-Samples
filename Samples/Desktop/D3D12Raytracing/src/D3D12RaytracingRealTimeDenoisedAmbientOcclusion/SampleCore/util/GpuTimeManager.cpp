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
#include "GpuTimeManager.h"

#ifndef IID_GRAPHICS_PPV_ARGS
#define IID_GRAPHICS_PPV_ARGS(x) IID_PPV_ARGS(x)
#endif

using namespace DirectX;
using namespace DX;
using namespace std;


static GpuTimeManager global_GpuTimeManager;
UINT GpuTimeManager::s_numInstances = 0;


namespace
{
    inline float lerp(float a, float b, float f)
    {
        return (1.f - f) * a + f * b;
    }

    inline float UpdateRunningAverage(float avg, float value)
    {
        return lerp(value, avg, 0.05f);
    }

    inline void DebugWarnings(UINT timerid, uint64_t start, uint64_t end)
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


GpuTimeManager& GpuTimeManager::instance()
{
    return global_GpuTimeManager;
}

void GpuTimeManager::RestoreDevice(ID3D12Device5* device, ID3D12CommandQueue* commandQueue, UINT maxFrameCount, UINT MaxNumTimers)
{
    m_maxframeCount = maxFrameCount;
    m_MaxNumTimers = MaxNumTimers;
    m_MaxNumTimerSlots = m_MaxNumTimers * 2;

    uint64_t GpuFrequency;
    commandQueue->GetTimestampFrequency(&GpuFrequency);
    m_GpuTickDelta = 1000.0 / static_cast<double>(GpuFrequency);

    D3D12_QUERY_HEAP_DESC QueryHeapDesc = {};
    QueryHeapDesc.Count = m_MaxNumTimerSlots;
    QueryHeapDesc.NodeMask = 1;
    QueryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
    ThrowIfFailed(device->CreateQueryHeap(&QueryHeapDesc, IID_GRAPHICS_PPV_ARGS(&m_QueryHeap)));
    m_QueryHeap->SetName(L"GpuTimeManager QueryHeap");

    // We allocate m_maxframeCount + 1 instances as an instance is guaranteed to be written to if maxPresentFrameCount frames
    // have been submitted since. This is due to a fact that Present stalls when none of the m_maxframeCount frames are done/available.
    size_t nPerFrameInstances = MaxNumTimers + 1;

    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(nPerFrameInstances * m_MaxNumTimerSlots * sizeof(UINT64));
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_GRAPHICS_PPV_ARGS(&m_ReadBackBuffer))
    );
    m_ReadBackBuffer->SetName(L"GpuTimeManager Readback Buffer");

    m_avg.resize(m_MaxNumTimers);
    m_timing.resize(m_MaxNumTimerSlots);
    m_avgPeriodTotal.resize(m_MaxNumTimers);    
    Reset();
}


void GpuTimeManager::ReleaseDevice()
{
    m_ReadBackBuffer.Reset();
    m_QueryHeap.Reset();
}

void GpuTimeManager::Start(ID3D12GraphicsCommandList4* commandList, UINT timerid)
{
    if (timerid == UINT_MAX)
    {
        return;
    }
    assert(timerid < m_MaxNumTimers && L"Timer ID out of range");
    commandList->EndQuery(m_QueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, timerid * 2);
}

void GpuTimeManager::Stop(ID3D12GraphicsCommandList4* commandList, UINT timerid)
{
    if (timerid == UINT_MAX)
    {
        return;
    }
    assert(timerid < m_MaxNumTimers && L"Timer ID out of range");
    commandList->EndQuery(m_QueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, timerid * 2 + 1);
}

float GpuTimeManager::GetElapsedMS(UINT timerid) const
{
    assert(timerid < m_MaxNumTimers && L"Timer ID out of range");

    UINT64 start = m_timing[timerid * 2];
    UINT64 end = m_timing[timerid * 2 + 1];

    if (end < start)
        return 0.f;

    return static_cast<float>(double(end - start) * m_GpuTickDelta);
}

UINT GpuTimeManager::NewTimer(void)
{
    return m_NumAllotedTimers++;
}

void GpuTimeManager::Reset()
{
    fill(m_avg.begin(), m_avg.end(), 0.f);
    fill(m_avgPeriodTotal.begin(), m_avgPeriodTotal.end(), 0.f);
    m_avgTimestampsTotal = 0;
    m_avgPeriodTimer.Reset();
    m_avgPeriodTimer.Start();
}

void GpuTimeManager::BeginFrame(ID3D12GraphicsCommandList4* commandList)
{
    UNREFERENCED_PARAMETER(commandList);
}

void GpuTimeManager::EndFrame(ID3D12GraphicsCommandList4* commandList)
{
    // Resolve query for the current frame.
    static UINT resolveToFrameID = 0;
    UINT64 resolveToBaseAddress = resolveToFrameID * m_MaxNumTimerSlots * sizeof(UINT64);
    commandList->ResolveQueryData(m_QueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, m_MaxNumTimerSlots, m_ReadBackBuffer.Get(), resolveToBaseAddress);

    // Grab read-back data for the queries from a finished frame m_maxframeCount ago.                                                           
    UINT readBackFrameID = (resolveToFrameID + 1) % (m_maxframeCount + 1);
    SIZE_T readBackBaseOffset = readBackFrameID * m_MaxNumTimerSlots * sizeof(UINT64);
    D3D12_RANGE dataRange =
    {
        readBackBaseOffset,
        readBackBaseOffset + m_MaxNumTimerSlots * sizeof(UINT64),
    };

    UINT64* timingData;
    ThrowIfFailed(m_ReadBackBuffer->Map(0, &dataRange, reinterpret_cast<void**>(&timingData)));
    memcpy(&m_timing[0], timingData, sizeof(UINT64) * m_MaxNumTimerSlots);
    m_ReadBackBuffer->Unmap(0, &CD3DX12_RANGE(0, 0));

    for (UINT j = 0; j < m_MaxNumTimers; ++j)
    {
        UINT64 start = m_timing[j * 2];
        UINT64 end = m_timing[j * 2 + 1];

        DebugWarnings(j, start, end);

        float value = float(double(end - start) * m_GpuTickDelta);
        m_avgPeriodTotal[j] += value;
    }
    m_avgTimestampsTotal++;

    // Update averages if the period duration has passed.
    m_avgPeriodTimer.Stop();
    float elapsedMs = m_avgPeriodTimer.GetElapsedMS();
    if (m_avgPeriodTimer.GetElapsedMS() >= m_avgRefreshPeriodMs)
    {
        for (UINT j = 0; j < m_MaxNumTimers; ++j)
        {
            m_avg[j] = m_avgPeriodTotal[j] / m_avgTimestampsTotal;
            m_avgPeriodTotal[j] = 0;
        }
        m_avgTimestampsTotal = 0;

        m_avgPeriodTimer.Start();
    }
    else
    {
        elapsedMs;
    }

    resolveToFrameID = readBackFrameID;
}