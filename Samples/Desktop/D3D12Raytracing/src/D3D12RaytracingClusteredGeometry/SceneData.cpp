//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT License (MIT).
//
//*********************************************************

#include "stdafx.h"
#include "SceneData.h"
#include <cmath>

namespace SceneData
{
    std::vector<ObjectSpec> BuildSceneDefinition()
    {
        using DirectX::XMFLOAT3;

        // Hex layout for the outer ring of 6 objects.  R is the radius
        // around scene origin; max object extent (R + sphere0 radius
        // 0.85) = 3.40, safely inside slab halfSize 3.5.
        constexpr float R = 2.55f;
        auto hex = [R](int i) {
            const float th = (float)i * (2.0f * 3.14159265358979323846f / 6.0f);
            return XMFLOAT3(R * std::cos(th), 0.0f, R * std::sin(th));
        };
        auto with_y = [](XMFLOAT3 p, float y) { p.y = y; return p; };

        std::vector<ObjectSpec> scene;

        // ----------------------------------------------------------------
        // SPHERES (one per hex slot 0..3).
        // ----------------------------------------------------------------
        // sphere0 - CHROME, no checker.  Lifted to y=0.40 so the bottom
        // (sphere r=0.85, world y - r = -0.45) clears the slab top at
        // y=-0.7.
        scene.push_back({
            "sphere0_chrome",
            GenKind::UVSphere,
            { .sphereRadius = 0.85f, .sphereNumLat = 64, .sphereNumLong = 128,
              .sphereTileLat = 8,    .sphereTileLong = 16 },
            /*firstClusterID*/0,
            with_y(hex(0), 0.40f), {0,0,0}, 1.0f, /*instanceID*/0,
            {}, 1.0f, 0.50f, 1.08f
        });

        // sphere1 - SHINY-MIRROR / TRANSLUCENT-GLASS checker.
        ObjectSpec s1 = {
            "sphere1_mirror_translucent_checker",
            GenKind::UVSphere,
            { .sphereRadius = 0.60f, .sphereNumLat = 48, .sphereNumLong = 96,
              .sphereTileLat = 8,    .sphereTileLong = 12 },
            100,
            with_y(hex(1), 0.4f), {0,0,0}, 1.0f, 1,
            {}, 1.0f, 0.50f, 1.08f
        };
        s1.checker.enabled = true;
        s1.checker.oddParity.overrideRefl = 0.90f;
        s1.checker.oddParity.overrideRefr = 0.0f;
        s1.checker.oddParity.overrideIor  = 0.0f;
        scene.push_back(s1);

        // sphere2 - MATTE-VIBRANT / TRANSLUCENT-GLASS checker.
        ObjectSpec s2 = {
            "sphere2_matte_translucent_checker",
            GenKind::UVSphere,
            { .sphereRadius = 0.55f, .sphereNumLat = 32, .sphereNumLong = 64,
              .sphereTileLat = 8,    .sphereTileLong = 8 },
            200,
            with_y(hex(2), -0.1f), {0,0,0}, 1.0f, 2,
            {}, 1.0f, 0.50f, 1.08f
        };
        s2.checker.enabled = true;
        s2.checker.oddParity.overrideRefr      = 0.85f;
        s2.checker.oddParity.overrideIor       = 1.50f;
        s2.checker.evenParity.baseColorScale   = 1.45f;
        scene.push_back(s2);

        // sphere3 - simple amethyst glass.
        scene.push_back({
            "sphere3_amethyst_glass",
            GenKind::UVSphere,
            { .sphereRadius = 0.45f, .sphereNumLat = 24, .sphereNumLong = 48,
              .sphereTileLat = 6,    .sphereTileLong = 8 },
            300,
            with_y(hex(3), 0.3f), {0,0,0}, 1.0f, 3,
            {}, 1.0f, 0.50f, 1.08f
        });

        // ----------------------------------------------------------------
        // TORUS - rotated ~60° around X so the donut hole faces the camera
        // dome (not straight down).
        // ----------------------------------------------------------------
        scene.push_back({
            "torus_amber_glass",
            GenKind::Torus,
            { .torusMajor = 0.55f, .torusMinor = 0.18f,
              .torusRingSegs = 64, .torusSideSegs = 32,
              .torusTileRing = 8,  .torusTileSide = 8 },
            400,
            with_y(hex(4), 0.2f), {1.0472f, 0.0f, 0.0f}, 1.0f, 4,
            {}, 1.0f, 0.50f, 1.08f
        });

        // ----------------------------------------------------------------
        // CUBE - one cluster per face.
        // ----------------------------------------------------------------
        scene.push_back({
            "cube_copper_glass",
            GenKind::Cube,
            { .cubeHalfExtent = 0.45f, .cubeFaceSubdiv = 11, .cubeTileSize = 11 },
            500,
            with_y(hex(5), 0.0f), {0,0,0}, 1.0f, 5,
            {}, 1.0f, 0.50f, 1.08f
        });

        // ----------------------------------------------------------------
        // FLOOR - glass slab CHECKER (top + bottom + 4 walls share cluster
        // colour + material per (tu, tv) column).  Odd-parity tiles are
        // MIRROR override; even-parity keep baseline translucent glass.
        //
        // Tint multipliers - tuned for READABLE per-tile colour variation
        // without saturating into "coloured bricks".  At clusterTint=0.65:
        //   surf = 0.65 * 0.65 = 0.42   (clear per-tile colour on
        //                                translucent tiles)
        //   refr = 0.65 * 0.45 = 0.29   (stained-glass through translucent
        //                                tile to the bottom face)
        //   refl = 0.65 * 0.31 = 0.20   (mirror tiles read directional
        //                                sky / horizon, NOT cluster
        //                                identity - keeps mirror parity
        //                                visually distinct from the
        //                                translucent parity)
        // ----------------------------------------------------------------
        ObjectSpec floorObj = {
            "floor_glass_slab_checker",
            GenKind::Slab,
            { .slabHalfSizeU = 3.5f, .slabHalfSizeV = 3.5f,
              .slabTilesU    = 6,    .slabTilesV    = 6,
              .slabTileQuadsU= 4,    .slabTileQuadsV= 4,
              .slabThickness = 0.28f },
            600,
            { 0.0f, -0.7f, 0.0f }, {0,0,0}, 1.0f, 6,
            // Floor TINT multipliers - LOW so the translucent tile reads
            // as truly clear glass: you see THROUGH to the sand below,
            // tinted only subtly by the cluster colour.  The mirror tiles
            // (odd-parity override) stay sky-coloured because reflection
            // is tinted at its own (low) multiplier.
            //   surf = 0.65 * 0.25 = 0.16   (subtle tile colour on the
            //                                front surface contribution)
            //   refr = 0.65 * 0.35 = 0.23   (subtle stained-glass tint on
            //                                the see-through to sand;
            //                                low so the sand BEIGE
            //                                dominates and the cluster
            //                                colour is a hint, not a wash)
            //   refl = 0.65 * 0.31 = 0.20   (mirror tiles read sky/horizon,
            //                                not cluster identity)
            {}, /*surf*/0.25f, /*refr*/0.35f, /*refl*/0.31f
        };
        floorObj.checker.enabled = true;
        floorObj.checker.oddParity.overrideRefl = 0.85f;
        floorObj.checker.oddParity.overrideRefr = 0.0f;
        floorObj.checker.oddParity.overrideIor  = 0.0f;
        scene.push_back(floorObj);

        // ----------------------------------------------------------------
        // KLEIN BOTTLE - parametric glass.  Tilted + yawed so the
        // self-intersecting neck/body geometry reads in the default
        // orbit.  No checker.
        // ----------------------------------------------------------------
        scene.push_back({
            "klein_clear_glass",
            GenKind::Klein,
            { .kleinScale = 0.65f, .kleinNumU = 64, .kleinNumV = 32,
              .kleinTileUSize = 8, .kleinTileVSize = 8 },
            700,
            { -1.55f, 1.55f, -1.00f }, { 0.90f, 0.55f, -0.55f }, 1.0f, 8,
            {}, 1.0f, 0.50f, 1.08f,
            /*nonOrientable*/true   // Klein bottle - needs TRIANGLE_CULL_DISABLE
        });

        return scene;
    }
}
