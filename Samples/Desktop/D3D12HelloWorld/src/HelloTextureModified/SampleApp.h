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

private:
    void LoadSceneAssets();
    void DrawDebugUi(HelloTextureEngine::DebugUiContext& context);

    GltfMeshData m_mesh;
    HelloTextureEngine::LightingParams m_lightingParams;
    HelloTextureEngine::RenderingPath m_renderingPath = HelloTextureEngine::RenderingPath::Deferred;
    bool m_lightingPassDebugGradient = false;
    std::array<float, 4> m_backBufferClearColor = {0.0f, 0.2f, 0.4f, 1.0f};

    // Temporary bridge: the engine still owns the rendering implementation until
    // device/swapchain/resource ownership can move behind an Engine-facing API.
    HelloTextureEngine m_engine;
};
