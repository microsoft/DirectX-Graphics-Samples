//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT License (MIT).
//
//*********************************************************
//
// SceneData.h
//
// SCENE / ART data.  Defines the full object list of the sample:
// what to generate, where to place it, what material slot it uses,
// and what per-cluster CHECKER + TINT overrides drive its visual.
//
// PURE DATA: no engine code, no D3D12 dependencies.  The engine
// BuildScene() function reads this table, calls the right generator
// for each entry, and attaches the per-cluster scene config so the
// generic BuildClusterMetadata() pass can produce the ClusterMeta
// GPU buffer.  Adding a new object = adding a row to kScene().
//
#pragma once

#include "RaytracingHlslCompat.h"   // XMFLOAT3, NUM_MATERIAL_SLOTS
#include "SceneCommon.h"            // CheckerConfig
#include <vector>

namespace SceneData
{
    // Procedural-generator selector.  Each kind is dispatched to its
    // corresponding ProceduralGeometry:: function by the engine code.
    enum class GenKind
    {
        UVSphere,
        Torus,
        Cube,
        Slab,
        Klein,
    };

    // Generator-specific arguments.  A flat sum-of-fields rather than a
    // variant to keep the data tables readable.  Each generator reads
    // only its own slice.
    struct GenArgs
    {
        // UV sphere
        float sphereRadius   = 0.0f;
        int   sphereNumLat   = 0;
        int   sphereNumLong  = 0;
        int   sphereTileLat  = 0;
        int   sphereTileLong = 0;
        // Torus
        float torusMajor     = 0.0f;
        float torusMinor     = 0.0f;
        int   torusRingSegs  = 0;
        int   torusSideSegs  = 0;
        int   torusTileRing  = 0;
        int   torusTileSide  = 0;
        // Cube
        float cubeHalfExtent = 0.0f;
        int   cubeFaceSubdiv = 0;
        int   cubeTileSize   = 0;
        // Slab
        float slabHalfSizeU  = 0.0f;
        float slabHalfSizeV  = 0.0f;
        int   slabTilesU     = 0;
        int   slabTilesV     = 0;
        int   slabTileQuadsU = 0;
        int   slabTileQuadsV = 0;
        float slabThickness  = 0.0f;
        // Klein
        float kleinScale     = 0.0f;
        int   kleinNumU      = 0;
        int   kleinNumV      = 0;
        int   kleinTileUSize = 0;
        int   kleinTileVSize = 0;
    };

    // One scene object.  All fields the engine needs to instantiate and
    // configure an object live here.
    struct ObjectSpec
    {
        const char*       name;            // human-readable, used in logs
        GenKind           kind;
        GenArgs           args;
        unsigned int      firstClusterID;
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 rotEuler;        // radians
        float             scale;
        unsigned int      instanceID;      // also = material slot in this sample

        // Per-cluster overrides applied by parity (gridU + gridV) & 1.
        // checker.enabled=false means no override (baseline material only).
        CheckerConfig     checker;

        // Per-object tint multipliers on the global clusterTint slider.
        // Effective tint blend = clusterTint * tintMul.
        //   surfTintMul: surface base-colour cluster bias.  1.0 = default.
        //   refrTintMul: refraction-tint blend.  0.50 = default (matches
        //                old hardcoded behaviour for non-floor instances).
        //   reflTintMul: reflection-tint blend.  1.08 = default (= 0.70 /
        //                0.65 default clusterTint, matches old hardcoded
        //                behaviour for chrome / non-floor instances).
        float surfTintMul = 1.0f;
        float refrTintMul = 0.50f;
        float reflTintMul = 1.08f;

        // True for non-orientable / self-intersecting surfaces.  Drives
        // D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE on the
        // TLAS instance.  Defaults to false; set true in BuildSceneDefinition
        // for the Klein bottle.
        bool nonOrientable = false;
    };

    // Returns the full scene definition.  Function-returns-vector rather
    // than constexpr-table so we can use small helpers (hex layout, etc.)
    // at definition time without exposing them through the header.
    std::vector<ObjectSpec> BuildSceneDefinition();
}
