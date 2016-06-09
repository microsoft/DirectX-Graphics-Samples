//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#include "DoFRS.hlsli"

#define USE_LINEAR_Z 1

SamplerState PointSampler : register(s0);
SamplerState ClampSampler : register(s1);
SamplerState BilinearSampler: register(s2);

cbuffer ConstantBuffer : register(b0)
{
	float FocusCenter;
	float FocalSpread;
	float FocalMinDist;	// Closer than this is max blurriness
	float FocalMaxDist;	// Farther than this is also max blurriness
	float2 RcpBufferDim;
	uint2 FullDimension;
	int2 HalfDimensionMinusOne;
	uint2 TiledDimension;
	float2 InvTiledDimension;
	uint DebugMode;
	uint DisablePreFilter;
	float ForegroundRange;
	float RcpForegroundRange;
	float AntiSparkleFilterStrength;
}

#define DEPTH_FOREGROUND_RANGE 0.01
#define MATH_CONST_PI 3.1415926535897
#define MAX_COC_RADIUS 16.0
#define RING1_THRESHOLD 1.0
#define RING2_THRESHOLD 6.0
#define RING3_THRESHOLD 11.0

float Max3( float a, float b, float c) { return max(max(a, b), c); }
float Min3( float a, float b, float c) { return min(min(a, b), c); }
float Med3( float a, float b, float c) { return clamp(a, min(b, c), max(b, c)); }
float Max4( float a, float b, float c, float d) { return Max3(a, b, max(c, d)); }
float Min4( float a, float b, float c, float d) { return Min3(a, b, min(c, d)); }
float Max4( float4 vec ) { return Max4( vec.x, vec.y, vec.z, vec.w ); }
float Min4( float4 vec ) { return Min4( vec.x, vec.y, vec.z, vec.w ); }

float ComputeCoC( float Depth )
{
	return max(1.0 / sqrt(MATH_CONST_PI), MAX_COC_RADIUS * saturate( abs(Depth - FocusCenter) * FocalSpread ));
}

float ComputeSignedCoC( float Depth )
{
	return ComputeCoC(Depth) * sign(Depth - FocusCenter);
}

float BackgroundPercent( float Depth, float TileMinDepth )
{
	return saturate((Depth - TileMinDepth) * RcpForegroundRange - 1.0);
}

float ForegroundPercent( float Depth, float TileMinDepth )
{
	return 1.0 - BackgroundPercent(Depth, TileMinDepth);
}

float2 DepthCmp2( float Depth, float TileMinDepth )
{
	float depthCmp = BackgroundPercent(Depth, TileMinDepth);
	return float2(depthCmp, 1.0 - depthCmp);
}

float SampleAlpha( float CoC )
{
	return rcp(MATH_CONST_PI * CoC * CoC);
}

float ComputeRenormalizationFactor( float ForegroundDepth, float MaxCoC )
{
	float FgCoC = ComputeCoC(ForegroundDepth);
	float Rings = (FgCoC - 1) / 5;
	float NumSamples = 1 + saturate(Rings) * 8 + saturate(Rings - 1) * 16 + saturate(Rings - 2) * 24;
	return 2.0 * MATH_CONST_PI * FgCoC * FgCoC / NumSamples;
}

static const float2 s_Ring1[8] =
{
	{  6.000000,  0.000000 }, { -6.000000, -0.000000 },
	{  4.242641,  4.242641 }, { -4.242641, -4.242641 },
	{  0.000000,  6.000000 }, { -0.000000, -6.000000 },
	{ -4.242641,  4.242641 }, {  4.242641, -4.242641 },
}; //  s_Ring1

static const float2 s_Ring2[16] =
{
	{  11.000000,  0.000000 }, { -11.000000, -0.000000 },
	{  10.162675,  4.209518 }, { -10.162675, -4.209518 },
	{  7.778175,   7.778175 }, { -7.778175, -7.778175 },
	{  4.209518,  10.162675 }, { -4.209518, -10.162675 },
	{  0.000000,  11.000000 }, { -0.000000, -11.000000 },
	{ -4.209518,  10.162675 }, {  4.209518, -10.162675 },
	{ -7.778175,   7.778175 }, {  7.778175, -7.778175 },
	{ -10.162675,  4.209518 }, {  10.162675, -4.209518 },
}; //  s_Ring2

static const float2 s_Ring3[24] =
{
	{  16.000000,  0.000000 }, { -16.000000, -0.000000 },
	{  15.454813,  4.141105 }, { -15.454813, -4.141105 },
	{  13.856406,  8.000000 }, { -13.856406, -8.000000 },
	{  11.313708, 11.313708 }, { -11.313708, -11.313708 },
	{  8.000000,  13.856406 }, { -8.000000, -13.856406 },
	{  4.141105,  15.454813 }, { -4.141105, -15.454813 },
	{  0.000000,  16.000000 }, { -0.000000, -16.000000 },
	{ -4.141105,  15.454813 }, {  4.141105, -15.454813 },
	{ -8.000000,  13.856406 }, {  8.000000, -13.856406 },
	{ -11.313708, 11.313708 }, {  11.313708, -11.313708 },
	{ -13.856406,  8.000000 }, {  13.856406, -8.000000 },
	{ -15.454813,  4.141105 }, {  15.454813, -4.141105 },
}; //  s_Ring3

static const int s_Ring1Q[8] =
{
	3, -3,
	50, -50,
	72, -72,
	46, -46,
}; //  s_Ring1Q

static const int s_Ring2Q[16] =
{
	6, -6,
	53, -53,
	100, -100,
	122, -122,
	144, -144,
	118, -118,
	92, -92,
	43, -43,
}; //  s_Ring2Q

static const int s_Ring3Q[24] =
{
	8, -8,
	56, -56,
	103, -103,
	150, -150,
	172, -172,
	194, -194,
	192, -192,
	190, -190,
	164, -164,
	138, -138,
	89, -89,
	40, -40,
}; //  s_Ring3Q
