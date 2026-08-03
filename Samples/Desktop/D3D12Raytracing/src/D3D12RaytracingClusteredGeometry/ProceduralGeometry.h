//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT License (MIT).
//
//*********************************************************
//
// ProceduralGeometry.h
//
// Procedural mesh generators that decompose into "clusters" suitable for the
// DXR2 cluster build path. The decomposition strategy is **spatial tiling**:
// each cluster covers a small contiguous patch of the surface in UV space. This
// matches how engines (Nanite, NVIDIA's animated-clusters sample, etc.)
// decompose meshes for cluster acceleration structures - small spatially
// localized chunks rather than long thin strips.
//
// All meshes are emitted as a vector of self-contained clusters: each cluster
// owns its own vertex buffer (8-bit local indices into a per-cluster VB), so
// the encoder can encode each cluster independently and the cluster build args
// reference per-cluster VertexBuffer + IndexBuffer GPU VAs directly.
//
// Per-cluster vertex/triangle counts stay well within DXR2's 256/256 limit
// for the parameter ranges this header is used at.
//
#pragma once

#include <vector>
#include <cstdint>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ProceduralGeometry
{
    struct float3 { float x, y, z; };

    // CLUSTER_FLAG_INTERIOR_SURFACE: this cluster is a face of an enclosed
    // glass volume (e.g. the slab's bottom face seen from inside, or one
    // of the slab's side walls).  The shader uses this to apply a back-
    // face refractivity boost so the interior surface reads as visible
    // instead of washing out the camera ray with transmitted exterior.
    static constexpr unsigned int CLUSTER_FLAG_INTERIOR_SURFACE = 0x1u;

    struct Cluster
    {
        unsigned int            clusterID;     // exposed via HLSL ClusterID() in hits
        std::vector<float3>     positions;     // local positions (mesh-local space)
        std::vector<float3>     normals;       // parallel to positions; analytic per-vertex
                                               // surface normals.  Travels through a SEPARATE
                                               // structured-buffer side channel because
                                               // DXR2 cluster geometry's CLAS only takes
                                               // positions in its vertex buffer.
        std::vector<uint8_t>    indices;       // local 8-bit indices into positions/normals

        // ------------------------------------------------------------------
        // Per-cluster GENERIC metadata, set by each generator at construction
        // time.  Lets the scene-build code drive per-cluster material /
        // colour overrides UNIFORMLY across all object types - no special-
        // casing per-instance in the closesthit shader, no hand-mirrored
        // cluster-id ranges in CPU code either.  Generic checker config
        // can be applied as `isB = (gridU + gridV) & 1` regardless of
        // whether this is a sphere lat/long tile, a slab top tile, or a
        // slab wall sub-cluster.
        // ------------------------------------------------------------------
        unsigned int            gridU = 0;         // 1st grid coord (latIdx / tu)
        unsigned int            gridV = 0;         // 2nd grid coord (longIdx / tv)
        unsigned int            matchedColorCid;   // ClusterColor() hash key.  For most
                                                   // clusters this == clusterID, but for
                                                   // slab bottom + wall sub-clusters the
                                                   // generator sets this to the matching
                                                   // TOP tile's cid so the colour layer
                                                   // is unified across the slab volume.
        unsigned int            flags = 0;         // CLUSTER_FLAG_* bits.
        // ------------------------------------------------------------------
        // Material region index within the owning OBJECT.  Lets a single
        // object split into multiple material regions: clusters with
        // matRegionIdx=0 form geometry 0 of the object's BLAS, clusters
        // with matRegionIdx=1 form geometry 1, etc.  Used by both paths:
        //   Cluster:     BUILD_CLAS_FROM_TRIANGLES_ARGS::BaseGeometryIndex
        //                gets stamped with this value per CLAS, so the
        //                BLAS-from-CLAS has geometries grouped by region
        //                and GeometryIndex() in the closest-hit returns
        //                this value at hit time.
        //   Traditional: clusters are reordered within each object so
        //                same-region clusters are contiguous in the VB+IB,
        //                and one D3D12_RAYTRACING_GEOMETRY_DESC is emitted
        //                per region (covering all that region's clusters).
        // Default 0 (single-region object) is what every procedural
        // generator emits; scene code overrides for multi-material objects
        // like the mixed-material small sphere.
        unsigned int            matRegionIdx = 0;
    };

    struct Mesh
    {
        std::vector<Cluster>    clusters;
        unsigned int            totalTriangles = 0;
        unsigned int            totalVertices  = 0;
    };

    // ------------------------------------------------------------------
    // UV sphere with **spatial-tile** cluster decomposition.
    // Generates a (numLat x numLong) quad mesh, then groups quads into
    // tiles of `tileLatSize` rows × `tileLongSize` columns, one cluster
    // per tile. Each tile owns its own vertex buffer with the
    // (tileLatSize+1) × (tileLongSize+1) vertices it needs.
    // numLat must be divisible by tileLatSize; numLong by tileLongSize.
    // ------------------------------------------------------------------
    inline Mesh GenerateUVSphereSpatialTiles(
        float radius, int numLat, int numLong,
        int tileLatSize, int tileLongSize, unsigned int firstClusterID = 0)
    {
        Mesh m;
        const int tilesLat  = numLat  / tileLatSize;
        const int tilesLong = numLong / tileLongSize;
        m.clusters.reserve((size_t)tilesLat * tilesLong);

        unsigned int clusterCounter = firstClusterID;
        for (int tLat = 0; tLat < tilesLat; ++tLat)
        for (int tLong = 0; tLong < tilesLong; ++tLong)
        {
            Cluster c;
            c.clusterID = clusterCounter++;
            c.gridU = (unsigned int)tLat;
            c.gridV = (unsigned int)tLong;
            c.matchedColorCid = c.clusterID;     // sphere clusters use their own cid for colour

            // Latitude/longitude index ranges this tile covers.
            const int latLo  = tLat  * tileLatSize;
            const int longLo = tLong * tileLongSize;

            // Vertex grid: (tileLatSize+1) rows × (tileLongSize+1) cols.
            const int rowSize = tileLongSize + 1;
            for (int li = 0; li <= tileLatSize; ++li)
            for (int lj = 0; lj <= tileLongSize; ++lj)
            {
                int latIdx  = latLo + li;
                int longIdx = longLo + lj;
                // Wrap longitude to share vertices across tile-east boundary,
                // even though we duplicate them within the cluster's local VB.
                // (Wrap matters for the global-equality story: vertices on the
                // shared edge get identical input positions and, with shared
                // exponent, identical decoded positions -> watertight.)
                if (longIdx == numLong) longIdx = 0;

                float theta = float((double)latIdx  * M_PI       / numLat);
                float phi   = float((double)longIdx * 2.0 * M_PI / numLong);
                float sinT = std::sin(theta), cosT = std::cos(theta);
                float cosP = std::cos(phi),   sinP = std::sin(phi);
                // Sphere normal at (theta, phi) is the unit radial direction;
                // position is just radius * normal so we share trig values.
                const float nx = sinT * cosP, ny = cosT, nz = sinT * sinP;
                c.positions.push_back({ radius * nx, radius * ny, radius * nz });
                c.normals.push_back  ({ nx, ny, nz });
            }

            // Two CCW (viewed from outside) triangles per quad.
            for (int li = 0; li < tileLatSize; ++li)
            for (int lj = 0; lj < tileLongSize; ++lj)
            {
                uint8_t i00 = (uint8_t)(li     * rowSize + lj    );  // top-left
                uint8_t i01 = (uint8_t)(li     * rowSize + lj + 1);  // top-right
                uint8_t i10 = (uint8_t)((li+1) * rowSize + lj    );  // bot-left
                uint8_t i11 = (uint8_t)((li+1) * rowSize + lj + 1);  // bot-right
                c.indices.push_back(i00); c.indices.push_back(i11); c.indices.push_back(i10);
                c.indices.push_back(i00); c.indices.push_back(i01); c.indices.push_back(i11);
            }

            m.totalTriangles += (unsigned int)tileLatSize * tileLongSize * 2;
            m.totalVertices  += (unsigned int)(tileLatSize + 1) * (tileLongSize + 1);
            m.clusters.push_back(std::move(c));
        }

        return m;
    }

    // ------------------------------------------------------------------
    // Torus with spatial-tile cluster decomposition.
    // ringSegs   = segments around the major (large) ring
    // sideSegs   = segments around the minor (tube cross-section) ring
    // tileRing/Side same divisibility constraints as the sphere version.
    // ------------------------------------------------------------------
    inline Mesh GenerateTorusSpatialTiles(
        float majorRadius, float minorRadius,
        int ringSegs, int sideSegs,
        int tileRing, int tileSide, unsigned int firstClusterID = 0)
    {
        Mesh m;
        const int tilesRing = ringSegs / tileRing;
        const int tilesSide = sideSegs / tileSide;
        m.clusters.reserve((size_t)tilesRing * tilesSide);

        unsigned int clusterCounter = firstClusterID;
        for (int tR = 0; tR < tilesRing; ++tR)
        for (int tS = 0; tS < tilesSide; ++tS)
        {
            Cluster c;
            c.clusterID = clusterCounter++;
            c.gridU = (unsigned int)tR;
            c.gridV = (unsigned int)tS;
            c.matchedColorCid = c.clusterID;
            const int rLo = tR * tileRing;
            const int sLo = tS * tileSide;

            const int rowSize = tileSide + 1;
            for (int ri = 0; ri <= tileRing; ++ri)
            for (int si = 0; si <= tileSide; ++si)
            {
                int rIdx = rLo + ri;
                int sIdx = sLo + si;
                if (rIdx == ringSegs) rIdx = 0;
                if (sIdx == sideSegs) sIdx = 0;

                float u = float((double)rIdx * 2.0 * M_PI / ringSegs);
                float v = float((double)sIdx * 2.0 * M_PI / sideSegs);
                float cu = std::cos(u), su = std::sin(u);
                float cv = std::cos(v), sv = std::sin(v);
                float r  = majorRadius + minorRadius * cv;
                c.positions.push_back({ r * cu, minorRadius * sv, r * su });
                // Torus normal at (u, v) = direction from tube central ring
                // out to the surface = (cos v cos u, sin v, cos v sin u).
                // Already unit length (Pythagorean identity).
                c.normals.push_back  ({ cv * cu, sv, cv * su });
            }

            for (int ri = 0; ri < tileRing; ++ri)
            for (int si = 0; si < tileSide; ++si)
            {
                uint8_t i00 = (uint8_t)(ri     * rowSize + si    );
                uint8_t i01 = (uint8_t)(ri     * rowSize + si + 1);
                uint8_t i10 = (uint8_t)((ri+1) * rowSize + si    );
                uint8_t i11 = (uint8_t)((ri+1) * rowSize + si + 1);
                c.indices.push_back(i00); c.indices.push_back(i11); c.indices.push_back(i10);
                c.indices.push_back(i00); c.indices.push_back(i01); c.indices.push_back(i11);
            }

            m.totalTriangles += (unsigned int)tileRing * tileSide * 2;
            m.totalVertices  += (unsigned int)(tileRing + 1) * (tileSide + 1);
            m.clusters.push_back(std::move(c));
        }
        return m;
    }

    // ------------------------------------------------------------------
    // Subdivided cube ("rounded box" without the rounding). Each face is
    // tessellated into (faceSubdiv x faceSubdiv) quads, each face becomes
    // (faceSubdiv/tileSize)^2 spatial-tile clusters. faceSubdiv must be
    // divisible by tileSize.
    // ------------------------------------------------------------------
    inline Mesh GenerateCubeSpatialTiles(
        float halfSize, int faceSubdiv, int tileSize, unsigned int firstClusterID = 0)
    {
        Mesh m;
        // Face basis: list of (origin, axisU, axisV) for each of the 6 faces.
        // axisU x axisV must equal the OUTWARD normal so the resulting
        // triangles have the correct CCW-from-outside winding for DXR's
        // RAY_FLAG_CULL_BACK_FACING_TRIANGLES.
        struct Face { float3 origin, axisU, axisV; };
        const float h = halfSize;
        const Face faces[6] = {
            // +X face (normal +X): U=+Y, V=+Z, U x V = +X
            {{ +h, -h, -h}, {0, 1, 0}, {0, 0, 1}},
            // -X face (normal -X): U=+Z, V=+Y, U x V = -X
            {{ -h, -h, -h}, {0, 0, 1}, {0, 1, 0}},
            // +Y face (normal +Y): U=+Z, V=+X, U x V = +Y
            {{ -h, +h, -h}, {0, 0, 1}, {1, 0, 0}},
            // -Y face (normal -Y): U=+X, V=+Z, U x V = -Y
            {{ -h, -h, -h}, {1, 0, 0}, {0, 0, 1}},
            // +Z face (normal +Z): U=+X, V=+Y, U x V = +Z
            {{ -h, -h, +h}, {1, 0, 0}, {0, 1, 0}},
            // -Z face (normal -Z): U=+Y, V=+X, U x V = -Z
            {{ -h, -h, -h}, {0, 1, 0}, {1, 0, 0}},
        };

        const int tilesPerSide = faceSubdiv / tileSize;
        m.clusters.reserve((size_t)6 * tilesPerSide * tilesPerSide);

        unsigned int clusterCounter = firstClusterID;
        for (int f = 0; f < 6; ++f)
        for (int tu = 0; tu < tilesPerSide; ++tu)
        for (int tv = 0; tv < tilesPerSide; ++tv)
        {
            Cluster c;
            c.clusterID = clusterCounter++;
            c.gridU = (unsigned int)tu;
            c.gridV = (unsigned int)tv;
            c.matchedColorCid = c.clusterID;
            const Face& face = faces[f];
            // Per-face flat normal = axisU x axisV (outward by construction
            // of the face table).  All vertices on this cluster share it -
            // cubes don't smooth across edges.
            const float3 nFace = {
                face.axisU.y * face.axisV.z - face.axisU.z * face.axisV.y,
                face.axisU.z * face.axisV.x - face.axisU.x * face.axisV.z,
                face.axisU.x * face.axisV.y - face.axisU.y * face.axisV.x };
            const float du = (2 * h) / faceSubdiv;
            const int rowSize = tileSize + 1;
            for (int li = 0; li <= tileSize; ++li)
            for (int lj = 0; lj <= tileSize; ++lj)
            {
                float u = du * (tu * tileSize + lj);
                float v = du * (tv * tileSize + li);
                c.positions.push_back({
                    face.origin.x + face.axisU.x * u + face.axisV.x * v,
                    face.origin.y + face.axisU.y * u + face.axisV.y * v,
                    face.origin.z + face.axisU.z * u + face.axisV.z * v });
                c.normals.push_back(nFace);
            }
            for (int li = 0; li < tileSize; ++li)
            for (int lj = 0; lj < tileSize; ++lj)
            {
                uint8_t i00 = (uint8_t)(li     * rowSize + lj    );
                uint8_t i01 = (uint8_t)(li     * rowSize + lj + 1);
                uint8_t i10 = (uint8_t)((li+1) * rowSize + lj    );
                uint8_t i11 = (uint8_t)((li+1) * rowSize + lj + 1);
                c.indices.push_back(i00); c.indices.push_back(i11); c.indices.push_back(i10);
                c.indices.push_back(i00); c.indices.push_back(i01); c.indices.push_back(i11);
            }
            m.totalTriangles += (unsigned int)tileSize * tileSize * 2;
            m.totalVertices  += (unsigned int)(tileSize + 1) * (tileSize + 1);
            m.clusters.push_back(std::move(c));
        }
        return m;
    }

    // ------------------------------------------------------------------
    // Flat XZ-plane built as a (tilesU x tilesV) grid of cluster tiles,
    // each cluster being (tileQuadsU x tileQuadsV) quads in the plane's
    // local frame (centred at origin, +Y normal). The N-cluster floor
    // demonstrates that cluster geometry doesn't have to be curved -
    // a flat ground plane benefits from CLAS just as much (still gets
    // per-cluster opacity flags, BLAS-from-CLAS aggregation, and the
    // memory savings of compressed1 / position-truncate).
    //
    // Output verts per cluster = (tileQuadsU+1) * (tileQuadsV+1); keep
    // the product <= 256 so 8-bit indices remain valid.
    // ------------------------------------------------------------------
    inline Mesh GeneratePlaneSpatialTiles(
        float halfSizeU, float halfSizeV,
        int tilesU, int tilesV,
        int tileQuadsU, int tileQuadsV,
        unsigned int firstClusterID = 0,
        float thickness = 0.0f)              // > 0 -> generate a closed SLAB (top + bottom + 4 side walls)
    {
        Mesh m;
        // Slab mode: top + bottom (each tilesU*tilesV clusters) + 4 side
        // walls (one cluster each).  Closed volume so refraction has well-
        // defined enter / exit faces from any incoming direction.
        const bool slab = (thickness > 0.0f);
        m.clusters.reserve((size_t)tilesU * tilesV * (slab ? 2u : 1u) + (slab ? 4u : 0u));
        const int rowSize = tileQuadsU + 1;
        const float tileWidthU = 2.0f * halfSizeU / (float)tilesU;
        const float tileWidthV = 2.0f * halfSizeV / (float)tilesV;
        const float quadWidthU = tileWidthU / (float)tileQuadsU;
        const float quadWidthV = tileWidthV / (float)tileQuadsV;

        unsigned int clusterCounter = firstClusterID;

        // ---- TOP FACES (normal +Y, CCW from above) ---------------------
        // Top tiles are LEAD clusters of the slab volume - their cids
        // are referenced by matching bottom + wall sub-clusters' matched-
        // color-cid so the entire (tu, tv) column reads as one unit.
        for (int tu = 0; tu < tilesU; ++tu)
        for (int tv = 0; tv < tilesV; ++tv)
        {
            Cluster c;
            c.clusterID = clusterCounter++;
            c.gridU = (unsigned int)tu;
            c.gridV = (unsigned int)tv;
            c.matchedColorCid = c.clusterID;             // top tile owns its own colour
            // Slab interior surface flag: top face is hit from inside
            // when refracted rays come back UP through the slab; that
            // path uses the back-face refractivity boost.
            if (slab) c.flags |= CLUSTER_FLAG_INTERIOR_SURFACE;
            const float baseU = -halfSizeU + (float)tu * tileWidthU;
            const float baseV = -halfSizeV + (float)tv * tileWidthV;

            for (int li = 0; li <= tileQuadsU; ++li)
            for (int lj = 0; lj <= tileQuadsV; ++lj)
            {
                c.positions.push_back({
                    baseU + (float)li * quadWidthU,
                    0.0f,
                    baseV + (float)lj * quadWidthV
                });
                c.normals.push_back({ 0.0f, 1.0f, 0.0f });   // floor faces +Y
            }
            for (int li = 0; li < tileQuadsU; ++li)
            for (int lj = 0; lj < tileQuadsV; ++lj)
            {
                uint8_t i00 = (uint8_t)(li     * rowSize + lj    );
                uint8_t i01 = (uint8_t)(li     * rowSize + lj + 1);
                uint8_t i10 = (uint8_t)((li+1) * rowSize + lj    );
                uint8_t i11 = (uint8_t)((li+1) * rowSize + lj + 1);
                c.indices.push_back(i00); c.indices.push_back(i01); c.indices.push_back(i11);
                c.indices.push_back(i00); c.indices.push_back(i11); c.indices.push_back(i10);
            }
            m.totalTriangles += (unsigned int)tileQuadsU * tileQuadsV * 2;
            m.totalVertices  += (unsigned int)(tileQuadsU + 1) * (tileQuadsV + 1);
            m.clusters.push_back(std::move(c));
        }

        // ---- SLAB EXTRAS (bottom + 4 side walls) -----------------------
        if (slab)
        {
            // BOTTOM FACES: same grid as top, mirrored in Y, CW so normal = -Y.
            for (int tu = 0; tu < tilesU; ++tu)
            for (int tv = 0; tv < tilesV; ++tv)
            {
                Cluster c;
                c.clusterID = clusterCounter++;
                const float baseU = -halfSizeU + (float)tu * tileWidthU;
                const float baseV = -halfSizeV + (float)tv * tileWidthV;

                for (int li = 0; li <= tileQuadsU; ++li)
                for (int lj = 0; lj <= tileQuadsV; ++lj)
                {
                    c.positions.push_back({
                        baseU + (float)li * quadWidthU,
                        -thickness,                              // bottom of slab
                        baseV + (float)lj * quadWidthV
                    });
                    c.normals.push_back({ 0.0f, -1.0f, 0.0f });  // faces -Y
                }
                for (int li = 0; li < tileQuadsU; ++li)
                for (int lj = 0; lj < tileQuadsV; ++lj)
                {
                    uint8_t i00 = (uint8_t)(li     * rowSize + lj    );
                    uint8_t i01 = (uint8_t)(li     * rowSize + lj + 1);
                    uint8_t i10 = (uint8_t)((li+1) * rowSize + lj    );
                    uint8_t i11 = (uint8_t)((li+1) * rowSize + lj + 1);
                    // Reverse winding so the outward normal is -Y.
                    c.indices.push_back(i00); c.indices.push_back(i11); c.indices.push_back(i01);
                    c.indices.push_back(i00); c.indices.push_back(i10); c.indices.push_back(i11);
                }
                m.totalTriangles += (unsigned int)tileQuadsU * tileQuadsV * 2;
                m.totalVertices  += (unsigned int)(tileQuadsU + 1) * (tileQuadsV + 1);
                m.clusters.push_back(std::move(c));
            }

            // 4 SIDE WALLS - SPLIT into per-tile-slot sub-clusters so each
            // visible slice has its OWN cluster and therefore its OWN
            // ClusterMeta entry (matched colour + material to the adjacent
            // top tile, set by the scene-build code).  Each wall has 6
            // sub-clusters (one per tilesU or tilesV slot it abuts).
            // Cluster ordering: -X wall slots 0..tilesV-1, +X wall slots
            // 0..tilesV-1, -Z wall slots 0..tilesU-1, +Z wall slots
            // 0..tilesU-1.  Total: 2*tilesV + 2*tilesU = 24 sub-clusters
            // (for a 6x6 slab).
            // pushSideWallSlot emits ONE quad (4 verts, 2 tris) that covers
            // exactly one tile-slot's worth of the wall - matched 1:1 in
            // position with the adjacent top tile.  matchedTopCid is the
            // cid of the top tile this wall slice abuts so colour stays
            // unified across the slab volume; (gridU,gridV) is the top
            // tile's grid coord so the scene-build checker config picks
            // the SAME parity for wall+top+bottom of one column.
            const unsigned int kFirstTopCidLocal = firstClusterID;
            auto pushSideWallSlot = [&](float3 p0, float3 p1, float3 p2, float3 p3, float3 nrm,
                                        unsigned int gridU, unsigned int gridV)
            {
                Cluster c;
                c.clusterID       = clusterCounter++;
                c.gridU           = gridU;
                c.gridV           = gridV;
                c.matchedColorCid = kFirstTopCidLocal + gridU * (unsigned int)tilesV + gridV;
                c.flags          |= CLUSTER_FLAG_INTERIOR_SURFACE;
                c.positions = { p0, p1, p2, p3 };
                c.normals   = { nrm, nrm, nrm, nrm };
                c.indices   = { 0, 1, 3, 0, 3, 2 };
                m.totalTriangles += 2;
                m.totalVertices  += 4;
                m.clusters.push_back(std::move(c));
            };
            const float xMin = -halfSizeU, xMax = halfSizeU;
            const float zMin = -halfSizeV, zMax = halfSizeV;
            const float yTop = 0.0f,       yBot = -thickness;
            // -X wall (x=xMin, normal=-X).  Slots iterate tv=0..tilesV-1.
            // Matching top column: tu=0.
            for (int tv = 0; tv < tilesV; ++tv)
            {
                const float z0 = zMin + (float)tv       * tileWidthV;
                const float z1 = zMin + (float)(tv + 1) * tileWidthV;
                pushSideWallSlot({xMin, yTop, z0}, {xMin, yBot, z0},
                                 {xMin, yTop, z1}, {xMin, yBot, z1}, {-1, 0, 0},
                                 /*tu=*/0u, /*tv=*/(unsigned int)tv);
            }
            // +X wall (x=xMax, normal=+X).  Matching top column: tu=tilesU-1.
            // Reversed near/far order so the consistent index winding
            // { 0,1,3, 0,3,2 } still produces outward normal +X.
            for (int tv = 0; tv < tilesV; ++tv)
            {
                const float z0 = zMin + (float)(tv + 1) * tileWidthV;
                const float z1 = zMin + (float)tv       * tileWidthV;
                pushSideWallSlot({xMax, yTop, z0}, {xMax, yBot, z0},
                                 {xMax, yTop, z1}, {xMax, yBot, z1}, {+1, 0, 0},
                                 /*tu=*/(unsigned int)(tilesU - 1), /*tv=*/(unsigned int)tv);
            }
            // -Z wall (z=zMin, normal=-Z).  Matching top row: tv=0.
            // Reversed near/far order: x goes from xMax -> xMin per slot.
            for (int tu = 0; tu < tilesU; ++tu)
            {
                const float x0 = xMin + (float)(tu + 1) * tileWidthU;
                const float x1 = xMin + (float)tu       * tileWidthU;
                pushSideWallSlot({x0, yTop, zMin}, {x0, yBot, zMin},
                                 {x1, yTop, zMin}, {x1, yBot, zMin}, {0, 0, -1},
                                 /*tu=*/(unsigned int)tu, /*tv=*/0u);
            }
            // +Z wall (z=zMax, normal=+Z).  Matching top row: tv=tilesV-1.
            for (int tu = 0; tu < tilesU; ++tu)
            {
                const float x0 = xMin + (float)tu       * tileWidthU;
                const float x1 = xMin + (float)(tu + 1) * tileWidthU;
                pushSideWallSlot({x0, yTop, zMax}, {x0, yBot, zMax},
                                 {x1, yTop, zMax}, {x1, yBot, zMax}, {0, 0, +1},
                                 /*tu=*/(unsigned int)tu, /*tv=*/(unsigned int)(tilesV - 1));
            }
        }
        return m;
    }

    // ------------------------------------------------------------------
    // Klein bottle - "bottle" immersion in R^3.  This is the iconic shape
    // people picture when they hear "Klein bottle":  a closed surface with
    // a slim neck that curves up over the body and dives back down through
    // the body wall to reconnect with the bottom from the inside.  The
    // surface is non-orientable - if you slide an arrow along it, you can
    // return to the start with the arrow flipped, so there is no inside
    // and no outside.  In R^3 the only way to render this is to let the
    // neck self-intersect the body (an unavoidable consequence of trying
    // to embed a non-orientable closed surface in three dimensions).
    //
    // Parametric form (standard "bottle" immersion, u ∈ [0, 2π], v ∈ [0,
    // 2π], piecewise at u=π where the neck meets the body):
    //
    //   r = 4 * (1 - cos(u)/2)
    //   if u < π:
    //       x = 6*cos(u)*(1+sin(u)) + r*cos(u)*cos(v)
    //       z = -16*sin(u)          - r*sin(u)*cos(v)
    //   else:
    //       x = 6*cos(u)*(1+sin(u)) + r*cos(v + π)
    //       z = -16*sin(u)
    //   y = r*sin(v)
    //
    // Original extents (x, y, z) ≈ (±12, ±6, ±16) - we normalise by 16
    // and swap the original z (the longest axis, which is the bottle's
    // height direction) into world Y so the bottle stands upright in the
    // scene.  bottleScale then maps to roughly the bottle's half-height
    // in world units.
    //
    // The mesh is open at the u=0/u=2π seam (v→-v identification across
    // that seam isn't representable on a regular cluster-tile grid), but
    // the visual seam falls inside the body where the neck rejoins, so
    // it's effectively invisible behind the self-intersection.
    //
    // Decomposed into (tilesU x tilesV) cluster tiles - one cluster per
    // (tu, tv) patch, each with its own (tileUSize+1) x (tileVSize+1)
    // local vertex buffer.
    // ------------------------------------------------------------------
    inline Mesh GenerateKleinBottleSpatialTiles(
        float bottleScale,
        int numU, int numV,
        int tileUSize, int tileVSize,
        unsigned int firstClusterID = 0)
    {
        Mesh m;
        const int tilesU = numU / tileUSize;
        const int tilesV = numV / tileVSize;
        m.clusters.reserve((size_t)tilesU * tilesV);
        const float kPi = 3.14159265358979323846f;

        // ------------------------------------------------------------------
        // BOTTLE-WITH-HANDLE Klein-bottle silhouette, made C1-smooth at the
        // body↔handle seam via a SMOOTHSTEP BLEND.
        //
        // History: the natural "literal glass bottle with handle looping
        // through the body" silhouette comes from gluing two parametrics -
        // one for the body, one for the handle - at u = π.  Both formulas
        // produce the SAME POSITION at u=π so the surface is C0-continuous,
        // but their DERIVATIVES at u=π differ (∂z/∂u disagrees by r·cv).
        // A hard switch (`if (u < π) ... else ...`) made the central-
        // difference normal calculator straddle two unrelated slopes
        // RIGHT at the cluster boundary that sits on u=π, producing
        // garbage normals → a triangular sky-coloured "perfect-clear"
        // refraction artifact on the bottle's handle.
        //
        // Fix: keep BOTH formulas exactly as they were (so the silhouette
        // away from the seam is unchanged) but BLEND them with a
        // smoothstep weight over a narrow window u ∈ [π − δ, π + δ].
        //
        //   weight(u) = smoothstep(0, 1, (u − (π − δ)) / (2δ))
        //   point(u, v) = (1 − weight) · body(u, v) + weight · handle(u, v)
        //
        // Because body(π, v) ≡ handle(π, v), the blend has zero
        // discontinuity at the seam (the weight·(handle − body) term
        // vanishes there).  Within the window the formula is C1 (since
        // each side is C1 and smoothstep is C1).  Outside the window
        // (|u − π| > δ) the formula reduces EXACTLY to body or handle,
        // so the silhouette away from the seam is identical to the
        // pre-fix bottle.  δ = π/16 means only ≈ 6 % of the bottle's u
        // range gets blended, and the change is visually a smooth
        // transition through the body↔handle join instead of a kink.
        // ------------------------------------------------------------------
        const float kVscale       = 13.0f;     // vertical-extent knob (was 16 textbook)
        const float kThinDrop     = 7.0f;      // drop the thin (u=0/2π) join down
        const float kBlendDelta   = kPi / 16.0f;  // smoothstep window half-width

        auto bodyPoint = [kVscale, kThinDrop](float u, float v) -> float3
        {
            const float cu = std::cos(u), su = std::sin(u);
            const float cv = std::cos(v);
            const float r  = 4.0f * (1.0f - cu * 0.5f);
            const float halfCos = std::cos(u * 0.5f);
            const float thinShift = -kThinDrop * halfCos * halfCos;
            const float x = 6.0f * cu * (1.0f + su) + r * cu * cv;
            const float z = -kVscale * su - r * su * cv + thinShift;
            const float y = r * std::sin(v);
            return { x, y, z };
        };
        auto handlePoint = [kVscale, kThinDrop, kPi](float u, float v) -> float3
        {
            const float cu = std::cos(u), su = std::sin(u);
            const float r  = 4.0f * (1.0f - cu * 0.5f);
            const float halfCos = std::cos(u * 0.5f);
            const float thinShift = -kThinDrop * halfCos * halfCos;
            const float x = 6.0f * cu * (1.0f + su) + r * std::cos(v + kPi);
            const float z = -kVscale * su + thinShift;
            const float y = r * std::sin(v);
            return { x, y, z };
        };
        auto kleinPoint = [&bodyPoint, &handlePoint, kPi, kBlendDelta](float u, float v) -> float3
        {
            if (u <= kPi - kBlendDelta) return bodyPoint(u, v);
            if (u >= kPi + kBlendDelta) return handlePoint(u, v);
            // Blend window: smoothstep weight 0 -> 1 across [π-δ, π+δ].
            const float t = (u - (kPi - kBlendDelta)) / (2.0f * kBlendDelta);
            const float w = t * t * (3.0f - 2.0f * t);   // smoothstep
            const float3 b = bodyPoint(u, v);
            const float3 h = handlePoint(u, v);
            return { (1.0f - w) * b.x + w * h.x,
                     (1.0f - w) * b.y + w * h.y,
                     (1.0f - w) * b.z + w * h.z };
        };

        const float invSpan = bottleScale / kVscale;   // normalises height to ±bottleScale

        unsigned int clusterCounter = firstClusterID;
        for (int tu = 0; tu < tilesU; ++tu)
        for (int tv = 0; tv < tilesV; ++tv)
        {
            Cluster c;
            c.clusterID = clusterCounter++;
            c.gridU = (unsigned int)tu;
            c.gridV = (unsigned int)tv;
            c.matchedColorCid = c.clusterID;
            const int rowSize = tileVSize + 1;

            for (int li = 0; li <= tileUSize; ++li)
            for (int lj = 0; lj <= tileVSize; ++lj)
            {
                const int gi = tu * tileUSize + li;
                const int gj = tv * tileVSize + lj;
                const float u = (float)gi / (float)numU * 2.0f * kPi;
                const float v = (float)gj / (float)numV * 2.0f * kPi;

                float3 p = kleinPoint(u, v);

                // Central-difference partials -> normal = Pu x Pv.
                // SAFE everywhere now: kleinPoint is C1 across the seam
                // thanks to the smoothstep blend above.
                const float h = 1e-3f;
                float3 pu1 = kleinPoint(u + h, v);
                float3 pu0 = kleinPoint(u - h, v);
                float3 pv1 = kleinPoint(u, v + h);
                float3 pv0 = kleinPoint(u, v - h);
                float3 Pu = { (pu1.x - pu0.x) * 0.5f / h,
                              (pu1.y - pu0.y) * 0.5f / h,
                              (pu1.z - pu0.z) * 0.5f / h };
                float3 Pv = { (pv1.x - pv0.x) * 0.5f / h,
                              (pv1.y - pv0.y) * 0.5f / h,
                              (pv1.z - pv0.z) * 0.5f / h };
                float3 n  = { Pu.y * Pv.z - Pu.z * Pv.y,
                              Pu.z * Pv.x - Pu.x * Pv.z,
                              Pu.x * Pv.y - Pu.y * Pv.x };
                float L = std::sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
                if (L < 1e-12f) { n = { 0, 1, 0 }; L = 1; }
                n.x /= L; n.y /= L; n.z /= L;

                // Output axes:  world X = bottle's wide axis (orig x);
                //               world Y = bottle's height axis (orig z);
                //               world Z = bottle's depth axis (orig y).
                c.positions.push_back({ invSpan * p.x,
                                        invSpan * p.z,
                                        invSpan * p.y });
                c.normals.push_back  ({ n.x, n.z, n.y });
            }
            // Two CCW triangles per quad.
            for (int li = 0; li < tileUSize; ++li)
            for (int lj = 0; lj < tileVSize; ++lj)
            {
                uint8_t i00 = (uint8_t)(li     * rowSize + lj    );
                uint8_t i01 = (uint8_t)(li     * rowSize + lj + 1);
                uint8_t i10 = (uint8_t)((li+1) * rowSize + lj    );
                uint8_t i11 = (uint8_t)((li+1) * rowSize + lj + 1);
                c.indices.push_back(i00); c.indices.push_back(i11); c.indices.push_back(i10);
                c.indices.push_back(i00); c.indices.push_back(i01); c.indices.push_back(i11);
            }
            m.totalTriangles += (unsigned int)tileUSize * tileVSize * 2;
            m.totalVertices  += (unsigned int)(tileUSize + 1) * (tileVSize + 1);
            m.clusters.push_back(std::move(c));
        }
        return m;
    }
}
