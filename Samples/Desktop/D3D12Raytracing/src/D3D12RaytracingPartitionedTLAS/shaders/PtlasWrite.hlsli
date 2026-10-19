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

// Included after the animation buffers and shared layouts are declared.
// This is the GPU side of the sample's WRITE_INSTANCE operation.

uint GetOriginalPartition(uint instanceIndex)
{
    return OriginalPartitioned.Load(instanceIndex * PtlasWriteInstanceStride + PtlasPartitionIndexOffset);
}

void AppendPtlasWrite(uint instanceIndex, Motion motion, uint partitionIndex)
{
    // One thread per dynamic instance, with at most one write per thread.
    // The output capacity is InstanceCount, so the append cannot overflow.
    uint slot;
    Operation.InterlockedAdd(PtlasOperationArgCountOffset, 1, slot);
    uint source = instanceIndex * PtlasWriteInstanceStride;
    uint destination = slot * PtlasWriteInstanceStride;

    // Preserve the stable InstanceIndex, application InstanceID, BLAS address,
    // mask, flags, and hit-group index from the original native API record.
    [unroll] for(uint word = 0; word < PtlasWriteInstanceStride / 4; ++word)
        DynamicWrites.Store(destination + word * 4, OriginalPartitioned.Load(source + word * 4));

    // A transform or partition change requires WRITE_INSTANCE. The smaller
    // UPDATE_INSTANCE operation cannot change either of those fields.
    DynamicWrites.Store4(destination, asuint(motion.row0));
    DynamicWrites.Store4(destination + 16, asuint(motion.row1));
    DynamicWrites.Store4(destination + 32, asuint(motion.row2));
    DynamicWrites.Store(destination + PtlasPartitionIndexOffset, partitionIndex);
}
