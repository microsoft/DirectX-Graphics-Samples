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

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <DirectXMath.h>
#include <assert.h>
#include <comdef.h>
#include <atlbase.h>
#include <memory>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <deque>
#include <string>
#include <strsafe.h>
#include "d3d12_1.h"
#include "d3dx12.h"
#include "dxc\dxcapi.h"
#include "dxc\dxcapi.use.h"
#include "dxc\dxcdxrfallbackcompiler.h"
#include "dxc\dxcdxrfallbackcompiler.use.h"
#include "dxc\hlsl\DxilContainer.h"
#include "Util.h"

#include "FallbackDebug.h"

#include "D3D12RaytracingFallback.h"
#include "RaytracingCompatibilityDebug.h"
#include "ComObject.h"

#include "NativeRaytracing.h"

#include "FallbackDxil.h"
#include "RaytracingHlslCompat.h"
#include "DxilShaderPatcher.h"
#include "AccelerationStructureValidator.h"
#include "AccelerationStructureBuilder.h"
#include "AccelerationStructureBuilderFactory.h"
#include "TraversalShaderBuilder.h"
#include "RaytracingProgram.h"
#include "RaytracingProgramFactory.h"
#include "FallbackLayer.h"

// Validators
#include "BVHValidator.h"

// Traversal Builders
#include "BVHTraversalShaderBuilder.h"

// Acceleration Structure Builders
#include "GetBVHCompactedSizeBindings.h"
#include "ShaderPass.h"
#include "BitonicSort.h"
#include "SceneAABBCalculator.h"
#include "MortonCodesCalculator.h"
#include "RearrangeElementsPass.h"
#include "LoadInstancesPass.h"
#include "LoadPrimitivesPass.h"
#include "ConstructHierarchyPass.h"
#include "ConstructAABBPass.h"
#include "PostBuildInfoQuery.h"
#include "GpuBvh2Copy.h"
#include "TreeletReorder.h"
#include "GpuBvh2Builder.h"

// Dispatchers
#include "UberShaderBindings.h"
#include "UberShaderRaytracingProgram.h"

#define USE_PIX_MARKERS 0
#if USE_PIX_MARKERS
#include <pix3.h>
static const UINT FallbackPixColor = PIX_COLOR(10, 10, 255);
#endif
