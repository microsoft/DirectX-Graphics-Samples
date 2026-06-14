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
#include <chrono>
#include "Scene/Scene.h"

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
    void OnWindowSizeChanged(UINT width, UINT height) override;
    void OnIdle() override;

    void UpdateSampleState();

private:
    enum class MeshSource
    {
        Gltf,
        Cube,
    };

    static constexpr MeshSource kMeshSource = MeshSource::Gltf;

    void LoadSceneAssets();
    void DrawDebugUi(const HelloTextureEngine::DebugUiContext& context);
    void InitInstanceData(const Engine::SceneMesh& mesh);
    void UpdateInstanceData(float deltaTime);

    GltfMeshData LoadGltfScene() const;
    static Engine::SceneMesh ConvertToSceneMesh(const GltfMeshData& mesh);

    static constexpr UINT kMaxInstanceCount = HelloTextureEngine::kMaxInstanceCount;
    static constexpr float kTranslationSpeed = 0.005f;
    static constexpr float kPI = 3.141592f;
    static constexpr float kRotationSpeed = kPI / 180.f / 3.f;
    static constexpr float kOffsetBounds = 5.f;
    static constexpr float kMouseRotationSpeed = 0.01f;

    struct InstanceDataForCPU
    {
        InstanceDataForCPU(XMFLOAT3 pos, XMFLOAT3 rot) : pos(pos), rot(rot) {}
        XMFLOAT3 pos;
        XMFLOAT3 rot;
    };

    static XMFLOAT3 InstanceIdToXYZ(int instanceId);

    Engine::Scene m_scene;
    Engine::SceneMesh m_sceneMesh;
    std::vector<InstanceDataForCPU> m_instanceDataForCPU;

    HelloTextureEngine::LightingParams m_lightingParams;
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
    int m_lastMouseX = 0;
    int m_lastMouseY = 0;
    XMFLOAT2 m_dragRotation = {0.0f, 0.0f};

    std::chrono::steady_clock::time_point m_prevTime;

    GraphicsDevice m_graphicsDevice;

    // The engine receives the graphics device so DXSample ownership can stay in SampleApp.
    HelloTextureEngine m_engine;
};
