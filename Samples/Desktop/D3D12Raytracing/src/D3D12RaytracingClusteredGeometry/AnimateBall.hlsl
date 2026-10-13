//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT License (MIT).
//
//*********************************************************
//
// AnimateBall.hlsl
//
// Per-frame GPU compute pass that writes the deformed positions of the
// animated ball into its per-frame vertex buffer.
//
// Layout matches the per-frame buffer's expectations: one tightly-packed
// float3 per vertex, in cluster-major order (cluster 0's verts first,
// then cluster 1, etc.).  INSTANTIATE_CLUSTER_TEMPLATES per-cluster args
// (built once at init) reference slices of this buffer by byte offset.
//
// Buffers are bound via root descriptors (no descriptor heap involvement),
// so this pass is self-contained -- no SetDescriptorHeaps required.
//
//---------------------------------------------------------------------------

// Rest-pose positions: uploaded once at init from CPU, never modified.
// Tightly-packed float3 array (12 bytes/vertex).
ByteAddressBuffer   g_restPositions  : register(t0);

// Animated positions: GPU writes here every frame; INSTANTIATE_CLUSTER_TEMPLATES
// reads from it immediately after via a UAV barrier + state transition.
RWByteAddressBuffer g_animPositions  : register(u0);

// Per-frame constant-buffer slice:
//   t           = animation time in seconds (m_animSeconds)
//   vertexCount = total number of vertices to process (obj.totalVertexCount)
cbuffer Params : register(b0)
{
    float g_t;
    uint  g_vertexCount;
};

// Radial scale is 1 + amp * mean(sine waves), around the sphere's local origin.
// Keep kWobbleAmp synchronized with the template deformation envelope in C++.
static const float kWobbleAmp  = 0.20f;
static const float kWobbleFreq = 20.0f;

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x >= g_vertexCount) return;

    const uint byteOff = tid.x * 12u;
    float3 p = asfloat(g_restPositions.Load3(byteOff));

    float sx = sin(kWobbleFreq * p.x + g_t * 3.0f + 0.0f);
    float sy = sin(kWobbleFreq * p.y + g_t * 3.0f + 1.7f);
    float sz = sin(kWobbleFreq * p.z + g_t * 3.0f + 3.4f);
    float scale = 1.0f + kWobbleAmp * (sx + sy + sz) * (1.0f / 3.0f);

    g_animPositions.Store3(byteOff, asuint(p * scale));
}
