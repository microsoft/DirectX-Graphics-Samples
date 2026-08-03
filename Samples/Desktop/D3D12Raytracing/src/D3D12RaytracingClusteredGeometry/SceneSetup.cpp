//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
//
//*********************************************************

// ============================================================================
// SceneSetup.cpp
//
// Demo-specific scene definition, factored out of the main
// D3D12RaytracingClusteredGeometry.cpp so it can focus on CORE acceleration-
// structure management + rendering.  Engineers wiring DXR2 into their own
// renderer typically have their own scene system; this file shows the path
// from a scene description (SceneData.h) to populated ClusterObject instances
// that the AS pipeline consumes.
//
// Contains:
//   * BuildScene     -- iterates SceneData entries, runs the per-shape
//     procedural generator, fills the ClusterObject with mesh + per-cluster
//     bounds + per-region material lookup tables.  Also handles the mixed-
//     material sphere (instance 3) split into two regions.
//   * BuildMaterials -- copies the static MaterialData::kMaterials table
//     into the per-frame upload buffer the shader binds at t6.
//
// All methods here are still members of D3D12RaytracingClusteredGeometry.
// ============================================================================

#include "stdafx.h"
#include "D3D12RaytracingClusteredGeometry.h"
#include "SceneData.h"
#include "MaterialData.h"
#include "DirectXRaytracingHelper.h"

using namespace std;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

// ---- BuildScene -------------------------------------------
// =====================================================================================
// Scene construction
//
// Multiple procedurally-generated objects, each becoming one Cluster BLAS:
//   - 4 spheres of varying subdivisions (different cluster counts per BLAS)
//   - 1 torus
//   - 1 subdivided cube
// All clusters use **spatial-tile** decomposition - small contiguous patches
// of the surface (meshlet-style), not long latitude bands.
// =====================================================================================
void D3D12RaytracingClusteredGeometry::BuildScene()
{
    // (COMPRESSED1 precision used to be a constexpr 12 here; promoted to the
    // m_compressedBitsPerComponent member so the '[' / ']' keys can cycle it
    // live in COMPRESSED1 mode.  EncodeCompressedClusters() reads the live
    // member, so re-calling it after a slider change re-encodes everything.)
    // GENERIC scene-build pass.  Iterates over the pure-data scene
    // definition (SceneData::BuildSceneDefinition()) and dispatches to
    // the right ProceduralGeometry generator per ObjectSpec.  All art /
    // material / placement / per-cluster-checker config lives in
    // SceneData.cpp - this function has zero hardcoded geometry numbers
    // and zero per-object branches outside the GenKind switch.
    using SD = SceneData::ObjectSpec;
    const auto scene = SceneData::BuildSceneDefinition();

    auto genMesh = [](const SD& s) -> ProceduralGeometry::Mesh
    {
        switch (s.kind)
        {
        case SceneData::GenKind::UVSphere:
            return ProceduralGeometry::GenerateUVSphereSpatialTiles(
                s.args.sphereRadius, s.args.sphereNumLat, s.args.sphereNumLong,
                s.args.sphereTileLat, s.args.sphereTileLong, s.firstClusterID);
        case SceneData::GenKind::Torus:
            return ProceduralGeometry::GenerateTorusSpatialTiles(
                s.args.torusMajor, s.args.torusMinor,
                s.args.torusRingSegs, s.args.torusSideSegs,
                s.args.torusTileRing, s.args.torusTileSide, s.firstClusterID);
        case SceneData::GenKind::Cube:
            return ProceduralGeometry::GenerateCubeSpatialTiles(
                s.args.cubeHalfExtent, s.args.cubeFaceSubdiv,
                s.args.cubeTileSize, s.firstClusterID);
        case SceneData::GenKind::Slab:
            return ProceduralGeometry::GeneratePlaneSpatialTiles(
                s.args.slabHalfSizeU, s.args.slabHalfSizeV,
                s.args.slabTilesU, s.args.slabTilesV,
                s.args.slabTileQuadsU, s.args.slabTileQuadsV,
                s.firstClusterID, s.args.slabThickness);
        case SceneData::GenKind::Klein:
            return ProceduralGeometry::GenerateKleinBottleSpatialTiles(
                s.args.kleinScale, s.args.kleinNumU, s.args.kleinNumV,
                s.args.kleinTileUSize, s.args.kleinTileVSize, s.firstClusterID);
        }
        // Unreachable - all enum cases handled above.
        return ProceduralGeometry::Mesh{};
    };

    for (const SD& s : scene)
    {
        ClusterObject obj;
        obj.mesh          = genMesh(s);
        obj.worldPos      = s.pos;
        obj.worldScale    = s.scale;
        obj.worldRotEuler = s.rotEuler;
        obj.instanceID    = s.instanceID;
        obj.checker       = s.checker;
        obj.surfTintMul   = s.surfTintMul;
        obj.refrTintMul   = s.refrTintMul;
        obj.reflTintMul   = s.reflTintMul;
        obj.nonOrientable = s.nonOrientable;
        m_objects.push_back(std::move(obj));
    }

    // -------------------------------------------------------------
    // Mixed-material demo: split the smallest sphere (sphere3, was
    // amethyst glass) into chrome upper hemisphere + amethyst glass
    // lower hemisphere.  Per-cluster matRegionIdx assignment drives
    //   - the cluster path's CLAS BaseGeometryIndex stamp (so
    //     GeometryIndex() at hit time returns the region),
    //   - the traditional path's per-region geom-desc layout (one
    //     geom desc per region, NOT per cluster),
    //   - per-(InstIdx, GeomIdx) material lookup
    //     (chrome for region 0, amethyst glass for region 1),
    //   - a mixed-instance shader-table block selected at TLAS build time.
    //     Both regions use GlassHit; with refractivity 0 the chrome region
    //     skips refraction while the glass region executes the full path.
    // Region split is by cluster centroid Y in object space -- the
    // cluster's tile boundaries already align with parametric latitude
    // rings on the UV sphere, so the equator is a clean cluster seam
    // (no partial-cluster splits).
    for (auto& obj : m_objects)
    {
        if (obj.instanceID != 3) continue;  // only sphere3 (amethyst -> mixed)

        for (auto& cl : obj.mesh.clusters)
        {
            float centroidY = 0.0f;
            for (const auto& p : cl.positions) centroidY += p.y;
            centroidY /= (float)cl.positions.size();
            cl.matRegionIdx = (centroidY >= 0.0f) ? 0u : 1u;   // 0 = upper (chrome), 1 = lower (glass)
        }
        // Region 0 = chrome (material slot 0, same as sphere0's body).
        // Region 1 = amethyst glass (material slot 3, sphere3's original).
        obj.perRegionMaterialSlot = { 0u, 3u };
        SampleLog::LogF(L"[mixed sphere] obj instanceID=%u split into 2 regions "
                        L"(upper hemisphere -> material slot %u, lower -> %u)\n",
                        obj.instanceID,
                        obj.perRegionMaterialSlot[0],
                        obj.perRegionMaterialSlot[1]);
        break;
    }

    // Determine per-cluster offsets in the global cluster array (used by the
    // BLAS-from-CLAS builds to slice the global CLAS-address array per-object).
    UINT runningOffset = 0;
    for (auto& obj : m_objects)
    {
        obj.globalClusterStart = runningOffset;
        obj.clusterCount       = (UINT)obj.mesh.clusters.size();
        runningOffset         += obj.clusterCount;
    }
    m_totalClusterCount = runningOffset;
    // BuildAnimatedObjectSetup creates local animated IDs starting at zero.
    // RegenerateWorkloadCloneInstances will move this base after any clone IDs.
    UINT maxStaticClusterId = 0;
    bool anyStaticClusters = false;
    for (const auto& obj : m_objects)
    for (const auto& cluster : obj.mesh.clusters)
    {
        maxStaticClusterId = std::max(maxStaticClusterId, cluster.clusterID);
        anyStaticClusters = true;
    }
    m_animatedClusterIdOffset = anyStaticClusters ? maxStaticClusterId + 1u : 0u;
    m_totalTriangleCount = 0;
    for (const auto& obj : m_objects)
        m_totalTriangleCount += obj.mesh.totalTriangles;
    // m_animatedObject.mesh.totalTriangles is added later (after animated
    // mesh is generated in BuildAnimatedObjectSetup).

    // Capture how many m_objects are SOURCES (not clones).  Any future
    // RebuildStaticAccelerationStructures call will truncate m_objects
    // back to this count before regenerating clones for the current
    // [N] workload-scaling mode.
    m_sourceObjectCount = (UINT)m_objects.size();

    SampleLog::LogF(L"\n[scene] %zu objects, %u total clusters\n",
                    m_objects.size(), m_totalClusterCount);
    UINT objIdx = 0;
    for (const auto& obj : m_objects)
    {
        SampleLog::LogF(L"  object[%u]: %u clusters, %u tris, %u verts at (%.2f,%.2f,%.2f) scale %.2f\n",
                        objIdx++, obj.clusterCount, obj.mesh.totalTriangles, obj.mesh.totalVertices,
                        obj.worldPos.x, obj.worldPos.y, obj.worldPos.z, obj.worldScale);
    }

    SampleLog::LogF(L"\n[vertex format] %s\n",
                    m_vertexMode == VertexMode::Compressed1 ? L"COMPRESSED1 (shared-exponent quantized)"
                                                            : L"FLOAT32_3 (no quantization)");

    // COMPRESSED1 is encoded eagerly so the runtime format toggle can rebuild
    // without reconstructing the scene.  The exact per-cluster vertex bound is
    // validated on the current experimental WARP.
    EncodeCompressedClusters();
    // FLOAT32_3 path uses obj.mesh.clusters[i].positions directly at upload
    // time; obj.rawPositions is unused and intentionally left empty.  Log
    // the equivalent byte count so the user can compare paths at a glance.
    {
        size_t totalBytes = 0;
        for (const auto& obj : m_objects)
            for (const auto& c : obj.mesh.clusters)
                totalBytes += c.positions.size() * sizeof(ProceduralGeometry::float3);
        SampleLog::LogF(L"[float32_3] %u clusters: %zu bytes total\n",
                        m_totalClusterCount, totalBytes);
    }
}


// ---- BuildMaterials -------------------------------------------
// ---------------------------------------------------------------------------------
// Concatenate per-cluster vertex blob + index buffer + per-cluster build args
// into a single upload buffer. Layout (clusters in object order, all flattened):
// =====================================================================================
// Per-instance material assignment + upload to a structured buffer.
//
// 9 slots, indexed by InstanceID() in HLSL.  Every object in this scene
// has reflectivity > 0 (the user wanted everything mirror-tinged); a
// subset additionally has translucency or refractivity to demo any-hit
// stochastic transparency / Snell-law refraction.  Stacking is allowed -
// e.g., the Klein bottle is BOTH partly reflective AND frosted.
//
//   0  large sphere       diffuse + soft 20% reflection                (matte plastic)
//   1  medium sphere      85% mirror reflection                        (chrome)
//   2  small sphere       40% reflection + 50% stochastic translucency (frosted mirror)
//   3  smallest sphere    35% reflection                               (warm metal)
//   4  torus              25% reflection                               (brushed brass)
//   5  cube               60% reflection                               (polished steel)
//   6  floor              15% reflection                               (wet stone)
//   7  ANIMATED sphere    20% reflection + 70% refraction (IOR 1.5)    (animated glass)
//   8  Klein bottle       30% reflection + 55% stochastic translucency (frosted purple)
//
// Per-instance flags are derived from the floats:
//   translucency==0 && refractivity==0  -> FORCE_OPAQUE (any-hit skipped)
//   otherwise                            -> NO opaque flag (any-hit fires)
// (A 100% reflective surface is still fully OPAQUE to ray traversal -
//  reflection is composed in closesthit, not in any-hit.)
// =====================================================================================
void D3D12RaytracingClusteredGeometry::BuildMaterials()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Pure copy from the data table.  The actual MaterialDesc values
    // live in MaterialData.cpp - this engine code never touches the
    // baseColor / refl / refr / ior numbers directly.
    std::copy(MaterialData::kMaterials.begin(),
              MaterialData::kMaterials.end(),
              m_materials.begin());

    AllocateUploadBuffer(device, m_materials.data(),
                         m_materials.size() * sizeof(MaterialDesc),
                         &m_materialsBuffer, L"Per-instance materials");

    SampleLog::LogF(L"[materials] %u slots loaded from MaterialData::kMaterials\n",
                    (unsigned)m_materials.size());
}
