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

struct GltfMaterial
{
    int baseColorTextureIndex = -1;
    float baseColorFactor[4] = {1, 1, 1, 1};
};

struct GltfTextureData
{
    int width = 0;
    int height = 0;
    int component = 0;
    std::vector<unsigned char> pixels; // RGBA8
};

struct GltfMeshData
{
    std::vector<GltfVertex> vertices;
    std::vector<uint32_t> indices;

    std::vector<GltfMaterial> materials; // interim
    int materialIndex = 0;

    std::vector<GltfTextureData> textures; // interim
};

bool LoadGltfMesh(const std::string &path, GltfMeshData &outMesh);
