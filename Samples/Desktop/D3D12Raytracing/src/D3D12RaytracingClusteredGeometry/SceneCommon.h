//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT License (MIT).
//
//*********************************************************
//
// SceneCommon.h
//
// Shared per-cluster checker-config types used by BOTH the runtime
// (ClusterObject in the engine code) and the scene-definition data
// tables (SceneData::ObjectSpec).  Lives in its own header so neither
// side has to know about the other - keeps the data definition free
// of "engine" dependencies.
//
#pragma once

// Per-side material override for a (gridU + gridV) parity tile.  Each
// field's <0 sentinel means "no override - keep the baseline material's
// value".  baseColorScale defaults to 1.0 (no scale) and is a multiplier
// rather than a sentinel.
//
// Used by BOTH the slab's top/bottom/wall checker (gridU = tu, gridV = tv)
// AND by the spheres' lat/long checker (gridU = latIdx, gridV = longIdx).
// Same formula across all object kinds - the shader never branches by
// instance ID to decide parity.
struct CheckerOverride
{
    float overrideRefl     = -1.0f;
    float overrideRefr     = -1.0f;
    float overrideIor      = -1.0f;
    float baseColorScale   =  1.0f;
};

struct CheckerConfig
{
    bool             enabled = false;
    CheckerOverride  evenParity;     // (gridU + gridV) & 1 == 0
    CheckerOverride  oddParity;      // (gridU + gridV) & 1 == 1
};
