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
#include "MeshletCommon.hlsli"

float3 Luminosity(float3 color)
{
    return dot(color, float3(0.2126, 0.7152, 0.0722)).xxx;
}

[RootSignature(ROOT_SIG)]
float4 main(VertexOut pin) : SV_TARGET
{
    float ambientIntensity = 0.1;
    float3 lightColor = float3(1, 1, 1);
    float3 lightDir = -normalize(float3(1, -1, 1));

    float3 color = 0.8;
    float shininess = 16;

    // Per-meshlet color shading
    if (Constants.DrawMeshlets && (Instance.Flags & MESHLET_FLAG) != 0) 
    {
        color = float3(
            float(pin.MeshletIndex & 1),
            float(pin.MeshletIndex & 3) / 4,
            float(pin.MeshletIndex & 7) / 8
        );

        // When the user has cursored over a meshlet or has selected one, desaturate every other meshlet.
        if (Constants.HighlightedIndex != -1 ||
            Constants.SelectedIndex != -1)
        {
            // A meshlet is highlighted or selected

            if (pin.MeshletIndex != Constants.SelectedIndex &&
                pin.MeshletIndex != Constants.HighlightedIndex)
            {
                // This meshlet is not highlighted - desaturate its color.
                color = lerp(color, Luminosity(color) + 0.2, 0.8);
            }
        }
    }

    float3 normal = normalize(pin.Normal);

    // Do some fancy Blinn-Phong shading!
    float cosAngle   = saturate(dot(normal, lightDir));
    float3 viewDir   = -normalize(pin.PositionVS);
    float3 halfAngle = normalize(lightDir + viewDir);

    float blinnTerm = saturate(dot(normal, halfAngle));
    blinnTerm = cosAngle != 0.0 ? blinnTerm : 0.0;
    blinnTerm = pow(blinnTerm, shininess);

    float3 finalColor = (cosAngle + blinnTerm + ambientIntensity) * color;

    return float4(finalColor, 1);
}
