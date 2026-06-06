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
    float fov = 60.0f;
};

struct alignas(16) InstanceData
{
    DirectX::XMFLOAT4X4 world;
    DirectX::XMFLOAT4X4 prevWorld;
    UINT materialId;
};

class Scene
{
public:
    CameraState camera;
    std::vector<InstanceData> instances;
    const GltfMeshData* mesh = nullptr;
};

} // namespace Engine
