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

// Animation and partition policy adapted from vk_partitioned_tlas shaders:
// animation_init.comp.glsl, animation_physics.comp.glsl and animation_update_instances.comp.glsl.
// See THIRD_PARTY_NOTICES.md for source revisions and acknowledgments.

// GPU-generated TLAS descriptors and compact PTLAS write operations.
// The byte offsets match D3D12_RAYTRACING_INSTANCE_DESC (64 bytes) and
// D3D12_RTAS_PARTITIONED_TLAS_OPERATION_WRITE_INSTANCE_ARGS (104 bytes).
ByteAddressBuffer OriginalRegular : register(t0);
ByteAddressBuffer OriginalPartitioned : register(t1);
ByteAddressBuffer PhysicsPrevious : register(t2);
ByteAddressBuffer PartitionActivity : register(t3);
ByteAddressBuffer SeedAngularVelocities : register(t4);
RWByteAddressBuffer CurrentRegular : register(u0);
RWByteAddressBuffer DynamicWrites : register(u1);
RWByteAddressBuffer Operation : register(u2);
// Application mirror of the partition assigned by each domino's latest PTLAS
// write. Animate maintains it; raytracing.hlsl reads it as DynamicPartitions.
RWByteAddressBuffer InstancePartitions : register(u3);
RWByteAddressBuffer PhysicsCurrent : register(u4);
RWByteAddressBuffer PartitionActivityOutput : register(u5);

#define HLSL 1
#include "RaytracingHlslCompat.h"
#include "PtlasShaderBindings.h"
#include "PtlasWrite.hlsli"
ConstantBuffer<AnimationConstants> Parameters : register(b0);

float3 position(Motion motion)
{
    return float3(motion.row0.w, motion.row1.w, motion.row2.w);
}

void setPosition(inout Motion motion, float3 value)
{
    motion.row0.w = value.x;
    motion.row1.w = value.y;
    motion.row2.w = value.z;
}

float3 basisX(Motion motion) { return float3(motion.row0.x, motion.row1.x, motion.row2.x); }
float3 basisY(Motion motion) { return float3(motion.row0.y, motion.row1.y, motion.row2.y); }
float3 basisZ(Motion motion) { return float3(motion.row0.z, motion.row1.z, motion.row2.z); }

float3 transformPoint(Motion motion, float3 localPoint)
{
    return float3(dot(motion.row0.xyz, localPoint) + motion.row0.w,
                  dot(motion.row1.xyz, localPoint) + motion.row1.w,
                  dot(motion.row2.xyz, localPoint) + motion.row2.w);
}

Motion initialMotion(uint index)
{
    uint base = index * RegularInstanceStride;
    Motion motion;
    motion.row0 = asfloat(OriginalRegular.Load4(base));
    motion.row1 = asfloat(OriginalRegular.Load4(base + 16));
    motion.row2 = asfloat(OriginalRegular.Load4(base + 32));
    motion.linearVelocity = 0;
    motion.angularVelocity = 0;
    motion.flags = 0;
    motion.partitionId = GetOriginalPartition(index);
    motion.newPartitionId = motion.partitionId;
    motion.lastContact = 0xffffffffu;
    motion.firstContact = 0xffffffffu;
    motion.lastModified = 0;
    uint interval = max(1u, Parameters.InstanceCount / max(1u, Parameters.SelfToppleCount));
    bool seed = Parameters.SelfToppleCount != 0 && (Parameters.FullScene != 0
        ? (index % interval == 0 && index / interval < Parameters.SelfToppleCount)
        : index == Parameters.InstanceCount - 1);
    if(seed)
        motion.angularVelocity = asfloat(SeedAngularVelocities.Load3((Parameters.FullScene != 0 ? index / interval : 0) * 12));
    return motion;
}

Motion loadStoredMotion(uint index)
{
    uint base = index * 96;
    Motion motion;
    motion.row0 = asfloat(PhysicsPrevious.Load4(base));
    motion.row1 = asfloat(PhysicsPrevious.Load4(base + 16));
    motion.row2 = asfloat(PhysicsPrevious.Load4(base + 32));
    uint4 linearWords = PhysicsPrevious.Load4(base + 48);
    uint4 angular = PhysicsPrevious.Load4(base + 64);
    uint4 history = PhysicsPrevious.Load4(base + 80);
    motion.linearVelocity = asfloat(linearWords.xyz);
    motion.flags = linearWords.w;
    motion.angularVelocity = asfloat(angular.xyz);
    motion.partitionId = angular.w;
    motion.lastContact = history.x;
    motion.firstContact = history.y;
    motion.lastModified = history.z;
    motion.newPartitionId = history.w;
    return motion;
}

Motion loadMotion(uint index)
{
    if(Parameters.ResetPhysics != 0) return initialMotion(index);
    return loadStoredMotion(index);
}

void storeMotion(uint index, Motion motion)
{
    uint base = index * 96;
    PhysicsCurrent.Store4(base, asuint(motion.row0));
    PhysicsCurrent.Store4(base + 16, asuint(motion.row1));
    PhysicsCurrent.Store4(base + 32, asuint(motion.row2));
    PhysicsCurrent.Store4(base + 48, uint4(asuint(motion.linearVelocity), motion.flags));
    PhysicsCurrent.Store4(base + 64, uint4(asuint(motion.angularVelocity), motion.partitionId));
    PhysicsCurrent.Store4(base + 80, uint4(motion.lastContact, motion.firstContact,
                                          motion.lastModified, motion.newPartitionId));
}

#include "physics_core.hlsli"

[numthreads(256, 1, 1)]
void Simulate(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    if(index >= Parameters.InstanceCount) return;
    Motion motion = loadMotion(index);
    if(Parameters.ManualTopple != 0 && index == Parameters.ToppleIndex)
    {
        motion.flags |= 4u;
        if(motion.firstContact == 0xffffffff)
            motion.firstContact = Parameters.FrameIndex;
    }
    bool changed = false;
    if(Parameters.AdvancePhysics != 0)
    {
        // Each thread advances its own domino against local copies of its two
        // path neighbors. Only its own Motion is stored; neighbor edits stay local.
        Motion previousNeighbor = motion, nextNeighbor = motion;
        if(index > 0) previousNeighbor = loadMotion(index - 1);
        if(index + 1 < Parameters.InstanceCount) nextNeighbor = loadMotion(index + 1);
        // Five fixed 0.008-second substeps advance 0.04 simulated seconds per
        // running frame or single step. Wall-clock frame duration is unused.
        [loop] for(uint substep = 0; substep < 5; ++substep)
            changed = computeMotion(index, motion, previousNeighbor, nextNeighbor) || changed;
        if(changed) motion.lastModified = Parameters.FrameIndex;
    }
    // Update the other ping-pong buffer while a domino moves and for two frames
    // after it stops, so both buffers retain its final transform and velocities.
    if(Parameters.ResetPhysics != 0 || changed || motion.lastModified >= Parameters.FrameIndex - 2u)
        storeMotion(index, motion);
}

[numthreads(256, 1, 1)]
void MarkPartitions(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    if(index >= Parameters.InstanceCount)
        return;
    Motion motion = loadStoredMotion(index);
    if(motion.lastModified == Parameters.FrameIndex)
    {
        // Attribute activity to the original spatial partition even while the
        // domino is global. This history drives grouping and the scenery highlight.
        uint originalPartition = GetOriginalPartition(index);
        uint ignored;
        PartitionActivityOutput.InterlockedMax(originalPartition * 4, Parameters.FrameIndex, ignored);
    }
}

[numthreads(256, 1, 1)]
void Animate(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    if (index >= Parameters.InstanceCount)
        return;

    Motion motion = loadStoredMotion(index);
    uint regularOffset = index * RegularInstanceStride;
    uint partition = GetOriginalPartition(index);
    bool active = motion.lastModified == Parameters.FrameIndex;
    if (Parameters.MarkAllInPartition != 0 && PartitionActivity.Load(partition * 4) == Parameters.FrameIndex)
        active = true;
    if (active && Parameters.UpdateMode != 0)
    {
        uint partitionX = partition % Parameters.PartitionAxis;
        uint partitionZ = partition / Parameters.PartitionAxis;
        float2 center = (float2(partitionX, partitionZ) + 0.5) * (2.0 * Parameters.SceneExtent / Parameters.PartitionAxis) - Parameters.SceneExtent;
        float3 toCamera = float3(center.x - Parameters.CameraX, -Parameters.CameraY, center.y - Parameters.CameraZ);
        if (Parameters.UpdateMode == 1 || dot(toCamera, toCamera) >= Parameters.GlobalDistance * Parameters.GlobalDistance)
            partition = PtlasGlobalPartition;
    }

    if (Parameters.UsePtlas != 0)
    {
        uint stateOffset = index * 4;
        // ForceSync initializes or refreshes this mirror without reading stale
        // membership. It emits every domino, including ones that have stopped
        // moving while the regular TLAS path was selected.
        uint oldPartition = Parameters.ForceSync != 0 ? partition : InstancePartitions.Load(stateOffset);
        // Keep a stationary domino in the global partition
        // through the first quiet frame after its original partition moved.
        if (!active && Parameters.MarkAllInPartition != 0 && oldPartition == PtlasGlobalPartition
            && Parameters.FrameIndex > 0 && PartitionActivity.Load(partition * 4) >= Parameters.FrameIndex - 1u)
            partition = PtlasGlobalPartition;
        if (Parameters.ForceSync != 0 || motion.lastModified == Parameters.FrameIndex || oldPartition != partition)
        {
            AppendPtlasWrite(index, motion, partition);
            InstancePartitions.Store(stateOffset, partition);
        }
    }
    else
    {
        [unroll] for (uint word = 0; word < RegularInstanceStride / 4; ++word)
            CurrentRegular.Store(regularOffset + word * 4, OriginalRegular.Load(regularOffset + word * 4));
        CurrentRegular.Store4(regularOffset, asuint(motion.row0));
        CurrentRegular.Store4(regularOffset + 16, asuint(motion.row1));
        CurrentRegular.Store4(regularOffset + 32, asuint(motion.row2));
    }
}
