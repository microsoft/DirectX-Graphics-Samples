#include "FullscreenTriangle.hlsli"

cbuffer ToneMapConstants : register(b3)
{
    uint toneMapOperator;
    uint transferFunction;
    float exposure;
    float paperWhiteNits;
    float maxDisplayNits;
};

FullscreenVSOutput VSMain(uint vertexId : SV_VertexID)
{
    return FullscreenTriangleVS(vertexId);
}

float4 PSMain(FullscreenVSOutput input) : SV_TARGET
{
    float x = saturate(input.uv.x);
    float y = saturate(input.uv.y);

    if (abs(y - 0.5) < 0.002)
    {
        return float4(0.0, 0.0, 0.0, 1.0);
    }

    float maxLinear = y < 0.5 ? 1.0 : 9.0;
    float perceptualMax = pow(maxLinear, 1.0 / 2.2);
    float perceptualValue = x * perceptualMax;
    float sceneLinear = pow(perceptualValue, 2.2);

    if (y > 0.5)
    {
        float displayMaxSceneLinear = max(maxDisplayNits, max(paperWhiteNits, 1.0)) / max(paperWhiteNits, 1.0);
        float markerX = pow(saturate(displayMaxSceneLinear / 9.0), 1.0 / 2.2);

        if (displayMaxSceneLinear < 9.0)
        {
            float lineWidth = max(fwidth(x) * 2.0, 0.0015);
            float marker = 1.0 - smoothstep(lineWidth, lineWidth * 1.8, abs(x - markerX));
            sceneLinear = lerp(sceneLinear, 0.0, marker);
        }
    }

    return float4(sceneLinear.xxx, 1.0);
}
