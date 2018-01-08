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
#include "GpuTimeManager.h"
#include "GraphicsCore.h"
#include "CommandContext.h"
#include "CommandListManager.h"

namespace
{
    ID3D12QueryHeap* sm_QueryHeap = nullptr;
    ID3D12Resource* sm_ReadBackBuffer = nullptr;
    uint64_t* sm_TimeStampBuffer = nullptr;
    uint64_t sm_Fence = 0;
    uint32_t sm_MaxNumTimers = 0;
    uint32_t sm_NumTimers = 1;
    uint64_t sm_ValidTimeStart = 0;
    uint64_t sm_ValidTimeEnd = 0;
    double sm_GpuTickDelta = 0.0;
}

void GpuTimeManager::Initialize(uint32_t MaxNumTimers)
{
    uint64_t GpuFrequency;
    Graphics::g_CommandManager.GetCommandQueue()->GetTimestampFrequency(&GpuFrequency);
    sm_GpuTickDelta = 1.0 / static_cast<double>(GpuFrequency);

    D3D12_HEAP_PROPERTIES HeapProps;
    HeapProps.Type = D3D12_HEAP_TYPE_READBACK;
    HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    HeapProps.CreationNodeMask = 1;
    HeapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC BufferDesc;
    BufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    BufferDesc.Alignment = 0;
    BufferDesc.Width = sizeof(uint64_t) * MaxNumTimers * 2;
    BufferDesc.Height = 1;
    BufferDesc.DepthOrArraySize = 1;
    BufferDesc.MipLevels = 1;
    BufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    BufferDesc.SampleDesc.Count = 1;
    BufferDesc.SampleDesc.Quality = 0;
    BufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    BufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ASSERT_SUCCEEDED(Graphics::g_Device->CreateCommittedResource( &HeapProps, D3D12_HEAP_FLAG_NONE, &BufferDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr, MY_IID_PPV_ARGS(&sm_ReadBackBuffer) ));
    sm_ReadBackBuffer->SetName(L"GpuTimeStamp Buffer");

    D3D12_QUERY_HEAP_DESC QueryHeapDesc;
    QueryHeapDesc.Count = MaxNumTimers * 2;
    QueryHeapDesc.NodeMask = 1;
    QueryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
    ASSERT_SUCCEEDED(Graphics::g_Device->CreateQueryHeap(&QueryHeapDesc, MY_IID_PPV_ARGS(&sm_QueryHeap)));
    sm_QueryHeap->SetName(L"GpuTimeStamp QueryHeap");

    sm_MaxNumTimers = (uint32_t)MaxNumTimers;
}

void GpuTimeManager::Shutdown()
{
    if (sm_ReadBackBuffer != nullptr)
        sm_ReadBackBuffer->Release();

    if (sm_QueryHeap != nullptr)
        sm_QueryHeap->Release();
}

uint32_t GpuTimeManager::NewTimer(void)
{
    return sm_NumTimers++;
}

void GpuTimeManager::StartTimer(CommandContext& Context, uint32_t TimerIdx)
{
    Context.InsertTimeStamp(sm_QueryHeap, TimerIdx * 2);
}

void GpuTimeManager::StopTimer(CommandContext& Context, uint32_t TimerIdx)
{
    Context.InsertTimeStamp(sm_QueryHeap, TimerIdx * 2 + 1);
}

void GpuTimeManager::BeginReadBack(void)
{
    Graphics::g_CommandManager.WaitForFence(sm_Fence);

    D3D12_RANGE Range;
    Range.Begin = 0;
    Range.End = (sm_NumTimers * 2) * sizeof(uint64_t);
    ASSERT_SUCCEEDED(sm_ReadBackBuffer->Map(0, &Range, reinterpret_cast<void**>(&sm_TimeStampBuffer)));

    sm_ValidTimeStart = sm_TimeStampBuffer[0];
    sm_ValidTimeEnd = sm_TimeStampBuffer[1];

    // On the first frame, with random values in the timestamp query heap, we can avoid a misstart.
    if (sm_ValidTimeEnd < sm_ValidTimeStart)
    {
        sm_ValidTimeStart = 0ull;
        sm_ValidTimeEnd = 0ull;
    }
}

void GpuTimeManager::EndReadBack(void)
{
    // Unmap with an empty range to indicate nothing was written by the CPU
    D3D12_RANGE EmptyRange = {};
    sm_ReadBackBuffer->Unmap(0, &EmptyRange);
    sm_TimeStampBuffer = nullptr;

    CommandContext& Context = CommandContext::Begin();
    Context.InsertTimeStamp(sm_QueryHeap, 1);
    Context.ResolveTimeStamps(sm_ReadBackBuffer, sm_QueryHeap, sm_NumTimers * 2);
    Context.InsertTimeStamp(sm_QueryHeap, 0);
    sm_Fence = Context.Finish();
}

float GpuTimeManager::GetTime(uint32_t TimerIdx)
{
    ASSERT(sm_TimeStampBuffer != nullptr, "Time stamp readback buffer is not mapped");
    ASSERT(TimerIdx < sm_NumTimers, "Invalid GPU timer index");

    uint64_t TimeStamp1 = sm_TimeStampBuffer[TimerIdx * 2];
    uint64_t TimeStamp2 = sm_TimeStampBuffer[TimerIdx * 2 + 1];

    if (TimeStamp1 < sm_ValidTimeStart || TimeStamp2 > sm_ValidTimeEnd || TimeStamp2 <= TimeStamp1 )
        return 0.0f;

    return static_cast<float>(sm_GpuTickDelta * (TimeStamp2 - TimeStamp1));
}
