/*
*  Note that SM6 is not supported in the integrated shader compiling functionality in Viusal Studio 2017.
*  In the VS project file, we specify the shaders will be built via a custom build script, CompileShader_SM6.bat.
*  Please refer to CompileShader_SM6.bat to see compiling commands. 
*  
*  You may need to modify the SDK paths in CompileShader_SM6.bat to match the installed SDK. By default the path is pointing to 15063 SDK.
*/

cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 orthProjMatrix;
    float2 mousePosition;
    float2 resolution;
    float time;
    uint renderMode;
    uint laneSize;
    float4 padding[10];
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
{
    PSInput result;

    result.position = mul(position, orthProjMatrix);
    result.color = color;

    return result;
}


// use this to generate grid-like texture
float texPattern(float2 position)
{
    float scale = 0.13;
    float t = sin(position.x * scale) + cos(position.y * scale);
    float c = smoothstep(0.0, 0.2, t*t);
    
    return c;
}


float4 PSMain(PSInput input) : SV_TARGET
{
    float4 outputColor;

    // Add grid-like texture pattern on top of the color
    float texP = texPattern(input.position.xy );
    outputColor = texP * input.color;

    switch (renderMode)
    {
        case 1:
        {
            // Just pass through the color we generate before
            break;
        }
        case 2:
        {
            // Example of query intrinsics: WaveGetLaneIndex
            // Gradiently color the wave block by their lane id. Black for the smallest lane id and White for the largest lane id.
            outputColor = WaveGetLaneIndex() / float(laneSize);
            break;
        }
        case 3:
        {
            // Example of query intrinsics: WaveIsFirstLane
            // Mark the first lane as white pixel
            if (WaveIsFirstLane())
                outputColor = float4(1., 1., 1., 1.);
            break;
        }
        case 4:
        {
            // Example of query intrinsics: WaveIsFirstLane
            // Mark the first active lane as white pixel. Mark the last active lane as red pixel.
            if (WaveIsFirstLane())
                outputColor = float4(1., 1., 1., 1.);
            if (WaveGetLaneIndex() == WaveActiveMax(WaveGetLaneIndex()))
                outputColor = float4(1., 0., 0., 1.);
            break;
        }
        case 5:
        {
            // Example of vote intrinsics: WaveActiveBallot
            // Active lanes ratios (# of total activelanes / # of total lanes).
            uint4 activeLaneMask = WaveActiveBallot(true);
            uint numActiveLanes = countbits(activeLaneMask.x) + countbits(activeLaneMask.y) + countbits(activeLaneMask.z) + countbits(activeLaneMask.w);
            float activeRatio = (float)numActiveLanes / float(laneSize);
            outputColor = float4(activeRatio, activeRatio, activeRatio, 1.0);
            break;
        }
        case 6:
        {
            // Example of wave broadcast intrinsics: WaveReadLaneFirst
            // Broadcast the color in first lan to the wave.
            outputColor = WaveReadLaneFirst(outputColor);
            break;
        }

        case 7:
        {
            // Example of wave reduction intrinsics: WaveActiveSum
            // Paint the wave with the averaged color inside the wave.
            uint4 activeLaneMask = WaveActiveBallot(true);
            uint numActiveLanes = countbits(activeLaneMask.x) + countbits(activeLaneMask.y) + countbits(activeLaneMask.z) + countbits(activeLaneMask.w);
            float4 avgColor = WaveActiveSum(outputColor) / float(numActiveLanes);
            outputColor = avgColor;
            break;
        }

        case 8:
        {
            // Example of wave scan intrinsics: WavePrefixSum
            // First, compute the prefix sum of distance each lane to first lane.
            // Then, use the prefix sum value to color each pixel.
            float4 basePos = WaveReadLaneFirst(input.position);
            float4 prefixSumPos = WavePrefixSum(input.position - basePos);
            
            // Get the number of total active lanes.
            uint4 activeLaneMask = WaveActiveBallot(true);
            uint numActiveLanes = countbits(activeLaneMask.x) + countbits(activeLaneMask.y) + countbits(activeLaneMask.z) + countbits(activeLaneMask.w);
            
            outputColor = prefixSumPos/numActiveLanes;

            break;
        }

        case 9:
        {
            // Example of Quad-Wide shuffle intrinsics: QuadReadAcrossX and QuadReadAcrossY
            // Color pixels based on their quad id:
            //  q0 -> red
            //  q1 -> green
            //  q2 -> blue
            //  q3 -> white
            //
            //   -------------> x
            //  |   [0] [1]
            //  |   [2] [3]
            //  V
            //  Y
            //
            float dx = QuadReadAcrossX(input.position.x)-input.position.x;
            float dy = QuadReadAcrossY(input.position.y)-input.position.y;

            
            // q0
            if (dx > 0 && dy > 0)
                outputColor = float4(1, 0, 0, 1);
            // q1
            else if (dx <0 && dy > 0)
                outputColor = float4(0, 1, 0, 1);
            // q2
            else if (dx > 0 && dy < 0)
                outputColor = float4(0, 0, 1, 1);
            // q3
            else if (dx < 0 && dy < 0)
                outputColor = float4(1, 1, 1, 1);
            else
                outputColor = float4(0, 0, 0, 1);

            break;
        }

        default:
        {
            break;
        }
    }
    
    return outputColor;
}
