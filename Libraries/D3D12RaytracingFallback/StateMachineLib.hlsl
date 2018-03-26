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
#define     HLSL
#include "WaveDimensions.h"
#include "UberShaderBindings.h"
#include "DebugLog.hlsli" // Needs to get include somewhere

void Fallback_Scheduler(int initialStateId, uint dimx, uint dimy);

[numthreads(THREAD_GROUP_WIDTH, THREAD_GROUP_HEIGHT, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint GI : SV_GroupIndex)
{
  int stateId = RayGenShaderTable.Load(0);
  Fallback_Scheduler(stateId, RayDispatchDimensionsWidth, RayDispatchDimensionsHeight);
}
