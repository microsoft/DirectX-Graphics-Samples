//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT License (MIT).
//
//*********************************************************
//
// MaterialData.h
//
// PER-INSTANCE material table.  Pure ART DATA: indexed by InstanceID()
// in the closesthit shader, uploaded once into a structured buffer at
// scene-build time.  Engine code (D3D12RaytracingClusteredGeometry.cpp)
// just calls MaterialData::kMaterials and uploads it - no hardcoded
// material values anywhere in the .cpp.
//
#pragma once

#include "RaytracingHlslCompat.h"
#include <array>

namespace MaterialData
{
    extern const std::array<MaterialDesc, NUM_MATERIAL_SLOTS> kMaterials;
}
