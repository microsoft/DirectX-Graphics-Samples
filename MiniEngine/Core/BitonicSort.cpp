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
#include "ReadbackBuffer.h"
#include "Math/Common.h"
#include "Math/Random.h"

#include "CompiledShaders/BitonicIndirectArgsCS.h"
#include "CompiledShaders/Bitonic32PreSortCS.h"
#include "CompiledShaders/Bitonic32InnerSortCS.h"
#include "CompiledShaders/Bitonic32OuterSortCS.h"
#include "CompiledShaders/Bitonic64PreSortCS.h"
#include "CompiledShaders/Bitonic64InnerSortCS.h"
#include "CompiledShaders/Bitonic64OuterSortCS.h"

namespace BitonicSort
{
    IndirectArgsBuffer s_DispatchArgs;

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
    void Shutdown(void);
}

void BitonicSort::Initialize( void )
{    
    s_DispatchArgs.Create(L"Bitonic sort dispatch args", 22*23/2, 12);

    s_RootSignature.Reset(4, 0);
    s_RootSignature[0].InitAsConstants(0, 2);
    s_RootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
    s_RootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
    s_RootSignature[3].InitAsConstants(1, 2);
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

void BitonicSort::Shutdown( void )
{
    s_DispatchArgs.Destroy();
}

void BitonicSort::Sort( 
    ComputeContext& Context,
    GpuBuffer& KeyIndexList,
    GpuBuffer& CounterBuffer,
    uint32_t CounterOffset,
    bool IsPartiallyPreSorted,
    bool SortAscending
)
{
    const uint32_t ElementSizeBytes = KeyIndexList.GetElementSize();
    const uint32_t MaxNumElements = KeyIndexList.GetElementCount();
    const uint32_t AlignedMaxNumElements = Math::AlignPowerOfTwo(MaxNumElements);
    const uint32_t MaxIterations = Math::Log2(std::max(2048u, AlignedMaxNumElements)) - 10;

    ASSERT(ElementSizeBytes == 4 || ElementSizeBytes == 8, "Invalid key-index list for bitonic sort");

    Context.SetRootSignature(s_RootSignature);

    // This controls two things.  It is a key that will sort to the end, and it is a mask used to
    // determine whether the current group should sort ascending or descending.
    Context.SetConstants(3, CounterOffset, SortAscending ? 0xffffffff : 0);

    // Generate execute indirect arguments
    Context.SetPipelineState(s_BitonicIndirectArgsCS);
    Context.TransitionResource(CounterBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(s_DispatchArgs, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.SetConstants(0, MaxIterations);
    Context.SetDynamicDescriptor(1, 0, CounterBuffer.GetSRV());
    Context.SetDynamicDescriptor(2, 0, s_DispatchArgs.GetUAV());
    Context.Dispatch(1, 1, 1);

    // Pre-Sort the buffer up to k = 2048.  This also pads the list with invalid indices
    // that will drift to the end of the sorted list.
    Context.TransitionResource(s_DispatchArgs, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
    Context.TransitionResource(KeyIndexList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.InsertUAVBarrier(KeyIndexList);
    Context.SetDynamicDescriptor(2, 0, KeyIndexList.GetUAV());

    if (!IsPartiallyPreSorted)
    {
        Context.SetPipelineState(ElementSizeBytes == 4 ? s_Bitonic32PreSortCS : s_Bitonic64PreSortCS);
        Context.DispatchIndirect(s_DispatchArgs, 0);
        Context.InsertUAVBarrier(KeyIndexList);
    }

    uint32_t IndirectArgsOffset = 12;

    // We have already pre-sorted up through k = 2048 when first writing our list, so
    // we continue sorting with k = 4096.  For unnecessarily large values of k, these
    // indirect dispatches will be skipped over with thread counts of 0.

    for (uint32_t k = 4096; k <= AlignedMaxNumElements; k *= 2)
    {
        Context.SetPipelineState(ElementSizeBytes == 4 ? s_Bitonic32OuterSortCS : s_Bitonic64OuterSortCS);

        for (uint32_t j = k / 2; j >= 2048; j /= 2)
        {
            Context.SetConstants(0, k, j);
            Context.DispatchIndirect(s_DispatchArgs, IndirectArgsOffset);
            Context.InsertUAVBarrier(KeyIndexList);
            IndirectArgsOffset += 12;
        }

        Context.SetPipelineState(ElementSizeBytes == 4 ? s_Bitonic32InnerSortCS : s_Bitonic64InnerSortCS);
        Context.DispatchIndirect(s_DispatchArgs, IndirectArgsOffset);
        Context.InsertUAVBarrier(KeyIndexList);
        IndirectArgsOffset += 12;
    }
}

template <typename T>
inline void VerifySort(T* List, uint32_t ListLength, bool bAscending)
{
    const T IndexMask = Math::AlignPowerOfTwo(ListLength) - 1;

    for (uint32_t i = 0; i < ListLength - 1; ++i)
    {
        ASSERT((List[i] & IndexMask) < ListLength, "Corrupted list index detected");

        if (bAscending)
        {
            ASSERT(List[i] <= List[i + 1], "Invalid sort order:  non-ascending");
        }
        else
        {
            ASSERT(List[i] >= List[i + 1], "Invalid sort order:  non-descending");
        }
    }

    ASSERT((List[ListLength - 1] & IndexMask) < ListLength, "Corrupted list index detected");
}

void TestBitonicSort(uint32_t ListSize, bool b64Bit, bool bAscending)
{
    const uint32_t SizeOfElem = b64Bit ? sizeof(uint64_t) : sizeof(uint32_t);
    const uint32_t IndexMask = Math::AlignPowerOfTwo(ListSize) - 1;

    // Allocate memory for list on CPU
    void* BufferPtr = std::malloc(ListSize * SizeOfElem);

    // Initialize list with random keys and valid indices
    if (b64Bit)
    {
        uint64_t* BufferPtr64 = (uint64_t*)BufferPtr;
        for (uint32_t i = 0; i < ListSize; ++i)
            BufferPtr64[i] = ((uint64_t)(uint32_t)Math::g_RNG.NextInt() << 32 | i);
    }
    else
    {
        uint32_t* BufferPtr32 = (uint32_t*)BufferPtr;
        for (uint32_t i = 0; i < ListSize; ++i)
            BufferPtr32[i] = (((uint32_t)Math::g_RNG.NextInt() & ~IndexMask) | i);
    }

    // Upload list to GPU
    ByteAddressBuffer RandomListGpu;
    RandomListGpu.Create(L"GPU Sort List", ListSize, SizeOfElem, BufferPtr);
    std::free(BufferPtr);

    // Put the list size in GPU memory
    __declspec(align(16)) uint32_t ListCounter[1] = { ListSize };
    ByteAddressBuffer RandomListCount;
    RandomListCount.Create(L"GPU List Counter", 1, sizeof(uint32_t), ListCounter);

    // Allocate readback buffers to view data with the CPU
    ReadbackBuffer IndirectArgs;
    IndirectArgs.Create(L"Readback Indirect Args", 22 * 23 / 2, 12);

    ReadbackBuffer ReadbackList;
    ReadbackList.Create(L"Random List For Sort", ListSize, SizeOfElem);

    // Begin GPU work of sorting.  Then copy results back to CPU.
    ComputeContext& Ctx = ComputeContext::Begin(L"Bitonic Sort Test");
    BitonicSort::Sort(Ctx, RandomListGpu, RandomListCount, 0, false, bAscending);
    Ctx.CopyBuffer(IndirectArgs, BitonicSort::s_DispatchArgs);
    Ctx.CopyBuffer(ReadbackList, RandomListGpu);
    Ctx.Finish(true);

    // For debug purposes, you may uncomment these lines to verify that the args were created correctly.
    //typedef uint32_t Args[3];
    //Args* iArgs = (Args*)IndirectArgs.Map();
    BufferPtr = ReadbackList.Map();

    // Scan through all items to ensure they are sorted in the proper order and that
    // their sort index is in the right range (uncorrupted).
    if (b64Bit)
        VerifySort((uint64_t*)BufferPtr, ListSize, bAscending);
    else
        VerifySort((uint32_t*)BufferPtr, ListSize, bAscending);

    ReadbackList.Unmap();
    //IndirectArgs.Unmap();
}

void BitonicSort::Test( void )
{
    for (uint32_t ThreadGroupCount = 1; ThreadGroupCount < 256; ++ThreadGroupCount)
    {
        uint32_t ListSize = 500 * ThreadGroupCount;
        TestBitonicSort(ListSize, true, true);
        TestBitonicSort(ListSize, true, false);
        TestBitonicSort(ListSize, false, true);
        TestBitonicSort(ListSize, false, false);
    }
}
