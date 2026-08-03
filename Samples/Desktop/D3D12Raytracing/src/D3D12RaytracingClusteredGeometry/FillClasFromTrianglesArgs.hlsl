//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT License (MIT).
//
//*********************************************************
//
// FillClasFromTrianglesArgs.hlsl
//
// GPU-side generator for the per-cluster
// D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS array (the args
// the BUILD_CLAS pass reads).  CPU prepares a small per-cluster metadata
// buffer + a few global constants; this CS expands each metadata entry
// into the full 80-byte args struct.
//
// Per-cluster metadata layout (28 bytes/cluster, matches ClasArgsMeta in
// the C++ side):
//
//   offset  0 (4 B)   UINT   clusterID
//   offset  4 (4 B)   UINT   triangleCount   (must fit in u16)
//   offset  8 (4 B)   UINT   vertexCount     (must fit in u16)
//   offset 12 (4 B)   UINT   vbByteOffset    (from g_baseGpuVa)
//   offset 16 (4 B)   UINT   ibByteOffset    (from g_baseGpuVa)
//   offset 20 (4 B)   UINT   opaqueFlag      (0 or D3D12_RTAS_CLUSTERED_GEOMETRY_FLAG_OPAQUE)
//   offset 24 (4 B)   UINT   matRegionIdx    (becomes per-CLAS BaseGeometryIndex)
//
// Layout of D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS (80 bytes):
//
//   offset  0 (4 B)   UINT   ClusterID
//   offset  4 (4 B)   UINT   ClusterFlags                       = 0
//   offset  8 (2 B)   UINT16 TriangleCount
//   offset 10 (2 B)   UINT16 VertexCount
//   offset 12 (4 B)   UINT   BaseGeometryIndexAndFlags          = matRegionIdx | opaqueFlag
//   offset 16 (4 B)   UINT   OpacityMicromapBaseLocation        = 0
//   offset 20 (2 B)   UINT16 VertexBufferStride                 = g_vertexBufferStride
//   offset 22 (2 B)   UINT16 IndexBufferStride                  = 1
//   offset 24 (2 B)   UINT16 OpacityMicromapIndexBufferStride   = 0
//   offset 26 (2 B)   UINT16 GeometryIndexAndFlagsArrayStride   = 0
//   offset 28 (2 B)   UINT16 PositionTruncateBitCount           = g_positionTruncateBits
//   offset 30 (2 B)   UINT16 ReservedPadding                    = 0
//   offset 32 (8 B)   u64    VertexBuffer                       = g_baseGpuVa + vbByteOffset
//   offset 40 (8 B)   u64    IndexBuffer                        = g_baseGpuVa + ibByteOffset
//   offset 48 (8 B)   u64    GeometryIndexAndFlagsArray         = 0
//   offset 56 (8 B)   u64    GeometryIndexAndFlagsIndexBuffer   = 0
//   offset 64 (8 B)   u64    OpacityMicromapArray               = 0
//   offset 72 (8 B)   u64    OpacityMicromapIndexBuffer         = 0
//
//---------------------------------------------------------------------------

#define HLSL
#include "RaytracingHlslCompat.h"

cbuffer Constants : register(b0)
{
    uint g_baseGpuVaLo;
    uint g_baseGpuVaHi;
    uint g_clusterCount;
    uint g_vertexBufferStride;     // 12 (FLOAT32_3) or 0 (COMPRESSED1)
    uint g_positionTruncateBits;   // 0..23
};

// Input: 28 bytes/cluster of metadata.  Layout (matches CPU ClasArgsMeta):
//   offset  0 (4 B)   UINT   clusterID
//   offset  4 (4 B)   UINT   triangleCount   (must fit in u16)
//   offset  8 (4 B)   UINT   vertexCount     (must fit in u16)
//   offset 12 (4 B)   UINT   vbByteOffset    (from g_baseGpuVa)
//   offset 16 (4 B)   UINT   ibByteOffset    (from g_baseGpuVa)
//   offset 20 (4 B)   UINT   opaqueFlag      (0 or D3D12_RTAS_CLUSTERED_GEOMETRY_FLAG_OPAQUE)
//   offset 24 (4 B)   UINT   matRegionIdx    (24-bit unsigned, packed into bits 0..23
//                                              of BaseGeometryIndexAndFlags so
//                                              GeometryIndex() at hit time returns it)
ByteAddressBuffer   g_meta    : register(t0);

// Output: 80 bytes/cluster.
RWByteAddressBuffer g_argsOut : register(u0);

// Helper: 64-bit add of (gvaLo, gvaHi) + offset.
uint2 add64(uint gvaLo, uint gvaHi, uint offset)
{
    const uint sumLo = gvaLo + offset;
    const uint carry = (sumLo < gvaLo) ? 1u : 0u;
    return uint2(sumLo, gvaHi + carry);
}

// Helper: pack two u16s into one u32 (low/high).
uint pack16(uint lo, uint hi)
{
    return (lo & 0xFFFFu) | ((hi & 0xFFFFu) << 16);
}

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    const uint idx = tid.x;
    if (idx >= g_clusterCount) return;

    // Load 28 bytes of per-cluster metadata.
    const uint3 m0 = g_meta.Load3(idx * 28u +  0);  // {clusterID, triCount, vertCount}
    const uint3 m1 = g_meta.Load3(idx * 28u + 12);  // {vbOff, ibOff, opaqueFlag}
    const uint  m2 = g_meta.Load (idx * 28u + 24);  // {matRegionIdx}

    const uint  clusterID    = m0.x;
    const uint  triCount     = m0.y;
    const uint  vertCount    = m0.z;
    const uint  vbOff        = m1.x;
    const uint  ibOff        = m1.y;
    const uint  opaqueFlag   = m1.z;
    const uint  matRegionIdx = m2;

    const uint2 vbGva = add64(g_baseGpuVaLo, g_baseGpuVaHi, vbOff);
    const uint2 ibGva = add64(g_baseGpuVaLo, g_baseGpuVaHi, ibOff);

    // DXR2 spec v0.27+: geometry index occupies bits 0..23 and clustered
    // geometry flags occupy bits 31..30.  `opaqueFlag` is already the full
    // D3D12_RTAS_CLUSTERED_GEOMETRY_FLAG_OPAQUE mask (or zero), not a boolean.
    const uint baseGeomIdxAndFlags = (matRegionIdx & 0x00FFFFFFu) | opaqueFlag;

    const uint baseByte = idx * 80u;
    g_argsOut.Store (baseByte +  0, clusterID);                                                // ClusterID
    g_argsOut.Store (baseByte +  4, 0u);                                                       // ClusterFlags
    g_argsOut.Store (baseByte +  8, pack16(triCount, vertCount));                              // TriCount/VertCount
    g_argsOut.Store (baseByte + 12, baseGeomIdxAndFlags);                                      // BaseGeometryIndexAndFlags
    g_argsOut.Store (baseByte + 16, 0u);                                                       // OpacityMicromapBaseLocation
    g_argsOut.Store (baseByte + 20, pack16(g_vertexBufferStride, 1u));                         // VBStride/IBStride
    g_argsOut.Store (baseByte + 24, pack16(0u, 0u));                                           // OMM IB Stride / GeomIdxAndFlagsArrayStride
    g_argsOut.Store (baseByte + 28, pack16(g_positionTruncateBits, 0u));                       // PosTruncBits/Pad
    g_argsOut.Store2(baseByte + 32, vbGva);                                                    // VertexBuffer
    g_argsOut.Store2(baseByte + 40, ibGva);                                                    // IndexBuffer
    g_argsOut.Store2(baseByte + 48, uint2(0, 0));                                              // GeometryIndexAndFlagsArray
    g_argsOut.Store2(baseByte + 56, uint2(0, 0));                                              // GeometryIndexAndFlagsIndexBuffer
    g_argsOut.Store2(baseByte + 64, uint2(0, 0));                                              // OpacityMicromapArray
    g_argsOut.Store2(baseByte + 72, uint2(0, 0));                                              // OpacityMicromapIndexBuffer
}
