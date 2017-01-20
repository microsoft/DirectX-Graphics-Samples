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

#include "ShaderUtility.hlsli"
#include "MotionBlurRS.hlsli"

Texture2D<float2> ReprojectionBuffer : register(t0);
Texture2D<float4> InTemporalColor : register(t1);
Texture2D<float3> SrcColor : register(t2);
//Texture2D<float3> InTemporalMin : register(t3);
//Texture2D<float3> InTemporalMax : register(t4);

RWTexture2D<float4> OutTemporalColor : register(u0);		// color to save for next frame including its validity in alpha
RWTexture2D<uint> DstColor : register(u1);

SamplerState LinearSampler : register(s0);

cbuffer Constants : register(b0)
{
	float2 RcpBufferDim;	// 1 / width, 1 / height
	float TemporalBlendFactor;
	float RcpSpeedLimiter;
}

float3 TM( float3 rgb )
{
	return rgb / (1 + RGBToLuminance(rgb));
}

float3 ITM( float3 rgb )
{
	return rgb / (1 - RGBToLuminance(rgb));
}

float2 GetPreviousCoord( uint2 CurrentCoord, out float Speed )
{
	float2 Velocity = ReprojectionBuffer[CurrentCoord];
	Speed = length(Velocity);
	return float2(CurrentCoord + 0.5) + Velocity;
}

float3 ClipColor( float3 Color, float3 BoxMin, float3 BoxMax, float Dilation=1.0 )
{
	float3 BoxCenter = (BoxMax + BoxMin) * 0.5;
	float3 HalfDim = (BoxMax - BoxMin) * 0.5 * Dilation + 0.001;
	float3 Displacement = Color - BoxCenter;
	float3 Units = abs(Displacement / HalfDim);
	float MaxUnit = max(max(Units.x, Units.y), max(Units.z, 1.0));
	return BoxCenter + Displacement / MaxUnit;
}

float3 GetCurrentBBox(int2 curST, inout float3 curMin, inout float3 curMax)
{
	float3 center = SrcColor[curST].rgb;
	float3 top = SrcColor[curST - int2(0, 1)].rgb;
	float3 bot = SrcColor[curST + int2(0, 1)].rgb;
	curMin = min(center, min(top, bot));
	curMax = max(center, max(top, bot));
	float3 lef = SrcColor[curST - int2(1, 0)].rgb;
	float3 rig = SrcColor[curST + int2(1, 0)].rgb;
	curMin = min(curMin, min(lef, rig));
	curMax = max(curMax, max(lef, rig));

#ifdef MORE_SAMPLES
	float3 topL = SrcColor[curST - int2(1, 1)].rgb;
	float3 botR = SrcColor[curST + int2(1, 1)].rgb;
	curMin = min(center, min(topL, botR));
	curMax = max(center, max(topL, botR));
	float3 lefB = SrcColor[curST - int2(1, -1)].rgb;
	float3 rigT = SrcColor[curST + int2(1, -1)].rgb;
	curMin = min(curMin, min(lefB, rigT));
	curMax = max(curMax, max(lefB, rigT));
#endif

	return center;
}

void GetPreviousBBox(int2 prevST, inout float3 prevMin, inout float3 prevMax)
{
	//prevMin = InTemporalMin[prevST];
	//prevMax = InTemporalMax[prevST];

	prevMin = prevMax = InTemporalColor[prevST].rgb;
	float3 top = InTemporalColor[prevST - int2(0, 1)].rgb;
	float3 bot = InTemporalColor[prevST + int2(0, 1)].rgb;
	prevMin = min(prevMin, min(top, bot));
	prevMax = max(prevMax, max(top, bot));
	float3 lef = InTemporalColor[prevST - int2(1, 0)].rgb;
	float3 rig = InTemporalColor[prevST + int2(1, 0)].rgb;
	prevMin = min(prevMin, min(lef, rig));
	prevMax = max(prevMax, max(lef, rig));

#ifdef MORE_SAMPLES
	top = InTemporalColor[prevST - int2(1, 1)].rgb;
	bot = InTemporalColor[prevST + int2(1, 1)].rgb;
	prevMin = min(prevMin, min(top, bot));
	prevMax = max(prevMax, max(top, bot));
	lef = InTemporalColor[prevST - int2(1, -1)].rgb;
	rig = InTemporalColor[prevST + int2(1, -1)].rgb;
	prevMin = min(prevMin, min(lef, rig));
	prevMax = max(prevMax, max(lef, rig));
#endif
}

[RootSignature(MotionBlur_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
	float speed;
	float2 prevST = GetPreviousCoord(DTid.xy, speed);

	float3 thisColor;
	float4 lastColor;

	float3 minRGB, maxRGB;
	thisColor = GetCurrentBBox(DTid.xy, minRGB, maxRGB);

	lastColor = InTemporalColor.SampleLevel(LinearSampler, prevST * RcpBufferDim, 0);
	float3 prevMin, prevMax;
	GetPreviousBBox(prevST, prevMin, prevMax);

	// Do bounding boxes of this frame and last frame intersect?
	if (any(maxRGB < prevMin || prevMax < minRGB) || speed > 0.01)
		lastColor.rgb = ClipColor(lastColor.rgb, minRGB, maxRGB);

	lastColor.w *= saturate(1.0 - speed * RcpSpeedLimiter);
	float3 result = ITM(lerp(TM(thisColor), TM(lastColor.rgb), lastColor.w));

	DstColor[DTid.xy] = Pack_R11G11B10_FLOAT(result);

	// After reset: 1/2, 2/3, 3/4, 4/5, etc.
	lastColor.w = 1 - rcp(1 + rcp(1 - lastColor.w));

	OutTemporalColor[DTid.xy] = float4(result, min(TemporalBlendFactor, lastColor.w));
}