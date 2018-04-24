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
#include "FallbackDebug.h"

#if ENABLE_UAV_LOG
void OutputDebugLog(ID3D12RaytracingFallbackDevice *pDevice);
#endif

#if ENABLE_ACCELERATION_STRUCTURE_VISUALIZATION
void VisualizeAccelerationStructureLevel(ID3D12RaytracingFallbackDevice *pDevice, UINT level);
#endif

void BuildRaytracingAccelerationStructureOnCpu(
    _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
    _Out_ void *pData);
