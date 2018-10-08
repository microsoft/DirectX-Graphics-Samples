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
#ifdef SCALED_MORTON_CODES
uint GetMortonCodesFromUnitCoord(float3 unitCoord, uint3 numPerIteration)
{
    uint bits = 0;
    const uint bitsPerIteration = numPerIteration.x + numPerIteration.y + numPerIteration.z;
    const uint numIterations = 32 / bitsPerIteration;
    const uint3 bitsPerAxis = numPerIteration * numIterations;

    unsigned int mortonCode = 0;
    const unsigned uint3 maxCoord = pow(2, bitsPerAxis);

    const float3 adjustedCoord = min(max(unitCoord * maxCoord, 0.0f), maxCoord - 1);
    const unsigned int numAxis = 3;
    const uint3 coords = adjustedCoord;

    uint bitsUsed = 0;
    uint3 bitsUsedPerAxis = 0;
    const uint cylesPerIteration = max(numPerIteration.x, max(numPerIteration.y, numPerIteration.z));

    for (uint iterations = 0; iterations < numIterations; iterations++)
    {
        for (uint i = 0; i < cylesPerIteration; i++)
        {
            for (uint axis = 0; axis < numAxis; axis++)
            {
                if (i < numPerIteration[axis])
                {
                    const uint bit = BIT(bitsUsedPerAxis[axis]) & coords[axis];
                    if (bit)
                    {
                        mortonCode |= BIT(bitsUsed);
                    }
                    bitsUsedPerAxis[axis]++;
                    bitsUsed++;
                }
            }
        }
    }
    return mortonCode;
}

uint CalculateMortonCode(float3 elementCentroid)
{
    const float epsilon = 0.00001;

    AABB sceneAABB = GetSceneAABB();
    float3 dim = sceneAABB.max - sceneAABB.min;
    uint3 numPerIteration = 0;

    uint smallestIndex = (dim.x < dim.y && dim.x < dim.z) ? 0 : (dim.y < dim.z) ? 1 : 2;
    uint index1 = (smallestIndex + 1) % 3;
    uint index2 = (smallestIndex + 2) % 3;
    numPerIteration[smallestIndex] = 1;
    numPerIteration[index1] = min(uint(dim[index1] / dim[smallestIndex]), 8);
    numPerIteration[index2] = min(uint(dim[index2] / dim[smallestIndex]), 8);

    float3 sceneDimension = max(sceneAABB.max - sceneAABB.min, epsilon);
    float3 unitCoord = (elementCentroid - sceneAABB.min) / sceneDimension;

    return GetMortonCodesFromUnitCoord(unitCoord, numPerIteration);
}
#else
uint GetMortonCodesFromUnitCoord(float3 unitCoord)
{
    unsigned int mortonCode = 0;
    const unsigned int numBits = 10;
    unsigned int maxCoord = pow(2, numBits);

    float3 adjustedCoord = min(max(unitCoord * maxCoord, 0.0f), maxCoord - 1);
    const unsigned int numAxis = 3;
    uint coords[numAxis] = { adjustedCoord.y, adjustedCoord.x, adjustedCoord.z };
    for (uint bitIndex = 0; bitIndex < numBits; bitIndex++)
    {
        for (uint axis = 0; axis < numAxis; axis++)
        {
            uint bit = BIT(bitIndex) & coords[axis];
            if (bit)
            {
                mortonCode |= BIT(bitIndex * numAxis + axis);
            }
        }
    }
    return mortonCode;
}

uint CalculateMortonCode(float3 elementCentroid)
{
    const float epsilon = 0.00001;

    AABB sceneAABB = GetSceneAABB();
    float3 sceneDimension = max(sceneAABB.max - sceneAABB.min, epsilon);
    float3 unitCoord = (elementCentroid - sceneAABB.min) / sceneDimension;

    return GetMortonCodesFromUnitCoord(unitCoord);
}
#endif

[numthreads(THREAD_GROUP_1D_WIDTH, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint elementIndex = DTid.x;
    if (elementIndex >= Constants.NumberOfElements) return;

    float3 elementCentroid = GetCentroid(elementIndex);
    uint mortonCode = CalculateMortonCode(elementCentroid);

    OutputMortonCodesBuffer[elementIndex] = mortonCode;
    OutputIndicesBuffer[elementIndex] = elementIndex;
}