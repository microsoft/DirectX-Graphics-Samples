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
// Author(s):  Julia Careaga
//             James Stanard
//

#include "pch.h"
#include "BufferManager.h"
#include "Camera.h"
#include "ColorBuffer.h"
#include "CommandContext.h"
#include "GameCore.h"
#include "GraphicsCore.h"
#include "Math/Random.h"
#include "ParticleEffectManager.h"
#include "ParticleEffect.h"
#include "ParticleEffectProperties.h"
#include "TextureManager.h"
#include <mutex>

#include "CompiledShaders/ParticleSpawnCS.h"
#include "CompiledShaders/ParticleUpdateCS.h"
#include "CompiledShaders/ParticleDispatchIndirectArgsCS.h"
#include "CompiledShaders/ParticleFinalDispatchIndirectArgsCS.h"
#include "CompiledShaders/ParticleLargeBinCullingCS.h"
#include "CompiledShaders/ParticleBinCullingCS.h"

#include "CompiledShaders/ParticleTileRenderCS.h"
#include "CompiledShaders/ParticleTileRenderFastCS.h"
#include "CompiledShaders/ParticleTileRenderSlowDynamicCS.h"
#include "CompiledShaders/ParticleTileRenderFastDynamicCS.h"
#include "CompiledShaders/ParticleTileRenderSlowLowResCS.h"
#include "CompiledShaders/ParticleTileRenderFastLowResCS.h"

#include "CompiledShaders/ParticleTileRender2CS.h"
#include "CompiledShaders/ParticleTileRenderFast2CS.h"
#include "CompiledShaders/ParticleTileRenderSlowDynamic2CS.h"
#include "CompiledShaders/ParticleTileRenderFastDynamic2CS.h"
#include "CompiledShaders/ParticleTileRenderSlowLowRes2CS.h"
#include "CompiledShaders/ParticleTileRenderFastLowRes2CS.h"

#include "CompiledShaders/ParticleTileCullingCS.h"
#include "CompiledShaders/ParticleDepthBoundsCS.h"

#include "CompiledShaders/ParticleSortIndirectArgsCS.h"
#include "CompiledShaders/ParticlePreSortCS.h"
#include "CompiledShaders/ParticleInnerSortCS.h"
#include "CompiledShaders/ParticleOuterSortCS.h"
#include "CompiledShaders/ParticlePS.h"
#include "CompiledShaders/ParticleVS.h"

#define EFFECTS_ERROR uint32_t(0xFFFFFFFF)

#define MAX_TOTAL_PARTICLES 0x40000		// 256k (18-bit indices)
#define MAX_PARTICLES_PER_BIN 1024
#define BIN_SIZE_X 128
#define BIN_SIZE_Y 64
#define TILE_SIZE 16

// It's good to have 32 tiles per bin to maximize the tile culling phase
#define TILES_PER_BIN_X (BIN_SIZE_X / TILE_SIZE)
#define TILES_PER_BIN_Y (BIN_SIZE_Y / TILE_SIZE)
#define TILES_PER_BIN (TILES_PER_BIN_X * TILES_PER_BIN_Y)

using namespace Graphics;
using namespace Math;
using namespace ParticleEffects;

namespace ParticleEffects
{
    BoolVar Enable("Graphics/Particle Effects/Enable", true);
    BoolVar EnableSpriteSort("Graphics/Particle Effects/Sort Sprites", true);
    BoolVar EnableTiledRendering("Graphics/Particle Effects/Tiled Rendering", true);
    BoolVar PauseSim("Graphics/Particle Effects/Pause Simulation", false);
    const char* ResolutionLabels[] = { "High-Res", "Low-Res", "Dynamic" };
    EnumVar TiledRes("Graphics/Particle Effects/Tiled Sample Rate", 2, 3, ResolutionLabels);
    NumVar DynamicResLevel("Graphics/Particle Effects/Dynamic Resolution Cutoff", 0.0f, -4.0f, 4.0f, 0.5f);
    NumVar MipBias("Graphics/Particle Effects/Mip Bias", 0.0f, -4.0f, 4.0f, 0.5f);
    
    ComputePSO s_ParticleSpawnCS;
    ComputePSO s_ParticleUpdateCS;
    ComputePSO s_ParticleDispatchIndirectArgsCS;

    StructuredBuffer SpriteVertexBuffer;
    
    UINT s_ReproFrame = 0;//201;
    RandomNumberGenerator s_RNG;
}

struct CBChangesPerView
{
    Matrix4 gInvView;
    Matrix4 gViewProj;

    float gVertCotangent;
    float gAspectRatio;
    float gRcpFarZ;
    float gInvertZ;

    float gBufferWidth;
    float gBufferHeight;
    float gRcpBufferWidth;
    float gRcpBufferHeight;

    uint32_t gBinsPerRow;
    uint32_t gTileRowPitch;
    uint32_t gTilesPerRow;
    uint32_t gTilesPerCol;
};

namespace
{
    ComputePSO s_ParticleFinalDispatchIndirectArgsCS;
    ComputePSO s_ParticleLargeBinCullingCS; 
    ComputePSO s_ParticleBinCullingCS; 
    ComputePSO s_ParticleTileCullingCS; 
    ComputePSO s_ParticleTileRenderSlowCS[3];	// High-Res, Low-Res, Dynamic-Res
    ComputePSO s_ParticleTileRenderFastCS[3]; 	// High-Res, Low-Res, Dynamic-Res (disable depth tests)
    ComputePSO s_ParticleDepthBoundsCS;
    GraphicsPSO s_NoTileRasterizationPSO;
    ComputePSO s_ParticleSortIndirectArgsCS;
    ComputePSO s_ParticlePreSortCS;
    ComputePSO s_ParticleInnerSortCS;
    ComputePSO s_ParticleOuterSortCS;

    RootSignature RootSig;

    StructuredBuffer SpriteIndexBuffer;
    IndirectArgsBuffer SortIndirectArgs;

    IndirectArgsBuffer DrawIndirectArgs;
    IndirectArgsBuffer FinalDispatchIndirectArgs;
    StructuredBuffer VisibleParticleBuffer;
    StructuredBuffer BinParticles[2];
    StructuredBuffer BinCounters[2];
    StructuredBuffer TileCounters;
    ByteAddressBuffer TileHitMasks;

    StructuredBuffer TileDrawPackets;
    StructuredBuffer TileFastDrawPackets;
    IndirectArgsBuffer TileDrawDispatchIndirectArgs;

    CBChangesPerView s_ChangesPerView;

    GpuResource TextureArray;
    D3D12_CPU_DESCRIPTOR_HANDLE TextureArraySRV;
    std::vector<std::wstring> TextureNameArray;

    std::vector<std::unique_ptr<ParticleEffect>> ParticleEffectsPool;
    std::vector<ParticleEffect*> ParticleEffectsActive;

    static bool s_InitComplete = false; 
    UINT TotalElapsedFrames;

    void SetFinalBuffers(ComputeContext& CompContext)
    {
        CompContext.SetPipelineState(s_ParticleFinalDispatchIndirectArgsCS);

        CompContext.TransitionResource(SpriteVertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
        CompContext.TransitionResource(FinalDispatchIndirectArgs, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        CompContext.TransitionResource(DrawIndirectArgs, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        D3D12_CPU_DESCRIPTOR_HANDLE pDispatchIndirectUAV[2] = {FinalDispatchIndirectArgs.GetUAV(), DrawIndirectArgs.GetUAV()};
        CompContext.SetDynamicDescriptors(3, 0, _countof(pDispatchIndirectUAV), pDispatchIndirectUAV);
        CompContext.SetDynamicDescriptor(4, 0, SpriteVertexBuffer.GetCounterSRV(CompContext));

        CompContext.Dispatch( 1, 1, 1 );
    }

    void MaintainTextureList(ParticleEffectProperties& effectProperties)
    {
        std::wstring name = effectProperties.TexturePath;

        for (uint32_t i = 0; i < TextureNameArray.size(); i++)
        {
            if (name.compare(TextureNameArray[i]) == 0)
            {
                effectProperties.EmitProperties.TextureID = i;
                return;
            }
        }
    
        TextureNameArray.push_back(name);
        UINT TextureID = (UINT)(TextureNameArray.size() - 1);
        effectProperties.EmitProperties.TextureID = TextureID;

        const ManagedTexture* managedTex = TextureManager::LoadDDSFromFile(name.c_str(), true);
        managedTex->WaitForLoad();

        GpuResource& ParticleTexture = *const_cast<ManagedTexture*>(managedTex);
        CommandContext::InitializeTextureArraySlice(TextureArray, TextureID, ParticleTexture);
    }


    void RenderTiles(ComputeContext& CompContext, ColorBuffer& ColorTarget, ColorBuffer& LinearDepth)
    {	
        size_t ScreenWidth = ColorTarget.GetWidth();
        size_t ScreenHeight = ColorTarget.GetHeight();

        ASSERT(ColorTarget.GetFormat() == DXGI_FORMAT_R32_UINT || g_bTypedUAVLoadSupport_R11G11B10_FLOAT,
            "Without typed UAV loads, tiled particles must render to a R32_UINT buffer");

        {
            ScopedTimer _p(L"Compute Depth Bounds", CompContext);

            CompContext.SetPipelineState(s_ParticleDepthBoundsCS);

            CompContext.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            CompContext.TransitionResource(g_MinMaxDepth8, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            CompContext.TransitionResource(g_MinMaxDepth16, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            CompContext.TransitionResource(g_MinMaxDepth32, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            CompContext.SetDynamicDescriptor(3, 0, g_MinMaxDepth8.GetUAV());
            CompContext.SetDynamicDescriptor(3, 1, g_MinMaxDepth16.GetUAV());
            CompContext.SetDynamicDescriptor(3, 2, g_MinMaxDepth32.GetUAV());
            CompContext.SetDynamicDescriptor(4, 0, LinearDepth.GetSRV());

            CompContext.Dispatch2D(ScreenWidth, ScreenHeight, 32, 32);
        }

        {
            ScopedTimer _p(L"Culling & Sorting", CompContext);

            CompContext.ResetCounter(VisibleParticleBuffer);

            // The first step inserts each particle into all of the large bins it intersects.  Large bins
            // are 512x256.
            CompContext.SetPipelineState(s_ParticleLargeBinCullingCS);
            CompContext.SetConstants(0, 5, 4);

            CompContext.TransitionResource(SpriteVertexBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            CompContext.TransitionResource(DrawIndirectArgs, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            CompContext.TransitionResource(FinalDispatchIndirectArgs, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
            CompContext.TransitionResource(BinParticles[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            CompContext.TransitionResource(BinCounters[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            CompContext.TransitionResource(VisibleParticleBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

            CompContext.SetDynamicDescriptor(3, 0, BinParticles[0].GetUAV());
            CompContext.SetDynamicDescriptor(3, 1, BinCounters[0].GetUAV());
            CompContext.SetDynamicDescriptor(3, 2, VisibleParticleBuffer.GetUAV());
            CompContext.SetDynamicDescriptor(4, 0, SpriteVertexBuffer.GetSRV());
            CompContext.SetDynamicDescriptor(4, 1, DrawIndirectArgs.GetSRV());

            CompContext.DispatchIndirect(FinalDispatchIndirectArgs);

            // The second step refines the binning by inserting particles into the appropriate small bins.
            // Small bins are 128x64.
            CompContext.SetPipelineState(s_ParticleBinCullingCS);
            CompContext.SetConstants(0, 3, 2);

            CompContext.TransitionResource(VisibleParticleBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            CompContext.TransitionResource(BinParticles[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            CompContext.TransitionResource(BinCounters[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            CompContext.TransitionResource(BinParticles[1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            CompContext.TransitionResource(BinCounters[1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

            CompContext.SetDynamicDescriptor(3, 0, BinParticles[1].GetUAV());
            CompContext.SetDynamicDescriptor(3, 1, BinCounters[1].GetUAV());
            CompContext.SetDynamicDescriptor(4, 0, VisibleParticleBuffer.GetSRV());
            CompContext.SetDynamicDescriptor(4, 1, BinParticles[0].GetSRV());
            CompContext.SetDynamicDescriptor(4, 2, BinCounters[0].GetSRV());

            CompContext.Dispatch2D(ScreenWidth, ScreenHeight, 4 * BIN_SIZE_X, 4 * BIN_SIZE_Y);

            // The final sorting step will perform a bitonic sort on each bin's particles (front to
            // back).  Afterward, it will generate a bitmap for each tile indicating which of the bin's
            // particles occupy the tile.  This allows each tile to iterate over a sorted list of particles
            // ignoring the ones that do not intersect.
            CompContext.SetPipelineState(s_ParticleTileCullingCS);

            CompContext.FillBuffer(TileDrawDispatchIndirectArgs, 0, 0, sizeof(uint32_t));
            CompContext.FillBuffer(TileDrawDispatchIndirectArgs, 12, 0, sizeof(uint32_t));

            CompContext.TransitionResource(BinParticles[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            CompContext.TransitionResource(TileHitMasks, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            CompContext.TransitionResource(TileDrawPackets, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            CompContext.TransitionResource(TileFastDrawPackets, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            CompContext.TransitionResource(TileDrawDispatchIndirectArgs, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            CompContext.TransitionResource(BinParticles[1], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            CompContext.TransitionResource(BinCounters[1], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            CompContext.TransitionResource(g_MinMaxDepth8, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            CompContext.TransitionResource(g_MinMaxDepth16, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            CompContext.TransitionResource(g_MinMaxDepth32, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

            CompContext.SetDynamicDescriptor(3, 0, BinParticles[0].GetUAV());
            CompContext.SetDynamicDescriptor(3, 1, TileHitMasks.GetUAV());
            CompContext.SetDynamicDescriptor(3, 2, TileDrawPackets.GetUAV());
            CompContext.SetDynamicDescriptor(3, 3, TileFastDrawPackets.GetUAV());
            CompContext.SetDynamicDescriptor(3, 4, TileDrawDispatchIndirectArgs.GetUAV());

            CompContext.SetDynamicDescriptor(4, 0, BinParticles[1].GetSRV());
            CompContext.SetDynamicDescriptor(4, 1, BinCounters[1].GetSRV());
            CompContext.SetDynamicDescriptor(4, 2, TILE_SIZE == 16 ? g_MinMaxDepth16.GetSRV() : g_MinMaxDepth32.GetSRV());
            CompContext.SetDynamicDescriptor(4, 3, VisibleParticleBuffer.GetSRV());

            CompContext.Dispatch2D(ScreenWidth, ScreenHeight, BIN_SIZE_X, BIN_SIZE_Y);
        }

        {
            ScopedTimer _p(L"Tiled Rendering", CompContext);

            CompContext.TransitionResource(TileDrawDispatchIndirectArgs, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
            CompContext.TransitionResource(ColorTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            CompContext.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            CompContext.TransitionResource(BinParticles[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            CompContext.TransitionResource(TileHitMasks, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            CompContext.TransitionResource(TileDrawPackets, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            CompContext.TransitionResource(TileFastDrawPackets, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            CompContext.TransitionResource(TextureArray, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

            CompContext.SetDynamicDescriptor(3, 0, ColorTarget.GetUAV());

            D3D12_CPU_DESCRIPTOR_HANDLE SRVs[] =
            {
                VisibleParticleBuffer.GetSRV(),
                TileHitMasks.GetSRV(),
                TextureArraySRV,
                LinearDepth.GetSRV(),
                BinParticles[0].GetSRV(),
                TileDrawPackets.GetSRV(),
                TileFastDrawPackets.GetSRV(),
                (TILE_SIZE == 16 ? g_MinMaxDepth16.GetSRV() : g_MinMaxDepth32.GetSRV()),
            };
            CompContext.SetDynamicDescriptors(4, 0, _countof(SRVs), SRVs);

            CompContext.SetConstants(0, (float)DynamicResLevel, (float)MipBias);

            CompContext.SetPipelineState(s_ParticleTileRenderSlowCS[TiledRes]);
            CompContext.DispatchIndirect(TileDrawDispatchIndirectArgs, 0);

            CompContext.SetPipelineState(s_ParticleTileRenderFastCS[TiledRes]);
            CompContext.DispatchIndirect(TileDrawDispatchIndirectArgs, 12);
        }
    }

    void BitonicSort( ComputeContext& CompContext )
    {
        uint32_t IndirectArgsOffset = 12;

        // We have already pre-sorted up through k = 2048 when first writing our list, so
        // we continue sorting with k = 4096.  For unnecessarily large values of k, these
        // indirect dispatches will be skipped over with thread counts of 0.

        for (uint32_t k = 4096; k <= 256*1024; k *= 2)
        {
            CompContext.SetPipelineState(s_ParticleOuterSortCS);

            for (uint32_t j = k / 2; j >= 2048; j /= 2)
            {
                CompContext.SetConstants(0, k, j);
                CompContext.DispatchIndirect(SortIndirectArgs, IndirectArgsOffset);
                CompContext.InsertUAVBarrier(SpriteIndexBuffer);
            }

            CompContext.SetPipelineState(s_ParticleInnerSortCS);
            CompContext.SetConstants(0, k);
            CompContext.DispatchIndirect(SortIndirectArgs, IndirectArgsOffset);
            CompContext.InsertUAVBarrier(SpriteIndexBuffer);

            IndirectArgsOffset += 12;
        }
    }

    void RenderSprites(GraphicsContext& GrContext, ColorBuffer& ColorTarget, DepthBuffer& DepthTarget, ColorBuffer& LinearDepth)
    {
        if (EnableSpriteSort)
        {
            ScopedTimer _p(L"Sort Particles", GrContext);
            ComputeContext& CompContext = GrContext.GetComputeContext();
            CompContext.SetRootSignature(RootSig);

            CompContext.SetDynamicConstantBufferView(1, sizeof(CBChangesPerView), &s_ChangesPerView);

            CompContext.SetPipelineState(s_ParticleSortIndirectArgsCS);
            CompContext.TransitionResource(SpriteVertexBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            CompContext.TransitionResource(SortIndirectArgs, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            CompContext.TransitionResource(DrawIndirectArgs, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            CompContext.SetDynamicDescriptor(3, 0, SortIndirectArgs.GetUAV());
            CompContext.SetDynamicDescriptor(4, 1, DrawIndirectArgs.GetSRV());
            CompContext.Dispatch(1, 1, 1);

            CompContext.SetPipelineState(s_ParticlePreSortCS);
            CompContext.TransitionResource(SortIndirectArgs, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
            CompContext.TransitionResource(SpriteIndexBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            CompContext.SetDynamicDescriptor(3, 0, SpriteIndexBuffer.GetUAV());
            CompContext.SetDynamicDescriptor(4, 0, SpriteVertexBuffer.GetSRV());
            CompContext.DispatchIndirect(SortIndirectArgs, 0);

            CompContext.InsertUAVBarrier(SpriteIndexBuffer);

            BitonicSort(CompContext);
        }

        D3D12_RECT scissor;
        scissor.left = 0;
        scissor.top = 0;
        scissor.right = (LONG)ColorTarget.GetWidth();
        scissor.bottom = (LONG)ColorTarget.GetHeight();

        D3D12_VIEWPORT viewport;
        viewport.TopLeftX = 0.0;
        viewport.TopLeftY = 0.0;
        viewport.Width = (float)ColorTarget.GetWidth();
        viewport.Height = (float)ColorTarget.GetHeight();
        viewport.MinDepth = 0.0;
        viewport.MaxDepth = 1.0;

        GrContext.SetPipelineState(s_NoTileRasterizationPSO);
        GrContext.TransitionResource(SpriteVertexBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        GrContext.TransitionResource(DrawIndirectArgs, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
        GrContext.TransitionResource(TextureArray, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        GrContext.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        GrContext.TransitionResource(SpriteIndexBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        GrContext.SetDynamicDescriptor(4, 0, SpriteVertexBuffer.GetSRV() );
        GrContext.SetDynamicDescriptor(4, 1, TextureArraySRV);
        GrContext.SetDynamicDescriptor(4, 2, LinearDepth.GetSRV());
        GrContext.SetDynamicDescriptor(4, 3, SpriteIndexBuffer.GetSRV());
        GrContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        GrContext.TransitionResource(ColorTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
        GrContext.TransitionResource(DepthTarget, D3D12_RESOURCE_STATE_DEPTH_READ);
        GrContext.SetRenderTarget(ColorTarget.GetRTV(), DepthTarget.GetDSV_DepthReadOnly());
        GrContext.SetViewportAndScissor(viewport, scissor);
        GrContext.DrawIndirect(DrawIndirectArgs);
    }

} // {anonymous} namespace

//---------------------------------------------------------------------
//
//	Initialize
//
//---------------------------------------------------------------------

void ParticleEffects::Initialize( uint32_t MaxDisplayWidth, uint32_t MaxDisplayHeight )
{	
    D3D12_SAMPLER_DESC SamplerBilinearBorderDesc = SamplerPointBorderDesc;
    SamplerBilinearBorderDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;

    RootSig.Reset(5, 3);
    RootSig.InitStaticSampler(0, SamplerBilinearBorderDesc);
    RootSig.InitStaticSampler(1, SamplerPointBorderDesc);
    RootSig.InitStaticSampler(2, SamplerPointClampDesc);
    RootSig[0].InitAsConstants(0, 3);
    RootSig[1].InitAsConstantBuffer(1);
    RootSig[2].InitAsConstantBuffer(2);
    RootSig[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 8);
    RootSig[4].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10);
    RootSig.Finalize(L"Particle Effects");

#define CreatePSO( ObjName, ShaderByteCode ) \
    ObjName.SetRootSignature(RootSig); \
    ObjName.SetComputeShader(ShaderByteCode, sizeof(ShaderByteCode) ); \
    ObjName.Finalize();
    CreatePSO(s_ParticleSpawnCS, g_pParticleSpawnCS);
    CreatePSO(s_ParticleUpdateCS, g_pParticleUpdateCS);
    CreatePSO(s_ParticleDispatchIndirectArgsCS, g_pParticleDispatchIndirectArgsCS);
    CreatePSO(s_ParticleFinalDispatchIndirectArgsCS, g_pParticleFinalDispatchIndirectArgsCS);

    CreatePSO(s_ParticleLargeBinCullingCS, g_pParticleLargeBinCullingCS);
    CreatePSO(s_ParticleBinCullingCS, g_pParticleBinCullingCS);
    CreatePSO(s_ParticleTileCullingCS, g_pParticleTileCullingCS);
    if (g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
    {
        CreatePSO(s_ParticleTileRenderSlowCS[0], g_pParticleTileRender2CS);
        CreatePSO(s_ParticleTileRenderFastCS[0], g_pParticleTileRenderFast2CS);
        CreatePSO(s_ParticleTileRenderSlowCS[1], g_pParticleTileRenderSlowLowRes2CS);
        CreatePSO(s_ParticleTileRenderFastCS[1], g_pParticleTileRenderFastLowRes2CS);
        CreatePSO(s_ParticleTileRenderSlowCS[2], g_pParticleTileRenderSlowDynamic2CS);
        CreatePSO(s_ParticleTileRenderFastCS[2], g_pParticleTileRenderFastDynamic2CS);
    }
    else
    {
        CreatePSO(s_ParticleTileRenderSlowCS[0], g_pParticleTileRenderCS);
        CreatePSO(s_ParticleTileRenderFastCS[0], g_pParticleTileRenderFastCS);
        CreatePSO(s_ParticleTileRenderSlowCS[1], g_pParticleTileRenderSlowLowResCS);
        CreatePSO(s_ParticleTileRenderFastCS[1], g_pParticleTileRenderFastLowResCS);
        CreatePSO(s_ParticleTileRenderSlowCS[2], g_pParticleTileRenderSlowDynamicCS);
        CreatePSO(s_ParticleTileRenderFastCS[2], g_pParticleTileRenderFastDynamicCS);
    }
    CreatePSO(s_ParticleDepthBoundsCS, g_pParticleDepthBoundsCS);
    CreatePSO(s_ParticleSortIndirectArgsCS, g_pParticleSortIndirectArgsCS);
    CreatePSO(s_ParticlePreSortCS, g_pParticlePreSortCS);
    CreatePSO(s_ParticleInnerSortCS, g_pParticleInnerSortCS);
    CreatePSO(s_ParticleOuterSortCS, g_pParticleOuterSortCS);

#undef CreatePSO

    //VSPS Render, no tiles.
    s_NoTileRasterizationPSO.SetRootSignature(RootSig);
    s_NoTileRasterizationPSO.SetRasterizerState(RasterizerTwoSided);
    s_NoTileRasterizationPSO.SetDepthStencilState(DepthStateReadOnly);
    s_NoTileRasterizationPSO.SetBlendState(BlendPreMultiplied);
    s_NoTileRasterizationPSO.SetInputLayout(0, nullptr);
    s_NoTileRasterizationPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE); 
    s_NoTileRasterizationPSO.SetRenderTargetFormat(g_SceneColorBuffer.GetFormat(), g_SceneDepthBuffer.GetFormat());
    s_NoTileRasterizationPSO.SetVertexShader(g_pParticleVS, sizeof(g_pParticleVS));
    s_NoTileRasterizationPSO.SetPixelShader(g_pParticlePS, sizeof(g_pParticlePS));
    s_NoTileRasterizationPSO.Finalize();

    __declspec(align(16)) UINT InitialDrawIndirectArgs[4] = { 4, 0, 0, 0 };
    DrawIndirectArgs.Create(L"ParticleEffects::DrawIndirectArgs", 1, sizeof(D3D12_DRAW_ARGUMENTS), InitialDrawIndirectArgs);
    __declspec(align(16)) UINT InitialDispatchIndirectArgs[6] = { 0, 1, 1, 0, 1, 1 };
    FinalDispatchIndirectArgs.Create(L"ParticleEffects::FinalDispatchIndirectArgs", 1, sizeof(D3D12_DISPATCH_ARGUMENTS), InitialDispatchIndirectArgs);
    SpriteVertexBuffer.Create(L"ParticleEffects::SpriteVertexBuffer", MAX_TOTAL_PARTICLES, sizeof(ParticleVertex));
    VisibleParticleBuffer.Create(L"ParticleEffects::VisibleParticleBuffer", MAX_TOTAL_PARTICLES, sizeof(ParticleScreenData));
    SpriteIndexBuffer.Create(L"ParticleEffects::SpriteIndexBuffer", MAX_TOTAL_PARTICLES, sizeof(UINT));	
    SortIndirectArgs.Create(L"ParticleEffects::SortIndirectArgs", 8, sizeof(D3D12_DISPATCH_ARGUMENTS));
    TileDrawDispatchIndirectArgs.Create(L"ParticleEffects::DrawPackets_IArgs", 2, sizeof(D3D12_DISPATCH_ARGUMENTS), InitialDispatchIndirectArgs);

    const uint32_t LargeBinsPerRow = DivideByMultiple(MaxDisplayWidth, 4 * BIN_SIZE_X);
    const uint32_t LargeBinsPerCol = DivideByMultiple(MaxDisplayHeight, 4 * BIN_SIZE_Y);
    const uint32_t BinsPerRow = LargeBinsPerRow * 4;
    const uint32_t BinsPerCol = LargeBinsPerCol * 4;
    const uint32_t MaxParticlesPerLargeBin = MAX_PARTICLES_PER_BIN * 16;
    const uint32_t ParticleBinCapacity = LargeBinsPerRow * LargeBinsPerCol * MaxParticlesPerLargeBin;
    const uint32_t TilesPerRow = DivideByMultiple(MaxDisplayWidth, TILE_SIZE);
    const uint32_t TilesPerCol = DivideByMultiple(MaxDisplayHeight, TILE_SIZE);

    // Padding is necessary to eliminate bounds checking when outputting data to bins or tiles.
    const uint32_t PaddedTilesPerRow = AlignUp(TilesPerRow, TILES_PER_BIN_X * 4);
    const uint32_t PaddedTilesPerCol = AlignUp(TilesPerCol, TILES_PER_BIN_Y * 4);

    BinParticles[0].Create(L"ParticleEffects::BinParticles[0]", ParticleBinCapacity, sizeof(UINT));
    BinParticles[1].Create(L"ParticleEffects::BinParticles[1]", ParticleBinCapacity, sizeof(UINT));
    BinCounters[0].Create(L"ParticleEffects::LargeBinCounters", LargeBinsPerRow * LargeBinsPerCol, sizeof(UINT));
    BinCounters[1].Create(L"ParticleEffects::BinCounters", BinsPerRow * BinsPerCol, sizeof(UINT));
    TileCounters.Create(L"ParticleEffects::TileCounters", PaddedTilesPerRow * PaddedTilesPerCol, sizeof(UINT));
    TileHitMasks.Create(L"ParticleEffects::TileHitMasks", PaddedTilesPerRow * PaddedTilesPerCol, MAX_PARTICLES_PER_BIN / 8);
    TileDrawPackets.Create(L"ParticleEffects::DrawPackets", TilesPerRow * TilesPerCol, sizeof(UINT));
    TileFastDrawPackets.Create(L"ParticleEffects::FastDrawPackets", TilesPerRow * TilesPerCol, sizeof(UINT));

    D3D12_RESOURCE_DESC TexDesc = {};
    TexDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    TexDesc.Format = DXGI_FORMAT_BC3_UNORM_SRGB;
    TexDesc.Width = 64;
    TexDesc.Height = 64;
    TexDesc.DepthOrArraySize = 16;
    TexDesc.MipLevels = 4;
    TexDesc.SampleDesc.Count = 1;
    TexDesc.Alignment = 0x10000;

    D3D12_HEAP_PROPERTIES HeapProps = {};
    HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    HeapProps.CreationNodeMask = 1;
    HeapProps.VisibleNodeMask = 1;

    ID3D12Resource* tex = nullptr;
    ASSERT_SUCCEEDED( g_Device->CreateCommittedResource( &HeapProps, D3D12_HEAP_FLAG_NONE,
        &TexDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, MY_IID_PPV_ARGS(&tex)) );
    tex->SetName(L"Particle TexArray");
    TextureArray = GpuResource(tex, D3D12_RESOURCE_STATE_COPY_DEST);
    tex->Release();

    D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    SRVDesc.Format = DXGI_FORMAT_BC3_UNORM_SRGB;
    SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
    SRVDesc.Texture2DArray.MipLevels = 4;
    SRVDesc.Texture2DArray.ArraySize = 16;
    
    TextureArraySRV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    g_Device->CreateShaderResourceView(TextureArray.GetResource(), &SRVDesc, TextureArraySRV);

    if (s_ReproFrame > 0)
        s_RNG.SetSeed(1);
    
    TotalElapsedFrames = 0;
    s_InitComplete = true;
}

void ParticleEffects::Shutdown( void )
{
    ClearAll();

    SpriteVertexBuffer.Destroy();
    DrawIndirectArgs.Destroy();
    FinalDispatchIndirectArgs.Destroy();
    SpriteVertexBuffer.Destroy();
    VisibleParticleBuffer.Destroy();
    SpriteIndexBuffer.Destroy();
    SortIndirectArgs.Destroy();
    TileDrawDispatchIndirectArgs.Destroy();

    BinParticles[0].Destroy();
    BinParticles[1].Destroy();
    BinCounters[0].Destroy();
    BinCounters[1].Destroy();
    TileCounters.Destroy();
    TileHitMasks.Destroy();
    TileDrawPackets.Destroy();
    TileFastDrawPackets.Destroy();
    TextureArray.Destroy();
}

//Returns index into Pool
EffectHandle ParticleEffects::PreLoadEffectResources( ParticleEffectProperties& effectProperties )
{
    if (!s_InitComplete)
        return EFFECTS_ERROR;

    static std::mutex s_TextureMutex;
    s_TextureMutex.lock();
    MaintainTextureList(effectProperties);
    ParticleEffectsPool.emplace_back(new ParticleEffect(effectProperties));
    s_TextureMutex.unlock();

    EffectHandle index = (EffectHandle)ParticleEffectsPool.size() - 1;
    ParticleEffectsPool[index]->LoadDeviceResources(Graphics::g_Device);
    return index;
}

//Returns index into Active
EffectHandle ParticleEffects::InstantiateEffect( EffectHandle effectHandle )
{
    if (!s_InitComplete || effectHandle >= ParticleEffectsPool.size() || effectHandle < 0)
        return EFFECTS_ERROR;
    
    ParticleEffect* effect = ParticleEffectsPool[effectHandle].get();
    if (effect != NULL)
    {
        static std::mutex s_InstantiateEffectFromPoolMutex;
        s_InstantiateEffectFromPoolMutex.lock();
        ParticleEffectsActive.push_back(effect);
        s_InstantiateEffectFromPoolMutex.unlock();
    }
    else
        return EFFECTS_ERROR;

    EffectHandle index = (EffectHandle)ParticleEffectsActive.size() - 1;
    return index;
    
}

//Returns index into Active
EffectHandle ParticleEffects::InstantiateEffect( ParticleEffectProperties& effectProperties )
{
    if (!s_InitComplete)
        return EFFECTS_ERROR;

    static std::mutex s_InstantiateNewEffectMutex;
    s_InstantiateNewEffectMutex.lock();
    MaintainTextureList(effectProperties);
    ParticleEffect* newEffect = new ParticleEffect(effectProperties);
    ParticleEffectsPool.emplace_back(newEffect);
    ParticleEffectsActive.push_back(newEffect);
    s_InstantiateNewEffectMutex.unlock();

    EffectHandle index = (EffectHandle)ParticleEffectsActive.size() - 1;
    ParticleEffectsActive[index]->LoadDeviceResources(Graphics::g_Device);
    return index;	
}

//---------------------------------------------------------------------
//
//	Update
//
//---------------------------------------------------------------------

void ParticleEffects::Update(ComputeContext& Context, float timeDelta )
{
    if (!Enable || !s_InitComplete || ParticleEffectsActive.size() == 0)
        return;

    ScopedTimer _prof(L"Particle Update", Context);

    if (++TotalElapsedFrames == s_ReproFrame)
        PauseSim = true;

    if (PauseSim)
        return;

    Context.ResetCounter(SpriteVertexBuffer);

    if (ParticleEffectsActive.size() == 0)
        return;

    Context.SetRootSignature(RootSig);
    Context.SetConstants(0, timeDelta);
    Context.TransitionResource(SpriteVertexBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.SetDynamicDescriptor(3, 0, SpriteVertexBuffer.GetUAV());

    for (UINT i = 0; i < ParticleEffectsActive.size(); ++i)
    {	
        ParticleEffectsActive[i]->Update(Context, timeDelta);

        if (ParticleEffectsActive[i]->GetLifetime() <= ParticleEffectsActive[i]->GetElapsedTime())
        {
            //Erase from vector
            auto iter = ParticleEffectsActive.begin() + i;
            static std::mutex s_EraseEffectMutex;
            s_EraseEffectMutex.lock();
            ParticleEffectsActive.erase(iter);
            s_EraseEffectMutex.unlock();
        }
    }

    SetFinalBuffers(Context);
}


//---------------------------------------------------------------------
//
//	Render
//
//---------------------------------------------------------------------

void ParticleEffects::Render( CommandContext& Context, const Camera& Camera, ColorBuffer& ColorTarget, DepthBuffer& DepthTarget, ColorBuffer& LinearDepth)
{
    if (!Enable || !s_InitComplete || ParticleEffectsActive.size() == 0)
        return;

    uint32_t Width = (uint32_t)ColorTarget.GetWidth();
    uint32_t Height = (uint32_t)ColorTarget.GetHeight();

    ASSERT(
        Width == DepthTarget.GetWidth() &&
        Height == DepthTarget.GetHeight() &&
        Width == LinearDepth.GetWidth() &&
        Height == LinearDepth.GetHeight(),
        "There is a mismatch in buffer dimensions for rendering particles"
    );

    ScopedTimer _prof(L"Particle Render", Context);

    uint32_t BinsPerRow = 4 * DivideByMultiple(Width, 4 * BIN_SIZE_X);

    s_ChangesPerView.gViewProj = Camera.GetViewProjMatrix();  
    s_ChangesPerView.gInvView = Invert(Camera.GetViewMatrix());
    float HCot = Camera.GetProjMatrix().GetX().GetX();
    float VCot = Camera.GetProjMatrix().GetY().GetY();
    s_ChangesPerView.gVertCotangent = VCot;
    s_ChangesPerView.gAspectRatio = HCot / VCot;
    s_ChangesPerView.gRcpFarZ = 1.0f / Camera.GetFarClip();
    s_ChangesPerView.gInvertZ = Camera.GetNearClip() / (Camera.GetFarClip() - Camera.GetNearClip());
    s_ChangesPerView.gBufferWidth = (float)Width;
    s_ChangesPerView.gBufferHeight = (float)Height;
    s_ChangesPerView.gRcpBufferWidth = 1.0f / Width;
    s_ChangesPerView.gRcpBufferHeight = 1.0f / Height;
    s_ChangesPerView.gBinsPerRow = BinsPerRow;
    s_ChangesPerView.gTileRowPitch = BinsPerRow * TILES_PER_BIN_X;
    s_ChangesPerView.gTilesPerRow = DivideByMultiple(Width, TILE_SIZE);
    s_ChangesPerView.gTilesPerCol = DivideByMultiple(Height, TILE_SIZE);

    // For now, UAV load support for R11G11B10 is required to read-modify-write the color buffer, but
    // the compositing could be deferred.
    WARN_ONCE_IF(EnableTiledRendering && !g_bTypedUAVLoadSupport_R11G11B10_FLOAT,
        "Unable to composite tiled particles without support for R11G11B10F UAV loads");
    EnableTiledRendering = EnableTiledRendering && g_bTypedUAVLoadSupport_R11G11B10_FLOAT;

    if (EnableTiledRendering)
    {
        ComputeContext& CompContext = Context.GetComputeContext();
        CompContext.TransitionResource(ColorTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        CompContext.TransitionResource(BinCounters[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        CompContext.TransitionResource(BinCounters[1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

        CompContext.ClearUAV(BinCounters[0]);
        CompContext.ClearUAV(BinCounters[1]);
        CompContext.SetRootSignature(RootSig);
        CompContext.SetDynamicConstantBufferView(1, sizeof(CBChangesPerView), &s_ChangesPerView);

        RenderTiles(CompContext, ColorTarget, LinearDepth);

        CompContext.InsertUAVBarrier(ColorTarget);
    }
    else
    {
        GraphicsContext& GrContext = Context.GetGraphicsContext();
        GrContext.SetRootSignature(RootSig);
        GrContext.SetDynamicConstantBufferView(1, sizeof(CBChangesPerView), &s_ChangesPerView);	
        RenderSprites(GrContext, ColorTarget, DepthTarget, LinearDepth);
    }

}



//---------------------------------------------------------------------
//
//	Clean up
//
//---------------------------------------------------------------------

void ParticleEffects::ClearAll()
{
    ParticleEffectsActive.clear();
    ParticleEffectsPool.clear();
    TextureNameArray.clear();
}

void ParticleEffects::ResetEffect(EffectHandle EffectID)
{
    if (!s_InitComplete || ParticleEffectsActive.size() == 0 || PauseSim || EffectID >= ParticleEffectsActive.size())
        return;
    
    ParticleEffectsActive[EffectID]->Reset();
}


float ParticleEffects::GetCurrentLife(EffectHandle EffectID)
{
    if (!s_InitComplete || ParticleEffectsActive.size() == 0 || PauseSim || EffectID >= ParticleEffectsActive.size())
        return -1.0;
    
    return ParticleEffectsActive[EffectID]->GetElapsedTime();
}
