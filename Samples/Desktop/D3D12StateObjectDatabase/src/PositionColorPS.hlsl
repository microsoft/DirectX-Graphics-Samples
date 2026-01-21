struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

[RootSignature("RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)")]
float4 PositionColorPS(PSInput input) : SV_TARGET
{
    return input.color;
}