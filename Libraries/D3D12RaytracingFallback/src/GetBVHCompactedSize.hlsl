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

#define GET_BVH_SIZE(ID) case ID: size =  GetBVHSize(BVH##ID);  break

[numthreads(THREAD_GROUP_1D_WIDTH, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    if (DTid.x >= Constants.NumberOfBoundBVHs) return;

    uint outputOffset = DTid.x * 4;
    uint size = 0;
    switch (DTid.x + 1)
    {
        GET_BVH_SIZE(1);
        GET_BVH_SIZE(2);
        GET_BVH_SIZE(3);
        GET_BVH_SIZE(4);
        GET_BVH_SIZE(5);
        GET_BVH_SIZE(6);
        GET_BVH_SIZE(7);
        GET_BVH_SIZE(8);
        GET_BVH_SIZE(9);
        GET_BVH_SIZE(10);
        GET_BVH_SIZE(11);
        GET_BVH_SIZE(12);
        GET_BVH_SIZE(13);
        GET_BVH_SIZE(14);
        GET_BVH_SIZE(15);
        GET_BVH_SIZE(16);
        GET_BVH_SIZE(17);
        GET_BVH_SIZE(18);
        GET_BVH_SIZE(19);
        GET_BVH_SIZE(20);
        GET_BVH_SIZE(21);
        GET_BVH_SIZE(22);
        GET_BVH_SIZE(23);
        GET_BVH_SIZE(24);
        GET_BVH_SIZE(25);
        GET_BVH_SIZE(26);
        GET_BVH_SIZE(27);
        GET_BVH_SIZE(28);
        GET_BVH_SIZE(29);
        GET_BVH_SIZE(30);
    }
    OutputCount.Store(outputOffset, size);
}
