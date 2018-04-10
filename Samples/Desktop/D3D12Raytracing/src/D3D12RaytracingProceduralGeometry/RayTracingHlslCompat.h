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

#ifdef HLSL
#include "HlslCompat.h"
#else
using namespace DirectX;

// Shader will use byte encoding to access indices.
typedef UINT16 Index;
#endif

struct SceneConstantBuffer
{
    XMMATRIX projectionToWorld;
    XMVECTOR cameraPosition;
    XMVECTOR lightPosition;
    XMVECTOR lightAmbientColor;
    XMVECTOR lightDiffuseColor;
};

struct CubeConstantBuffer
{
    XMVECTOR diffuseColor;
};

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT3 normal;
};

struct SphereAABB
{
    XMFLOAT3 center;
    float  radius;
};

struct RectangularPrismAABB
{
    XMFLOAT3 minPosition;
    XMFLOAT3 maxPosition;
};

// ToDo: align all structs
// Performance tip: Align structures on sizeof(float4) boundary.
// Ref: https://developer.nvidia.com/content/understanding-structured-buffer-performance
struct AABBPrimitiveAttributes
{
    XMMATRIX bottomLevelASToLocalSpace;   // Matrix from bottom-level object space to local primitive space
    XMFLOAT3 albedo;
    float    padding;
};

