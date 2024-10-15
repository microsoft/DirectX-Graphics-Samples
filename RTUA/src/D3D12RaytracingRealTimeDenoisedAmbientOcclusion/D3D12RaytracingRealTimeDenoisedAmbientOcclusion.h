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
#include "DirectXRaytracingHelper.h"
#include "RaytracingAccelerationStructure.h"
#include "PerformanceTimers.h"
#include "GpuTimeManager.h"
#include "Sampler.h"
#include "UILayer.h"
#include "RTAOGpuKernels.h"
#include "PBRTParser.h"
#include "SceneParameters.h"
#include "RTAO.h"
#include "Pathtracer.h"
#include "Denoiser.h"
#include "Composition.h"
#include "Scene.h"
#include "EngineTuning.h"


namespace Sample_Args
{
    extern EnumVar CompositionMode;
}


namespace Sample_GPUTime {
    enum { Pathtracing = 0, AOraytracing, AOdenoising, Count };
}


namespace Sample
{
    D3D12RaytracingRealTimeDenoisedAmbientOcclusion& instance();

    extern void OnRecreateRaytracingResources(void*);

    static const UINT FrameCount = 3;

    extern GpuResource g_debugOutput[2];
}

class D3D12RaytracingRealTimeDenoisedAmbientOcclusion : public DXSample
{

public:
    D3D12RaytracingRealTimeDenoisedAmbientOcclusion(UINT width, UINT height, std::wstring name);
    ~D3D12RaytracingRealTimeDenoisedAmbientOcclusion();


    // IDeviceNotify
    virtual void OnReleaseWindowSizeDependentResources() override { ReleaseWindowSizeDependentResources(); };
    virtual void OnCreateWindowSizeDependentResources() override { CreateWindowSizeDependentResources(); };

    // Messages
    virtual void OnInit();
    virtual void OnKeyDown(UINT8 key);
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnSizeChanged(UINT width, UINT height, bool minimized);

    virtual IDXGISwapChain* GetSwapchain() { return m_deviceResources->GetSwapChain(); }
    const DX::DeviceResources& GetDeviceResources() { return *m_deviceResources; }

    void RequestSceneInitialization() { m_isSceneInitializationRequested = true; }
    void RequestRecreateRaytracingResources() { m_isRecreateRaytracingResourcesRequested = true; }
    RTAO& RTAOComponent() { return m_RTAO; }

private:
    std::mt19937 m_generatorURNG;
                
    ComPtr<ID3D12Fence>                 m_fence;
    UINT64                              m_fenceValues[Sample::FrameCount];
    Microsoft::WRL::Wrappers::Event     m_fenceEvent;

    std::shared_ptr<DX::DescriptorHeap> m_cbvSrvUavHeap;

    // Raytracing scene
    bool m_isProfiling = false;
    UINT m_numRemainingFramesToProfile = 0;
    std::map<std::wstring, std::list<std::wstring>> m_profilingResults;
    GpuResource m_raytracingOutput;

    // Sample components
    Pathtracer m_pathtracer;
    RTAO m_RTAO;
    Denoiser m_denoiser;
    Composition m_composition;
    Scene m_scene;

    // Application state
    UINT m_raytracingWidth;
    UINT m_raytracingHeight;
    DX::GPUTimer m_sampleGpuTimes[Sample_GPUTime::Count];
    float m_fps;
    bool m_isSceneInitializationRequested;
    bool m_isRecreateRaytracingResourcesRequested;
    bool m_renderOnce = false;
    int m_framesToRender = 0;

    // UI
    std::unique_ptr<UILayer> m_uiLayer;

    // Utility functions
    void RecreateD3D();
    void UpdateUI();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void ReleaseDeviceDependentResources();
    void ReleaseWindowSizeDependentResources();
    void CreateDescriptorHeaps();
    void CreateRaytracingOutputResource();
    void CreateDebugResources();
    void CreateAuxilaryDeviceResources();
    void CopyRaytracingOutputToBackbuffer(D3D12_RESOURCE_STATES outRenderTargetState = D3D12_RESOURCE_STATE_PRESENT);
    void CalculateFrameStats();
    void WriteProfilingResultsToFile();
};
