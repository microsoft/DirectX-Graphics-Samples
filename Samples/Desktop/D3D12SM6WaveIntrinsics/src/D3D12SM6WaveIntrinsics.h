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

using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

class UILayer;

class D3D12SM6WaveIntrinsics : public DXSample
{
public:
    D3D12SM6WaveIntrinsics(UINT width, UINT height, std::wstring name);
    inline ID3D12Device* GetDevice() { return m_d3d12Device.Get(); }
    inline ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue.Get(); }
    inline ID3D12Resource* GetUIRenderTarget() { return m_uiRenderTarget.Get(); }
    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();
    virtual void OnKeyDown(UINT8 key);
    virtual void OnSizeChanged(UINT width, UINT height, bool minimized);
    virtual void OnMouseMove(UINT x, UINT y);
    virtual void OnLeftButtonDown(UINT x, UINT y);
    virtual void OnLeftButtonUp(UINT x, UINT y);
    virtual IDXGISwapChain* GetSwapchain() { return m_swapChain.Get(); }

private:
    // In this sample we overload the meaning of FrameCount to mean both the maximum
    // number of frames that will be queued to the GPU at a time, as well as the number
    // of back buffers in the DXGI swap chain. For the majority of applications, this
    // is convenient and works well. However, there will be certain cases where an
    // application may want to queue up more frames than there are back buffers
    // available.
    // It should be noted that excessive buffering of frames dependent on user input
    // may result in noticeable latency in your app.
    static const UINT FrameCount = 2;

    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT4 color;
    };

    struct Vertex2
    {
        XMFLOAT3 position;
        XMFLOAT2 uv;
    };

    struct SceneConstantBuffer
    {
        XMMATRIX orthProjMatrix;
        XMFLOAT2 mousePosition;
        XMFLOAT2 resolution;
        float time;
        UINT renderingMode;
        UINT laneSize;
        UINT padding;
    };

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_d3d12Device;
    ComPtr<ID3D12Resource> m_renderPass1RenderTargets;
    ComPtr<ID3D12Resource> m_renderPass2RenderTargets[FrameCount];
    ComPtr<ID3D12Resource> m_uiRenderTarget;
    ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12RootSignature> m_renderPass1RootSignature;
    ComPtr<ID3D12RootSignature> m_renderPass2RootSignature;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_cbSrvHeap;
    ComPtr<ID3D12PipelineState> m_renderPass1PSO;
    ComPtr<ID3D12PipelineState> m_renderPass2PSO;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    UINT m_rtvDescriptorSize;
    UINT m_cbSrvDescriptorSize;

    // App resources.
    ComPtr<ID3D12Resource> m_renderPass1VertexBuffer;
    ComPtr<ID3D12Resource> m_renderPass2VertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_renderPass1VertexBufferView;
    D3D12_VERTEX_BUFFER_VIEW m_renderPass2VertexBufferView;
    ComPtr<ID3D12Resource> m_constantBuffer;
    SceneConstantBuffer m_constantBufferData;
    UINT8* m_pCbSrvDataBegin;
    float m_mousePosition[2];
    bool m_mouseLeftButtonDown;

    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValues[FrameCount];

    // Shader Model 6 feature support result
    D3D12_FEATURE_DATA_D3D12_OPTIONS1 m_WaveIntrinsicsSupport;
    UINT m_rendermode;

    // UILayer
    std::shared_ptr<UILayer> m_uiLayer;

    void CreateDevice(const ComPtr<IDXGIFactory4>& factory);
    void LoadPipeline();
    void LoadAssets();
    void RestoreD3DResources();
    void ReleaseD3DResources();
    void LoadSizeDependentResources();
    void MoveToNextFrame();
    void WaitForGpu();
    void RenderScene();
    void RenderUI();
};
