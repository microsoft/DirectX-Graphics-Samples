//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT License (MIT).
//
//*********************************************************

#include "stdafx.h"
#include "MaterialData.h"

namespace MaterialData
{
    // Per-instance materials.  Indexed by InstanceID() in the shader.
    //
    // Most objects are GLASS variants (different IORs / tints) so the
    // closesthit's refraction + internal-reflection compose all over
    // the scene.  Sphere0 (largest, foreground) is opaque CHROME -
    // distinct non-glass focal point for visual contrast.
    //
    // Per-CLUSTER checker overrides (mirror / translucent / matte
    // variants applied at scene-build time) live in SceneData::kScene
    // - NOT here.  This table is the baseline material per instance,
    // before any per-cluster override.
    //
    // Layout matches struct MaterialDesc { XMFLOAT4 baseColor;
    //     float reflectivity, refractivity, ior, translucency; }.
    //
    const std::array<MaterialDesc, NUM_MATERIAL_SLOTS> kMaterials = {{
        // {{ baseColor.xyzw },           refl,  refr,  ior,   trans }
        //   slot 0 - sphere0 CHROME (opaque foreground mirror)
        {  { 0.95f, 0.95f, 1.00f, 0.0f }, 0.95f, 0.0f,  0.0f,  0.0f },
        //   slot 1 - sphere1 clear glass (densest IOR)
        {  { 0.92f, 0.95f, 1.00f, 0.0f }, 0.10f, 0.82f, 1.55f, 0.0f },
        //   slot 2 - sphere2 matte aqua (FORCE_OPAQUE; checker adds variants)
        {  { 0.55f, 0.95f, 0.85f, 0.0f }, 0.0f,  0.0f,  0.0f,  0.0f },
        //   slot 3 - sphere3 amethyst glass
        {  { 0.85f, 0.65f, 1.00f, 0.0f }, 0.10f, 0.78f, 1.50f, 0.0f },
        //   slot 4 - torus amber glass
        {  { 0.95f, 0.80f, 0.55f, 0.0f }, 0.10f, 0.78f, 1.50f, 0.0f },
        //   slot 5 - cube translucent copper-tinted glass
        {  { 0.92f, 0.78f, 0.60f, 0.0f }, 0.10f, 0.82f, 1.50f, 0.0f },
        //   slot 6 - floor glass slab (per-tile checker via scene config).
        //             High refractivity (0.93) so translucent tiles read as
        //             clear glass you can see through to the bottom + sand.
        {  { 0.85f, 0.92f, 0.95f, 0.0f }, 0.06f, 0.93f, 1.50f, 0.0f },
        //   slot 7 - animated clear glass (refl 0.30; ior 1.5 - standard glass, dialled back from earlier 2.4/3.0 crystal-dramatic look)
        {  { 0.85f, 0.90f, 1.00f, 0.0f }, 0.30f, 0.78f, 1.5f,  0.0f },
        //   slot 8 - Klein bottle clear glass
        {  { 0.85f, 0.90f, 1.00f, 0.0f }, 0.08f, 0.78f, 1.5f,  0.0f },
    }};
}
