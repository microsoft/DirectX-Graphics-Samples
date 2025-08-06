#ifndef STAR_NEST_HLSL
#define STAR_NEST_HLSL

// Star Nest by Pablo Roman Andrioli
// Volumetric fractal clouds effect
float3 CalculateStarNest(float3 rayOrigin, float3 rayDir)
{
    float3 skyPosition = rayDir * 10.0f;
    
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
    float d = smoothstep(0.1, 0.9, fractalDensity);

    // Cloud color ramp
    float3 blue = float3(0.4, 0.6, 1.0); // Soft blue
    float3 pink = float3(1.0, 0.6, 0.8); // Soft pink
    float3 cloud = lerp(blue, pink, d);

    // Return cloud color and density (w component)
    return float4(cloud, d).xyz;
}

#endif // STAR_NEST_HLSL