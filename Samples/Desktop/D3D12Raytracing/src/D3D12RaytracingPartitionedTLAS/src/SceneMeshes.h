//*********************************************************
//
// Copyright (c) 2022-2025, NVIDIA CORPORATION. All rights reserved.
// Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// Adapted from nvpro_core2/nvutils/primitives.cpp.
// Cube, plane, tetrahedron and icosahedron implementations retain their NVIDIA origin.
// See THIRD_PARTY_NOTICES.md for source revisions and acknowledgments.

// Procedural meshes with positions, normals, texture coordinates, and indices.
#pragma once
#include "SceneMath.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace scene_meshes
{
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

struct Mesh
{
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> texcoords;
    std::vector<uint32_t> indices;
};

inline void addVertex(Mesh& mesh, Vector3 p, Vector3 n, Vector2 uv)
{
    mesh.positions.push_back(p);
    mesh.normals.push_back(n);
    mesh.texcoords.push_back(uv);
}

inline void addFacetedTriangle(Mesh& mesh, Vector3 a, Vector3 b, Vector3 c)
{
    const Vector3 normal = scene::Normalize(scene::Cross(scene::Normalize(b - a), scene::Normalize(c - a)));
    const uint32_t first = static_cast<uint32_t>(mesh.positions.size());
    for (const Vector3 p : {a, b, c})
    {
        const Vector3 radial = scene::Normalize(p);
        const Vector2 uv(0.5f + std::atan2(radial.z, radial.x) / (2.0f * DirectX::XM_PI),
                         0.5f - std::asin(radial.y) / DirectX::XM_PI);
        addVertex(mesh, p, normal, uv);
    }
    mesh.indices.insert(mesh.indices.end(), {first, first + 1, first + 2});
}

inline Mesh cube(float width = 1.0f, float height = 1.0f, float depth = 1.0f)
{
    Mesh mesh;
    const Vector3 s(width * 0.5f, height * 0.5f, depth * 0.5f);
    const std::array<Vector3, 8> p = {{{-s.x, -s.y, -s.z},
                                       {-s.x, -s.y, s.z},
                                       {-s.x, s.y, -s.z},
                                       {-s.x, s.y, s.z},
                                       {s.x, -s.y, -s.z},
                                       {s.x, -s.y, s.z},
                                       {s.x, s.y, -s.z},
                                       {s.x, s.y, s.z}}};
    const std::array<Vector3, 6> n = {{{-1, 0, 0}, {0, 0, 1}, {1, 0, 0}, {0, 0, -1}, {0, -1, 0}, {0, 1, 0}}};
    const std::array<Vector2, 4> uv = {{{0, 0}, {0, 1}, {1, 1}, {1, 0}}};
    const std::array<std::array<uint32_t, 4>, 6> faces = {
        {{0, 1, 3, 2}, {1, 5, 7, 3}, {5, 4, 6, 7}, {4, 0, 2, 6}, {4, 5, 1, 0}, {2, 3, 7, 6}}};
    for (uint32_t face = 0; face < faces.size(); ++face)
    {
        const uint32_t first = static_cast<uint32_t>(mesh.positions.size());
        for (uint32_t corner = 0; corner < 4; ++corner)
            addVertex(mesh, p[faces[face][corner]], n[face], uv[corner]);
        mesh.indices.insert(mesh.indices.end(), {first, first + 1, first + 2, first, first + 2, first + 3});
    }
    return mesh;
}

inline Mesh plane(float width, float depth)
{
    Mesh mesh;
    constexpr uint32_t steps = 10;
    const float increment = 1.0f / float(steps);
    for (uint32_t z = 0; z <= steps; ++z)
        for (uint32_t x = 0; x <= steps; ++x)
            addVertex(mesh,
                      Vector3((-0.5f + float(x) * increment) * width, 0.0f, (-0.5f + float(z) * increment) * depth),
                      Vector3(0, 1, 0), Vector2(float(x) / steps, float(steps - z) / steps));
    for (uint32_t z = 0; z < steps; ++z)
        for (uint32_t x = 0; x < steps; ++x)
        {
            const uint32_t first = x + z * (steps + 1);
            mesh.indices.insert(mesh.indices.end(),
                                {first, first + steps + 2, first + 1, first, first + steps + 1, first + steps + 2});
        }
    return mesh;
}

inline Mesh icosahedron()
{
    Mesh mesh;
    const float sq5 = std::sqrt(5.0f);
    float a = 2.0f / (1.0f + sq5);
    const float b = std::sqrt((3.0f + sq5) / (1.0f + sq5));
    a /= b;
    const float r = 0.5f;
    const std::array<Vector3, 12> v = {{{0, r * a, r / b},
                                        {0, r * a, -r / b},
                                        {0, -r * a, r / b},
                                        {0, -r * a, -r / b},
                                        {r * a, r / b, 0},
                                        {r * a, -r / b, 0},
                                        {-r * a, r / b, 0},
                                        {-r * a, -r / b, 0},
                                        {r / b, 0, r * a},
                                        {r / b, 0, -r * a},
                                        {-r / b, 0, r * a},
                                        {-r / b, 0, -r * a}}};
    const std::array<std::array<uint32_t, 3>, 20> triangles = {
        {{1, 6, 4},  {0, 4, 6},   {0, 10, 2},  {0, 2, 8}, {1, 9, 3},  {1, 3, 11}, {2, 7, 5},
         {3, 5, 7},  {6, 11, 10}, {7, 10, 11}, {4, 8, 9}, {5, 9, 8},  {0, 6, 10}, {0, 8, 4},
         {1, 11, 6}, {1, 4, 9},   {3, 7, 11},  {3, 9, 5}, {2, 10, 7}, {2, 5, 8}}};
    for (const auto& t : triangles)
        addFacetedTriangle(mesh, v[t[0]], v[t[1]], v[t[2]]);
    return mesh;
}

inline Mesh tetrahedron()
{
    Mesh mesh;
    const float a = 1.0f / 3.0f;
    const float b = std::sqrt(8.0f / 9.0f);
    const float c = std::sqrt(2.0f / 9.0f);
    const float d = std::sqrt(2.0f / 3.0f);
    const std::array<Vector3, 4> v = {
        {{0, 0.5f, 0}, {-c * 0.5f, -a * 0.5f, d * 0.5f}, {-c * 0.5f, -a * 0.5f, -d * 0.5f}, {b * 0.5f, -a * 0.5f, 0}}};
    for (const auto& t : std::array<std::array<uint32_t, 3>, 4>{{{0, 2, 1}, {0, 3, 2}, {0, 1, 3}, {3, 1, 2}}})
        addFacetedTriangle(mesh, v[t[0]], v[t[1]], v[t[2]]);
    return mesh;
}

} // namespace scene_meshes
