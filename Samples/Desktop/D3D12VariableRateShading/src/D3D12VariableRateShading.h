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

#include "DXSample.h"
#include "StepTimer.h"

using namespace DirectX;

class UILayer;
class VariableRateShadingScene;

class D3D12VariableRateShading : public DXSample
{
public:
    D3D12VariableRateShading(UINT width, UINT height, std::wstring name);
    ~D3D12VariableRateShading();

    static const UINT FrameCount = 3;

protected:
    virtual void OnInit();
    virtual void OnKeyDown(UINT8 key);
    virtual void OnKeyUp(UINT8 key);
    virtual void OnSizeChanged(UINT width, UINT height, bool minimized);
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();
    virtual IDXGISwapChain* GetSwapchain() { return m_swapChain.Get(); }

private:
    // D3D objects.
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<IDXGISwapChain4> m_swapChain;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12Fence> m_fence;
    
    // Scene rendering resources.
    std::unique_ptr<VariableRateShadingScene> m_scene;

    // UILayer.
    std::unique_ptr<UILayer> m_uiLayer;
    bool m_bCtrlKeyIsPressed;
    bool m_bShiftKeyIsPressed;
    float m_fps;
    
    StepTimer m_timer;

    // Frame synchronization objects.
    UINT   m_frameIndex;
    HANDLE m_fenceEvent;
    UINT64 m_fenceValues[FrameCount];

    // Window state.
    bool m_windowVisible;
    bool m_windowedMode;

    // Singleton object so that worker threads can share members.
    static D3D12VariableRateShading* s_app;

    void LoadPipeline();
    void LoadAssets();
    void LoadSizeDependentResources();
    void ReleaseSizeDependentResources();
    void UpdateUI();
    void RecreateD3DResources();
    void ReleaseD3DObjects(); 
    void CalculateFrameStats(); 
    void WaitForGpu(ID3D12CommandQueue* pCommandQueue);
    void MoveToNextFrame();
};