#include "stdafx.h"

#include "SceneFactory.h"
#include "ProceduralSceneBuilder.h"
#include "SampleScene.h"

#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <cassert>
#include <cstdint>
#include <memory>

using DirectX::XMFLOAT3;
using DirectX::XMStoreFloat4x4;
using DirectX::XMMATRIX;
using DirectX::XMMatrixIdentity;
using DirectX::XMMatrixTranspose;

namespace Engine
{

namespace
{

class CornellBoxScene : public SampleScene
{
public:
    static constexpr int kMaxInstanceCount = 1;

    const char* Name() const override
    {
        return "Cornell Box + Mirror Ball";
    }

    void Load() override
    {
        m_mesh = {};
        m_scene = {};

        // Textures
        const int whiteTex = Procedural::AddSolidColorTexture(m_mesh, 255, 255, 255, 255);
        const int redTex   = Procedural::AddSolidColorTexture(m_mesh, 255, 0, 0, 255);
        const int greenTex = Procedural::AddSolidColorTexture(m_mesh, 0, 255, 0, 255);
        const int blackTex = Procedural::AddSolidColorTexture(m_mesh, 0, 0, 0, 255);

        // Materials
        // 0: Red wall
        m_mesh.materials.push_back(MakeMat(redTex,   whiteTex, blackTex, 0.8f,  0.0f, 0.0f));
        // 1: Green wall
        m_mesh.materials.push_back(MakeMat(greenTex, whiteTex, blackTex, 0.8f,  0.0f, 0.0f));
        // 2: White diffuse (back wall, floor, ceiling)
        m_mesh.materials.push_back(MakeMat(whiteTex, whiteTex, blackTex, 0.8f,  0.0f, 0.0f));
        // 3: Mirror ball (metallic)
        m_mesh.materials.push_back(MakeMat(whiteTex, whiteTex, blackTex, 0.02f, 1.0f, 0.0f));
        // 4: Diffuse sphere
        m_mesh.materials.push_back(MakeMat(whiteTex, whiteTex, blackTex, 0.9f,  0.0f, 0.0f));
        // 5: Emissive panel
        m_mesh.materials.push_back(MakeMat(whiteTex, whiteTex, whiteTex, 0.8f,  0.0f, 1.0f));

        m_mesh.materialIndex = 0;

        // Geometry
        const float halfW = 1.5f;
        const float halfH = 1.5f;
        const float halfD = 1.5f;

        // Left wall (red)
        Procedural::AddQuad(m_mesh,
            XMFLOAT3{-halfW, 0.0f, 0.0f},
            XMFLOAT3{0.0f, 3.0f, 3.0f},
            XMFLOAT3{1.0f, 0.0f, 0.0f}, 0, true);
        // Right wall (green)
        Procedural::AddQuad(m_mesh,
            XMFLOAT3{halfW, 0.0f, 0.0f},
            XMFLOAT3{0.0f, 3.0f, 3.0f},
            XMFLOAT3{-1.0f, 0.0f, 0.0f}, 1);
        // Back wall (white)
        Procedural::AddQuad(m_mesh,
            XMFLOAT3{0.0f, 0.0f, -halfD},
            XMFLOAT3{3.0f, 3.0f, 0.0f},
            XMFLOAT3{0.0f, 0.0f, 1.0f}, 2, true);
        // Floor (white)
        Procedural::AddQuad(m_mesh,
            XMFLOAT3{0.0f, -halfH, 0.0f},
            XMFLOAT3{3.0f, 0.0f, 3.0f},
            XMFLOAT3{0.0f, 1.0f, 0.0f}, 2);
        // Ceiling (white)
        Procedural::AddQuad(m_mesh,
            XMFLOAT3{0.0f, halfH, 0.0f},
            XMFLOAT3{3.0f, 0.0f, 3.0f},
            XMFLOAT3{0.0f, -1.0f, 0.0f}, 2, true);
        // Mirror ball
        Procedural::AddSphere(m_mesh,
            XMFLOAT3{-0.4f, -0.2f, 0.5f}, 0.5f, 3);
        // Diffuse sphere
        Procedural::AddSphere(m_mesh,
            XMFLOAT3{0.6f, -0.9f, 0.0f}, 0.3f, 4);
        // Emissive panel on ceiling
        Procedural::AddQuad(m_mesh,
            XMFLOAT3{0.0f, halfH - 0.02f, 0.0f},
            XMFLOAT3{0.4f, 0.0f, 0.4f},
            XMFLOAT3{0.0f, -1.0f, 0.0f}, 5);

        m_scene.mesh = &m_mesh;
        Reset();
    }

    void Reset() override
    {
        m_scene.camera.pos = {0.0f, 0.0f, 5.0f};
        m_scene.camera.rot = {0.0f, 0.0f, 0.0f};
        m_scene.camera.fov = 50.0f;

        m_scene.instances.resize(1);
        XMStoreFloat4x4(&m_scene.instances[0].world, XMMatrixTranspose(XMMatrixIdentity()));
        m_scene.instances[0].prevWorld = m_scene.instances[0].world;
        m_scene.instances[0].materialId = 0;
    }

    void Update(float /*deltaTime*/, const SampleSceneUpdateContext& /*context*/) override
    {
    }

    Scene& GetScene() override { return m_scene; }
    const Scene& GetScene() const override { return m_scene; }
    SceneMesh& GetMesh() override { return m_mesh; }
    const SceneMesh& GetMesh() const override { return m_mesh; }

    int DisplayInstanceCount() const override { return 1; }
    int MaxDisplayInstanceCount() const override { return kMaxInstanceCount; }

    void SetDisplayInstanceCount(int count) override
    {
        assert(count <= kMaxInstanceCount);
        (void)count;
    }

    float DefaultMeshScale() const override { return 1.0f; }

private:
    static SceneMaterial MakeMat(int albedoTex, int mrTex, int emissiveTex,
                                 float rough, float metal, float emissive)
    {
        SceneMaterial mat = {};
        mat.albedoTexIndex = albedoTex;
        mat.metallicRoughnessTexIndex = mrTex;
        mat.emissiveTexIndex = emissiveTex;
        mat.occlusionTexIndex = albedoTex;
        mat.normalTexIndex = -1;
        mat.roughnessFactor = rough;
        mat.metallicFactor = metal;
        mat.occlusionStrength = 1.0f;
        mat.ambientOcclusionFactor = 1.0f;
        mat.emissiveScale = emissive;
        return mat;
    }

    Scene m_scene;
    SceneMesh m_mesh;
};

} // namespace

std::unique_ptr<SampleScene> SceneFactory::CreateCornellBox()
{
    return std::make_unique<CornellBoxScene>();
}

} // namespace Engine
