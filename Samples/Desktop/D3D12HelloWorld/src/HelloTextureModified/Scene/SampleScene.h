#pragma once

#include "Scene.h"

#include <DirectXMath.h>

namespace Engine
{

struct SampleSceneUpdateContext
{
    bool isPlaying = false;
    float meshScale = 1.0f;
    DirectX::XMFLOAT4 dragRotation = {0.0f, 0.0f, 0.0f, 1.0f};
};

struct GltfAssetDesc
{
    const char* name;
    const char* path;       // nullptr if asset is not available
    float cameraDistance;   // negative = behind camera along -Z
    float meshScale;
};

class SampleScene
{
public:
    virtual ~SampleScene() = default;

    virtual const char* Name() const = 0;
    virtual bool Available() const { return true; }
    virtual void Load() = 0;
    virtual void Reset() = 0;
    virtual void Update(float deltaTime, const SampleSceneUpdateContext& context) = 0;
    virtual Scene& GetScene() = 0;
    virtual const Scene& GetScene() const = 0;
    virtual SceneMesh& GetMesh() = 0;
    virtual const SceneMesh& GetMesh() const = 0;
    virtual int DisplayInstanceCount() const = 0;
    virtual int MaxDisplayInstanceCount() const = 0;
    virtual void SetDisplayInstanceCount(int count) = 0;
    virtual float DefaultMeshScale() const = 0;
};

class GltfGridBenchmarkScene : public SampleScene
{
public:
    static constexpr int kMaxInstanceCount = 1000;

    explicit GltfGridBenchmarkScene(const GltfAssetDesc& assetDesc, int maxInstanceCount = kMaxInstanceCount);

    const char* Name() const override;
    bool Available() const override { return m_assetDesc.path != nullptr; }
    void Load() override;
    void Reset() override;
    void Update(float deltaTime, const SampleSceneUpdateContext& context) override;
    Scene& GetScene() override;
    const Scene& GetScene() const override;
    SceneMesh& GetMesh() override;
    const SceneMesh& GetMesh() const override;
    int DisplayInstanceCount() const override;
    int MaxDisplayInstanceCount() const override;
    void SetDisplayInstanceCount(int count) override;
    float DefaultMeshScale() const override;

private:
    struct InstanceDataForCPU
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 rot;
    };

    static DirectX::XMFLOAT3 InstanceIdToXYZ(int instanceId);

    void InitInstanceData();

    GltfAssetDesc m_assetDesc = {};
    int m_maxInstanceCount = 0;
    int m_displayInstanceCount = 0;
    float m_accumTime = 0.0f;
    Scene m_scene;
    SceneMesh m_mesh;
    std::vector<InstanceDataForCPU> m_instanceDataForCPU;
};

class GltfObjectViewerScene : public SampleScene
{
public:
    static constexpr int kMaxInstanceCount = 1;

    explicit GltfObjectViewerScene(const GltfAssetDesc& assetDesc);

    const char* Name() const override;
    bool Available() const override { return m_assetDesc.path != nullptr; }
    void Load() override;
    void Reset() override;
    void Update(float deltaTime, const SampleSceneUpdateContext& context) override;
    Scene& GetScene() override;
    const Scene& GetScene() const override;
    SceneMesh& GetMesh() override;
    const SceneMesh& GetMesh() const override;
    int DisplayInstanceCount() const override;
    int MaxDisplayInstanceCount() const override;
    void SetDisplayInstanceCount(int count) override;
    float DefaultMeshScale() const override;

private:
    GltfAssetDesc m_assetDesc = {};
    Scene m_scene;
    SceneMesh m_mesh;
};

class MetallicRoughnessSphereScene : public SampleScene
{
public:
    static constexpr int kMaxInstanceCount = 49;

    explicit MetallicRoughnessSphereScene(int maxInstanceCount = kMaxInstanceCount);

    const char* Name() const override;
    void Load() override;
    void Reset() override;
    void Update(float deltaTime, const SampleSceneUpdateContext& context) override;
    Scene& GetScene() override;
    const Scene& GetScene() const override;
    SceneMesh& GetMesh() override;
    const SceneMesh& GetMesh() const override;
    int DisplayInstanceCount() const override;
    int MaxDisplayInstanceCount() const override;
    void SetDisplayInstanceCount(int count) override;
    float DefaultMeshScale() const override;

private:
    void CreateSphereMesh();
    void CreateMaterialArray(int neutralTextureIndex, int blackTextureIndex);
    void CreateInstances(const SampleSceneUpdateContext& context);

    int m_maxInstanceCount = 0;
    int m_displayInstanceCount = 0;
    Scene m_scene;
    SceneMesh m_mesh;
};

class ShadowTestGroundCubesScene : public SampleScene
{
public:
    static constexpr int kMaxInstanceCount = 1;

    explicit ShadowTestGroundCubesScene(int maxInstanceCount = kMaxInstanceCount);

    const char* Name() const override;
    void Load() override;
    void Reset() override;
    void Update(float deltaTime, const SampleSceneUpdateContext& context) override;
    Scene& GetScene() override;
    const Scene& GetScene() const override;
    SceneMesh& GetMesh() override;
    const SceneMesh& GetMesh() const override;
    int DisplayInstanceCount() const override;
    int MaxDisplayInstanceCount() const override;
    void SetDisplayInstanceCount(int count) override;
    float DefaultMeshScale() const override;

private:
    Scene m_scene;
    SceneMesh m_mesh;
    int m_maxInstanceCount = 0;
};

class AnimatedShadowGridScene : public SampleScene
{
public:
    static constexpr int kMaxInstanceCount = 64;

    explicit AnimatedShadowGridScene(int maxInstanceCount = kMaxInstanceCount);

    const char* Name() const override;
    void Load() override;
    void Reset() override;
    void Update(float deltaTime, const SampleSceneUpdateContext& context) override;
    Scene& GetScene() override;
    const Scene& GetScene() const override;
    SceneMesh& GetMesh() override;
    const SceneMesh& GetMesh() const override;
    int DisplayInstanceCount() const override;
    int MaxDisplayInstanceCount() const override;
    void SetDisplayInstanceCount(int count) override;
    float DefaultMeshScale() const override;

private:
    void InitInstances();
    void UpdateAnimations(float deltaTime, const SampleSceneUpdateContext& context);

    Scene m_scene;
    SceneMesh m_mesh;
    int m_maxInstanceCount = 0;
    int m_displayInstanceCount = 0;
    float m_accumTime = 0.0f;
    struct InstanceAnimData
    {
        float phase;
        float rotSpeed;
    };
    std::vector<InstanceAnimData> m_animData;
};

class ContactShadowTestScene : public SampleScene
{
public:
    static constexpr int kMaxInstanceCount = 1;

    explicit ContactShadowTestScene(int maxInstanceCount = kMaxInstanceCount);

    const char* Name() const override;
    void Load() override;
    void Reset() override;
    void Update(float deltaTime, const SampleSceneUpdateContext& context) override;
    Scene& GetScene() override;
    const Scene& GetScene() const override;
    SceneMesh& GetMesh() override;
    const SceneMesh& GetMesh() const override;
    int DisplayInstanceCount() const override;
    int MaxDisplayInstanceCount() const override;
    void SetDisplayInstanceCount(int count) override;
    float DefaultMeshScale() const override;

private:
    Scene m_scene;
    SceneMesh m_mesh;
    int m_maxInstanceCount = 0;
};

class OccluderWallTestScene : public SampleScene
{
public:
    static constexpr int kMaxInstanceCount = 1;

    explicit OccluderWallTestScene(int maxInstanceCount = kMaxInstanceCount);

    const char* Name() const override;
    void Load() override;
    void Reset() override;
    void Update(float deltaTime, const SampleSceneUpdateContext& context) override;
    Scene& GetScene() override;
    const Scene& GetScene() const override;
    SceneMesh& GetMesh() override;
    const SceneMesh& GetMesh() const override;
    int DisplayInstanceCount() const override;
    int MaxDisplayInstanceCount() const override;
    void SetDisplayInstanceCount(int count) override;
    float DefaultMeshScale() const override;

private:
    Scene m_scene;
    SceneMesh m_mesh;
    int m_maxInstanceCount = 0;
};

} // namespace Engine
