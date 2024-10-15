//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#ifndef CROSSBILATERALWEIGHTS_HLSLI
#define CROSSBILATERALWEIGHTS_HLSLI

#include "RaytracingShaderHelper.hlsli"

namespace CrossBilateral
{
    namespace Normal
    {
        struct Parameters
        {
            float Sigma;
            float SigmaExponent;
        };

        // Get cross bilateral normal based weights.
        float4 GetWeights(
            in float3 TargetNormal,
            in float3 SampleNormals[4],
            in Parameters Params)
        {
            float4 NdotSampleN = float4(
                dot(TargetNormal, SampleNormals[0]),
                dot(TargetNormal, SampleNormals[1]),
                dot(TargetNormal, SampleNormals[2]),
                dot(TargetNormal, SampleNormals[3]));

            // Apply adjustment scale to the dot product. 
            // Values greater than 1 increase tolerance scale 
            // for unwanted inflated normal differences,
            // such as due to low-precision normal quantization.
            NdotSampleN *= Params.Sigma;

            float4 normalWeights = pow(saturate(NdotSampleN), Params.SigmaExponent);

            return normalWeights;
        }
    }

    // Linear depth.
    namespace Depth
    {
        struct Parameters
        {
            float Sigma;
            float WeightCutoff;
            uint NumMantissaBits;
        };
               
        float4 GetWeights(
            in float TargetDepth,
            in float2 Ddxy,
            in float4 SampleDepths,
            in Parameters Params)
        {
            float depthThreshold = dot(1, abs(Ddxy));
            float depthFloatPrecision = FloatPrecision(TargetDepth, Params.NumMantissaBits);

            float depthTolerance = Params.Sigma * depthThreshold + depthFloatPrecision;
            float4 depthWeights = min(depthTolerance / (abs(SampleDepths - TargetDepth) + depthFloatPrecision), 1);
            depthWeights *= depthWeights >= Params.WeightCutoff;

            return depthWeights;
        }

        float4 GetWeights(
            in float TargetDepth,
            in float2 Ddxy,
            in float4 SampleDepths,
            in float2 SampleOffset, // offset in-between the samples to remap ddxy for.
            in Parameters Params)
        {
            float2 remappedDdxy = RemapDdxy(TargetDepth, Ddxy, SampleOffset);
            return GetWeights(TargetDepth, remappedDdxy, SampleDepths, Params);
        }
    }

    namespace Bilinear
    {
        // TargetOffset - offset from the top left ([0,0]) sample of the quad samples.
        float4 GetWeights(in float2 TargetOffset)
        {
            float4 bilinearWeights =
                float4(
                    (1 - TargetOffset.x) * (1 - TargetOffset.y),
                    TargetOffset.x * (1 - TargetOffset.y),
                    (1 - TargetOffset.x) * TargetOffset.y,
                    TargetOffset.x * TargetOffset.y);

            return bilinearWeights;
        }
    }

    namespace BilinearDepthNormal
    {
        struct Parameters
        {
            Normal::Parameters Normal;
            Depth::Parameters Depth;
        };

        float4 GetWeights(
            in float TargetDepth,
            in float3 TargetNormal,
            in float2 TargetOffset,
            in float2 Ddxy,
            in float4 SampleDepths,
            in float3 SampleNormals[4],
            in float2 SamplesOffset,
            Parameters Params)
        {
            float4 bilinearWeights = Bilinear::GetWeights(TargetOffset);
            float4 depthWeights = Depth::GetWeights(TargetDepth, Ddxy, SampleDepths, SamplesOffset, Params.Depth);
            float4 normalWeights = Normal::GetWeights(TargetNormal, SampleNormals, Params.Normal);

            return bilinearWeights * depthWeights * normalWeights;
        }

        float4 GetWeights(
            in float TargetDepth,
            in float3 TargetNormal,
            in float2 TargetOffset,
            in float2 Ddxy,
            in float4 SampleDepths,
            in float3 SampleNormals[4],
            Parameters Params)
        {
            float4 bilinearWeights = Bilinear::GetWeights(TargetOffset);
            float4 depthWeights = Depth::GetWeights(TargetDepth, Ddxy, SampleDepths, Params.Depth);
            float4 normalWeights = Normal::GetWeights(TargetNormal, SampleNormals, Params.Normal);

            return bilinearWeights * depthWeights * normalWeights;
        }
    }
}

#endif // CROSSBILATERALWEIGHTS_HLSLI