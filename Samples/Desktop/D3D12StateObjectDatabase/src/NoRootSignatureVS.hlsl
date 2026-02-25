Texture2D<float4> tex0 : register(t0);
float4 NoRootSignatureVS(float4 pos : POS) : SV_POSITION
{
    return tex0.Load(int3(0, 0, 0));
}