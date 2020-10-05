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

static const float PI = 3.14159265358979323846;

Texture2D depthMap : register(t0);
SamplerState sampleClamp : register(s0);

cbuffer SceneConstantBuffer : register(b0)
{
    float4   lightPosition;
    float4   cameraPosition;
    float4x4 viewInverse;
    float4x4 projInverse;
    float4x4 viewProjInverseAtNearZ1;
    float fogDensity;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 screenPos: TEXCOORD0;
    float4 worldPos : TEXCOORD1;
};

PSInput VSMain(float4 position : POSITION)
{
    PSInput result;

    result.position = position;
    result.screenPos = position;
    result.worldPos = mul(position, viewProjInverseAtNearZ1);

    return result;
}

// Analytic solution of in-scattering of light in participating media.
// Ref: http://blog.mmacklin.com/2010/05/29/in-scattering-demo/
// Models single scattering within isotropic media.
// Returns radiance intensity.
float InScatter(float3 rayStart, float3 rayDir, float3 lightPos, float dist)
{
    float scatterParamScale = 1.0f;
    float scatterProbability = 1.f / (4.f * PI); // normalization term for an isotropic phase function        
    float3 q = rayStart - lightPos;                // light to ray origin
    float b = dot(rayDir, q);
    float c = dot(q, q);

    // Evaluate integral.
    float s = 1.0f / sqrt(max(c - b * b, 0.00001f));
    float l = s * (atan((dist + b) * s) - atan(b*s));
    l *= scatterParamScale * scatterProbability;

    return l;
}

// Retrieves pixel's position in world space.
float3 CalculateWorldPositionFromDepthMap(float2 screenCoord)
{
    // Translate from homogeneous coords to texture coords.
    float2 depthTexCoord = 0.5f * screenCoord + 0.5f;
    depthTexCoord.y = 1.0f - depthTexCoord.y;

    float depth = depthMap.Sample(sampleClamp, depthTexCoord).r;

    float4 screenPos = float4(screenCoord.x, screenCoord.y, depth, 1.0);
    float4 viewPosition = mul(screenPos, projInverse);
    viewPosition /= viewPosition.w; // Perspective division
    float4 worldPosition = mul(viewPosition, viewInverse);

    return worldPosition.xyz;
}

// Calculate attentuation factor due to fog scaling with fog's density and distance travelled.
// Returns <0,1>
// - 1: no attenuation
// - 0: max attenuation
float FogAttenuation(float density, float distance)
{
    return saturate(1 / exp(distance*density));
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 pixelPosition = input.worldPos.xyz;
    float3 rayDir = normalize(pixelPosition - cameraPosition.xyz);
    float3 pixelAtDepthWorldPosition = CalculateWorldPositionFromDepthMap(input.screenPos.xy);
    float cameraToPixelDistance = length(pixelAtDepthWorldPosition - cameraPosition.xyz);
    cameraToPixelDistance = min(cameraToPixelDistance, 1000.0f);        // Cap distance for rays that don't hit any scene geometry.

                                                                        // Calculate scattered light intensity for the view ray.
    float scatteredLightIntensity = InScatter(pixelPosition, rayDir, lightPosition.xyz, cameraToPixelDistance);
    scatteredLightIntensity = pow(scatteredLightIntensity, 0.25f);      // Bump up low intensity values a bit to spread the light effect.
    float3 lightColor = float3(0.856f, 1.0f, 1.310f);                    // Blueish tint.
    float3 color = scatteredLightIntensity * lightColor;

    // Alpha blend the scattered light with the dest color from the scene render pass.
    // Apply a fog effect by attenuating dest color contribution.
    // Alpha blend operation: BLEND_ONE * BLEND_SRC_COLOR + SRC_ALPHA * BLEND_DEST_COLOR
    float destColorAttenuation = FogAttenuation(fogDensity, cameraToPixelDistance);
    return float4(color, destColorAttenuation);
}