//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT License (MIT).
//
//*********************************************************
//
// FillClusterTemplateArgs.hlsl
//
// GPU-side generator for the per-cluster
// D3D12_RTAS_OPERATION_BUILD_CLUSTER_TEMPLATES_FROM_TRIANGLES_ARGS array
// used during BUILD_CLUSTER_TEMPLATES for the animated showpiece ball.
//
// Layout (96 bytes per cluster, DXR2 spec v0.30):
//   bytes  0..79  -- D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS
//                    (same shape as the static path; see FillClasFromTrianglesArgs.hlsl)
//   bytes 80..87  -- u64 InstantiationBoundingBoxLimit (we always pass 0 --
//                    driver derives the AABB from the hint vertex AABB)
//   bytes 88..95  -- format-specific union:
//                    FLOAT32_3  = two zero reserved dwords
//                    COMPRESSED1 = {u32 template header, u32 zero padding}
//
// Before spec v0.30 the struct ended at byte 87.  Continuing to step by 88
// makes argument N+1 overwrite argument N's new format-specific tail; WARP
// then faults while decoding the first template build.
//
// Per-cluster metadata is simpler than the static path:
//   - ClusterID is implicit (= dispatch thread index)
//   - opaqueFlag is always 0 (no per-material opacity wiring for the
//     animated ball -- it's controlled per-instance by the TLAS desc)
//   - VertexBufferStride is always sizeof(float3)=12 because hint input is
//     FLOAT32_3 even when instantiated CLAS positions are stored as COMPRESSED1
//   - IndexBufferStride is always 1 (uint8 indices)
//
// So per-cluster input is just {triCount, vertCount, vbOff, ibOff}
// (16 bytes/cluster).
//
//---------------------------------------------------------------------------

cbuffer Constants : register(b0)
{
    uint g_baseGpuVaLo;
    uint g_baseGpuVaHi;
    uint g_clusterCount;
    uint g_positionTruncateBits;       // 0..23; ignored for COMPRESSED1
    uint g_useCompressedTemplate;      // 0 = FLOAT32_3, 1 = COMPRESSED1
    uint g_compressedTemplateHeader;   // packed exponent + x/y/z bit counts
};

ByteAddressBuffer   g_meta    : register(t0);  // 16 B/cluster
RWByteAddressBuffer g_argsOut : register(u0);  // 96 B/cluster (spec v0.30)

uint2 add64(uint gvaLo, uint gvaHi, uint offset)
{
    const uint sumLo = gvaLo + offset;
    const uint carry = (sumLo < gvaLo) ? 1u : 0u;
    return uint2(sumLo, gvaHi + carry);
}

uint pack16(uint lo, uint hi)
{
    return (lo & 0xFFFFu) | ((hi & 0xFFFFu) << 16);
}

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    const uint idx = tid.x;
    if (idx >= g_clusterCount) return;

    const uint4 m = g_meta.Load4(idx * 16u);  // {triCount, vertCount, vbOff, ibOff}
    const uint triCount  = m.x;
    const uint vertCount = m.y;
    const uint vbOff     = m.z;
    const uint ibOff     = m.w;

    const uint2 vbGva = add64(g_baseGpuVaLo, g_baseGpuVaHi, vbOff);
    const uint2 ibGva = add64(g_baseGpuVaLo, g_baseGpuVaHi, ibOff);

    const uint baseByte = idx * 96u;
    // --- TrianglesArgs sub-struct (80 bytes) ------------------------------
    g_argsOut.Store (baseByte +  0, idx);                                   // ClusterID = thread index
    g_argsOut.Store (baseByte +  4, 0u);                                    // ClusterFlags
    g_argsOut.Store (baseByte +  8, pack16(triCount, vertCount));           // TriCount/VertCount
    // The animated ball is single-region, so its geometry index and flags are zero.
    g_argsOut.Store (baseByte + 12, 0u);                                    // BaseGeometryIndexAndFlags
    g_argsOut.Store (baseByte + 16, 0u);                                    // OpacityMicromapBaseLocation
    g_argsOut.Store (baseByte + 20, pack16(12u, 1u));                       // VBStride=12, IBStride=1
    g_argsOut.Store (baseByte + 24, pack16(0u, 0u));                        // OMM IB Stride / GeomIdxStride
    const uint posTruncBits = (g_useCompressedTemplate != 0u) ? 0u : g_positionTruncateBits;
    g_argsOut.Store (baseByte + 28, pack16(posTruncBits, 0u));              // PosTruncBits/Pad
    g_argsOut.Store2(baseByte + 32, vbGva);                                 // VertexBuffer
    g_argsOut.Store2(baseByte + 40, ibGva);                                 // IndexBuffer
    g_argsOut.Store2(baseByte + 48, uint2(0, 0));                           // GeometryIndexAndFlagsArray
    g_argsOut.Store2(baseByte + 56, uint2(0, 0));                           // GeometryIndexAndFlagsIndexBuffer
    g_argsOut.Store2(baseByte + 64, uint2(0, 0));                           // OpacityMicromapArray
    g_argsOut.Store2(baseByte + 72, uint2(0, 0));                           // OpacityMicromapIndexBuffer
    // --- InstantiationBoundingBoxLimit (8 bytes) --------------------------
    g_argsOut.Store2(baseByte + 80, uint2(0, 0));                           // = 0 -> driver derives from hint
    // --- Format-specific union (8 bytes, spec v0.30) ----------------------
    if (g_useCompressedTemplate != 0u)
    {
        g_argsOut.Store (baseByte + 88, g_compressedTemplateHeader);        // COMPRESSED1 header
        g_argsOut.Store (baseByte + 92, 0u);                                // reserved padding
    }
    else
    {
        g_argsOut.Store2(baseByte + 88, uint2(0, 0));                       // FLOAT32_3 reserved args
    }
}
