RWTexture3D<uint4> InputTex : register(u0);
RWTexture3D<float4> SDFTex : register(u1);
RWTexture3D<uint4> OutputTex : register(u2);

cbuffer JFAConstants : register(b0) {
	float3 gridResolution; 
	uint stepSize; 
}


[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	InputTex[DTid] = uint4(stepSize, uint(gridResolution.x), 0, 255);
	OutputTex[DTid] = uint4(0, 255, 0, 255); 
	SDFTex[DTid] = uint4(0, 0, 1, 1); 
}