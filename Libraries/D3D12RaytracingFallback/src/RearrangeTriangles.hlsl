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
RWStructuredBuffer<Primitive> InputTriangleBuffer : UAV_REGISTER(InputElementBufferRegister);
RWStructuredBuffer<Primitive> OutputTriangleBuffer : UAV_REGISTER(OutputElementBufferRegister);

RWStructuredBuffer<PrimitiveMetaData> InputMetadataBuffer : UAV_REGISTER(InputMetadataBufferRegister);
RWStructuredBuffer<PrimitiveMetaData> OutputMetadataBuffer : UAV_REGISTER(OutputMetadataBufferRegister);

void CopyPrimitive(uint srcIndex, uint dstIndex)
{
    OutputTriangleBuffer[dstIndex] = InputTriangleBuffer[srcIndex];
    OutputMetadataBuffer[dstIndex] = InputMetadataBuffer[srcIndex];
    if (Constants.UpdatesAllowed)
    {
        OutputIndexBuffer[srcIndex] = dstIndex;
    }
}

[numthreads(THREAD_GROUP_1D_WIDTH, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint dstIndex = DTid.x;
    if (dstIndex >= Constants.NumberOfTriangles) return;
    
    uint srcIndex = IndexBuffer[dstIndex];
    CopyPrimitive(srcIndex, dstIndex);
}