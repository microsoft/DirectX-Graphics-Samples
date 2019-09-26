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
#ifndef RAYTRACINGSHADERHELPER_H
#define RAYTRACINGSHADERHELPER_H

#include "RayTracingHlslCompat.h"
#include "RTAO/Shaders/RTAO.hlsli"

#define INFINITY (1.0/0.0)

#define FLT_EPSILON     1.192092896e-07 // Smallest number such that 1.0 + FLT_EPSILON != 1.0
#define FLT_MIN         1.175494351e-38 
#define FLT_MAX         3.402823466e+38 
#define FLT_10BIT_MIN   6.1e-5
#define FLT_10BIT_MAX   6.5e4
#define PI              3.1415926535897f

float length_toPow2(float2 p)
{
    return dot(p, p);
}

float length_toPow2(float3 p)
{
    return dot(p, p);
}

uint Float2ToHalf(in float2 val)
{
    uint result = 0;
    result = f32tof16(val.x);
    result |= f32tof16(val.y) << 16;
    return result;
}

float2 HalfToFloat2(in uint val)
{
    float2 result;
    result.x = f16tof32(val);
    result.y = f16tof32(val >> 16);
    return result;
}

uint2 EncodeMaterial16b(uint materialID, float3 diffuse)
{
    uint2 result;
    result.x = materialID;
    result.x |= f32tof16(diffuse.r) << 16;
    result.y = Float2ToHalf(diffuse.gb);

    return result;
}

bool IsWithinBounds(in int2 index, in int2 dimensions)
{
    return index.x >= 0 && index.y >= 0 && index.x < dimensions.x && index.y < dimensions.y;
}

void DecodeMaterial16b(in uint2 material, out uint materialID, out float3 diffuse)
{
    materialID = material.x & 0xffff;
    diffuse.r = f16tof32(material.x >> 16);
    diffuse.gb = HalfToFloat2(material.y);
}

// Remaps a value to [0,1] for a given range.
float RemapToRange(in float value, in float rangeMin, in float rangeMax)
{
	return saturate((value - rangeMin) / (rangeMax - rangeMin));
}

// Returns a cycling <0 -> 1 -> 0> animation interpolant 
float CalculateAnimationInterpolant(in float elapsedTime, in float cycleDuration)
{
    float curLinearCycleTime = fmod(elapsedTime, cycleDuration) / cycleDuration;
    curLinearCycleTime = (curLinearCycleTime <= 0.5f) ? 2 * curLinearCycleTime : 1 - 2 * (curLinearCycleTime - 0.5f);
    return smoothstep(0, 1, curLinearCycleTime);
}

void swap(inout float a, inout float b)
{
    float temp = a;
    a = b;
    b = temp;
}

bool IsInRange(in uint val, in uint min, in float max)
{
    return (val >= min && val <= max);
}

bool IsInRange(in float val, in float min, in float max)
{
    return (val >= min && val <= max);
}

float RGBtoLuminance(in float3 color)
{
    return 0.2126f * color.r + 0.7152f * color.g + 0.0722f * color.b;
}

// Load three 16 bit indices from a byte addressed buffer.
static
uint3 Load3x16BitIndices(uint offsetBytes, ByteAddressBuffer Indices)
{
    uint3 indices;

    // ByteAdressBuffer loads must be aligned at a 4 byte boundary.
    // Since we need to read three 16 bit indices: { 0, 1, 2 } 
    // aligned at a 4 byte boundary as: { 0 1 } { 2 0 } { 1 2 } { 0 1 } ...
    // we will load 8 bytes (~ 4 indices { a b | c d }) to handle two possible index triplet layouts,
    // based on first index's offsetBytes being aligned at the 4 byte boundary or not:
    //  Aligned:     { 0 1 | 2 - }
    //  Not aligned: { - 0 | 1 2 }
    const uint dwordAlignedOffset = offsetBytes & ~3;
    const uint2 four16BitIndices = Indices.Load2(dwordAlignedOffset);

    // Aligned: { 0 1 | 2 - } => retrieve first three 16bit indices
    if (dwordAlignedOffset == offsetBytes)
    {
        indices.x = four16BitIndices.x & 0xffff;
        indices.y = (four16BitIndices.x >> 16) & 0xffff;
        indices.z = four16BitIndices.y & 0xffff;
    }
    else // Not aligned: { - 0 | 1 2 } => retrieve last three 16bit indices
    {
        indices.x = (four16BitIndices.x >> 16) & 0xffff;
        indices.y = four16BitIndices.y & 0xffff;
        indices.z = (four16BitIndices.y >> 16) & 0xffff;
    }

    return indices;
}

// Retrieve hit world position.
float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

// Retrieve hit object space position.
float3 HitObjectPosition()
{
    return ObjectRayOrigin() + RayTCurrent() * ObjectRayDirection();
}

float2 ClipSpaceToTexturePosition(in float4 clipSpacePosition)
{
    float3 NDCposition = clipSpacePosition.xyz / clipSpacePosition.w;   // Perspective divide to get Normal Device Coordinates: {[-1,1], [-1,1], (0, 1]}
    NDCposition.y = -NDCposition.y;                                     // Invert Y for DirectX-style coordinates.
    float2 texturePosition = (NDCposition.xy + 1) * 0.5f;               // [-1,1] -> [0, 1]
    return texturePosition;
}

// Retrieve attribute at a hit position interpolated from vertex attributes using the hit's barycentrics.
float3 HitAttribute(float3 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
    return vertexAttribute[0] +
        attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

// Retrieve attribute at a hit position interpolated from vertex attributes using the hit's barycentrics.
float2 HitAttribute(float2 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
    return vertexAttribute[0] +
        attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

// Generate camera's forward direction ray
inline float3 GenerateForwardCameraRayDirection(in float4x4 projectionToWorldWithCameraAtOrigin)
{
	float2 screenPos = float2(0, 0);
	
	// Unproject the pixel coordinate into a world positon.
	float4 world = mul(float4(screenPos, 0, 1), projectionToWorldWithCameraAtOrigin);
	return normalize(world.xyz);
}

inline Ray GenerateForwardCameraRay(in float3 cameraPosition, in float4x4 projectionToWorldWithCameraAtOrigin)
{
    float2 screenPos = float2(0, 0);

    // Unproject the pixel coordinate into a world positon.
    float4 world = mul(float4(screenPos, 0, 1), projectionToWorldWithCameraAtOrigin);

    Ray ray;
    ray.origin = cameraPosition;
    // Since the camera's eye was at 0,0,0 in projectionToWorldWithCameraAtOrigin 
    // the world.xyz is the direction.
    ray.direction = normalize(world.xyz);

    return ray;
}


// Generate a ray in world space for a camera pixel corresponding to an index from the dispatched 2D grid.
inline Ray GenerateCameraRay(uint2 index, in float3 cameraPosition, in float4x4 projectionToWorldWithCameraAtOrigin, float2 jitter = float2(0, 0))
{
    float2 xy = index + 0.5f; // center in the middle of the pixel.
	xy += jitter;
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

    // Unproject the pixel coordinate into a world positon.
    float4 world = mul(float4(screenPos, 0, 1), projectionToWorldWithCameraAtOrigin);

    Ray ray;
    ray.origin = cameraPosition;
	// Since the camera's eye was at 0,0,0 in projectionToWorldWithCameraAtOrigin 
	// the world.xyz is the direction.
	ray.direction = normalize(world.xyz);

    return ray;
}

// Texture coordinates on a horizontal plane.
float2 TexCoords(in float3 position)
{
    return position.xz;
}

// Calculate ray differentials.
void CalculateRayDifferentials(out float2 ddx_uv, out float2 ddy_uv, in float2 uv, in float3 hitPosition, in float3 surfaceNormal, in float3 cameraPosition, in float4x4 projectionToWorldWithCameraAtOrigin)
{
    // Compute ray differentials by intersecting the tangent plane to the  surface.
    Ray ddx = GenerateCameraRay(DispatchRaysIndex().xy + uint2(1, 0), cameraPosition, projectionToWorldWithCameraAtOrigin);
    Ray ddy = GenerateCameraRay(DispatchRaysIndex().xy + uint2(0, 1), cameraPosition, projectionToWorldWithCameraAtOrigin);

    // Compute ray differentials.
    float3 ddx_pos = ddx.origin - ddx.direction * dot(ddx.origin - hitPosition, surfaceNormal) / dot(ddx.direction, surfaceNormal);
    float3 ddy_pos = ddy.origin - ddy.direction * dot(ddy.origin - hitPosition, surfaceNormal) / dot(ddy.direction, surfaceNormal);

    // Calculate texture sampling footprint.
    ddx_uv = TexCoords(ddx_pos) - uv;
    ddy_uv = TexCoords(ddy_pos) - uv;
}

// Forward declaration.
float CheckersGridTextureBoxFilter(in float2 uv, in float2 dpdx, in float2 dpdy, in uint ratio);

// Return analytically integrated checkerboard texture (box filter).
float AnalyticalCheckersGridTexture(in float3 hitPosition, in float3 surfaceNormal, in float3 cameraPosition, in float4x4 projectionToWorldWithCameraAtOrigin )
{
    float2 ddx_uv;
    float2 ddy_uv;
    float2 uv = TexCoords(hitPosition);

    CalculateRayDifferentials(ddx_uv, ddy_uv, uv, hitPosition, surfaceNormal, cameraPosition, projectionToWorldWithCameraAtOrigin);
    return CheckersGridTextureBoxFilter(uv, ddx_uv, ddy_uv, 50);
}

// Fresnel reflectance - schlick approximation.
float3 FresnelReflectanceSchlick(in float3 I, in float3 N, in float3 F0)
{
    float cosi = saturate(dot(-I, N));
    return F0 + (1 - F0)*pow(1 - cosi, 5);
}

float3 RemoveSRGB(float3 x)
{
	return x < 0.04045 ? x / 12.92 : pow((x + 0.055) / 1.055, 2.4);
}

float3 ApplySRGB(float3 x)
{
	return x < 0.0031308 ? 12.92 * x : 1.055 * pow(abs(x), 1.0 / 2.4) - 0.055;
}

uint SmallestPowerOf2GreaterThan(in uint x)
{
    // Set all the bits behind the most significant non-zero bit in x to 1.
    // Essentially giving us the largest value that is smaller than the
    // next power of 2 we're looking for.
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;

    // Return the next power of two value.
    return x + 1;
}

// Returns float precision for a given float value.
// Values within (value -precision, value + precision) map to the same value. 
// Precision = exponentRange/MaxMantissaValue = (2^e+1 - 2^e) / (2^NumMantissaBits)
// Ref: https://blog.demofox.org/2017/11/21/floating-point-precision/
float FloatPrecision(in float x, in uint NumMantissaBits)
{
    // Find the exponent range the value is in.
    uint nextPowerOfTwo = SmallestPowerOf2GreaterThan(x);
    float exponentRange = nextPowerOfTwo - (nextPowerOfTwo >> 1);

    float MaxMantissaValue = 1 << NumMantissaBits;

    return exponentRange / MaxMantissaValue;
}

float FloatPrecisionR10(in float x)
{
    return FloatPrecision(x, 5);
}

float FloatPrecisionR16(in float x)
{
    return FloatPrecision(x, 10);
}

float FloatPrecisionR32(in float x)
{
    return FloatPrecision(x, 23);
}


/***************************************************************/
// Normal encoding
// Ref: https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
float2 OctWrap(float2 v)
{
    return (1.0 - abs(v.yx)) * (v.xy >= 0.0 ? 1.0 : -1.0);
}

// Converts a 3D unit vector to a 2D vector with <0,1> range. 
float2 EncodeNormal(float3 n)
{
    n /= (abs(n.x) + abs(n.y) + abs(n.z));
    n.xy = n.z >= 0.0 ? n.xy : OctWrap(n.xy);
    n.xy = n.xy * 0.5 + 0.5;
    return n.xy;
}

float3 DecodeNormal(float2 f)
{
    f = f * 2.0 - 1.0;

    // https://twitter.com/Stubbesaurus/status/937994790553227264
    float3 n = float3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
    float t = saturate(-n.z);
    n.xy += n.xy >= 0.0 ? -t : t;
    return normalize(n);
}
/***************************************************************/



// Pack [0.0, 1.0] float to 8 bit uint. 
uint Pack_R8_FLOAT(float r)
{
    return clamp(round(r * 255), 0, 255);
}

float Unpack_R8_FLOAT(uint r)
{
    return (r & 0xFF) / 255.0;
}

// pack two 8 bit uint2 into a 16 bit uint.
uint Pack_R8G8_to_R16_UINT(in uint r, in uint g)
{
    return (r & 0xff) | ((g & 0xff) << 8);
}

void Unpack_R16_to_R8G8_UINT(in uint v, out uint r, out uint g)
{
    r = v & 0xFF;
    g = (v >> 8) & 0xFF;
}


// Pack unsigned floating point, where 
// - rgb.rg are in [0, 1] range stored as two 8 bit uints.
// - rgb.b in [0, FLT_16_BIT_MAX] range stored as a 16bit float.
uint Pack_R8G8B16_FLOAT(float3 rgb)
{
    uint r = Pack_R8_FLOAT(rgb.r);
    uint g = Pack_R8_FLOAT(rgb.g) << 8;
    uint b = f32tof16(rgb.b) << 16;
    return r | g | b;
}

float3 Unpack_R8G8B16_FLOAT(uint rgb)
{
    float r = Unpack_R8_FLOAT(rgb);
    float g = Unpack_R8_FLOAT(rgb >> 8);
    float b = f16tof32(rgb >> 16);
    return float3(r, g, b);
}

uint NormalizedFloat3ToByte3(float3 v)
{
    return
        (uint(v.x * 255) << 16) +
        (uint(v.y * 255) << 8) +
        uint(v.z * 255);
}

float3 Byte3ToNormalizedFloat3(uint v)
{
    return float3(
        (v >> 16) & 0xff,
        (v >> 8) & 0xff,
        v & 0xff) / 255;
}

// Encode normal and depth with 16 bits allocated for each.
uint EncodeNormalDepth_N16D16(in float3 normal, in float depth)
{
    float3 encodedNormalDepth = float3(EncodeNormal(normal), depth);
    return Pack_R8G8B16_FLOAT(encodedNormalDepth);
}


// Decoded 16 bit normal and 16bit depth.
void DecodeNormalDepth_N16D16(in uint packedEncodedNormalAndDepth, out float3 normal, out float depth)
{
    float3 encodedNormalDepth = Unpack_R8G8B16_FLOAT(packedEncodedNormalAndDepth);
    normal = DecodeNormal(encodedNormalDepth.xy);
    depth = encodedNormalDepth.z;
}

uint EncodeNormalDepth(in float3 normal, in float depth)
{
    return EncodeNormalDepth_N16D16(normal, depth);
}

void DecodeNormalDepth(in uint encodedNormalDepth, out float3 normal, out float depth)
{
    DecodeNormalDepth_N16D16(encodedNormalDepth, normal, depth);
}

void DecodeNormal(in uint encodedNormalDepth, out float3 normal)
{
    float depthDummy;
    DecodeNormalDepth_N16D16(encodedNormalDepth, normal, depthDummy);
}

void UnpackEncodedNormalDepth(in uint packedEncodedNormalDepth, out float2 encodedNormal, out float depth)
{
    float3 encodedNormalDepth = Unpack_R8G8B16_FLOAT(packedEncodedNormalDepth);
    encodedNormal = encodedNormalDepth.xy;
    depth = encodedNormalDepth.z;
}

/***************************************************************/
// 3D value noise
// Ref: https://www.shadertoy.com/view/XsXfRH
// The MIT License
// Copyright © 2017 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
float hash(float3 p)
{
    p = frac(p*0.3183099 + .1);
    p *= 17.0;
    return frac(p.x*p.y*p.z*(p.x + p.y + p.z));
}
/***************************************************************/


// Sample normal map, convert to signed, apply tangent-to-world space transform.
float3 BumpMapNormalToWorldSpaceNormal(float3 bumpNormal, float3 surfaceNormal, float3 tangent)
{
    // Compute tangent frame.
    surfaceNormal = normalize(surfaceNormal);
    tangent = normalize(tangent);

    float3 bitangent = normalize(cross(tangent, surfaceNormal));
    float3x3 tangentSpaceToWorldSpace = float3x3(tangent, bitangent, surfaceNormal);

    return mul(bumpNormal, tangentSpaceToWorldSpace);
}

// Calculate a tangent from triangle's vertices and their uv coordinates.
// Ref: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
float3 CalculateTangent(in float3 v0, in float3 v1, in float3 v2, in float2 uv0, in float2 uv1, in float2 uv2)
{
    // Calculate edges
    // Position delta
    float3 deltaPos1 = v1 - v0;
    float3 deltaPos2 = v2 - v0;

    // UV delta
    float2 deltaUV1 = uv1 - uv0;
    float2 deltaUV2 = uv2 - uv0;

    float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
    return (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
}

float3 RayPlaneIntersection(float3 planeOrigin, float3 planeNormal, float3 rayOrigin, float3 rayDirection)
{
    float d = -dot(planeNormal, planeOrigin);
    float t = (-dot(planeNormal, rayOrigin) - d) / dot(planeNormal, rayDirection);
    return rayOrigin + t * rayDirection;
}

bool Inverse2x2(float2x2 mat, out float2x2 inverse)
{
    float determinant = mat[0][0] * mat[1][1] - mat[1][0] * mat[0][1];

    float rcpDeterminant = rcp(determinant);
    inverse[0][0] = mat[1][1];
    inverse[1][1] = mat[0][0];
    inverse[1][0] = -mat[0][1];
    inverse[0][1] = -mat[1][0];
    inverse = rcpDeterminant * inverse;

    return abs(determinant) > 0.00000001;
}


/*
 Using implementation described in PBRT, finding the partial derivative of the (change in position)/(change in UV coordinates)
 a.k.a dp/du and dp/dv

 Given the 3 UV and 3 triangle points, this can be represented as a linear equation:

 (uv0.u - uv2.u, uv0.v - uv2.v)   (dp/du)   =     (p0 - p2)
 (uv1.u - uv2.u, uv1.v - uv2.v)   (dp/dv)   =     (p1 - p2)

 To solve for dp/du, we invert the 2x2 matrix on the left side to get

 (dp/du)   = (uv0.u - uv2.u, uv0.v - uv2.v)^-1  (p0 - p2)
 (dp/dv)   = (uv1.u - uv2.u, uv1.v - uv2.v)     (p1 - p2)
*/
void CalculateTrianglePartialDerivatives(float2 uv0, float2 uv1, float2 uv2, float3 p0, float3 p1, float3 p2, out float3 dpdu, out float3 dpdv)
{
    float2x2 linearEquation;
    linearEquation[0] = uv0 - uv2;
    linearEquation[1] = uv1 - uv2;

    float2x3 pointVector;
    pointVector[0] = p0 - p2;
    pointVector[1] = p1 - p2;

    float2x2 inverse;
    Inverse2x2(linearEquation, inverse);

    dpdu = inverse[0][0] * pointVector[0] + inverse[0][1] * pointVector[1];
    dpdv = inverse[1][0] * pointVector[0] + inverse[1][1] * pointVector[1];
}

/*
Using implementation described in PBRT, finding the derivative for the UVs (dU, dV)  in both the x and y directions

Given the original point and the offset points (pX and pY) + the partial derivatives, the linear equation can be formed:
Note described only with pX, but the same is also applied to pY

( dpdu.x, dpdv.x)          =   (pX.x - p.x)
( dpdu.y, dpdv.y)   (dU)   =   (pX.y - p.y)
( dpdu.z, dpdv.z)   (dV)   =   (pX.z - p.z)

Because the problem is over-constrained (3 equations and only 2 unknowns), we pick 2 channels. Since one of the equations can
be degenerate, we pick the other 2 - namely 2 with least magnitude in their cross product, which are conveniently available in n.
THen we solve for dU, dV by inverting the matrix

dU    =   ( dpdu.x, dpdv.x)^-1  (pX.x - p.x)
dV    =   ( dpdu.y, dpdv.y)     (pX.y - p.y)
*/
void CalculateUVDerivatives(float3 normal, float3 dpdu, float3 dpdv, float3 p, float3 pX, float3 pY, out float2 ddx, out float2 ddy)
{
    int2 indices;
    float3 absNormal = abs(normal);
    if (absNormal.x > absNormal.y && absNormal.x > absNormal.z)
    {
        indices = int2(1, 2);
    }
    else if (absNormal.y > absNormal.z)
    {
        indices = int2(0, 2);
    }
    else
    {
        indices = int2(0, 1);
    }

    float2x2 linearEquation;
    linearEquation[0] = float2(dpdu[indices.x], dpdv[indices.x]);
    linearEquation[1] = float2(dpdu[indices.y], dpdv[indices.y]);

    float2x2 inverse;
    Inverse2x2(linearEquation, inverse);

    float2 pointOffset = float2(pX[indices.x] - p[indices.x], pX[indices.y] - p[indices.y]);
    ddx = abs(mul(inverse, pointOffset));

    pointOffset = float2(pY[indices.x] - p[indices.x], pY[indices.y] - p[indices.y]);
    ddy = abs(mul(inverse, pointOffset));
}


void CalculateUVDerivatives(
    in float2 uv, in float3 hitPosition, in float3 triangleNormal,
    in float3 p0, in float3 p1, in float3 p2, 
    in float2 uv0, in float2 uv1, in float2 uv2,
    in float3 cameraPosition, in float4x4 projectionToWorldWithCameraAtOrigin,
    out float2 ddx, out float2 ddy)
{
    Ray ray10 = GenerateCameraRay(DispatchRaysIndex().xy + uint2(1, 0), cameraPosition, projectionToWorldWithCameraAtOrigin);
    Ray ray01 = GenerateCameraRay(DispatchRaysIndex().xy + uint2(0, 1), cameraPosition, projectionToWorldWithCameraAtOrigin);

    float3 xOffsetPoint = RayPlaneIntersection(hitPosition, triangleNormal, ray10.origin, ray10.direction);
    float3 yOffsetPoint = RayPlaneIntersection(hitPosition, triangleNormal, ray01.origin, ray01.direction);

    float3 dpdu, dpdv;
    CalculateTrianglePartialDerivatives(uv0, uv1, uv2, p0, p1, p2, dpdu, dpdv);
    CalculateUVDerivatives(triangleNormal, dpdu, dpdv, hitPosition, xOffsetPoint, yOffsetPoint, ddx, ddy);
}

// Calculate derivatives of texture coordinates on a given triangle.
void CalculateUVDerivatives(
    in float3 hitPosition, in float2 uv, in float3 tangent, in float3 bitangent, in float3 triangleNormal,
    in float3 p0,                      // Current ray's intersection point with the triangle.
    in Ray rx, in Ray ry,              // Auxilary rays
    in float2 uv0, in float2 uv1, in float2 uv2,    // UV coordinates at the triangle's vertices.
    in float3 cameraPosition, in float4x4 projectionToWorldWithCameraAtOrigin,
    out float2 ddx, out float2 ddy,    // UV derivatives
    out float3 px, out float3 py)      // Auxilary rays' intersection points with the triangle.)
{
    px = RayPlaneIntersection(hitPosition, triangleNormal, rx.origin, rx.direction);
    py = RayPlaneIntersection(hitPosition, triangleNormal, ry.origin, ry.direction);

    CalculateUVDerivatives(triangleNormal, tangent, bitangent, hitPosition, px, py, ddx, ddy);
}

// Retrieves auxilary camera rays offset by one pixel in x and y directions in screen space. 
void GetAuxilaryCameraRays(in float3 cameraPosition, in float4x4 projectionToWorldWithCameraAtOrigin, out Ray rx, out Ray ry)
{
    rx = GenerateCameraRay(DispatchRaysIndex().xy + uint2(1, 0), cameraPosition, projectionToWorldWithCameraAtOrigin);
    ry = GenerateCameraRay(DispatchRaysIndex().xy + uint2(0, 1), cameraPosition, projectionToWorldWithCameraAtOrigin);
}

float min4(in float4 v)
{
    return min(min(v.x, v.y), min(v.z, v.w));
}

float max4(in float4 v)
{
    return max(max(v.x, v.y), max(v.z, v.w));
}

uint2 Get2DQuadIndexOffset(in uint i)
{
    const uint2 indexOffsets[4] = { {0, 0}, {1, 0}, {0, 1}, {1, 1} };
    return indexOffsets[i];
}

uint GetIndexOfValueClosestToTheReference(in float refValue, in float2 vValues)
{
    float2 delta = abs(refValue - vValues);

    uint outIndex = delta[1] < delta[0] ? 1 : 0;

    return outIndex;
}

uint GetIndexOfValueClosestToTheReference(in float refValue, in float4 vValues)
{
    float4 delta = abs(refValue - vValues);

    uint outIndex = delta[1] < delta[0] ? 1 : 0;
    outIndex = delta[2] < delta[outIndex] ? 2 : outIndex;
    outIndex = delta[3] < delta[outIndex] ? 3 : outIndex;

    return outIndex;
}

// Remap partial depth derivatives at z0 from [1,1] pixel offset to a new pixel offset.
float2 RemapDdxy(in float z0, in float2 ddxy, in float2 pixelOffset)
{    
    // Perspective correction for non-linear depth interpolation.
    // Ref: https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/visibility-problem-depth-buffer-depth-interpolation
    // Given a linear depth interpolation for finding z at offset q along z0 to z1
    //      z =  1 / (1 / z0 * (1 - q) + 1 / z1 * q)
    // and z1 = z0 + ddxy, where z1 is at a unit pixel offset [1, 1]
    // z can be calculated via ddxy as
    //
    //      z = (z0 + ddxy) / (1 + (1-q) / z0 * ddxy) 
    float2 z = (z0 + ddxy) / (1 + ((1 - pixelOffset) / z0) * ddxy);
    return sign(pixelOffset) * (z - z0);
}

float GetDepthAtPixelOffset(in float z0, in float2 ddxy, in float2 pixelOffset)
{
    float2 newDdxy = RemapDdxy(z0, ddxy, pixelOffset);
    return z0 + dot(1, newDdxy);
}

// Returns an approximate surface dimensions covered in a pixel. 
// This is a simplified model assuming pixel to pixel view angles are the same.
// z - linear depth of the surface at the pixel
// ddxy - partial depth derivatives
// tan_a - tangent of a per pixel view angle 
float2 ApproximateProjectedSurfaceDimensionsPerPixel(in float z, in float2 ddxy, in float tan_a)
{
    // Surface dimensions for a surface parallel at z.
    float2 dx = tan_a * z;
    
    // Using Pythagorean theorem approximate the surface dimensions given the ddxy.
    float2 w = sqrt(dx * dx + ddxy * ddxy);

    return w;
}

// Ensure only valid samples are interpolated. 
// Defaults to an average or an invalid value if none are valid.
float InterpolateValidValues(
    in float4 weights,
    in float4 SampleValues,
    in float minWeight = 1e-6,
    in float invalidValue = RTAO::InvalidAOCoefficientValue)
{
    float4 validSamples = SampleValues != invalidValue;
    weights *= validSamples;
    float weightSum = dot(weights, 1);
    if (weightSum < minWeight)
    {
        float numValidSamples = dot(validSamples, 1);
        if (numValidSamples > 1e-6)
        {
            return dot(validSamples, SampleValues) / numValidSamples;
        }
        else
        {
            return invalidValue;
        }
    }

    return dot(weights, SampleValues) / dot(weights, 1);
}

#endif // RAYTRACINGSHADERHELPER_H