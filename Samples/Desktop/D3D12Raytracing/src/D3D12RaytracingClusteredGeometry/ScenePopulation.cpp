//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
//
//*********************************************************

// ============================================================================
// ScenePopulation.cpp
//
// Scene-specific procedural clone generation, factored out of the main
// D3D12RaytracingClusteredGeometry.cpp so that file can focus on CORE
// acceleration-structure management + rendering.  Nothing in here is
// strictly required for clustered raytracing -- it's the [N] workload-
// scaling toggle's spiral-of-clones generator, useful as a demo of how
// to scale up CLAS+CBLAS+template stress, but engineers cribbing the AS
// pipeline can ignore this file entirely.
//
// All methods here are still members of D3D12RaytracingClusteredGeometry
// (declared in the class header); just their definitions live separately.
//
// Contains:
//   * EnsureCloneSourceLodMeshes -- pre-generates LOD chains for the
//     cloneable source meshes (sphere0/torus/cube/klein bottle), so distance-
//     LOD swaps don't do per-frame mesh regeneration.
//   * RegenerateWorkloadCloneInstances -- the spiral generator itself.
//     Places N extra clone instances in a constant-density Vogel spiral
//     out from the floor, deep-copies a source ClusterObject per static
//     clone, records an AnimatedCloneInstance per anim clone, and applies
//     distance LOD by swapping in a lower-tessellation source mesh per tier.
// ============================================================================

#include "stdafx.h"
#include "D3D12RaytracingClusteredGeometry.h"
#include "SceneData.h"
#include "MaterialData.h"

#include <random>
#include <unordered_map>
#include <algorithm>

using namespace std;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

// ---- EnsureCloneSourceLodMeshes -------------------------------------------
// ---------------------------------------------------------------------------------
// Lazily generate the LOD-chain meshes used by tessellation-based
// distance LOD for cloneable sources.  Idempotent.  Each chain:
//
//   index 0 = full source resolution (matches scene-source params)
//   index N = each subsequent level halves both grid dimensions
//             while keeping tile sizes constant; cluster count drops
//             quartically with LOD level, tris-per-cluster stays at
//             the source's value (~256 for sphere0, ~128 for torus/
//             klein) -- per-cluster CLAS overhead stays proportional
//             so we don't waste cluster slots on a handful of tris.
//
// Source params are hardcoded to match SceneData::BuildSceneDefinition
// (sphere0_chrome / torus_amber_glass / klein_clear_glass).  If those
// scene params change, this needs to follow.  Future cleanup: read
// from SceneData by name instead of hardcoding.
// ---------------------------------------------------------------------------------
void D3D12RaytracingClusteredGeometry::EnsureCloneSourceLodMeshes()
{
    if (!m_cloneSourceLodMeshes.empty()) return;

    constexpr int kMaxLodLevels = 4;

    // SPHERE0 chrome.  Source: r=0.85, numLat=64, numLong=128, tile 8x16.
    // Source has 64 clusters * 256 tris = 16K tris.
    // LOD 0: 64x128/8x16 -> 64 clusters x 256 tris
    // LOD 1: 32x64 /8x16 -> 16 clusters x 256 tris  (still tile-divisible)
    // LOD 2: 16x32 /8x16 ->  4 clusters x 256 tris
    // LOD 3:  8x16 /8x16 ->  1 cluster  x 256 tris
    {
        std::vector<ProceduralGeometry::Mesh> chain;
        for (int lod = 0; lod < kMaxLodLevels; ++lod)
        {
            const int nLat  = 64  >> lod;
            const int nLong = 128 >> lod;
            const int tLat  = 8;
            const int tLong = 16;
            if (nLat < tLat || nLong < tLong) break;
            chain.push_back(ProceduralGeometry::GenerateUVSphereSpatialTiles(
                0.85f, nLat, nLong, tLat, tLong, 0));
        }
        m_cloneSourceLodMeshes[0] = std::move(chain);
    }

    // TORUS amber glass.  Source: major=0.55 minor=0.18, ring=64 side=32, tile 8x8.
    // LOD 0: 64x32/8x8 -> 32 clusters x 128 tris
    // LOD 1: 32x16/8x8 ->  8 clusters x 128 tris
    // LOD 2: 16x8 /8x8 ->  2 clusters x 128 tris
    // LOD 3:  8x8 /8x8 ->  1 cluster  x 128 tris
    {
        std::vector<ProceduralGeometry::Mesh> chain;
        for (int lod = 0; lod < kMaxLodLevels; ++lod)
        {
            const int nRing = 64 >> lod;
            const int nSide = 32 >> lod;
            const int tRing = 8;
            const int tSide = 8;
            if (nRing < tRing || nSide < tSide) break;
            chain.push_back(ProceduralGeometry::GenerateTorusSpatialTiles(
                0.55f, 0.18f, nRing, nSide, tRing, tSide, 0));
        }
        m_cloneSourceLodMeshes[4] = std::move(chain);
    }

    // KLEIN clear glass.  Source: scale=0.65, numU=64 numV=32, tile 8x8.
    // (Klein bottle generator follows the same numU x numV grid logic.)
    {
        std::vector<ProceduralGeometry::Mesh> chain;
        for (int lod = 0; lod < kMaxLodLevels; ++lod)
        {
            const int nU = 64 >> lod;
            const int nV = 32 >> lod;
            const int tU = 8;
            const int tV = 8;
            if (nU < tU || nV < tV) break;
            chain.push_back(ProceduralGeometry::GenerateKleinBottleSpatialTiles(
                0.65f, nU, nV, tU, tV, 0));
        }
        m_cloneSourceLodMeshes[8] = std::move(chain);
    }

    // CUBE copper glass.  Source: halfExtent=0.45, faceSubdiv=11, tileSize=11.
    // Each face is one cluster -> 6 clusters total at LOD 0.  Cube
    // generator's faceSubdiv must divide evenly by tileSize, so we
    // can't strictly halve in a clean chain.  Pick LOD pairs that
    // approximate halving each step's triangle count while keeping
    // a single cluster per face:
    //   LOD 0: 11/11 -> 6 clusters x 242 tris (= source)
    //   LOD 1:  6/6  -> 6 clusters x  72 tris
    //   LOD 2:  4/4  -> 6 clusters x  32 tris
    //   LOD 3:  2/2  -> 6 clusters x   8 tris
    {
        const int cubeFaceSubdivs[]   = { 11, 6, 4, 2 };
        std::vector<ProceduralGeometry::Mesh> chain;
        for (int lod = 0; lod < kMaxLodLevels && lod < (int)_countof(cubeFaceSubdivs); ++lod)
        {
            const int fs = cubeFaceSubdivs[lod];
            chain.push_back(ProceduralGeometry::GenerateCubeSpatialTiles(
                0.45f, fs, fs, 0));
        }
        m_cloneSourceLodMeshes[5] = std::move(chain);
    }

    SampleLog::LogF(L"[clone-lod] generated %zu chains: sphere0=%zu, torus=%zu, cube=%zu, klein=%zu\n",
                    m_cloneSourceLodMeshes.size(),
                    m_cloneSourceLodMeshes.count(0) ? m_cloneSourceLodMeshes[0].size() : 0,
                    m_cloneSourceLodMeshes.count(4) ? m_cloneSourceLodMeshes[4].size() : 0,
                    m_cloneSourceLodMeshes.count(5) ? m_cloneSourceLodMeshes[5].size() : 0,
                    m_cloneSourceLodMeshes.count(8) ? m_cloneSourceLodMeshes[8].size() : 0);
}


// ---- RegenerateWorkloadCloneInstances -------------------------------------------
// ---------------------------------------------------------------------------------
// [N] workload-scaling clone generator.  Called at the top of
// RebuildStaticAccelerationStructures BEFORE the per-object resource
// reset + build pipeline runs, so the build code naturally builds
// CLAS+BLAS for every clone (1:1 instance:BLAS, matching the user's
// "stress CLAS+CBLAS+templates progressively" intent).
//
// Each clone:
//   * Cycles src = i % m_sourceObjectCount through the source pool
//     (currently static-only -- animated source isn't in m_objects so
//     it's skipped here; anim-clone follow-up is a separate task).
//   * Deep-copies the source's mesh + encoded vertex blob (heavy but
//     keeps build code unchanged -- no obj.mesh redirection plumbing).
//   * Clears all GPU-resource ComPtrs so the upcoming reset loop is a
//     no-op for the clone (and the upcoming build allocates fresh).
//   * Gets a spiral world position via golden-angle sunflower packing
//     starting just outside the floor, rising slowly in y so distant
//     clones don't fully occlude near ones.
//   * Records a uniformly-random material slot in obj.instanceID; the
//     TLAS-build code reads this to populate the per-instance material
//     override buffer (sentinel 0xFFFFFFFFu = no override for sources).
//     Choosing 'instanceID' for this payload is a slight repurposing
//     -- for sources it's the natural per-object material slot; for
//     clones it doubles as the random override.  BuildTlasClassic
//     distinguishes them via the index range [m_sourceObjectCount..end).
//
// Memory cost dominates at 10K:
//   * Mesh deep-copies: ~250 KB/clone * 10K = ~2.5 GB CPU RAM
//   * CLAS arrays:      ~200 clusters/clone * 500 B = ~100 KB GPU/clone
//   * BLAS storage:     ~50 KB GPU/clone
//   Total ~1.5 GB GPU + 2.5 GB CPU at 10K on a 4090-class card.
// Build time at 10K is dominated by ~2M CLAS builds + 10K BLAS-from-CLAS;
// expect 1-3 second toggle-delay (one-shot, doesn't repeat per frame).
// ---------------------------------------------------------------------------------
void D3D12RaytracingClusteredGeometry::RegenerateWorkloadCloneInstances()
{
    if (m_sourceObjectCount == 0)
    {
        // First call before BuildScene captured the source count -- nothing
        // to do; this RebuildStatic call is the initial-build path and
        // m_objects already holds only sources.
        return;
    }
    // Truncate any previously-generated clones from BOTH lists -- BEFORE
    // any early-return path below so cycling to N=0 actually clears
    // the previous tier's clones (was leaking 20 anim clones into the
    // N=0 TLAS, growing it from 3.5 KB to ~10 KB and inflating the
    // overlay's TLAS readout from 3.5 KB to 0.58 MB after a 10K -> 0
    // cycle).
    if (m_objects.size() > m_sourceObjectCount)
    {
        m_objects.resize(m_sourceObjectCount);
    }
    m_animatedClones.clear();

    const UINT N_extra = ExtraInstancesCount();
    if (N_extra == 0)
    {
        SampleLog::LogF(L"[clones] N=0 (no extras)\n");
        return;
    }

    // Filter the source pool for cloning.  We DON'T clone the floor
    // (instanceID 6) -- it's a giant slab and cloning it would clutter
    // the spiral with overlapping floors that occlude everything else
    // and intersect the camera path.  Pool restricted to the four
    // "iconic" static shapes the user picked PLUS the animated source:
    //   * sphere0 chrome (instanceID 0)
    //   * animated sphere (m_animatedObject -- handled separately
    //                      since it's not stored in m_objects)
    // Cycle position 0..3 picks a static source by instanceID; cycle
    // position 4 spawns an animated clone (every 5th clone is animated
    // -> N=100 yields 20 anim clones, N=1K yields 200, N=10K yields 2000).
    //
    // CURRENT model for animated clones (phase 1): each anim clone is
    // a TLAS instance pointing at the SOURCE's blasGPUVA -- they share
    // the source's per-frame CLAS+BLAS result so per-frame work is
    // O(1) regardless of clone count.  Phase 2 will give each anim
    // clone its own per-frame CLAS+BLAS to actually stress those
    // paths progressively.
    //
    // Match by instanceID rather than vector index so this stays
    // robust to scene-data reordering.
    const UINT kCloneSourceInstanceIDs[] = { 0u, 4u, 5u, 8u };
    std::vector<UINT> srcIndices;
    srcIndices.reserve(_countof(kCloneSourceInstanceIDs));
    for (UINT keepID : kCloneSourceInstanceIDs)
    {
        for (UINT i = 0; i < m_sourceObjectCount; ++i)
        {
            if (m_objects[i].instanceID == keepID)
            {
                srcIndices.push_back(i);
                break;
            }
        }
    }
    if (srcIndices.empty())
    {
        SampleLog::LogF(L"[clones] WARN: no matching source instanceIDs found "
                        L"in m_objects; clone pool is empty -- N=%u ignored\n", N_extra);
        return;
    }
    // Source-pool size INCLUDES the animated source as the LAST cycle
    // slot.  If animated is disabled at runtime, drop it from the pool.
    const bool   animInPool     = m_animatedObjectEnabled;
    const UINT   srcStaticN     = (UINT)srcIndices.size();
    const UINT   srcPoolSize    = srcStaticN + (animInPool ? 1u : 0u);
    const UINT   kAnimCycleSlot = srcStaticN;

    // (m_animatedClones already cleared above, before the N_extra==0 early-return.)

    // Inner-radius placement of the spiral.
    //
    // FLOOR-INTERSECTION FIX: the floor is a square slab with
    // slabHalfSize=3.5, thickness 0.28, worldPos y=-0.7 -- footprint
    // covers x,z in [-3.5..+3.5] (corner radius sqrt(3.5²+3.5²) =
    // 4.95) with top y at -0.56 and bottom at -0.84.  Push kInnerRadius
    // PAST the floor's corner diagonal so spiral clones never sit
    // inside the floor footprint, AND lift the height curve's
    // kInnerY so the LOWEST-y clone (innermost) has its mesh BOTTOM
    // above the floor TOP at every random scale value.
    //
    // Min clone bottom = kInnerY - maxRadius*maxScale.  With max
    // scale 0.7 and unit-sphere base (radius 1.0) wave envelope 1.3,
    // worst-case mesh radius = 1.3 * 0.7 = 0.91, so clone bottom =
    // kInnerY - 0.91.  Want > floor top (-0.56) so kInnerY > 0.35.
    // kInnerY = 1.0 gives 0.65 margin above floor.
    const float kFloorHalf      = 3.5f;
    // Margin past the floor's corner (sqrt(3.5²+3.5²) = 4.95) so even
    // jittered positions stay clear.  5.5 leaves ~0.55 unit clearance.
    const float kInnerRadius    = 5.5f;
    // Radial spacing -- area per clone slot.  Tight (1.0) per earlier
    // user feedback "near objects should be more closely packed", but
    // see below for scale clamp that keeps adjacent clones non-overlapping
    // at this spacing.
    (void)kFloorHalf;
    // Radial spacing widens at higher tiers (where distance LOD is
    // active) so the lower-detail outer clones land at LARGER world
    // distances -- the smaller pixel footprint hides the LOD's
    // cluster-count drop the way a real LOD chain hides its
    // mesh-decimation artifacts behind perspective.
    //
    // CONSTANT spacing across tiers (per user feedback "the extra
    // 100 instances should stay the same when 1000 are drawn -- just
    // more in the distance") so the spiral's position for clone i
    // depends ONLY on i, not on the active N.  Toggling 100 -> 1K
    // -> 10K extends the spiral outward without reshuffling the
    // existing clones.
    //
    // Density tuned tighter (1.0 area-equivalent vs old 1.5) per user
    // feedback "near objects should be more closely packed" -- inner
    // clones land at smaller (radius - innerR) so the per-clone
    // annulus area drops from ~7 sq units to ~3 sq units, doubling
    // density everywhere.
    const float kRadialSpacing  = 1.0f;
    // Per-clone position jitter -- bumps clones off the exact Vogel arms
    // so the spiral doesn't read as visible curves of empty space.
    // CONSERVATIVE 0.15 (was 0.30) so jittered positions stay > diameter
    // apart for the reduced max-scale range below.
    const float kSpiralJitterFrac = 0.15f;
    // Height curve: HYBRID sqrt + linear in (radius - innerRadius).
    // Pure-linear had near-flat per-clone dy in the first ~100
    // (constant-density Vogel packs them close together in radius;
    // tiny dR -> tiny dY -> visually reads as "inner rings moving
    // down with distance" because perspective alone determines
    // their screen position).  Hybrid sqrt+linear gives both
    // ends what they need:
    //   * sqrt term dominates at the inner band: the singular
    //     derivative of sqrt at 0 gives a noticeable per-clone
    //     ascent through the first ~100 clones.
    //   * linear term dominates at the outer band: keeps the
    //     back rings rising past where sqrt would taper out so
    //     they project clearly above the near rings after
    //     perspective compression.
    //
    // Computed against the constant-density Vogel formula
    //   r = sqrt(innerR^2 + i * spacing^2)
    // with spacing=1.0, so r(i=100)=10.97, r(i=10K)=100.1:
    //     i=0:    y = -1.50
    //     i=1:    y ~= -1.32  (visible per-clone rise from the start)
    //     i=10:   y ~= -0.85
    //     i=100:  y ~= +0.74
    //     i=1K:   y ~= +5.24
    //     i=10K:  y ~= +17.7
    // y curve: kInnerY chosen to make the stadium-seat curve (sqrt+linear
    // height rise with radius) visible in the camera FOV.  Camera orbits
    // at radius 3.5..6.5, eye y=1.5 looking at (0.5, 0, 0) with 60deg
    // vertical FOV -- visible y above the look-at line is only ~+1.5
    // units; visible y below is ~-5 units.  So lifting clones high up
    // pushes outer rings ABOVE the FOV entirely.
    //
    // Per user request "translate all of it down somewhat -- initial
    // objects can start slightly below the checker floor": kInnerY=-1.0
    // puts the innermost clone CENTER below the floor top (y=-0.56) so
    // the inner few clones overlap the floor edge -- intentional, makes
    // the spiral 'rise out of' the floor.  Floor itself is opaque/glass
    // so the partial intersection reads as the floor's surface being
    // partly occluded by clones sitting on the lip.
    const float kInnerY          = -1.00f;
    const float kHeightSqrtMul   =  0.50f;
    const float kHeightLinearMul =  0.15f;
    const float kGoldenAngleRad = 2.39996323f;        // golden angle in radians

    // Distance LOD.  On whenever there are extra clones, NOT just at
    // 1K/10K.  Earlier the gate was "tiers above 100" -- but the
    // pooled BLAS allocation (m_clusterBlasPoolBuffer) now sizes
    // each BLAS slot to its actual cluster count, so without LOD at
    // N=100 every clone gets a full-tessellation slot.  That made
    // N=100 cluster mode reserve MORE BLAS memory than N=1K (where
    // most clones are LOD'd to a handful of clusters each) -- a
    // confusing non-monotonic-with-N stat the user spotted.  Turning
    // LOD on at N=100 keeps the inner ring at full detail (the
    // kLodFullDetailFrac zone still covers the first 25% of the
    // radial range) and only LOD's the outer 75% of clones, which
    // restores monotonic memory scaling.
    const bool  kLodEnabled = (m_extraInstancesMode != ExtraInstancesMode::None);
    const float kLodFullDetailFrac = 0.25f;  // first 25% of radial range = full detail
    // Pre-compute maxRadius matching the spiral formula so we can
    // normalize the radius-to-LOD-bucket lookup.
    const float kMaxRadius     = kInnerRadius + sqrtf((float)std::max(N_extra, 1u)) * kRadialSpacing;
    const float kLodRadialSpan = std::max(0.001f, kMaxRadius - kInnerRadius);
    if (kLodEnabled)
        EnsureCloneSourceLodMeshes();

    // Per-clone deterministic RNG seeded from i.  Each clone i gets its
    // OWN std::mt19937 keyed by a hash of i; this guarantees the random
    // payload (material slot, rotation, scale) for clone i is identical
    // regardless of N, regardless of whether any other random consumer
    // was added in between (e.g. adding a new randomized field per
    // clone won't shift earlier clones).  Property: toggling
    // 100 -> 1K -> 10K extends the spiral by placing 900 / 9000 new
    // clones at indices >= the previous N WITHOUT changing the
    // positions, materials, rotations, or scales of any of the
    // previously-existing clones.
    constexpr UINT kCloneRngBaseSeed = 0xC10E5EEDu;
    std::uniform_int_distribution<UINT> matSlotDist(0u, (UINT)m_materials.size() - 1u);
    std::uniform_real_distribution<float> rotDist(0.0f, 6.2831853f);
    // Per-clone random scale.  Range tuned so that even the LARGEST
    // clone (max scale 0.7 × unit-sphere base × 1.3 wave envelope =
    // 0.91 effective radius) fits comfortably within the spiral's
    // ~1.0-unit radial spacing -- no inter-clone intersections at
    // jittered Vogel positions.  Was 0.6..1.4 which routinely
    // overlapped neighbours at the inner spiral radii.
    std::uniform_real_distribution<float> scaleDist(0.40f, 0.70f);

    m_objects.reserve(m_sourceObjectCount + N_extra);
    for (UINT i = 0; i < N_extra; ++i)
    {
        const UINT cycleSlot = i % srcPoolSize;

        // Per-clone RNG keyed by i.  Hash-mix the index with the base
        // seed using a Wang-style mixer so neighboring i's produce
        // visually-uncorrelated random values (a raw `seed ^ i` would
        // give nearly-identical RNG sequences for adjacent clones).
        UINT mixed = (UINT)i + kCloneRngBaseSeed;
        mixed = ((mixed >> 16) ^ mixed) * 0x119DE1F3u;
        mixed = ((mixed >> 16) ^ mixed) * 0x119DE1F3u;
        mixed =  (mixed >> 16) ^ mixed;
        std::mt19937 perCloneRng(mixed);

        // Spiral placement: Vogel/sunflower base with per-clone jitter
        // to break the periodic spiral arms.  Radius formula
        //   r = sqrt(innerR^2 + i * spacing^2)
        // grows r^2 linearly with i so each clone gets the SAME
        // annulus area (= pi * spacing^2 ~= 3 sq units at spacing
        // 1.0), giving constant density throughout the spiral.  The
        // jitter on angle + radius defeats the visible Vogel spiral
        // curves that radiate out as empty space (artifact of the
        // golden-angle's regularity) without disturbing the overall
        // density.  Height is linear in (radius - innerR) so back
        // rings get enough world y-rise to project above the near
        // rings after perspective compression.
        std::uniform_real_distribution<float> jitterDist(-0.5f, 0.5f);
        const float angleJitter   = jitterDist(perCloneRng) * kGoldenAngleRad * kSpiralJitterFrac;
        const float radiusJitter  = jitterDist(perCloneRng) * kRadialSpacing  * kSpiralJitterFrac;
        const float angle    = (float)i * kGoldenAngleRad + angleJitter;
        const float radiusBase = sqrtf(kInnerRadius * kInnerRadius
                                       + (float)i * kRadialSpacing * kRadialSpacing);
        const float radius   = std::max(kInnerRadius, radiusBase + radiusJitter);
        const float rOff     = std::max(0.0f, radius - kInnerRadius);
        const float heightY  = kInnerY + sqrtf(rOff) * kHeightSqrtMul
                                       + rOff * kHeightLinearMul;
        const DirectX::XMFLOAT3 spiralPos = {
            radius * cosf(angle),
            heightY,
            radius * sinf(angle)
        };
        const DirectX::XMFLOAT3 randRot = { rotDist(perCloneRng), rotDist(perCloneRng), rotDist(perCloneRng) };
        const float             randScale = scaleDist(perCloneRng);
        const UINT              randMatSlot = matSlotDist(perCloneRng);


        if (cycleSlot == kAnimCycleSlot)
        {
            // Animated clone: lives in the SAME spiral as the static
            // clones (every 5th slot in the cycle is the anim variant).
            // Same spiralPos, same per-clone random rotation + scale.
            AnimatedCloneInstance ac;
            ac.worldPos             = spiralPos;
            ac.worldRotEuler        = randRot;
            ac.worldScale           = randScale;
            ac.materialOverrideSlot = 0xFFFFFFFFu;
            m_animatedClones.push_back(ac);
            continue;
        }

        // Static clone path: deep-copy a source ClusterObject.
        const UINT srcIdx = srcIndices[cycleSlot];
        // Index-into-pre-clone vector is safe because reserve() above
        // prevents reallocation; capturing by value avoids dangling-ref
        // worries either way.
        ClusterObject clone = m_objects[srcIdx];   // deep-copy mesh + encoded
        // Reset all GPU-resource handles -- clones get fresh allocations
        // through the normal build pipeline.
        clone.blasStorage.Reset();      clone.blasGPUVA      = 0;
        clone.tradVertexBuffer.Reset(); clone.tradIndexBuffer.Reset();
        clone.tradNormalsBuffer.Reset();
        clone.tradBlasStorage.Reset();  clone.tradBlasScratch.Reset();
        clone.tradBlasGPUVA      = 0;
        clone.tradVertexCount    = 0;
        clone.tradTriangleCount  = 0;
        clone.tradBlasResultBytes  = 0;
        clone.tradBlasScratchBytes = 0;

        // Distance LOD: at the higher tiers, swap the clone's mesh for
        // a pre-generated lower-tessellation variant of the source.
        // Tessellation regen produces a complete (closed) lower-poly
        // mesh -- no missing chunks like the old "drop trailing
        // clusters" approach.  Cluster count drops quartically with
        // LOD level; tris-per-cluster stays at the source's value
        // (~256 sphere, ~128 torus/klein), so per-cluster CLAS
        // overhead stays proportional and we don't waste cluster
        // slots on a handful of tris at extreme LOD.
        if (kLodEnabled)
        {
            auto it = m_cloneSourceLodMeshes.find(m_objects[srcIdx].instanceID);
            if (it != m_cloneSourceLodMeshes.end() && !it->second.empty())
            {
                const float tRaw = std::clamp(
                    (radius - kInnerRadius) / kLodRadialSpan, 0.0f, 1.0f);
                // Remap so [0..kLodFullDetailFrac] -> 0 and
                //         [kLodFullDetailFrac..1] -> [0..1].
                const float tEff = (tRaw <= kLodFullDetailFrac)
                    ? 0.0f
                    : (tRaw - kLodFullDetailFrac) / (1.0f - kLodFullDetailFrac);
                // Map tEff in [0,1] to integer LOD level in
                // [0, chainSize-1] -- linear bucketing.  An exponential
                // mapping would push lower LODs further out, but the
                // outer-ring spiral spacing already does that.
                const int chainSize  = (int)it->second.size();
                int       lodLevel   = (int)std::floor(tEff * (float)chainSize);
                if (lodLevel >= chainSize) lodLevel = chainSize - 1;
                if (lodLevel > 0)
                {
                    // Swap to the lower-LOD mesh.  The rebuild path rebases
                    // its cluster IDs, refreshes shader-side tables, and
                    // re-encodes COMPRESSED1 from this mesh.
                    clone.mesh = it->second[lodLevel];
                    clone.encoded.clear();
                }
            }
        }

        clone.worldPos      = spiralPos;
        clone.worldRotEuler = randRot;
        clone.worldScale    = randScale;
        // Repurpose instanceID as the random material override slot;
        // BuildTlasClassic reads it for clones (index >= m_sourceObjectCount).
        clone.instanceID    = randMatSlot;

        m_objects.push_back(std::move(clone));
    }

    // Recompute global cluster offsets and assign scene-unique IDs.  LOD
    // generators start IDs at zero; without rebasing, their ClusterID()-keyed
    // shader data aliases the full-detail source mesh.
    UINT runningOffset = 0;
    for (auto& obj : m_objects)
    {
        obj.globalClusterStart = runningOffset;
        obj.clusterCount       = (UINT)obj.mesh.clusters.size();
        if (!obj.mesh.clusters.empty())
        {
            const UINT localFirstClusterID = obj.mesh.clusters.front().clusterID;
            for (auto& cluster : obj.mesh.clusters)
            {
                cluster.clusterID = runningOffset + (cluster.clusterID - localFirstClusterID);
                cluster.matchedColorCid = runningOffset
                    + (cluster.matchedColorCid - localFirstClusterID);
            }
        }
        runningOffset += obj.clusterCount;
    }
    m_totalClusterCount = runningOffset;
    m_totalTriangleCount = 0;
    for (const auto& obj : m_objects)
        m_totalTriangleCount += obj.mesh.totalTriangles;

    SampleLog::LogF(L"[clones] N=%u extra; %zu static clones + %zu animated clones "
                    L"(LOD %s); m_objects=%zu, m_totalClusterCount=%u, m_totalTriangleCount=%u\n",
                    N_extra,
                    m_objects.size() - m_sourceObjectCount,
                    m_animatedClones.size(),
                    kLodEnabled ? L"on" : L"off",
                    m_objects.size(),
                    m_totalClusterCount, m_totalTriangleCount);
}
