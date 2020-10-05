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
#include "GPUTimer.h"
#include "DXSampleHelper.h"

#include <exception>
#include <stdexcept>

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
};

void GPUTimer::Init(_In_ ID3D12Device* pDevice, _In_ ID3D12CommandQueue* pCommandQueue, UINT maxFrameCount)
{
    assert(pDevice != nullptr && pCommandQueue != nullptr);
    m_maxframeCount = maxFrameCount;

    UINT64 gpuFreq;
    ThrowIfFailed(pCommandQueue->GetTimestampFrequency(&gpuFreq));
    m_gpuFreqInv = 1000.0 / double(gpuFreq);

    D3D12_QUERY_HEAP_DESC desc = {};
    desc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
    desc.Count = c_timerSlots;
    ThrowIfFailed(pDevice->CreateQueryHeap(&desc, IID_PPV_ARGS(&m_heap)));
    SetName(m_heap.Get(), L"GPUTimerHeap");

    // We allocate m_maxframeCount + 1 instances as an instance is guaranteed to be written to if maxPresentFrameCount frames
    // have been submitted since. This is due to a fact that Present stalls when none of the m_maxframeCount frames are done/available.
    const size_t nPerFrameInstances = m_maxframeCount + 1;
    ThrowIfFailed(pDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(nPerFrameInstances * c_timerSlots * sizeof(UINT64)),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_buffer))
    );
    SetName(m_heap.Get(), L"GPUTimerBuffer");
}

void GPUTimer::Destroy()
{
    m_heap.Reset();
    m_buffer.Reset();
}

void GPUTimer::BeginFrame(_In_ ID3D12GraphicsCommandList* pCommandList)
{
    UNREFERENCED_PARAMETER(pCommandList);
}

void GPUTimer::EndFrame(_In_ ID3D12GraphicsCommandList* pCommandList)
{
    // Resolve queries for the current frame.
    static UINT resolveToFrameID = 0;
    const UINT64 resolveToBaseAddress = resolveToFrameID * c_timerSlots * sizeof(UINT64);
    pCommandList->ResolveQueryData(m_heap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, c_timerSlots, m_buffer.Get(), resolveToBaseAddress);

    // Grab read-back data for the queries from a finished frame m_maxframeCount ago.
    const UINT readBackFrameID = (resolveToFrameID + 1) % (m_maxframeCount + 1);
    const SIZE_T readBackBaseOffset = readBackFrameID * c_timerSlots * sizeof(UINT64);
    const D3D12_RANGE dataRange =
    {
        readBackBaseOffset,
        readBackBaseOffset + c_timerSlots * sizeof(UINT64),
    };

    UINT64* timingData;
    ThrowIfFailed(m_buffer->Map(0, &dataRange, reinterpret_cast<void**>(&timingData)));
    memcpy(m_timing, timingData, sizeof(UINT64) * c_timerSlots);
    m_buffer->Unmap(0, nullptr);

    for (UINT j = 0; j < c_maxTimers; ++j)
    {
        const UINT64 start = m_timing[j * 2];
        const UINT64 end = m_timing[j * 2 + 1];
        const float value = float(double(end - start) * m_gpuFreqInv);
        m_avg[j] = UpdateRunningAverage(m_avg[j], value);
    }

    resolveToFrameID = readBackFrameID;
}

void GPUTimer::Start(_In_ ID3D12GraphicsCommandList* pCommandList, UINT timerid)
{
    if (timerid >= c_maxTimers)
    {
        throw std::out_of_range("Timer ID out of range");
    }

    pCommandList->EndQuery(m_heap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, timerid * 2);
}

void GPUTimer::Stop(_In_ ID3D12GraphicsCommandList* pCommandList, UINT timerid)
{
    if (timerid >= c_maxTimers)
    {
        throw std::out_of_range("Timer ID out of range");
    }

    pCommandList->EndQuery(m_heap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, timerid * 2 + 1);
}

void GPUTimer::ResetAverage()
{
    memset(m_avg, 0, sizeof(m_avg));
}

double GPUTimer::GetElapsedMS(UINT timerid) const
{
    if (timerid >= c_maxTimers)
    {
        return 0.0;
    }
 
    const UINT64 start = m_timing[timerid * 2];
    const UINT64 end = m_timing[timerid * 2 + 1];

    if (end < start)
    {
        return 0.0;
    }

    return double(end - start) * m_gpuFreqInv;
}