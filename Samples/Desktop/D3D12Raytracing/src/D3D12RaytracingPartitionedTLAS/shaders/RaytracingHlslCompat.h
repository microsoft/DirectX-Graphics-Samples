//*********************************************************
//
// Copyright (c) 2024-2025, NVIDIA CORPORATION. All rights reserved.
// Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// The Motion record adapts AnimationState from vk_partitioned_tlas/shaders/shaderio.h.
// See THIRD_PARTY_NOTICES.md for source revisions and acknowledgments.

#pragma once

// Data shared by the D3D12 application and its shaders. Keep the layout
// assertions in the C++ build when changing these structures.
#ifdef HLSL
#define PTLAS_MAT4 float4x4
#define PTLAS_VEC4 float4
#define PTLAS_VEC3 float3
#define PTLAS_UINT uint
#else
#include <DirectXMath.h>
#include <cstdint>
#define PTLAS_MAT4 DirectX::XMFLOAT4X4
#define PTLAS_VEC4 DirectX::XMFLOAT4
#define PTLAS_VEC3 DirectX::XMFLOAT3
#define PTLAS_UINT uint32_t
#endif

struct FrameConstants
{
  PTLAS_MAT4 InvViewProjection;
  PTLAS_VEC4 CameraPosition;
  PTLAS_UINT PartitionAxis;
  PTLAS_UINT FrameIndex; // Rendered-frame clock shared with AnimationConstants, including paused frames.
  PTLAS_UINT ShowPartitionColors;
  float SceneExtent;
  PTLAS_VEC4 SunDirection;
  PTLAS_VEC4 MaterialParameters; // Metallic, roughness, light intensity, maximum ray depth.
  PTLAS_UINT PickX;
  PTLAS_UINT PickY;
  PTLAS_UINT PickActive;
  PTLAS_UINT StaticInstanceCount;
};

struct AnimationConstants
{
  PTLAS_UINT UpdateMode;
  PTLAS_UINT UsePtlas;
  PTLAS_UINT ForceSync; // Write every domino after initialization, reset, or switching back to PTLAS.
  PTLAS_UINT InstanceCount;
  PTLAS_UINT FullScene;
  PTLAS_UINT PartitionAxis;
  float CameraX;
  float CameraZ;
  float GlobalDistance;
  float SceneExtent;
  PTLAS_UINT ManualTopple;
  PTLAS_UINT ToppleIndex;
  float CameraY;
  PTLAS_UINT FrameIndex; // Zero-based rendered frames since resource creation; pause/reset-motion keep it running.
  PTLAS_UINT ResetPhysics;
  PTLAS_UINT AdvancePhysics;
  PTLAS_UINT SelfToppleCount;
  PTLAS_UINT MarkAllInPartition;
  float PhysicsStep; // Seconds per substep; Simulate runs five substeps when AdvancePhysics is set.
};

// 96-byte physics record shared by simulation shaders and CPU state readback.
// Raw loads/stores in animation.hlsl and raytracing.hlsl, and dump readers in
// tools/, depend on field offsets as well as sizeof(Motion).
// partitionId and newPartitionId both keep the original spatial partition.
// Current PTLAS membership is tracked separately in InstancePartitions; a
// physics state dump therefore contains no record of global-partition moves.
struct Motion
{
  PTLAS_VEC4 row0;
  PTLAS_VEC4 row1;
  PTLAS_VEC4 row2;
  PTLAS_VEC3 linearVelocity;
  PTLAS_UINT flags; // Low two bits: 0 free, 1 ground, 2 domino contact; 0x4 topple request, 0x8 ground-settled.
  PTLAS_VEC3 angularVelocity;
  PTLAS_UINT partitionId; // Original spatial partition, assigned during initialization.
  PTLAS_UINT lastContact; // Last domino contact or forced topple frame; UINT_MAX means none.
  PTLAS_UINT firstContact; // First domino contact or manual topple frame; UINT_MAX means none.
  PTLAS_UINT lastModified; // Last frame with a transform change in any substep; initialized to zero.
  PTLAS_UINT newPartitionId; // Initialized to partitionId and retained unchanged in the physics record.
};

struct RayPayload
{
  PTLAS_VEC3 color;
  PTLAS_VEC3 normal;
  PTLAS_VEC3 position;
  PTLAS_UINT hit; // 0 miss, 1 surface, 2 static surface in a recently active partition.
  PTLAS_UINT primaryHitId; // Coarse normal/partition hash for compositing; see raytracing.hlsl.
  float primaryDepth;
  PTLAS_UINT instanceId;
  PTLAS_UINT depth;
};

#undef PTLAS_MAT4
#undef PTLAS_VEC4
#undef PTLAS_VEC3
#undef PTLAS_UINT

#ifndef HLSL
static_assert(sizeof(FrameConstants) == 144);
static_assert(sizeof(AnimationConstants) == 76);
static_assert(sizeof(Motion) == 96);
static_assert(sizeof(RayPayload) == 56);
#endif
