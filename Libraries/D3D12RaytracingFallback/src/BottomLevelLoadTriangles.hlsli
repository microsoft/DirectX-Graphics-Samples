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
#include "LoadPrimitivesBindings.h"
#include "RayTracingHelper.hlsli"

uint3 GetUint32Index3(ByteAddressBuffer IndexBuffer, uint IndexBufferOffset, uint index)
{
    const uint stride = SizeOfUINT32;
    uint address = index * stride + Constants.IndexBufferOffset;
    return IndexBuffer.Load3(address);
}

uint3 GetUint16Index3(ByteAddressBuffer IndexBuffer, uint IndexBufferOffset, uint index)
{
    // RWByteAddress's load uint32 data by default. Need to convert
    // to a UINT16 index
    uint dataOffset = 0;
    const uint stride = SizeOfUINT16;
    uint address = index * stride + Constants.IndexBufferOffset;
    if (address % 4 == 2)
    {
        address -= 2;
        dataOffset = 1;
    }

    uint2 uint32Data = IndexBuffer.Load2(address);
    uint3 uint16Result;
    for (uint i = 0; i < 3; i++)
    {
        uint uint16IndexToRead = i + dataOffset;
        uint uint32IndexToRead = uint16IndexToRead / 2;
        uint uint32Word = uint32Data[uint32IndexToRead];
        if (uint16IndexToRead % 2 == 0)
        {
            uint16Result[i] = uint32Word & 0xffff;
        }
        else
        {
            uint16Result[i] = uint32Word >> 16;
        }
    }

    return uint16Result;
}

uint3 GetIndexNoIndexBuffer(uint startTriangleIndex)
{
    return uint3(startTriangleIndex, startTriangleIndex + 1, startTriangleIndex + 2);
}

uint3 GetIndex(uint threadIndex)
{
    uint triangleIndex = threadIndex * NumberOfVerticesPerTriangle;

    uint3 indexData;
#ifdef NO_INDEX_BUFFER
    indexData = GetIndexNoIndexBuffer(triangleIndex);
#endif
#ifdef INDEX_BUFFER_32_BIT
    indexData = GetUint32Index3(indexBuffer, Constants.IndexBufferOffset, triangleIndex);
#endif
#ifdef INDEX_BUFFER_16_BIT
    indexData = GetUint16Index3(indexBuffer, Constants.IndexBufferOffset, triangleIndex);
#endif

    return indexData;
}

float3 GetVertex(ByteAddressBuffer VertexBuffer, uint index, uint stride)
{
    return asfloat(VertexBuffer.Load3(index * stride));
}

float3 TransformVertex(float3 v, float3x4 transform)
{
    return mul(transform, float4(v, 1));
}

[numthreads(THREAD_GROUP_1D_WIDTH, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{ 
    uint localTriangleIndex = DTid.x;
    if (localTriangleIndex >= Constants.NumPrimitivesBound)
    {
        return;
    }

    uint NumberOfInternalNodes = Constants.TotalPrimitiveCount - 1;
    uint NumberOfAABBs = NumberOfInternalNodes + Constants.TotalPrimitiveCount;

    uint3 indicies = GetIndex(localTriangleIndex);

    Triangle tri;
    tri.v0 = GetVertex(elementBuffer, indicies[0], Constants.ElementBufferStride);
    tri.v1 = GetVertex(elementBuffer, indicies[1], Constants.ElementBufferStride);
    tri.v2 = GetVertex(elementBuffer, indicies[2], Constants.ElementBufferStride);
    
    if (Constants.HasValidTransform)
    {
        // The Dxil compiler has some issues handling row_major float3x4's
        // so manually pulling it out as a series of float4's
        float3x4 transform;
        transform[0] = TransformBuffer[0];
        transform[1] = TransformBuffer[1];
        transform[2] = TransformBuffer[2];

        tri.v0 = TransformVertex(tri.v0, transform);
        tri.v1 = TransformVertex(tri.v1, transform);
        tri.v2 = TransformVertex(tri.v2, transform);
    }

    uint globalTriangleIndex = localTriangleIndex + Constants.PrimitiveOffset;
    uint outputIndex = GetOutputIndex(globalTriangleIndex);
    
    PrimitiveBuffer[outputIndex] = CreateTrianglePrimitive(tri);
    StorePrimitiveMetadata(outputIndex, localTriangleIndex);
}
