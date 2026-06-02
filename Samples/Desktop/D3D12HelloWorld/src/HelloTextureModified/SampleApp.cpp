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

SampleApp::SampleApp(UINT width, UINT height, std::wstring name)
    : DXSample(width, height, name), m_engine(width, height, name)
{
}

void SampleApp::OnInit()
{
    LoadSceneAssets();
    m_engine.SetSceneMesh(&m_mesh);
    m_engine.SetDebugUiHandler([this](HelloTextureEngine::DebugUiContext& context) { DrawDebugUi(context); });
    m_engine.SetUseWarpDevice(m_useWarpDevice);
    m_engine.SetLightingParams(m_lightingParams);
    m_engine.OnInit();
}

void SampleApp::OnUpdate()
{
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
    m_engine.OnMouseDown(button, x, y);
}

void SampleApp::OnMouseUp(UINT8 button, int x, int y)
{
    m_engine.OnMouseUp(button, x, y);
}

void SampleApp::OnMouseMove(int x, int y)
{
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
    ImGui::SliderInt("Display Instance Count", &context.displayInstanceCount, 0, context.maxInstanceCount);
    ImGui::SliderFloat("Mesh Scale", &context.meshScale, 0.1f, 2.0f);
    ImGui::SliderFloat("Camera FovH", &context.cameraFov, 20.f, 150.f);
    ImGui::ColorEdit4("BackBuffer Clear", context.backBufferClearColor.data());
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

    int renderingPath = static_cast<int>(context.renderingPath);
    ImGui::Text("Rendering Path");
    ImGui::RadioButton("Forward", &renderingPath, static_cast<int>(RenderingPath::Forward));
    ImGui::SameLine();
    ImGui::RadioButton("Deferred", &renderingPath, static_cast<int>(RenderingPath::Deferred));
    context.renderingPath = static_cast<RenderingPath>(renderingPath);

    const bool deferredRendering = context.renderingPath == RenderingPath::Deferred;
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
    ImGui::Checkbox("Debug LightPass Gradient", &context.lightingPassDebugGradientEnabled);
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
}
