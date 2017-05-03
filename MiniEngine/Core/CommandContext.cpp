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
// Author:  James Stanard 
//

#include "pch.h"
#include "CommandContext.h"
#include "ColorBuffer.h"
#include "DepthBuffer.h"
#include "GraphicsCore.h"
#include "DescriptorHeap.h"
#include "EngineProfiling.h"

using namespace Graphics;


void ContextManager::DestroyAllContexts(void)
{
    for (uint32_t i = 0; i < 4; ++i)
        sm_ContextPool[i].clear();
}

CommandContext* ContextManager::AllocateContext(D3D12_COMMAND_LIST_TYPE Type)
{
    std::lock_guard<std::mutex> LockGuard(sm_ContextAllocationMutex);

    auto& AvailableContexts = sm_AvailableContexts[Type];

    CommandContext* ret = nullptr;
    if (AvailableContexts.empty())
    {
        ret = new CommandContext(Type);
        sm_ContextPool[Type].emplace_back(ret);
        ret->Initialize();
    }
    else
    {
        ret = AvailableContexts.front();
        AvailableContexts.pop();
        ret->Reset();
    }
    ASSERT(ret != nullptr);

    ASSERT(ret->m_Type == Type);

    return ret;
}

void ContextManager::FreeContext(CommandContext* UsedContext)
{
    ASSERT(UsedContext != nullptr);
    std::lock_guard<std::mutex> LockGuard(sm_ContextAllocationMutex);
    sm_AvailableContexts[UsedContext->m_Type].push(UsedContext);
}

void CommandContext::DestroyAllContexts(void)
{
    LinearAllocator::DestroyAll();
    DynamicDescriptorHeap::DestroyAll();
    g_ContextManager.DestroyAllContexts();
}

CommandContext& CommandContext::Begin( const std::wstring ID )
{
    CommandContext* NewContext = g_ContextManager.AllocateContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
    NewContext->SetID(ID);
    if (ID.length() > 0)
        EngineProfiling::BeginBlock(ID, NewContext);
    return *NewContext;
}

ComputeContext& ComputeContext::Begin(const std::wstring& ID, bool Async)
{
    ComputeContext& NewContext = g_ContextManager.AllocateContext(
        Async ? D3D12_COMMAND_LIST_TYPE_COMPUTE : D3D12_COMMAND_LIST_TYPE_DIRECT)->GetComputeContext();
    NewContext.SetID(ID);
    if (ID.length() > 0)
        EngineProfiling::BeginBlock(ID, &NewContext);
    return NewContext;
}

uint64_t CommandContext::Flush(bool WaitForCompletion)
{
    FlushResourceBarriers();

    ASSERT(m_CurrentAllocator != nullptr);

    uint64_t FenceValue = g_CommandManager.GetQueue(m_Type).ExecuteCommandList(m_CommandList);

    if (WaitForCompletion)
        g_CommandManager.WaitForFence(FenceValue);

    //
    // Reset the command list and restore previous state
    //

    m_CommandList->Reset(m_CurrentAllocator, nullptr);

    if (m_CurGraphicsRootSignature)
    {
        m_CommandList->SetGraphicsRootSignature(m_CurGraphicsRootSignature);
        m_CommandList->SetPipelineState(m_CurGraphicsPipelineState);
    }
    if (m_CurComputeRootSignature)
    {
        m_CommandList->SetComputeRootSignature(m_CurComputeRootSignature);
        m_CommandList->SetPipelineState(m_CurComputePipelineState);
    }

    BindDescriptorHeaps();

    return FenceValue;
}

uint64_t CommandContext::Finish( bool WaitForCompletion )
{
    ASSERT(m_Type == D3D12_COMMAND_LIST_TYPE_DIRECT || m_Type == D3D12_COMMAND_LIST_TYPE_COMPUTE);

    FlushResourceBarriers();

    if (m_ID.length() > 0)
        EngineProfiling::EndBlock(this);

    ASSERT(m_CurrentAllocator != nullptr);

    CommandQueue& Queue = g_CommandManager.GetQueue(m_Type);

    uint64_t FenceValue = Queue.ExecuteCommandList(m_CommandList);
    Queue.DiscardAllocator(FenceValue, m_CurrentAllocator);
    m_CurrentAllocator = nullptr;

    m_CpuLinearAllocator.CleanupUsedPages(FenceValue);
    m_GpuLinearAllocator.CleanupUsedPages(FenceValue);
    m_DynamicViewDescriptorHeap.CleanupUsedHeaps(FenceValue);
    m_DynamicSamplerDescriptorHeap.CleanupUsedHeaps(FenceValue);

    if (WaitForCompletion)
        g_CommandManager.WaitForFence(FenceValue);

    g_ContextManager.FreeContext(this);

    return FenceValue;
}

CommandContext::CommandContext(D3D12_COMMAND_LIST_TYPE Type) :
    m_Type(Type),
    m_DynamicViewDescriptorHeap(*this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
    m_DynamicSamplerDescriptorHeap(*this, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER),
    m_CpuLinearAllocator(kCpuWritable), 
    m_GpuLinearAllocator(kGpuExclusive)
{
    m_OwningManager = nullptr;
    m_CommandList = nullptr;
    m_CurrentAllocator = nullptr;
    ZeroMemory(m_CurrentDescriptorHeaps, sizeof(m_CurrentDescriptorHeaps));

    m_CurGraphicsRootSignature = nullptr;
    m_CurGraphicsPipelineState = nullptr;
    m_CurComputeRootSignature = nullptr;
    m_CurComputePipelineState = nullptr;
    m_NumBarriersToFlush = 0;
}

CommandContext::~CommandContext( void )
{
    if (m_CommandList != nullptr)
        m_CommandList->Release();
}

void CommandContext::Initialize(void)
{
    g_CommandManager.CreateNewCommandList(m_Type, &m_CommandList, &m_CurrentAllocator);
}

void CommandContext::Reset( void )
{
    // We only call Reset() on previously freed contexts.  The command list persists, but we must
    // request a new allocator.
    ASSERT(m_CommandList != nullptr && m_CurrentAllocator == nullptr);
    m_CurrentAllocator = g_CommandManager.GetQueue(m_Type).RequestAllocator();
    m_CommandList->Reset(m_CurrentAllocator, nullptr);

    m_CurGraphicsRootSignature = nullptr;
    m_CurGraphicsPipelineState = nullptr;
    m_CurComputeRootSignature = nullptr;
    m_CurComputePipelineState = nullptr;
    m_NumBarriersToFlush = 0;

    BindDescriptorHeaps();
}

void CommandContext::BindDescriptorHeaps( void )
{
    UINT NonNullHeaps = 0;
    ID3D12DescriptorHeap* HeapsToBind[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    for (UINT i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
        ID3D12DescriptorHeap* HeapIter = m_CurrentDescriptorHeaps[i];
        if (HeapIter != nullptr)
            HeapsToBind[NonNullHeaps++] = HeapIter;
    }

    if (NonNullHeaps > 0)
        m_CommandList->SetDescriptorHeaps(NonNullHeaps, HeapsToBind);
}

void GraphicsContext::SetRenderTargets( UINT NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[], D3D12_CPU_DESCRIPTOR_HANDLE DSV )
{
    m_CommandList->OMSetRenderTargets( NumRTVs, RTVs, FALSE, &DSV );
}

void GraphicsContext::SetRenderTargets(UINT NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[])
{
    m_CommandList->OMSetRenderTargets(NumRTVs, RTVs, FALSE, nullptr);
}

void GraphicsContext::BeginQuery(ID3D12QueryHeap* QueryHeap, D3D12_QUERY_TYPE Type, UINT HeapIndex)
{
    m_CommandList->BeginQuery(QueryHeap, Type, HeapIndex);
}

void GraphicsContext::EndQuery(ID3D12QueryHeap* QueryHeap, D3D12_QUERY_TYPE Type, UINT HeapIndex)
{
    m_CommandList->EndQuery(QueryHeap, Type, HeapIndex);
}

void GraphicsContext::ResolveQueryData(ID3D12QueryHeap* QueryHeap, D3D12_QUERY_TYPE Type, UINT StartIndex, UINT NumQueries, ID3D12Resource* DestinationBuffer, UINT64 DestinationBufferOffset)
{
    m_CommandList->ResolveQueryData(QueryHeap, Type, StartIndex, NumQueries, DestinationBuffer, DestinationBufferOffset);
}

void GraphicsContext::ClearUAV( GpuBuffer& Target )
{
    // After binding a UAV, we can get a GPU handle that is required to clear it as a UAV (because it essentially runs
    // a shader to set all of the values).
    D3D12_GPU_DESCRIPTOR_HANDLE GpuVisibleHandle = m_DynamicViewDescriptorHeap.UploadDirect(Target.GetUAV());
    const UINT ClearColor[4] = {};
    m_CommandList->ClearUnorderedAccessViewUint(GpuVisibleHandle, Target.GetUAV(), Target.GetResource(), ClearColor, 0, nullptr);
}

void ComputeContext::ClearUAV( GpuBuffer& Target )
{
    // After binding a UAV, we can get a GPU handle that is required to clear it as a UAV (because it essentially runs
    // a shader to set all of the values).
    D3D12_GPU_DESCRIPTOR_HANDLE GpuVisibleHandle = m_DynamicViewDescriptorHeap.UploadDirect(Target.GetUAV());
    const UINT ClearColor[4] = {};
    m_CommandList->ClearUnorderedAccessViewUint(GpuVisibleHandle, Target.GetUAV(), Target.GetResource(), ClearColor, 0, nullptr);
}

void GraphicsContext::ClearUAV( ColorBuffer& Target )
{
    // After binding a UAV, we can get a GPU handle that is required to clear it as a UAV (because it essentially runs
    // a shader to set all of the values).
    D3D12_GPU_DESCRIPTOR_HANDLE GpuVisibleHandle = m_DynamicViewDescriptorHeap.UploadDirect(Target.GetUAV());
    CD3DX12_RECT ClearRect(0, 0, (LONG)Target.GetWidth(), (LONG)Target.GetHeight());

    //TODO: My Nvidia card is not clearing UAVs with either Float or Uint variants.
    const float* ClearColor = Target.GetClearColor().GetPtr();
    m_CommandList->ClearUnorderedAccessViewFloat(GpuVisibleHandle, Target.GetUAV(), Target.GetResource(), ClearColor, 1, &ClearRect);
}

void ComputeContext::ClearUAV( ColorBuffer& Target )
{
    // After binding a UAV, we can get a GPU handle that is required to clear it as a UAV (because it essentially runs
    // a shader to set all of the values).
    D3D12_GPU_DESCRIPTOR_HANDLE GpuVisibleHandle = m_DynamicViewDescriptorHeap.UploadDirect(Target.GetUAV());
    CD3DX12_RECT ClearRect(0, 0, (LONG)Target.GetWidth(), (LONG)Target.GetHeight());

    //TODO: My Nvidia card is not clearing UAVs with either Float or Uint variants.
    const float* ClearColor = Target.GetClearColor().GetPtr();
    m_CommandList->ClearUnorderedAccessViewFloat(GpuVisibleHandle, Target.GetUAV(), Target.GetResource(), ClearColor, 1, &ClearRect);
}

void GraphicsContext::ClearColor( ColorBuffer& Target )
{
    m_CommandList->ClearRenderTargetView(Target.GetRTV(), Target.GetClearColor().GetPtr(), 0, nullptr);
}

void GraphicsContext::ClearDepth( DepthBuffer& Target )
{
    m_CommandList->ClearDepthStencilView(Target.GetDSV(), D3D12_CLEAR_FLAG_DEPTH, Target.GetClearDepth(), Target.GetClearStencil(), 0, nullptr );
}

void GraphicsContext::ClearStencil( DepthBuffer& Target )
{
    m_CommandList->ClearDepthStencilView(Target.GetDSV(), D3D12_CLEAR_FLAG_STENCIL, Target.GetClearDepth(), Target.GetClearStencil(), 0, nullptr);
}

void GraphicsContext::ClearDepthAndStencil( DepthBuffer& Target )
{
    m_CommandList->ClearDepthStencilView(Target.GetDSV(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, Target.GetClearDepth(), Target.GetClearStencil(), 0, nullptr);
}

void GraphicsContext::SetViewportAndScissor( const D3D12_VIEWPORT& vp, const D3D12_RECT& rect )
{
    ASSERT(rect.left < rect.right && rect.top < rect.bottom);
    m_CommandList->RSSetViewports( 1, &vp );
    m_CommandList->RSSetScissorRects( 1, &rect );
}

void GraphicsContext::SetViewport( const D3D12_VIEWPORT& vp )
{
    m_CommandList->RSSetViewports( 1, &vp );
}

void GraphicsContext::SetViewport( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT minDepth, FLOAT maxDepth )
{
    D3D12_VIEWPORT vp;
    vp.Width = w;
    vp.Height = h;
    vp.MinDepth = minDepth;
    vp.MaxDepth = maxDepth;
    vp.TopLeftX = x;
    vp.TopLeftY = y;
    m_CommandList->RSSetViewports( 1, &vp );
}

void GraphicsContext::SetScissor( const D3D12_RECT& rect )
{
    ASSERT(rect.left < rect.right && rect.top < rect.bottom);
    m_CommandList->RSSetScissorRects( 1, &rect );
}

void CommandContext::TransitionResource(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate)
{
    D3D12_RESOURCE_STATES OldState = Resource.m_UsageState;

    if (m_Type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
    {
        ASSERT((OldState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == OldState);
        ASSERT((NewState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == NewState);
    }

    if (OldState != NewState)
    {
        ASSERT(m_NumBarriersToFlush < 16, "Exceeded arbitrary limit on buffered barriers");
        D3D12_RESOURCE_BARRIER& BarrierDesc = m_ResourceBarrierBuffer[m_NumBarriersToFlush++];

        BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        BarrierDesc.Transition.pResource = Resource.GetResource();
        BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        BarrierDesc.Transition.StateBefore = OldState;
        BarrierDesc.Transition.StateAfter = NewState;

        // Check to see if we already started the transition
        if (NewState == Resource.m_TransitioningState)
        {
            BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
            Resource.m_TransitioningState = (D3D12_RESOURCE_STATES)-1;
        }
        else
            BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

        Resource.m_UsageState = NewState;
    }
    else if (NewState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
        InsertUAVBarrier(Resource, FlushImmediate);

    if (FlushImmediate || m_NumBarriersToFlush == 16)
        FlushResourceBarriers();
}

void CommandContext::BeginResourceTransition(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate)
{
    // If it's already transitioning, finish that transition
    if (Resource.m_TransitioningState != (D3D12_RESOURCE_STATES)-1)
        TransitionResource(Resource, Resource.m_TransitioningState);

    D3D12_RESOURCE_STATES OldState = Resource.m_UsageState;

    if (OldState != NewState)
    {
        ASSERT(m_NumBarriersToFlush < 16, "Exceeded arbitrary limit on buffered barriers");
        D3D12_RESOURCE_BARRIER& BarrierDesc = m_ResourceBarrierBuffer[m_NumBarriersToFlush++];

        BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        BarrierDesc.Transition.pResource = Resource.GetResource();
        BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        BarrierDesc.Transition.StateBefore = OldState;
        BarrierDesc.Transition.StateAfter = NewState;

        BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;

        Resource.m_TransitioningState = NewState;
    }

    if (FlushImmediate || m_NumBarriersToFlush == 16)
        FlushResourceBarriers();
}

void CommandContext::InsertUAVBarrier(GpuResource& Resource, bool FlushImmediate)
{
    ASSERT(m_NumBarriersToFlush < 16, "Exceeded arbitrary limit on buffered barriers");
    D3D12_RESOURCE_BARRIER& BarrierDesc = m_ResourceBarrierBuffer[m_NumBarriersToFlush++];

    BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    BarrierDesc.UAV.pResource = Resource.GetResource();

    if (FlushImmediate)
        FlushResourceBarriers();
}

void CommandContext::InsertAliasBarrier(GpuResource& Before, GpuResource& After, bool FlushImmediate)
{
    ASSERT(m_NumBarriersToFlush < 16, "Exceeded arbitrary limit on buffered barriers");
    D3D12_RESOURCE_BARRIER& BarrierDesc = m_ResourceBarrierBuffer[m_NumBarriersToFlush++];

    BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
    BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    BarrierDesc.Aliasing.pResourceBefore = Before.GetResource();
    BarrierDesc.Aliasing.pResourceAfter = After.GetResource();

    if (FlushImmediate)
        FlushResourceBarriers();
}

void CommandContext::WriteBuffer( GpuResource& Dest, size_t DestOffset, const void* BufferData, size_t NumBytes )
{
    ASSERT(BufferData != nullptr && Math::IsAligned(BufferData, 16));
    DynAlloc TempSpace = m_CpuLinearAllocator.Allocate( NumBytes, 512 );
    SIMDMemCopy(TempSpace.DataPtr, BufferData, Math::DivideByMultiple(NumBytes, 16));
    CopyBufferRegion(Dest, DestOffset, TempSpace.Buffer, TempSpace.Offset, NumBytes );
}

void CommandContext::FillBuffer( GpuResource& Dest, size_t DestOffset, DWParam Value, size_t NumBytes )
{
    DynAlloc TempSpace = m_CpuLinearAllocator.Allocate( NumBytes, 512 );
    __m128 VectorValue = _mm_set1_ps(Value.Float);
    SIMDMemFill(TempSpace.DataPtr, VectorValue, Math::DivideByMultiple(NumBytes, 16));
    CopyBufferRegion(Dest, DestOffset, TempSpace.Buffer, TempSpace.Offset, NumBytes );
}

void CommandContext::InitializeTexture( GpuResource& Dest, UINT NumSubresources, D3D12_SUBRESOURCE_DATA SubData[] )
{
    UINT64 uploadBufferSize = GetRequiredIntermediateSize(Dest.GetResource(), 0, NumSubresources);

    CommandContext& InitContext = CommandContext::Begin();

    // copy data to the intermediate upload heap and then schedule a copy from the upload heap to the default texture
    DynAlloc mem = InitContext.ReserveUploadMemory(uploadBufferSize);
    UpdateSubresources(InitContext.m_CommandList, Dest.GetResource(), mem.Buffer.GetResource(), 0, 0, NumSubresources, SubData);
    InitContext.TransitionResource(Dest, D3D12_RESOURCE_STATE_GENERIC_READ);

    // Execute the command list and wait for it to finish so we can release the upload buffer
    InitContext.Finish(true);
}

void CommandContext::CopySubresource(GpuResource& Dest, UINT DestSubIndex, GpuResource& Src, UINT SrcSubIndex)
{
    FlushResourceBarriers();

    D3D12_TEXTURE_COPY_LOCATION DestLocation =
    {
        Dest.GetResource(),
        D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
        DestSubIndex
    };

    D3D12_TEXTURE_COPY_LOCATION SrcLocation =
    {
        Src.GetResource(),
        D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
        SrcSubIndex
    };

    m_CommandList->CopyTextureRegion(&DestLocation, 0, 0, 0, &SrcLocation, nullptr);
}

void CommandContext::InitializeTextureArraySlice(GpuResource& Dest, UINT SliceIndex, GpuResource& Src)
{
    CommandContext& Context = CommandContext::Begin();

    Context.TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST);
    Context.FlushResourceBarriers();

    const D3D12_RESOURCE_DESC& DestDesc = Dest.GetResource()->GetDesc();
    const D3D12_RESOURCE_DESC& SrcDesc = Src.GetResource()->GetDesc();

    ASSERT(SliceIndex < DestDesc.DepthOrArraySize &&
        SrcDesc.DepthOrArraySize == 1 &&
        DestDesc.Width == SrcDesc.Width &&
        DestDesc.Height == SrcDesc.Height &&
        DestDesc.MipLevels <= SrcDesc.MipLevels
        );

    UINT SubResourceIndex = SliceIndex * DestDesc.MipLevels;

    for (UINT i = 0; i < DestDesc.MipLevels; ++i)
    {
        D3D12_TEXTURE_COPY_LOCATION destCopyLocation =
        {
            Dest.GetResource(),
            D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
            SubResourceIndex + i
        };

        D3D12_TEXTURE_COPY_LOCATION srcCopyLocation =
        {
            Src.GetResource(),
            D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
            i
        };

        Context.m_CommandList->CopyTextureRegion(&destCopyLocation, 0, 0, 0, &srcCopyLocation, nullptr);
    }

    Context.TransitionResource(Dest, D3D12_RESOURCE_STATE_GENERIC_READ);
    Context.Finish(true);
}

void CommandContext::ReadbackTexture2D(GpuResource& ReadbackBuffer, PixelBuffer& SrcBuffer)
{
    // The footprint may depend on the device of the resource, but we assume there is only one device.
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedFootprint;
    g_Device->GetCopyableFootprints(&SrcBuffer.GetResource()->GetDesc(), 0, 1, 0, &PlacedFootprint, nullptr, nullptr, nullptr);

    // This very short command list only issues one API call and will be synchronized so we can immediately read
    // the buffer contents.
    CommandContext& Context = CommandContext::Begin(L"Copy texture to memory");

    Context.TransitionResource(SrcBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, true);

    Context.m_CommandList->CopyTextureRegion(
        &CD3DX12_TEXTURE_COPY_LOCATION(ReadbackBuffer.GetResource(), PlacedFootprint), 0, 0, 0,
        &CD3DX12_TEXTURE_COPY_LOCATION(SrcBuffer.GetResource(), 0), nullptr);

    Context.Finish(true);
}

void CommandContext::InitializeBuffer( GpuResource& Dest, const void* BufferData, size_t NumBytes, size_t Offset)
{
    CommandContext& InitContext = CommandContext::Begin();

    DynAlloc mem = InitContext.ReserveUploadMemory(NumBytes);
    SIMDMemCopy(mem.DataPtr, BufferData, Math::DivideByMultiple(NumBytes, 16));

    // copy data to the intermediate upload heap and then schedule a copy from the upload heap to the default texture
    InitContext.TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST, true);
    InitContext.m_CommandList->CopyBufferRegion(Dest.GetResource(), Offset, mem.Buffer.GetResource(), 0, NumBytes);
    InitContext.TransitionResource(Dest, D3D12_RESOURCE_STATE_GENERIC_READ, true);

    // Execute the command list and wait for it to finish so we can release the upload buffer
    InitContext.Finish(true);
}
