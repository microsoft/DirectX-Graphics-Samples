#include "stdafx.h"

#include "SampleScene.h"

#include "../GltfLoader.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <DirectXMathVector.inl>
#include <Windows.h>
#include <cstdint>
#include <cstdlib>
#include <utility>
#include <vector>
#include "Scene.h"

using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;
using DirectX::XMLoadFloat3;
using DirectX::XMLoadFloat4;
using DirectX::XMMATRIX;
using DirectX::XMMatrixIdentity;
using DirectX::XMMatrixRotationQuaternion;
using DirectX::XMMatrixRotationRollPitchYaw;
using DirectX::XMMatrixScaling;
using DirectX::XMMatrixTranslation;
using DirectX::XMMatrixTranspose;
using DirectX::XMStoreFloat3;
using DirectX::XMStoreFloat4x4;
using DirectX::XMVector3Transform;
using DirectX::XMVector3TransformNormal;

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

int AddSolidTexture(SceneMesh& mesh, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    SceneTexture texture = {};
    texture.width = 1;
    texture.height = 1;
    texture.component = 4;
    texture.pixels = {r, g, b, a};
    mesh.textures.push_back(std::move(texture));
    return static_cast<int>(mesh.textures.size() - 1);
}

float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

SceneMesh CreatePlaneMesh(float sizeX, float sizeZ)
{
    SceneMesh mesh = {};
    const float hx = sizeX * 0.5f;
    const float hz = sizeZ * 0.5f;

    mesh.vertices.resize(4);
    mesh.vertices[0] = {{-hx, 0.0f, -hz}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}};
    mesh.vertices[1] = {{hx, 0.0f, -hz}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}};
    mesh.vertices[2] = {{hx, 0.0f, hz}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};
    mesh.vertices[3] = {{-hx, 0.0f, hz}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};

    mesh.indices = {0, 2, 1, 0, 3, 2};
    return mesh;
}

SceneMesh CreateCubeMesh(float size)
{
    SceneMesh mesh = {};
    const float h = size * 0.5f;

    mesh.vertices.resize(24);
    mesh.indices.resize(36);

    uint32_t vi = 0;
    uint32_t ii = 0;

    const auto addFace = [&](const XMFLOAT3& normal,
                             const XMFLOAT3& v0,
                             const XMFLOAT3& v1,
                             const XMFLOAT3& v2,
                             const XMFLOAT3& v3,
                             bool flipWinding = false)
    {
        const uint32_t base = vi;
        mesh.vertices[vi++] = {v0, {0.0f, 1.0f}, normal};
        mesh.vertices[vi++] = {v1, {1.0f, 1.0f}, normal};
        mesh.vertices[vi++] = {v2, {1.0f, 0.0f}, normal};
        mesh.vertices[vi++] = {v3, {0.0f, 0.0f}, normal};

        if (flipWinding)
        {
            mesh.indices[ii++] = base + 0;
            mesh.indices[ii++] = base + 1;
            mesh.indices[ii++] = base + 2;
            mesh.indices[ii++] = base + 0;
            mesh.indices[ii++] = base + 2;
            mesh.indices[ii++] = base + 3;
        }
        else
        {
            mesh.indices[ii++] = base + 0;
            mesh.indices[ii++] = base + 2;
            mesh.indices[ii++] = base + 1;
            mesh.indices[ii++] = base + 0;
            mesh.indices[ii++] = base + 3;
            mesh.indices[ii++] = base + 2;
        }
    };

    addFace({1.0f, 0.0f, 0.0f}, {h, -h, -h}, {h, -h, h}, {h, h, h}, {h, h, -h});
    addFace({-1.0f, 0.0f, 0.0f}, {-h, -h, h}, {-h, -h, -h}, {-h, h, -h}, {-h, h, h});
    addFace({0.0f, 1.0f, 0.0f}, {-h, h, -h}, {h, h, -h}, {h, h, h}, {-h, h, h});
    addFace({0.0f, -1.0f, 0.0f}, {-h, -h, h}, {h, -h, h}, {h, -h, -h}, {-h, -h, -h});
    addFace({0.0f, 0.0f, 1.0f}, {-h, -h, h}, {h, -h, h}, {h, h, h}, {-h, h, h}, true);
    addFace({0.0f, 0.0f, -1.0f}, {h, -h, -h}, {-h, -h, -h}, {-h, h, -h}, {h, h, -h}, true);

    return mesh;
}

SceneMesh CreateSphereMesh(float radius, int stackCount, int sliceCount)
{
    SceneMesh mesh = {};

    const int vertexCount = (stackCount + 1) * (sliceCount + 1);
    mesh.vertices.resize(vertexCount);

    int vi = 0;
    for (int stack = 0; stack <= stackCount; stack++)
    {
        const float v = static_cast<float>(stack) / static_cast<float>(stackCount);
        const float theta = v * kPI;
        const float sinTheta = std::sin(theta);
        const float cosTheta = std::cos(theta);

        for (int slice = 0; slice <= sliceCount; slice++)
        {
            const float u = static_cast<float>(slice) / static_cast<float>(sliceCount);
            const float phi = u * 2.0f * kPI;
            const float sinPhi = std::sin(phi);
            const float cosPhi = std::cos(phi);

            const XMFLOAT3 normal = {sinTheta * cosPhi, cosTheta, sinTheta * sinPhi};
            const XMFLOAT3 position = {normal.x * radius, normal.y * radius, normal.z * radius};
            mesh.vertices[vi] = {position, XMFLOAT2(u, v), normal};
            vi++;
        }
    }

    const int stride = sliceCount + 1;
    for (int stack = 0; stack < stackCount; stack++)
    {
        for (int slice = 0; slice < sliceCount; slice++)
        {
            const uint32_t a = static_cast<uint32_t>(stack * stride + slice);
            const uint32_t b = static_cast<uint32_t>((stack + 1) * stride + slice);
            const uint32_t c = static_cast<uint32_t>((stack + 1) * stride + slice + 1);
            const uint32_t d = static_cast<uint32_t>(stack * stride + slice + 1);

            mesh.indices.push_back(a);
            mesh.indices.push_back(d);
            mesh.indices.push_back(b);
            mesh.indices.push_back(d);
            mesh.indices.push_back(c);
            mesh.indices.push_back(b);
        }
    }

    return mesh;
}

void AppendMesh(SceneMesh& dest, const SceneMesh& src)
{
    const uint32_t baseVertex = static_cast<uint32_t>(dest.vertices.size());
    dest.vertices.insert(dest.vertices.end(), src.vertices.begin(), src.vertices.end());
    for (uint32_t idx : src.indices)
    {
        dest.indices.push_back(baseVertex + idx);
    }
}

void AppendTransformedMesh(SceneMesh& dest, const SceneMesh& src, DirectX::FXMMATRIX transform)
{
    const uint32_t baseVertex = static_cast<uint32_t>(dest.vertices.size());
    const size_t srcVertexCount = src.vertices.size();
    dest.vertices.reserve(dest.vertices.size() + srcVertexCount);

    // Build normal transform matrix (invert-transpose of rotation/scale part)
    DirectX::XMMATRIX normalMatrix = transform;
    normalMatrix.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    DirectX::XMVECTOR det;
    normalMatrix = DirectX::XMMatrixInverse(&det, normalMatrix);
    if (DirectX::XMVectorGetX(det) != 0.0f)
    {
        normalMatrix = DirectX::XMMatrixTranspose(normalMatrix);
    }
    else
    {
        normalMatrix = DirectX::XMMatrixIdentity();
    }

    for (size_t i = 0; i < srcVertexCount; i++)
    {
        SceneVertex v = src.vertices[i];

        DirectX::XMVECTOR pos = XMLoadFloat3(&v.position);
        DirectX::XMVECTOR nrm = XMLoadFloat3(&v.normal);

        pos = XMVector3Transform(pos, transform);
        nrm = XMVector3TransformNormal(nrm, normalMatrix);
        nrm = DirectX::XMVector3Normalize(nrm);

        XMStoreFloat3(&v.position, pos);
        XMStoreFloat3(&v.normal, nrm);

        dest.vertices.push_back(v);
    }

    for (uint32_t idx : src.indices)
    {
        dest.indices.push_back(baseVertex + idx);
    }
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
        const XMMATRIX dragRotMat = XMMatrixRotationQuaternion(XMLoadFloat4(&context.dragRotation));
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

        std::sort(result.begin(),
                  result.end(),
                  [](const GridPoint& a, const GridPoint& b)
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
    const int neutralTextureIndex = AddSolidTexture(m_mesh, 255, 255, 255, 255);
    const int blackTextureIndex = AddSolidTexture(m_mesh, 0, 0, 0, 255);
    CreateMaterialArray(neutralTextureIndex, blackTextureIndex);
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
    m_mesh.textures.clear();
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
            m_mesh.indices.push_back(d);
            m_mesh.indices.push_back(b);
            m_mesh.indices.push_back(d);
            m_mesh.indices.push_back(c);
            m_mesh.indices.push_back(b);
        }
    }
}

void MetallicRoughnessSphereScene::CreateMaterialArray(int neutralTextureIndex, int blackTextureIndex)
{
    m_mesh.materials.clear();
    m_mesh.materials.reserve(kSphereRows * kSphereColumns);
    for (int row = 0; row < kSphereRows; row++)
    {
        const float roughness = Lerp(0.04f, 1.0f, static_cast<float>(row) / static_cast<float>(kSphereRows - 1));
        for (int column = 0; column < kSphereColumns; column++)
        {
            const float metallic = static_cast<float>(column) / static_cast<float>(kSphereColumns - 1);

            SceneMaterial material = {};
            material.albedoTexIndex = neutralTextureIndex;
            material.metallicRoughnessTexIndex = neutralTextureIndex;
            material.emissiveTexIndex = blackTextureIndex;
            material.occlusionTexIndex = neutralTextureIndex;
            material.normalTexIndex = -1;
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
    const XMMATRIX dragRotMat = XMMatrixRotationQuaternion(XMLoadFloat4(&context.dragRotation));

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

ShadowTestGroundCubesScene::ShadowTestGroundCubesScene(int maxInstanceCount) : m_maxInstanceCount(maxInstanceCount) {}

const char* ShadowTestGroundCubesScene::Name() const
{
    return "Shadow Test: Ground + Cubes";
}

void ShadowTestGroundCubesScene::Load()
{
    const SceneMesh floorMesh = CreatePlaneMesh(10.0f, 10.0f);
    const SceneMesh cubeMesh = CreateCubeMesh(0.5f);

    m_mesh = floorMesh;
    {
        const XMMATRIX t = XMMatrixTranslation(-2.0f, 0.25f, -1.5f);
        AppendTransformedMesh(m_mesh, cubeMesh, t);
    }
    {
        const XMMATRIX t = XMMatrixTranslation(1.5f, 0.25f, 1.5f);
        AppendTransformedMesh(m_mesh, cubeMesh, t);
    }
    {
        const XMMATRIX t = XMMatrixTranslation(-1.0f, 0.25f, 2.5f);
        AppendTransformedMesh(m_mesh, cubeMesh, t);
    }
    {
        const XMMATRIX t = XMMatrixTranslation(2.5f, 0.25f, -2.0f);
        AppendTransformedMesh(m_mesh, cubeMesh, t);
    }
    {
        const XMMATRIX t = XMMatrixTranslation(0.0f, 0.25f, 0.0f);
        AppendTransformedMesh(m_mesh, cubeMesh, t);
    }

    const int whiteTex = AddSolidTexture(m_mesh, 255, 255, 255, 255);
    SceneMaterial mat = {};
    mat.albedoTexIndex = whiteTex;
    mat.metallicRoughnessTexIndex = whiteTex;
    mat.emissiveTexIndex = AddSolidTexture(m_mesh, 0, 0, 0, 255);
    mat.occlusionTexIndex = whiteTex;
    mat.normalTexIndex = -1;
    mat.roughnessFactor = 0.6f;
    mat.metallicFactor = 0.0f;
    mat.occlusionStrength = 1.0f;
    mat.ambientOcclusionFactor = 1.0f;
    mat.emissiveScale = 0.0f;
    m_mesh.materials.push_back(mat);

    m_mesh.materialIndex = 0;

    m_scene.mesh = &m_mesh;
    Reset();
}

void ShadowTestGroundCubesScene::Reset()
{
    m_scene.camera.pos = {0.0f, 4.0f, -8.0f};
    m_scene.camera.rot = {0.0f, 0.0f, 0.0f};
    m_scene.camera.fov = 60.0f;

    m_scene.instances.resize(1);
    XMStoreFloat4x4(&m_scene.instances[0].world, XMMatrixTranspose(XMMatrixIdentity()));
    m_scene.instances[0].prevWorld = m_scene.instances[0].world;
    m_scene.instances[0].materialId = 0;
}

void ShadowTestGroundCubesScene::Update(float deltaTime, const SampleSceneUpdateContext& context)
{
    UNREFERENCED_PARAMETER(deltaTime);
    UNREFERENCED_PARAMETER(context);
}

Scene& ShadowTestGroundCubesScene::GetScene()
{
    return m_scene;
}

const Scene& ShadowTestGroundCubesScene::GetScene() const
{
    return m_scene;
}

SceneMesh& ShadowTestGroundCubesScene::GetMesh()
{
    return m_mesh;
}

const SceneMesh& ShadowTestGroundCubesScene::GetMesh() const
{
    return m_mesh;
}

int ShadowTestGroundCubesScene::DisplayInstanceCount() const
{
    return 1;
}

int ShadowTestGroundCubesScene::MaxDisplayInstanceCount() const
{
    return 1;
}

void ShadowTestGroundCubesScene::SetDisplayInstanceCount(int count)
{
    UNREFERENCED_PARAMETER(count);
}

float ShadowTestGroundCubesScene::DefaultMeshScale() const
{
    return 1.0f;
}

AnimatedShadowGridScene::AnimatedShadowGridScene(int maxInstanceCount)
    : m_maxInstanceCount(maxInstanceCount), m_displayInstanceCount(maxInstanceCount)
{
}

const char* AnimatedShadowGridScene::Name() const
{
    return "Animated Shadow Grid";
}

void AnimatedShadowGridScene::Load()
{
    m_mesh = CreateCubeMesh(1.0f);

    const int whiteTex = AddSolidTexture(m_mesh, 255, 255, 255, 255);
    const int blackTex = AddSolidTexture(m_mesh, 0, 0, 0, 255);
    const int grayTex = AddSolidTexture(m_mesh, 160, 160, 160, 255);

    // Material 0: floor (gray, rough, non-metallic)
    {
        SceneMaterial mat = {};
        mat.albedoTexIndex = grayTex;
        mat.metallicRoughnessTexIndex = grayTex;
        mat.emissiveTexIndex = blackTex;
        mat.occlusionTexIndex = grayTex;
        mat.normalTexIndex = -1;
        mat.roughnessFactor = 0.8f;
        mat.metallicFactor = 0.0f;
        mat.occlusionStrength = 1.0f;
        mat.ambientOcclusionFactor = 1.0f;
        mat.emissiveScale = 0.0f;
        m_mesh.materials.push_back(mat);
    }
    // Material 1: cube (white, slightly metallic)
    {
        SceneMaterial mat = {};
        mat.albedoTexIndex = whiteTex;
        mat.metallicRoughnessTexIndex = whiteTex;
        mat.emissiveTexIndex = blackTex;
        mat.occlusionTexIndex = whiteTex;
        mat.normalTexIndex = -1;
        mat.roughnessFactor = 0.5f;
        mat.metallicFactor = 0.2f;
        mat.occlusionStrength = 1.0f;
        mat.ambientOcclusionFactor = 1.0f;
        mat.emissiveScale = 0.0f;
        m_mesh.materials.push_back(mat);
    }

    m_mesh.materialIndex = 0;
    m_scene.mesh = &m_mesh;
    Reset();
}

void AnimatedShadowGridScene::Reset()
{
    m_scene.camera.pos = {0.0f, 3.0f, -8.0f};
    m_scene.camera.rot = {0.0f, 0.0f, 0.0f};
    m_scene.camera.fov = 60.0f;
    m_accumTime = 0.0f;
    InitInstances();
}

void AnimatedShadowGridScene::InitInstances()
{
    m_scene.instances.resize(m_maxInstanceCount);
    m_animData.resize(m_maxInstanceCount);

    // Instance 0: static receiver floor, using the same unit cube mesh as the animated cubes.
    {
        const XMMATRIX scaleMat = XMMatrixScaling(10.0f, 0.05f, 10.0f);
        const XMMATRIX transMat = XMMatrixTranslation(0.0f, -0.025f, 0.0f);
        XMStoreFloat4x4(&m_scene.instances[0].world, XMMatrixTranspose(scaleMat * transMat));
        m_scene.instances[0].prevWorld = m_scene.instances[0].world;
        m_scene.instances[0].materialId = 0;
    }

    // Remaining instances: animated cubes
    const int cubeCount = m_maxInstanceCount - 1;
    const int gridDim = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(cubeCount))));
    const float spacing = 1.5f;
    const float offset = static_cast<float>(gridDim - 1) * spacing * 0.5f;

    for (int i = 0; i < cubeCount; i++)
    {
        const int idx = i + 1;
        const int row = i / gridDim;
        const int col = i % gridDim;
        const float x = static_cast<float>(col) * spacing - offset;
        const float z = static_cast<float>(row) * spacing - offset;

        const float cubeScale = DefaultMeshScale();
        const XMMATRIX scaleMat = XMMatrixScaling(cubeScale, cubeScale, cubeScale);
        const XMMATRIX transMat = XMMatrixTranslation(x, cubeScale * 0.5f, z);
        XMStoreFloat4x4(&m_scene.instances[idx].world, XMMatrixTranspose(scaleMat * transMat));
        m_scene.instances[idx].prevWorld = m_scene.instances[idx].world;
        m_scene.instances[idx].materialId = 1;

        m_animData[idx].phase = static_cast<float>(i) / static_cast<float>(cubeCount) * 2.0f * kPI;
        m_animData[idx].rotSpeed = 0.5f + static_cast<float>(i % 5) * 0.25f;
    }
}

void AnimatedShadowGridScene::Update(float deltaTime, const SampleSceneUpdateContext& context)
{
    if (context.isPlaying)
    {
        m_accumTime += deltaTime;
    }
    UpdateAnimations(deltaTime, context);
}

void AnimatedShadowGridScene::UpdateAnimations(float deltaTime, const SampleSceneUpdateContext& context)
{
    const int cubeCount = m_maxInstanceCount - 1;
    const int gridDim = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(cubeCount))));
    const float spacing = 1.5f;
    const float offset = static_cast<float>(gridDim - 1) * spacing * 0.5f;

    for (int i = 0; i < m_maxInstanceCount; i++)
    {
        m_scene.instances[i].prevWorld = m_scene.instances[i].world;

        if (i == 0)
        {
            continue;
        }

        const int cubeIdx = i - 1;
        const int row = cubeIdx / gridDim;
        const int col = cubeIdx % gridDim;
        const float baseX = static_cast<float>(col) * spacing - offset;
        const float baseZ = static_cast<float>(row) * spacing - offset;

        const float animOffset = std::sin(m_accumTime * m_animData[i].rotSpeed + m_animData[i].phase) * 0.3f;
        const float rotY = m_accumTime * m_animData[i].rotSpeed + m_animData[i].phase;
        const float bounce = std::abs(std::sin(m_accumTime * 2.0f + m_animData[i].phase)) * 0.2f;

        const XMMATRIX scaleMat = XMMatrixScaling(context.meshScale, context.meshScale, context.meshScale);
        const XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(rotY * 0.5f, rotY, rotY * 0.3f);
        const XMMATRIX transMat =
            XMMatrixTranslation(baseX + animOffset, context.meshScale * 0.5f + bounce, baseZ + animOffset * 0.5f);
        const XMMATRIX dragRotMat = XMMatrixRotationQuaternion(XMLoadFloat4(&context.dragRotation));
        XMStoreFloat4x4(&m_scene.instances[i].world, XMMatrixTranspose(scaleMat * rotMat * dragRotMat * transMat));
    }
}

Scene& AnimatedShadowGridScene::GetScene()
{
    return m_scene;
}

const Scene& AnimatedShadowGridScene::GetScene() const
{
    return m_scene;
}

SceneMesh& AnimatedShadowGridScene::GetMesh()
{
    return m_mesh;
}

const SceneMesh& AnimatedShadowGridScene::GetMesh() const
{
    return m_mesh;
}

int AnimatedShadowGridScene::DisplayInstanceCount() const
{
    return m_displayInstanceCount;
}

int AnimatedShadowGridScene::MaxDisplayInstanceCount() const
{
    return m_maxInstanceCount;
}

void AnimatedShadowGridScene::SetDisplayInstanceCount(int count)
{
    m_displayInstanceCount = std::clamp(count, 0, m_maxInstanceCount);
}

float AnimatedShadowGridScene::DefaultMeshScale() const
{
    return 0.6f;
}

ContactShadowTestScene::ContactShadowTestScene(int maxInstanceCount) : m_maxInstanceCount(maxInstanceCount) {}

const char* ContactShadowTestScene::Name() const
{
    return "Contact Shadow Test";
}

void ContactShadowTestScene::Load()
{
    const SceneMesh floorMesh = CreatePlaneMesh(6.0f, 6.0f);
    const SceneMesh sphereMesh = CreateSphereMesh(0.5f, 16, 32);

    m_mesh = floorMesh;
    {
        const XMMATRIX t = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
        AppendTransformedMesh(m_mesh, sphereMesh, t);
    }

    const int whiteTex = AddSolidTexture(m_mesh, 255, 255, 255, 255);
    const int grayTex = AddSolidTexture(m_mesh, 180, 180, 180, 255);
    const int blackTex = AddSolidTexture(m_mesh, 0, 0, 0, 255);

    {
        SceneMaterial mat = {};
        mat.albedoTexIndex = grayTex;
        mat.metallicRoughnessTexIndex = grayTex;
        mat.emissiveTexIndex = blackTex;
        mat.occlusionTexIndex = grayTex;
        mat.normalTexIndex = -1;
        mat.roughnessFactor = 0.8f;
        mat.metallicFactor = 0.0f;
        mat.occlusionStrength = 1.0f;
        mat.ambientOcclusionFactor = 1.0f;
        mat.emissiveScale = 0.0f;
        m_mesh.materials.push_back(mat);
    }
    {
        SceneMaterial mat = {};
        mat.albedoTexIndex = whiteTex;
        mat.metallicRoughnessTexIndex = whiteTex;
        mat.emissiveTexIndex = blackTex;
        mat.occlusionTexIndex = whiteTex;
        mat.normalTexIndex = -1;
        mat.roughnessFactor = 0.3f;
        mat.metallicFactor = 0.7f;
        mat.occlusionStrength = 1.0f;
        mat.ambientOcclusionFactor = 1.0f;
        mat.emissiveScale = 0.0f;
        m_mesh.materials.push_back(mat);
    }

    m_mesh.materialIndex = 0;

    m_scene.mesh = &m_mesh;
    Reset();
}

void ContactShadowTestScene::Reset()
{
    m_scene.camera.pos = {1.0f, 0.8f, -2.5f};
    m_scene.camera.rot = {0.0f, 0.0f, 0.0f};
    m_scene.camera.fov = 50.0f;

    m_scene.instances.resize(1);
    XMStoreFloat4x4(&m_scene.instances[0].world, XMMatrixTranspose(XMMatrixIdentity()));
    m_scene.instances[0].prevWorld = m_scene.instances[0].world;
    m_scene.instances[0].materialId = 0;
}

void ContactShadowTestScene::Update(float deltaTime, const SampleSceneUpdateContext& context)
{
    UNREFERENCED_PARAMETER(deltaTime);
    UNREFERENCED_PARAMETER(context);
}

Scene& ContactShadowTestScene::GetScene()
{
    return m_scene;
}

const Scene& ContactShadowTestScene::GetScene() const
{
    return m_scene;
}

SceneMesh& ContactShadowTestScene::GetMesh()
{
    return m_mesh;
}

const SceneMesh& ContactShadowTestScene::GetMesh() const
{
    return m_mesh;
}

int ContactShadowTestScene::DisplayInstanceCount() const
{
    return 1;
}

int ContactShadowTestScene::MaxDisplayInstanceCount() const
{
    return 1;
}

void ContactShadowTestScene::SetDisplayInstanceCount(int count)
{
    UNREFERENCED_PARAMETER(count);
}

float ContactShadowTestScene::DefaultMeshScale() const
{
    return 1.0f;
}

OccluderWallTestScene::OccluderWallTestScene(int maxInstanceCount) : m_maxInstanceCount(maxInstanceCount) {}

const char* OccluderWallTestScene::Name() const
{
    return "Occluder Wall Test";
}

void OccluderWallTestScene::Load()
{
    const SceneMesh floorMesh = CreatePlaneMesh(10.0f, 10.0f);
    const SceneMesh cubeMesh = CreateCubeMesh(0.5f);
    const SceneMesh wallCubeMesh = CreateCubeMesh(1.0f);
    const SceneMesh sphereMesh = CreateSphereMesh(0.4f, 12, 24);

    m_mesh = floorMesh;
    // Wall: thin depth (0.15), tall (1.5), wide (3.0), centered at (0, 0.75, -0.5)
    {
        const XMMATRIX s = XMMatrixScaling(0.15f, 1.5f, 3.0f);
        const XMMATRIX t = XMMatrixTranslation(0.0f, 0.75f, -0.5f);
        AppendTransformedMesh(m_mesh, wallCubeMesh, s * t);
    }
    // Sphere behind wall
    {
        const XMMATRIX t = XMMatrixTranslation(0.0f, 0.4f, 1.5f);
        AppendTransformedMesh(m_mesh, sphereMesh, t);
    }
    // Cube left of wall
    {
        const XMMATRIX t = XMMatrixTranslation(-1.8f, 0.25f, -1.0f);
        AppendTransformedMesh(m_mesh, cubeMesh, t);
    }
    // Cube right of wall
    {
        const XMMATRIX t = XMMatrixTranslation(1.8f, 0.25f, -1.5f);
        AppendTransformedMesh(m_mesh, cubeMesh, t);
    }

    const int whiteTex = AddSolidTexture(m_mesh, 255, 255, 255, 255);
    const int grayTex = AddSolidTexture(m_mesh, 160, 160, 160, 255);
    const int blackTex = AddSolidTexture(m_mesh, 0, 0, 0, 255);

    {
        SceneMaterial mat = {};
        mat.albedoTexIndex = grayTex;
        mat.metallicRoughnessTexIndex = grayTex;
        mat.emissiveTexIndex = blackTex;
        mat.occlusionTexIndex = grayTex;
        mat.normalTexIndex = -1;
        mat.roughnessFactor = 0.8f;
        mat.metallicFactor = 0.0f;
        mat.occlusionStrength = 1.0f;
        mat.ambientOcclusionFactor = 1.0f;
        mat.emissiveScale = 0.0f;
        m_mesh.materials.push_back(mat);
    }

    m_mesh.materialIndex = 0;
    m_scene.mesh = &m_mesh;
    Reset();
}

void OccluderWallTestScene::Reset()
{
    m_scene.camera.pos = {0.0f, 3.0f, -6.0f};
    m_scene.camera.rot = {0.0f, 0.0f, 0.0f};
    m_scene.camera.fov = 55.0f;

    m_scene.instances.resize(1);
    XMStoreFloat4x4(&m_scene.instances[0].world, XMMatrixTranspose(XMMatrixIdentity()));
    m_scene.instances[0].prevWorld = m_scene.instances[0].world;
    m_scene.instances[0].materialId = 0;
}

void OccluderWallTestScene::Update(float deltaTime, const SampleSceneUpdateContext& context)
{
    UNREFERENCED_PARAMETER(deltaTime);
    UNREFERENCED_PARAMETER(context);
}

Scene& OccluderWallTestScene::GetScene()
{
    return m_scene;
}

const Scene& OccluderWallTestScene::GetScene() const
{
    return m_scene;
}

SceneMesh& OccluderWallTestScene::GetMesh()
{
    return m_mesh;
}

const SceneMesh& OccluderWallTestScene::GetMesh() const
{
    return m_mesh;
}

int OccluderWallTestScene::DisplayInstanceCount() const
{
    return 1;
}

int OccluderWallTestScene::MaxDisplayInstanceCount() const
{
    return 1;
}

void OccluderWallTestScene::SetDisplayInstanceCount(int count)
{
    UNREFERENCED_PARAMETER(count);
}

float OccluderWallTestScene::DefaultMeshScale() const
{
    return 1.0f;
}

} // namespace Engine
