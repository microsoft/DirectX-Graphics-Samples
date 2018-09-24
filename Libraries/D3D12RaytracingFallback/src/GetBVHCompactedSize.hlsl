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
#include "RayTracingHelper.hlsli"
#include "GetBVHCompactedSizeBindings.h"

uint GetBVHSize(RWByteAddressBuffer bvh)
{
    return bvh.Load(OffsetToTotalSize);
}

#define GetBVHSize(ID) case ID: size =  GetBVHSize(BVH##ID);  break

[numthreads(THREAD_GROUP_1D_WIDTH, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    if (DTid.x >= Constants.NumberOfBoundBVHs) return;

    uint outputOffset = DTid.x * 4;
    uint size = 0;
    switch (DTid.x + 1)
    {
        GetBVHSize(1);
        GetBVHSize(2);
        GetBVHSize(3);
        GetBVHSize(4);
        GetBVHSize(5);
        GetBVHSize(6);
        GetBVHSize(7);
        GetBVHSize(8);
        GetBVHSize(9);
        GetBVHSize(10);
        GetBVHSize(11);
        GetBVHSize(12);
        GetBVHSize(13);
        GetBVHSize(14);
        GetBVHSize(15);
        GetBVHSize(16);
        GetBVHSize(17);
        GetBVHSize(18);
        GetBVHSize(19);
        GetBVHSize(20);
        GetBVHSize(21);
        GetBVHSize(22);
        GetBVHSize(23);
        GetBVHSize(24);
        GetBVHSize(25);
        GetBVHSize(26);
        GetBVHSize(27);
        GetBVHSize(28);
        GetBVHSize(29);
        GetBVHSize(30);
    }
    OutputCount.Store(outputOffset, size);
}
