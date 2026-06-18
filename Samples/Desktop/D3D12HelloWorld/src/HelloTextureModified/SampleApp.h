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

#pragma once

#include "D3D12HelloTexture.h"
#include "DXSample.h"
#include "Scene/SampleScene.h"
#include "Ui/ImGuiSystem.h"

#include <chrono>
#include <memory>

class SampleApp : public DXSample
{
public:
    SampleApp(UINT width, UINT height, std::wstring name);

    void OnInit() override;
    void OnUpdate() override;
    void OnRender() override;
    void OnDestroy() override;
    void OnKeyDown(UINT8 key) override;
    void OnKeyUp(UINT8 key) override;
    void OnMouseDown(UINT8 button, int x, int y) override;
    void OnMouseUp(UINT8 button, int x, int y) override;
    void OnMouseMove(int x, int y) override;
    void OnMouseWheel(int wheelDelta) override;
    void OnWindowSizeChanged(UINT width, UINT height) override;
    void OnIdle() override;

    void UpdateSampleState();

private:
    enum class AppMode
    {
        SceneSelect,
        Running,
    };

    static constexpr int kDefaultSceneIndex = 0;

    void CreateSampleScenes();
    void LoadSceneCpuData(int sceneIndex);
    void OpenSelectedScene();
    void CloseRunningScene();
    void InitializeImGui();
    void UpdateUiFrame();
    Engine::SampleScene& LoadedScene();
    const Engine::SampleScene& LoadedScene() const;
    void DrawDebugUi(const HelloTextureEngine::UiFrameContext& context);
    void DrawSceneSelectUi();

    static constexpr UINT kMaxInstanceCount = HelloTextureEngine::kMaxInstanceCount;
    static constexpr float kMousePanSpeed = 0.01f;
    static constexpr float kMouseWheelCameraSpeed = 0.25f;
    static constexpr float kMouseWheelFovSpeed = 1.0f;
    static constexpr float kCameraMinZ = -100.0f;
    static constexpr float kCameraMaxZ = 100.0f;
    static constexpr UINT kImGuiDescriptorCount = 100;

    std::vector<std::unique_ptr<Engine::SampleScene>> m_sampleScenes;
    Engine::SampleScene* m_loadedScene = nullptr;
    int m_loadedSceneIndex = -1;
    int m_selectedSceneIndex = kDefaultSceneIndex;
    AppMode m_appMode = AppMode::SceneSelect;
    bool m_sceneResourcesLoaded = false;

    HelloTextureEngine::LightingParams m_lightingParams;
    Engine::ProceduralEnvironmentSettings m_environmentSettings;
    bool m_environmentAutoUpdate = Engine::kUseGpuProceduralEnvMap;
    bool m_environmentReloadPending = false;
    HelloTextureEngine::RenderingPath m_renderingPath = HelloTextureEngine::RenderingPath::Deferred;
    bool m_iblEnabled = true;
    bool m_lightingPassDebugGradient = false;
    int m_selectedMaterialIndex = 0;
    std::array<float, 4> m_backBufferClearColor = {0.0f, 0.2f, 0.4f, 1.0f};
    HelloTextureEngine::ToneMapParams m_toneMapParams;
    HelloTextureEngine::RenderViewMode m_renderViewMode = HelloTextureEngine::RenderViewMode::LightPass;
    bool m_requestHdrDump = false;

    int m_displayInstanceCount = static_cast<int>(kMaxInstanceCount);
    float m_meshScale = 0.5f;
    bool m_isPlaying = false;

    bool m_isDragging = false;
    bool m_isMiddleDragging = false;
    int m_lastMouseX = 0;
    int m_lastMouseY = 0;
    XMFLOAT3 m_lastArcballVector = {0.0f, 0.0f, 1.0f};
    XMFLOAT4 m_dragRotation = {0.0f, 0.0f, 0.0f, 1.0f};
    XMFLOAT2 m_dragPan = {0.0f, 0.0f};

    std::chrono::steady_clock::time_point m_prevTime;

    GraphicsDevice m_graphicsDevice;
    ComPtr<ID3D12DescriptorHeap> m_imguiHeap;
    Engine::ImGuiSystem m_imguiSystem;

    // The engine receives the graphics device so DXSample ownership can stay in SampleApp.
    HelloTextureEngine m_engine;
};
