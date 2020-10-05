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

#include <vector>

void BuildAdjacencyList(
    const uint16_t* indices, uint32_t indexCount,
    const DirectX::XMFLOAT3* positions, uint32_t vertexCount,
    uint32_t* adjacency
);

void BuildAdjacencyList(
    const uint32_t* indices, uint32_t indexCount,
    const DirectX::XMFLOAT3* positions, uint32_t vertexCount,
    uint32_t* adjacency
);

DirectX::XMVECTOR MinimumBoundingSphere(DirectX::XMFLOAT3* points, uint32_t count);
