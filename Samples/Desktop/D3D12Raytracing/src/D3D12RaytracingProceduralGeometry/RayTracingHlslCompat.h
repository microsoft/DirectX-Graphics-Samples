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

// Override for debugging, PIX does not support local root constants yet.
#define USE_LOCAL_ROOT_CONSTANTS 1

// Workaround for NV driver as it requires all shaders 
// to have a local root signature bound, even if it's empty.
#define USE_NON_NULL_LOCAL_ROOT_SIG 1  

// Workaround for the Fallback Layer not supporting default exports for DXIL libraries
#define DEFINE_EXPLICIT_SHADER_EXPORTS 1

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

struct MaterialConstantBuffer
{
    XMFLOAT4 albedo;
};

struct AABBConstantBuffer
{
    UINT geometryIndex;
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

struct AABBPrimitiveAttributes
{
    XMMATRIX bottomLevelASToLocalSpace;   // Matrix from bottom-level object space to local primitive space
};

