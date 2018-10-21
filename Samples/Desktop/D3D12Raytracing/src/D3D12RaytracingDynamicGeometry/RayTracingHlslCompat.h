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
#define RUNTIME_AS_UPDATES 1
#define USE_GPU_TRANSFORM 0

#define RAYGEN_SINGLE_COLOR_SHADING 0
#define SAMPLES_CS_VISUALIZATION 1

#define GBUFFER_AO_NORMAL_VISUALIZATION 0
#define GBUFFER_AO_HIT_VISUALIZATION 0
#define GBUFFER_AO_USE_ALBEDO 1

#define ONLY_SQUID_SCENE_BLAS 1

// ToDo move
namespace ReduceSumCS {
	namespace ThreadGroup
	{
		enum Enum { Width = 8, Height = 16, Size = Width * Height, NumElementsToLoadPerThread = 10 };	
	}
}

#ifdef HLSL
#include "util\HlslCompat.h"
#else
using namespace DirectX;

// Shader will use byte encoding to access vertex indices.
typedef UINT16 Index;
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

// ToDo rename
struct ReduceSumCSCB
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

static const XMFLOAT4 BackgroundColor = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
static const float InShadowRadiance = 0.35f;

#endif // RAYTRACINGHLSLCOMPAT_H