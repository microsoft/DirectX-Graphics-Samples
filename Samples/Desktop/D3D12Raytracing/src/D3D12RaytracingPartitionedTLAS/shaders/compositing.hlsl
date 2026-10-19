//*********************************************************
//
// Copyright (c) 2024-2025, NVIDIA CORPORATION. All rights reserved.
// Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// Compositing stages adapted from vk_partitioned_tlas/shaders/compositing.comp.glsl.
// See THIRD_PARTY_NOTICES.md for source revisions and acknowledgments.

// Four-pass image compositing for lighting, ambient occlusion, and edges.

// Channel flow; D3D12RaytracingPartitionedTLAS::Composite inserts UAV barriers
// between these dispatches. DepthImage.R stays the primary ray distance.
// Stage       TraceImage RGB / A            PrimaryImage RGB / A
// RayGen      secondary light or sky / AO   direct light / bit-cast grouping key
// Pass 0      combined lighting + fog / AO  unchanged
// Pass 1      unchanged / edge mask         combined lighting / same key
// Pass 2      stylized color / edge mask    unchanged
// Pass 3      unchanged                     unchanged; writes Output RGBA
// Pass 4      unchanged                     unchanged; downsamples Output to DisplayOutput
// The edge mask is 0 at edges and 1 elsewhere. The grouping key is read with
// asuint; value 255 skips lighting combination in pass 0 (background convention).
// Pass 4 runs only at render scale 2. At scale 1, Output aliases DisplayOutput.
RWTexture2D<float4> TraceImage : register(u0);
RWTexture2D<float4> PrimaryImage : register(u1);
RWTexture2D<float4> DepthImage : register(u2);
RWTexture2D<float4> Output : register(u3);
RWTexture2D<float4> DisplayOutput : register(u4);

cbuffer CompositingConstants : register(b0)
{
    uint PassId;
};

int increment(int existing)
{
    if(abs(existing) < 2) return existing + 1;
    if(abs(existing) < 5) return existing + 2;
    return existing + 4;
}

float grayscale(float3 color)
{
    return dot(color, float3(0.2126, 0.7152, 0.0722));
}

float3 medianFilter(int2 coord, int2 maximum)
{
    float3 neighbors[9];
    [unroll] for(int y = -1; y <= 1; ++y)
        [unroll] for(int x = -1; x <= 1; ++x)
            neighbors[(y + 1) * 3 + x + 1] = PrimaryImage[clamp(coord + int2(x, y), 0, maximum)].rgb;
    [unroll] for(int i = 0; i < 8; ++i)
        [unroll] for(int j = i + 1; j < 9; ++j)
            if(neighbors[i].r > neighbors[j].r)
            {
                float3 temporary = neighbors[i];
                neighbors[i] = neighbors[j];
                neighbors[j] = temporary;
            }
    return neighbors[4];
}

[numthreads(16, 16, 1)]
void Composite(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if(PassId == 4)
    {
        // Average the four internal-resolution pixels for each display pixel.
        uint displayWidth, displayHeight;
        DisplayOutput.GetDimensions(displayWidth, displayHeight);
        if(dispatchThreadID.x >= displayWidth || dispatchThreadID.y >= displayHeight) return;
        int2 displayCoord = int2(dispatchThreadID.xy);
        int2 source = displayCoord * 2;
        DisplayOutput[displayCoord] = 0.25 * (Output[source]
            + Output[source + int2(1, 0)] + Output[source + int2(0, 1)]
            + Output[source + int2(1, 1)]);
        return;
    }
    uint width, height;
    TraceImage.GetDimensions(width, height);
    if(dispatchThreadID.x >= width || dispatchThreadID.y >= height) return;
    int2 coord = int2(dispatchThreadID.xy);
    int2 maximum = int2(width - 1, height - 1);

    if(PassId == 0)
    {
        // Smooth AO within a grouping key, add primary and secondary lighting,
        // then apply distance fog. Preserve AO in A for other threads' reads.
        float4 imagePixel = TraceImage[coord];
        float4 primaryPixel = PrimaryImage[coord];
        uint centerId = asuint(primaryPixel.w);
        if(centerId == 255u) return;

        const int halfWindow = 20;
        float aoResult = imagePixel.w;
        float aoWeight = 1.0;
        [loop] for(int y = -halfWindow; y <= halfWindow; y = increment(y))
        {
            int localY = coord.y + y;
            if(localY < 0 || localY >= int(height)) continue;
            [loop] for(int x = -halfWindow; x <= halfWindow; x = increment(x))
            {
                int localX = coord.x + x;
                if(localX < 0 || localX >= int(width) || (x == 0 && y == 0)) continue;
                int2 localCoord = int2(localX, localY);
                if(asuint(PrimaryImage[localCoord].w) == centerId)
                {
                    float distance = sqrt(float(x * x + y * y));
                    float weight = (sqrt(2.0 * halfWindow * halfWindow) - distance)
                        / sqrt(2.0 * halfWindow * halfWindow);
                    aoResult += TraceImage[localCoord].w * weight;
                    aoWeight += weight;
                }
            }
        }
        aoResult /= aoWeight;
        float3 finalColor = imagePixel.rgb + aoResult * primaryPixel.rgb;
        float depth = DepthImage[coord].x;
        if(depth > 1000.0)
            finalColor = lerp(finalColor, 0.7.xxx, min(0.8, (depth - 1000.0) / 1000.0));
        TraceImage[coord] = float4(finalColor, imagePixel.w);
        return;
    }

    if(PassId == 1)
    {
        // Detect edges from neighboring keys. Preserve the keys in PrimaryImage.A
        // while saving combined RGB there as the input for the median filter.
        float depth = DepthImage[coord].x;
        uint centerId = asuint(PrimaryImage[coord].w);
        bool isEdge = false;
        [unroll] for(int y = -1; y <= 1; ++y)
            [unroll] for(int x = -1; x <= 1; ++x)
            {
                int2 nearby = coord + int2(x, y);
                if(any(nearby < 0) || any(nearby > maximum) || (x == 0 && y == 0)) continue;
                uint nearbyId = asuint(PrimaryImage[nearby].w);
                if(nearbyId < centerId || (depth < 10.0 && nearbyId != centerId))
                    isEdge = true;
            }
        float4 centerPixel = TraceImage[coord];
        TraceImage[coord] = float4(centerPixel.rgb, isEdge ? 0.0 : 1.0);
        PrimaryImage[coord] = float4(centerPixel.rgb, asfloat(centerId));
        return;
    }

    if(PassId == 2)
    {
        // Median-filter PrimaryImage RGB, quantize brightness, and blend with
        // unfiltered lighting by distance. TraceImage.A keeps the edge mask.
        float3 median = medianFilter(coord, maximum);
        float gray = grayscale(median);
        float level = floor(gray * 10.0) / (9.0 * max(gray, 0.00001));
        float3 quantized = median * level;
        float depth = DepthImage[coord].x;
        float4 centerPixel = TraceImage[coord];
        float blend = clamp((depth - 100.0) / 100.0, 0.5, 1.0);
        TraceImage[coord] = float4(lerp(quantized, centerPixel.rgb, blend), centerPixel.w);
        return;
    }

    if(PassId == 3)
    {
        // Sample the edge mask at three perturbed positions for the sketch
        // outline, darken RGB, and write the final opaque UNORM image.
        float depth = DepthImage[coord].x;
        float errorRange = depth > 5.0 ? 0.001 * (1.0 - min(1.0, (depth - 5.0) / 10.0)) : 0.001;
        float center = TraceImage[coord].w;
        if(center > 0.1 && center < 0.6) errorRange = 0.0;
        float2 uv = float2(coord) / float2(width, height);
        float2 uv0 = uv + errorRange * float2(sin(30.0 * uv.y), sin(30.0 * uv.x));
        float2 uv1 = uv + errorRange * float2(sin(30.0 * uv.y + 1.047), sin(30.0 * uv.x + 3.142));
        float2 uv2 = uv + errorRange * float2(sin(30.0 * uv.y + 2.094), sin(30.0 * uv.x + 1.571));
        float e0 = TraceImage[clamp(int2(uv0 * float2(width, height)), 0, maximum)].w;
        float e1 = TraceImage[clamp(int2(uv1 * float2(width, height)), 0, maximum)].w;
        float e2 = TraceImage[clamp(int2(uv2 * float2(width, height)), 0, maximum)].w;
        float depthWeight = saturate(depth / 100.0);
        float edge = max(e0 * e1 * e2, depthWeight);
        if(edge < 1.0) edge *= depthWeight;
        float3 color = TraceImage[coord].rgb * edge;
        color = saturate(color);
        Output[coord] = float4(color, 1.0);
    }
}
