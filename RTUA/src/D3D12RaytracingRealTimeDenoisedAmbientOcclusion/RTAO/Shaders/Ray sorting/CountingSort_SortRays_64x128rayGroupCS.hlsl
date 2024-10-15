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

// Desc: Counting Sort of rays based on their origin depth and direction.
// Supports:
// - Up to 8K rays per ray group + 12 bit hash key
// - Max Ray Group dimensions: 64x128
// Rays can be disabled by setting their ray origin depth to 0 (i.e. invalidating them). 
// Such rays will get moved to the end of the sorted ray group 
// and have a source index offset of [0xff, 0xff]. 
// The ray hash is calculated from ray direction and origin depth
// Ref: Costa2014, Ray Reordering Techniques for GPU Ray-Cast Ambient Occlusion

// Algorithm: Counting Sort
// - Load ray origin depths and ray direction hashes into SMem.
// - Calculate min max origin depth per ray group.
// - Generate hash keys from ray directions and origin depths.
// - Calculates histograms for the key hashes.
// - Calculates a prefix sum of the histograms.
// - Scatter write the ray source index offsets based on their hash and the prefix sum for the hash key into SMem cache.
// - Linearly spill sorted ray source index offsets from SMem cache into VRAM.

// Shared Memory layout
// 1 - [8K 8b] - Ray Direction Hash
// 2 - [8K 16b] - Ray origin depth
// 3 - [4 x 512 x 16b] - Min/Max depth // In the final algorithm only small part is used of this range
// 4 - [8K 16b] - Ray hash key
// 5 - [4K 16b] - histrogram
// 6 - [8K 16b] - SrcRay index
//
//// Memory diagram:
//// - each lane represent 8 bits of a 32 bit element going from least to most significant bits top to bottom
//// - each column represents 2K elements
//// - each memory diagram represents the end state after an algorithm step.
//// - cells that changed are represent by an x in the second diagram on the right
//// - X-Y region is aliased with cells representing X or Y
//// - "-" zeroed out region
////
//// Memory layout at subsequent algorithm steps:
//// - Load ray origin depths and ray direction hashes into SMem.
////  | - - 1 1 |  Least significant bits
////  | - - 1 1 |
////  | 2 2 2 2 |
////  | 2 2 2 2 |  Most significant bits
////
//// - Calculate min max origin depth per ray group.
////  | 3 3 1 1 |     | x x - - | 
////  | 3 3 1 1 |     | x x - - | 
////  | 2 2 2 2 |     | - - - - | 
////  | 2 2 2 2 |     | - - - - | 
////
//// - Generate hash keys from ray directions and origin depths.
////  | 5 5 - - |     | x x x x | 
////  | 5 5 - - |     | x x x x | 
////  | 4 4 4 4 |     | x x x x | 
////  | 4 4 4 4 |     | x x x x | 
////
//// - Calculates a prefix sum of the histograms.
////  | 5 5 - - |     | x x - - | 
////  | 5 5 - - |     | x x - - | 
////  | 4 4 4 4 |     | - - - - | 
////  | 4 4 4 4 |     | - - - - | 
////
//// - Scatter write the ray source index offsets based on their hash and the prefix sum for the hash key into SMem cache.
////  | 5 5 6 6 |     | - - x x | 
////  | 5 5 6 6 |     | - - x x | 
////  | 4-6 4 4 |     | x x - - | 
////  | 4-6 4 4 |     | x x - - | 


#define HLSL
#include "RaytracingHlslCompat.h"
#include "RaytracingShaderHelper.hlsli"
#include "Ray Sorting/RaySorting.hlsli"

Texture2D<NormalDepthTexFormat> g_inRayDirectionOriginDepth : register(t0);    // R11G11B10 texture. Note that this format doesn't store negative values.

// Source ray index offset for a given sorted ray index offset within a ray group.
// This is essentially a sorted source ray index offsets buffer within a ray group.
// Inactive rays have a valid index but have INACTIVE_RAY_INDEX_BIT_Y bit set in the y coordinate to 1.
RWTexture2D<uint2> g_outSortedToSourceRayIndexOffset : register(u0);   

RWTexture2D<float4> g_outDebug : register(u2); 

ConstantBuffer<SortRaysConstantBuffer> cb: register(b0);

namespace HashKey {
    enum { 
        RayDirectionKeyBits1D = 4, 
        RayOriginDepthKeyBits = 2,
        NumBits = 2 * RayDirectionKeyBits1D + RayOriginDepthKeyBits  // <= 12
    };
}

#define MIN_WAVE_LANE_COUNT 16
#define MAX_WAVES ((MAX_RAYS + MIN_WAVE_LANE_COUNT - 1) / MIN_WAVE_LANE_COUNT)

namespace SMem
{
    namespace Size
    {
        enum {
            Histogram = NUM_KEYS,               // <= 4096
        };
    }

    // 32bit element offset
    namespace Offset 
    {
        enum {
            Histogram = 0,
            Key8b = Size::Histogram,
            Key16b = 8192,
            Depth16b = 8192,
            WaveDepthMin = 0,
            WaveDepthMax = MAX_WAVES,           // <= 512
            RayIndex = Size::Histogram,
        };
    }
}

#if MAX_RAYS > 8192 || NUM_KEYS > 4096
The shader supports up to 8192 input rays and 4096 num keys.
#endif

//********************************************************************
// Hash Key
//  - a hash calculated from ray direction and origin depth
//  - max values:
//      12 bits(4096) for 8K rays.
//      13 bits(8192) for 4K rays.
// The 15th and 16th bits are reserved:
// - 15th bit == (1) - invalid ray. These rays will get sorted to the end.
// - 16th bit == (1) - invalid key. To handle when a key is replaced by Source Ray index in SMEM.

#if (KEY_NUM_BITS > 13) || (KEY_NUM_BITS > 12 && MAX_RAYS > 4096)
Key bit size is out of supported limits.
#endif
#if (RAY_DIRECTION_HASH_KEY_BITS_1D > 4)
Ray direction hash key can only go up to 8 bits for both direction axes since
its stored in 8bit format.
#endif


//********************************************************************


//********************************************************************
// Ray Count SMem cache.
// Supports up to 16 bit (64K) ray counts per bin.
// Used for:
//  - to store number of binned rays. 
//  - as an intermediate cache for prefix sum computations.
// Stores 16bit values, with two values per entry.
// Stored as two ping-pong buffers.
// - Hi bits: odd ping-pong buffer ID
// - Lo bits: even ping-pong buffer ID
//********************************************************************

//********************************************************************
// SMEM stores 16 bit values, two 16bit values per 32bit entry:
// - Hi bits: odd indices
// - Lo bits: even indices
// SMEM is used for two mutually exclusive and temporally overlapping purposes
// so as to fit all caching within Shared Memory limits:
// - First it caches the hashed key per pixel - 15 bits
// - Second it caches the source index offset for a given sorted pixel - 2D 7+8bit index.
//   The values for the two purposes overlap in the cache during the shader execution. 
//   The key is generated first, but the source indices overwrite it later, while
//   the key may still be needer by another thread. 
//   Therefore the most significant bit is used
//   to denote whether the stored hashed key is still valid. 
//   If the key is no longer valid, it is regenerated again. 
//   To lower the collision and keep as many cached keys, the cache
//   is extended to the remaining shader memory limit and the keys
//   are stored at an offset. This way, the last 2 * offset
//   keys won't be invalidated.
//
//  PERFORMANCE tip:
//   Use as little rays and as small hash key bit size to leave 
//   as much room as possible for the hashed keys.
#define SMCACHE_SIZE 8192
groupshared uint SMEM[SMCACHE_SIZE];
//********************************************************************


//********************************************************************
// Store a 16 bit value in the Shared Memory.
// The 16 bit value is stored in 32bit value range <0, 8K)
// in layered fashion to avoid bank conflicts on subsequent 
// index accesses among subsequent threads. 
// It is stored in 16bit layers at index16b starting from indexOffset32b at first row
// which is the row of least significant 16 bits.
//  index16B - 32bit offset up to (16K - 1).
//  indexOffset32b - 32bit offset up to (8K - 1). 
//  index16b / 2 + indexOffset32b must be less than 8K.
// For example: 
//  - index16b == {0 - 4}, 
//  - indexOffset32b == 6
//  Shared memory {8x32b}:
//  | - - - - - 0 1 2 |  Least significant bits
//  | - - - - - 0 1 2 |
//  | 3 4 - - - - - - |
//  | 3 4 - - - - - - |  Most significant bits
void Store16bitUintInSMem(in uint index16b, in uint value, in uint indexOffset32b)
{
    uint offsetIndex = indexOffset32b + index16b;
    bool useHi16Bits = offsetIndex >= SMCACHE_SIZE;
    uint smemIndex = offsetIndex - useHi16Bits * SMCACHE_SIZE;
    uint packedValue = (value & 0xffff) << (useHi16Bits * 16);

    uint bitsToZeroOut = useHi16Bits ? 0x0000ffff : 0xffff0000;
    InterlockedAnd(SMEM[smemIndex], bitsToZeroOut);
    InterlockedAdd(SMEM[smemIndex], packedValue);
}

void Store16bitUintInLow16bitSMem(in uint index16b, in uint value, in uint indexOffset32b)
{
    uint packedValue = value & 0xffff;
    uint smemIndex = indexOffset32b + index16b;

    uint bitsToZeroOut = 0xffff0000;
    InterlockedAnd(SMEM[smemIndex], bitsToZeroOut);
    InterlockedAdd(SMEM[smemIndex], packedValue);
}

void Store16bitFloatInSMem(in uint index16b, in float value, in uint indexOffset32b)
{
    uint encoded16bitFloat = f32tof16(value);
    Store16bitUintInSMem(index16b, encoded16bitFloat, indexOffset32b);
}

void Store16bitFloatInLow16bitSMem(in uint index16b, in float value, in uint indexOffset32b)
{
    uint encoded16bitFloat = f32tof16(value);
    Store16bitUintInLow16bitSMem(index16b, encoded16bitFloat, indexOffset32b);
}

uint Load16bitUintFromSMem(in uint index16b, in uint indexOffset32b)
{
    uint offsetIndex = indexOffset32b + index16b;
    bool useHi16Bits = offsetIndex >= SMCACHE_SIZE;
    uint smemIndex = offsetIndex - useHi16Bits * SMCACHE_SIZE;
    uint packedValue = SMEM[smemIndex];
    return (packedValue >> (useHi16Bits * 16)) & 0xffff;
}

uint Load16bitUintFromLow16bitSMem(in uint index16b, in uint indexOffset32b)
{
    uint smemIndex = indexOffset32b + index16b;
    uint packedValue = SMEM[smemIndex];
    return packedValue & 0xffff;
}

uint Load16bitUintFromHi16bitSMem(in uint index16b, in uint indexOffset32b)
{
    uint smemIndex = (indexOffset32b + index16b) - SMCACHE_SIZE;
    uint packedValue = SMEM[smemIndex];
    return (packedValue >> 16) & 0xffff;
}

float Load16bitFloatFromSMem(in uint index16b, in uint indexOffset32b)
{
    uint encoded16bitFloat = Load16bitUintFromSMem(index16b, indexOffset32b);
    return f16tof32(encoded16bitFloat);
}

float Load16bitFloatFromLow16bitSMem(in uint index16b, in uint indexOffset32b)
{
    uint encoded16bitFloat = Load16bitUintFromLow16bitSMem(index16b, indexOffset32b);
    return f16tof32(encoded16bitFloat);
}

float Load16bitFloatFromHi16bitSMem(in uint index16b, in uint indexOffset32b)
{
    uint encoded16bitFloat = Load16bitUintFromHi16bitSMem(index16b, indexOffset32b);
    return f16tof32(encoded16bitFloat);
}

uint AddTo16bitValueInSMem(in uint index16b, in uint value, in uint indexOffset32b)
{
    uint offsetIndex = indexOffset32b + index16b;
    bool useHi16Bits = offsetIndex >= SMCACHE_SIZE;
    uint smemIndex = offsetIndex - useHi16Bits * SMCACHE_SIZE;
    uint packedValue = (value & 0xffff) << (useHi16Bits * 16);
    uint newValue;
    InterlockedAdd(SMEM[smemIndex], packedValue, newValue);

    return (newValue >> (useHi16Bits * 16)) & 0xffff;
}
//********************************************************************


//********************************************************************
// Store an 8 bit value in the Shared Memory.
// The 8bit value is stored in 32bit value range <indexOffset32b, 8K)
// in layered fashion to avoid bank conflicts on subsequent 
// index accesses among subsequent threads. 
// It is stored in the least significant 16 bit layer at index8b 
// starting from indexOffset32b at each row.
//  index8b - 32bit offset up to (16K - 1).
//  indexOffset32b - 32bit offset up to (8K - 1). 
//  index8b / 2 + indexOffset32b must be less than 8K.
// For example: 
//  - index8b == {0 - 4}, 
//  - indexOffset32b == 6
//  Shared memory {8x32b}:
//  | - - - - - 0 1 2 |  Least significant 8 bits
//  | - - - - - 3 4 - |
//  | - - - - - - - - |
//  | - - - - - - - - |  Most significant 8 bits
void Store8bitUintInLow16bitSMem(in uint index8b, in uint value, in uint indexOffset32b)
{
    uint offsetIndex = indexOffset32b + index8b;
    bool useHi8Bits = offsetIndex >= SMCACHE_SIZE;
    uint smemIndex = offsetIndex - useHi8Bits * (SMCACHE_SIZE - indexOffset32b);
    uint packedValue = (value & 0xff) << (useHi8Bits * 8);

    uint bitsToZeroOut = useHi8Bits ? 0xffff00ff : 0xffffff00;
    InterlockedAnd(SMEM[smemIndex], bitsToZeroOut);
    InterlockedAdd(SMEM[smemIndex], packedValue);
}

uint Load8bitUintFromLow16bitSMem(in uint index8b, in uint indexOffset32b)
{
    uint offsetIndex = indexOffset32b + index8b;
    bool useHi8Bits = offsetIndex >= SMCACHE_SIZE;
    uint smemIndex = offsetIndex - useHi8Bits * (SMCACHE_SIZE - indexOffset32b);
    uint packedValue = SMEM[smemIndex];
    return (packedValue >> (useHi8Bits * 8)) & 0xff;
}
//********************************************************************


void InitializeSharedMemory(in uint GI)
{
    for (uint i = GI; i < SMCACHE_SIZE; i += NUM_THREADS)
    {
        SMEM[i] = 0;
    }
    GroupMemoryBarrierWithGroupSync();
}

// Create a hash key from a ray direction. 
uint CreateRayDirectionHashKey(in float2 encodedRayDirection)
{
    float2 rayDirectionKey;
    if (cb.useOctahedralRayDirectionQuantization)
    {
        rayDirectionKey = encodedRayDirection;
    }
    else // Spherical coordinates.
    {
        float3 rayDirection = DecodeNormal(encodedRayDirection.xy);

        // Convert the vector from cartesian to spherical coordinates.
        float azimuthAngle = atan2(rayDirection.y, rayDirection.x);
        float polarAngle = acos(rayDirection.z);

        // Normalize to <0,1>.
        rayDirectionKey = 
            float2(
                (azimuthAngle / (2 * PI)),
                polarAngle / PI);
    }

    // Calculate hashes.
    const uint NormalHashKeyBins1D = 1 << RAY_DIRECTION_HASH_KEY_BITS_1D;
    const uint MaxNormalHashKeyBinValue = NormalHashKeyBins1D - 1;
    uint2 rayDirectionHashKey = min(rayDirectionKey * MaxNormalHashKeyBinValue, MaxNormalHashKeyBinValue);

    return   (rayDirectionHashKey.y << RAY_DIRECTION_HASH_KEY_BITS_1D)
            + rayDirectionHashKey.x;
}

// Create a hash key from a ray origin depth. 
uint CreateDepthHashKey(in float rayOriginDepth, in float2 rayGroupMinMaxDepth)
{
    if (DEPTH_HASH_KEY_BITS == 0)
        return 0;

    float relativeDepth = rayOriginDepth - rayGroupMinMaxDepth.x;
    float rayGroupDepthRange = rayGroupMinMaxDepth.y - rayGroupMinMaxDepth.x;
    const uint DepthHashKeyBins = 1 << DEPTH_HASH_KEY_BITS;
    const uint MaxDepthHashKeyBinValue = DepthHashKeyBins - 1;
    
    // Simple linear quantization within the min/max range.
    float binDepthSize = max(rayGroupDepthRange / MaxDepthHashKeyBinValue, cb.binDepthSize);
    uint depthHashKey = min(relativeDepth / binDepthSize, MaxDepthHashKeyBinValue);

    return depthHashKey;
}

// Create a hash key from a ray index. 
uint CreateIndexHashKey(in uint2 rayIndex)
{
    if (INDEX_HASH_KEY_BITS == 0)
        return 0;

    const uint IndexHashKeyBins = 1 << INDEX_HASH_KEY_BITS;
    const uint MaxIndexHashKeyBinValue = IndexHashKeyBins - 1;
    uint2 RayGroupDim = uint2(SortRays::RayGroup::Width, SortRays::RayGroup::Height);
#if INDEX_HASH_KEY_BITS == 12
    uint indexHashKey = ((rayIndex.y * RayGroupDim.x) + rayIndex.x) / 2;
#elif 0
    uint indexHashKey = rayIndex.y / (RayGroupDim.y / 4);
#else
    uint indexHashKey = ((rayIndex.y >= RayGroupDim.y / 2) << 1) + (rayIndex.x >= RayGroupDim.x / 2);
#endif
    return ((indexHashKey >> (2 - INDEX_HASH_KEY_BITS)) & (MaxIndexHashKeyBinValue));
}

uint CreateRayHashKey(in uint2 rayIndex, in uint rayDirectionHashKey, in float rayOriginDepth, in float2 rayGroupMinMaxDepth)
{
    uint rayOriginDepthHashKey = CreateDepthHashKey(rayOriginDepth, rayGroupMinMaxDepth);
    uint rayIndexHashKey = CreateIndexHashKey(rayIndex);

    uint hashKey = (rayOriginDepthHashKey << (2 * RAY_DIRECTION_HASH_KEY_BITS_1D + INDEX_HASH_KEY_BITS))
            + (rayDirectionHashKey << (INDEX_HASH_KEY_BITS))
            + rayIndexHashKey;

    // Prevent aliasing with the inactive ray key value.
    hashKey = min(hashKey, INACTIVE_RAY_KEY - 1);

    return hashKey;
}

uint CreateRayHashKey(in uint2 rayIndex, in float2 encodedRayDirection, in float rayOriginDepth, in float2 rayGroupMinMaxDepth)
{
    uint rayDirectionHashKey = CreateRayDirectionHashKey(encodedRayDirection);
    uint rayOriginDepthHashKey = CreateDepthHashKey(rayOriginDepth, rayGroupMinMaxDepth);
    uint rayIndexHashKey = CreateIndexHashKey(rayIndex);

    uint hashKey = (rayOriginDepthHashKey << (2 * RAY_DIRECTION_HASH_KEY_BITS_1D + INDEX_HASH_KEY_BITS))
        + (rayDirectionHashKey << (INDEX_HASH_KEY_BITS))
        + rayIndexHashKey;

    // Prevent aliasi with the inactive ray key value.
    hashKey = min(hashKey, INACTIVE_RAY_KEY - 1);

    return hashKey;
}


// Calculate ray direction hash keys and cache depths.
void CalculatePartialRayDirectionHashKeyAndCacheDepth(in uint2 Gid, in uint GI)
{
    uint2 RayGroupDim = uint2(SortRays::RayGroup::Width, SortRays::RayGroup::Height);
    uint2 GroupStart = Gid * RayGroupDim;
    uint2 NextGroupStart = GroupStart + RayGroupDim;

    // Trim the Ray Group Dim to valid dims.
    RayGroupDim = min(NextGroupStart, cb.dim) - GroupStart;
    uint NumRays = RayGroupDim.y * RayGroupDim.x;

    for (uint ray = GI; ray < NumRays; ray += NUM_THREADS)
    {
        uint2 rayIndex = uint2(ray % RayGroupDim.x, ray / RayGroupDim.x);
        uint2 pixel = GroupStart + rayIndex;

        float2 encodedRayDirection;
        float rayOriginDepth;
        UnpackEncodedNormalDepth(g_inRayDirectionOriginDepth[pixel], encodedRayDirection, rayOriginDepth);
        bool isRayValid = rayOriginDepth != INVALID_RAY_ORIGIN_DEPTH; 

        // The ray direction hash key doesn't need to store if the ray value is valid for now, 
        // and there's no space when storing it as 8bit. The reason the hash from the
        // ray direction needs to be stored as 8bit is to leave room in Shared Memory 
        // for Ray Origin Depth Min Max reduction values. Once that is computed, 
        // ray origin depth hash can be computed and the final 16bit hash key will reserve 
        // bits for invalid rays. Until then the ray origin depth is used to identify
        // invalid rays.        
        uint rayDirectionHashKey = 0;
        if (isRayValid)
        {
            rayDirectionHashKey = CreateRayDirectionHashKey(encodedRayDirection);
        }

        // Cache the depth.
        Store16bitFloatInSMem(ray, rayOriginDepth, SMem::Offset::Depth16b);

        // Cache the key.
        Store8bitUintInLow16bitSMem(ray, rayDirectionHashKey, SMem::Offset::Key8b);
    }
    GroupMemoryBarrierWithGroupSync();
}

// Calculate an estimate of depth min max range of all rays within the ray group.
float2 CalculateRayGroupMinMaxDepth(in uint GI, uint2 Gid)
{
    if (DEPTH_HASH_KEY_BITS == 0)
        return 0;

    uint2 RayGroupDim = uint2(SortRays::RayGroup::Width, SortRays::RayGroup::Height);
    uint2 GroupStart = Gid * RayGroupDim;
    uint2 NextGroupStart = GroupStart + RayGroupDim;

    // Trim the Ray Group Dim to valid dims.
    RayGroupDim = min(NextGroupStart, cb.dim) - GroupStart;
    uint NumRays = RayGroupDim.y * RayGroupDim.x;

    // Optimization
    // Rather than retrieving true Min/Max of the set, estimate the Min/Max
    // via a single wave with sparse taps.
    if (GI < WaveGetLaneCount())
    {
        uint sampleDistance = NumRays / WaveGetLaneCount();
        uint index = GI * sampleDistance;

        // Ensure each lane hits a different memory bank.
        // This also breaks the uniformity of sampling up a little bit.
        // Given NUM_RAYS is power of 2, the max index value 
        // has all bits 1s, and thus index won't go out of bounds.
        uint MaxValueFromWaveGetLaneCount = 128;
        uint mask = ~(MaxValueFromWaveGetLaneCount - 1);
        index = index & mask + GI;

        float rayOriginDepth = Load16bitFloatFromHi16bitSMem(index, SMem::Offset::Depth16b);
        bool isRayValid = rayOriginDepth != INVALID_RAY_ORIGIN_DEPTH;
        float MaxRayOriginDepthValue = FLT_10BIT_MAX;
        float waveDepthMin = WaveActiveMin(isRayValid ? rayOriginDepth : MaxRayOriginDepthValue);
        float waveDepthMax = WaveActiveMax(rayOriginDepth);

        if (WaveGetLaneIndex() == 0)
        {
            Store16bitFloatInLow16bitSMem(0, waveDepthMin, SMem::Offset::WaveDepthMin);
            Store16bitFloatInLow16bitSMem(0, waveDepthMax, SMem::Offset::WaveDepthMax);
        }
    }
    GroupMemoryBarrierWithGroupSync();

    float depthMin = Load16bitFloatFromLow16bitSMem(0, SMem::Offset::WaveDepthMin);
    float depthMax = Load16bitFloatFromLow16bitSMem(0, SMem::Offset::WaveDepthMax);

    return float2(depthMin, depthMax);
}

// Combine the depth hash key with the ray direction hash keys and update the key histograms.
void FinalizeHashKeyAndCalculateKeyHistogram(in uint GI, in uint2 Gid, in float2 rayGroupMinMaxDepth)
{
    // Initalize histogram values to 0.
    for (uint key = GI; key < NUM_KEYS; key += NUM_THREADS)
    {
        Store16bitUintInSMem(key, 0, SMem::Offset::Histogram);
    }
    GroupMemoryBarrierWithGroupSync();

    uint2 RayGroupDim = uint2(SortRays::RayGroup::Width, SortRays::RayGroup::Height);
    uint2 GroupStart = Gid * RayGroupDim;
    uint2 NextGroupStart = GroupStart + RayGroupDim;

    // Trim the Ray Group Dim to valid dims.
    RayGroupDim = min(NextGroupStart, cb.dim) - GroupStart;
    uint NumRays = RayGroupDim.y * RayGroupDim.x;

    for (uint ray = GI; ray < NumRays; ray += NUM_THREADS)
    {
        float rayOriginDepth = Load16bitFloatFromSMem(ray, SMem::Offset::Depth16b);
        bool isRayValid = rayOriginDepth != INVALID_RAY_ORIGIN_DEPTH;
        uint hashKey = INACTIVE_RAY_KEY;

        if (isRayValid)
        {
            uint rayDirectionHashKey = Load8bitUintFromLow16bitSMem(ray, SMem::Offset::Key8b);
            uint2 rayIndex = uint2(ray % SortRays::RayGroup::Width, ray / SortRays::RayGroup::Width);
            hashKey = CreateRayHashKey(rayIndex, rayDirectionHashKey, rayOriginDepth, rayGroupMinMaxDepth);
        }

        // Increase histogram bin count.
        AddTo16bitValueInSMem(hashKey, 1, SMem::Offset::Histogram);

        // Cache the key.
        Store16bitUintInSMem(ray, hashKey, SMem::Offset::Key16b);
    }
    GroupMemoryBarrierWithGroupSync();
}

void GenerateHashKeysAndKeyHistogram(in uint2 Gid, in uint GI, out float2 rayGroupMinMaxDepth)
{
    CalculatePartialRayDirectionHashKeyAndCacheDepth(Gid, GI);
    rayGroupMinMaxDepth = CalculateRayGroupMinMaxDepth(GI, Gid);
    FinalizeHashKeyAndCalculateKeyHistogram(GI, Gid, rayGroupMinMaxDepth);
}

// Prefix sum 
// Assumes power of 2 input size.
// Ref: https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch39.html
void PrefixSum(in uint GI)
{
    uint NumIterations = log2(NUM_KEYS);

    // Up-sweep / reduce phase
    for (uint d = 0; d < NumIterations; d++)
    {
        uint step = pow(2, d + 1);
        uint NumSteps = NUM_KEYS / step;
        for (uint j = GI; j < NumSteps; j += NUM_THREADS)
        {
            uint k = j * step;
            uint i1 = k + step / 2 - 1;
            uint i2 = k + step - 1;

            uint v1 = Load16bitUintFromSMem(i1, SMem::Offset::Histogram);
            uint v2 = Load16bitUintFromSMem(i2, SMem::Offset::Histogram);
            uint sum = v1 + v2;

            Store16bitUintInSMem(i2, sum, SMem::Offset::Histogram);
        }
        GroupMemoryBarrierWithGroupSync();
    }

    // Down-sweep
    Store16bitUintInSMem(NUM_KEYS - 1, 0, SMem::Offset::Histogram);
    GroupMemoryBarrierWithGroupSync();
    for (int p = NumIterations - 1; p >= 0 ; p--)
    {
        uint step = pow(2, p + 1);
        uint NumSteps = NUM_KEYS / step;
        for (uint j = GI; j < NumSteps; j += NUM_THREADS)
        {
            uint k = j * step;
            uint i1 = k + step / 2 - 1;
            uint i2 = k + step - 1;

            uint v1 = Load16bitUintFromSMem(i1, SMem::Offset::Histogram);
            uint v2 = Load16bitUintFromSMem(i2, SMem::Offset::Histogram);

            uint sum = v1 + v2;

            Store16bitUintInSMem(i1, v2, SMem::Offset::Histogram);
            Store16bitUintInSMem(i2, sum, SMem::Offset::Histogram);
        }
        GroupMemoryBarrierWithGroupSync();
    }
}


// Flattens a [128,128] ray index into a 13 bit flat index.
// Preserves inactive ray bit in the 8th bit of the y coordinate in the outputs 15th bit.
uint FlattenRayIndex(in uint2 index)
{
    return index.x + (index.y << 7);
}

// Expands a 14 bit flat index into a [128,128] ray index.
// Preserves inactive ray 15th bit information of the flat index in the 8th bit of the y coordinate.
uint2 UnflattenRayIndex(in uint index)
{
    return uint2(index & 0x7F, index >> 7);
}

// Write the sorted indices to shared memory to avoid costly scatter writes to VRAM.
// Later, these are linearly spilled from shared memory to VRAM.
void ScatterWriteSortedIndicesToSharedMemory(in uint2 Gid, in uint GI, in float2 rayGroupMinMaxDepth)
{
    uint2 RayGroupDim = uint2(SortRays::RayGroup::Width, SortRays::RayGroup::Height);
    uint2 GroupStart = Gid * RayGroupDim;
    uint2 NextGroupStart = GroupStart + RayGroupDim;

    // Trim the Ray Group Dim to valid dims.
    RayGroupDim = min(NextGroupStart, cb.dim) - GroupStart;
    uint NumRays = RayGroupDim.y * RayGroupDim.x;

    for (uint ray = GI; ray < NumRays; ray += NUM_THREADS)
    {
        uint2 rayIndex = uint2(ray % RayGroupDim.x, ray / RayGroupDim.x);
        uint2 pixel = GroupStart + rayIndex;

        // Get the key for the corresponding pixel.
        uint key;
        bool isRayValid;

        // First, see if the cached key is stil valid.
        uint cacheValue = Load16bitUintFromSMem(ray, SMem::Offset::Key16b);
        bool isHashKeyEntry = !(cacheValue & INVALID_16BIT_KEY_BIT);

        if (isHashKeyEntry)
        {
            isRayValid = cacheValue != INACTIVE_RAY_KEY;
            key = cacheValue;
        }
        else  // The cached key has been already replaced with the ray's source index. Regenerate the key.
        {
            float2 encodedRayDirection;
            float rayOriginDepth;
            UnpackEncodedNormalDepth(g_inRayDirectionOriginDepth[pixel], encodedRayDirection, rayOriginDepth);
            isRayValid = rayOriginDepth != INVALID_RAY_ORIGIN_DEPTH;

            if (isRayValid)
            {
                key = CreateRayHashKey(rayIndex, encodedRayDirection, rayOriginDepth, rayGroupMinMaxDepth);
            }
            else
            {
                key = INACTIVE_RAY_KEY;
            }
        }
            
        uint index = AddTo16bitValueInSMem(key, 1, SMem::Offset::Histogram);

        // To avoid costly scatter writes to VRAM, cache indices into SMem here instead.            
        uint encodedRayIndex = FlattenRayIndex(rayIndex);
        encodedRayIndex |= isRayValid ? 0 : INACTIVE_RAY_INDEX_BIT;
        encodedRayIndex |= INVALID_16BIT_KEY_BIT;     // Denote the target cache entry doesn't store a key no more.
        Store16bitUintInSMem(index, encodedRayIndex, SMem::Offset::RayIndex);
    }
    GroupMemoryBarrierWithGroupSync();
}


// Spill cached sorted indices to VRAM
// Also scatter write the inversion of the indices.
// The inverted table will be used to find sorted ray index given a ray index and 
// instead of doing expensive scatter write after ray tracing, the subsequent pass
// reading ray tracing results will do gather read.
void SpillCachedIndicesToVRAMAndCacheInvertedSortedIndices(in uint2 Gid, in uint GI)
{
    uint2 RayGroupDim = uint2(SortRays::RayGroup::Width, SortRays::RayGroup::Height);
    uint2 GroupStart = Gid * RayGroupDim;
    uint2 NextGroupStart = GroupStart + RayGroupDim;

    // Trim the Ray Group Dim to valid dims.
    RayGroupDim = min(NextGroupStart, cb.dim) - GroupStart;
    uint NumRays = RayGroupDim.y * RayGroupDim.x;

    // Sequentially spill cached source indices into VRAM.
    // Cache sorted indices for each source index in the Shared Memory.
    for (uint index = GI; index < NumRays; index += NUM_THREADS)
    {
        uint packedSrcIndex = Load16bitUintFromSMem(index, SMem::Offset::RayIndex);
        bool isActiveRay = !(packedSrcIndex & INACTIVE_RAY_INDEX_BIT);

        // Strip the invalid tag bit.
        packedSrcIndex &= ~INVALID_16BIT_KEY_BIT;

        uint2 sortedIndex = uint2(index % RayGroupDim.x, index / RayGroupDim.x);
        uint2 outPixel = GroupStart + sortedIndex;
        g_outSortedToSourceRayIndexOffset[outPixel] = UnflattenRayIndex(packedSrcIndex);   // Output the source index for this sorted ray index.
    }
}

[numthreads(SortRays::ThreadGroup::Width, SortRays::ThreadGroup::Height, 1)]
void main(uint2 Gid : SV_GroupID, uint2 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
    InitializeSharedMemory(GI);

    float2 rayGroupMinMaxDepth;
    GenerateHashKeysAndKeyHistogram(Gid, GI, rayGroupMinMaxDepth);

    PrefixSum(GI);

    ScatterWriteSortedIndicesToSharedMemory(Gid, GI, rayGroupMinMaxDepth);

    SpillCachedIndicesToVRAMAndCacheInvertedSortedIndices(Gid, GI);
}
