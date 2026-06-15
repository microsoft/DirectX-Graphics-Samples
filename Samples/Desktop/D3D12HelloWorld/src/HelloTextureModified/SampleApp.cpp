//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"

#include <algorithm>
#include "CubeMesh.h"
#include "SampleApp.h"
#include "imgui.h"
#include <cmath>

SampleApp::SampleApp(UINT width, UINT height, std::wstring name)
    : DXSample(width, height, name), m_prevTime(std::chrono::steady_clock::now()),
      m_engine(m_graphicsDevice)
{
}

void SampleApp::OnInit()
{
    LoadSceneAssets();
    InitInstanceData(m_sceneMesh);

    GraphicsDeviceDesc deviceDesc = {};
    deviceDesc.hwnd = Win32Application::GetHwnd();
    deviceDesc.swapChainWidth = GetWidth();
    deviceDesc.swapChainHeight = GetHeight();
    deviceDesc.bufferCount = HelloTextureEngine::kSwapChainBufferCount;
    deviceDesc.swapChainFormat = HelloTextureEngine::kSwapChainFormat;
    deviceDesc.useWarpDevice = m_useWarpDevice;
    m_graphicsDevice.Initialize(deviceDesc);

    m_scene.mesh = &m_sceneMesh;
    m_scene.camera.pos = {0.f, 0.f, -10.f};
    m_engine.SetScene(m_scene);

    m_engine.SetDebugUiHandler([this](const HelloTextureEngine::DebugUiContext& context) { DrawDebugUi(context); });
    m_engine.SetUpdateHandler([this]() { UpdateSampleState(); });
    m_engine.SetLightingParams(m_lightingParams);
    m_engine.SetRenderingPath(m_renderingPath);
    m_engine.SetLightingPassDebugGradient(m_lightingPassDebugGradient);
    m_engine.SetBackBufferClearColor(m_backBufferClearColor);
    m_engine.SetDisplayInstanceCount(m_displayInstanceCount);
    m_engine.SetToneMapParams(m_toneMapParams);
    m_engine.SetRenderViewMode(m_renderViewMode);

    m_engine.Initialize(GetWidth(), GetHeight());
}

void SampleApp::UpdateSampleState()
{
    auto now = std::chrono::steady_clock::now();
    const float deltaTime = std::chrono::duration<float>(now - m_prevTime).count();
    m_prevTime = now;

    static constexpr float kCameraMoveSpeed = 0.01f;
    if (GetForegroundWindow() == Win32Application::GetHwnd())
    {
        auto& camera = m_scene.camera;

        if (GetAsyncKeyState('A') & 0x8000)
            camera.pos.x -= kCameraMoveSpeed;
        if (GetAsyncKeyState('D') & 0x8000)
            camera.pos.x += kCameraMoveSpeed;
        if ((GetAsyncKeyState('W') & 0x8000) && (GetAsyncKeyState(VK_CONTROL) & 0x8000))
            camera.pos.y -= kCameraMoveSpeed;
        if ((GetAsyncKeyState('S') & 0x8000) && (GetAsyncKeyState(VK_CONTROL) & 0x8000))
            camera.pos.y += kCameraMoveSpeed;
        if ((GetAsyncKeyState('W') & 0x8000) && !(GetAsyncKeyState(VK_CONTROL) & 0x8000))
            camera.pos.z += kCameraMoveSpeed;
        if ((GetAsyncKeyState('S') & 0x8000) && !(GetAsyncKeyState(VK_CONTROL) & 0x8000))
            camera.pos.z -= kCameraMoveSpeed;
    }

    UpdateInstanceData(deltaTime);

    m_engine.SetScene(m_scene);
    m_engine.SetDisplayInstanceCount(m_displayInstanceCount);
}

void SampleApp::OnKeyDown(UINT8 key)
{
    if (key == VK_SPACE)
    {
        m_isPlaying = !m_isPlaying;
    }
}

void SampleApp::OnKeyUp(UINT8 key) {}

void SampleApp::OnMouseDown(UINT8 button, int x, int y)
{
    if (button == VK_LBUTTON)
    {
        m_isDragging = true;
        m_lastMouseX = x;
        m_lastMouseY = y;
    }
    else if (button == VK_MBUTTON)
    {
        m_isMiddleDragging = true;
        m_lastMouseX = x;
        m_lastMouseY = y;
    }
}

void SampleApp::OnMouseUp(UINT8 button, int x, int y)
{
    if (button == VK_LBUTTON)
    {
        m_isDragging = false;
    }
    else if (button == VK_MBUTTON)
    {
        m_isMiddleDragging = false;
    }
}

void SampleApp::OnMouseMove(int x, int y)
{
    if (m_isDragging)
    {
        const int dx = x - m_lastMouseX;
        const int dy = y - m_lastMouseY;
        m_lastMouseX = x;
        m_lastMouseY = y;
        m_dragRotation.y += static_cast<float>(dx) * kMouseRotationSpeed;
        m_dragRotation.x += static_cast<float>(dy) * kMouseRotationSpeed;
    }
    else if (m_isMiddleDragging)
    {
        const int dx = x - m_lastMouseX;
        const int dy = y - m_lastMouseY;
        m_lastMouseX = x;
        m_lastMouseY = y;

        auto& camera = m_scene.camera;
        camera.pos.x += static_cast<float>(dx) * kMousePanSpeed;
        camera.pos.y -= static_cast<float>(dy) * kMousePanSpeed;
    }
}

void SampleApp::OnMouseWheel(int wheelDelta)
{
    auto& camera = m_scene.camera;
    const float wheelSteps = static_cast<float>(wheelDelta) / static_cast<float>(WHEEL_DELTA);

    if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
    {
        camera.fov = std::clamp(camera.fov - wheelSteps * kMouseWheelFovSpeed, 20.0f, 150.0f);
        return;
    }

    camera.pos.z = std::clamp(camera.pos.z + wheelSteps * kMouseWheelCameraSpeed, kCameraMinZ, kCameraMaxZ);
}

void SampleApp::OnWindowSizeChanged(UINT width, UINT height)
{
    m_engine.RequestResize(width, height);
}

void SampleApp::OnIdle()
{
    m_engine.RunFrame();
}

void SampleApp::OnUpdate()
{
    // Win32Application does not call OnUpdate() in the current DXSample loop.
    // It calls OnIdle(), which is delegated to HelloTextureEngine::RunFrame().
    // The engine invokes UpdateHandler at the appropriate point in its frame.
    //
    // Keep this override for the DXSample interface. If Win32Application starts
    // calling OnUpdate() in the future, SampleApp frame state should still update
    // through the same path used by the engine callback.
    UpdateSampleState(); // currently this is not called.
}

void SampleApp::OnRender()
{
    // Win32Application does not call OnRender() in the current DXSample loop.
    // Rendering is driven by HelloTextureEngine::RunFrame(), which calls the engine's
    // render path after its frame update.
    // Keep this override for the DXSample interface. If Win32Application starts
    // calling OnRender() in the future, delegate to the engine render path.
    m_engine.RenderFrame(); // currently this is not called.
}

void SampleApp::OnDestroy()
{
    m_engine.Shutdown();
}

void SampleApp::LoadSceneAssets()
{
    switch (kMeshSource)
    {
        case MeshSource::Gltf:
            m_sceneMesh = ConvertToSceneMesh(LoadGltfScene());
            break;
        case MeshSource::Cube:
            m_sceneMesh = ConvertToSceneMesh(CreateCubeMesh());
            break;
    }

    assert(!m_sceneMesh.vertices.empty());
}

GltfMeshData SampleApp::LoadGltfScene() const
{
    GltfMeshData mesh;
    const bool loaded = LoadGltfMesh("Assets\\Models\\DamagedHelmet\\glTF\\DamagedHelmet.gltf", mesh);
    assert(loaded);
    return mesh;
}

Engine::SceneMesh SampleApp::ConvertToSceneMesh(const GltfMeshData& mesh)
{
    Engine::SceneMesh sceneMesh = {};
    sceneMesh.vertices = mesh.vertices;
    sceneMesh.indices = mesh.indices;
    sceneMesh.materialIndex = mesh.materialIndex;

    sceneMesh.materials.reserve(mesh.materials.size());
    for (const GltfMaterial& material : mesh.materials)
    {
        Engine::SceneMaterial sceneMaterial = {};
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
        Engine::SceneTexture sceneTexture = {};
        sceneTexture.width = texture.width;
        sceneTexture.height = texture.height;
        sceneTexture.component = texture.component;
        sceneTexture.pixels = texture.pixels;
        sceneMesh.textures.push_back(std::move(sceneTexture));
    }

    return sceneMesh;
}

void SampleApp::DrawDebugUi(const HelloTextureEngine::DebugUiContext& context)
{
    using RenderingPath = HelloTextureEngine::RenderingPath;
    using RenderViewMode = HelloTextureEngine::RenderViewMode;

    ImGui::SetNextWindowSize(ImVec2(400, 140), ImGuiCond_FirstUseEver);
    ImGui::Begin("Debug");

    ImGui::Text("Hello ImGui");
    ImGui::Text("FrameIndex: %d", context.frameIndex);
    ImGui::SliderInt("Display Instance Count", &m_displayInstanceCount, 0, static_cast<int>(kMaxInstanceCount));
    ImGui::SliderFloat("Mesh Scale", &m_meshScale, 0.1f, 2.0f);
    ImGui::SliderFloat("Camera FovH", &m_scene.camera.fov, 20.f, 150.f);
    ImGui::ColorEdit4("Background Color", m_backBufferClearColor.data());
    ImGui::SliderFloat3("Light Direction", &m_lightingParams.lightDirection.x, -1.0f, 1.0f);
    ImGui::ColorEdit3("Light Color", &m_lightingParams.lightColor.x);
    ImGui::Checkbox("IBL Enabled", &m_iblEnabled);
    ImGui::BeginDisabled(!m_iblEnabled);
    ImGui::SliderFloat("IBL Intensity", &m_lightingParams.iblIntensity, 0.0f, 2.0f);
    ImGui::EndDisabled();
    ImGui::Checkbox("Show Skybox", &m_lightingParams.skyboxEnabled);
    ImGui::Checkbox("Skybox Preview", &m_lightingParams.skyboxPreview);
    ImGui::BeginDisabled(!m_lightingParams.skyboxPreview);
    ImGui::SliderFloat("Skybox Preview Exposure", &m_lightingParams.skyboxPreviewExposure, 0.0f, 2.0f);
    ImGui::EndDisabled();
    ImGui::SliderFloat("Direct Light Intensity", &m_lightingParams.diffuseIntensity, 0.0f, 4.0f);

    if (!m_sceneMesh.materials.empty())
    {
        ImGui::Text("Material Controls");
        const int materialCount = static_cast<int>(m_sceneMesh.materials.size());
        if (m_selectedMaterialIndex >= materialCount)
        {
            m_selectedMaterialIndex = materialCount - 1;
        }
        ImGui::SliderInt("Material", &m_selectedMaterialIndex, 0, materialCount - 1);

        Engine::SceneMaterial& material = m_sceneMesh.materials[m_selectedMaterialIndex];
        bool materialChanged = false;
        materialChanged |= ImGui::SliderFloat("Roughness", &material.roughnessFactor, 0.04f, 1.0f);
        materialChanged |= ImGui::SliderFloat("Metallic", &material.metallicFactor, 0.0f, 1.0f);
        materialChanged |= ImGui::SliderFloat("Indirect Occlusion", &material.ambientOcclusionFactor, 0.0f, 1.0f);
        materialChanged |= ImGui::SliderFloat("Emissive Luminance", &material.emissiveScale, 0.0f, 4.0f);

        const float f0 = 0.04f * (1.0f - material.metallicFactor) + material.metallicFactor;
        ImGui::Text("Specular F0: %.2f", f0);

        if (materialChanged)
        {
            HelloTextureEngine::MaterialParams params = {};
            params.roughnessFactor = material.roughnessFactor;
            params.metallicFactor = material.metallicFactor;
            params.ambientOcclusionFactor = material.ambientOcclusionFactor;
            params.emissiveScale = material.emissiveScale;
            m_engine.SetMaterialParams(static_cast<UINT>(m_selectedMaterialIndex), params);
        }
    }

    ImGui::Text("Tone Mapping");
    ImGui::RadioButton("None", &m_toneMapParams.operatorIndex, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Reinhard", &m_toneMapParams.operatorIndex, 1);
    ImGui::SameLine();
    ImGui::RadioButton("ACES", &m_toneMapParams.operatorIndex, 2);
    ImGui::SliderFloat("Exposure", &m_toneMapParams.exposure, 0.0f, 4.0f);
    ImGui::SliderFloat("Paper White", &m_toneMapParams.paperWhiteNits, 80.0f, 500.0f, "%.0f nits");
    ImGui::SliderFloat("Display Max", &m_toneMapParams.maxDisplayNits, 100.0f, 4000.0f, "%.0f nits");

    int renderingPath = static_cast<int>(m_renderingPath);
    ImGui::Text("Rendering Path");
    ImGui::RadioButton("Forward", &renderingPath, static_cast<int>(RenderingPath::Forward));
    ImGui::SameLine();
    ImGui::RadioButton("Deferred", &renderingPath, static_cast<int>(RenderingPath::Deferred));
    m_renderingPath = static_cast<RenderingPath>(renderingPath);

    const bool deferredRendering = m_renderingPath == RenderingPath::Deferred;
    int renderViewMode = static_cast<int>(m_renderViewMode);
    ImGui::Text("Debug View");
    ImGui::BeginDisabled(!deferredRendering);
    ImGui::RadioButton("Lit", &renderViewMode, static_cast<int>(RenderViewMode::LightPass));
    ImGui::RadioButton("Albedo", &renderViewMode, static_cast<int>(RenderViewMode::GBufferAlbedo));
    ImGui::SameLine();
    ImGui::RadioButton("Normal", &renderViewMode, static_cast<int>(RenderViewMode::GBufferNormal));
    ImGui::SameLine();
    ImGui::RadioButton("Material", &renderViewMode, static_cast<int>(RenderViewMode::GBufferMaterial));
    ImGui::RadioButton("MotionVector", &renderViewMode, static_cast<int>(RenderViewMode::GBufferMotionVector));
    ImGui::SameLine();
    ImGui::RadioButton("PBR Params", &renderViewMode, static_cast<int>(RenderViewMode::GBufferPBRParams));
    ImGui::SameLine();
    ImGui::RadioButton("Depth", &renderViewMode, static_cast<int>(RenderViewMode::Depth));
    m_renderViewMode = static_cast<RenderViewMode>(renderViewMode);
    ImGui::EndDisabled();

    if (!deferredRendering)
    {
        m_renderViewMode = RenderViewMode::LightPass;
    }

    const bool lightPassView = deferredRendering && m_renderViewMode == RenderViewMode::LightPass;
    ImGui::BeginDisabled(!lightPassView);
    ImGui::Checkbox("Debug LightPass Gradient", &m_lightingPassDebugGradient);
    ImGui::EndDisabled();

    if (lightPassView && m_lightingPassDebugGradient)
    {
        if (ImGui::Button("Validate HDR Gradient"))
        {
            m_requestHdrDump = true;
        }
    }

    ImGui::Text("CPU Frame: %.2f ms (%.1f FPS)", context.cpuFrameTime, 1000.0f / context.cpuFrameTime);

    const auto& gpuCheckPoints = context.gpuCheckPoints;
    const size_t gpuCheckPointCount = gpuCheckPoints.size();
    if (gpuCheckPointCount >= 2)
    {
        for (int i = 1; i < static_cast<int>(gpuCheckPointCount); i++)
        {
            const auto& checkPoint = gpuCheckPoints[i];
            if (i < static_cast<int>(gpuCheckPointCount) - 1)
            {
                const float timeFromPrevious = checkPoint.timeStamp - gpuCheckPoints[i - 1].timeStamp;
                ImGui::Text("GPU[%d] %s: %f ms", i, checkPoint.name.c_str(), timeFromPrevious);
            }
            else
            {
                ImGui::Text("GPU[%d] Total: %f ms", i, checkPoint.timeStamp);
            }
        }
    }

    ImGui::End();

    HelloTextureEngine::LightingParams lightingParams = m_lightingParams;
    if (!m_iblEnabled)
    {
        lightingParams.iblIntensity = 0.0f;
    }
    m_engine.SetLightingParams(lightingParams);
    m_engine.SetRenderingPath(m_renderingPath);
    m_engine.SetLightingPassDebugGradient(m_lightingPassDebugGradient);
    m_engine.SetBackBufferClearColor(m_backBufferClearColor);
    m_engine.SetDisplayInstanceCount(m_displayInstanceCount);
    m_engine.SetToneMapParams(m_toneMapParams);
    m_engine.SetRenderViewMode(m_renderViewMode);
    m_engine.SetRequestHdrDump(m_requestHdrDump);
    m_requestHdrDump = false;
}

void SampleApp::InitInstanceData(const Engine::SceneMesh& mesh)
{
    m_scene.instances.resize(kMaxInstanceCount);
    m_instanceDataForCPU.clear();
    for (int i = 0; i < static_cast<int>(kMaxInstanceCount); i++)
    {
        const XMFLOAT3 pos = InstanceIdToXYZ(i);
        m_instanceDataForCPU.emplace_back(pos, XMFLOAT3(0.0f, 0.0f, 0.0f));

        auto& d = m_scene.instances[i];
        const UINT gltfMaterialCount = static_cast<UINT>(mesh.materials.size());
        d.materialId = gltfMaterialCount > 0 ? i % gltfMaterialCount : 0;

        const XMMATRIX scaleMat = XMMatrixScaling(m_meshScale, m_meshScale, m_meshScale);
        const XMMATRIX transMat = XMMatrixTranslation(pos.x, pos.y, pos.z);
        XMStoreFloat4x4(&d.world, XMMatrixTranspose(scaleMat * transMat));
        d.prevWorld = d.world;
    }
}

void SampleApp::UpdateInstanceData(float deltaTime)
{
    static float accumTime = 0.f;
    if (m_isPlaying)
    {
        accumTime += deltaTime;
        if (accumTime > 1.0f)
        {
            accumTime = 0.f;
        }
    }

    for (int i = 0; i < static_cast<int>(kMaxInstanceCount); i++)
    {
        m_scene.instances[i].prevWorld = m_scene.instances[i].world;
        bool resetMotionVector = false;

        if (m_isPlaying)
        {
            m_instanceDataForCPU[i].pos.x += kTranslationSpeed;
        }
        if (m_instanceDataForCPU[i].pos.x > kOffsetBounds)
        {
            m_instanceDataForCPU[i].pos.x = -kOffsetBounds;
            resetMotionVector = true;
        }
        if (m_isPlaying)
        {
            auto& rot = m_instanceDataForCPU[i].rot;
            rot.x = std::fmod(rot.x + kRotationSpeed, 2.0f * kPI);
            rot.y = std::fmod(rot.y + kRotationSpeed, 2.0f * kPI);
            rot.z = std::fmod(rot.z + kRotationSpeed, 2.0f * kPI);
        }

        const XMMATRIX scaleMat = XMMatrixScaling(m_meshScale, m_meshScale, m_meshScale);
        const XMMATRIX transMat = XMMatrixTranslation(
            m_instanceDataForCPU[i].pos.x, m_instanceDataForCPU[i].pos.y, m_instanceDataForCPU[i].pos.z);
        const XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(
            m_instanceDataForCPU[i].rot.x, m_instanceDataForCPU[i].rot.y, m_instanceDataForCPU[i].rot.z);
        const XMMATRIX dragRotMat = XMMatrixRotationRollPitchYaw(m_dragRotation.x, m_dragRotation.y, 0.0f);
        XMStoreFloat4x4(&m_scene.instances[i].world, XMMatrixTranspose(scaleMat * rotMat * dragRotMat * transMat));
        if (resetMotionVector)
        {
            m_scene.instances[i].prevWorld = m_scene.instances[i].world;
        }
    }
}

XMFLOAT3 SampleApp::InstanceIdToXYZ(int instanceId)
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
        constexpr int startZ = -halfDimZ;

        for (int z = -halfDimZ; z <= halfDimZ; ++z)
        {
            for (int y = -halfDimY; y <= halfDimY; ++y)
            {
                for (int x = -halfDimX; x <= halfDimX; ++x)
                {
                    const int dx = x - centerX;
                    const int dy = y - centerY;
                    const int dz = z - startZ;
                    const int dist2 = dx * dx + dy * dy + dz * dz;
                    result.push_back({x, y, z, dist2});
                }
            }
        }

        std::sort(result.begin(),
                  result.end(),
                  [](const GridPoint& a, const GridPoint& b)
                  {
                      if (a.dist2 != b.dist2)
                          return a.dist2 < b.dist2;
                      if (a.z != b.z)
                          return a.z > b.z; // ZÃ£ÂÅ’Ã¥Â¤Â§Ã£ÂÂÃ£Ââ€žÃ¦â€“Â¹Ã£â€šâ€™Ã¥â€žÂªÃ¥â€¦Ë†
                      if (a.y != b.y)
                          return a.y < b.y;
                      return a.x < b.x;
                  });

        return result;
    }();

    instanceId = std::clamp(instanceId, 0, maxInstanceCount - 1);

    const GridPoint& p = points[instanceId];

    return XMFLOAT3(static_cast<float>(p.x), static_cast<float>(p.y), static_cast<float>(p.z));
}
