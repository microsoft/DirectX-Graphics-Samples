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
#include "DebugLog.h"

static const uint EntryStartOffset = 1; // First entry used as a header for the entry count
void Log(uint4 LogEntry)
{
#if ENABLE_UAV_LOG
    if (DispatchRaysIndex().x == LOG_RAY_INDEX_X && DispatchRaysIndex().y == LOG_RAY_INDEX_Y)
    {
        int count = DebugLog[0].x;
        DebugLog[count + EntryStartOffset] = LogEntry;
        DebugLog[0].x = count + 1;
    }
#endif
}

void LogNoData(uint EntryType)
{
    Log(uint4(EntryType, 0, 0, 0));
}

export void LogTraceRayStart()
{
    LogNoData(TRACE_RAY_BEGIN);
}

export void LogTraceRayEnd()
{
    LogNoData(TRACE_RAY_END);
}

export void LogInt(int val)
{
    Log(uint4(LOG_INT, val, 0, 0));
}

void LogInt2(int2 val)
{
  Log(uint4(LOG_INT2, val, 0));
}

void LogInt3(int3 val)
{
  Log(uint4(LOG_INT3, val));
}

void LogFloat(float val)
{
    Log(uint4(LOG_FLOAT, asuint(val), 0, 0));
}

void LogFloat3(float3 val)
{
  Log(uint4(LOG_FLOAT3, asuint(val)));
}
