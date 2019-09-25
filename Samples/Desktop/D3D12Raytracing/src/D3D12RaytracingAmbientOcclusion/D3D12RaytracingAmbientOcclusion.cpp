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
#include "D3D12RaytracingAmbientOcclusion.h"
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
    D3D12RaytracingAmbientOcclusion* g_pSample = nullptr;
    D3D12RaytracingAmbientOcclusion& instance()
    {
        return *g_pSample;
    }

    GpuResource g_debugOutput[2];

    void OnRecreateRaytracingResources(void*)
    {
        g_pSample->RequestRecreateRaytracingResources();
    }

    D3D12RaytracingAmbientOcclusion::D3D12RaytracingAmbientOcclusion(UINT width, UINT height, wstring name) :
        DXSample(width, height, name),
        m_isSceneInitializationRequested(false),
        m_isRecreateRaytracingResourcesRequested(false)
    {
        ThrowIfFalse(g_pSample == nullptr, L"There can be only one sample object instance.");
        g_pSample = this;
        UpdateForSizeChange(width, height);
        m_generatorURNG.seed(1729);
    }

    void D3D12RaytracingAmbientOcclusion::OnInit()
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
            FrameCount,
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

    D3D12RaytracingAmbientOcclusion::~D3D12RaytracingAmbientOcclusion()
    {
        GameInput::Shutdown();
    }

    void D3D12RaytracingAmbientOcclusion::WriteProfilingResultsToFile()
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
    void D3D12RaytracingAmbientOcclusion::CreateDeviceDependentResources()
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
    void D3D12RaytracingAmbientOcclusion::CreateRaytracingOutputResource()
    {
        auto device = m_deviceResources->GetD3DDevice();
        auto backbufferFormat = m_deviceResources->GetBackBufferFormat();

        CreateRenderTargetResource(device, backbufferFormat, m_width, m_height, m_cbvSrvUavHeap.get(), &m_raytracingOutput, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    }

    void D3D12RaytracingAmbientOcclusion::CreateDebugResources()
    {
        auto device = m_deviceResources->GetD3DDevice();
        auto backbufferFormat = m_deviceResources->GetBackBufferFormat();

        DXGI_FORMAT debugFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
        
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

    void D3D12RaytracingAmbientOcclusion::CreateAuxilaryDeviceResources()
    {
        auto device = m_deviceResources->GetD3DDevice();
        auto commandQueue = m_deviceResources->GetCommandQueue();
        auto commandList = m_deviceResources->GetCommandList();

        EngineProfiling::RestoreDevice(device, commandQueue, FrameCount);
        
        for (UINT i = 0; i < Sample_GPUTime::Count; i++)
        {
            m_sampleGpuTimes[i].RestoreDevice(device, m_deviceResources->GetCommandQueue(), FrameCount);
            m_sampleGpuTimes[i].SetAvgRefreshPeriodMS(500);
        }
    }

    void D3D12RaytracingAmbientOcclusion::CreateDescriptorHeaps()
    {
        auto device = m_deviceResources->GetD3DDevice();

        // Allocate large enough number of descriptors.
        UINT NumDescriptors = 10000;
        m_cbvSrvUavHeap = make_shared<DX::DescriptorHeap>(device, NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
    
    void D3D12RaytracingAmbientOcclusion::OnKeyDown(UINT8 key)
    {
        m_scene.OnKeyDown(key);

        float fValue;
        // ToDoF call componanet's handlers5
        switch (key)
        {
        case VK_ESCAPE:
            throw HrException(E_APPLICATION_EXITING);
        case 'L':
            m_scene.ToggleAnimateLight();
            break;
        case 'C':
            m_scene.ToggleAnimateCamera();
            break;
        case 'T':
            m_scene.ToggleAnimateScene();
            break;
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
        case VK_NUMPAD5:
            m_renderOnce = !m_renderOnce;
            m_framesToRender = m_renderOnce ? 1 : 0;
            break;
            break;
        case VK_NUMPAD7:
            Denoiser_Args::LowTspp.SetValue(true);
            break;
        case VK_RETURN:
            Composition_Args::AOEnabled.Bang();
            break;

        default:
            break;
        }
    }

    // Update frame-based values.
    void D3D12RaytracingAmbientOcclusion::OnUpdate()
    {
#if ENABLE_PROFILING
        if (m_isProfiling)
        {
            if (m_numRemainingFramesToProfile == 0)
            {
                m_isProfiling = false;
                m_timer.SetFixedTimeStep(false);
                WriteProfilingResultsToFile();
                m_animateCamera = false;
            }
            else
            {
                m_numRemainingFramesToProfile--;
            }
        }
#endif

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

    // ToDoF extend or remove
    // Parse supplied command line args.
    void D3D12RaytracingAmbientOcclusion::ParseCommandLineArgs(WCHAR* argv[], int argc)
    {
        DXSample::ParseCommandLineArgs(argv, argc);
    }

    // Copy the raytracing output to the backbuffer.
    void D3D12RaytracingAmbientOcclusion::CopyRaytracingOutputToBackbuffer(D3D12_RESOURCE_STATES outRenderTargetState)
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

    void D3D12RaytracingAmbientOcclusion::UpdateUI()
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

            // ToDoF remove/cleanup
            // Do GPUtimers/scopedtimers in this cpp and use those?
            if (m_isProfiling)
            {
                set<wstring> profileMarkers = {
                        L"DownsampleGBuffer",
                        L"RTAO_Root",
                        L"TemporalSupersamplingReverseReproject",
                        L"[Sorted]CalculateAmbientOcclusion",
                        L"CalculateAmbientOcclusion_Root",
                        L"Adaptive Ray Gen",
                        L"Sort Rays",
                        L"AO DispatchRays 2D",
                        L"RenderPass_TemporalSupersamplingBlendWithCurrentFrame",
                        L"DenoiseAO",
                        L"Upsample AO",
                        L"Low-Tspp Multi-pass blur"
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
    void D3D12RaytracingAmbientOcclusion::CreateWindowSizeDependentResources()
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
                m_uiLayer = make_unique<UILayer>(FrameCount, device, commandQueue);
            }
            m_uiLayer->Resize(renderTargets, m_width, m_height);
        }
    }

    // Release resources that are dependent on the size of the main window.
    void D3D12RaytracingAmbientOcclusion::ReleaseWindowSizeDependentResources()
    {
        if (m_enableUI)
        {
            m_uiLayer.reset();
        }
        m_raytracingOutput.resource.Reset();
    }

    // Release all resources that depend on the device.
    void D3D12RaytracingAmbientOcclusion::ReleaseDeviceDependentResources()
    {
        EngineProfiling::ReleaseDevice();
    }

    void D3D12RaytracingAmbientOcclusion::RecreateD3D()
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
    void D3D12RaytracingAmbientOcclusion::OnRender()
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

        // ToDoF finalize UI tree
        ScopedTimer _prof(L"Render", commandList);
        {
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
    void D3D12RaytracingAmbientOcclusion::CalculateFrameStats()
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
    void D3D12RaytracingAmbientOcclusion::OnSizeChanged(UINT width, UINT height, bool minimized)
    {
        UpdateForSizeChange(width, height);

        if (!m_deviceResources->WindowSizeChanged(width, height, minimized))
        {
            return;
        }
    }
}