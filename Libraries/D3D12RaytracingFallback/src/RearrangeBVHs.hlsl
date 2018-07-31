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
#include "RearrangeTrianglesBindings.h"
#include "RayTracingHelper.hlsli"

RWStructuredBuffer<uint4> InputBVHBuffer : UAV_REGISTER(InputElementBufferRegister);
RWStructuredBuffer<BVHMetadata> InputBVHMetadataBuffer : UAV_REGISTER(InputMetadataBufferRegister);
RWStructuredBuffer<uint4> OutputBVHBuffer : UAV_REGISTER(OutputElementBufferRegister);
RWStructuredBuffer<BVHMetadata> OutputBVHMetadataBuffer : UAV_REGISTER(OutputMetadataBufferRegister);

void CopyBVHAABB(uint srcIndex, uint dstIndex)
{
    const uint sizeInUint4s = SizeOfAABBNodeSibling / (SizeOfUINT32 * 4);

    uint srcAddress = srcIndex * sizeInUint4s;
    uint dstAddress = dstIndex * sizeInUint4s;

    if (Constants.UpdatesAllowed)
    {
        OutputIndexBuffer[srcIndex] = dstIndex;
    }

    [unroll]
    for (uint i = 0; i < sizeInUint4s; i++)
    {
        OutputBVHBuffer[dstAddress + i] = InputBVHBuffer[srcAddress + i];
    }
}

void CopyBVHMetadata(uint srcIndex, uint dstIndex)
{
    OutputBVHMetadataBuffer[dstIndex] = InputBVHMetadataBuffer[srcIndex];
}

void CopyBVH(uint srcIndex, uint dstIndex)
{
    CopyBVHAABB(srcIndex, dstIndex);
    CopyBVHMetadata(srcIndex, dstIndex);
}

[numthreads(THREAD_GROUP_1D_WIDTH, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint dstIndex = DTid.x;
    if (dstIndex >= Constants.NumberOfTriangles) return;

    uint srcIndex = IndexBuffer[dstIndex];
    CopyBVH(srcIndex, dstIndex);
}
