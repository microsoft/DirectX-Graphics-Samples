#define RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT|ALLOW_STREAM_OUTPUT)," \
    "DescriptorTable(SRV(t0, numDescriptors=1))," \
    "DescriptorTable(UAV(u0, numDescriptors=2))," \
    "DescriptorTable(Sampler(s0, numDescriptors=2))"

Texture2D<float4> tex0 : register(t0);
[RootSignature(RootSig)]
float4 RootSignatureVS(float4 pos : POS) : SV_POSITION  
{
    return tex0.Load(int3(0, 0, 0));
}