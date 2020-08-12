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
#include "Model.h"
#include "StepTimer.h"
#include "SimpleCamera.h"

#include "CullDataVisualizer.h"
#include "FrustumVisualizer.h"

using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

class D3D12MeshletCull : public DXSample
{
public:
    D3D12MeshletCull(UINT width, UINT height, std::wstring name);

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();
    virtual void OnKeyDown(UINT8 key);
    virtual void OnKeyUp(UINT8 key);

private:
    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();
    void MoveToNextFrame();
    void WaitForGpu();

    XMVECTOR GetSamplePoint() const;
    void Pick();

private:
    struct SceneObject
    {
        Model                  Model;
        DirectX::XMFLOAT4X4    World;
        ComPtr<ID3D12Resource> InstanceResource;
        void*                  InstanceData;
        uint32_t               Flags;
    };

private:
    static const uint32_t FrameCount = 2;

    // Pipeline objects.
    CD3DX12_VIEWPORT                   m_viewport;
    CD3DX12_RECT                       m_scissorRect;

    ComPtr<ID3D12Device2>              m_device;
    ComPtr<IDXGISwapChain3>            m_swapChain;
    ComPtr<ID3D12Resource>             m_renderTargets[FrameCount];
    ComPtr<ID3D12Resource>             m_depthStencil;
    ComPtr<ID3D12CommandAllocator>     m_commandAllocators[FrameCount];
    ComPtr<ID3D12CommandQueue>         m_commandQueue;
    ComPtr<ID3D12GraphicsCommandList6> m_commandList;

    ComPtr<ID3D12DescriptorHeap>       m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap>       m_dsvHeap;

    uint32_t                           m_rtvDescriptorSize;
    uint32_t                           m_dsvDescriptorSize;

    ComPtr<ID3D12Resource>             m_constantBuffer;
    void*                              m_constantsData;

    StepTimer                          m_timer;
    SimpleCamera                       m_mainCam;
    SimpleCamera                       m_debugCam;
    float                              m_fovy;

    ComPtr<ID3D12RootSignature>        m_rootSignature;
    ComPtr<ID3D12PipelineState>        m_pipelineState;

    std::vector<SceneObject>           m_objects;

    // Synchronization objects.
    uint32_t                           m_frameIndex;
    uint32_t                           m_frameCounter;
    HANDLE                             m_fenceEvent;
    ComPtr<ID3D12Fence>                m_fence;
    UINT64                             m_fenceValues[FrameCount];

    FrustumVisualizer                  m_frustumDraw;
    CullDataVisualizer                 m_cullDataDraw;

    uint32_t                           m_highlightedIndex;
    uint32_t                           m_selectedIndex;
    bool                               m_drawMeshlets;
    bool                               m_moveCam;
};
