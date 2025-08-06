//*********************************************************
// 
// Copyright (c) Pablo Roman Andrioli. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// This file contains an HLSL adaptation of the Star Nest
// volumetric fractal clouds effect formula originally
// authored by Pablo Roman Andrioli.
// 
//*********************************************************


#ifndef STAR_NEST_H
#define STAR_NEST_H

float3 CalculateStarNest(float3 skyPosition, float3 rayDir)
{
    // Volumetric fractal clouds effect
	const int FractalSampleSteps = 200;
	const int FractalIterations = 50;
	const float SampleSpacing = 0.05;
	const float FractalOffset = 0.53;

	float fractalDensity = 0;
	float fadeFactor = 1.0;

	for (int s = 0; s < FractalSampleSteps; ++s)
	{
		float3 p = skyPosition + rayDir * (s * SampleSpacing);
		p = abs(fmod(p, 2.0) - 1.0);

		float pa = 0;
		float a = 0;
		for (int i = 0; i < FractalIterations; ++i)
		{
			float invLen2 = 1.0 / dot(p, p);
			p = abs(p) * invLen2 - FractalOffset;
			float lenP = length(p);
			a += abs(lenP - pa);
			pa = lenP;
		}

		fractalDensity += a * fadeFactor;
		fadeFactor *= 0.9;
	}
	// Normalize & smooth fractal density
	fractalDensity = saturate(fractalDensity / (FractalSampleSteps * FractalIterations * 0.12));
	float smoothedDensity = smoothstep(0.1, 0.9, fractalDensity);
	return smoothedDensity;
}

#endif // STAR_NEST_H