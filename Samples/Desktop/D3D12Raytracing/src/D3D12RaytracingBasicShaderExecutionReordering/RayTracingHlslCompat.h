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

#ifndef RAYTRACINGHLSLCOMPAT_H
#define RAYTRACINGHLSLCOMPAT_H

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
    uint32_t enableSER;
};

struct ObjectConstantBuffer
{
    XMFLOAT4 albedo;
    uint32_t materialID;
};

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT3 normal;
    //XMFLOAT2 textureCoordinate;
};

#define CHECKERBOARD_FLOOR_MATERIAL 1

enum TextureIdentifier
{
    TextureID_None = -1,

    // During raytracing
    TextureID_Checkerboard = 0,

    TextureCount
};
#endif // RAYTRACINGHLSLCOMPAT_H