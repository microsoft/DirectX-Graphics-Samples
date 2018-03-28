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
namespace FallbackLayer
{
    class BvhValidator : public IAccelerationStructureValidator
    {
    public:
        virtual bool VerifyBottomLevelOutput(
            CpuGeometryDescriptor *pCpuGeometryDescriptors,
            UINT geometryCount,
            const BYTE *pOutputCpuData, std::wstring &errorMessage);

        virtual bool VerifyTopLevelOutput(
            const AABB *pReferenceBoxes,
            float **ppInstanceTransforms,
            UINT numBoxes,
            const BYTE *pOutputCpuData,
            std::wstring &errorMessage);

    private:

        class LeafNode
        {
        public:
            virtual bool IsContainedByBox(const AABB &box) = 0;
            virtual bool IsLeafEqual(void *pLeafData, const AABB &leafAABB) = 0;
            bool LeafFound = false;
        };

        struct Vertex
        {
            float x, y, z;
        };

        AABB TransformAABB(const AABB &box, _In_reads_(12) const float* transform);

        class AABBLeafNode : public LeafNode
        {
        public:
            AABBLeafNode(const AABB &nBox) : box(nBox) {}
            virtual bool IsLeafEqual(void *pLeafData, const AABB &leafAABB);
            virtual bool IsContainedByBox(const AABB &box);

            AABB box;
        };

        class TriangleLeafNode : public LeafNode
        {
        public:
            TriangleLeafNode(Vertex nV0, Vertex nV1, Vertex nV2) : v0(nV0), v1(nV1), v2(nV2) {}
            virtual bool IsContainedByBox(const AABB &box);
            virtual bool IsLeafEqual(void *pLeafData, const AABB &leafAABB);
            Vertex v0, v1, v2;
        };

        typedef std::unique_ptr<LeafNode> LeafNodePtr;

        bool VerifyBVHOutput(
            std::vector<LeafNodePtr> &pExpectedLeafNodes,
            const BYTE *pOutputCpuData,
            std::wstring &errorMessage);

        static bool IsVertexContainedByAABB(const AABB &aabb, const BvhValidator::Vertex &v);
        static bool IsVertexEqual(const Vertex &vertex1, const Vertex &vertex2);

        template <typename TriangleNode>
        static bool IsTriangleEqual(const TriangleNode triangle, const Triangle *pTriangle)
        {
            BvhValidator::Vertex v[3];
            for (UINT vertexIndex = 0; vertexIndex < 3; vertexIndex++)
            {
                v[vertexIndex].x = pTriangle->v[vertexIndex].x;
                v[vertexIndex].y = pTriangle->v[vertexIndex].y;
                v[vertexIndex].z = pTriangle->v[vertexIndex].z;
            }

            return IsVertexEqual(triangle.v0, v[0]) && 
                   IsVertexEqual(triangle.v1, v[1]) &&
                   IsVertexEqual(triangle.v2, v[2]);
        }
    };

    void DecompressAABB(
        AABB& box,
        const AABBNode& packedBox);
}
