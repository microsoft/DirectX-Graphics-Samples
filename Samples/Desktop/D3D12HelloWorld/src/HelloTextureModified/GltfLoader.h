// GltfLoader.h
#pragma once

#include <DirectXMath.h>
#include <string>
#include <vector>

struct GltfVertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 uv;
    DirectX::XMFLOAT3 normal;
};

struct GltfMeshData
{
    std::vector<GltfVertex> vertices;
    std::vector<uint32_t> indices;
};

bool LoadGltfMesh(const std::string &path, GltfMeshData &outMesh);
