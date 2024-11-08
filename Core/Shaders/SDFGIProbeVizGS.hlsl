cbuffer CameraData : register(b0)
{
    matrix viewProjMatrix;
    float3 cameraPos;
};

struct VS_OUTPUT {
    float4 pos : WORLD_POSITION;
    float intensity : COLOR;
};

struct GS_OUTPUT {
    float4 pos : SV_POSITION;
    float intensity : COLOR;
};

[maxvertexcount(6)]
void main(point VS_OUTPUT input[1], inout TriangleStream<GS_OUTPUT> triStream)
{
    float4 centerPos = input[0].pos;
    float size = 10.0f;

    // Quads will face the camera.
    float3 forward = normalize(cameraPos - centerPos.xyz);
    float3 up = float3(0, 1, 0);
    float3 right = normalize(cross(up, forward));
    up = cross(forward, right);

    float3 offsets[4] = {
        -right * size - up * size,
         right * size - up * size,
         right * size + up * size,
        -right * size + up * size
    };

    GS_OUTPUT output;
    
    // First triangle.
    // Bottom left.
    output.pos = mul(viewProjMatrix, centerPos + float4(offsets[0], 0.0f));
    output.intensity = input[0].intensity;
    triStream.Append(output);
     // Bottom right.
    output.pos = mul(viewProjMatrix, centerPos + float4(offsets[1], 0.0f));
    output.intensity = input[0].intensity;
    triStream.Append(output);
     // Top right.
    output.pos = mul(viewProjMatrix, centerPos + float4(offsets[2], 0.0f));
    output.intensity = input[0].intensity;
    triStream.Append(output);

    triStream.RestartStrip();

    // Second triangle.
    // Top right.
    output.pos = mul(viewProjMatrix, centerPos + float4(offsets[2], 0.0f));
    output.intensity = input[0].intensity;
    triStream.Append(output);
    // Top left.
    output.pos = mul(viewProjMatrix, centerPos + float4(offsets[3], 0.0f));
    output.intensity = input[0].intensity;
    triStream.Append(output);
    // Bottom left.
    output.pos = mul(viewProjMatrix, centerPos + float4(offsets[0], 0.0f));
    output.intensity = input[0].intensity;
    triStream.Append(output);
}
