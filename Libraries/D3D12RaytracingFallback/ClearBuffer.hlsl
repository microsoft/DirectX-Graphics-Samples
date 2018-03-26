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
#define HLSL
#include "TreeletReorderBindings.h"
#include "RayTracingHelper.hlsli"

[numthreads(THREAD_GROUP_1D_WIDTH, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    const uint NumberOfInternalNodes = GetNumInternalNodes(Constants.NumberOfElements);
    if (DTid.x >= NumberOfInternalNodes) return;

    NumTrianglesBuffer.Store(DTid.x * SizeOfUINT32, 0);
}
