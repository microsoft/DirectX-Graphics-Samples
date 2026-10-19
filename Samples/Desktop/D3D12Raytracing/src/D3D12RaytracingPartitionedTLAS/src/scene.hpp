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

// Adapted from vk_partitioned_tlas/src/partitioned_tlas_scene.cpp.
// The Hilbert paths, spline sampling and static scenery retain their original attribution.
// See THIRD_PARTY_NOTICES.md for source revisions and acknowledgments.

// Procedural domino paths and static scenery.
#pragma once
#include "SceneMath.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <vector>

namespace scene
{

inline void rotateHilbert(int side, int& x, int& y, int rx, int ry)
{
    if (ry == 0)
    {
        if (rx == 1)
        {
            x = side - 1 - x;
            y = side - 1 - y;
        }
        std::swap(x, y);
    }
}

inline Vector2 hilbertPoint(float t, int level)
{
    const int dimension = 1 << level;
    int distance = static_cast<int>(t * dimension * dimension);
    int x = 0;
    int y = 0;
    for (int side = 1; side < dimension; side *= 2)
    {
        const int rx = (distance / 2) % 2;
        const int ry = (distance ^ rx) % 2;
        rotateHilbert(side, x, y, rx, ry);
        x += side * rx;
        y += side * ry;
        distance /= 4;
    }
    return Vector2(static_cast<float>(x) / dimension, static_cast<float>(y) / dimension);
}

inline std::vector<Vector3> generateDominoPositions(float sceneSize)
{
    const uint32_t controlPointCount = static_cast<uint32_t>(sceneSize * 4.0f);
    std::vector<Vector3> controls;
    controls.reserve(controlPointCount);
    for (uint32_t index = 0; index < controlPointCount; ++index)
    {
        const float t = (static_cast<float>(index) + 0.5f) / controlPointCount;
        const Vector2 position = (hilbertPoint(t, 8) * 2.0f - Vector2(1.0f)) * sceneSize;
        controls.emplace_back(position.x, position.y, 0.0f);
    }

    std::vector<Vector3> positions;
    for (uint32_t index = 0; index + 1 < controlPointCount; ++index)
    {
        const int i0 = std::max(0, static_cast<int>(index) - 1);
        const int i1 = static_cast<int>(index);
        const int i2 = static_cast<int>(index) + 1;
        const int i3 = std::min(static_cast<int>(controlPointCount) - 1, static_cast<int>(index) + 2);
        const float distance = scene::Length(controls[i1] - controls[i2]);
        const uint32_t samples = static_cast<uint32_t>(std::ceil(distance / 0.8f));
        const float step = 1.0f / samples;
        for (uint32_t sample = 0; sample < samples; ++sample)
        {
            const float t = static_cast<float>(sample) * step;
            positions.push_back(scene::CatmullRom(controls[i0], controls[i1], controls[i2], controls[i3], t));
        }
    }
    return positions;
}

inline Matrix objectTransform(const Vector3& position, const Vector3& scale, float yaw = 0.0f)
{
    const float cosine = std::cos(yaw);
    Matrix transform = InstanceTransform(position, scale, std::sin(yaw), cosine);
    // Y-axis component of the axis-angle rotation.
    transform._22 = (cosine + (1.0f - cosine)) * scale.y;
    return transform;
}

// Emit static objects in a fixed order with a repeatable random sequence.
template <typename Emit>
uint32_t generateStaticObjects(const std::vector<Vector3>& dominoes, float sceneSize, uint32_t partitionsPerAxis,
                               uint32_t targetCount, Emit&& emit)
{
    const float partitionSize = 2.0f * sceneSize / partitionsPerAxis;
    float tileSize = 3.0f;
    const float tileCountPerPartition = std::floor(partitionSize / tileSize);
    tileSize += ((partitionSize / tileSize - tileCountPerPartition) * tileSize) / tileCountPerPartition;
    const uint32_t gridSide = static_cast<uint32_t>(1.0f + 2.0f * sceneSize / tileSize);
    std::vector<uint8_t> occupied(static_cast<size_t>(gridSide) * gridSide, 0);
    auto inBounds = [&](int x, int y) {
        return x >= 0 && y >= 0 && x < static_cast<int>(gridSide) && y < static_cast<int>(gridSide);
    };
    auto isOccupied = [&](int x, int y) {
        return !inBounds(x, y) || occupied[static_cast<size_t>(y) * gridSide + x] != 0;
    };
    auto setOccupied = [&](int x, int y) {
        if (inBounds(x, y))
            occupied[static_cast<size_t>(y) * gridSide + x] = 1;
    };
    auto gridCoordinate = [&](float position) {
        const float normalized = std::clamp(0.5f + 0.5f * position / sceneSize, 0.0f, 1.0f);
        return static_cast<int>(normalized * gridSide);
    };

    struct ArchEndpoint
    {
        uint32_t x0, y0, x1, y1;
    };
    std::vector<ArchEndpoint> arches;
    arches.reserve(dominoes.size() / 10);
    for (size_t index = 0; index < dominoes.size(); ++index)
    {
        const Vector3 position = dominoes[index];
        const int gridX = gridCoordinate(position.x);
        const int gridY = gridCoordinate(position.y);
        for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx)
                setOccupied(gridX + dx, gridY + dy);
        if (index % 10 == 0)
        {
            const Vector2 tangent = index == 0 ? Vector2(dominoes[1].x - dominoes[0].x, dominoes[1].y - dominoes[0].y)
                                               : Vector2(dominoes[index].x - dominoes[index - 1].x,
                                                         dominoes[index].y - dominoes[index - 1].y);
            if (scene::Length(tangent) > 0.0001f)
            {
                const Vector2 normal = scene::Normalize(Vector2(tangent.y, -tangent.x));
                const uint32_t gx = static_cast<uint32_t>(gridX);
                const uint32_t gy = static_cast<uint32_t>(gridY);
                arches.push_back(
                    {gx - static_cast<uint32_t>(2.5f * normal.x), gy - static_cast<uint32_t>(2.5f * normal.y),
                     gx + static_cast<uint32_t>(2.5f * normal.x), gy + static_cast<uint32_t>(2.5f * normal.y)});
            }
        }
    }

    uint32_t emitted = 0;
    auto push = [&](const Matrix& transform, uint32_t kind) {
        if (emitted < targetCount)
        {
            emit(transform, kind);
            ++emitted;
        }
    };
    for (uint32_t y = 0; y < partitionsPerAxis; ++y)
        for (uint32_t x = 0; x < partitionsPerAxis; ++x)
        {
            const Vector3 position{-sceneSize + (x + 0.5f) * partitionSize, -0.1f,
                                   -sceneSize + (y + 0.5f) * partitionSize};
            push(objectTransform(position, Vector3(1.0f)), 0);
        }

    std::mt19937 random(0x4e565054u);
    std::uniform_int_distribution<std::mt19937::result_type> gridDistribution(0, gridSide - 1);
    std::uniform_real_distribution<float> unit(0.0f, 1.0f);
    const uint32_t decorationCount = gridSide * gridSide / 100;
    for (uint32_t index = 0; index < decorationCount; ++index)
    {
        uint32_t x = 0;
        uint32_t y = 0;
        while (isOccupied(static_cast<int>(x), static_cast<int>(y)))
        {
            x = gridDistribution(random);
            y = gridDistribution(random);
        }
        const float worldX = ((2.0f * (x + 0.4f * (unit(random) - 0.5f)) / gridSide) - 1.0f) * sceneSize;
        const float worldZ = ((2.0f * (y + 0.4f * (unit(random) - 0.5f)) / gridSide) - 1.0f) * sceneSize;
        setOccupied(x, y);
        const float height = 0.2f + 3.0f * unit(random);
        push(objectTransform(Vector3(worldX, height * 0.5f, worldZ), Vector3(height), unit(random) * DirectX::XM_PI),
             1);
    }

    for (uint32_t y = 0; y < gridSide; ++y)
        for (uint32_t x = 0; x < gridSide; ++x)
        {
            const bool nearDomino = isOccupied(static_cast<int>(x), static_cast<int>(y));
            const float worldX = ((2.0f * (x + 0.01f * unit(random)) / gridSide) - 1.0f) * sceneSize;
            const float worldZ = ((2.0f * (y + 0.01f * unit(random)) / gridSide) - 1.0f) * sceneSize;
            // Consume six random samples per tile before its optional height
            // sample. Keeping the sequence fixed makes layouts repeatable.
            for (uint32_t sample = 0; sample < 4; ++sample)
                (void)unit(random);
            const float height = nearDomino ? 1.0f : 1.0f + 5.0f * unit(random);
            // Tiles remain aligned to the ground grid.
            push(objectTransform(Vector3(worldX, -0.1f, worldZ), Vector3(1.0f, height, 1.0f)), 2);
        }

    const float cellWidth = sceneSize / gridSide;
    const float archObjectSize = cellWidth / 3.0f;
    const uint32_t archLineCount = static_cast<uint32_t>(cellWidth / archObjectSize);
    for (const ArchEndpoint& arch : arches)
    {
        if (emitted >= targetCount)
            break;
        if (isOccupied(static_cast<int>(arch.x0), static_cast<int>(arch.y0)) ||
            isOccupied(static_cast<int>(arch.x1), static_cast<int>(arch.y1)))
            continue;
        setOccupied(arch.x0, arch.y0);
        setOccupied(arch.x1, arch.y1);
        const Vector2 first =
            (2.0f * Vector2(float(arch.x0), float(arch.y0)) / Vector2(static_cast<float>(gridSide)) - Vector2(1.0f)) *
            sceneSize;
        const Vector2 second =
            (2.0f * Vector2(float(arch.x1), float(arch.y1)) / Vector2(static_cast<float>(gridSide)) - Vector2(1.0f)) *
            sceneSize;
        const float archLength = scene::Length(second - first);
        const Vector2 direction = scene::Normalize(second - first);
        const Vector2 normal(direction.y, -direction.x);
        const uint32_t sampleCount = static_cast<uint32_t>(archLength / archObjectSize);
        for (uint32_t sample = 0; sample < sampleCount && emitted < targetCount; ++sample)
        {
            const float ratio = static_cast<float>(sample + 1) / sampleCount;
            const float lift = std::sin(ratio * DirectX::XM_PI * 0.5f) * 3.0f;
            for (uint32_t line = 0; line < archLineCount && emitted < targetCount; ++line)
            {
                const float lineRatio = static_cast<float>(line + 1) / archLineCount;
                const Vector2 side = 2.0f * (lineRatio - 0.5f) * lift * normal;
                const Vector2 offset(line * archObjectSize - 0.5f * cellWidth);
                const Vector2 left = first + offset + direction * 0.5f * archLength * ratio + side;
                const Vector2 right = second + offset - direction * 0.5f * archLength * ratio + side;
                const float yaw = ratio * DirectX::XM_PI * 0.5f;
                push(objectTransform(Vector3(left.x, 0.1f + lift, left.y), Vector3(archObjectSize), yaw), 3);
                push(objectTransform(Vector3(right.x, 0.1f + lift, right.y), Vector3(archObjectSize), yaw), 3);
            }
        }
    }

    return emitted;
}

} // namespace scene
