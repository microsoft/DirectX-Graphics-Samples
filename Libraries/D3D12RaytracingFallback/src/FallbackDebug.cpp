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
#include "pch.h"
#include "DebugLog.h"
#include <string>

#if ENABLE_UAV_LOG
void OutputDebugLog(ID3D12RaytracingFallbackDevice *pDevice)
{
    FallbackLayer::RaytracingDevice &device = *(FallbackLayer::RaytracingDevice *)pDevice;

    FallbackLayer::LogEntry *pEntries;
    device.GetUAVDebugLogReadbackHeap().Map(0, nullptr, (void **)&pEntries);
    UINT entryCount = pEntries[0].EntryType;

    pEntries++;
    for (UINT i = 0; i < entryCount; i++)
    {
        switch (pEntries[i].EntryType)
        {
        case TRACE_RAY_BEGIN:
            OutputDebugString(L"Entered trace ray!\n");
            break;
        case TRACE_RAY_END:
            OutputDebugString(L"Exiting trace ray!\n");
            break;
        case LOG_INT:
            OutputDebugString((L"int: " + std::to_wstring(pEntries[i].ValueA) + L"\n").c_str());
            break;
        case LOG_INT2:
            OutputDebugString((L"int2: (" + std::to_wstring(pEntries[i].ValueA)
                                  + L", " + std::to_wstring(pEntries[i].ValueB) + L")\n").c_str());
            break;
        case LOG_INT3:
            OutputDebugString((L"int3: (" + std::to_wstring(pEntries[i].ValueA)
                                  + L", " + std::to_wstring(pEntries[i].ValueB)
                                  + L", " + std::to_wstring(pEntries[i].ValueC) + L")\n").c_str());
            break;
        case LOG_FLOAT:
            OutputDebugString((L"float: " + std::to_wstring(*(float*)&pEntries[i].ValueA) + L"\n").c_str());
            break;
        case LOG_FLOAT3:
            OutputDebugString((L"float3: (" + std::to_wstring(*(float*)&pEntries[i].ValueA) 
                                    + L", " + std::to_wstring(*(float*)&pEntries[i].ValueB)
                                    + L", " + std::to_wstring(*(float*)&pEntries[i].ValueC) + L")\n").c_str());         
            break;
        }
    }
}
#endif

#if ENABLE_ACCELERATION_STRUCTURE_VISUALIZATION
void VisualizeAccelerationStructureLevel(ID3D12RaytracingFallbackDevice *pDevice, UINT level)
{
    FallbackLayer::RaytracingDevice &device = *(FallbackLayer::RaytracingDevice *)pDevice;
    device.VisualizeAccelerationStructureLevel(level);
}
#endif
