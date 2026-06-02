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

#include "SampleApp.h"
#include "imgui.h"
#include <cmath>

SampleApp::SampleApp(UINT width, UINT height, std::wstring name)
    : DXSample(width, height, name), m_engine(width, height, name),
      m_prevTime(std::chrono::steady_clock::now())
{
}

void SampleApp::OnInit()
{
    LoadSceneAssets();
    InitInstanceData(m_mesh);
    m_engine.SetSceneMesh(&m_mesh);
    m_engine.SetDebugUiHandler([this](HelloTextureEngine::DebugUiContext& context) { DrawDebugUi(context); });
    m_engine.SetUseWarpDevice(m_useWarpDevice);
    m_engine.SetLightingParams(m_lightingParams);
    m_engine.SetRenderingPath(m_renderingPath);
    m_engine.SetLightingPassDebugGradient(m_lightingPassDebugGradient);
    m_engine.SetBackBufferClearColor(m_backBufferClearColor);
    m_engine.SetCameraState(m_camera);
    m_engine.SetInstanceData(m_instanceData);
    m_engine.SetDisplayInstanceCount(m_displayInstanceCount);
    m_engine.OnInit();
}

void SampleApp::OnUpdate()
{
    auto now = std::chrono::steady_clock::now();
    const float deltaTime = std::chrono::duration<float>(now - m_prevTime).count();
    m_prevTime = now;

    static constexpr float kCameraMoveSpeed = 0.01f;
    if (GetForegroundWindow() == Win32Application::GetHwnd())
    {
        if (GetAsyncKeyState(VK_SPACE) & 0x8000)
        {
            m_isPlaying = !m_isPlaying;
            Sleep(200);
        }
        if (GetAsyncKeyState('A') & 0x8000) m_camera.pos.x -= kCameraMoveSpeed;
        if (GetAsyncKeyState('D') & 0x8000) m_camera.pos.x += kCameraMoveSpeed;
        if ((GetAsyncKeyState('W') & 0x8000) && (GetAsyncKeyState(VK_CONTROL) & 0x8000))  m_camera.pos.y -= kCameraMoveSpeed;
        if ((GetAsyncKeyState('S') & 0x8000) && (GetAsyncKeyState(VK_CONTROL) & 0x8000))  m_camera.pos.y += kCameraMoveSpeed;
        if ((GetAsyncKeyState('W') & 0x8000) && !(GetAsyncKeyState(VK_CONTROL) & 0x8000)) m_camera.pos.z += kCameraMoveSpeed;
        if ((GetAsyncKeyState('S') & 0x8000) && !(GetAsyncKeyState(VK_CONTROL) & 0x8000)) m_camera.pos.z -= kCameraMoveSpeed;
    }

    UpdateInstanceData(deltaTime);

    m_engine.SetCameraState(m_camera);
    m_engine.SetInstanceData(m_instanceData);
    m_engine.SetDisplayInstanceCount(m_displayInstanceCount);
    m_engine.OnUpdate();
}

void SampleApp::OnRender()
{
    m_engine.OnRender();
}

void SampleApp::OnDestroy()
{
    m_engine.OnDestroy();
}

void SampleApp::OnKeyDown(UINT8 key)
{
    m_engine.OnKeyDown(key);
}

void SampleApp::OnKeyUp(UINT8 key)
{
    m_engine.OnKeyUp(key);
}

void SampleApp::OnMouseDown(UINT8 button, int x, int y)
{
    if (button == VK_LBUTTON)
    {
        m_isDragging = true;
        m_lastMouseX = x;
        m_lastMouseY = y;
    }
    m_engine.OnMouseDown(button, x, y);
}

void SampleApp::OnMouseUp(UINT8 button, int x, int y)
{
    if (button == VK_LBUTTON)
    {
        m_isDragging = false;
    }
    m_engine.OnMouseUp(button, x, y);
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
    m_engine.OnMouseMove(x, y);
}

void SampleApp::OnWindowSizeChanged(UINT width, UINT height)
{
    m_engine.OnWindowSizeChanged(width, height);
}

void SampleApp::OnIdle()
{
    m_engine.OnIdle();
}

void SampleApp::LoadSceneAssets()
{
    const bool loaded = LoadGltfMesh("Assets\\Models\\DamagedHelmet\\glTF\\DamagedHelmet.gltf", m_mesh);
    assert(loaded);
}

void SampleApp::DrawDebugUi(HelloTextureEngine::DebugUiContext& context)
{
    using RenderingPath = HelloTextureEngine::RenderingPath;
    using RenderViewMode = HelloTextureEngine::RenderViewMode;

    ImGui::SetNextWindowSize(ImVec2(400, 140), ImGuiCond_FirstUseEver);
    ImGui::Begin("Debug");

    ImGui::Text("Hello ImGui");
    ImGui::Text("FrameIndex: %d", context.frameIndex);
    ImGui::SliderInt("Display Instance Count", &m_displayInstanceCount, 0, static_cast<int>(kMaxInstanceCount));
    ImGui::SliderFloat("Mesh Scale", &m_meshScale, 0.1f, 2.0f);
    ImGui::SliderFloat("Camera FovH", &m_camera.fov, 20.f, 150.f);
    ImGui::ColorEdit4("BackBuffer Clear", m_backBufferClearColor.data());
    ImGui::SliderFloat3("Light Direction", &m_lightingParams.lightDirection.x, -1.0f, 1.0f);
    ImGui::ColorEdit3("Light Color", &m_lightingParams.lightColor.x);
    ImGui::SliderFloat("Ambient", &m_lightingParams.ambientIntensity, 0.0f, 1.0f);
    ImGui::SliderFloat("Diffuse", &m_lightingParams.diffuseIntensity, 0.0f, 4.0f);

    ImGui::Text("ToneMap");
    ImGui::RadioButton("None", &context.toneMapOperator, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Reinhard", &context.toneMapOperator, 1);
    ImGui::SameLine();
    ImGui::RadioButton("ACES", &context.toneMapOperator, 2);
    ImGui::SliderFloat("Exposure", &context.exposure, 0.0f, 4.0f);
    ImGui::SliderFloat("Paper White", &context.paperWhiteNits, 80.0f, 500.0f, "%.0f nits");
    ImGui::SliderFloat("Display Max", &context.maxDisplayNits, 100.0f, 4000.0f, "%.0f nits");

    int renderingPath = static_cast<int>(m_renderingPath);
    ImGui::Text("Rendering Path");
    ImGui::RadioButton("Forward", &renderingPath, static_cast<int>(RenderingPath::Forward));
    ImGui::SameLine();
    ImGui::RadioButton("Deferred", &renderingPath, static_cast<int>(RenderingPath::Deferred));
    m_renderingPath = static_cast<RenderingPath>(renderingPath);

    const bool deferredRendering = m_renderingPath == RenderingPath::Deferred;
    if (ImGui::Button("Dump HDR Buffers"))
    {
        context.requestHdrDump = true;
    }

    int renderViewMode = static_cast<int>(context.renderViewMode);
    ImGui::Text("Render View");
    ImGui::BeginDisabled(!deferredRendering);
    ImGui::RadioButton("LightPass", &renderViewMode, static_cast<int>(RenderViewMode::LightPass));
    ImGui::RadioButton("Albedo", &renderViewMode, static_cast<int>(RenderViewMode::GBufferAlbedo));
    ImGui::SameLine();
    ImGui::RadioButton("Normal", &renderViewMode, static_cast<int>(RenderViewMode::GBufferNormal));
    ImGui::SameLine();
    ImGui::RadioButton("Material", &renderViewMode, static_cast<int>(RenderViewMode::GBufferMaterial));
    ImGui::RadioButton("MotionVector", &renderViewMode, static_cast<int>(RenderViewMode::GBufferMotionVector));
    ImGui::SameLine();
    ImGui::RadioButton("PBRParams", &renderViewMode, static_cast<int>(RenderViewMode::GBufferPBRParams));
    ImGui::SameLine();
    ImGui::RadioButton("Depth", &renderViewMode, static_cast<int>(RenderViewMode::Depth));
    context.renderViewMode = static_cast<RenderViewMode>(renderViewMode);
    ImGui::EndDisabled();

    const bool lightPassView = deferredRendering && context.renderViewMode == RenderViewMode::LightPass;
    ImGui::BeginDisabled(!lightPassView);
    ImGui::Checkbox("Debug LightPass Gradient", &m_lightingPassDebugGradient);
    ImGui::EndDisabled();

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

    m_engine.SetLightingParams(m_lightingParams);
    m_engine.SetRenderingPath(m_renderingPath);
    m_engine.SetLightingPassDebugGradient(m_lightingPassDebugGradient);
    m_engine.SetBackBufferClearColor(m_backBufferClearColor);
    m_engine.SetCameraState(m_camera);
    m_engine.SetDisplayInstanceCount(m_displayInstanceCount);
}

XMFLOAT3 SampleApp::InstanceIdToXYZ(int instanceId)
{
    const int dimX = 10, dimY = 10, dimZ = 10;
    const int x = instanceId % dimX - (kMaxInstanceCount / dimZ / dimY) / 2;
    const int y = (instanceId / dimX) % dimY - (kMaxInstanceCount / dimX / dimY) / 2;
    const int z = -instanceId / (dimX * dimY) + (kMaxInstanceCount / dimX / dimY) / 2;
    return XMFLOAT3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}

void SampleApp::InitInstanceData(const GltfMeshData& mesh)
{
    m_instanceData.resize(kMaxInstanceCount);
    m_instanceDataForCPU.clear();
    for (int i = 0; i < static_cast<int>(kMaxInstanceCount); i++)
    {
        const XMFLOAT3 pos = InstanceIdToXYZ(i);
        m_instanceDataForCPU.emplace_back(pos, XMFLOAT3(0.0f, 0.0f, 0.0f));

        auto& d = m_instanceData[i];
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
        m_instanceData[i].prevWorld = m_instanceData[i].world;
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
        const XMMATRIX transMat = XMMatrixTranslation(m_instanceDataForCPU[i].pos.x, m_instanceDataForCPU[i].pos.y,
                                                      m_instanceDataForCPU[i].pos.z);
        const XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(m_instanceDataForCPU[i].rot.x,
                                                              m_instanceDataForCPU[i].rot.y,
                                                              m_instanceDataForCPU[i].rot.z);
        const XMMATRIX dragRotMat = XMMatrixRotationRollPitchYaw(m_dragRotation.x, m_dragRotation.y, 0.0f);
        XMStoreFloat4x4(&m_instanceData[i].world, XMMatrixTranspose(scaleMat * rotMat * dragRotMat * transMat));
        if (resetMotionVector)
        {
            m_instanceData[i].prevWorld = m_instanceData[i].world;
        }
    }
}
