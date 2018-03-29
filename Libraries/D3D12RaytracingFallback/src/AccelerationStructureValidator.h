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
#pragma once

struct AABB;

namespace FallbackLayer
{
    // Struct that contains all the data that was used to create the acceleration structure
    // that's being validated, but on the CPU instead.
    struct CpuGeometryDescriptor
    {
        const float *m_pVertexData;
        UINT m_numVerticies;
        const void *m_pIndexBuffer;
        UINT m_numIndicies;
        DXGI_FORMAT m_indexBufferFormat;
        std::vector<float> transform;

        CpuGeometryDescriptor() {}

        CpuGeometryDescriptor(
            const float *pVertexData,
            UINT numVerticies,
            const void *pIndexBuffer,
            UINT numIndicies,
            DXGI_FORMAT indexBufferFormat,
            const float *pTransform = nullptr) :
            m_pVertexData(pVertexData),
            m_numVerticies(numVerticies),
            m_pIndexBuffer(pIndexBuffer),
            m_numIndicies(numIndicies),
            m_indexBufferFormat(indexBufferFormat) 
        {
            const float IdentityMatrix[] =
            {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
            };

            if (!pTransform)
            {
                pTransform = IdentityMatrix;
            }

            for (UINT i = 0; i < 12; i++)
            {
                transform.push_back(pTransform[i]);
            }
        }

        CpuGeometryDescriptor(
            const float *pVertexData,
            UINT numVerticies) :
            CpuGeometryDescriptor(pVertexData, numVerticies, nullptr, 0, DXGI_FORMAT_UNKNOWN) {}


        CpuGeometryDescriptor(
            const float *pVertexData,
            UINT numVerticies,
            const UINT16 *pIndexBuffer,
            UINT numIndicies) :
            CpuGeometryDescriptor(pVertexData, numVerticies, pIndexBuffer, numIndicies, DXGI_FORMAT_R16_UINT) {}

        CpuGeometryDescriptor(
            const float *pVertexData,
            UINT numVerticies,
            const UINT32 *pIndexBuffer,
            UINT numIndicies) :
            CpuGeometryDescriptor(pVertexData, numVerticies, pIndexBuffer, numIndicies, DXGI_FORMAT_R32_UINT) {}

        UINT GetSizeOfIndex()
        {
            switch (m_indexBufferFormat)
            {
            case DXGI_FORMAT_R32_UINT:
                return sizeof(UINT32);
            case DXGI_FORMAT_R16_UINT:
                return sizeof(UINT16);
            default:
                return (UINT)-1;
            }
        }
    };

    class IAccelerationStructureValidator
    {
    public:
        virtual ~IAccelerationStructureValidator() {};

        virtual bool VerifyBottomLevelOutput(
            CpuGeometryDescriptor *pCpuGeometryDescriptors,
            UINT geometryCount,
            const BYTE *pOutputCpuData, std::wstring &errorMessage) = 0;

        virtual bool VerifyTopLevelOutput(
            const AABB *pReferenceBoxes,
            float **ppInstanceTransforms,
            UINT numReferenceBoxes,
            const BYTE *pOutputCpuData,
            std::wstring &errorMessage) = 0;
    };
}
