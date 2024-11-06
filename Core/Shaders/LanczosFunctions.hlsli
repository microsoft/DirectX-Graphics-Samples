//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author(s):  James Stanard
//             Jack Elliott
//

float Sinc(float x)
{
    if (x == 0.0)
        return 1.0;
    else
        return sin(x) / x;
}

float Lanczos(float x, float a)
{
    static const float M_PI = 3.1415926535897932384626433832795;
    return abs(x) < a ? Sinc(x*M_PI) * Sinc(x/a*M_PI) : 0.0;
}

float4 ComputeWeights(float fracPart)
{
    float4 weights;
    weights.x = Lanczos(-1.0 - fracPart, 2.0);
    weights.y = Lanczos( 0.0 - fracPart, 2.0);
    weights.z = Lanczos( 1.0 - fracPart, 2.0);
    weights.w = Lanczos( 2.0 - fracPart, 2.0);
    return weights / dot(weights, 1);
}

float4 GetUpscaleFilterWeights(float offset)
{
    //return ComputeWeights(offset);

    // Precompute weights for 16 discrete offsets
    const float4 FilterWeights[16] =
    {
        ComputeWeights( 0.5 / 16.0),
        ComputeWeights( 1.5 / 16.0),
        ComputeWeights( 2.5 / 16.0),
        ComputeWeights( 3.5 / 16.0),
        ComputeWeights( 4.5 / 16.0),
        ComputeWeights( 5.5 / 16.0),
        ComputeWeights( 6.5 / 16.0),
        ComputeWeights( 7.5 / 16.0),
        ComputeWeights( 8.5 / 16.0),
        ComputeWeights( 9.5 / 16.0),
        ComputeWeights(10.5 / 16.0),
        ComputeWeights(11.5 / 16.0),
        ComputeWeights(12.5 / 16.0),
        ComputeWeights(13.5 / 16.0),
        ComputeWeights(14.5 / 16.0),
        ComputeWeights(15.5 / 16.0)
    };
    return FilterWeights[(uint)(offset * 16.0)];
}
