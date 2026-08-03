//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT License (MIT).
//
//*********************************************************
//
// RaytracingHlslCompat.h
//
// Constant-buffer / shader-record types shared between C++ and HLSL.
// (HlslCompat.h handles the HLSL-side typedef'ing of XMFLOAT3/etc to float3 etc.)
//
#pragma once

#ifdef HLSL
#include "HlslCompat.h"
#else
#include <DirectXMath.h>
typedef DirectX::XMFLOAT4   XMFLOAT4;
typedef DirectX::XMUINT4    XMUINT4;
typedef DirectX::XMMATRIX   XMMATRIX;
typedef UINT                uint;
#endif

struct SceneConstantBuffer
{
    XMMATRIX  viewToWorld;       // ray gen camera basis (also applies translation)
    XMFLOAT4  cameraPosition;    // .xyz = world-space eye position
    XMFLOAT4  miscParams;        // .x = aspect ratio, .y = tan(fov/2),
                                 // .z = AA sample count, .w = cluster-rainbow tint blend (0..1)
    XMFLOAT4  lightDir;          // .xyz = world-space direction TO the sun (normalized),
                                 // .w   = ambient floor [0..1] (so shadowed regions remain
                                 //        readable instead of pitch black)
    // Runtime tweakable knobs (live-cycled via the keyboard, see OnKeyDown).
    // Kept distinct from miscParams so adding more sliders doesn't disturb
    // the existing fields the raygen / closesthit shaders already read.
    XMUINT4   runtimeParams;     // .x = max reflection bounces (0..5).
                                 // .y = max refraction bounces (0..5).
                                 // .z = geometryMode (0 = clustered/DXR2 -- use
                                 //      ClusterID() lookups; 1 = traditional/
                                 //      DXR1 -- use InstanceID() + per-instance
                                 //      offsets table).
                                 // .w reserved for future sliders.
};

// Per-instance material.  Each instance carries an independent BLEND of
// optical effects rather than a single "kind" - so a chrome sphere with
// frosted holes punched through it is just `reflectivity=0.85,
// translucency=0.40` on the same MaterialDesc.  Closesthit composes the
// final colour as:
//
//   base = baseColor * cluster_palette                 (per-cluster tint)
//   surface = base * (ambient + (1-ambient)*NdotL*shadow)
//
//   if (refractivity > 0 && depth <= 1)
//       refracted = TraceBounce(refracted_dir, depth+1)
//       surface   = lerp(surface, refracted, refractivity)
//
//   if (reflectivity > 0 && depth == 0)
//       reflected = TraceBounce(reflected_dir, depth+1)
//       surface   = lerp(surface, reflected, reflectivity)
//
// AnyHit fires for any instance whose translucency > 0 OR whose
// refractivity > 0 (those instances do NOT get FORCE_OPAQUE).  AnyHit
// only does the stochastic reject - refraction is handled in closesthit
// via the bounce ray.
//   - translucency  -> per-triangle hash; IgnoreHit() if random <
//                      translucency.  Gives "frosted-glass" speckle.
//   - refractivity  -> AnyHit accepts unconditionally; closesthit's
//                      Snell-bend ray delivers the see-through.
//
// Per-instance FORCE_OPAQUE flag (and per-cluster OPAQUE flag) is set
// when BOTH translucency == 0 AND refractivity == 0 - opaque materials
// (with or without reflection) skip any-hit dispatch entirely.
#define NUM_MATERIAL_SLOTS   9u   // 4 spheres + torus + cube + floor + animated + klein

struct MaterialDesc
{
    // baseColor.xyz multiplies the shader's per-cluster colour.  Use
    // (1,1,1) to let the cluster-rainbow show through unmodified, or a
    // tint to colour-grade the whole instance.  baseColor.w is unused.
    XMFLOAT4 baseColor;
    float    reflectivity;   // [0..1] mirror-reflection blend at depth 0
    float    refractivity;   // [0..1] Snell-refraction blend at depth 0+1
    float    ior;            // refractive index (1.5=glass, 1.33=water, ignored if refractivity==0)
    float    translucency;   // [0..1] stochastic any-hit reject probability
};

// ----------------------------------------------------------------------------
// Per-CLUSTER metadata.  Indexed by ClusterID() on the GPU; built once
// per-cluster on the CPU at scene-build time.
//
// CRITICAL DESIGN POINT: every per-cluster decision the closesthit shader
// needs lives HERE - in CPU-authored data.  The shader does ONE load:
//     ClusterMeta m = g_clusterMeta[cid];
// and applies the overrides unconditionally.  No InstanceID() branches,
// no hand-mirrored cluster-id ranges, no parity formulas, no oPos-based
// wall-tile-slot decoding.  Adding a new material variant is "tweak a
// few fields in BuildClusterMetadata" - touch the CPU code, leave the
// shader alone.
//
// Stride is 48 bytes (multiple of 16) for clean ByteAddressBuffer indexing
// at cid * sizeof(ClusterMeta).
// ----------------------------------------------------------------------------
#define CLUSTER_META_FLAG_INTERIOR_SURFACE 0x1u   // back-face refr *= 0.4

struct ClusterMeta
{
    uint  colorIndex;          // ClusterColor() hash key.  For matched
                               // bottom + wall sub-clusters this is the
                               // matching top-tile cid; otherwise the
                               // cluster's own cid.
    uint  flags;               // CLUSTER_META_FLAG_* bits.
    float overrideRefl;        // <0 = no override; >=0 = replace mat.reflectivity
    float overrideRefr;        // <0 = no override; >=0 = replace mat.refractivity
    float overrideIor;         // <0 = no override; >=0 = replace mat.ior
    float baseColorScale;      // 1.0 = no scale; multiplies mat.baseColor.xyz
    // Per-cluster tint multipliers - the shader's clusterTint slider
    // gets multiplied by these before the final lerp blend.  Lets each
    // object dial its surface / refraction / reflection cluster-colour
    // bias independently (e.g. chrome wants strong reflection-tint to
    // expose the cluster grid on a near-perfect mirror; floor wants
    // weak reflection-tint so sky-direction variation dominates over
    // cluster identity on its mirror tiles).
    float surfTintMul;         // surface base-colour blend.  1.0 = use clusterTint as-is.
    float refrTintMul;         // refraction tint blend.  Default 0.50.
    float reflTintMul;         // reflection tint blend.  Default ~1.08 (= 0.70 / 0.65).
    // Per-cluster material-slot fallback retained for the traditional path's
    // mixed-region geometry lookup.  Cluster mode uses the canonical
    // g_perInstGeomMaterial[InstanceIndex(), GeometryIndex()] table.
    uint  materialSlot;
    uint  _pad0;
    uint  _pad1;
};
