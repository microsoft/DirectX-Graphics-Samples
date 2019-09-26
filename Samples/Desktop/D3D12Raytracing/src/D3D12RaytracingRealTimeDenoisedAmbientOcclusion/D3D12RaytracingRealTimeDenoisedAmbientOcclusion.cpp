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
#include "D3D12RaytracingRealTimeDenoisedAmbientOcclusion.h"
#include "GameInput.h"
#include "EngineTuning.h"
#include "EngineProfiling.h"
#include "RTAO\RTAO.h"

using namespace std;
using namespace DX;
using namespace DirectX;
using namespace SceneEnums;
using namespace GameCore;

HWND g_hWnd = 0;

namespace Sample
{
    D3D12RaytracingRealTimeDenoisedAmbientOcclusion* g_pSample = nullptr;
    D3D12RaytracingRealTimeDenoisedAmbientOcclusion& instance()
    {
        return *g_pSample;
    }

    GpuResource g_debugOutput[2];

    void OnRecreateRaytracingResources(void*)
    {
        g_pSample->RequestRecreateRaytracingResources();
    }
}

D3D12RaytracingRealTimeDenoisedAmbientOcclusion::D3D12RaytracingRealTimeDenoisedAmbientOcclusion(UINT width, UINT height, wstring name) :
    DXSample(width, height, name),
    m_isSceneInitializationRequested(false),
    m_isRecreateRaytracingResourcesRequested(false)
{
    ThrowIfFalse(Sample::g_pSample == nullptr, L"There can be only one sample object instance.");
    Sample::g_pSample = this;
    UpdateForSizeChange(width, height);
    m_generatorURNG.seed(1729);
}

void D3D12RaytracingRealTimeDenoisedAmbientOcclusion::OnInit()
{
    UINT flags =
        m_syncInterval > 0
        ? 0
        // Sample shows handling of use cases with tearing support, which is OS dependent and has been supported since TH2.
        // Since the DXR requires October 2018 update, we don't need to handle non-tearing cases.
        : DeviceResources::c_RequireTearingSupport;

    m_deviceResources = make_shared<DeviceResources>(
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_UNKNOWN,
       Sample:: FrameCount,
        D3D_FEATURE_LEVEL_11_0,
        flags,
        m_adapterIDoverride
        );
    m_deviceResources->RegisterDeviceNotify(this);
    m_deviceResources->SetWindow(Win32Application::GetHwnd(), m_width, m_height);

    g_hWnd = Win32Application::GetHwnd();
    GameInput::Initialize();
    EngineTuning::Initialize();

    m_deviceResources->InitializeDXGIAdapter();

    ThrowIfFalse(IsDirectXRaytracingSupported(m_deviceResources->GetAdapter()),
        L"ERROR: DirectX Raytracing is not supported by your GPU and driver.\n\n");

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();
    m_deviceResources->CreateWindowSizeDependentResources();
}

D3D12RaytracingRealTimeDenoisedAmbientOcclusion::~D3D12RaytracingRealTimeDenoisedAmbientOcclusion()
{
    GameInput::Shutdown();
}

void D3D12RaytracingRealTimeDenoisedAmbientOcclusion::WriteProfilingResultsToFile()
{
    std::wofstream outputFile(L"Profile.csv", std::ofstream::trunc);

    // Column headers.
    size_t maxNumResults = 0;
    for (auto& column : m_profilingResults)
    {
        outputFile << column.first << L",";
        maxNumResults = max(maxNumResults, column.second.size());
    }
    outputFile << L"\n";

    // Column results.
    for (size_t i = 0; i < maxNumResults; i++)
    {
        for (auto& column : m_profilingResults)
        {
            if (column.second.size())
            {
                outputFile << column.second.front();
                column.second.pop_front();
            }
            outputFile << L",";
        }
        outputFile << L"\n";
    }
    outputFile.close();
}

// Create resources that depend on the device.
void D3D12RaytracingRealTimeDenoisedAmbientOcclusion::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    CreateDescriptorHeaps();
    CreateAuxilaryDeviceResources();
        
    m_scene.Setup(m_deviceResources, m_cbvSrvUavHeap);
    m_pathtracer.Setup(m_deviceResources, m_cbvSrvUavHeap, m_scene);

    // With BLAS and their instanceContributionToHitGroupIndex initialized during 
    // Pathracer setup's shader table build, initialize the AS.
    // Make sure to call this before RTAO build shader tables as it queries
    // max instanceContributionToHitGroupIndex from the scene's AS.
    m_scene.InitializeAccelerationStructures();

    m_RTAO.Setup(m_deviceResources, m_cbvSrvUavHeap, m_scene);
    m_denoiser.Setup(m_deviceResources, m_cbvSrvUavHeap);
    m_composition.Setup(m_deviceResources, m_cbvSrvUavHeap);
}

// Create a 2D output texture for raytracing.
void D3D12RaytracingRealTimeDenoisedAmbientOcclusion::CreateRaytracingOutputResource()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto backbufferFormat = m_deviceResources->GetBackBufferFormat();

    CreateRenderTargetResource(device, backbufferFormat, m_width, m_height, m_cbvSrvUavHeap.get(), &m_raytracingOutput, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

void D3D12RaytracingRealTimeDenoisedAmbientOcclusion::CreateDebugResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto backbufferFormat = m_deviceResources->GetBackBufferFormat();

    DXGI_FORMAT debugFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
    using namespace Sample;

    // Debug resources
    {
        // Preallocate subsequent descriptor indices for both SRV and UAV groups.
        g_debugOutput[0].uavDescriptorHeapIndex = m_cbvSrvUavHeap->AllocateDescriptorIndices(ARRAYSIZE(g_debugOutput));
        g_debugOutput[0].srvDescriptorHeapIndex = m_cbvSrvUavHeap->AllocateDescriptorIndices(ARRAYSIZE(g_debugOutput));
        for (UINT i = 0; i < ARRAYSIZE(g_debugOutput); i++)
        {
            g_debugOutput[i].uavDescriptorHeapIndex = g_debugOutput[0].uavDescriptorHeapIndex + i;
            g_debugOutput[i].srvDescriptorHeapIndex = g_debugOutput[0].srvDescriptorHeapIndex + i;
            CreateRenderTargetResource(device, debugFormat, m_raytracingWidth, m_raytracingHeight, m_cbvSrvUavHeap.get(), &g_debugOutput[i], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"Debug");
        }
    }
}

void D3D12RaytracingRealTimeDenoisedAmbientOcclusion::CreateAuxilaryDeviceResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandQueue = m_deviceResources->GetCommandQueue();
    auto commandList = m_deviceResources->GetCommandList();

    EngineProfiling::RestoreDevice(device, commandQueue, Sample::FrameCount);
        
    for (UINT i = 0; i < Sample_GPUTime::Count; i++)
    {
        m_sampleGpuTimes[i].RestoreDevice(device, m_deviceResources->GetCommandQueue(), Sample::FrameCount);
        m_sampleGpuTimes[i].SetAvgRefreshPeriodMS(500);
    }
}

void D3D12RaytracingRealTimeDenoisedAmbientOcclusion::CreateDescriptorHeaps()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Allocate large enough descriptor pool.
    UINT NumDescriptors = 10000;
    m_cbvSrvUavHeap = make_shared<DX::DescriptorHeap>(device, NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}
    
void D3D12RaytracingRealTimeDenoisedAmbientOcclusion::OnKeyDown(UINT8 key)
{
    m_scene.OnKeyDown(key);

    float fValue;
    switch (key)
    {
    case VK_ESCAPE:
        throw HrException(E_APPLICATION_EXITING);

    case VK_NUMPAD0:
        RTAO_Args::Spp_useGroundTruthSpp.Bang();
        break;
    case VK_NUMPAD1:
        Composition_Args::CompositionMode.SetValue(CompositionType::AmbientOcclusionOnly_RawOneFrame);
        break;
    case VK_NUMPAD2:
        Composition_Args::CompositionMode.SetValue(CompositionType::AmbientOcclusionOnly_Denoised);
        break;
    case VK_NUMPAD3:
        Composition_Args::CompositionMode.SetValue(CompositionType::PBRShading);
        break;
    case VK_NUMPAD4:
        fValue = IsInRange(static_cast<float>(RTAO_Args::MaxRayHitTime), 3.9f, 4.1f) ? 22.f : 4.f;
        m_RTAO.SetMaxRayHitTime(fValue);
        break;
    case VK_SPACE:
        m_renderOnce = !m_renderOnce;
        m_framesToRender = m_renderOnce ? 1 : 0;
        break;
    case VK_RETURN:
        Composition_Args::AOEnabled.Bang();
        break;
    case VK_F9:
        if (m_isProfiling)
            WriteProfilingResultsToFile();
        else
        {
            m_numRemainingFramesToProfile = 1000;
            float perFrameSeconds = Scene_Args::CameraRotationDuration / m_numRemainingFramesToProfile;
            m_scene.m_timer.SetTargetElapsedSeconds(perFrameSeconds);
            m_scene.m_timer.ResetElapsedTime();
            m_scene.m_animateCamera = true;
            EngineTuning::SetIsVisible(false);
            EngineProfiling::DrawProfiler.SetValue(true);
            EngineProfiling::DrawCpuTime.SetValue(false);
        }
        m_isProfiling = !m_isProfiling;
        m_scene.m_timer.SetFixedTimeStep(m_isProfiling);

    default:
        break;
    }
}

// Update frame-based values.
void D3D12RaytracingRealTimeDenoisedAmbientOcclusion::OnUpdate()
{
    if (m_isProfiling)
    {
        if (m_numRemainingFramesToProfile == 0)
        {
            m_isProfiling = false;
            m_scene.m_timer.SetFixedTimeStep(false);
            WriteProfilingResultsToFile();
            m_scene.m_animateCamera = false;
        }
        else
        {
            m_numRemainingFramesToProfile--;
        }
    }

    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();
    auto prevFrameIndex = m_deviceResources->GetPreviousFrameIndex();

    if (m_isSceneInitializationRequested)
    {
        m_isSceneInitializationRequested = false;
        m_deviceResources->WaitForGpu();
        OnInit();
    }

    if (m_isRecreateRaytracingResourcesRequested)
    {
        m_isRecreateRaytracingResourcesRequested = false;
        m_deviceResources->WaitForGpu();

        OnCreateWindowSizeDependentResources();
    }


    CalculateFrameStats();

    float elapsedTime = static_cast<float>(m_scene.Timer().GetElapsedSeconds());
    GameInput::Update(elapsedTime);
    EngineTuning::Update(elapsedTime);
    EngineProfiling::Update();

    m_scene.OnUpdate();

    if (m_enableUI)
    {
        UpdateUI();
    }

}

// Copy the raytracing output to the backbuffer.
void D3D12RaytracingRealTimeDenoisedAmbientOcclusion::CopyRaytracingOutputToBackbuffer(D3D12_RESOURCE_STATES outRenderTargetState)
{
    auto commandList = m_deviceResources->GetCommandList();
    auto resourceStateTracker = m_deviceResources->GetGpuResourceStateTracker();
    auto renderTarget = m_deviceResources->GetRenderTarget();

    ID3D12Resource* raytracingOutput = m_raytracingOutput.GetResource();

    resourceStateTracker->FlushResourceBarriers();
    CopyResource(
        commandList,
        raytracingOutput,
        renderTarget,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        outRenderTargetState);
}

void D3D12RaytracingRealTimeDenoisedAmbientOcclusion::UpdateUI()
{
    vector<wstring> labels;

    // Header information
    {
        // Prints <component name>[<resolution X>x<resolution Y>]: <GPU time>
        wstringstream wLabel;
        auto PrintComponentInfo = [&](const wstring& componentName, UINT width, UINT height, float gpuTime)
        {
            wLabel << componentName << L"[" << width << L"x" << height << "]: " << setprecision(2) << fixed << gpuTime << "ms" << L"\n";
        };
        PrintComponentInfo(L"Pathtracing", m_pathtracer.Width(), m_pathtracer.Height(), m_sampleGpuTimes[Sample_GPUTime::Pathtracing].GetAverageMS());
        PrintComponentInfo(L"AO raytracing", m_RTAO.RaytracingWidth(), m_RTAO.RaytracingHeight(), m_sampleGpuTimes[Sample_GPUTime::AOraytracing].GetAverageMS());
        PrintComponentInfo(L"AO denoising", m_denoiser.DenoisingWidth(), m_denoiser.DenoisingHeight(), m_sampleGpuTimes[Sample_GPUTime::AOdenoising].GetAverageMS());
        labels.push_back(wLabel.str());
    }
    // Engine tuning.
    {     
        wstringstream wLabel;
        EngineTuning::Display(&wLabel, m_isProfiling);
        labels.push_back(wLabel.str());

        if (m_isProfiling)
        {
            set<wstring> profileMarkers = {
                    L"Pathtracing",
                        L"CalculatePartialDerivatives",
                        L"DownsampleGBuffer",
                    L"RTAO",
                        L"CalculateAmbientOcclusion",
                            L"Sort Rays",
                            L"AO Ray Generator",
                            L"AO DispatchRays",
                        L"Denoise",
                            L"Temporal Supersampling p1 (Reverse Reprojection)",
                            L"Temporal Supersampling p2 (BlendWithCurrentFrame)",
                            L"AtrousWaveletTransformFilter",
                            L"Disocclusions blur",
                    L"CompositionCS",
                        L"Upsample AO",
            };

            wstring line;
            while (getline(wLabel, line))
            {
                std::wstringstream ss(line);
                wstring name;
                wstring time;
                getline(ss, name, L':');
                getline(ss, time);
                for (auto& profileMarker : profileMarkers)
                {
                    if (name.find(profileMarker) != wstring::npos)
                    {
                        m_profilingResults[profileMarker].push_back(time);
                        break;
                    }
                }
            }
        }
    }

    wstring uiText = L"";
    for (auto s : labels)
    {
        uiText += s;
    }

    m_uiLayer->UpdateLabels(uiText);
}

// Create resources that are dependent on the size of the main window.
void D3D12RaytracingRealTimeDenoisedAmbientOcclusion::CreateWindowSizeDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandQueue = m_deviceResources->GetCommandQueue();
    auto renderTargets = m_deviceResources->GetRenderTargets();

    UINT GBufferWidth = m_width;
    UINT GBufferHeight = m_height;

    if (RTAO_Args::QuarterResAO)
    {
        m_raytracingWidth = CeilDivide(GBufferWidth, 2);
        m_raytracingHeight = CeilDivide(GBufferHeight, 2);
    }
    else
    {
        m_raytracingWidth = GBufferWidth;
        m_raytracingHeight = GBufferHeight;
    }

    m_pathtracer.SetResolution(GBufferWidth, GBufferHeight, m_raytracingWidth, m_raytracingHeight);
    m_RTAO.SetResolution(m_raytracingWidth, m_raytracingHeight);
    m_denoiser.SetResolution(m_raytracingWidth, m_raytracingHeight);
    m_composition.SetResolution(GBufferWidth, GBufferHeight);

    CreateRaytracingOutputResource();
    CreateDebugResources();

    if (m_enableUI)
    {
        if (!m_uiLayer)
        {
            m_uiLayer = make_unique<UILayer>(Sample::FrameCount, device, commandQueue);
        }
        m_uiLayer->Resize(renderTargets, m_width, m_height);
    }
}

// Release resources that are dependent on the size of the main window.
void D3D12RaytracingRealTimeDenoisedAmbientOcclusion::ReleaseWindowSizeDependentResources()
{
    if (m_enableUI)
    {
        m_uiLayer.reset();
    }
    m_raytracingOutput.resource.Reset();
}

// Release all resources that depend on the device.
void D3D12RaytracingRealTimeDenoisedAmbientOcclusion::ReleaseDeviceDependentResources()
{
    EngineProfiling::ReleaseDevice();
}

void D3D12RaytracingRealTimeDenoisedAmbientOcclusion::RecreateD3D()
{
    // Give GPU a chance to finish its execution in progress.
    try
    {
        m_deviceResources->WaitForGpu();
    }
    catch (HrException&)
    {
        // Do nothing, currently attached adapter is unresponsive.
    }
    m_deviceResources->HandleDeviceLost();
}

// Render the scene.
void D3D12RaytracingRealTimeDenoisedAmbientOcclusion::OnRender()
{
    if (!m_deviceResources->IsWindowVisible())
    {
        return;
    }

    if (m_renderOnce && m_framesToRender == 0)
    {
        return;
    }
    m_framesToRender--;

    auto commandList = m_deviceResources->GetCommandList();

    // Begin frame.
    m_deviceResources->Prepare();
    EngineProfiling::BeginFrame(commandList);        
    for (UINT i = 0; i < Sample_GPUTime::Count; i++)
    {
        m_sampleGpuTimes[i].BeginFrame(commandList);
    }

    // Render frame.
    {
        ScopedTimer _prof(L"Render", commandList);

        // Acceleration structure update.
        m_scene.OnRender();

        // Pathracing
        {
            m_sampleGpuTimes[Sample_GPUTime::Pathtracing].Start(commandList);
            m_pathtracer.Run(m_scene);
            m_sampleGpuTimes[Sample_GPUTime::Pathtracing].Stop(commandList);
        }

        // RTAO
        {
            ScopedTimer _prof(L"RTAO", commandList);

            GpuResource* GBufferResources = m_pathtracer.GBufferResources(RTAO_Args::QuarterResAO);

            // Raytracing
            {
                m_sampleGpuTimes[Sample_GPUTime::AOraytracing].Start(commandList);
                m_RTAO.Run(
                    m_scene.AccelerationStructure()->GetTopLevelASResource()->GetGPUVirtualAddress(),
                    GBufferResources[GBufferResource::HitPosition].gpuDescriptorReadAccess,
                    GBufferResources[GBufferResource::SurfaceNormalDepth].gpuDescriptorReadAccess,
                    GBufferResources[GBufferResource::AOSurfaceAlbedo].gpuDescriptorReadAccess);
                m_sampleGpuTimes[Sample_GPUTime::AOraytracing].Stop(commandList);
            }

            // Denoising
            {
                m_sampleGpuTimes[Sample_GPUTime::AOdenoising].Start(commandList);
                m_denoiser.Run(m_pathtracer, m_RTAO);
                m_sampleGpuTimes[Sample_GPUTime::AOdenoising].Stop(commandList);
            }
        }
            
        // Composition
        m_composition.Render(&m_raytracingOutput, m_scene, m_pathtracer, m_RTAO, m_denoiser, m_width, m_height);

        // UILayer will transition backbuffer to a present state.
        CopyRaytracingOutputToBackbuffer(m_enableUI ? D3D12_RESOURCE_STATE_RENDER_TARGET : D3D12_RESOURCE_STATE_PRESENT);
    }

    // End frame.
    for (UINT i = 0; i < Sample_GPUTime::Count; i++)
    {
        m_sampleGpuTimes[i].EndFrame(commandList);
    }
    EngineProfiling::EndFrame(commandList);
    m_deviceResources->ExecuteCommandList();

    // UI overlay.
    if (m_enableUI)
    {
        m_uiLayer->Render(m_deviceResources->GetCurrentFrameIndex());
    }

    m_deviceResources->Present(D3D12_RESOURCE_STATE_PRESENT, m_syncInterval);
}

// Compute the average frames per second and million rays per second.
void D3D12RaytracingRealTimeDenoisedAmbientOcclusion::CalculateFrameStats()
{
    static int frameCnt = 0;
    static double prevTime = 0.0f;
    double totalTime = m_scene.Timer().GetTotalSeconds();

    frameCnt++;

    // Compute averages over one second period.
    if ((totalTime - prevTime) >= 1.0f)
    {
        float diff = static_cast<float>(totalTime - prevTime);
        m_fps = static_cast<float>(frameCnt) / diff; // Normalize to an exact second.

        frameCnt = 0;
        prevTime = totalTime;

        // Display partial UI on the window title bar if UI is disabled.
 
        wstringstream windowText;
        windowText << setprecision(2) << fixed
            << L"    fps: " << m_fps 
            << L"    GPU[" << m_deviceResources->GetAdapterID() << L"]: " << m_deviceResources->GetAdapterDescription();
        SetCustomWindowText(windowText.str().c_str());
    }
}

// Handle OnSizeChanged message event.
void D3D12RaytracingRealTimeDenoisedAmbientOcclusion::OnSizeChanged(UINT width, UINT height, bool minimized)
{
    UpdateForSizeChange(width, height);

    if (!m_deviceResources->WindowSizeChanged(width, height, minimized))
    {
        return;
    }
}