struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

[RootSignature("RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)")]
PSInput PositionColorVS(float4 position : POSITION, float4 color : COLOR)
{
    PSInput result;

    result.position = position;
    result.color = color;

    return result;
}