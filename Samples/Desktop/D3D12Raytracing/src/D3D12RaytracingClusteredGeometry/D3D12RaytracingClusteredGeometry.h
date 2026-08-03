//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include "DXSample.h"
#include "StepTimer.h"
#include "ProceduralGeometry.h"
#include "Compressed1.h"
#include "RaytracingHlslCompat.h"
#include "SceneCommon.h"          // CheckerConfig shared with SceneData::ObjectSpec
#include <DirectXMath.h>
#include <array>
#include <chrono>
#include <string>
#include <vector>
#include <unordered_map>

namespace GlobalRootSig {
    enum {
        OutputUAVSlot = 0,
        AccelerationStructureSlot,
        SceneCBVSlot,
        MaterialsSRVSlot,           // Stage C: per-instance MaterialDesc[] indexed by InstanceID()
        ClusterNormalsSRVSlot,      // Stage F: StructuredBuffer<float3> g_clusterNormals  (per-vertex)
        ClusterIndicesSRVSlot,      // Stage F: StructuredBuffer<uint>   g_clusterIndices  (uint32 per index)
        ClusterOffsetsSRVSlot,      // Stage F: StructuredBuffer<uint2>  g_clusterOffsets  (per-cluster
                                    //                                                      vertOff, idxOff)
        ClusterMetaSRVSlot,         // Refactor: ByteAddressBuffer of ClusterMeta[] - data-driven
                                    //           per-cluster material/colour override metadata.
        // Traditional-BLAS path: per-triangle cluster-ID lookup +
        // per-(InstIdx, GeomIdx) tri-base table.  Lets the traditional
        // closest-hit recover the same cid the cluster path gets from
        // ClusterID(), even though geom descs now group multiple
        // clusters into one material-region geometry.
        TradTriToCidSRVSlot,
        TradGeomTriBaseSRVSlot,
        // Per-(InstIdx, GeomIdx) material-slot lookup used by the clustered
        // path.  The traditional path retains its per-cluster materialSlot
        // fallback for the mixed-material sphere (see LoadHitContext).
        PerInstGeomMaterialSRVSlot,
        // Per-instance material override.  One UINT per TLAS instance,
        // indexed by InstanceIndex().  Sentinel 0xFFFFFFFF = "use the
        // path-specific material lookup" (default for non-clone instances).
        // Anything else = apply that material slot
        // uniformly to every cluster of the instance.  Used by the
        // [N] workload-scaling toggle to give cloned instances visual
        // variety without per-clone CLAS/BLAS storage.
        InstanceMaterialOverrideSRVSlot,
        Count
    };
}

// One object in the scene. Each object becomes one Cluster BLAS in the TLAS.
struct ClusterObject
{
    ProceduralGeometry::Mesh                 mesh;            // CPU-side mesh + cluster decomp
    // Per-cluster vertex payload for the COMPRESSED1 path.  Populated for
    // every object at scene-build time regardless of the active VertexMode
    // (cheap CPU encode; ~50 ms one-time) so the runtime keyboard toggle
    // ('v' in OnKeyDown) can swap formats without re-running BuildScene.
    // The FLOAT32_3 path reads obj.mesh.clusters[i].positions directly at
    // upload time and never touches this buffer.
    std::vector<Compressed1::EncodedCluster>             encoded;
    UINT                                     globalClusterStart = 0;
    UINT                                     clusterCount       = 0;

    // Set after BuildBlasFromClasIndirect (we use EXPLICIT_DESTINATIONS so the
    // BLAS GPU VA is known on CPU and can be baked into the TLAS instance).
    Microsoft::WRL::ComPtr<ID3D12Resource>   blasStorage;
    D3D12_GPU_VIRTUAL_ADDRESS                blasGPUVA = 0;

    // ---- Traditional (DXR1) BLAS path -------------------------------------
    // Populated by BuildTraditionalStaticAS().  Concatenated per-object data
    // (vertices/indices/normals across all this object's clusters merged into
    // single contiguous buffers, with indices renumbered to address into the
    // Traditional (DXR1) per-object resources.  Populated by
    // BuildTraditionalStaticAS the FIRST time we render traditional mode
    // (lazy-init), torn down on cluster-mode switch.
    // tradVertexBuffer / tradIndexBuffer are NULL when the object lives in
    // the pooled trad-VB / trad-IB buffers (m_tradVertexPool / m_tradIndexPool
    // owned by D3D12RaytracingClusteredGeometry); the per-object GVA is then
    // tradVbGPUVA / tradIbGPUVA + offset.  At [N] >= 1000 trad mode the pool
    // turns 16K+ CreateCommittedResource calls into 2, cutting startup time
    // by ~16 seconds on RTX 4090.
    Microsoft::WRL::ComPtr<ID3D12Resource>   tradVertexBuffer;     // float3[] all verts of this object
    Microsoft::WRL::ComPtr<ID3D12Resource>   tradIndexBuffer;      // uint32[] all triangles (3*tri_count uints)
    Microsoft::WRL::ComPtr<ID3D12Resource>   tradNormalsBuffer;    // float3[] per-vertex normals
    // Pool-owned GVAs (valid only when the per-object ComPtr above is null).
    D3D12_GPU_VIRTUAL_ADDRESS                tradVbGPUVA = 0;
    D3D12_GPU_VIRTUAL_ADDRESS                tradIbGPUVA = 0;
    // tradBlasStorage is NULL when the per-object BLAS lives in
    // m_tradBlasWorstcasePool / m_tradBlasCompactPool (the BLAS pool);
    // tradBlasGPUVA below holds the per-object slot GVA in that case.
    Microsoft::WRL::ComPtr<ID3D12Resource>   tradBlasStorage;      // BLAS result
    Microsoft::WRL::ComPtr<ID3D12Resource>   tradBlasScratch;      // BLAS scratch (rebuild) or refit scratch
    D3D12_GPU_VIRTUAL_ADDRESS                tradBlasGPUVA = 0;
    UINT                                     tradVertexCount = 0;
    UINT                                     tradTriangleCount = 0;
    UINT64                                   tradBlasResultBytes  = 0;  // == alloc bytes (worst-case prebuild size)
    UINT64                                   tradBlasActualBytes  = 0;  // == result in Implicit; compacted size in Compact
    UINT64                                   tradBlasScratchBytes = 0;

    // Per-object world placement (used by TLAS instance desc).
    DirectX::XMFLOAT3                        worldPos      = { 0, 0, 0 };
    float                                    worldScale    = 1.0f;
    // Euler rotation in radians, applied as Rx * Ry * Rz before scale +
    // translation. Used so the torus can present its donut hole to the
    // camera and the Klein bottle's figure-8 lobe stands proud instead of
    // being viewed straight-on. Defaults to identity for objects that
    // don't care about orientation.
    DirectX::XMFLOAT3                        worldRotEuler = { 0, 0, 0 };
    UINT                                     instanceID    = 0;
    // Per-(material-region) material-slot override.  Empty for single-
    // region (single-material) objects -- material lookup then falls back
    // to instanceID.  For mixed-material objects, index N is the material
    // slot for matRegionIdx N.  The clustered path reads this mapping through
    // g_perInstGeomMaterial; the traditional mixed-region fallback bakes the
    // same mapping into ClusterMeta.
    std::vector<UINT>                        perRegionMaterialSlot;

    // ------------------------------------------------------------------
    // Per-object SCENE/ART config copied from SceneData::ObjectSpec at
    // BuildScene() time.  Drives the GENERIC BuildClusterMetadata() pass
    // which produces the per-cluster GPU buffer: shader has zero per-
    // object branches.
    // ------------------------------------------------------------------
    CheckerConfig checker;            // see SceneCommon.h

    // Per-object TINT MULTIPLIERS (applied to ALL clusters of this object,
    // baked into per-cluster ClusterMeta).  Each multiplies the global
    // clusterTint slider before its respective lerp blend.
    float surfTintMul = 1.0f;
    float refrTintMul = 0.50f;
    float reflTintMul = 1.08f;

    // True for non-orientable / self-intersecting surfaces (Klein bottle)
    // that need D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE on
    // their TLAS instance.  Back-face culling is fine and faster for
    // every orientable mesh in this scene (sphere, torus, cube, slab),
    // so we only pay the double-sided-traversal cost on the one instance
    // that needs it.
    bool nonOrientable = false;
};

class D3D12RaytracingClusteredGeometry : public DXSample
{
public:
    D3D12RaytracingClusteredGeometry(UINT width, UINT height, std::wstring name);

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // DXSample messages.
    virtual void OnInit() override;
    // Override DXSample's default-false: this sample maximizes on launch
    // on hardware adapters in interactive (non-headless) mode.  See the
    // .cpp implementation for the exact gating.
    virtual bool ShouldMaximizeWindowOnLaunch() const override;
    virtual void OnUpdate() override;
    virtual void OnRender() override;
    virtual void OnSizeChanged(UINT width, UINT height, bool minimized) override;
    virtual void OnDestroy() override;
    virtual void OnKeyDown(UINT8 key) override;
    virtual IDXGISwapChain* GetSwapchain() override { return m_deviceResources->GetSwapChain(); }
    virtual void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc) override;

private:
    static const UINT FrameCount = 3;

    // ---------- DXR1 + DXR2 device / cmdlist interfaces ----------
    ComPtr<ID3D12Device5>                m_dxrDevice;
    ComPtr<ID3D12GraphicsCommandList4>   m_dxrCommandList;
    ComPtr<ID3D12DeviceRaytracing2>      m_dxr2Device;
    ComPtr<ID3D12CommandListRaytracing2> m_dxr2CommandList;
    bool m_clustersAndPtlasSupported = false;
    // Aggregated traditional-static-BLAS sizes, refreshed each rebuild for
    // the overlay's apples-to-apples comparison vs the cluster path's
    // CLAS+BLAS totals.  See BuildTraditionalStaticAS.
    UINT64 m_traditionalStaticTotalResultBytes  = 0;   // sum of obj.tradBlasResultBytes (== alloc bytes)
    UINT64 m_traditionalStaticTotalActualBytes  = 0;   // == result bytes in Implicit; sum of compacted sizes in Compact
    UINT64 m_traditionalStaticTotalScratchBytes = 0;
    double m_traditionalStaticBuildMs            = 0.0;
    // Per-triangle cluster-ID lookup table for the traditional-BLAS
    // closest-hit.  Indexed by a (per-instance, per-geom) tri base +
    // PrimitiveIndex().  Returns the global cluster ID for that
    // triangle's source CLAS in the cluster path, so the same
    // g_clusterMeta / g_clusterNormals / etc. tables feed both paths.
    //
    // Cluster path does NOT read this: it uses ClusterID() directly
    // (DXR2 intrinsic).  Traditional path needs it because grouping
    // multiple clusters into one geometry desc (per-material-region
    // layout) means GeometryIndex() no longer maps 1:1 to clusters.
    //
    // Sizes for the static scene: ~44k tris × 4 = 176 KB tri-to-cid;
    // ~32 entries (8 instances × up to MaxGeomsPerInst) × 4 = 128 B
    // for the geom tri-base table.  See BuildTradCidLookup.
    ComPtr<ID3D12Resource>               m_tradTriToCidBuffer;
    ComPtr<ID3D12Resource>               m_tradGeomTriBaseBuffer;
    // Per-(InstanceIdx, GeometryIdx) material slot lookup.  One uint
    // per slot.  Closesthit reads
    //   matSlot = g_perInstGeomMaterial[InstIdx*MaxGeoms + GeomIdx]
    //   mat     = g_materials[matSlot]
    // For single-region objects only entry 0 is meaningful (set to
    // obj.instanceID so the lookup matches the legacy InstanceID()-
    // based behaviour).  For multi-region objects (e.g. the mixed-
    // material small sphere with chrome upper / glass lower) each
    // region gets its own material slot.
    //
    // Mixed-material instances select a dedicated shader-table block at TLAS
    // build time.  Both regions use GlassHit; the material lookup above makes
    // refractivity 0 on chrome (no refraction trace) and nonzero on glass.
    ComPtr<ID3D12Resource>               m_perInstGeomMaterialBuffer;
    // Max geometries per instance the shader is willing to look up.
    // Padding for the flat per-(InstIdx, GeomIdx) layout.  Bump if a
    // future object grows past it.
    static constexpr UINT                kMaxGeomsPerInstance = 8;

    // Vertex format for cluster builds. Toggle via --vertex-format float|compressed.
    // Default is FLOAT32_3; both formats are validated on experimental WARP.
    enum class VertexMode { Compressed1, Float32_3 };
    VertexMode                           m_vertexMode = VertexMode::Float32_3;

    // ---------- CLAS memory-allocation strategy (selectable via --clas-alloc) ----------
    // Implicit  - one IMPLICIT_DESTINATIONS build into a worst-case-sized buffer.
    //             Simplest, lowest CPU overhead, highest steady memory.
    // GetSizes  - 2-pass: first runs MODE_GET_SIZES to learn per-cluster bytes,
    //             then EXPLICIT_DESTINATIONS build into an exact-sized buffer.
    //             No over-allocation; CPU stalls on size-array readback.
    // Compact   - 1-pass IMPLICIT build (worst-case alloc) PLUS a per-cluster
    //             size readback (free side effect of the build); then
    //             MOVE_CLUSTER_OBJECTS in IMPLICIT mode compacts the live CLAS
    //             into a tightly-sized buffer. Peak GPU memory = worst-case +
    //             compacted; final GPU memory = compacted.
    enum class ClasAllocMode { Implicit, GetSizes, Compact };
    // Default to Compact = implicit-dest worst-case build + MOVE_CLUSTER_OBJECTS
    // post-build pack-down.  Gives tight final memory (~5x smaller than raw
    // Implicit on this scene) at the cost of a one-time GPU flush during the
    // initial AS build.  Implicit mode is still selectable via --clas-alloc
    // implicit / cycling with [A] for users who want the "no post-process,
    // worst-case alloc" baseline to compare against.
    ClasAllocMode                        m_clasAllocMode = ClasAllocMode::Compact;

    // ---------- Static-AS per-frame rebuild mode (cycled via [R]) ----------
    // Simulates LOD-driven AS churn by forcing rebuilds every frame even
    // though the inputs don't actually change.  The timing readouts in the
    // overlay show the per-frame cost.
    //   None        - static AS built once at init (current default)
    //   BlasOnly    - re-run BUILD_BLAS_FROM_CLAS every frame, in place
    //   ClasAndBlas - re-run static CLAS build AND BLAS build every frame
    //                 (only works in CLAS Implicit alloc-mode; in other
    //                 modes the existing CLAS result buffer is sized for
    //                 compacted/exact-fit output and can't safely host a
    //                 re-run, so this falls back to BlasOnly with an
    //                 overlay note)
    enum class StaticRebuildMode { None, BlasOnly, ClasAndBlas };
    StaticRebuildMode                    m_staticRebuildMode = StaticRebuildMode::None;
    // Headless measurement helpers (set via --log-pf-every / --exit-after-frames /
    // --at <frame>:<action>).  Zero = disabled.  Frame counter already exists
    // as m_framesRendered.  See OnRender for usage.
    UINT                                 m_logPfEveryFrames   = 0;
    UINT                                 m_logRawPfEveryFrames = 0;
    UINT                                 m_exitAfterFrames    = 0;
    // Wall-clock benchmark mode: when m_benchSeconds > 0, the app waits for
    // the first rendered frame post-init, then runs for m_benchSeconds
    // wall-clock seconds, then writes a structured JSON snapshot to
    // m_benchOutPath (adapter+driver+config+memory+timings+FPS) and posts
    // WM_QUIT.  Wall-clock instead of frame-count because hardware FPS
    // varies wildly across adapters (RTX 4090 ~120 fps vs WARP ~0.1 fps);
    // a frame-count exit on slow adapters would either fire before init
    // settled or take hours.  See WriteBenchmarkSnapshot().
    double                               m_benchSeconds         = 0.0;
    std::wstring                         m_benchOutPath;
    std::chrono::steady_clock::time_point m_benchStartWallTime = std::chrono::steady_clock::time_point::min();
    bool                                 m_benchSnapshotWritten = false;
    // Scheduled actions: each entry = (frame index, action key).  Action keys
    // are short strings matched in OnRender; supported = "alloc-implicit" /
    // "alloc-getsizes" / "alloc-compact" / "rebuild-none" / "rebuild-blas" /
    // "rebuild-clas-blas" / "log" / "exit".  Multiple --at args allowed,
    // executed in frame order (stable).
    struct ScheduledAction { UINT frame; std::wstring action; };
    std::vector<ScheduledAction>         m_scheduledActions;
    const wchar_t*                       StaticRebuildModeName() const
    {
        switch (m_staticRebuildMode)
        {
        case StaticRebuildMode::None:        return L"off";
        case StaticRebuildMode::BlasOnly:    return L"BLAS only";
        case StaticRebuildMode::ClasAndBlas: return L"CLAS + BLAS";
        }
        return L"?";
    }

    // ---------- Geometry path ([T] toggle) ----------
    // Lets the demo flip between the cluster-based DXR2 pipeline (default)
    // and a traditional DXR1-style per-object monolithic BLAS, so the user
    // can directly A/B the per-frame cost, memory footprint, build wall-
    // clock, and per-cluster colour control between the two paths.
    //
    //   Clusters     - one CLAS per source cluster; per-object BLAS built
    //                  from those CLAS (DXR2 path, current default)
    //   Traditional  - one classic BLAS per object built from concatenated
    //                  triangle data (DXR1 path)
    enum class GeometryMode { Clusters, Traditional };
    GeometryMode                         m_geometryMode = GeometryMode::Clusters;
    const wchar_t*                       GeometryModeName() const
    {
        switch (m_geometryMode)
        {
        case GeometryMode::Clusters:    return L"clustered";
        case GeometryMode::Traditional: return L"traditional";
        }
        return L"?";
    }
    bool IsTraditional() const { return m_geometryMode == GeometryMode::Traditional; }

    // ---------- Animated-BLAS update strategy (Traditional mode only,
    //            cycled via [F]) ----------
    //   Rebuild - full BUILD_RAYTRACING_ACCELERATION_STRUCTURE each frame
    //             from updated vertices.  Slowest, most correct, no
    //             topology constraints.
    //   Refit   - ALLOW_UPDATE + PERFORM_UPDATE flags; driver edits the
    //             existing BLAS in place from updated vertex positions.
    //             Cheaper but only valid while topology is unchanged
    //             (same index/vertex count, same triangle ordering).
    enum class TraditionalAnimMode { Rebuild, Refit };
    // Traditional-BLAS allocation strategy.  Mirrors the cluster path's
    // ClasAllocMode but at the per-object-BLAS level rather than per-CLAS.
    //   Implicit -- one shot: prebuild reports a worst-case size, allocate
    //               that, build into it, done.  No post-build copy.  This
    //               is the simplest path and the buffer is whatever size
    //               the driver thinks the build needs at worst.
    //   Compact  -- build with ALLOW_COMPACTION into a worst-case buffer,
    //               emit POSTBUILD_INFO_COMPACTED_SIZE per BLAS, GPU-flush,
    //               read back the actual compacted sizes, allocate a tight
    //               compact buffer per object, CopyRaytracingAccelerationStructure
    //               with COPY_MODE_COMPACT into it, release the worst-case
    //               source.  This is what production engines actually
    //               ship and gives the smaller of the two numbers on
    //               every adapter I've measured.
    enum class TraditionalAllocMode { Implicit, Compact };
    TraditionalAnimMode                  m_traditionalAnimMode  = TraditionalAnimMode::Rebuild;
    TraditionalAllocMode                 m_traditionalAllocMode = TraditionalAllocMode::Compact;
    const wchar_t*                       TraditionalAnimModeName() const
    {
        switch (m_traditionalAnimMode)
        {
        case TraditionalAnimMode::Rebuild: return L"rebuild";
        case TraditionalAnimMode::Refit:   return L"refit";
        }
        return L"?";
    }
    const wchar_t*                       TraditionalAllocModeName() const
    {
        switch (m_traditionalAllocMode)
        {
        case TraditionalAllocMode::Implicit: return L"implicit (no compaction)";
        case TraditionalAllocMode::Compact:  return L"post-build compact";
        }
        return L"?";
    }

    // ---------- Workload scaling: extra cloned instances ----------
    // [N] cycles through {None, Hundred, Thousand, TenThousand} to add
    // a tail of extra TLAS instances after the static + animated ones.
    // 10K cap (instead of 100K) keeps memory in a feasible band on a
    // 4090-class card: each clone gets its OWN full CLAS array + BLAS
    // (no sharing), so per-clone storage is ~tens-to-hundreds of KB
    // depending on which source it cycled from -- 10K * ~150 KB ~= 1.5 GB
    // of acceleration-structure storage, plus the per-frame
    // template-instantiate / BLAS-from-CLAS cost for animated clones
    // (every 8th clone is an animated one).  100K is left as a
    // commented-out tier for users on bigger cards.
    //
    // Each extra instance:
    //   * Cycles through the source-object pool (m_objects[0..N) + the
    //     animated object as the wrap-around N+1th).  Cycle wrap-arounds
    //     spawn additional animated clones, each with its own per-frame
    //     CLAS / BLAS work (the WHOLE point: stress
    //     CLAS+BLAS+templates progressively).
    //   * Builds its OWN CLAS array (one CLAS per source cluster) and
    //     its OWN BLAS-from-CLAS into private GPU resources.  No CLAS
    //     or BLAS GVA is shared between clones; the TLAS sees N_total
    //     distinct BLAS GVAs.
    //   * Gets a random material slot picked from m_materials, fed
    //     into the per-instance override buffer below so the visual
    //     diversity is high (override is uniform per-instance --
    //     applies to every cluster of the instance -- so a cloned
    //     mixed sphere collapses to one material; acceptable for a
    //     workload-scaling demo).
    //   * Sits on a golden-angle sunflower spiral that starts just
    //     outside the floor and rises in height with radius so distant
    //     clones don't fully occlude the near ones.
    enum class ExtraInstancesMode : UINT {
        None        = 0,
        Hundred     = 100,
        Thousand    = 1000,
        TenThousand = 10000,
    };
    ExtraInstancesMode                   m_extraInstancesMode = ExtraInstancesMode::None;
    UINT                                 ExtraInstancesCount() const { return (UINT)m_extraInstancesMode; }
    const wchar_t*                       ExtraInstancesModeName() const
    {
        switch (m_extraInstancesMode)
        {
        case ExtraInstancesMode::None:        return L"0";
        case ExtraInstancesMode::Hundred:     return L"100";
        case ExtraInstancesMode::Thousand:    return L"1,000";
        case ExtraInstancesMode::TenThousand: return L"10,000";
        }
        return L"?";
    }

    // ---------- BVH-build flag selector ----------
    // The fundamental tradeoff knob for BVH builds in BOTH trad and
    // cluster paths: do you want the BUILD to be fast (PREFER_FAST_BUILD
    // / FAST_BUILD -- shallow BVH, cheap to construct, more expensive
    // to traverse), the TRACE to be fast (PREFER_FAST_TRACE / FAST_TRACE
    // -- the driver spends more time choosing splits to produce a tight
    // BVH that traverses cheaply), or NONE (driver picks a default,
    // typically biased toward FAST_TRACE).  This sample defaults to
    // FAST_TRACE so the per-frame DispatchRays cost shows the BVH at
    // its sharpest -- the toggle lets you measure exactly what that
    // optimisation buys you vs the alternative.
    //
    // The triad maps cleanly across both modes:
    //   trad (DXR1) BuildRaytracingAccelerationStructure flags use
    //     D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_*
    //   cluster (DXR2) ExecuteIndirectRTASOperations flags use
    //     D3D12_RTAS_OPERATION_FLAG_*
    // BuildFlagModeDxr1() / BuildFlagModeRtas() return the right enum
    // for each path; ALLOW_UPDATE (trad anim refit) and
    // ALLOW_DATA_ACCESS (cluster CLAS / template) are OR-ed in
    // separately where the build needs them.
    //
    // Toggling this MUST trigger a full RebuildStaticAccelerationStructures
    // since every BLAS / CLAS / template was built with the previous
    // flag value baked into its BVH.
    enum class BuildFlagMode : UINT {
        None      = 0,
        FastBuild = 1,
        FastTrace = 2,
    };
    BuildFlagMode m_buildFlagMode = BuildFlagMode::FastTrace;
public:
    const wchar_t* BuildFlagModeName() const
    {
        // Show the API-specific enum name so toggling [T] cluster<->trad
        // makes the active build flag's identity clear (DXR2 calls the
        // "fast build" variant FAST_OPERATION; DXR1 calls it FAST_BUILD).
        const bool isCluster = (m_geometryMode == GeometryMode::Clusters);
        switch (m_buildFlagMode)
        {
        case BuildFlagMode::None:      return L"NONE";
        case BuildFlagMode::FastBuild: return isCluster ? L"FAST_OPERATION" : L"FAST_BUILD";
        case BuildFlagMode::FastTrace: return L"FAST_TRACE";
        }
        return L"?";
    }
    D3D12_RTAS_OPERATION_FLAGS BuildFlagModeRtas() const
    {
        switch (m_buildFlagMode)
        {
        case BuildFlagMode::None:      return D3D12_RTAS_OPERATION_FLAG_NONE;
        case BuildFlagMode::FastBuild: return D3D12_RTAS_OPERATION_FLAG_FAST_OPERATION;   // DXR2's "fast build" is named FAST_OPERATION
        case BuildFlagMode::FastTrace: return D3D12_RTAS_OPERATION_FLAG_FAST_TRACE;
        }
        return D3D12_RTAS_OPERATION_FLAG_FAST_TRACE;
    }
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS BuildFlagModeDxr1() const
    {
        switch (m_buildFlagMode)
        {
        case BuildFlagMode::None:      return D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
        case BuildFlagMode::FastBuild: return D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
        case BuildFlagMode::FastTrace: return D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
        }
        return D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    }
private:
    // Per-instance material override buffer: one UINT per TLAS instance.
    // Sentinel 0xFFFFFFFF = "keep the path-specific material lookup".
    // Any other value = use g_materials[value] uniformly for every cluster
    // of that instance.  Set 0xFFFFFFFF for non-clone
    // instances and a randomly-picked slot for each clone.  Lives in the
    // global root signature so the closesthit shader can read it via
    // InstanceIndex().
    Microsoft::WRL::ComPtr<ID3D12Resource> m_instanceMaterialOverrideBuffer;

    // ---------- Position-truncate bits (FLOAT32_3 mode only) ----------
    // Per-vertex float positions can have their LOW N mantissa bits zeroed
    // before the CLAS build sees them - the bits-needed savings are passed
    // to the driver via D3D12_RTAS_CLUSTER_TRIANGLES_INPUTS_DESC's
    // MinPositionTruncateBitCount + the per-cluster
    // D3D12_RTAS_OPERATION_BUILD_CLAS_FROM_TRIANGLES_ARGS::PositionTruncateBitCount.
    // The driver may store positions more compactly when both fields agree
    // that low bits are zero. Range 0 (no truncation) to ~22 (kills all
    // mantissa, useful only as a stress test).
    //
    // DEFAULT: 12 bits. On this scene's ~1m bounding box that leaves
    // ~244 micrometer per-vertex resolution - sub-pixel-equivalent at any
    // sane camera distance, visually identical to 0-bit, ~27% smaller CLAS
    // bytes. Pass --position-truncate 0 for the no-truncation baseline,
    // or --position-truncate 16/22 to see the artifacts kick in.
    //
    // For larger scenes you'd pick a smaller value (or normalize positions
    // first); the right knob is "what per-vertex precision do you need in
    // world units" not "how many bits".
    //
    // Ignored in COMPRESSED1 mode (the union slot is taken by
    // MaxCompressedClusterPositionsSize there).
    UINT                                 m_positionTruncateBits = 12;

    // ANTIALIASING / SUPERSAMPLING.  How many primary rays per output pixel,
    // Anti-aliasing samples per pixel for the raygen shader.  Each pixel
    // gets N jittered primary rays; supported values are 1, 2, 4.
    //   Default 0 = AUTO (resolved in OnInit once the adapter is known):
    //     - WARP / Basic Render -> 1   (a single AA sample is the only way
    //       the software rasterizer stays interactive; 4x quadruples the
    //       per-pixel ray cost which already dominates the frame at
    //       ~0.5-3 s/frame.  user explicitly asked for this default in
    //       the dxr2-sample brain branch on 2026-05-18.)
    //     - HW (anything else)  -> 4   (a 4090 handles 4x easily and the
    //       3-bounce reflective glass benefits visibly from the noise
    //       reduction).
    //   Explicit override via --aa-samples N takes precedence over the
    //   auto-default and skips the OnInit resolve below.
    // Pass-through to the raygen shader is via SceneConstantBuffer.miscParams.z.
    UINT                                 m_aaSamplesPerPixel = 0;

    // CLUSTER-RAINBOW VISUALISATION KNOB.  miscParams.w in the scene CB.
    // Multiplies the cosine-palette per-cluster tint into the material
    // baseColor in the closesthit:
    //   tint = lerp(white, ClusterColor(cid), m_clusterTint)
    // Default 0.5 - clusters are clearly visible (this IS a sample about
    // clustered geometry after all) without completely overwriting the
    // material colours.  Set to 0 via --cluster-tint 0 for pure material
    // rendering, 1 for the original "cluster rainbow dominates everything"
    // look.
    float                                m_clusterTint = 0.65f;

    // ---------- Runtime knobs exposed via the on-screen overlay ----------
    // Bounce-depth slider.  Stored as a single int that maps to a (refl, refr)
    // pair via the helpers below, so the sequence has a single canonical
    // direction and going up + back down RESTORES the default +2 gap (rather
    // than collapsing irrevocably once the cap is reached).  Mapping:
    //   slider -2 ..  0:  refl stays at 0, refr ramps 0 -> 2     (gap 0..2)
    //   slider  0 ..  3:  gap is exactly 2  (default lives at slider=3 = (3,5))
    //   slider  3 ..  5:  refr stays at 5, refl ramps 3 -> 5     (gap 2..0)
    // Pipeline's MaxRecursionDepth is 16; deepest reachable here is refr=5
    // plus shadow-ray leaf, well within budget.
    int                                  m_bounceSlider = 3;        // [-2, 5]
    static constexpr int                 kBounceSliderMin = -2;
    static constexpr int                 kBounceSliderMax =  5;
    UINT ReflectionBounces() const
    {
        return (UINT)std::clamp(m_bounceSlider,            0,  5);
    }
    UINT RefractionBounces() const
    {
        return (UINT)std::clamp(m_bounceSlider + 2,        0,  5);
    }

    // Per-component precision for COMPRESSED1 vertex format.  Default 12
    // bits matches the original constexpr that lived in the .cpp; the
    // '[' / ']' keys cycle it in COMPRESSED1 mode (the same keys cycle
    // m_positionTruncateBits when the active mode is FLOAT32_3).  Each
    // change triggers RebuildStaticAccelerationStructures (re-encodes the
    // static compressed blobs and rebuilds animated templates with the same
    // stored format/precision, then rebuilds CLAS/BLAS/TLAS).
    UINT                                 m_compressedBitsPerComponent = 12;
    const wchar_t*                       ClasAllocModeName() const
    {
        switch (m_clasAllocMode)
        {
        case ClasAllocMode::Implicit: return L"implicit-dest (worst-case alloc)";
        case ClasAllocMode::GetSizes: return L"explicit-dest (exact-fit alloc, 2-pass)";
        case ClasAllocMode::Compact:  return L"implicit-dest + post-build compact";
        }
        return L"?";
    }
    // Stats captured by BuildClasIndirect, reported via RenderUI overlay + log.
    struct ClasMemStats
    {
        UINT64 resultPrebuildMax  = 0;  // prebuild.ResultDataMaxSizeInBytes (worst case)
        UINT64 resultInitialBytes = 0;  // what we actually allocated initially
        UINT64 resultFinalBytes   = 0;  // final live result-buffer size after this function returns
        UINT64 peakResidentBytes  = 0;  // max concurrent live result-buffer memory
        UINT64 scratchBytesPhase1 = 0;
        UINT64 scratchBytesPhase2 = 0;
        UINT64 sumActualBytes     = 0;  // sum of per-cluster sizes (= the irreducible CLAS storage)
        double cpuWallMsPhase1    = 0.0;
        double cpuWallMsPhase2    = 0.0;
    };
    ClasMemStats                         m_clasMemStats;

    // ---------- Scene = N procedurally-generated objects ----------
    std::vector<ClusterObject>           m_objects;
    // Source-object count captured the first time the scene is built.
    // m_objects[0..m_sourceObjectCount) are the "real" scene objects;
    // m_objects[m_sourceObjectCount..end()) are clones spawned by the
    // [N] workload-scaling toggle.  RebuildStaticAccelerationStructures
    // truncates m_objects back to m_sourceObjectCount at the top, then
    // regenerates clones for the current m_extraInstancesMode before
    // running the per-object build pipeline.  Zero until the first
    // BuildScene finishes.
    UINT                                 m_sourceObjectCount   = 0;
    UINT                                 m_totalClusterCount = 0;
    UINT                                 m_totalTriangleCount = 0;   // sum of all clusters' tris across static objects + animated (cached at scene-build time for the title bar)

    // ---------- One big upload buffer holding per-cluster vert+idx data
    // for the static CLAS build.  Used to be interleaved with the args
    // array too; now the args live in a separate DEFAULT-heap UAV
    // (m_clasArgsBuffer below) so a compute shader can write them. ----------
    ComPtr<ID3D12Resource>               m_clusterInputBuffer;
    // GPU-written args for the static BUILD_CLAS_FROM_TRIANGLES op.
    // FillClasFromTrianglesArgs CS reads m_clasArgsMetaBuffer + a few root
    // constants and writes here; the RTAS op then reads here as its
    // IndirectArgumentArray.
    ComPtr<ID3D12Resource>               m_clasArgsBuffer;
    // Per-cluster metadata input for the CS above (24 bytes/cluster --
    // see FillClasFromTrianglesArgs.hlsl for the schema).  Upload heap,
    // rebuilt whenever the vertex-format / position-truncate-bits change.
    // Distinct from the shader-side m_clusterMetaBuffer (per-cluster
    // material + colour data consumed by raygen).
    ComPtr<ID3D12Resource>               m_clasArgsMetaBuffer;
    D3D12_GPU_VIRTUAL_ADDRESS            m_clasArgsArrayGPUVA = 0;
    UINT                                 m_clasArgsStride     = 0;

    // ---------- AS results / scratch / address arrays ----------
    // Single CLAS build covers ALL clusters from ALL objects (one batched op).
    ComPtr<ID3D12Resource>               m_clasResultBuffer;
    ComPtr<ID3D12Resource>               m_clasScratchBuffer;
    ComPtr<ID3D12Resource>               m_clasAddressArray;       // N_total_clusters x GVA
    ComPtr<ID3D12Resource>               m_clasSizeArray;          // N_total_clusters x UINT64
    ComPtr<ID3D12Resource>               m_clasMoveArgsBuffer;     // ClasAllocMode::Compact only - kept alive across function returns

    // Single BLAS-from-CLAS build covers all BLASes (one per object).
    ComPtr<ID3D12Resource>               m_blasScratchBuffer;
    // GPU-written N-object BUILD_BLAS_FROM_CLAS_ARGS buffer (was upload-mapped).
    // Filled by FillBlasFromClasArgs CS from m_blasArgsMeta + root constants.
    ComPtr<ID3D12Resource>               m_blasArgsBuffer;         // N_objects x BUILD_BLAS_FROM_CLAS_ARGS
    // Per-object {clusterCount, clasArrayGvaLo, clasArrayGvaHi} input for
    // the BLAS-args CS.  12 bytes/object, upload heap, built once at init.
    ComPtr<ID3D12Resource>               m_blasArgsMeta;
    ComPtr<ID3D12Resource>               m_blasResultAddrBuffer;   // N_objects x GVA (explicit dests)
    // Pool buffer holding ALL static-object BLAS storage in one
    // contiguous allocation.  Each ClusterObject::blasGPUVA points
    // somewhere inside this buffer (poolBase + per-object offset);
    // ClusterObject::blasStorage stays null since the per-object
    // ComPtr would be redundant.  Eliminates the per-clone
    // CreateCommittedResource overhead that made N=10K cluster mode
    // hang on driver-side allocation churn -- with the pool the cost
    // is a single CreateCommittedResource regardless of clone count.
    // EXPLICIT_DESTINATIONS BLAS-from-CLAS mode takes per-arg
    // destination GPU VAs so the indirect build naturally writes
    // each BLAS into its slice of the pool.
    ComPtr<ID3D12Resource>               m_clusterBlasPoolBuffer;
    // Traditional-path BLAS storage pool.  Mirrors the cluster-path
    // m_clusterBlasPoolBuffer optimization: one shared committed
    // buffer holding every static object's BLAS storage in implicit-
    // alloc mode, or holding the WORST-CASE temp BLAS storage during
    // a compact-alloc build's pass 1 (the compacted finals live in
    // m_tradBlasCompactPool below).  Eliminates the per-clone
    // CreateCommittedResource overhead that scales poorly at high
    // [N] clone counts -- 7500 driver calls collapse to ONE.
    ComPtr<ID3D12Resource>               m_tradBlasWorstcasePool;
    // Compact-alloc-mode-only: holds the final compacted BLASes
    // (sized to sum of postbuild compacted sizes).  Pass 2 of the
    // compact build copies each temp BLAS from m_tradBlasWorstcasePool
    // into its slot here via COPY_MODE_COMPACT, then drops the
    // worst-case pool.  obj.tradBlasGPUVA ends up pointing into this.
    ComPtr<ID3D12Resource>               m_tradBlasCompactPool;
    // Traditional-path VB + IB pools.  Each pool is ONE committed UPLOAD
    // resource sized to the sum of every static object's per-object
    // VB / IB bytes; per-object data lives at an offset within the pool
    // and per-object GVAs (ClusterObject::tradVbGPUVA / tradIbGPUVA)
    // point into that slot.  Eliminates 2 x N CreateCommittedResource
    // calls per trad-mode rebuild -- at [N]=10K that's 16000 driver
    // calls collapsing to 2, which is the single biggest contributor
    // to trad N=10K startup time (was ~16-18 seconds, drops to a
    // couple of seconds after pooling).  Per-object ComPtrs above
    // stay null when the pool is in use, signalling that the pool
    // owns the underlying memory.
    ComPtr<ID3D12Resource>               m_tradVertexPool;
    ComPtr<ID3D12Resource>               m_tradIndexPool;
    // Shared scratch buffer used by every per-object BLAS build in
    // the trad-path static AS build.  One buffer sized to the
    // largest single-BLAS scratch requirement; per-object builds
    // run sequentially and emit a UAV barrier on the scratch
    // between them so each build sees the scratch as available
    // again before clobbering it.  Saves another N-1 driver alloc
    // calls.
    ComPtr<ID3D12Resource>               m_tradBlasSharedScratch;

    ComPtr<ID3D12Resource>               m_tlasBuffer;
    ComPtr<ID3D12Resource>               m_tlasScratchBuffer;
    ComPtr<ID3D12Resource>               m_tlasInstanceDescs;      // upload heap, N_objects descs

    // ============ ANIMATED OBJECT (cluster templates + per-frame instantiation) ============
    // One additional object whose Cluster BLAS is rebuilt every frame from
    // pre-built cluster templates. CPU computes new positions per frame from a
    // pulsating-sphere formula; INSTANTIATE_CLUSTER_TEMPLATES turns the
    // templates + fresh positions into CLAS; BUILD_BLAS_FROM_CLAS rebuilds the
    // BLAS storage in-place (the BLAS storage GPU VA stays fixed - the
    // animated TLAS instance always points at the same VA, only the BVH
    // contents change). TLAS is rebuilt every frame to pick up the new BLAS
    // root bounds.
    struct AnimatedObject
    {
        ProceduralGeometry::Mesh             mesh;                   // topology + rest positions
        std::vector<std::vector<DirectX::XMFLOAT3>> hintPositions;   // per-cluster, max-deformation envelope
        UINT                                 clusterCount       = 0;
        UINT                                 maxTrisPerCluster  = 0;
        UINT                                 maxVertsPerCluster = 0;
        UINT                                 totalVertexCount   = 0;
        UINT                                 vertexBufferStride = 0; // bytes per vertex (sizeof(float3))

        DirectX::XMFLOAT3                    worldPos     = {0,0,0};
        float                                worldScale   = 1.0f;
        UINT                                 instanceID   = 0;

        // --- Built once at init ---
        // Hint vertex blob for the cluster-template build (positions + indices).
        Microsoft::WRL::ComPtr<ID3D12Resource> templateInputBuffer;
        // Per-cluster BUILD_CLUSTER_TEMPLATES_FROM_TRIANGLES_ARGS array.
        // GPU-written by FillClusterTemplateArgs CS from templateMetaBuffer
        // + root constants; was a CPU-mapped slice inside templateInputBuffer
        // in the old single-buffer layout.
        Microsoft::WRL::ComPtr<ID3D12Resource> templateArgsBuffer;
        // Per-cluster metadata for the template-args CS (16 bytes/cluster:
        // triCount, vertCount, vbOff, ibOff -- see FillClusterTemplateArgs.hlsl).
        Microsoft::WRL::ComPtr<ID3D12Resource> templateMetaBuffer;
        D3D12_GPU_VIRTUAL_ADDRESS              templateArgsArrayGPUVA = 0;
        UINT                                   templateArgsStride     = 0;
        // Template BVH storage (one templated CLAS per source cluster).
        Microsoft::WRL::ComPtr<ID3D12Resource> templateResultBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> templateScratchBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> templateAddressArray;  // per-cluster GVA into templateResultBuffer

        // Rest-pose positions (DEFAULT-heap SRV) -- read by AnimateBall.cs
        // every frame to compute the ripple offset.  Uploaded ONCE at init
        // from the CPU mesh; immutable after that.  Same flat layout as
        // perFrameVertexBuffer below: tightly-packed float3 per vertex,
        // cluster-major order.
        Microsoft::WRL::ComPtr<ID3D12Resource> restPositionsBuffer;

        // --- Reused every frame ---
        // Per-frame deformed positions.  Written by AnimateBall.cs (UAV) and
        // read by INSTANTIATE_CLUSTER_TEMPLATES (NON_PIXEL_SHADER_RESOURCE);
        // state cycles UAV <-> NON_PIXEL_SHADER_RESOURCE inside
        // UpdateAnimatedObjectPerFrame.  Lives entirely on the GPU; no CPU
        // map.  (Was an UPLOAD-heap mapped buffer in the original CPU-driven
        // path -- see git history for the pre-compute version.)
        Microsoft::WRL::ComPtr<ID3D12Resource> perFrameVertexBuffer;
        // Per-frame INSTANTIATE_CLUSTER_TEMPLATES_ARGS array (one per cluster).
        // Each arg points to its cluster template + the slice of perFrameVertexBuffer.
        // GPU-written by FillInstantiateArgs compute shader (see
        // FillInstantiateArgs.hlsl) instead of the old CPU-side fill -- a
        // real LOD-driven renderer would similarly emit these from a
        // culling/selection CS.  Lives in UNORDERED_ACCESS state; CS write
        // and INSTANTIATE read are separated by a UAV barrier.
        Microsoft::WRL::ComPtr<ID3D12Resource> perFrameInstArgsBuffer;
        // Per-cluster byte offset into perFrameVertexBuffer.  CPU-computed
        // once via prefix sum of per-cluster vertex sizes, uploaded once,
        // read every dispatch by FillInstantiateArgs.  Upload heap (constant
        // across the object's lifetime).
        Microsoft::WRL::ComPtr<ID3D12Resource> vertexOffsetArray;
        // Instantiated CLAS results (rewritten each frame in IMPLICIT_DESTINATIONS mode).
        Microsoft::WRL::ComPtr<ID3D12Resource> perFrameClasResultBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> perFrameClasScratchBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> perFrameClasAddressArray;    // N_clusters x GVA
        // Per-frame INSTANTIATE size readback - written ONCE per config-change
        // by a one-shot measurement INSTANTIATE that runs at the end of
        // BuildAnimatedObjectSetup; the regular per-frame INSTANTIATE in
        // UpdateAnimatedObjectPerFrame doesn't touch these (no readback cost
        // on the hot path).
        Microsoft::WRL::ComPtr<ID3D12Resource> perFrameClasSizesBuffer;       // UAV, N x UINT64
        Microsoft::WRL::ComPtr<ID3D12Resource> perFrameClasSizesReadback;     // readback, N x UINT64
        // BLAS storage (fixed GPU VA across frames, rebuilt in place every frame).
        Microsoft::WRL::ComPtr<ID3D12Resource> blasStorage;
        Microsoft::WRL::ComPtr<ID3D12Resource> blasScratchBuffer;
        // GPU-written single-entry BUILD_BLAS_FROM_CLAS_ARGS for this object,
        // produced by FillBlasFromClasArgs CS (shared with the static path
        // -- see CreateFillBlasArgsPipeline).  DEFAULT/UAV; the per-frame
        // BLAS rebuild reads it as IndirectArgumentArray.  Was an
        // UPLOAD-heap mapped buffer in the original CPU-driven path.
        Microsoft::WRL::ComPtr<ID3D12Resource> blasArgsBuffer;
        // Input metadata for the BLAS-args CS (single 12-byte entry:
        // {clusterCount, gvaLo, gvaHi} pointing at perFrameClasAddressArray).
        Microsoft::WRL::ComPtr<ID3D12Resource> blasArgsMeta;
        Microsoft::WRL::ComPtr<ID3D12Resource> blasResultAddrBuffer;        // upload, 1 entry = blasStorage VA
        D3D12_GPU_VIRTUAL_ADDRESS              blasGPUVA = 0;

        // -------- TRADITIONAL (DXR1) MODE per-frame path --------
        // Built once at init when m_geometryMode == Traditional; reused
        // every frame.  The traditional path does NOT use templates or
        // CLAS -- AnimateBall.cs still writes perFrameVertexBuffer above
        // (shared), then we BuildRaytracingAccelerationStructure on a
        // single per-object D3D12_RAYTRACING_GEOMETRY_DESC built from
        // (perFrameVertexBuffer, tradIndexBuffer).  Per-frame cost:
        // either a full rebuild (PREFER_FAST_TRACE) or an UPDATE (using
        // ALLOW_UPDATE source), selected at runtime via [F]
        // (m_traditionalAnimMode).
        // Flat IB in cluster-major order, with per-cluster local indices
        // biased by each cluster's vertex offset so a single geom desc
        // can address the whole ball.  Built once at init from the mesh,
        // never rewritten (only positions change per frame, not topology).
        Microsoft::WRL::ComPtr<ID3D12Resource> tradIndexBuffer;
        UINT                                   tradTriangleCount = 0;
        // Per-frame-rebuilt traditional BLAS.  Allocated to
        // prebuild.ResultDataMaxSizeInBytes once; the GVA is stable.
        Microsoft::WRL::ComPtr<ID3D12Resource> tradBlasStorage;
        Microsoft::WRL::ComPtr<ID3D12Resource> tradBlasScratchBuffer;
        UINT64                                 tradBlasResultBytes  = 0;
        UINT64                                 tradBlasScratchBytes = 0;
        D3D12_GPU_VIRTUAL_ADDRESS              tradBlasGPUVA        = 0;
        // True when we've done at least one rebuild on tradBlasStorage,
        // so PERFORM_UPDATE has a valid source to refit from.  Flipped
        // on by the rebuild path; cleared by [V] / [T] / mode toggles
        // that drop the BLAS contents (e.g. when going cluster->trad).
        bool                                   tradBlasInitialized  = false;
    };
    AnimatedObject                          m_animatedObject;
    bool                                    m_animatedObjectEnabled = false;

    // [N] workload-scaling animated clones.  Each entry is a "shadow"
    // TLAS instance that points at m_animatedObject.blasGPUVA (cluster
    // mode) or .tradBlasGPUVA (trad mode) -- i.e. clones SHARE the
    // source animated object's per-frame CLAS+BLAS results.  This
    // model adds visual copies in the spiral but does NOT yet stress
    // the per-frame INSTANTIATE / BLAS-from-CLAS path -- each anim
    // clone is "free" per-frame cost-wise (one extra TLAS row).
    // Phase-2 enhancement: per-clone per-frame CLAS+BLAS work (real
    // template/CLAS/BLAS stress).  Deferred to a separate commit.
    struct AnimatedCloneInstance
    {
        DirectX::XMFLOAT3 worldPos      = {0, 0, 0};
        DirectX::XMFLOAT3 worldRotEuler = {0, 0, 0};
        float             worldScale    = 1.0f;
        UINT              materialOverrideSlot = 0;  // for the per-instance override buffer
        // Phase-2 per-clone BLAS GPU VA (cluster mode only).  A slot
        // inside m_animClonesBlasPool; set by BuildAnimatedClonesSetup
        // after the pool is sized; consumed by BuildTlasClassic.
        // Zero in trad mode and at phase-1 baseline (TLAS falls back
        // to the shared source GVA).
        D3D12_GPU_VIRTUAL_ADDRESS blasGPUVA = 0;
    };
    std::vector<AnimatedCloneInstance>      m_animatedClones;
    // Cluster mode only: per-clone BLAS storage pool.  Allocated by
    // BuildAnimatedClonesSetup; each clone's blasGPUVA = poolBase + k*size.
    Microsoft::WRL::ComPtr<ID3D12Resource>  m_animClonesBlasPool;
    // Trad-mode phase-2 analog: per-anim-clone DXR1 BLAS storage pool.
    // Each per-frame UpdateAnimatedTradPerFrame call rebuilds the source
    // animated BLAS and then issues N more BuildRaytracingAccelerationStructure
    // calls, one per anim clone, each into its own pool slot.  DXR1 has
    // no batched-build API so this is genuinely O(N) driver calls per
    // frame -- exactly the kind of overhead DXR2's batched
    // ExecuteIndirectRTASOperations API is designed to eliminate, which
    // is the headline DXR2-vs-DXR1 comparison this sample exists to
    // demonstrate.  Allocated by BuildAnimatedClonesTradSetup.
    Microsoft::WRL::ComPtr<ID3D12Resource>  m_animClonesTradBlasPool;
    Microsoft::WRL::ComPtr<ID3D12Resource>  m_animClonesTradBlasScratch;
    // Phase-2 separate args + result-addr buffers for the per-frame
    // BUILD_BLAS_FROM_CLAS batched op.  We can NOT replace source's
    // obj.blasArgsBuffer / obj.blasResultAddrBuffer in-place because
    // BuildAnimatedObjectSetup just recorded GPU work referencing those
    // resources -- releasing them before the GPU consumes that work is
    // UB.  These are NEW resources sized for (1 + N_animClones) entries
    // that the per-frame op switches to when the pool exists.
    Microsoft::WRL::ComPtr<ID3D12Resource>  m_animClonesBlasArgsBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource>  m_animClonesBlasResultAddrBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource>  m_animClonesBlasScratchBuffer;

    // Pre-generated LOD-chain meshes for the cloneable source types.
    // Keyed by source instanceID (0=sphere0, 4=torus, 8=klein).  Each
    // vector entry is a tessellation level: index 0 = full source
    // resolution, index N = low-poly.  Each level halves both grid
    // dimensions while keeping tile sizes constant -- cluster count
    // drops quartically with LOD level, tris-per-cluster stays at the
    // source's value (cluster overhead stays proportional, total work
    // drops fast).  Generated once per [N] toggle and reused across
    // every clone that lands at the same (source, LOD level) bucket.
    //
    // Tessellation-regen LOD reads as "low-poly" rather than "broken"
    // (no missing chunks, just lower surface resolution) -- the right
    // way to do per-cluster tri-count reduction on procedural geometry,
    // per user feedback "for lower tris per cluster you just tessellate
    // less."  Per-cluster minimum tri count is held at ~64 to avoid
    // per-CLAS metadata overhead dominating at extreme LOD.
    std::unordered_map<UINT, std::vector<ProceduralGeometry::Mesh>> m_cloneSourceLodMeshes;

    // ---------- Raytracing pipeline + shader tables ----------
    ComPtr<ID3D12StateObject>            m_dxrStateObject;
    ComPtr<ID3D12RootSignature>          m_globalRootSignature;
    ComPtr<ID3D12RootSignature>          m_localRootSignature;       // empty; required by WARP
    ComPtr<ID3D12Resource>               m_rayGenShaderTable;
    ComPtr<ID3D12Resource>               m_missShaderTable;
    ComPtr<ID3D12Resource>               m_hitGroupShaderTable;

    // ---------- Compute pipeline: AnimateBall (GPU-side ball deformation) -------
    // Replaces the old per-frame CPU ripple loop.  Root sig has:
    //   slot 0: 32-bit root constants (float t, uint vertexCount)
    //   slot 1: raw SRV (rest positions, t0)
    //   slot 2: raw UAV (animated positions, u0)
    // Used by UpdateAnimatedObjectPerFrame's Dispatch -- one thread group per
    // 64 verts, IM->UAV barrier, then INSTANTIATE_CLUSTER_TEMPLATES reads.
    ComPtr<ID3D12RootSignature>          m_animComputeRS;
    ComPtr<ID3D12PipelineState>          m_animComputePSO;
    // Per-frame anim CS params buffer (mapped, persistent).  Carries
    // (float t, uint vertexCount) for AnimateBall.hlsl.  Reason for using
    // a CB instead of root constants: root constants bake into the cmd
    // list at record time so in-flight cmd lists carry their old (pre-
    // pause) time values until the pipeline drains -- you'd see the wobble
    // animation continue 2-3 frames after the camera froze.  A mapped CB
    // lets the GPU read the LATEST CPU-written content at execute time,
    // so the frozen time value propagates immediately to every in-flight
    // frame, matching the camera's mapped m_sceneCB behaviour.
    ComPtr<ID3D12Resource>               m_animParamsBuf;
    struct AnimParams { float t; UINT vertexCount; UINT pad0; UINT pad1; };
    AnimParams*                          m_animParamsMapped = nullptr;
    // GPU pipeline that writes per-cluster
    // INSTANTIATE_CLUSTER_TEMPLATES_ARGS into AnimatedObject::
    // perFrameInstArgsBuffer.  Built once at init by
    // CreateFillInstantiateArgsPipeline, dispatched once at the end of
    // BuildAnimatedObjectSetup (and on every config-change rebuild).
    ComPtr<ID3D12RootSignature>          m_fillInstArgsRS;
    ComPtr<ID3D12PipelineState>          m_fillInstArgsPSO;
    // Args-fill pipelines for the remaining RTAS op types.  Each writes
    // its corresponding D3D12_RTAS_OPERATION_*_ARGS array from a small
    // CPU-prepared per-entry metadata buffer + a few root constants.
    // See the matching .hlsl files for layout details.
    ComPtr<ID3D12RootSignature>          m_fillMoveArgsRS;
    ComPtr<ID3D12PipelineState>          m_fillMoveArgsPSO;
    ComPtr<ID3D12RootSignature>          m_fillBlasArgsRS;
    ComPtr<ID3D12PipelineState>          m_fillBlasArgsPSO;
    ComPtr<ID3D12RootSignature>          m_fillClasTriArgsRS;
    ComPtr<ID3D12PipelineState>          m_fillClasTriArgsPSO;
    ComPtr<ID3D12RootSignature>          m_fillTemplateArgsRS;
    ComPtr<ID3D12PipelineState>          m_fillTemplateArgsPSO;

    // ---------- Output texture + descriptor heap ----------
    // Descriptor heap layout (CBV_SRV_UAV, shader-visible):
    //   slot 0 = raytracing-output UAV
    //   slot 1 = SegoeUI SpriteFont texture SRV (created inside DirectXTK
    //            during SpriteFont's resource upload).
    // NumDescriptors is 8 with plenty of headroom -- AllocateDescriptor()
    // hands them out monotonically.
    ComPtr<ID3D12DescriptorHeap>         m_descriptorHeap;
    UINT                                 m_descriptorSize       = 0;
    UINT                                 m_descriptorsAllocated = 0;
    ComPtr<ID3D12Resource>               m_raytracingOutput;
    D3D12_GPU_DESCRIPTOR_HANDLE          m_raytracingOutputUAV  = {};

    // ---------- DirectXTK on-screen overlay (SpriteBatch + SpriteFont) ----------
    // Used by RenderUI() to paint per-frame stats + key bindings directly on
    // the back buffer.  Replaces the title-bar text approach (Win11 title bars
    // truncate around ~280 visible chars regardless of available pixel width).
    std::unique_ptr<DirectX::GraphicsMemory> m_graphicsMemory;
    std::unique_ptr<DirectX::SpriteBatch>    m_spriteBatch;
    std::unique_ptr<DirectX::SpriteFont>     m_uiFont;
    // 1x1 white texture used by the overlay's per-segment dark backing.
    // SpriteBatch stretches it to each text segment's exact bounding box
    // (cursor.x..cursor.x+measureX, pos.y..pos.y+kLineH) and tints it
    // with a semi-transparent dark colour, so the bright body text reads
    // against any scene colour underneath.  Allocated in CreateUIFont,
    // lives in the shared descriptor heap.
    Microsoft::WRL::ComPtr<ID3D12Resource>   m_overlayPanelTexture;
    D3D12_GPU_DESCRIPTOR_HANDLE              m_overlayPanelTextureGpu = {};
    // Adaptive overlay sizing -- the overlay TARGETS kScale=0.625 (the
    // size you see at 4K) but if that scale would overflow the back
    // buffer width (typically on 1280x720), kScale is dropped just
    // enough to fit.  Both values below are in UNSCALED atlas pixels
    // (i.e. as if rendered at kScale=1.0) so they're independent of
    // whatever scale the previous frame used.
    //  - m_overlayContentUnscaledWidth: total width of col1 text +
    //    col2 text (NOT including margins or the col1->col2 gap, both
    //    of which are fixed-pixel and don't scale).  This frame's
    //    fit-scale = (bb_width - margins - gap) / this_value, clamped
    //    to kTargetScale.  Monotonic-max so the chosen scale converges
    //    upward as we observe wider lines (e.g. after a mode toggle
    //    that lengthens a number).
    //  - m_overlayCol1MaxRightUnscaled: max right-edge of col1's text
    //    in unscaled units, used to anchor col2.x at any kScale.
    //    Also monotonic-max so col2 only drifts right, never reflows
    //    leftward when col1's numbers shrink between frames.
    // Initial values are calibrated so the first frame on 1280x720
    // renders at <= kTargetScale (won't flash overflow before the
    // measurement catches up); on 4K they're way under the available
    // width so kScale snaps to kTargetScale immediately.
    float                                    m_overlayContentUnscaledWidth  = 2100.0f;
    float                                    m_overlayCol1MaxRightUnscaled  = 1180.0f;
    // Height counterpart -- max bottom edge of any text emitted this
    // frame, max-monotonic in unscaled atlas units, used to clamp
    // kScale to fit the back buffer HEIGHT as well as width.  Handles
    // wide-but-short windows (e.g. 2560x400) where there's plenty of
    // horizontal slack but the keys column would overflow the bottom.
    // Initial value is calibrated for ~the smallest reasonable height
    // (720) -- on bigger heights it's irrelevant (the width constraint
    // dominates), on smaller heights it kicks in immediately so we
    // don't flash overflow.
    float                                    m_overlayContentUnscaledHeight = 1080.0f;

    // ---------- Scene constant buffer ----------
    ComPtr<ID3D12Resource>               m_sceneCB;
    SceneConstantBuffer*                 m_sceneCBMapped = nullptr;

    // ---------- Per-instance materials ----------
    // m_materials[InstanceID] = MaterialDesc.  Indexed by InstanceID() in
    // HLSL via a structured-buffer SRV at root parameter MaterialsSRVSlot.
    // Populated in BuildMaterials() and uploaded once at init time.
    std::array<MaterialDesc, NUM_MATERIAL_SLOTS> m_materials = {};
    ComPtr<ID3D12Resource>               m_materialsBuffer;
    void BuildMaterials();

    // ---------- Per-vertex normal side channel (smooth shading) ----------
    // DXR2 cluster geometry only carries positions in the CLAS vertex buffer,
    // so per-vertex normals and the cluster index buffer have to travel
    // through a SEPARATE channel for the closesthit shader to interpolate
    // them by barycentrics.  Three upload-heap StructuredBuffers + an
    // offset table:
    //
    //   m_clusterNormalsBuffer : float3[]   - all per-cluster normals
    //                                          concatenated.  Vertex offset
    //                                          per cluster comes from the
    //                                          offsets table below.
    //   m_clusterIndicesBuffer : uint[]     - all per-cluster index buffers
    //                                          concatenated.  Stored as
    //                                          uint32 per index for clean
    //                                          StructuredBuffer access; the
    //                                          source uint8 indices are
    //                                          widened on upload.
    //   m_clusterOffsetsBuffer : uint2[]    - indexed by ClusterID().  .x =
    //                                          start in normals buffer; .y
    //                                          = start in indices buffer.
    //                                          Sized to max(ClusterID)+1
    //                                          (sparse - unused slots
    //                                          contain (~0u,~0u)).
    //
    // The closesthit shader then does:
    //   uint cid = ClusterID();
    //   uint2 off = g_clusterOffsets[cid];
    //   uint i0 = g_clusterIndices[off.y + PrimitiveIndex()*3 + 0];
    //   ... interpolate g_clusterNormals[off.x + i_k] via barycentrics.
    //
    // For the animated sphere we reuse its BASE normals (computed once at
    // gen time) - the per-frame morph deformation is small enough that the
    // static normals stay visually plausible without per-frame re-upload.
    ComPtr<ID3D12Resource>               m_clusterNormalsBuffer;
    ComPtr<ID3D12Resource>               m_clusterIndicesBuffer;
    ComPtr<ID3D12Resource>               m_clusterOffsetsBuffer;
    UINT                                 m_clusterNormalsCount = 0;
    UINT                                 m_clusterIndicesCount = 0;
    UINT                                 m_clusterOffsetsCount = 0;
    void BuildClusterShaderSideBuffers();
    // Traditional-BLAS cid recovery: builds the per-triangle cid table
    // + per-(InstanceIdx, GeometryIdx) tri-base table that the
    // traditional path's closest-hit uses to map (GeometryIndex(),
    // PrimitiveIndex()) -> cid.  Called as part of the traditional
    // build path; rebuilt on any geometry-mode toggle.
    void BuildTradCidLookup();
    // Per-(InstIdx, GeomIdx) material-slot lookup.  Built unconditionally
    // at init so the binding's always valid; rebuilt when scene clusters
    // are re-emitted.  See m_perInstGeomMaterialBuffer.
    void BuildPerInstGeomMaterialTable();

    // Per-cluster metadata buffer (ClusterMeta[], indexed by ClusterID()).
    // Drives ALL per-cluster material / colour decisions in the shader -
    // see the big design comment on ClusterMeta in RaytracingHlslCompat.h.
    ComPtr<ID3D12Resource>               m_clusterMetaBuffer;
    UINT                                 m_clusterMetaCount = 0;
    void BuildClusterMetadata();

    // ---------- Timestamp queries for AS build wall-clocks ----------
    // Init-time slots 0..5 straddle the static CLAS / BLAS / TLAS builds (3 pairs).
    // Per-frame uses a separate heap + readback with 3 ring-buffer slots so
    // the CPU reads timestamps written ~3 frames ago (safely past GPU
    // completion) without stalling.
    ComPtr<ID3D12QueryHeap>              m_buildQueryHeap;
    ComPtr<ID3D12Resource>               m_buildQueryReadback;
    ComPtr<ID3D12QueryHeap>              m_pfQueryHeap;
    ComPtr<ID3D12Resource>               m_pfQueryReadback;
    static const UINT                    kBuildTimestampCount    = 8;
    // 5 op pairs per frame: INSTANTIATE / BLAS-from-CLAS (animated) /
    // TLAS rebuild / static-BLAS rebuild / static-CLAS rebuild.  The
    // last two are unused when m_staticRebuildMode == None (timestamps
    // still recorded, just delta = ~0).  Splitting INSTANTIATE out from
    // BLAS lets the overlay attribute time changes to vertex format and
    // precision (which only affect INSTANTIATE).
    static const UINT                    kPerFrameTsPerSlot      = 10;
    static const UINT                    kPerFrameRingSlots      = 3;
    UINT64                               m_timestampFrequency = 0;
    double                               m_clasBuildMs        = 0.0;
    double                               m_blasBuildMs        = 0.0;
    double                               m_tlasBuildMs        = 0.0;
    double                               m_totalBuildMs       = 0.0;
    // Per-frame (EMA-smoothed) - split out so the precision / vertex-format
    // sweep can attribute time changes to the right op.
    double                               m_pfInstantiateMs    = 0.0;  // INSTANTIATE_CLUSTER_TEMPLATES alone
    double                               m_pfBlasRebuildMs    = 0.0;  // BUILD_BLAS_FROM_CLAS alone (animated)
    double                               m_pfTlasRebuildMs    = 0.0;  // TLAS rebuild
    double                               m_pfStaticBlasMs     = 0.0;  // [R] mode 1+2: re-run static BLAS
    double                               m_pfStaticClasMs     = 0.0;  // [R] mode 2: re-run static CLAS
    UINT                                 m_pfWriteSlot        = 0;
    UINT                                 m_pfFramesCaptured   = 0;
    UINT64                               m_totalClasBytes     = 0;
    UINT64                               m_totalBlasBytes     = 0;

    // ---------- Overlay stats snapshot ----------
    // To minimize per-frame overhead the overlay does NOT read live numbers
    // each frame -- everything except FPS is snapshotted into this struct on
    // a config change (RebuildStaticAccelerationStructures + BuildAnimatedObjectSetup)
    // and read from here every frame.  Per-frame timing values are snapped
    // a few frames LATER (once the ring buffer has refilled with post-rebuild
    // samples) so the displayed ms reflects the new config, not the old one.
    struct OverlayStats
    {
        // Static path
        UINT64 staticClasAllocBytes      = 0;
        UINT64 staticClasActualBytes     = 0;          // sumActualBytes from rebuild
        UINT64 staticClasScratchBytes    = 0;
        UINT64 staticBlasTotalBytes      = 0;
        UINT64 staticBlasScratchBytes    = 0;    // BLAS-from-CLAS scratch (cluster path)
        UINT64 staticClusterInputBytes   = 0;    // m_clusterInputBuffer (vertex+index data) -- cluster-path analog of trad's VB+IB
        // Traditional (DXR1) static path
        UINT64 traditionalBlasTotalBytes      = 0;   // sum of per-obj worst-case prebuild sizes (== alloc)
        UINT64 traditionalBlasActualBytes     = 0;   // sum of per-obj final-storage sizes (compacted in Compact mode)
        UINT64 traditionalBlasScratchBytes    = 0;
        UINT64 traditionalVbBytes             = 0;
        UINT64 traditionalIbBytes             = 0;
        double traditionalBuildMs             = 0.0;
        // Geometry mode at snapshot time -- gates which lines the overlay
        // shows + drives delta colouring across the [T] toggle.
        int    geometryMode                   = 0;  // matches GeometryMode enum order
        // Animated path
        UINT64 animatedTemplateBytes     = 0;
        UINT64 animatedTemplateScratchBytes = 0;        // template build scratch (cluster path)
        UINT64 animatedTemplateInputBytes   = 0;        // templateInputBuffer (hint verts + indices)
        UINT64 animatedRestPositionsBytes   = 0;        // restPositionsBuffer (input to AnimateBall.cs every frame, both modes)
        UINT64 animatedPerFrameClasAllocBytes  = 0;
        UINT64 animatedPerFrameClasActualBytes = 0;    // sumActual from one-shot INSTANTIATE size readback
        UINT64 animatedPerFrameClasScratchBytes = 0;
        UINT64 animatedBlasBytes         = 0;
        UINT64 animatedBlasScratchBytes  = 0;           // BLAS-from-CLAS scratch (cluster path animated)
        // Traditional-mode animated BLAS (DXR1 per-frame rebuild/refit).
        // Populated when geometryMode==Traditional; zero otherwise.  The
        // resident-memory value is the BLAS storage (tradBlasResultBytes);
        // tradScratch is the scratch buffer the per-frame build reuses.
        UINT64 animatedTradBlasBytes     = 0;
        UINT64 animatedTradScratchBytes  = 0;
        UINT64 animatedTradIbBytes       = 0;    // flat IB (cluster-major) -- input data, not BVH
        // Phase-2 anim clones: per-clone BLAS POOL storage (each anim clone
        // has its own per-frame-rebuilt BLAS).  Tracked separately so the
        // ANIMATED section can show the per-source memory vs the pool
        // memory split.  Mode-exclusive: cluster mode uses
        // animClonesBlasPoolBytes (BUILD_BLAS_FROM_CLAS dest pool),
        // trad mode uses animClonesTradBlasPoolBytes (DXR1 dest pool).
        UINT64 animClonesBlasPoolBytes      = 0;
        UINT64 animClonesBlasScratchBytes   = 0;
        UINT64 animClonesTradBlasPoolBytes  = 0;
        UINT64 animClonesTradBlasScratchBytes = 0;
        UINT   animClonesPooledCount        = 0;   // how many clones got per-clone BLAS (rest share source's)
        // Last selected trad-mode anim update strategy (0=rebuild, 1=refit).
        // Snapshotted so the overlay's "[F]" line tracks the live state.
        int    animatedTradModeIsRefit   = 0;
        // TLAS
        UINT64 tlasBytes                 = 0;
        // Per-frame timing (snapped a few frames after the rebuild completes,
        // once the timestamp ring buffer has refilled with post-rebuild samples).
        // Split out so users can see precision/format affecting INSTANTIATE
        // specifically, with BLAS / TLAS reported alongside as the constant
        // baseline.
        double pfInstantiateMs           = 0.0;
        double pfBlasRebuildMs           = 0.0;
        double pfTlasRebuildMs           = 0.0;
        double pfStaticBlasMs            = 0.0;  // [R] per-frame static BLAS
        double pfStaticClasMs            = 0.0;  // [R] per-frame static CLAS
        bool   pfTimingValid             = false;
        // Cluster + tri counts (only change when geometry changes; updated at rebuild).
        UINT   totalClusterCount         = 0;
        UINT   totalTriangleCount        = 0;
    };
    OverlayStats m_overlayStats;
    // Previous-snapshot copy so the overlay can colour each number red/green
    // when a config toggle changes it (red = went up = "worse" for memory/time,
    // green = went down).  Populated by CaptureOverlayStatsSnapshot before it
    // overwrites m_overlayStats.  m_overlayStatsHasPrev gates the colouring
    // off for the very first capture (no previous to compare against).
    OverlayStats m_overlayStatsPrev;
    bool         m_overlayStatsHasPrev = false;
    // Per-mode "last snapshot taken while in this mode".  These exist so
    // mode-specific stats (cluster-only CLAS bytes, trad-only BLAS bytes,
    // etc.) get an intra-mode delta on a [T] cross-mode toggle instead of
    // a garbage delta against the other mode's stale value.  Shared
    // stats (TOTAL AS memory, TLAS, per-frame timing) deliberately keep
    // using m_overlayStatsPrev so a [T] toggle paints those red/green
    // with the cross-mode comparison the user wants ("how much memory
    // does trad cost vs cluster, side-by-side") -- see deltaColourMatch
    // / deltaColourCross usage in the overlay renderer.
    OverlayStats m_overlayStatsLastInCluster;
    OverlayStats m_overlayStatsLastInTrad;
    bool         m_overlayStatsHasLastInCluster = false;
    bool         m_overlayStatsHasLastInTrad    = false;
    // Delta-colour expiry timestamp.  CaptureOverlayStatsSnapshot resets this
    // to (now + 5s) so the red/green tinting auto-fades back to subtle after
    // a quiet period -- prevents the screen permanently glowing red/green
    // after a sequence of toggles.  Uses std::chrono::steady_clock so it's
    // wall-clock (animation-pause independent).
    std::chrono::steady_clock::time_point m_overlayStatsDeltaUntil =
        std::chrono::steady_clock::time_point::min();
    // Per-frame timing snapshot: rolling fixed-window mean of N raw samples,
    // continuously refreshed (~1 s at 60 FPS).  Numbers in the overlay
    // update live so users can see current GPU cost evolving; the red/green
    // delta colouring is gated separately to a 5-second window after each
    // mode toggle (see m_overlayStatsDeltaUntil + m_overlayStatsPrev) so
    // colour reflects "what changed because of the toggle", not just the
    // tiny frame-to-frame noise of the rolling refresh.
    //
    // On toggle (CaptureOverlayStatsSnapshot):
    //   - Save current overlay values to m_overlayStatsPrev (colour baseline)
    //   - Reset the accumulator and skip kPerFrameRingSlots readbacks so the
    //     first new snap is fully post-toggle data
    //
    // Each frame:
    //   - If still skipping, decrement and return
    //   - Else accumulate this frame's raw deltas
    //   - When m_pfSnapTargetCount samples have been collected, divide and
    //     write into m_overlayStats, then immediately start the next window
    //
    // m_pfSnapTargetCount used to be a compile-time constant kSnapshotSampleCount=60.
    // 60 samples is ~1 s on a 60 fps HW adapter but ~10 MINUTES on WARP
    // at the sample's ~10 s/frame rate -- the overlay's PER-FRAME line
    // would read "recalculating..." for the entire useful WARP session.
    // Now adapter-aware: resolved in OnInit from sentinel 0 to
    //   software adapter (WARP / Basic Render) -> 5 samples  (~half a min
    //                                              of WARP -- bearable)
    //   hardware adapter                        -> 60 samples (~1 s -- same as before)
    // Explicit CLI override is possible via --pf-snap-count N (added
    // for headless measurement runs that want a longer averaging window).
    INT                                  m_pfSnapTargetCount      = 0;   // 0 = auto, resolved in OnInit
    INT                                  m_pfSnapSkipFramesLeft   = 0;
    INT                                  m_pfSnapSamplesCollected = 0;
    struct PfSnapAccum {
        double instMs       = 0.0;
        double blasMs       = 0.0;
        double tlasMs       = 0.0;
        double staticBlasMs = 0.0;
        double staticClasMs = 0.0;
    };
    PfSnapAccum                          m_pfSnapAccum;
    // True from the moment a config-change toggle fires (CaptureOverlayStatsSnapshot)
    // until the first post-toggle per-frame snap window completes (~kPerFrameRingSlots
    // + kSnapshotSampleCount frames = ~1.1 s at 60 FPS).  The overlay uses this to
    // print "recalculating..." in place of the per-frame timing numbers during the
    // settle, so the user isn't staring at the OLD mode's millisecond figures and
    // wondering "did the toggle do anything?".  Set in CaptureOverlayStatsSnapshot
    // (skipped on init), cleared in the snap-window-completes branch.
    bool                                 m_pfTimingSettlingAfterToggle = false;

    // ---------- App state ----------
    StepTimer m_timer;

    // Wall-clock per-frame timing for the overlay's "FPS / ms-per-frame"
    // line.  StepTimer caps GetElapsedSeconds() at 100 ms (m_qpcMaxDelta
    // = frequency/10) to keep animation steps sane after debugger pauses
    // -- great for game-logic update, useless for "how slow is WARP
    // really?".  On a 3 s/frame WARP run the StepTimer-derived value
    // would lock at 100 ms while the FPS counter (uses the UNCLAMPED
    // delta into its 1-second sliding window) honestly reports ~0 fps;
    // the two disagree by ~30x and the user can't tell what's true.
    //
    // Replaced with a TRUE rolling-window average (was EMA initially --
    // EMA at alpha=0.2 still jittered noticeably on a 240 fps HW run
    // since one fat frame skews the value for ~5 frames).  Rolling
    // average over m_frameTimeWindow samples gives a clean, predictable
    // display value:
    //   - HW: 60 samples = ~0.5-1 s window at 60-120 fps -- smooths
    //     out vsync jitter and short hitches; matches user's intuitive
    //     "FPS" reading.
    //   - WARP: 3 samples -- frames are seconds long, a 60-sample
    //     window would lag behind state changes by MINUTES.  3 is the
    //     smallest count that still hides single-frame outliers.
    // Window size is adapter-resolved in OnInit (sentinel 0 = auto).
    //
    // Implementation: circular buffer of frame-time samples (in
    // seconds) + running sum.  m_frameTimeRingIdx is the slot we'll
    // overwrite next; m_frameTimeRingCount is how many valid entries
    // we've written so far (saturates at m_frameTimeWindow).
    std::chrono::steady_clock::time_point m_lastFrameWallTime{};
    UINT                                 m_frameTimeWindow    = 0;        // 0 = auto-resolve in OnInit
    std::vector<double>                  m_frameTimeRing;                  // sized to m_frameTimeWindow
    UINT                                 m_frameTimeRingIdx   = 0;
    UINT                                 m_frameTimeRingCount = 0;
    double                               m_frameTimeRingSum   = 0.0;       // sum of valid entries
    double    m_animSeconds       = 0.0;          // wall-clock anim time (drives AnimateBall.cs); freezable with [M]
    bool      m_animPaused        = false;
    // Camera-pan time + pause state, decoupled from animation time so a bench
    // run can lock the camera at a fixed viewpoint (eliminating per-frame
    // glass/refraction overlap variance) while still measuring per-frame AS
    // rebuild costs (animated CLAS / BLAS).  Both times tick at the same wall-
    // clock rate when both are unpaused, so a fresh run with neither paused
    // matches the historical (single m_animSeconds) behaviour.  [Space] toggles
    // camera; [M] toggles animation.  --camera-paused / --anim-paused CLI flags
    // start either or both frozen.
    double    m_cameraSeconds     = 0.0;
    bool      m_cameraPaused      = false;

    // Screenshot capture (--screenshot N path.png).
    int          m_screenshotFrame     = -1;
    double       m_screenshotAtSeconds = -1.0;
    std::wstring m_screenshotPath;
    UINT         m_framesRendered = 0;
    bool         m_screenshotTaken = false;

    // ---------- Initialization helpers ----------
    void CreateDeviceDependentResources();
    void QueryDXR2Support();
    void BuildScene();                              // populates m_objects (CPU-side meshes + encoding)
    void BuildAccelerationStructures();
    // Encodes the per-cluster Compressed1 blobs (positions -> obj.encoded for
    // every static object).  Called once from BuildScene and again from
    // RebuildStaticAccelerationStructures whenever the user cycles
    // m_compressedBitsPerComponent via the '[' / ']' keys in COMPRESSED1 mode.
    void EncodeCompressedClusters();
    // Tear down CLAS/BLAS/TLAS state and rebuild with the current
    // m_vertexMode + m_clasAllocMode.  When enabled, the animated object's
    // templates and per-frame CLAS/BLAS resources are recreated too, so [V]
    // and [ ] apply one format/precision choice consistently across the scene.
    void RebuildStaticAccelerationStructures(const wchar_t* reason);
    // Regenerate the [N] workload-scaling clones: truncates m_objects to
    // m_sourceObjectCount, then appends ExtraInstancesCount() deep-copied
    // clones cycled through the source pool with spiral transforms and
    // random material assignments.  Recomputes m_totalClusterCount +
    // per-object globalClusterStart for the new total.  Called from
    // RebuildStaticAccelerationStructures BEFORE the per-object reset
    // loop + build pipeline so clones flow through the same code as
    // sources and end up with their own CLAS arrays + BLASes.
    void RegenerateWorkloadCloneInstances();
    // Lazily populate m_cloneSourceLodMeshes -- pre-generates a chain
    // of progressively-lower-tessellation meshes per cloneable source.
    // Idempotent: returns immediately if the cache is non-empty.
    // Called from RegenerateWorkloadCloneInstances when LOD is active.
    void EnsureCloneSourceLodMeshes();
    void UploadClusterInputs();
    void BuildClasIndirect();              // dispatches to one of the three below
    void BuildClasImplicit();              // ClasAllocMode::Implicit
    void BuildClasGetSizes();              // ClasAllocMode::GetSizes
    void BuildClasCompact();               // ClasAllocMode::Compact
    void BuildBlasFromClasIndirect();
    // Per-object traditional (DXR1) BLAS build for static objects.  Called
    // instead of BuildBlasFromClasIndirect when m_geometryMode ==
    // Traditional.  Concatenates each object's per-cluster vertex+index
    // data into single buffers, then issues a classic
    // BuildRaytracingAccelerationStructure for each object.
    // (Cluster path's m_clasArgsBuffer / m_clasMoveArgsBuffer / etc. are
    // simply not allocated in Traditional mode.)
    void BuildTraditionalStaticAS();
    void BuildTlasClassic();
    void RebuildTlasPerFrame();
    // Per-frame static-AS rebuild paths (driven by m_staticRebuildMode = [R]).
    // Both re-issue their RTAS op against the existing static buffers (no
    // allocations).  Caller emits the surrounding EndQuery timestamps.
    void RebuildStaticBlasPerFrame();
    void RebuildStaticClasPerFrame();
    void BuildAnimatedObjectSetup();          // generates mesh, builds templates ONCE
    // Phase 2 anim-clones setup.  Called after BuildAnimatedObjectSetup
    // whenever m_animatedClones is non-empty.  Allocates the per-clone
    // BLAS pool and re-builds the source's per-frame BLAS args + dest
    // arrays so the existing per-frame BUILD_BLAS_FROM_CLAS op produces
    // 1 + N_animClones BLASes in one batched call (source + every
    // clone).  Each clone shares the source's per-frame CLAS results
    // (same per-cluster GVAs in obj.perFrameClasAddressArray) so this
    // stresses the BLAS-from-CLAS path per-clone without multiplying
    // INSTANTIATE work.  Clones' per-clone BLAS GVAs go into
    // AnimatedCloneInstance.blasGPUVA, which BuildTlasClassic then
    // bakes into the TLAS instance descs.
    void BuildAnimatedClonesSetup();
    // Trad-mode analog: allocate the per-anim-clone DXR1 BLAS pool +
    // shared scratch.  No-op outside trad mode or when m_animatedClones
    // is empty.  Per-clone BLAS storage = N x (source's prebuild result
    // size), so memory budget can be tight at the higher [N] tiers --
    // a 4090 swallows N=10K fine, smaller GPUs may want to stop at N=1K.
    void BuildAnimatedClonesTradSetup();
    // Traditional-mode addendum to BuildAnimatedObjectSetup.  Runs after
    // the shared mesh + perFrameVertexBuffer setup (which both modes need
    // because AnimateBall.cs writes the same per-vertex animation buffer
    // in either case).  Builds the flat per-object index buffer and
    // allocates the per-frame-rebuilt traditional BLAS storage + scratch.
    // No GPU work is issued here -- the actual BuildRaytracingAccelerationStructure
    // for the trad BLAS runs every frame in UpdateAnimatedTradPerFrame
    // (rebuild or refit per m_traditionalAnimMode).
    void BuildAnimatedTraditionalAS();
    // Per-frame trad-mode update: AnimateBall.cs writes
    // perFrameVertexBuffer (UAV) just like in cluster mode, then we
    // BuildRaytracingAccelerationStructure on tradBlasStorage with
    // either PREFER_FAST_TRACE (full rebuild) or PERFORM_UPDATE
    // (refit), driven by m_traditionalAnimMode + the [F] key.  Emits
    // per-frame timestamps at (pfTimestampBase+0..1) bracketing
    // AnimateBall.cs and (pfTimestampBase+2..3) bracketing the BLAS
    // build for the overlay, mirroring the cluster path's split.
    void UpdateAnimatedTradPerFrame(UINT pfTimestampBase = UINT_MAX);
    // Per-frame animated rebuild.  When pfTimestampBase != UINT_MAX the
    // function emits two timestamp pairs against m_pfQueryHeap at
    // (base+0,base+1) bracketing INSTANTIATE and (base+2,base+3) bracketing
    // BLAS-from-CLAS so the overlay can split out template-instantiation
    // time from BLAS rebuild time (and report each one against the current
    // vertex format / precision).  Callers that don't need the breakdown
    // (e.g. RebuildStaticAccelerationStructures, which is a one-shot init
    // path) pass UINT_MAX to skip the emissions.
    void UpdateAnimatedObjectPerFrame(UINT pfTimestampBase = UINT_MAX);
    // One-shot per-cluster CLAS size readback for the animated INSTANTIATE op.
    // Called at the end of BuildAnimatedObjectSetup (after the first INSTANTIATE
    // has run) - issues an INSTANTIATE with ResultSizeArray hooked up, copies
    // the size buffer to readback, waits, sums, writes the result into
    // m_overlayStats.animatedPerFrameClasActualBytes.  Synchronous + one-time
    // per config change - no per-frame cost.
    void MeasureAnimatedClasBytesOneShot();
    // Walk all buffers + m_clasMemStats and copy the displayed numbers into
    // m_overlayStats.  Called from RebuildStaticAccelerationStructures after
    // the static + animated paths are both finished.  Per-frame ms values are
    // NOT captured here - they're snapped a few frames later by Tick() once
    // the ring buffer has refilled with post-rebuild samples.
    void CaptureOverlayStatsSnapshot();
    // Two halves of CaptureOverlayStatsSnapshot, exposed so callers whose
    // code path mutates m_overlayStats mid-rebuild (specifically
    // RebuildStaticAccelerationStructures, via MeasureAnimatedClasBytesOneShot
    // inside BuildAnimatedObjectSetup) can stash prev BEFORE the build and
    // refresh AFTER.  Without this split, the rebuild path stashes an
    // already-half-mutated prev -> delta colouring stops working for the
    // per-frame-CLAS-actual field.  See StashOverlayStatsAsPrev definition
    // comment for the full mechanism.
    void StashOverlayStatsAsPrev();
    void RefreshOverlayStatsCurrent();
    // Headless benchmark snapshot writer.  Serialises adapter+driver+config
    // +memory+timings+FPS as a JSON object and writes it to m_benchOutPath.
    // Called once, just before PostQuitMessage when m_benchSeconds has
    // elapsed (or from --at <frame>:bench).  All numeric fields use
    // canonical units (bytes for memory, microseconds for sub-ms timings,
    // milliseconds for build-time totals, FPS as 1.0/secPerFrame).  Uses
    // m_overlayStats as the source of truth so the JSON reports the same
    // numbers the overlay would show -- no separate measurement path
    // to drift out of sync.  Schema version field gates compatibility for
    // the cross-machine comparison report.
    void WriteBenchmarkSnapshot();
    void DumpClusterStatsAsync();
    void ReadBuildTimestamps();
    void CreateUIFont();
    void RenderUI();
    void CreateRaytracingPipelineAndShaderTables();
    // Compile-once-at-init compute pipeline used by the per-frame GPU ball
    // deformation pass (see UpdateAnimatedObjectPerFrame).
    void CreateAnimationComputePipeline();
    void CreateFillInstantiateArgsPipeline();
    void CreateFillMoveArgsPipeline();
    void CreateFillBlasArgsPipeline();
    void CreateFillClasTriArgsPipeline();
    void CreateFillTemplateArgsPipeline();
    void CreateDescriptorHeapAndRaytracingOutput();
    void UpdateSceneConstantBuffer();

    UINT AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* outCpu);
    void DoRender();
    void CaptureBackBufferToFile(const std::wstring& path);
    static HRESULT SaveBGRAToPng(const std::wstring& path, UINT width, UINT height,
                                 const uint8_t* data, UINT rowPitchBytes);

    // Shader entry-point names (must match Raytracing.hlsl).
    // Shader entry-point names (must match Raytracing.hlsl). Two ray indices:
    //   0 = primary ray  (Miss + ClusterHitGroup)
    //   1 = shadow ray   (ShadowMiss + ShadowHitGroup -- the shadow hit group's
    //                     closesthit is never invoked because shadow rays use
    //                     SKIP_CLOSEST_HIT_SHADER, but DXR still requires a
    //                     hit-group record per ray-contribution index)
    static const wchar_t* c_raygenName;
    static const wchar_t* c_opaqueClosestHitName;
    static const wchar_t* c_glassClosestHitName;
    static const wchar_t* c_glassAnyHitName;
    static const wchar_t* c_missName;
    static const wchar_t* c_shadowMissName;
    static const wchar_t* c_opaqueHitGroupName;
    static const wchar_t* c_glassHitGroupName;
    static const wchar_t* c_shadowHitGroupName;
};
