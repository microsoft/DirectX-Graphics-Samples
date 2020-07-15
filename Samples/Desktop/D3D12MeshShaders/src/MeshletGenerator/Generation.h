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

#include <DirectXMath.h>

#include <assert.h>
#include <memory>
#include <vector>

template <typename T>
struct InlineMeshlet
{
    struct PackedTriangle
    {
        uint32_t i0 : 10;
        uint32_t i1 : 10;
        uint32_t i2 : 10;
        uint32_t spare : 2;
    };

    std::vector<T>              UniqueVertexIndices;
    std::vector<PackedTriangle> PrimitiveIndices;
};
    
void Meshletize(
    uint32_t maxVerts, uint32_t maxPrims,
    const uint16_t* indices, uint32_t indexCount,
    const DirectX::XMFLOAT3* positions, uint32_t vertexCount,
    std::vector<InlineMeshlet<uint16_t>>& output
);

void Meshletize(
    uint32_t maxVerts, uint32_t maxPrims,
    const uint32_t* indices, uint32_t indexCount,
    const DirectX::XMFLOAT3* positions, uint32_t vertexCount,
    std::vector<InlineMeshlet<uint32_t>>& output
);
