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
//

#include "pch.h"
#include "BitonicSort.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "CommandContext.h"
#include "Math/Common.h"

#include "CompiledShaders/BitonicIndirectArgsCS.h"
#include "CompiledShaders/Bitonic32PreSortCS.h"
#include "CompiledShaders/Bitonic32InnerSortCS.h"
#include "CompiledShaders/Bitonic32OuterSortCS.h"
#include "CompiledShaders/Bitonic64PreSortCS.h"
#include "CompiledShaders/Bitonic64InnerSortCS.h"
#include "CompiledShaders/Bitonic64OuterSortCS.h"

namespace BitonicSort
{
    RootSignature s_RootSignature;
    ComputePSO s_BitonicIndirectArgsCS;
    ComputePSO s_Bitonic32PreSortCS;
    ComputePSO s_Bitonic32InnerSortCS;
    ComputePSO s_Bitonic32OuterSortCS;
    ComputePSO s_Bitonic64PreSortCS;
    ComputePSO s_Bitonic64InnerSortCS;
    ComputePSO s_Bitonic64OuterSortCS;

    // Called once by Core to initialize shaders
    void Initialize(void);
}

void BitonicSort::Initialize( void )
{	
    s_RootSignature.Reset(4, 0);
    s_RootSignature[0].InitAsConstants(0, 2);
    s_RootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
    s_RootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
    s_RootSignature[3].InitAsConstants(1, 1);
    s_RootSignature.Finalize(L"Bitonic Sort");

#define CreatePSO( ObjName, ShaderByteCode ) \
    ObjName.SetRootSignature(s_RootSignature); \
    ObjName.SetComputeShader(ShaderByteCode, sizeof(ShaderByteCode) ); \
    ObjName.Finalize();

    CreatePSO(s_BitonicIndirectArgsCS, g_pBitonicIndirectArgsCS);
    CreatePSO(s_Bitonic32PreSortCS,    g_pBitonic32PreSortCS);
    CreatePSO(s_Bitonic32InnerSortCS,  g_pBitonic32InnerSortCS);
    CreatePSO(s_Bitonic32OuterSortCS,  g_pBitonic32OuterSortCS);
    CreatePSO(s_Bitonic64PreSortCS,    g_pBitonic64PreSortCS);
    CreatePSO(s_Bitonic64InnerSortCS,  g_pBitonic64InnerSortCS);
    CreatePSO(s_Bitonic64OuterSortCS,  g_pBitonic64OuterSortCS);

#undef CreatePSO
}

void BitonicSort::Sort( 
    ComputeContext& Context,
    GpuBuffer& KeyIndexList,
    GpuBuffer& CounterBuffer,
    uint32_t CounterOffset,
    IndirectArgsBuffer& IndirectArgsBuffer,
    bool IsPartiallyPreSorted,
    bool SortAscending
)
{
    const uint32_t ElementSizeBytes = KeyIndexList.GetElementSize();
    const uint32_t MaxNumElements = KeyIndexList.GetElementCount();
    const uint32_t AlignedMaxNumElements = Math::AlignPowerOfTwo(MaxNumElements);

    ASSERT(ElementSizeBytes == 4 || ElementSizeBytes == 8, "Invalid key-index list for bitonic sort");

    const uint32_t RequiredIndirectArgsSize = std::max((int)Math::Log2(AlignedMaxNumElements) - 10, 1) * 12;
    ASSERT(RequiredIndirectArgsSize <= IndirectArgsBuffer.GetBufferSize(),
        "Please allocate an indirect arguments buffer of at least %u bytes for bitonic sorting", RequiredIndirectArgsSize);

    Context.SetRootSignature(s_RootSignature);

    // This controls two things.  It is a key that will sort to the end, and it is a mask used to
    // determine whether the current group should sort ascending or descending.
    Context.SetConstants(3, SortAscending ? 0xffffffff : 0);

    // Generate execute indirect arguments
    Context.SetPipelineState(s_BitonicIndirectArgsCS);
    Context.TransitionResource(CounterBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(IndirectArgsBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.SetConstants(0, AlignedMaxNumElements, CounterOffset);
    Context.SetDynamicDescriptor(1, 0, CounterBuffer.GetSRV());
    Context.SetDynamicDescriptor(2, 0, IndirectArgsBuffer.GetUAV());
    Context.Dispatch(1, 1, 1);

    // For k=2048, use args at 0.  For each doubling of k thereafter, increment args by 12 bytes
    uint32_t IndirectArgsOffset = 0;

    // Pre-Sort the buffer up to k = 2048.  This also pads the list with invalid indices
    // that will drift to the end of the sorted list.
    Context.TransitionResource(IndirectArgsBuffer, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
    Context.TransitionResource(KeyIndexList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.InsertUAVBarrier(KeyIndexList);
    Context.SetDynamicDescriptor(2, 0, KeyIndexList.GetUAV());

    if (!IsPartiallyPreSorted)
    {
        Context.SetPipelineState(ElementSizeBytes == 4 ? s_Bitonic32PreSortCS : s_Bitonic64PreSortCS);
        Context.DispatchIndirect(IndirectArgsBuffer, IndirectArgsOffset);
        Context.InsertUAVBarrier(KeyIndexList);
    }

    // We have already pre-sorted up through k = 2048 when first writing our list, so
    // we continue sorting with k = 4096.  For unnecessarily large values of k, these
    // indirect dispatches will be skipped over with thread counts of 0.

    for (uint32_t k = 4096; k <= AlignedMaxNumElements; k *= 2)
    {
        Context.SetPipelineState(ElementSizeBytes == 4 ? s_Bitonic32OuterSortCS : s_Bitonic64OuterSortCS);
        IndirectArgsOffset += 12;

        for (uint32_t j = k / 2; j >= 2048; j /= 2)
        {
            Context.SetConstants(0, k, j);
            Context.DispatchIndirect(IndirectArgsBuffer, IndirectArgsOffset);
            Context.InsertUAVBarrier(KeyIndexList);
        }

        Context.SetPipelineState(ElementSizeBytes == 4 ? s_Bitonic32InnerSortCS : s_Bitonic64InnerSortCS);
        Context.DispatchIndirect(IndirectArgsBuffer, IndirectArgsOffset);
        Context.InsertUAVBarrier(KeyIndexList);
    }
}
