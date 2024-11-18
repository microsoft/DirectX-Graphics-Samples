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
// Author(s):	James Stanard

#include "Common.hlsli"
#include "Lighting.hlsli"

Texture2D<float3> texDiffuse		: register(t0);
Texture2D<float3> texSpecular		: register(t1);
//Texture2D<float4> texEmissive		: register(t2);
Texture2D<float3> texNormal			: register(t3);
//Texture2D<float4> texLightmap		: register(t4);
//Texture2D<float4> texReflection	: register(t5);
Texture2D<float> texSSAO			: register(t12);
Texture2D<float> texShadow			: register(t13);
Texture2DArray<float4> IrradianceAtlas : register(t21);

struct VSOutput
{
	sample float4 position : SV_Position;
	sample float3 worldPos : WorldPos;
	sample float2 uv : TexCoord0;
	sample float3 viewDir : TexCoord1;
	sample float3 shadowCoord : TexCoord2;
	sample float3 normal : Normal;
	sample float3 tangent : Tangent;
	sample float3 bitangent : Bitangent;
};

cbuffer SDFGIConstants : register(b2) {
	float4x4 RandomRotation;

	float3 GridSize;
    bool UseAtlas;

	float3 ProbeSpacing;
	uint ProbeAtlasBlockResolution;

	float3 SceneMinBounds;
	uint GutterSize;
};

struct MRT
{
	float3 Color : SV_Target0;
	float3 Normal : SV_Target1;
};

float2 signNotZero(float2 v) {
    return float2((v.x >= 0.0 ? 1.0 : -1.0), (v.y >= 0.0 ? 1.0 : -1.0));
}

float2 octEncode(float3 v) {
    float l1norm = abs(v.x) + abs(v.y) + abs(v.z);
    float2 result = v.xy * (1.0 / l1norm);
    
    if (v.z < 0.0) {
        result = (1.0 - abs(result.yx)) * signNotZero(result.xy);
    }
    
    return result;
}

float3 ShadeFragmentWithProbes(
    float3 fragmentWorldPos,       
    float3 normal                 
) {
	uint ProbeAtlasBlockResolution = 8;
    uint GutterSize = 1;
	float3 ProbeSpacing = float3(395, 395, 395);
	float3 GridSize = float3(10, 4, 6);
	float3 SceneMinBounds = float3(-1920.94592, -126.442497, -1182.80713);
	uint atlasWidth = 91;
	uint atlasHeight = 37;

    float3 localPos = (fragmentWorldPos - SceneMinBounds) / ProbeSpacing;
    float3 probeCoord = floor(localPos); 

    float3 interpWeight = frac(localPos);

    uint3 probeIndices[8] = {
        uint3(probeCoord),
        uint3(probeCoord + float3(1, 0, 0)),
        uint3(probeCoord + float3(0, 1, 0)),
        uint3(probeCoord + float3(1, 1, 0)),
        uint3(probeCoord + float3(0, 0, 1)),
        uint3(probeCoord + float3(1, 0, 1)),
        uint3(probeCoord + float3(0, 1, 1)),
        uint3(probeCoord + float3(1, 1, 1))
    };

    for (int i = 0; i < 8; ++i) {
        probeIndices[i] = clamp(probeIndices[i], uint3(0, 0, 0), uint3(GridSize) - 1);
    }

    float4 irradiance[8];
    for (int i = 0; i < 8; ++i) {
		// float2 encodedDir = octEncode(normalize(mul(RandomRotation, float4(probeIndices[i] - localPos, 1.0)).xyz));
		// float2 encodedDir = octEncode(normalize(mul(RandomRotation, float4(-normal, 1.0)).xyz));
        float2 encodedDir = octEncode(-normal);
		// float2 encodedDir = octEncode(normalize(float3(0.1, -0.7, -0.43)));
		// float2 mappedDir = encodedDir * 0.5 + 0.5;
		// return float3(mappedDir, 0);
		encodedDir = clamp(encodedDir, -1.0, 1.0);
        uint3 atlasCoord = probeIndices[i] * uint3(ProbeAtlasBlockResolution + GutterSize, ProbeAtlasBlockResolution + GutterSize, 1);
        float2 texCoord = atlasCoord.xy + uint2(
            (encodedDir.x * 0.5 + 0.5) * (ProbeAtlasBlockResolution - GutterSize),
            (encodedDir.y * 0.5 + 0.5) * (ProbeAtlasBlockResolution - GutterSize)
        );
		texCoord = texCoord / float2(atlasWidth, atlasHeight);

		irradiance[i] = IrradianceAtlas.SampleLevel(defaultSampler, float3(texCoord, probeIndices[i].z), 0);
		// irradiance[i] = IrradianceAtlas.SampleLevel(defaultSampler, float3(texCoord, 5), 0);
    }

    float4 resultIrradiance = lerp(
        lerp(lerp(irradiance[0], irradiance[1], interpWeight.x),
             lerp(irradiance[2], irradiance[3], interpWeight.x), interpWeight.y),
        lerp(lerp(irradiance[4], irradiance[5], interpWeight.x),
             lerp(irradiance[6], irradiance[7], interpWeight.x), interpWeight.y),
        interpWeight.z
    );

    return resultIrradiance.rgb;
	// return float3(1, 0, 0);
	// return IrradianceAtlas.SampleLevel(defaultSampler, float3(0.5, 0.5, 2), 0).rgb;
	// return probeIndices[5].xyz / GridSize;
	// return SceneMinBounds;
	// return interpWeight;
	// return fragmentWorldPos - SceneMinBounds;
	// return localPos;
	// return RandomRotation[2].xyz;
	// return probeCoord / GridSize;
}

[RootSignature(Renderer_RootSig)]
MRT main(VSOutput vsOutput)
{
	MRT mrt;

	uint2 pixelPos = uint2(vsOutput.position.xy);
# define SAMPLE_TEX(texName) texName.Sample(defaultSampler, vsOutput.uv)

    float3 diffuseAlbedo = SAMPLE_TEX(texDiffuse);
    float3 colorSum = 0;
    {
        float ao = texSSAO[pixelPos];
        colorSum += ApplyAmbientLight( diffuseAlbedo, ao, AmbientColor );
    }

    float gloss = 128.0;
    float3 normal;
    {
        normal = SAMPLE_TEX(texNormal) * 2.0 - 1.0;
        AntiAliasSpecular(normal, gloss);
        float3x3 tbn = float3x3(normalize(vsOutput.tangent), normalize(vsOutput.bitangent), normalize(vsOutput.normal));
        normal = normalize(mul(normal, tbn));
    }

    float3 specularAlbedo = float3( 0.56, 0.56, 0.56 );
    float specularMask = SAMPLE_TEX(texSpecular).g;
    float3 viewDir = normalize(vsOutput.viewDir);

	colorSum += ApplyDirectionalLight( diffuseAlbedo, specularAlbedo, specularMask, gloss, normal, viewDir, SunDirection, SunColor, vsOutput.shadowCoord, texShadow );

	ShadeLights(colorSum, pixelPos,
		diffuseAlbedo,
		specularAlbedo,
		specularMask,
		gloss,
		normal,
		viewDir,
		vsOutput.worldPos
	);

	mrt.Normal = normal;
	if (UseAtlas) {
		mrt.Color = ShadeFragmentWithProbes(vsOutput.worldPos, normalize(vsOutput.normal));
	} else {
		mrt.Color = colorSum;
	}
	return mrt;
}
