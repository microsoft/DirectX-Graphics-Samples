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
// Author:  James Stanard
//

float W1(float x, float A)
{
    return x * x * ((A + 2) * x - (A + 3)) + 1.0;
}

float W2(float x, float A)
{
    return A * (x * (x * (x - 5) + 8) - 4);
}

float4 ComputeWeights(float d1, float A)
{
    return float4(W2(1.0 + d1, A), W1(d1, A), W1(1.0 - d1, A), W2(2.0 - d1, A));
}

float4 GetBicubicFilterWeights(float offset, float A)
{
    //return ComputeWeights(offset, A);

    // Precompute weights for 16 discrete offsets
    const float4 FilterWeights[16] =
    {
        ComputeWeights( 0.5 / 16.0, -0.5),
        ComputeWeights( 1.5 / 16.0, -0.5),
        ComputeWeights( 2.5 / 16.0, -0.5),
        ComputeWeights( 3.5 / 16.0, -0.5),
        ComputeWeights( 4.5 / 16.0, -0.5),
        ComputeWeights( 5.5 / 16.0, -0.5),
        ComputeWeights( 6.5 / 16.0, -0.5),
        ComputeWeights( 7.5 / 16.0, -0.5),
        ComputeWeights( 8.5 / 16.0, -0.5),
        ComputeWeights( 9.5 / 16.0, -0.5),
        ComputeWeights(10.5 / 16.0, -0.5),
        ComputeWeights(11.5 / 16.0, -0.5),
        ComputeWeights(12.5 / 16.0, -0.5),
        ComputeWeights(13.5 / 16.0, -0.5),
        ComputeWeights(14.5 / 16.0, -0.5),
        ComputeWeights(15.5 / 16.0, -0.5)
    };

    return FilterWeights[(uint)(offset * 16.0)];
}
