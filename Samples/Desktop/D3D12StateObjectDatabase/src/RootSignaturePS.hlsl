#define RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT|ALLOW_STREAM_OUTPUT)," \
    "DescriptorTable(SRV(t0, numDescriptors=1))," \
    "DescriptorTable(UAV(u0, numDescriptors=2))," \
    "DescriptorTable(Sampler(s0, numDescriptors=2))"

[RootSignature(RootSig)]
float4 RootSignaturePS(float4 pos : SV_POSITION) : SV_TARGET 
{
    return pos;
}