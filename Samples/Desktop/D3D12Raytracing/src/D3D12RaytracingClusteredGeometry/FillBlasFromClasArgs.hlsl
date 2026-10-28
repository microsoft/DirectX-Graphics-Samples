//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT License (MIT).
//
//*********************************************************
//
// FillBlasFromClasArgs.hlsl
//
// GPU-side generator for the per-object
// D3D12_RTAS_OPERATION_BUILD_BLAS_FROM_CLAS_ARGS array.  Shared between
// the static multi-object case (N_obj entries, where each entry points at
// a sub-slice of the global m_clasAddressArray) and the animated single-
// object case (1 entry pointing at obj.perFrameClasAddressArray).
//
// Input metadata is a tightly-packed array of {clasCount, clasArrayGvaLo,
// clasArrayGvaHi} triples (12 bytes/entry).  The CPU builds this once,
// upload-heap, identical handling for both cases.
//
// Layout of D3D12_RTAS_OPERATION_BUILD_BLAS_FROM_CLAS_ARGS (16 bytes):
//
//   offset  0 (4 B)   UINT       ClasAddressCount
//   offset  4 (4 B)   UINT       ClasAddressStride
//   offset  8 (8 B)   u64 GVA    ClasAddressArray
//
//---------------------------------------------------------------------------

cbuffer Constants : register(b0)
{
    uint g_entryCount;       // number of args to emit (== number of input entries)
    uint g_clasAddressStride;// sizeof(D3D12_GPU_VIRTUAL_ADDRESS) = 8
};

// Input: per-entry {count, gvaLo, gvaHi} (12 B each).
ByteAddressBuffer   g_inputs  : register(t0);

// Output: D3D12_RTAS_OPERATION_BUILD_BLAS_FROM_CLAS_ARGS array (16 B/elem).
RWByteAddressBuffer g_argsOut : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    const uint idx = tid.x;
    if (idx >= g_entryCount) return;

    const uint3 inp = g_inputs.Load3(idx * 12u);
    const uint  count    = inp.x;
    const uint2 gvaLoHi  = uint2(inp.y, inp.z);

    const uint baseByte = idx * 16u;
    g_argsOut.Store (baseByte +  0, count);              // ClasAddressCount
    g_argsOut.Store (baseByte +  4, g_clasAddressStride);// ClasAddressStride
    g_argsOut.Store2(baseByte +  8, gvaLoHi);            // ClasAddressArray (u64)
}
