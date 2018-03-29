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
#include "GpuBvh2CopyBindings.h"
#include "RayTracingHelper.hlsli"

static const uint BytesPerLoad = 4;

[numthreads(GPU_BVH2_COPY_THREAD_GROUP_WIDTH, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint copySizeInBytes = SourceBVH.Load(OffsetToTotalSize);
    uint offsetToCopy = DTid.x * BytesPerLoad;
    while (offsetToCopy < copySizeInBytes)
    {
        DestBVH.Store(offsetToCopy, SourceBVH.Load(offsetToCopy));
        offsetToCopy += BytesPerLoad * GPU_BVH2_COPY_THREAD_GROUP_WIDTH * Constants.DispatchWidth;
    }
}
