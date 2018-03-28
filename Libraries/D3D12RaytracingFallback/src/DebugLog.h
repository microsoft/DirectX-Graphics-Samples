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
#pragma once
#include "FallbackDebug.h"

#define LOG_RAY_INDEX_X 10
#define LOG_RAY_INDEX_Y 15

// Add entries here:
#define TRACE_RAY_BEGIN   0
#define TRACE_RAY_END     1
#define LOG_INT           2
#define LOG_INT2          3
#define LOG_INT3          4
#define LOG_FLOAT         5
#define LOG_FLOAT3        6

#ifdef HLSL
void BeginLog();
void LogInt(int val);
void LogInt2(int2 val);
void LogInt3(int3 val);
void LogFloat(float val);
void LogFloat3(float3 val);
void LogTraceRayStart();
void LogTraceRayEnd();
#endif // HLSL
