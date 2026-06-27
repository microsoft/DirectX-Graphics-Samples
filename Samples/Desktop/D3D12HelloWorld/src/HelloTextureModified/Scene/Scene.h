#pragma once

#include "../DXSampleHelper.h"
#include "../GltfLoader.h"

#include <vector>

namespace Engine
{

struct CameraState
{
    DirectX::XMFLOAT3 pos = {0.0f, 0.0f, -5.0f};
    DirectX::XMFLOAT3 rot = {0.0f, 0.0f, 0.0f};
    DirectX::XMFLOAT3 gazePoint = {0.0f, 0.0f, 0.0f};
    float fov = 60.0f;
};

struct alignas(16) InstanceData
{
    DirectX::XMFLOAT4X4 world;
    DirectX::XMFLOAT4X4 prevWorld;
    UINT materialId;
};

using SceneVertex = GltfVertex;

struct SceneTexture
{
    int width = 0;
    int height = 0;
    int component = 0;
    std::vector<unsigned char> pixels;
};

struct SceneMaterial
{
    int albedoTexIndex = -1;
    int metallicRoughnessTexIndex = -1;
    int emissiveTexIndex = -1;
    int occlusionTexIndex = -1;
    int normalTexIndex = -1;
    float roughnessFactor = 1.0f;
    float metallicFactor = 1.0f;
    float occlusionStrength = 1.0f;
    float ambientOcclusionFactor = 1.0f;
    float emissiveScale = 1.0f;
};

struct SceneMesh
{
    std::vector<SceneVertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<SceneMaterial> materials;
    int materialIndex = 0;
    std::vector<SceneTexture> textures;
};

class Scene
{
public:
    CameraState camera;
    std::vector<InstanceData> instances;
    const SceneMesh* mesh = nullptr;
};

} // namespace Engine
