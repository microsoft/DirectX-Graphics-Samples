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
#define MAX_RECURSION_DEPTH 2

#ifdef HLSL
#include "HlslCompat.h"
#else
using namespace DirectX;

// Shader will use byte encoding to access indices.
typedef UINT32 Index;
#endif
class ObjModelLoader;

enum SortMode
{
    SORTMODE_OFF = 0,
    SORTMODE_BY_HIT = 1,
    SORTMODE_BY_MATERIAL = 2,
    SORTMODE_BY_BOTH = 3
};

struct SceneConstantBuffer
{
    XMMATRIX projectionToWorld;
    XMVECTOR cameraPosition;
    XMVECTOR lightPosition;
    XMVECTOR lightAmbientColor;
    XMVECTOR lightDiffuseColor;
    uint32_t sortMode;
};

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 uv;
};

struct ObjectConstantBuffer
{
    XMFLOAT4 albedo;
    uint32_t materialID;
};

#endif // RAYTRACINGHLSLCOMPAT_H