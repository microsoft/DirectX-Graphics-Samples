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
RWStructuredBuffer<Triangle> InputTriangleBuffer : UAV_REGISTER(InputElementBufferRegister);
RWStructuredBuffer<Triangle> OutputTriangleBuffer : UAV_REGISTER(OutputElementBufferRegister);

RWStructuredBuffer<TriangleMetaData> InputMetadataBuffer : UAV_REGISTER(InputMetadataBufferRegister);
RWStructuredBuffer<TriangleMetaData> OutputMetadataBuffer : UAV_REGISTER(OutputMetadataBufferRegister);

void CopyTriangle(uint srcIndex, uint dstIndex)
{
    OutputTriangleBuffer[dstIndex] = InputTriangleBuffer[srcIndex];
    OutputMetadataBuffer[dstIndex] = InputMetadataBuffer[srcIndex];
}

[numthreads(THREAD_GROUP_1D_WIDTH, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint dstIndex = DTid.x;
    if (dstIndex >= NumberOfTriangles) return;
    
    uint srcIndex = IndexBuffer[dstIndex];
    CopyTriangle(srcIndex, dstIndex);
}
