//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT License (MIT).
//
//*********************************************************
//
// FillInstantiateArgs.hlsl
//
// GPU-side generator for the per-cluster
// D3D12_RTAS_OPERATION_INSTANTIATE_CLUSTER_TEMPLATES_ARGS array that
// drives the animated ball's per-frame INSTANTIATE pass.
//
// Replaces a CPU-side loop that previously had to:
//   1. Read per-cluster template GVAs back from GPU into CPU memory
//      (m_dxr2CommandList->BUILD_CLUSTER_TEMPLATES output, then
//      ExecuteCommandList + WaitForGpu + CopyBufferRegion + Map)
//   2. Write per-cluster args into a persistently-mapped UPLOAD heap
//      buffer
//
// In a real renderer with GPU-driven LOD / cluster selection, the args
// array would be emitted by a culling / selection compute shader that
// also runs on the GPU.  This is the simplest demonstration of that
// pattern: one thread per cluster, each thread writes one
// INSTANTIATE_CLUSTER_TEMPLATES_ARGS record at a fixed byte offset.
//
// Layout of D3D12_RTAS_OPERATION_INSTANTIATE_CLUSTER_TEMPLATES_ARGS
// (32 bytes per element, see d3d12.h):
//
//   offset  0 (4 B)   INT       GeometryIndexOffset       = 0
//   offset  4 (4 B)   UINT      ClusterIdOffset           = g_clusterIdOffset
//   offset  8 (8 B)   u64 GVA   ClusterTemplate           = g_templateGvas[idx]
//   offset 16 (8 B)   u64 GVA   VertexBuffer.StartAddress = g_pfVbGva + g_vertexOffsets[idx]
//   offset 24 (8 B)   u64       VertexBuffer.StrideInBytes = g_vertexStride
//
// Buffers are bound via root descriptors (no descriptor heap involvement)
// so this pass is self-contained -- caller does NOT need to bind a
// descriptor heap before Dispatch.
//
//---------------------------------------------------------------------------

cbuffer Constants : register(b0)
{
    uint  g_pfVbGvaLo;       // low 32 bits of per-frame vertex buffer GPU VA
    uint  g_pfVbGvaHi;       // high 32 bits
    uint  g_clusterCount;    // total number of args to emit
    int   g_clusterIdOffset; // baseline cluster-ID offset (default 800)
    uint  g_vertexStride;    // sizeof(float3) = 12
};

// Input: per-cluster template GVAs (8 bytes each).  Produced by
// ExecuteIndirectRTASOperations(BUILD_CLUSTER_TEMPLATES) into obj.templateAddressArray.
// Bound as RWByteAddressBuffer (we only read) so we can reuse it without a
// state transition -- the producer leaves it in UNORDERED_ACCESS, a UAV
// barrier separates the two passes.
RWByteAddressBuffer g_templateGvas   : register(u1);

// Input: per-cluster byte offsets into the per-frame vertex buffer
// (4 bytes each, cumulative prefix sum of per-cluster vertex sizes,
// pre-computed CPU-side at setup time).
ByteAddressBuffer   g_vertexOffsets  : register(t0);

// Output: D3D12_RTAS_OPERATION_INSTANTIATE_CLUSTER_TEMPLATES_ARGS array.
RWByteAddressBuffer g_argsOut        : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    const uint idx = tid.x;
    if (idx >= g_clusterCount) return;

    // Per-cluster template GVA (u64, 8 B at idx*8).
    const uint2 tmplLoHi = g_templateGvas.Load2(idx * 8u);

    // Per-cluster vertex-buffer byte offset (u32, 4 B at idx*4).
    const uint vbOffset = g_vertexOffsets.Load(idx * 4u);

    // 64-bit add: pfVbGva + vbOffset.  HLSL has no native u64, so do
    // it as two u32s with explicit carry.
    const uint addLo = g_pfVbGvaLo + vbOffset;
    const uint carry = (addLo < g_pfVbGvaLo) ? 1u : 0u;
    const uint vbStartLo = addLo;
    const uint vbStartHi = g_pfVbGvaHi + carry;

    const uint baseByte = idx * 32u;
    g_argsOut.Store (baseByte +  0, 0u);                            // GeometryIndexOffset = 0
    g_argsOut.Store (baseByte +  4, asuint(g_clusterIdOffset));     // ClusterIdOffset
    g_argsOut.Store2(baseByte +  8, tmplLoHi);                      // ClusterTemplate (u64)
    g_argsOut.Store2(baseByte + 16, uint2(vbStartLo, vbStartHi));   // VertexBuffer.StartAddress (u64)
    g_argsOut.Store2(baseByte + 24, uint2(g_vertexStride, 0u));     // VertexBuffer.StrideInBytes (u64)
}
