//*********************************************************
//
// Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

// ByteAddressBuffer access to the DXR instance and indirect-operation ABI.
// Keep the native D3D12 structures on the CPU; verify every shader offset here.
#ifdef HLSL
#define PTLAS_UINT uint
#else
#include <d3d12.h>
#include <cstddef>
#define PTLAS_UINT UINT
#endif

static const PTLAS_UINT RegularInstanceStride = 64;
static const PTLAS_UINT PtlasWriteInstanceStride = 104;
static const PTLAS_UINT PtlasPartitionIndexOffset = 76;
static const PTLAS_UINT PtlasOperationArgCountOffset = 4;
static const PTLAS_UINT PtlasGlobalPartition = 0xffffffffu;

#ifndef HLSL
static_assert(RegularInstanceStride == sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
static_assert(PtlasWriteInstanceStride == sizeof(D3D12_RTAS_PARTITIONED_TLAS_OPERATION_WRITE_INSTANCE_ARGS));
static_assert(PtlasPartitionIndexOffset == offsetof(D3D12_RTAS_PARTITIONED_TLAS_OPERATION_WRITE_INSTANCE_ARGS, PartitionIndex));
static_assert(PtlasOperationArgCountOffset == offsetof(D3D12_RTAS_PARTITIONED_TLAS_OPERATION, ArgCount));
static_assert(PtlasGlobalPartition == D3D12_RTAS_PARTITIONED_TLAS_PARTITION_INDEX_GLOBAL_PARTITION);
#endif

#undef PTLAS_UINT
