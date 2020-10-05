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
#include "Shared.h"
#include "Common.hlsli"

[RootSignature(ROOT_SIG)]
float4 main(VertexOut pin) : SV_TARGET
{
    const float ambientIntensity = 0.1;
    const float3 lightColor = float3(1, 1, 1);
    const float3 lightDir = -normalize(float3(1, -1, 1));

    float3 diffuseColor;
    float shininess;

    if (Constants.RenderMode == 0)
    {
        diffuseColor = 0.5;
        shininess = 4.0;
    }
    else if (Constants.RenderMode == 1)
    {
        uint meshletIndex = pin.MeshletIndex;
        diffuseColor = float3(
            float(meshletIndex & 1),
            float(meshletIndex & 3) / 4,
            float(meshletIndex & 7) / 8);
        shininess = 16.0;
    }
    else // Dynamic LOD spectrum (LOD 0 - red, LOD n - green)
    {
        diffuseColor = pin.Color.rgb;
        shininess = 8.0;
    }

    float3 normal = normalize(pin.Normal);

    // Do some fancy Blinn-Phong shading!
    float cosAngle = saturate(dot(normal, lightDir));
    float3 viewDir = -normalize(pin.PositionVS);
    float3 halfAngle = normalize(lightDir + viewDir);

    float blinnTerm = saturate(dot(normal, halfAngle));
    blinnTerm = cosAngle != 0.0 ? blinnTerm : 0.0;
    blinnTerm = pow(blinnTerm, shininess);

    float3 finalColor = (cosAngle + blinnTerm + ambientIntensity) * diffuseColor;

    return float4(finalColor, 1);
}
