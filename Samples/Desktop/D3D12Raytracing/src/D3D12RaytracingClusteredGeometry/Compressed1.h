//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT License (MIT).
//
//*********************************************************
//
// Compressed1.h
//
// CPU-side encoder for the DXR2 D3D12_VERTEX_FORMAT_COMPRESSED1 vertex format.
//
// Per-cluster layout in the vertex buffer:
//   - 12-byte D3D12_VERTEX_FORMAT_COMPRESSED1_HEADER
//       (8-bit biased exponent, three 24-bit signed anchors,
//        three 4-bit (bit_count - 1) widths per axis, range [1..16])
//   - immediately followed by per-vertex (x_bits + y_bits + z_bits)-bit
//     packed deltas, LSB-first within each component, no padding between
//     components or between vertices.
//
// Decode:  pos[c] = (anchor[c] + delta[c]) * 2^(exponent - 127)
//
// This implementation mirrors the d3d12conf reference encoder:
//   1. Iterate exponent upward until anchor + max(delta) fits in a 24-bit
//      signed range AND every delta fits in 32 bits.
//   2. Anchor = floor(min(pos) / scale + 0.5). Since the same rounding is
//      used for the per-vertex quantization, anchor = min(quantized set)
//      by construction, so every delta is >= 0 with no separate clamp.
//   3. Per-axis bit count chosen as ceil(log2(maxDelta[axis] + 1)), then
//      clamped to [1, maxPrecisionBits].
//   4. Bitstream packed LSB-first, X then Y then Z per vertex.
//
// All header field manipulation uses the d3d12.h ENCODE_/DECODE_ macros.
//
#pragma once

#include "ProceduralGeometry.h"
#include <vector>
#include <cstdint>
#include <cmath>
#include <cfloat>
#include <algorithm>
#include <d3d12.h>

namespace Compressed1
{
    struct EncodedCluster
    {
        D3D12_VERTEX_FORMAT_COMPRESSED1_HEADER header = {};
        std::vector<uint8_t> bitstream;       // packed deltas (no padding)
        unsigned int          vertexCount   = 0;
        unsigned int          xBits         = 0;
        unsigned int          yBits         = 0;
        unsigned int          zBits         = 0;

        // Total bytes for the whole compressed cluster (header + bitstream).
        size_t TotalBytes() const { return sizeof(header) + bitstream.size(); }
    };

    // Minimum number of bits to represent `v` (>=1; returns 1 for v=0 because the
    // spec requires bit_count in [1..16]).
    inline uint32_t BitsForValue(uint32_t v)
    {
        uint32_t b = 1;
        while ((v >> b) != 0u && b < 32u) ++b;
        return b;
    }

    inline void WriteBitsLSB(std::vector<uint8_t>& dst, uint64_t& bitOff,
                             uint32_t value, uint32_t nBits)
    {
        uint32_t written = 0;
        while (written < nBits)
        {
            const uint64_t bytePos   = bitOff / 8;
            const uint32_t bitInByte = (uint32_t)(bitOff & 7);
            const uint32_t take      = std::min<uint32_t>(8 - bitInByte, nBits - written);
            const uint32_t mask      = (1u << take) - 1u;
            const uint32_t chunk     = (value >> written) & mask;
            dst[(size_t)bytePos] &= (uint8_t)(~(mask << bitInByte));
            dst[(size_t)bytePos] |= (uint8_t)(chunk << bitInByte);
            written += take;
            bitOff  += take;
        }
    }

    inline uint32_t ReadBitsLSB(const std::vector<uint8_t>& src, uint64_t& bitOff, uint32_t nBits)
    {
        uint32_t value = 0, read = 0;
        while (read < nBits)
        {
            const uint64_t bytePos   = bitOff / 8;
            const uint32_t bitInByte = (uint32_t)(bitOff & 7);
            const uint32_t take      = std::min<uint32_t>(8 - bitInByte, nBits - read);
            const uint32_t mask      = (1u << take) - 1u;
            const uint32_t chunk     = (src[(size_t)bytePos] >> bitInByte) & mask;
            value |= (chunk << read);
            read   += take;
            bitOff += take;
        }
        return value;
    }

    // Encode a single cluster.
    //   - `maxPrecisionBits` caps per-axis bit count (1..16 per spec).
    //   - `forcedBiasedExponent` in [1..232]: start the search there and never
    //      decrease (so a scene-wide pick can be enforced for cluster-edge
    //      watertightness). Pass -1 for an unconstrained per-cluster pick.
    inline EncodedCluster Encode(const std::vector<ProceduralGeometry::float3>& verts,
                                 int maxPrecisionBits = 12,
                                 int forcedBiasedExponent = -1)
    {
        EncodedCluster out;
        out.vertexCount = (unsigned int)verts.size();
        if (verts.empty()) return out;

        maxPrecisionBits = std::clamp(maxPrecisionBits, 1, 16);

        // Per-axis min in input units.
        float mn[3] = { FLT_MAX,  FLT_MAX,  FLT_MAX };
        for (auto& v : verts)
        {
            const float p[3] = { v.x, v.y, v.z };
            for (int c = 0; c < 3; ++c) if (p[c] < mn[c]) mn[c] = p[c];
        }

        // Iterate exponent until the encoding fits. `maxDeltaValue` is the
        // cap implied by maxPrecisionBits; deltas exceeding this trigger
        // another exponent bump (giving up some precision for headroom).
        const uint64_t maxDeltaValue = (maxPrecisionBits >= 32) ? ~0ull : ((1ull << maxPrecisionBits) - 1ull);
        int    exponent = (forcedBiasedExponent > 0) ? forcedBiasedExponent : 1;
        int32_t anchor[3] = { 0, 0, 0 };
        std::vector<uint32_t> deltas; deltas.reserve(verts.size() * 3);
        uint32_t maxDelta[3] = { 0, 0, 0 };
        bool ok = false;

        for (;;)
        {
            const double scale    = std::ldexp(1.0, exponent - 127);
            const double invScale = 1.0 / scale;

            // Anchor = floor(min/scale + 0.5). Monotonic with the per-vertex
            // rounding below, so anchor IS the min over quantized values
            // -> all deltas >= 0 without a separate clamp.
            for (int i = 0; i < 3; ++i)
            {
                double a = std::floor((double)mn[i] * invScale + 0.5);
                a = std::clamp(a, -8388608.0, 8388607.0);    // 24-bit signed
                anchor[i] = (int32_t)a;
            }

            deltas.clear();
            maxDelta[0] = maxDelta[1] = maxDelta[2] = 0;
            bool overflow = false;
            for (const auto& v : verts)
            {
                const float p[3] = { v.x, v.y, v.z };
                for (int i = 0; i < 3; ++i)
                {
                    const double qd = std::floor((double)p[i] * invScale + 0.5);
                    const uint64_t d = (uint64_t)(int64_t)qd - (uint64_t)anchor[i];
                    if (d > 0xFFFFFFFFull) { overflow = true; }
                    if (d > maxDeltaValue) { overflow = true; }
                    deltas.push_back((uint32_t)d);
                    if ((uint32_t)d > maxDelta[i]) maxDelta[i] = (uint32_t)d;
                }
            }

            if (!overflow)
            {
                bool anchorOk = true;
                for (int i = 0; i < 3; ++i)
                {
                    if ((int64_t)anchor[i] + (int64_t)maxDelta[i] > 8388607ll) { anchorOk = false; break; }
                }
                if (anchorOk) { ok = true; break; }
            }
            if (++exponent > 232) { ok = false; break; }      // saturate
        }
        (void)ok;

        // Per-axis bit counts, each at least 1, capped at maxPrecisionBits.
        const uint32_t bitsNeeded[3] = {
            std::clamp<uint32_t>(BitsForValue(maxDelta[0]), 1u, (uint32_t)maxPrecisionBits),
            std::clamp<uint32_t>(BitsForValue(maxDelta[1]), 1u, (uint32_t)maxPrecisionBits),
            std::clamp<uint32_t>(BitsForValue(maxDelta[2]), 1u, (uint32_t)maxPrecisionBits),
        };
        out.xBits = bitsNeeded[0];
        out.yBits = bitsNeeded[1];
        out.zBits = bitsNeeded[2];

        // Header.
        ENCODE_D3D12_COMPRESSED1(out.header,
                                 (uint32_t)exponent,
                                 anchor[0], anchor[1], anchor[2],
                                 bitsNeeded[0] - 1, bitsNeeded[1] - 1, bitsNeeded[2] - 1);

        // Bitstream: (bitsX + bitsY + bitsZ) bits per vertex, packed LSB-first.
        const uint64_t totalBits = (uint64_t)(bitsNeeded[0] + bitsNeeded[1] + bitsNeeded[2]) * verts.size();
        out.bitstream.assign((size_t)((totalBits + 7) / 8), 0);
        uint64_t bitOff = 0;
        for (size_t v = 0; v < verts.size(); ++v)
        {
            uint32_t dx = deltas[v * 3 + 0];
            uint32_t dy = deltas[v * 3 + 1];
            uint32_t dz = deltas[v * 3 + 2];
            // Defensive clamp; with the loop above this should never fire.
            const uint32_t maskX = (bitsNeeded[0] >= 32) ? ~0u : (1u << bitsNeeded[0]) - 1u;
            const uint32_t maskY = (bitsNeeded[1] >= 32) ? ~0u : (1u << bitsNeeded[1]) - 1u;
            const uint32_t maskZ = (bitsNeeded[2] >= 32) ? ~0u : (1u << bitsNeeded[2]) - 1u;
            if (dx > maskX) dx = maskX;
            if (dy > maskY) dy = maskY;
            if (dz > maskZ) dz = maskZ;
            WriteBitsLSB(out.bitstream, bitOff, dx, bitsNeeded[0]);
            WriteBitsLSB(out.bitstream, bitOff, dy, bitsNeeded[1]);
            WriteBitsLSB(out.bitstream, bitOff, dz, bitsNeeded[2]);
        }
        return out;
    }

    // CPU-side reference decoder for the roundtrip self-check.
    inline std::vector<ProceduralGeometry::float3> Decode(const EncodedCluster& enc)
    {
        const int e  = (int)DECODE_D3D12_COMPRESSED1_EXPONENT(enc.header);
        const int aX = (int)DECODE_D3D12_COMPRESSED1_X_ANCHOR(enc.header);
        const int aY = (int)DECODE_D3D12_COMPRESSED1_Y_ANCHOR(enc.header);
        const int aZ = (int)DECODE_D3D12_COMPRESSED1_Z_ANCHOR(enc.header);
        const uint32_t nx = (uint32_t)DECODE_D3D12_COMPRESSED1_X_BITS(enc.header) + 1u;
        const uint32_t ny = (uint32_t)DECODE_D3D12_COMPRESSED1_Y_BITS(enc.header) + 1u;
        const uint32_t nz = (uint32_t)DECODE_D3D12_COMPRESSED1_Z_BITS(enc.header) + 1u;
        const float unit  = (float)std::ldexp(1.0, e - 127);

        std::vector<ProceduralGeometry::float3> out(enc.vertexCount);
        uint64_t bitOff = 0;
        for (unsigned int i = 0; i < enc.vertexCount; ++i)
        {
            uint32_t dx = ReadBitsLSB(enc.bitstream, bitOff, nx);
            uint32_t dy = ReadBitsLSB(enc.bitstream, bitOff, ny);
            uint32_t dz = ReadBitsLSB(enc.bitstream, bitOff, nz);
            out[i].x = (float)((double)(aX + (int32_t)dx) * unit);
            out[i].y = (float)((double)(aY + (int32_t)dy) * unit);
            out[i].z = (float)((double)(aZ + (int32_t)dz) * unit);
        }
        return out;
    }
}
