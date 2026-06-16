#include "stdafx.h"

#include "SampleScene.h"

#include "../GltfLoader.h"

#include <algorithm>
#include <cassert>
#include <cmath>

using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;
using DirectX::XMMatrixRotationRollPitchYaw;
using DirectX::XMMatrixScaling;
using DirectX::XMMatrixTranslation;
using DirectX::XMMatrixTranspose;
using DirectX::XMStoreFloat4x4;
using DirectX::XMMATRIX;

namespace Engine
{

namespace
{
static constexpr float kTranslationSpeed = 0.005f;
static constexpr float kPI = 3.141592f;
static constexpr float kRotationSpeed = kPI / 180.f / 3.f;
static constexpr float kOffsetBounds = 5.f;
static constexpr int kSphereRows = 7;
static constexpr int kSphereColumns = 7;
static constexpr int kSphereStackCount = 16;
static constexpr int kSphereSliceCount = 32;

GltfMeshData LoadDamagedHelmet()
{
    GltfMeshData mesh;
    const bool loaded = LoadGltfMesh("Assets\\Models\\DamagedHelmet\\glTF\\DamagedHelmet.gltf", mesh);
    assert(loaded);
    return mesh;
}

SceneMesh ConvertToSceneMesh(const GltfMeshData& mesh)
{
    SceneMesh sceneMesh = {};
    sceneMesh.vertices = mesh.vertices;
    sceneMesh.indices = mesh.indices;
    sceneMesh.materialIndex = mesh.materialIndex;

    sceneMesh.materials.reserve(mesh.materials.size());
    for (const GltfMaterial& material : mesh.materials)
    {
        SceneMaterial sceneMaterial = {};
        sceneMaterial.albedoTexIndex = material.albedoTexIndex;
        sceneMaterial.metallicRoughnessTexIndex = material.metallicRoughnessTexIndex;
        sceneMaterial.emissiveTexIndex = material.emissiveTexIndex;
        sceneMaterial.occlusionTexIndex = material.occlusionTexIndex;
        sceneMaterial.normalTexIndex = material.normalTexIndex;
        sceneMaterial.roughnessFactor = material.roughnessFactor;
        sceneMaterial.metallicFactor = material.metallicFactor;
        sceneMaterial.occlusionStrength = material.occlusionStrength;
        sceneMaterial.ambientOcclusionFactor = 1.0f;
        sceneMaterial.emissiveScale = 1.0f;
        sceneMesh.materials.push_back(std::move(sceneMaterial));
    }

    sceneMesh.textures.reserve(mesh.textures.size());
    for (const GltfTextureData& texture : mesh.textures)
    {
        SceneTexture sceneTexture = {};
        sceneTexture.width = texture.width;
        sceneTexture.height = texture.height;
        sceneTexture.component = texture.component;
        sceneTexture.pixels = texture.pixels;
        sceneMesh.textures.push_back(std::move(sceneTexture));
    }

    return sceneMesh;
}

void AddWhiteTexture(SceneMesh& mesh)
{
    SceneTexture texture = {};
    texture.width = 1;
    texture.height = 1;
    texture.component = 4;
    texture.pixels = {255, 255, 255, 255};
    mesh.textures.push_back(std::move(texture));
}

float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

} // namespace

GltfGridScene::GltfGridScene(int maxInstanceCount)
    : m_maxInstanceCount(maxInstanceCount), m_displayInstanceCount(maxInstanceCount)
{
}

const char* GltfGridScene::Name() const
{
    return "glTF Grid";
}

void GltfGridScene::Load()
{
    m_mesh = ConvertToSceneMesh(LoadDamagedHelmet());
    assert(!m_mesh.vertices.empty());
    m_scene.mesh = &m_mesh;
    Reset();
}

void GltfGridScene::Reset()
{
    m_scene.camera.pos = {0.0f, 0.0f, -10.0f};
    m_scene.camera.rot = {0.0f, 0.0f, 0.0f};
    m_scene.camera.fov = 60.0f;
    m_accumTime = 0.0f;
    InitInstanceData();
}

void GltfGridScene::Update(float deltaTime, const SampleSceneUpdateContext& context)
{
    if (context.isPlaying)
    {
        m_accumTime += deltaTime;
        if (m_accumTime > 1.0f)
        {
            m_accumTime = 0.0f;
        }
    }

    for (int i = 0; i < m_maxInstanceCount; i++)
    {
        m_scene.instances[i].prevWorld = m_scene.instances[i].world;
        bool resetMotionVector = false;

        if (context.isPlaying)
        {
            m_instanceDataForCPU[i].pos.x += kTranslationSpeed;
        }
        if (m_instanceDataForCPU[i].pos.x > kOffsetBounds)
        {
            m_instanceDataForCPU[i].pos.x = -kOffsetBounds;
            resetMotionVector = true;
        }
        if (context.isPlaying)
        {
            auto& rot = m_instanceDataForCPU[i].rot;
            rot.x = std::fmod(rot.x + kRotationSpeed, 2.0f * kPI);
            rot.y = std::fmod(rot.y + kRotationSpeed, 2.0f * kPI);
            rot.z = std::fmod(rot.z + kRotationSpeed, 2.0f * kPI);
        }

        const XMMATRIX scaleMat = XMMatrixScaling(context.meshScale, context.meshScale, context.meshScale);
        const XMMATRIX transMat = XMMatrixTranslation(
            m_instanceDataForCPU[i].pos.x, m_instanceDataForCPU[i].pos.y, m_instanceDataForCPU[i].pos.z);
        const XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(
            m_instanceDataForCPU[i].rot.x, m_instanceDataForCPU[i].rot.y, m_instanceDataForCPU[i].rot.z);
        const XMMATRIX dragRotMat =
            XMMatrixRotationRollPitchYaw(context.dragRotation.x, context.dragRotation.y, 0.0f);
        XMStoreFloat4x4(&m_scene.instances[i].world, XMMatrixTranspose(scaleMat * rotMat * dragRotMat * transMat));
        if (resetMotionVector)
        {
            m_scene.instances[i].prevWorld = m_scene.instances[i].world;
        }
    }
}

Scene& GltfGridScene::GetScene()
{
    return m_scene;
}

const Scene& GltfGridScene::GetScene() const
{
    return m_scene;
}

SceneMesh& GltfGridScene::GetMesh()
{
    return m_mesh;
}

const SceneMesh& GltfGridScene::GetMesh() const
{
    return m_mesh;
}

int GltfGridScene::DisplayInstanceCount() const
{
    return m_displayInstanceCount;
}

int GltfGridScene::MaxDisplayInstanceCount() const
{
    return m_maxInstanceCount;
}

void GltfGridScene::SetDisplayInstanceCount(int count)
{
    m_displayInstanceCount = std::clamp(count, 0, m_maxInstanceCount);
}

float GltfGridScene::DefaultMeshScale() const
{
    return 0.5f;
}

void GltfGridScene::InitInstanceData()
{
    m_scene.instances.resize(m_maxInstanceCount);
    m_instanceDataForCPU.clear();
    m_instanceDataForCPU.reserve(m_maxInstanceCount);
    for (int i = 0; i < m_maxInstanceCount; i++)
    {
        const XMFLOAT3 pos = InstanceIdToXYZ(i);
        m_instanceDataForCPU.push_back({pos, XMFLOAT3(0.0f, 0.0f, 0.0f)});

        auto& d = m_scene.instances[i];
        const UINT materialCount = static_cast<UINT>(m_mesh.materials.size());
        d.materialId = materialCount > 0 ? i % materialCount : 0;

        const XMMATRIX scaleMat = XMMatrixScaling(DefaultMeshScale(), DefaultMeshScale(), DefaultMeshScale());
        const XMMATRIX transMat = XMMatrixTranslation(pos.x, pos.y, pos.z);
        XMStoreFloat4x4(&d.world, XMMatrixTranspose(scaleMat * transMat));
        d.prevWorld = d.world;
    }
}

XMFLOAT3 GltfGridScene::InstanceIdToXYZ(int instanceId)
{
    constexpr int dimX = 10;
    constexpr int dimY = 10;
    constexpr int dimZ = 10;
    constexpr int maxInstanceCount = dimX * dimY * dimZ;

    struct GridPoint
    {
        int x;
        int y;
        int z;
        int dist2;
    };

    static const std::vector<GridPoint> points = []
    {
        std::vector<GridPoint> result;
        result.reserve(maxInstanceCount);

        constexpr int halfDimX = dimX / 2;
        constexpr int halfDimY = dimY / 2;
        constexpr int halfDimZ = dimZ / 2;

        constexpr int centerX = 0;
        constexpr int centerY = 0;

        for (int z = -halfDimZ; z <= halfDimZ; z++)
        {
            for (int y = -halfDimY; y <= halfDimY; y++)
            {
                for (int x = -halfDimX; x <= halfDimX; x++)
                {
                    const int dx = x - centerX;
                    const int dy = y - centerY;
                    const int dist2 = dx * dx + dy * dy;
                    result.push_back({x, y, z, dist2});
                }
            }
        }

        std::sort(result.begin(), result.end(), [](const GridPoint& a, const GridPoint& b)
        {
            if (a.z != b.z)
            {
                return a.z < b.z;
            }
            if (a.dist2 != b.dist2)
            {
                return a.dist2 < b.dist2;
            }
            if (a.y != b.y)
            {
                return a.y < b.y;
            }
            return a.x < b.x;
        });

        return result;
    }();

    instanceId = std::clamp(instanceId, 0, maxInstanceCount - 1);
    const GridPoint& p = points[instanceId];
    return {
        static_cast<float>(p.x),
        static_cast<float>(p.y),
        static_cast<float>(p.z),
    };
}

MetallicRoughnessSphereScene::MetallicRoughnessSphereScene(int maxInstanceCount)
    : m_maxInstanceCount(maxInstanceCount), m_displayInstanceCount(kSphereRows * kSphereColumns)
{
}

const char* MetallicRoughnessSphereScene::Name() const
{
    return "Metallic/Roughness Spheres";
}

void MetallicRoughnessSphereScene::Load()
{
    CreateSphereMesh();
    AddWhiteTexture(m_mesh);
    CreateMaterialArray();
    m_scene.mesh = &m_mesh;
    Reset();
}

void MetallicRoughnessSphereScene::Reset()
{
    m_scene.camera.pos = {0.0f, 0.0f, -8.0f};
    m_scene.camera.rot = {0.0f, 0.0f, 0.0f};
    m_scene.camera.fov = 55.0f;

    SampleSceneUpdateContext context = {};
    context.meshScale = DefaultMeshScale();
    CreateInstances(context);
}

void MetallicRoughnessSphereScene::Update(float deltaTime, const SampleSceneUpdateContext& context)
{
    UNREFERENCED_PARAMETER(deltaTime);
    CreateInstances(context);
}

Scene& MetallicRoughnessSphereScene::GetScene()
{
    return m_scene;
}

const Scene& MetallicRoughnessSphereScene::GetScene() const
{
    return m_scene;
}

SceneMesh& MetallicRoughnessSphereScene::GetMesh()
{
    return m_mesh;
}

const SceneMesh& MetallicRoughnessSphereScene::GetMesh() const
{
    return m_mesh;
}

int MetallicRoughnessSphereScene::DisplayInstanceCount() const
{
    return m_displayInstanceCount;
}

int MetallicRoughnessSphereScene::MaxDisplayInstanceCount() const
{
    return kSphereRows * kSphereColumns;
}

void MetallicRoughnessSphereScene::SetDisplayInstanceCount(int count)
{
    m_displayInstanceCount = std::clamp(count, 0, kSphereRows * kSphereColumns);
}

float MetallicRoughnessSphereScene::DefaultMeshScale() const
{
    return 0.85f;
}

void MetallicRoughnessSphereScene::CreateSphereMesh()
{
    m_mesh.vertices.clear();
    m_mesh.indices.clear();
    m_mesh.materialIndex = 0;

    for (int stack = 0; stack <= kSphereStackCount; stack++)
    {
        const float v = static_cast<float>(stack) / static_cast<float>(kSphereStackCount);
        const float theta = v * kPI;
        const float sinTheta = std::sin(theta);
        const float cosTheta = std::cos(theta);

        for (int slice = 0; slice <= kSphereSliceCount; slice++)
        {
            const float u = static_cast<float>(slice) / static_cast<float>(kSphereSliceCount);
            const float phi = u * 2.0f * kPI;
            const float sinPhi = std::sin(phi);
            const float cosPhi = std::cos(phi);

            const XMFLOAT3 normal = {sinTheta * cosPhi, cosTheta, sinTheta * sinPhi};
            const XMFLOAT3 position = {normal.x * 0.5f, normal.y * 0.5f, normal.z * 0.5f};
            m_mesh.vertices.push_back({position, XMFLOAT2(u, v), normal});
        }
    }

    const int stride = kSphereSliceCount + 1;
    for (int stack = 0; stack < kSphereStackCount; stack++)
    {
        for (int slice = 0; slice < kSphereSliceCount; slice++)
        {
            const uint32_t a = static_cast<uint32_t>(stack * stride + slice);
            const uint32_t b = static_cast<uint32_t>((stack + 1) * stride + slice);
            const uint32_t c = static_cast<uint32_t>((stack + 1) * stride + slice + 1);
            const uint32_t d = static_cast<uint32_t>(stack * stride + slice + 1);

            m_mesh.indices.push_back(a);
            m_mesh.indices.push_back(b);
            m_mesh.indices.push_back(d);
            m_mesh.indices.push_back(d);
            m_mesh.indices.push_back(b);
            m_mesh.indices.push_back(c);
        }
    }
}

void MetallicRoughnessSphereScene::CreateMaterialArray()
{
    m_mesh.materials.clear();
    m_mesh.materials.reserve(kSphereRows * kSphereColumns);
    for (int row = 0; row < kSphereRows; row++)
    {
        const float roughness = Lerp(0.04f, 1.0f, static_cast<float>(row) / static_cast<float>(kSphereRows - 1));
        for (int column = 0; column < kSphereColumns; column++)
        {
            const float metallic =
                static_cast<float>(column) / static_cast<float>(kSphereColumns - 1);

            SceneMaterial material = {};
            material.albedoTexIndex = 0;
            material.metallicRoughnessTexIndex = 0;
            material.emissiveTexIndex = 0;
            material.occlusionTexIndex = 0;
            material.normalTexIndex = 0;
            material.roughnessFactor = roughness;
            material.metallicFactor = metallic;
            material.occlusionStrength = 1.0f;
            material.ambientOcclusionFactor = 1.0f;
            material.emissiveScale = 0.0f;
            m_mesh.materials.push_back(material);
        }
    }
}

void MetallicRoughnessSphereScene::CreateInstances(const SampleSceneUpdateContext& context)
{
    m_scene.instances.resize(m_maxInstanceCount);
    const float spacing = 1.25f;
    const float xOffset = static_cast<float>(kSphereColumns - 1) * spacing * 0.5f;
    const float yOffset = static_cast<float>(kSphereRows - 1) * spacing * 0.5f;
    const XMMATRIX dragRotMat = XMMatrixRotationRollPitchYaw(context.dragRotation.x, context.dragRotation.y, 0.0f);

    for (int i = 0; i < m_maxInstanceCount; i++)
    {
        m_scene.instances[i].prevWorld = m_scene.instances[i].world;
        const int visibleIndex = i % (kSphereRows * kSphereColumns);
        const int row = visibleIndex / kSphereColumns;
        const int column = visibleIndex % kSphereColumns;
        const float x = static_cast<float>(column) * spacing - xOffset;
        const float y = yOffset - static_cast<float>(row) * spacing;

        const XMMATRIX scaleMat = XMMatrixScaling(context.meshScale, context.meshScale, context.meshScale);
        const XMMATRIX transMat = XMMatrixTranslation(x, y, 0.0f);
        XMStoreFloat4x4(&m_scene.instances[i].world, XMMatrixTranspose(scaleMat * dragRotMat * transMat));
        m_scene.instances[i].materialId = static_cast<UINT>(visibleIndex);
    }
}

} // namespace Engine
