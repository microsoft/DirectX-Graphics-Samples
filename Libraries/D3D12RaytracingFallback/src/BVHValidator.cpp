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
#pragma once
#include "pch.h"

namespace FallbackLayer
{

    // TODO: Likely too large, but good enough for current validation
#define TEST_EPSILON 0.001

    bool IsChildContainedByParent(const AABB &parent, const AABB &child)
    {
        return
            parent.min.x - TEST_EPSILON <= child.min.x &&
            parent.min.y - TEST_EPSILON <= child.min.y &&
            parent.min.z - TEST_EPSILON <= child.min.z &&

            parent.max.x + TEST_EPSILON >= child.max.x &&
            parent.max.y + TEST_EPSILON >= child.max.y &&
            parent.max.z + TEST_EPSILON >= child.max.z;
    }

    bool IsFloatEqual(float a, float b)
    {
        return fabs(a - b) < TEST_EPSILON;
    }

    bool BvhValidator::IsVertexEqual(const BvhValidator::Vertex &vertex1, const BvhValidator::Vertex &vertex2)
    {
        return IsFloatEqual(vertex1.x, vertex2.x) &&
            IsFloatEqual(vertex1.y, vertex2.y) &&
            IsFloatEqual(vertex1.z, vertex2.z);
    }

    bool BvhValidator::IsVertexContainedByAABB(const AABB &aabb, const BvhValidator::Vertex &v)
    {
        return v.x + TEST_EPSILON >= aabb.min.x &&
            v.y + TEST_EPSILON >= aabb.min.y &&
            v.z + TEST_EPSILON >= aabb.min.z &&
            v.x - TEST_EPSILON <= aabb.max.x &&
            v.y - TEST_EPSILON <= aabb.max.y &&
            v.z - TEST_EPSILON <= aabb.max.z;
    }

    bool IsChildNodeIndexValid(UINT nodeIndex)
    {
        return nodeIndex != 0;
    }

    bool BvhValidator::VerifyBVHOutput(
        std::vector<LeafNodePtr> &pExpectedLeafNodes,
        const BYTE *pOutputCpuData,
        std::wstring &errorMessage)
    {
#define ThrowError(msg) errorMessage = msg; throw false;
#define ThrowErrorIfFalse(exp, msg) if(!(exp)) {ThrowError(msg);}

        try
        {
            // Given the list of triangles used to construct the BVH,
            // go through the tree layer by layer (breadth-first) ensuring that:
            // 1. The child nodes are contained in the parent node
            // 2. All leaves must be able to fit within at least one of the AABBs at that level 
            //    (unless that leaf's node has already been found)

            const UINT verticesPerTriangle = 3;
            const UINT floatsPerVertex = 3;
            for (auto &pLeaf : pExpectedLeafNodes)
            {
                pLeaf->LeafFound = false;
            }

            BVHOffsets offsets = *(BVHOffsets*)pOutputCpuData;
            AABBNode *pNodeArray = (AABBNode*)((BYTE *)pOutputCpuData + offsets.offsetToBoxes);
            Primitive *pPrimitiveArray = (Primitive*)((BYTE *)pOutputCpuData + offsets.offsetToVertices);

            std::deque<AABBNode*> nodeQueue;

            nodeQueue.push_back(&pNodeArray[0]);
            UINT nodesInLevel = 1;
            while (nodeQueue.size())
            {
                AABBNode *pCompressedNode = nodeQueue.front();
                nodeQueue.pop_front();
                nodesInLevel--;
                bool bProcessedLastNodeInCurrentLevel = nodesInLevel == 0;

                AABB parentAABB;
                FallbackLayer::DecompressAABB(parentAABB, *pCompressedNode);

                const bool bIsLeaf = pCompressedNode->leaf;

                for (auto &pLeaf : pExpectedLeafNodes)
                {
                    if (pLeaf->IsContainedByBox(parentAABB))
                    {
                        pLeaf->LeafFound = true;
                    }
                }

                if (!bIsLeaf)
                {
                    {
                        ThrowErrorIfFalse(IsChildNodeIndexValid(pCompressedNode->internalNode.leftNodeIndex), L"Circular referance to root node");
                        AABBNode *pLeftNode = &pNodeArray[pCompressedNode->internalNode.leftNodeIndex];
                        AABB leftAABB;
                        FallbackLayer::DecompressAABB(leftAABB, *pLeftNode);
                        ThrowErrorIfFalse(IsChildContainedByParent(parentAABB, leftAABB), L"AABB not contained by parent");

                        nodeQueue.push_back(pLeftNode);
                    }

                    {
                        UINT rightNodeIndex = pCompressedNode->rightNodeIndex;
      
                        ThrowErrorIfFalse(IsChildNodeIndexValid(rightNodeIndex), L"Circular referance to root node");
                        AABBNode *pRightNode = &pNodeArray[rightNodeIndex];
                        AABB rightAABB;
                        FallbackLayer::DecompressAABB(rightAABB, *pRightNode);
                        ThrowErrorIfFalse(IsChildContainedByParent(parentAABB, rightAABB), L"AABB not contained by parent");

                        nodeQueue.push_back(pRightNode);
                    }
                }
                else
                {
                    // TODO: Hacky way to use the same code path for both bottom and top level
                    // BVHs. Doing the triangle calculations for both paths, should 
                    UINT firstTriangleId = pCompressedNode->leafNode.firstTriangleId;
                    UINT numTriangles = 1; // pCompressedNode->numTriangles;
                    ThrowErrorIfFalse(numTriangles > 0, L"Invalid value for numTriangles");

                    for (UINT triangleId = firstTriangleId; triangleId < firstTriangleId + numTriangles; triangleId++)
                    {
                        Primitive *pTriangle = &pPrimitiveArray[triangleId];
                        for (int j = (int)pExpectedLeafNodes.size() - 1; j >= 0; j--)
                        {
                            if (pExpectedLeafNodes[j]->IsLeafEqual((void *)pTriangle, parentAABB))
                            {
                                pExpectedLeafNodes.erase(pExpectedLeafNodes.begin() + j);
                            }
                        }
                    }

                }

                // If a whole level has been traversed, verify
                // that all triangles have been accounted for
                if (bProcessedLastNodeInCurrentLevel)
                {
                    for (auto &pLeaf : pExpectedLeafNodes)
                    {
                        if (!pLeaf->LeafFound)
                        {
                            ThrowError(L"One of the BVH levels has AABBs that can't contain one of the leaf nodes");
                        }

                        // Reset the flags after verification
                        pLeaf->LeafFound = false;
                    }
                    nodesInLevel = static_cast<UINT>(nodeQueue.size());
                }
            }
            ThrowErrorIfFalse(pExpectedLeafNodes.size() == 0, L"Didn't find a leaf node for one or more of the expected leaves");
        }
        catch (bool)
        {
            return false;
        }
        return true;
    }

    bool BvhValidator::AABBLeafNode::IsContainedByBox(const AABB &parentBox)
    {
        return IsChildContainedByParent(parentBox, box);
    };

    bool BvhValidator::AABBLeafNode::IsLeafEqual(void *pLeafData, const AABB &leafAABB)
    {
        UNREFERENCED_PARAMETER(pLeafData);
        return IsChildContainedByParent(leafAABB, box);
    }

    template<typename V>
    V Transform(V &v, _In_reads_(12) const float* transform)
    {
        return {
            v.x * transform[0] + v.y * transform[1] + v.z  * transform[2]  + transform[3],
            v.x * transform[4] + v.y * transform[5] + v.z  * transform[6]  + transform[7],
            v.x * transform[8] + v.y * transform[9] + v.z  * transform[10] + transform[11]
        };
    }

    AABB BvhValidator::TransformAABB(const AABB &box, _In_reads_(12) const float* transform)
    {
        float3 vertices[8];
        vertices[0] = box.min;
        vertices[1] = { box.min.x, box.min.y, box.max.z };
        vertices[2] = { box.min.x, box.max.y, box.max.z };
        vertices[3] = { box.min.x, box.max.y, box.min.z };
        vertices[4] = { box.max.x, box.max.y, box.min.z };
        vertices[5] = { box.max.x, box.min.y, box.min.z };
        vertices[6] = { box.max.x, box.min.y, box.max.z };
        vertices[7] = box.max;
        
        AABB transformedBox;
        transformedBox.min = { FLT_MAX,FLT_MAX, FLT_MAX };
        transformedBox.max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
        for (UINT i = 0; i < ARRAYSIZE(vertices); i++)
        {
            float3 v = Transform(vertices[0], transform);
            transformedBox.min = min(v, transformedBox.min);
            transformedBox.max = max(v, transformedBox.max);
        }
        return transformedBox;
    }


    bool BvhValidator::VerifyTopLevelOutput(
        const AABB *pReferenceBoxes,
        float **ppInstanceTransforms,
        UINT numBoxes,
        const BYTE *pOutputCpuData,
        std::wstring &errorMessage)
    {
        std::vector<LeafNodePtr> pLeafNodes;
        for (UINT i = 0; i < numBoxes; i ++)
        {
            AABB aabb = pReferenceBoxes[i];
            if (ppInstanceTransforms)
            {
                aabb = TransformAABB(aabb, ppInstanceTransforms[i]);
            }
            pLeafNodes.push_back(std::unique_ptr<LeafNode>(new AABBLeafNode(aabb)));
        }

        return VerifyBVHOutput(pLeafNodes, pOutputCpuData, errorMessage);
    }

    bool BvhValidator::TriangleLeafNode::IsContainedByBox(const AABB &box)
    {
        return IsVertexContainedByAABB(box, v0) &&
            IsVertexContainedByAABB(box, v1) &&
            IsVertexContainedByAABB(box, v2);
    }

    bool BvhValidator::TriangleLeafNode::IsLeafEqual(void *pLeafData, const AABB &leafAABB)
    {
        UNREFERENCED_PARAMETER(leafAABB);
        Primitive *pPrimitive = (Primitive *)pLeafData;
        Triangle *pTriangle = &pPrimitive->triangle;
        return IsTriangleEqual(*this, pTriangle);
    }

    UINT CalculateBaseIndex(UINT triangleIndex)
    {
        return triangleIndex * 3;
    }

    UINT GetIndex(void *pIndexBufferData, UINT readIndex, DXGI_FORMAT format)
    {
        switch (format)
        {
        case DXGI_FORMAT_R32_UINT:
        {
            auto pIndexBuffer = (UINT32 *)pIndexBufferData;
            return pIndexBuffer[readIndex];
        }
        case DXGI_FORMAT_R16_UINT:
        {
            auto pIndexBuffer = (UINT16 *)pIndexBufferData;
            return pIndexBuffer[readIndex];
        }
        case DXGI_FORMAT_UNKNOWN:
            return readIndex;
        default:
            ThrowFailure(E_INVALIDARG, L"Invalid format provided for the index buffer, must be: DXGI_FORMAT_R32_UINT/DXGI_FORMAT_R16_UINT/DXGI_FORMAT_UNKNOWN");
            return (UINT)-1;
        }
    }

    bool BvhValidator::VerifyBottomLevelOutput(
        CpuGeometryDescriptor *pCpuGeometryDescriptors,
        UINT geometryCount,
        const BYTE *pBVHData, std::wstring &errorMessage)
    {
        std::vector<std::unique_ptr<LeafNode>> pLeafNodes;

        for (UINT geometryIndex = 0; geometryIndex < geometryCount; geometryIndex++)
        {
            CpuGeometryDescriptor &geometryDescriptor = pCpuGeometryDescriptors[geometryIndex];
            const UINT vertexStrideInBytes = sizeof(float) * 3;
            const float *pVerticies = geometryDescriptor.m_pVertexData;
            void *pIndicies = (UINT16*)geometryDescriptor.m_pIndexBuffer;

            const UINT vertexCount = pIndicies ? geometryDescriptor.m_numIndicies : geometryDescriptor.m_numVerticies;
            const UINT triangleCount = vertexCount / 3;

            const UINT verticesPerTriangle = 3;
            const UINT floatsPerVertex = 3;
            BYTE *pVertexByteBuffer = (BYTE *)pVerticies;
            for (UINT i = 0; i < triangleCount; i ++)
            {
                Vertex v[verticesPerTriangle];
                UINT baseIndex = CalculateBaseIndex(i);
                for (UINT vertexIndex = 0; vertexIndex < verticesPerTriangle; vertexIndex++)
                {
                    float *pVertex = (float *)&pVertexByteBuffer[vertexStrideInBytes * GetIndex(pIndicies, vertexIndex + baseIndex, geometryDescriptor.m_indexBufferFormat)];
                    v[vertexIndex] = { pVertex[0], pVertex[1], pVertex[2] };

                    v[vertexIndex] = Transform(v[vertexIndex], geometryDescriptor.transform.data());
                }

                pLeafNodes.push_back(std::unique_ptr<LeafNode>(new TriangleLeafNode(v[0], v[1], v[2])));
            }
        }

        return VerifyBVHOutput(pLeafNodes, pBVHData, errorMessage);
    }

    void DecompressAABB(
        AABB& box,
        const AABBNode& packedBox)
    {
        box.min.x = packedBox.center[0] - packedBox.halfDim[0];
        box.min.y = packedBox.center[1] - packedBox.halfDim[1];
        box.min.z = packedBox.center[2] - packedBox.halfDim[2];
        box.max.x = packedBox.center[0] + packedBox.halfDim[0];
        box.max.y = packedBox.center[1] + packedBox.halfDim[1];
        box.max.z = packedBox.center[2] + packedBox.halfDim[2];
    }
}
