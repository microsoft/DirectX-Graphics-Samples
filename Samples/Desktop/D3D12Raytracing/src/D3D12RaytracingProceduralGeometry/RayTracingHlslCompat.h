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


// ToDo revert caching hitposition to avoid live values

#define ANIMATE_PRIMITIVES 0

// ToDo check for unreleased resources.
#define ENABLE_NEW_CODE 0

// Workaround for dynamic indexing issue in DXR shaders on Nvidia
#define DO_NOT_USE_DYNAMIC_INDEXING 1 

// Workaround for NV driver not supporting null local root signatures. 
// Use an empty local root signature where a shader does not require it.
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

// PERFOMANCE TIP: Set max recursion depth as low as needed
// as drivers may apply optimization strategies for low recursion depths.
#define MAX_RAY_RECURSION_DEPTH 3 // ToDo ~ primary rays + reflections + shadow rays.

// ToDo cleanup
struct ProceduralPrimitiveAttributes
{
    XMFLOAT3 normal;
};
struct ShadowRayPayload
{
    bool hit;
};


struct RayPayload
{
    XMFLOAT4 color;
    UINT   recursionDepth;
};

struct SceneConstantBuffer
{
    XMMATRIX projectionToWorld;
    XMVECTOR cameraPosition;
    XMVECTOR lightPosition;
    XMVECTOR lightAmbientColor;
    XMVECTOR lightDiffuseColor;
    // ToDo move out?
    float    totalTime;                 // Elapsed application time.
};

// ToDo split or rename
struct MaterialConstantBuffer
{
    XMFLOAT4 albedo;
    float stepScale;                      // Step scale for ray marching of signed distance primitives. 
                                          // - Some object transformations don't preserve the distances and 
                                          //   thus require shorter steps.
    XMFLOAT3 padding;
};

struct AABBConstantBuffer
{
    UINT geometryIndex;
    UINT primitiveType;
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


namespace AnalyticPrimitive {
    enum Enum {
        AABB = 0,
        Sphere,
        Spheres,
        Count
    };
}

namespace VolumetricPrimitive {
    enum Enum {
        Metaballs = 0,
        Count
    };
}

namespace SignedDistancePrimitive {
    enum Enum {
        Cone = 0,
        MiniSpheres,
        IntersectedRoundCube,
        Torus,
        TwistedTorus,
        Pyramid, 
        Cog,
        Cylinder,
        SquareTorus,
        Count
    };
}

struct AABBPrimitiveAttributes
{
    XMMATRIX localSpaceToBottomLevelAS;   // Matrix from local primitive space to bottom-level object space.
    XMMATRIX bottomLevelASToLocalSpace;   // Matrix from bottom-level object space to local primitive space.
};

#endif // RAYTRACINGHLSLCOMPAT_H