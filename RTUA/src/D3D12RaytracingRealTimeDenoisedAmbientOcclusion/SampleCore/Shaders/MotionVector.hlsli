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

// Desc: Motion vector calculation functions.
#ifndef MOTIONVECTOR_HLSL
#define MOTIONVECTOR_HLSL

#include "RaytracingShaderHelper.hlsli"

float GetPlaneConstant(in float3 planeNormal, in float3 pointOnThePlane)
{
    // Given a plane equation N * P + d = 0
    // d = - N * P
    return -dot(planeNormal, pointOnThePlane);
}

bool IsPointOnTheNormalSideOfPlane(in float3 P, in float3 planeNormal, in float3 pointOnThePlane)
{
    float d = GetPlaneConstant(planeNormal, pointOnThePlane);
    return dot(P, planeNormal) + d > 0;
}

float3 ReflectPointThroughPlane(in float3 P, in float3 planeNormal, in float3 pointOnThePlane)
{
    //           |
    //           |
    //  P ------ C ------ R
    //           |
    //           |
    // Given a point P, plane with normal N and constant d, the projection point C of P onto plane is:
    // C = P + t*N
    //
    // Then the reflected point R of P through the plane can be computed using t as:
    // R = P + 2*t*N

    // Given C = P + t*N, and C lying on the plane,
    // C*N + d = 0
    // then
    // C = - d/N
    // -d/N = P + t*N
    // 0 = d + P*N + t*N*N
    // t = -(d + P*N) / N*N

    float d = GetPlaneConstant(planeNormal, pointOnThePlane);
    float3 N = planeNormal;
    float t = -(d + dot(P, N)) / dot(N, N);

    return P + 2 * t * N;
}


// Reflects a point across a planar mirror. 
// Returns FLT_MAX if the input point is already behind the mirror.
float3 ReflectFrontPointThroughPlane(
    in float3 p,
    in float3 mirrorSurfacePoint,
    in float3 mirrorNormal)
{
    if (!IsPointOnTheNormalSideOfPlane(p, mirrorNormal, mirrorSurfacePoint))
    {
        return FLT_MAX;
    }

    return ReflectPointThroughPlane(p, mirrorNormal, mirrorSurfacePoint);
}

float3 GetWorldHitPositionInPreviousFrame(
    in float3 hitObjectPosition,
    in uint BLASInstanceIndex,
    in uint3 vertexIndices,
    in BuiltInTriangleIntersectionAttributes attr,
    out float3x4 _BLASTransform)
{
    // Variables prefixed with underscore _ denote values in the previous frame.

    // Calculate hit object position of the hit in the previous frame.
    float3 _hitObjectPosition;
    if (l_materialCB.isVertexAnimated)
    {
        float3 _vertices[3] = {
            l_verticesPrevFrame[vertexIndices[0]].position,
            l_verticesPrevFrame[vertexIndices[1]].position,
            l_verticesPrevFrame[vertexIndices[2]].position };
        _hitObjectPosition = HitAttribute(_vertices, attr);
    }
    else // non-vertex animated geometry 
    {
        _hitObjectPosition = hitObjectPosition;
    }

    // Transform the hit object position to world space.
    _BLASTransform = g_prevFrameBottomLevelASInstanceTransform[BLASInstanceIndex];
    return mul(_BLASTransform, float4(_hitObjectPosition, 1));
}

// Calculate a texture space motion vector from previous to current frame.
float2 CalculateMotionVector(
    in float3 _hitPosition,
    out float _depth,
    in uint2 DTid)
{
    // Variables prefixed with underscore _ denote values in the previous frame.
    float3 _hitViewPosition = _hitPosition - g_cb.prevFrameCameraPosition;
    float3 _cameraDirection = GenerateForwardCameraRayDirection(g_cb.prevFrameProjToViewCameraAtOrigin);
    _depth = dot(_hitViewPosition, _cameraDirection);

    // Calculate screen space position of the hit in the previous frame.
    float4 _clipSpacePosition = mul(float4(_hitPosition, 1), g_cb.prevFrameViewProj);
    float2 _texturePosition = ClipSpaceToTexturePosition(_clipSpacePosition);

    float2 xy = DispatchRaysIndex().xy + 0.5f;   // Center in the middle of the pixel.
    float2 texturePosition = xy / DispatchRaysDimensions().xy;

    return texturePosition - _texturePosition;
}

#endif // MOTIONVECTOR_HLSL