// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT License (MIT).

#include "ProceduralGeometry.h"
#include <array>
#include <cstdio>
#include <cstring>
#include <map>
#include <stdexcept>
#include <utility>

using Position = std::array<uint32_t, 3>;
using Edge = std::pair<Position, Position>;

void Require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

Position Bits(const ProceduralGeometry::float3& position)
{
    Position result;
    static_assert(sizeof(result) == sizeof(position));
    std::memcpy(result.data(), &position, sizeof(position));
    return result;
}

void CheckSlab(int tilesU, int tilesV, int quadsU, int quadsV)
{
    const auto mesh = ProceduralGeometry::GeneratePlaneSpatialTiles(
        3.5f, 2.9f, tilesU, tilesV, quadsU, quadsV, 17, 0.28f);
    const auto topCount = static_cast<size_t>(tilesU * tilesV);
    Require(mesh.clusters.size() == 2 * topCount + 2 * (tilesU + tilesV),
            "Unexpected slab cluster count");
    std::map<Edge, std::pair<unsigned, int>> edges;
    unsigned triangles = 0, vertices = 0;
    for (size_t c = 0; c < mesh.clusters.size(); ++c)
    {
        const auto& cluster = mesh.clusters[c];
        Require(cluster.positions.size() <= 256 && cluster.indices.size() / 3 <= 256,
                "Cluster exceeds CLAS limits");
        Require(cluster.positions.size() == cluster.normals.size(), "Missing face normals");
        Require(cluster.clusterID == 17 + c, "Cluster IDs changed");
        const auto top = cluster.gridU * tilesV + cluster.gridV;
        Require(cluster.matchedColorCid == mesh.clusters[top].clusterID,
                "Slab face lost its matching top-tile material");
        vertices += static_cast<unsigned>(cluster.positions.size());
        for (size_t i = 0; i < cluster.indices.size(); i += 3)
        {
            Position triangle[3];
            for (unsigned corner = 0; corner < 3; ++corner)
            {
                const auto index = cluster.indices[i + corner];
                Require(index < cluster.positions.size(), "Triangle index out of bounds");
                triangle[corner] = Bits(cluster.positions[index]);
            }
            const auto& a = cluster.positions[cluster.indices[i]];
            const auto& b = cluster.positions[cluster.indices[i + 1]];
            const auto& d = cluster.positions[cluster.indices[i + 2]];
            const auto& n = cluster.normals[cluster.indices[i]];
            const float ux = b.x - a.x, uy = b.y - a.y, uz = b.z - a.z;
            const float vx = d.x - a.x, vy = d.y - a.y, vz = d.z - a.z;
            Require((uy * vz - uz * vy) * n.x + (uz * vx - ux * vz) * n.y +
                    (ux * vy - uy * vx) * n.z > 0, "Incorrect triangle winding");
            for (unsigned corner = 0; corner < 3; ++corner)
            {
                auto first = triangle[corner], second = triangle[(corner + 1) % 3];
                const int direction = first < second ? 1 : -1;
                if (second < first)
                    std::swap(first, second);
                auto& usage = edges[{first, second}];
                ++usage.first;
                usage.second += direction;
            }
            ++triangles;
        }
    }
    Require(triangles == mesh.totalTriangles && vertices == mesh.totalVertices,
            "Mesh totals do not match emitted geometry");
    for (const auto& edge : edges)
        Require(edge.second.first == 2 && edge.second.second == 0,
                "Slab has a crack, T-junction, or inconsistent shared-edge winding");

    // Exact duplicate endpoints necessarily stay equal for every float-truncation level.
    for (unsigned truncate = 0; truncate <= 23; ++truncate)
    {
        const uint32_t mask = ~((1u << truncate) - 1u);
        std::map<Edge, unsigned> quantized;
        for (const auto& edge : edges)
        {
            Edge key = edge.first;
            for (auto& component : key.first)
                component &= mask;
            for (auto& component : key.second)
                component &= mask;
            quantized[key] += edge.second.first;
        }
        for (const auto& edge : quantized)
            Require(edge.second % 2 == 0, "Quantization separated a shared edge");
    }
}

int main()
{
    try
    {
        CheckSlab(6, 6, 4, 4);
        CheckSlab(3, 5, 2, 7);
        CheckSlab(1, 1, 1, 1);
        const auto plane = ProceduralGeometry::GeneratePlaneSpatialTiles(3.5f, 2.9f, 3, 5, 2, 7);
        Require(plane.clusters.size() == 15 && plane.totalTriangles == 420,
                "Zero-thickness plane behavior changed");
        std::puts("Slab seams, winding, CLAS limits, materials, and float truncation: PASS");
        return 0;
    }
    catch (const std::exception& error)
    {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
