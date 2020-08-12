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
#include "Shared.h"

using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

class D3D12DynamicLOD : public DXSample
{
public:
    D3D12DynamicLOD(UINT width, UINT height, std::wstring name);

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
    void RegenerateInstances();

private:
    static const UINT FrameCount = 2;

    enum class RenderMode
    {
        Flat,
        Meshlets,
        LOD,
        Count
    };

    // Pipeline objects.
    CD3DX12_VIEWPORT                    m_viewport;
    CD3DX12_RECT                        m_scissorRect;
        
    uint32_t                            m_rtvDescriptorSize;
    uint32_t                            m_dsvDescriptorSize;
    uint32_t                            m_srvDescriptorSize;
    
    ComPtr<ID3D12Device2>               m_device;
    ComPtr<IDXGISwapChain3>             m_swapChain;
    ComPtr<ID3D12Resource>              m_renderTargets[FrameCount];
    ComPtr<ID3D12Resource>              m_depthStencil;
    ComPtr<ID3D12CommandAllocator>      m_commandAllocators[FrameCount];
    ComPtr<ID3D12CommandQueue>          m_commandQueue;

    // Synchronization objects.
    uint32_t                            m_frameIndex;
    uint32_t                            m_frameCounter;
    HANDLE                              m_fenceEvent;
    ComPtr<ID3D12Fence>                 m_fence;
    UINT64                              m_fenceValues[FrameCount];
        
    ComPtr<ID3D12DescriptorHeap>        m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap>        m_dsvHeap;
    ComPtr<ID3D12DescriptorHeap>        m_srvHeap;
    
    ComPtr<ID3D12RootSignature>         m_rootSignature;
    ComPtr<ID3D12PipelineState>         m_pipelineState;
    ComPtr<ID3D12Resource>              m_constantBuffer;
    ComPtr<ID3D12Resource>              m_instanceBuffer;
    ComPtr<ID3D12Resource>              m_instanceUpload;

    ComPtr<ID3D12GraphicsCommandList6>  m_commandList;
    Constants*                          m_constantData;
    Instance*                           m_instanceData;

    StepTimer                           m_timer;
    SimpleCamera                        m_camera;
    std::vector<Model>                  m_lods;
    RenderMode                          m_renderMode;
    uint32_t                            m_instanceLevel;

    uint32_t                            m_instanceCount;
    bool                                m_updateInstances;

private:
    static const float    c_fovy;

    static const wchar_t* c_lodFilenames[];

    static const wchar_t* c_ampShaderFilename;
    static const wchar_t* c_meshShaderFilename;
    static const wchar_t* c_pixelShaderFilename;
};
