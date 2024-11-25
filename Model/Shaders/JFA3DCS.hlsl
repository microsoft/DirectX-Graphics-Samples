RWTexture3D<uint4> InputTex : register(u0);
RWTexture3D<float> SDFTex : register(u1);
RWTexture3D<uint4> OutputTex : register(u2);

cbuffer JFAConstants : register(b0) {
	float3 gridResolution;
	uint stepSize;
}


[numthreads(8, 8, 8)]  // Doesn't work!
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint4 currentPixel = InputTex[DTid];

	float bestDistance = 999999999999.;
	uint4 bestColor = uint4(0, 0, 0, 0); 

	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			for (int k = -1; k <= 1; k++) {
				uint3 neighbourCoord = DTid + (stepSize * uint3(i, j, k));

				if (any(neighbourCoord < uint3(0, 0, 0)) || any(neighbourCoord >= uint3(gridResolution.xyz))) {
					continue;
				}

				uint4 seed = InputTex[neighbourCoord];
				float dist = distance(seed.xyz, DTid);

				if ((seed.a != 0) && (dist < bestDistance)) {
					bestDistance = dist; 
					bestColor = seed;
				}
			}
		}
	}

	GroupMemoryBarrierWithGroupSync();
	SDFTex[DTid] = bestDistance;
	OutputTex[DTid] = bestColor; 
}