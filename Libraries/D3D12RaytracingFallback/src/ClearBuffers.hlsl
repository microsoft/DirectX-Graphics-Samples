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
void main(uint3 DTid : SV_DispatchThreadID)
{
    const uint NumberOfLeafNodes = Constants.NumberOfElements;
    const uint NumberOfInternalNodes = GetNumInternalNodes(NumberOfLeafNodes);
    const uint TotalNumberOfNodes = NumberOfLeafNodes + NumberOfInternalNodes;
    const uint MaxNumberOfTreelets = NumberOfLeafNodes / FullTreeletSize;

    if (DTid.x == 0)
    {
        BaseTreeletsCountBuffer.Store(0, 0);
    }

    if (DTid.x < MaxNumberOfTreelets)
    {
        BaseTreeletsIndexBuffer[DTid.x] = TotalNumberOfNodes;
    }

    if (DTid.x < NumberOfInternalNodes)
    {
        NumTrianglesBuffer.Store(DTid.x * SizeOfUINT32, 0);
    }
}