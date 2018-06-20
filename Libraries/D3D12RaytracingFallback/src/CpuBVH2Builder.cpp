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
#include "pch.h"

namespace FallbackLayer
{
    struct BVH
    {
        std::vector<AABBNode>   m_nodes;
        std::vector<float> m_triangles;
        std::vector<PrimitiveMetaData> m_metadata;
    };

    static
        void AddExtentToBox(
            AABB& box,
            const AABB& extent)
    {
        box.min.x = std::min(box.min.x, extent.min.x);
        box.max.x = std::max(box.max.x, extent.max.x);

        box.min.y = std::min(box.min.y, extent.min.y);
        box.max.y = std::max(box.max.y, extent.max.y);

        box.min.z = std::min(box.min.z, extent.min.z);
        box.max.z = std::max(box.max.z, extent.max.z);
    }

    UINT UnpackTriangleIndex(
        UINT v)
    {
        return v & 0x00ffffff;
    }

    static
        void ComputeBox(
            AABB& overallBox,
            const std::vector<AABB>& boxes,
            const std::vector<PrimitiveMetaData>& metadata)
    {
        if (metadata.empty())
        {
            overallBox.max.x = overallBox.min.x = 0;
            overallBox.max.y = overallBox.min.y = 0;
            overallBox.max.z = overallBox.min.z = 0;
            return;
        }

        overallBox = boxes[metadata[0].PrimitiveIndex];

        const UINT32 numTris = (UINT32)metadata.size();
        for (UINT32 i = 1; i < numTris; ++i)
        {
            const UINT32 triId = metadata[i].PrimitiveIndex;
            assert(triId < boxes.size());
            const AABB& newBox = boxes[triId];

            AddExtentToBox(overallBox, newBox);
        }
    }

    //
    // Convert a 16-bit float to 32-bit.
    //

    static
        float Fp16ToFp32(USHORT v)
    {
        static const UINT kMultiple = 0x77800000;   // 2**112
        const UINT BiasedFloat = (v & 0x8000) << 16 | (v & 0x7FFF) << 13;
        return (float&)BiasedFloat * (float&)kMultiple;
    }

    //
    // Round toward minus infinity.  Guarantees output <= input.
    //

    static
        USHORT Fp32ToFp16(float v, float RoundDirection = 0.0f)
    {
        assert(!!_finite(v));
        assert(v > -65504 && v < 65504);

        // Multiplying by 2^-112 causes exponents below -14 to denormalize
        static const UINT kMultiple = 0x07800000;   // 2**-112
        const float BiasedFloat = v * (float&)kMultiple;
        const UINT u = (UINT&)BiasedFloat;

        const UINT sign = u & 0x80000000;
        UINT body = u & 0x0fffffff;

        // Increase the magnitude before truncation to ensure proper bounds
        if (v * RoundDirection > 0.0f)
        {
            if (body == 0)
                body = 0x800000;
            else
                body += 0x1fff;
        }

        return (USHORT)(sign >> 16 | body >> 13);
    }

    static
        float QuantizeToFp16(
            float v)
    {
        return v;
    }

    static
        UINT32 BuildBVHAddNode(
            BVH& bvh,
            const AABB& box,
            UINT32 maxDimension)
    {
        UNREFERENCED_PARAMETER(maxDimension);
        assert(maxDimension < 3);
        const UINT32 nodeIndex = (UINT32)bvh.m_nodes.size();

        float cX = (box.max.x + box.min.x) * 0.5f;
        float cY = (box.max.y + box.min.y) * 0.5f;
        float cZ = (box.max.z + box.min.z) * 0.5f;
        cX = QuantizeToFp16(cX);
        cY = QuantizeToFp16(cY);
        cZ = QuantizeToFp16(cZ);

        float dX = max(box.max.x - cX, cX - box.min.x);
        float dY = max(box.max.y - cY, cY - box.min.y);
        float dZ = max(box.max.z - cZ, cZ - box.min.z);

        AABBNode packedBox;
        packedBox.center[0] = cX;
        packedBox.center[1] = cY;
        packedBox.center[2] = cZ;
        packedBox.halfDim[0] = dX;
        packedBox.halfDim[1] = dY;
        packedBox.halfDim[2] = dZ;
        packedBox.nodeAllBits = 0;

        bvh.m_nodes.push_back(packedBox);

        assert(bvh.m_nodes.size() - 1 == nodeIndex);

        bvh.m_nodes[nodeIndex].internalNode.separatingAxis = 0;

        return nodeIndex;
    }

    static
        UINT32 BuildBVHAddLeaf(
            BVH& bvh,
            const AABB& box,
            const std::vector<PrimitiveMetaData>& metadata)
    {
        const UINT32 nodeIndex = BuildBVHAddNode(bvh, box, 0);

        bvh.m_nodes[nodeIndex].nodeAllBits = 0;
        bvh.m_nodes[nodeIndex].leaf = true;

        const UINT32 idIndex = (UINT32)bvh.m_metadata.size();

        std::copy(metadata.begin(), metadata.end(), std::back_inserter(bvh.m_metadata));

        assert(metadata.size() < 128);
        assert(idIndex < (1 << 24));

        bvh.m_nodes[nodeIndex].leafNode.firstTriangleId = idIndex;
        bvh.m_nodes[nodeIndex].leafNode.numTriangleIds = (UINT32)metadata.size();

        return nodeIndex;
    }

    //
    // Sort min to max by centroid
    //

    static
        void SortByCentroid(
            std::vector<PrimitiveMetaData>& metadata,
            const std::vector<AABB>& boxes,
            UINT32 maxDimension)
    {
        struct TriPosition
        {
            float   pos;
            UINT32  id;
        };

        std::vector<TriPosition> sortTris(metadata.size());

        for (UINT32 i = 0; i < metadata.size(); ++i)
        {
            const UINT32 triId = metadata[i].PrimitiveIndex;
            const AABB& box = boxes[triId];

            const float boxCenter = (box.maxArr[maxDimension] + box.minArr[maxDimension]) / 2;

            sortTris[i].pos = boxCenter;
            sortTris[i].id = metadata[i].PrimitiveIndex;
        }

        // Split the list into left and right sublists
        std::sort(sortTris.begin(), sortTris.end(), [](auto&& a, auto&& b) -> bool { return a.pos < b.pos; });

        // Update the output
        for (UINT32 i = 0; i < metadata.size(); ++i)
        {
            metadata[i].PrimitiveIndex = sortTris[i].id;
        }
    }


    static
        float ComputeBoxSurfaceArea(
            const AABB& box)
    {
        const float dims[3] =
        {
            box.max.x - box.min.x,
            box.max.y - box.min.y,
            box.max.z - box.min.z
        };

        return 2 * (dims[0] * dims[1] + dims[0] * dims[2] + dims[1] * dims[2]);
    }

    static
        void InitBoxToInverseMax(
            AABB& box)
    {
        box.max.x = box.max.y = box.max.z = -10e10f;//FLT_MAX;
        box.min.x = box.min.y = box.min.z = 10e10f;//FLT_MAX;
    }

    //
    // A feeble attempt at a SAH builder
    //

    static
        void SahSplit(
            std::vector<PrimitiveMetaData>& metadata,
            UINT32& maxDimension,
            UINT32& numTrisInLeftNode,
            const AABB& nodeBox,
            const std::vector<AABB>& boxes)
    {
        static const UINT NUM_SAH_BINS = 64;

        struct SahBin
        {
            AABB    box;
            UINT    numTriangles;
        };

        // NOTE: use vector if this blows out the stack?
        SahBin  sahBins[3][NUM_SAH_BINS];

        // For the score to be meaningful it seems we need to normalize it to something
        const float normalizeToParent = 1.f / ComputeBoxSurfaceArea(nodeBox);

        const UINT numTris = (UINT)metadata.size();

        float bestSah = FLT_MAX;
        maxDimension = 0;
        //numTrisInLeftNode = triangleIds.size() / 2;

        // Compute SAH score per axis
        for (UINT i = 0; i < 3; ++i)
        {
            const float extents = nodeBox.maxArr[i] - nodeBox.minArr[i];
            if (extents == 0)
                continue;

            const float rangeMin = nodeBox.minArr[i];

            const float inverseExtents = 1.f / extents;

            // Init boxes
            for (UINT j = 0; j < NUM_SAH_BINS; ++j)
            {
                sahBins[i][j].numTriangles = 0;
                InitBoxToInverseMax(sahBins[i][j].box);
            }

            // Place triangles into the buckets
            for (UINT j = 0; j < metadata.size(); ++j)
            {
                const UINT triId = metadata[j].PrimitiveIndex;

                const AABB& triBox = boxes[triId];

                const float centroid = (triBox.maxArr[i] + triBox.minArr[i]) * 0.5f;

                const UINT binIndex = std::min(NUM_SAH_BINS - 1,
                    UINT(NUM_SAH_BINS * ((centroid - rangeMin) * inverseExtents)));

                sahBins[i][binIndex].numTriangles++;
                AddExtentToBox(sahBins[i][binIndex].box, triBox);
            }

            // Make sure we caught all of them once
            UINT testTris = 0;
            for (UINT j = 0; j < NUM_SAH_BINS; ++j)
            {
                testTris += sahBins[i][j].numTriangles;
            }
            assert(testTris == numTris);

            // Precompute left and right boxes with counts to be able to test plane positionings

            AABB leftBoxes[NUM_SAH_BINS];
            AABB rightBoxes[NUM_SAH_BINS];

            for (UINT j = 0; j < NUM_SAH_BINS; ++j)
            {
                const UINT rightIdx = NUM_SAH_BINS - j - 1;

                rightBoxes[rightIdx] = sahBins[i][rightIdx].box;
                leftBoxes[j] = sahBins[i][j].box;

                if (j > 0)
                {
                    AddExtentToBox(leftBoxes[j], leftBoxes[j - 1]);
                    AddExtentToBox(rightBoxes[rightIdx], rightBoxes[rightIdx + 1]);
                }
            }

            UINT numTrianglesOnLeft = 0;
            UINT numTrianglesOnRight = numTris;

            // Find the plane with the best score
            for (UINT j = 0; j < NUM_SAH_BINS - 1; ++j)
            {
                if (!sahBins[i][j].numTriangles)
                {
                    continue;
                }

                numTrianglesOnLeft += sahBins[i][j].numTriangles;
                numTrianglesOnRight -= sahBins[i][j].numTriangles;

                const float sah = (numTrianglesOnLeft * ComputeBoxSurfaceArea(leftBoxes[j]) +
                    numTrianglesOnRight * ComputeBoxSurfaceArea(rightBoxes[j + 1])) *
                    normalizeToParent;

                assert(!_isnan(sah));

                if (sah < bestSah)
                {
                    bestSah = sah;
                    maxDimension = i;
                    numTrisInLeftNode = numTrianglesOnLeft;
                }
            }
        }

        //
        // Split the set to try to get a balanced tree
        //

        SortByCentroid(metadata, boxes, maxDimension);
    }

    //
    // It's a good idea to do a breadth-first build because then nodes from the same level
    // get adjacent memory locations. It does take a lot of memory though.
    //
    // "Uniform BVH"
    // -- both children are valid for all internal nodes
    // -- left child's index is +1 of the parent index, right child's index is stored
    //    in the packed AABB structure.
    // -- there could be a varaible number of triangles in leaves
    //
    static
        void BuildBVH(
            BVH& bvh,
            const std::vector<AABB>& boxes,
            const std::vector<PrimitiveMetaData>& primitiveMetaData,
            UINT32 maxTrisInLeaf)
    {
        //
        // These are huge so use pointers
        //
        struct StackItem
        {
            std::vector<PrimitiveMetaData> primitiveMetaData;
            UINT32              parentIndex;
            UINT                right : 1;
            UINT                axis : 2;
        };

        std::deque<StackItem*>   fifoLefts;
        std::deque<StackItem*>   fifoRights;

        StackItem* temp = new StackItem;
        temp->parentIndex = (UINT)-1;
        temp->primitiveMetaData = primitiveMetaData;
        temp->right = false;
        temp->axis = 0;
        fifoRights.push_back(temp);

        while (!fifoLefts.empty() || !fifoRights.empty())
        {
            // Uniform BVH pops the first left node
            StackItem* item = nullptr;

            if (!fifoRights.empty())
            {
                item = fifoRights.back();
                fifoRights.pop_back();
            }

            if (!item)
            {
                item = fifoLefts.back();
                fifoLefts.pop_back();
            }

            //
            // Compute overall bounding box
            //
            AABB nodeBox;
            ComputeBox(nodeBox, boxes, item->primitiveMetaData);

            const UINT32 numTrianglesInNode = (UINT32)item->primitiveMetaData.size();
            const UINT32 parentIndex = item->parentIndex;

            UINT32 thisNodeIndex;

            // Leaf or internal node?
            if (numTrianglesInNode <= maxTrisInLeaf)
            {
                thisNodeIndex = BuildBVHAddLeaf(bvh, nodeBox, item->primitiveMetaData);
            }
            else
            {
                //
                // Find separating plane. Use Median for speed.
                // SAH is better but also more expensive to build.
                //

                UINT splitDimension;
                UINT leftChildNumNodes;

                SahSplit(item->primitiveMetaData,
                    splitDimension,
                    leftChildNumNodes,
                    nodeBox,
                    boxes);

                assert(leftChildNumNodes <= item->primitiveMetaData.size());

                // Try to balance by using the median if SAH failed
                if ((leftChildNumNodes == 0 ||
                    leftChildNumNodes == item->primitiveMetaData.size()) &&
                    item->primitiveMetaData.size() > MAX_TRIS_IN_LEAF)
                {
                    leftChildNumNodes = (UINT)item->primitiveMetaData.size() / 2;
                }

                const UINT32 rightChildNumNodes = (UINT32)item->primitiveMetaData.size() - leftChildNumNodes;


                //
                // "Recurse"
                //

                thisNodeIndex = BuildBVHAddNode(bvh, nodeBox, splitDimension);

                StackItem* leftItem = new StackItem;
                leftItem->parentIndex = thisNodeIndex;
                leftItem->primitiveMetaData.resize(leftChildNumNodes);
                leftItem->right = false;
                leftItem->axis = splitDimension;
                for (UINT32 i = 0; i < leftChildNumNodes; ++i)
                {
                    leftItem->primitiveMetaData[i] = item->primitiveMetaData[i];
                }

                StackItem* rightItem = new StackItem;
                rightItem->parentIndex = thisNodeIndex;
                rightItem->primitiveMetaData.resize(rightChildNumNodes);
                rightItem->right = true;
                rightItem->axis = splitDimension;
                for (UINT32 i = 0; i < rightChildNumNodes; ++i)
                {
                    rightItem->primitiveMetaData[i] = item->primitiveMetaData[i + leftChildNumNodes];
                }

                fifoLefts.push_back(leftItem);
                fifoRights.push_back(rightItem);
            }

            // Update child link of the parent
            if (parentIndex != -1)
            {
                if (!item->right)
                {
                    bvh.m_nodes[parentIndex].internalNode.leftNodeIndex = thisNodeIndex;
                    bvh.m_nodes[parentIndex].rightNodeIndex = parentIndex + 1;
                }
            }

            // Free the item after use
            delete item;
        }
    }

    void BuildUniformBVH(
        _In_  UINT NumElements,
        _In_reads_opt_(NumElements)  const D3D12_RAYTRACING_GEOMETRY_DESC *pGeometries,
        BVH &bvh)
    {
        using namespace DirectX;
        //
        // Compute number of triangles
        //

        UINT    totalNumberOfTriangles = 0;

        for (UINT i = 0; i < NumElements; ++i)
        {
            auto &geometry = pGeometries[i];
            if (geometry.Type == D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES)
            {
                totalNumberOfTriangles += GetPrimitiveCountFromGeometryDesc(geometry);
            }
            else
            {
                throw - 1; // Intersection shaders not supported yet
            }

        }

        //
        // Create AABBs
        //

        std::vector<AABB> boxes;
        boxes.resize(totalNumberOfTriangles);

        std::vector<PrimitiveMetaData> primitiveMetaData;
        primitiveMetaData.resize(totalNumberOfTriangles);

        std::vector<float>  triangleVertices;
        triangleVertices.resize(totalNumberOfTriangles * 9);

        UINT triangleIndex = 0;
        for (UINT i = 0; i < NumElements; ++i)
        {
            auto &geometry = pGeometries[i];
            if (geometry.Type != D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES)
            {
                throw - 1; // Intersection shaders not supported yet
            }

            auto &triangles = geometry.Triangles;
            if (GetPrimitiveCountFromGeometryDesc(geometry) == 0)

            {
                continue;
            }

            // 
            const UINT64 vertexStrideDwords = triangles.VertexBuffer.StrideInBytes / 4;
            const UINT numTris = GetPrimitiveCountFromGeometryDesc(geometry);

            float *pVertexData = (float *)geometry.Triangles.VertexBuffer.StartAddress;
            float *pIndexData = (float *)geometry.Triangles.IndexBuffer;

            const float* pVertices = (float*)(pVertexData);
            const UINT16* pIndices = (UINT16*)(pIndexData);

            for (UINT j = 0; j < numTris; ++j)
            {
                const UINT16 i0 = pIndices[j * 3 + 0];
                const UINT16 i1 = pIndices[j * 3 + 1];
                const UINT16 i2 = pIndices[j * 3 + 2];

                const float* v0 = &pVertices[i0 * vertexStrideDwords];
                const float* v1 = &pVertices[i1 * vertexStrideDwords];
                const float* v2 = &pVertices[i2 * vertexStrideDwords];

                float* pTriVerts = &triangleVertices[triangleIndex * 9];

                pTriVerts[0] = v0[0];
                pTriVerts[1] = v0[1];
                pTriVerts[2] = v0[2];

                pTriVerts[3] = v1[0];
                pTriVerts[4] = v1[1];
                pTriVerts[5] = v1[2];

                pTriVerts[6] = v2[0];
                pTriVerts[7] = v2[1];
                pTriVerts[8] = v2[2];

                AABB& box = boxes[triangleIndex];
                for (UINT k = 0; k < 3; ++k)
                {
#define AABB_Min_Padding 0.001f
                    box.minArr[k] = std::min(v2[k], std::min(v0[k], v1[k]));
                    box.maxArr[k] = std::max(v2[k], std::max(v0[k], v1[k])) + AABB_Min_Padding;

                    if (_isnan(box.minArr[k]) ||
                        _isnan(box.maxArr[k]))
                    {
                        box.minArr[k] = 0;
                        box.maxArr[k] = 0;
                    }
                }

                // Create out internal triangle indices.
                PrimitiveMetaData metadata;
                metadata.GeometryContributionToHitGroupIndex = i;
                metadata.PrimitiveIndex = triangleIndex;
                metadata.GeometryFlags = geometry.Flags;
                primitiveMetaData[triangleIndex] = metadata;

                // Next triangle
                triangleIndex++;
            }
        }

        //
        // Create a BVH
        //

        BuildBVH(bvh, boxes, primitiveMetaData, MAX_TRIS_IN_LEAF);

        //
        // Now copy and compress geometry
        //

        // Copy verts
        const UINT numTris = triangleIndex;
        bvh.m_triangles.resize(numTris * 3 * 3);
        assert(bvh.m_triangles.size() == triangleVertices.size());
        assert(sizeof(bvh.m_triangles[0]) == sizeof(triangleVertices[0]));

        for (UINT i = 0; i < numTris; ++i)
        {
            UINT inputIndex = bvh.m_metadata[i].PrimitiveIndex;
            float *pInputTriangle = &triangleVertices.data()[inputIndex * 9];
            float* pOutputTriangle = &bvh.m_triangles[i * 9];

            // Construct three planes and write to pPlanes
            XMVECTOR V0 = XMVectorSet(pInputTriangle[0], pInputTriangle[1], pInputTriangle[2], 0.0f);
            XMVECTOR V1 = XMVectorSet(pInputTriangle[3], pInputTriangle[4], pInputTriangle[5], 0.0f);
            XMVECTOR V2 = XMVectorSet(pInputTriangle[6], pInputTriangle[7], pInputTriangle[8], 0.0f);

            XMStoreFloat3((XMFLOAT3*)pOutputTriangle + 0, V0);
            XMStoreFloat3((XMFLOAT3*)pOutputTriangle + 1, V1);
            XMStoreFloat3((XMFLOAT3*)pOutputTriangle + 2, V2);
        }
    }
}

void BuildRaytracingAccelerationStructureOnCpu(
    _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
    _Out_ void *pData)
{
    FallbackLayer::BVH bvh;
    FallbackLayer::BuildUniformBVH(pDesc->Inputs.NumDescs, pDesc->Inputs.pGeometryDescs, bvh);

    BYTE* outputData = (BYTE*)pData;
    BVHOffsets offsets;
    offsets.offsetToBoxes = sizeof(BVHOffsets);
    const UINT sizeofBoxes = (UINT)(bvh.m_nodes.size() * sizeof(*bvh.m_nodes.data()));
    offsets.offsetToVertices = offsets.offsetToBoxes + sizeofBoxes;
    
    UINT numTriangles = (UINT)bvh.m_triangles.size() / 9;
    const UINT sizeofVertices = numTriangles * sizeof(Primitive);
    offsets.offsetToPrimitiveMetaData = offsets.offsetToVertices + sizeofVertices;

    const UINT sizeofMetadata = (UINT)(bvh.m_metadata.size() * sizeof(*bvh.m_metadata.data()));
    offsets.totalSize = offsets.offsetToPrimitiveMetaData + sizeofMetadata;

    memcpy(outputData,  &offsets, sizeof(offsets));
    memcpy(outputData + offsets.offsetToBoxes, bvh.m_nodes.data(), sizeofBoxes);

    Primitive *pPrimitives = (Primitive *)(outputData + offsets.offsetToVertices);
    for (UINT i = 0; i < numTriangles; i++)
    {
        Triangle *pTriangle = (Triangle *)((BYTE *)bvh.m_triangles.data() + sizeof(Triangle) * i);
        pPrimitives[i].PrimitiveType = TRIANGLE_TYPE;
        pPrimitives[i].triangle = *pTriangle;
    }
    memcpy(outputData + offsets.offsetToPrimitiveMetaData, bvh.m_metadata.data(), sizeofMetadata);
}
