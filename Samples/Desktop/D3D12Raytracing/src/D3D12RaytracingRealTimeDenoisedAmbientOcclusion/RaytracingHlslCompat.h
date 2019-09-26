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

//************************** Global Overrides **************************************************
#define RENDER_GRASS_GEOMETRY 1
#define PRINT_OUT_CAMERA_CONFIG 1

#ifdef _DEBUG
#define LOAD_ONLY_ONE_PBRT_MESH 0 // Set to 1 to speed up application start on debug.
#else
#define LOAD_ONLY_ONE_PBRT_MESH 0 
#endif
//**********************************************************************************************

#define FOVY 45.f
#define NEAR_PLANE 0.001f
#define FAR_PLANE 1000.0f
#define HitDistanceOnMiss 0
#define SAMPLER_FILTER D3D12_FILTER_ANISOTROPIC

#ifdef HLSL
typedef uint NormalDepthTexFormat;
#else
#define COMPACT_NORMAL_DEPTH_DXGI_FORMAT DXGI_FORMAT_R32_UINT
#endif

namespace ReduceSumCS {
	namespace ThreadGroup {
		enum Enum { Width = 8, Height = 16, Size = Width * Height, NumElementsToLoadPerThread = 10 };	
	}
}

namespace AtrousWaveletTransformFilterCS {
    namespace ThreadGroup {
        enum Enum { Width = 16, Height = 16, Size = Width * Height };
    }
}

namespace DefaultComputeShaderParams {
    namespace ThreadGroup {
        enum Enum { Width = 8, Height = 8, Size = Width * Height };
    }
}


#ifdef HLSL
#include "HlslCompat.hlsli"
typedef UINT Index;
#else
using namespace DirectX;

typedef UINT Index;
#endif

struct Ray
{
    XMFLOAT3 origin;
    XMFLOAT3 direction;
};

// GBuffer data collected during pathtracing 
// for subsequent Ambient Occlusion ray trace pass and denoising.
struct AmbientOcclusionGBuffer
{
    float tHit;
    XMFLOAT3 hitPosition;           // Position of the hit for which to calculate Ambient coefficient.
    UINT diffuseByte3;              // Diffuse reflectivity of the hit surface.
    // TODO: RTAO pipeline uses 16b encoded normal, therefore same bit enconding could be applied here 
    //  to lower the struct's size and potentially improve Pathtracer's perf without much/any quality loss in RTAO.
    //  Furthermore, _encodedNormal below could use lower bit range too.
    XMFLOAT2 encodedNormal;         // Normal of the hit surface. 

    // Members for Motion Vector calculation.
    XMFLOAT3 _virtualHitPosition;   // virtual hitPosition in the previous frame.
                                    // For non-reflected points this is a true world position of a hit.
                                    // For reflected points, this is a world position of a hit reflected across the reflected surface 
                                    //   ultimately giving the same screen space coords when projected and the depth corresponding to the ray depth.
    XMFLOAT2 _encodedNormal;        // surface normal in the previous frame
};


struct PathtracerRayPayload
{
    UINT rayRecursionDepth;
    XMFLOAT3 radiance;              // TODO encode
    AmbientOcclusionGBuffer AOGBuffer;
};

struct ShadowRayPayload
{
    float tHit;         // Hit time <0,..> on Hit. -1 on miss.
};

struct AtrousWaveletTransformFilterConstantBuffer
{
    XMUINT2 textureDim;
    float depthWeightCutoff;
    bool usingBilateralDownsampledBuffers;

    BOOL useAdaptiveKernelSize;
    float kernelRadiusLerfCoef;
    UINT minKernelWidth;
    UINT maxKernelWidth;

    float rayHitDistanceToKernelWidthScale;
    float rayHitDistanceToKernelSizeScaleExponent;
    BOOL perspectiveCorrectDepthInterpolation;
    float minVarianceToDenoise;

    float valueSigma;
    float depthSigma;
    float normalSigma;
    UINT DepthNumMantissaBits;
};

struct CalculateVariance_BilateralFilterConstantBuffer
{
    XMUINT2 textureDim;
    float normalSigma;
    float depthSigma;

    BOOL outputMean;
    BOOL useDepthWeights;
    BOOL useNormalWeights;
    UINT kernelWidth;

    UINT kernelRadius;
    float padding[3];
};

struct CalculateMeanVarianceConstantBuffer
{
    XMUINT2 textureDim;
    UINT kernelWidth;
    UINT kernelRadius;

    BOOL doCheckerboardSampling;
    BOOL areEvenPixelsActive;
    UINT pixelStepY;
    float padding;
};

struct RayGenConstantBuffer
{
    XMUINT2 textureDim;
    BOOL doCheckerboardRayGeneration;
    BOOL checkerboardGenerateRaysForEvenPixels;
    
    UINT seed;
    UINT numSamplesPerSet;
    UINT numSampleSets;
    UINT numPixelsPerDimPerSet;
};

struct SortRaysConstantBuffer
{
    XMUINT2 dim;
    BOOL useOctahedralRayDirectionQuantization;
    // Depth for a bin within which to sort further based on direction.
    float binDepthSize;
};

namespace SortRays {
    namespace ThreadGroup {
        enum Enum { Width = 64, Height = 16, Size = Width * Height };
    }

    namespace RayGroup {
        enum Enum { NumElementPairsPerThread = 4, Width = ThreadGroup::Width, Height = NumElementPairsPerThread * 2 * ThreadGroup::Height, Size = Width * Height };
    }
#ifndef HLSL
    static_assert( RayGroup::Width <= 64 
                && RayGroup::Height <= 128
                && RayGroup::Size <= 8192, "Ray group dimensions are outside the supported limits set by the Counting Sort shader.");
#endif
}

struct PathtracerConstantBuffer
{
    XMMATRIX projectionToWorldWithCameraAtOrigin;
    XMFLOAT3 cameraPosition;
    BOOL     useBaseAlbedoFromMaterial;
    XMFLOAT3 lightPosition;     
    BOOL     useNormalMaps;
    XMFLOAT3 lightColor;
    float    defaultAmbientIntensity;

    XMMATRIX prevFrameViewProj;
    XMMATRIX prevFrameProjToViewCameraAtOrigin;
    XMFLOAT3 prevFrameCameraPosition;
    float    padding;

	float Znear;
	float Zfar;
    UINT  maxRadianceRayRecursionDepth;
    UINT  maxShadowRayRecursionDepth;

};

struct RTAOConstantBuffer
{
    UINT seed;
    UINT numSamplesPerSet;
    UINT numSampleSets;
    UINT numPixelsPerDimPerSet;

    float maxAORayHitTime;             // Max AO ray hit time used for tMax in TraceRay.
    BOOL approximateInterreflections;      // Approximate interreflections. 
    float diffuseReflectanceScale;         // Diffuse reflectance from occluding surfaces. 
    float minimumAmbientIllumination;      // Ambient illumination coef when a ray is occluded.

    float maxTheoreticalAORayHitTime;  // Max AO ray hit time used in falloff computation accounting for
                                       // exponentialFalloffMinOcclusionCutoff and maxAORayHitTime.    
    BOOL useSortedRays;
    XMUINT2 raytracingDim;

    BOOL applyExponentialFalloff;     // Apply exponential falloff to AO coefficient based on ray hit distance.    
    float exponentialFalloffDecayConstant;
    BOOL doCheckerboardSampling;
    BOOL areEvenPixelsActive;

    UINT spp;
    float padding[3];
};

 
// Final render output composition modes.
enum CompositionType {
    PBRShading = 0,
    AmbientOcclusionOnly_Denoised,
    AmbientOcclusionOnly_RawOneFrame,
    AmbientOcclusionAndDisocclusionMap,
    AmbientOcclusionVariance,
    AmbientOcclusionLocalVariance,
    RTAOHitDistance,
    NormalsOnly,
    DepthOnly,
    Albedo,
    BaseMaterialAlbedo,
    Count
};

namespace TextureResourceFormatRGB
{
    enum Type {
        R32G32B32A32_FLOAT = 0,
        R16G16B16A16_FLOAT,
        R11G11B10_FLOAT,
        Count
    };
#ifndef HLSL
    inline DXGI_FORMAT ToDXGIFormat(UINT type)
    {
        switch (type)
        {
        case R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case R16G16B16A16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case R11G11B10_FLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;
        }
        return DXGI_FORMAT_UNKNOWN;
    }
#endif
}

namespace TextureResourceFormatR
{
    enum Type {
        R32_FLOAT = 0,
        R16_FLOAT,
        R8_UNORM,
        Count
    };
#ifndef HLSL
    inline DXGI_FORMAT ToDXGIFormat(UINT type)
    {
        switch (type)
        {
        case R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
        case R16_FLOAT: return DXGI_FORMAT_R16_FLOAT;
        case R8_UNORM: return DXGI_FORMAT_R8_UNORM;
        }
        return DXGI_FORMAT_UNKNOWN;
    }
#endif
}


namespace TextureResourceFormatRG
{
    enum Type {
        R32G32_FLOAT = 0,
        R16G16_FLOAT,
        R8G8_SNORM,
        Count
    };
#ifndef HLSL
    inline DXGI_FORMAT ToDXGIFormat(UINT type)
    {
        switch (type)
        {
        case R32G32_FLOAT: return DXGI_FORMAT_R32G32_FLOAT;
        case R16G16_FLOAT: return DXGI_FORMAT_R16G16_FLOAT;
        case R8G8_SNORM: return DXGI_FORMAT_R8G8_SNORM;
        }
        return DXGI_FORMAT_UNKNOWN;
    }
#endif
}

struct ComposeRenderPassesConstantBuffer
{
    CompositionType compositionType;
    UINT isAOEnabled;
    float RTAO_MaxRayHitDistance;
    float defaultAmbientIntensity;
    
    BOOL variance_visualizeStdDeviation;
    float variance_scale;
    float padding[2];
};

struct TextureDimConstantBuffer
{
    XMUINT2 textureDim;
    XMFLOAT2 invTextureDim;
};

struct FilterConstantBuffer
{
    XMUINT2 textureDim;
    UINT step;
    float padding;
};

struct TemporalSupersampling_ReverseReprojectConstantBuffer
{
    XMUINT2 textureDim;
    XMFLOAT2 invTextureDim;

    float depthSigma;
    UINT DepthNumMantissaBits;      // Number of Mantissa Bits in the floating format of the input depth resources format.
    BOOL usingBilateralDownsampledBuffers;
    float padding;
};

struct TemporalSupersampling_BlendWithCurrentFrameConstantBuffer
{
    float stdDevGamma;
    BOOL clampCachedValues;
    float clamping_minStdDevTolerance;
    float padding;

    float clampDifferenceToTsppScale;
    BOOL forceUseMinSmoothingFactor;
    float minSmoothingFactor;
    UINT minTsppToUseTemporalVariance;

    UINT blurStrength_MaxTspp;
    float blurDecayStrength;
    BOOL checkerboard_enabled;
    BOOL checkerboard_areEvenPixelsActive;
};

struct CalculatePartialDerivativesConstantBuffer
{
    XMUINT2 textureDim;
    float padding[2];
};

struct DownAndUpsampleFilterConstantBuffer
{
    XMFLOAT2 invHiResTextureDim;
    XMFLOAT2 invLowResTextureDim;
};

// Grass Geometry
#define N_GRASS_TRIANGLES 5
#define N_GRASS_VERTICES 7
#define MAX_GRASS_STRAWS_1D 100
struct GenerateGrassStrawsConstantBuffer_AppParams
{
    XMUINT2 activePatchDim; // Dimensions of active grass straws.
    XMUINT2 maxPatchDim;    // Dimensions of the whole vertex buffer.

    XMFLOAT2 timeOffset;
    float grassHeight;
    float grassScale;
    
    XMFLOAT3 patchSize;
    float grassThickness;

    XMFLOAT3 windDirection;
    float windStrength;

    float positionJitterStrength;
    float bendStrengthAlongTangent;
    float padding[2];
};

struct GenerateGrassStrawsConstantBuffer
{
    XMFLOAT2 invActivePatchDim;
    float padding1;
    float padding2;
    GenerateGrassStrawsConstantBuffer_AppParams p;
};


// Attributes per primitive type.
struct PrimitiveConstantBuffer
{
	UINT     materialID;          
    UINT     isVertexAnimated; 
    UINT     padding[2];
};

namespace MaterialType {
    enum Type {
        Default,
        Matte,      // Lambertian scattering
        Mirror,     // Specular reflector that isn't modified by the Fresnel equations.
        AnalyticalCheckerboardTexture
    };
}

struct PrimitiveMaterialBuffer
{
	XMFLOAT3 Kd;
	XMFLOAT3 Ks;
    XMFLOAT3 Kr;
    XMFLOAT3 Kt;
    XMFLOAT3 opacity;
    XMFLOAT3 eta;
    float roughness;
    BOOL hasDiffuseTexture;
    BOOL hasNormalTexture;
    BOOL hasPerVertexTangents;
    MaterialType::Type type;
    float padding;
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

struct VertexPositionNormalTextureTangent
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 textureCoordinate;
	XMFLOAT3 tangent;
};

// Ray types traced in this sample.
namespace PathtracerRayType {
    enum Enum {
        Radiance = 0,	// ~ Radiance ray generating color and GBuffer data
        Shadow,         // ~ Shadow/visibility rays
        Count
    };
}

namespace RTAORayType {
    enum Enum {
        AO = 0,	
        Count
    };
}


namespace TraceRayParameters
{
    static const UINT InstanceMask = ~0;   // Everything is visible.
    namespace HitGroup {
        static const UINT Offset[PathtracerRayType::Count] =
        {
            0, // Radiance ray
            1, // Shadow ray
        };
		static const UINT GeometryStride = PathtracerRayType::Count;
    }
    namespace MissShader {
        static const UINT Offset[PathtracerRayType::Count] =
        {
            0, // Radiance ray
            1, // Shadow ray
        };
    }
}

namespace RTAOTraceRayParameters
{
    static const UINT InstanceMask = ~0;   // Everything is visible.
    namespace HitGroup {
        static const UINT Offset[RTAORayType::Count] =
        {
            0, // AO ray
        };
        // Since there is only one closest hit shader across shader records in RTAO, 
        // always access the first shader record of each BLAS instance shader record range.
        static const UINT GeometryStride = 0;
    }
    namespace MissShader {
        static const UINT Offset[RTAORayType::Count] =
        {
            0, // AO ray
        };
    }
}

static const XMFLOAT4 BackgroundColor = XMFLOAT4(0.79f, 0.88f, 0.98f, 1.0f);

#endif // RAYTRACINGHLSLCOMPAT_H