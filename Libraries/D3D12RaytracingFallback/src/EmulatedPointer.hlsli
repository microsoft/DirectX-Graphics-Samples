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
#define uint64 uint2 
#define GpuVA uint64

#define EmulatedPointerOffsetIndex 0
#define EmulatedPointerDescriptorHeapIndex 1

GpuVA PointerAdd(GpuVA address, uint offset)
{
    address[EmulatedPointerOffsetIndex] += offset;
    return address;
}

struct RWByteAddressBufferPointer
{
    RWByteAddressBuffer buffer;
    uint offsetInBytes;
};

static
RWByteAddressBufferPointer CreateRWByteAddressBufferPointer(in RWByteAddressBuffer buffer, uint offsetInBytes)
{
    RWByteAddressBufferPointer pointer;
    pointer.buffer = buffer;
    pointer.offsetInBytes = offsetInBytes;
    return pointer;
}
