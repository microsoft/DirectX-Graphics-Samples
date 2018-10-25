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

//**********************************************************************************************
//
// RaytracingHLSLCompat.h
//
// A header with shared definitions for C++ and HLSL source files. 
//
//**********************************************************************************************
#define ENABLE_RAYTRACING 1
#define RUNTIME_AS_UPDATES 0
#define USE_GPU_TRANSFORM 1

#define DEBUG_AS 0

#define RAYGEN_SINGLE_COLOR_SHADING 0
#define SAMPLES_CS_VISUALIZATION 1

#define GBUFFER_AO_NORMAL_VISUALIZATION 0
#define GBUFFER_AO_COUNT_AO_HITS 1
#define GBUFFER_AO_USE_ALBEDO 1
#define AO_ANY_HIT_FULL_OCCLUSION 0


#define GBUFFER_AO_SEPRATE_PATHS 1

#define ONLY_SQUID_SCENE_BLAS 1
#if ONLY_SQUID_SCENE_BLAS
#define PBRT_SCENE 0
#define CAMERA_Y_SCALE 1
#define FLAT_FACE_NORMALS 0
#define INDEX_FORMAT_UINT 1
#else

#define INDEX_FORMAT_UINT 0
#define FLAT_FACE_NORMALS 1
#define CAMERA_Y_SCALE 1.3f
#define NUM_GEOMETRIES_1000 1
#define NUM_GEOMETRIES_100000 0
#define NUM_GEOMETRIES_1000000 0
#ifndef HLSL
static_assert(NUM_GEOMETRIES_1000 + NUM_GEOMETRIES_100000  + NUM_GEOMETRIES_1000000 == 1, L"Pick only 1.");
#endif
#endif

#define AO_OVERDOSE_BEND_NORMALS_DOWN 1
#define TESSELATED_GEOMETRY_BOX 1
#define TESSELATED_GEOMETRY_BOX_TETRAHEDRON 1
#define TESSELATED_GEOMETRY_BOX_TETRAHEDRON_REMOVE_BOTTOM_TRIANGLE 1
#define TESSELATED_GEOMETRY_THIN 1
#define TESSELATED_GEOMETRY_TILES 0
#define TESSELATED_GEOMETRY_TILES_WIDTH 4
#define TESSELATED_GEOMETRY_ASPECT_RATIO_DIMENSIONS 1

#if ONLY_SQUID_SCENE_BLAS 
#define AO_RAY_T_MAX 35
#else
#define AO_RAY_T_MAX 0.06
#endif

// ToDo separate per-vertex attributes from VB

// ToDo move
namespace ReduceSumCS {
	namespace ThreadGroup
	{
		enum Enum { Width = 8, Height = 16, Size = Width * Height, NumElementsToLoadPerThread = 10 };	
	}
}

#ifdef HLSL
#include "util\HlslCompat.h"
#if INDEX_FORMAT_UINT
typedef UINT Index;
#endif
#else
using namespace DirectX;

#if INDEX_FORMAT_UINT
typedef UINT Index;
#else
typedef UINT16 Index;
#endif
#endif

#define AO_ONLY 1

// PERFORMANCE TIP: Set max recursion depth as low as needed
// as drivers may apply optimization strategies for low recursion depths.
#define MAX_RAY_RECURSION_DEPTH 3    // ~ primary rays + reflections + shadow rays from reflected geometry.

// ToDo:
// Options:
// - shading - simple/complex
// - instanced/unique goemetry
// - deformed geometry
// - Dynamic options
// - Update/Build
#define ALBEDO_SHADING 0
#define NORMAL_SHADING 0
#define DEPTH_SHADING 0
#define SINGLE_COLOR_SHADING 0

struct ProceduralPrimitiveAttributes
{
    XMFLOAT3 normal;
};

struct RayPayload
{
    XMFLOAT4 color;
    UINT   recursionDepth;
};

struct GBufferRayPayload
{
	bool hit;
	XMFLOAT3 hitPosition;
	XMFLOAT3 surfaceNormal;	// ToDo test encoding normal into 2D
};

struct ShadowRayPayload
{
    bool hit;
};

struct RNGConstantBuffer
{
    XMUINT2 uavOffset;     // offset where [0,0] thread should write to.
    XMUINT2 dispatchDimensions;  // for 2D dispatches
	UINT sampleSetBase;
    UINT numSamples;
    UINT numSampleSets;
    UINT numSamplesToShow; 
    // TODo: Why is padding to 16 needed? cb gets corrupted otherwise. Put a static_assert in ConstantBuffer
    XMUINT2 stratums;      // Stratum resolution
    XMUINT2 grid;      // Grid resolution
};

struct SceneConstantBuffer
{
    XMMATRIX projectionToWorldWithCameraEyeAtOrigin;	// projection to world matrix with Camera at (0,0,0).
    XMVECTOR cameraPosition;
    XMVECTOR lightPosition;
    XMVECTOR lightAmbientColor;
    XMVECTOR lightDiffuseColor;
    float    reflectance;
    float    elapsedTime;                 // Elapsed application time.
    UINT seed;
    UINT numSamples;
    UINT numSampleSets;
    UINT numSamplesToUse;    
    XMFLOAT2 padding;
};

// Attributes per primitive type.
struct PrimitiveConstantBuffer
{
    XMFLOAT4 albedo;
    float reflectanceCoef;
    float diffuseCoef;
    float specularCoef;
    float specularPower;
    float stepScale;                      // Step scale for ray marching of signed distance primitives. 
                                          // - Some object transformations don't preserve the distances and 
                                          //   thus require shorter steps.
    XMFLOAT3 padding;
};

// Attributes per primitive instance.
struct PrimitiveInstanceConstantBuffer
{
    UINT instanceIndex;  
    UINT primitiveType; // Procedural primitive type
};

// Dynamic attributes per primitive instance.
struct PrimitiveInstancePerFrameBuffer
{
    XMMATRIX localSpaceToBottomLevelAS;   // Matrix from local primitive space to bottom-level object space.
    XMMATRIX bottomLevelASToLocalSpace;   // Matrix from bottom-level object space to local primitive space.
};

struct AlignedUnitSquareSample2D
{
    XMFLOAT2 value;
    XMUINT2 padding;  // Padding to 16B
};

struct AlignedHemisphereSample3D
{
    XMFLOAT3 value;
    UINT padding;  // Padding to 16B
};

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT3 normal;
};

struct VertexPositionNormalTexture
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
};

struct VertexPositionNormalTextureTangent
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 textureCoordinate;
	XMFLOAT3 tangent;
};


namespace RayGenShaderType {
	enum Enum {
		GBuffer = 0,
		PrimaryAndAO,
		AO,
		Count
	};
}


// Ray types traced in this sample.
namespace RayType {
    enum Enum {
        Radiance = 0,   // ~ Primary, reflected camera/view rays calculating color for each hit.
        Shadow,         // ~ Shadow/visibility rays, only testing for occlusion
		GBuffer,		// ~ Primary camera ray generating GBuffer data.
        Count
    };
}

namespace TraceRayParameters
{
    static const UINT InstanceMask = ~0;   // Everything is visible.
    namespace HitGroup {
        static const UINT Offset[RayType::Count] =
        {
            0, // Radiance ray
            1, // Shadow ray
			2  // GBuffer ray
        };
		// ToDo For now all geometries reusing shader records
		static const UINT GeometryStride = RayType::Count;
    }
    namespace MissShader {
        static const UINT Offset[RayType::Count] =
        {
            0, // Radiance ray
            1, // Shadow ray
			2, // GBuffer ray
        };
    }
}

static const XMFLOAT4 BackgroundColor = XMFLOAT4(0.79f, 0.88f, 0.98f, 1.0f);
// ToDo
static const float InShadowRadiance = 0.35f;

#endif // RAYTRACINGHLSLCOMPAT_H