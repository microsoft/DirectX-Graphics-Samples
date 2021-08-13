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

// Illustrate how to render to a target that supports HDR.
class D3D12HDR : public DXSample
{
public:
    D3D12HDR(UINT width, UINT height, std::wstring name);

    inline ID3D12Device* GetDevice() { return m_device.Get(); }
    inline ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue.Get(); }
    inline ID3D12Resource* GetUIRenderTarget() { return m_UIRenderTarget.Get(); }
    inline DXGI_FORMAT GetBackBufferFormat() { return m_swapChainFormats[m_currentSwapChainBitDepth]; }
    inline std::wstring GetDisplayCurve()
    {
        return m_rootConstants[DisplayCurve] == sRGB ? L"sRGB" : m_rootConstants[DisplayCurve] == ST2084 ? L"ST.2084" : L"Linear";
    }
    inline bool GetHDRSupport() { return m_hdrSupport; }
    inline float GetReferenceWhiteNits() { return m_referenceWhiteNits; }
    inline UINT GetHDRMetaDataPoolIndex() { return m_hdrMetaDataPoolIdx; }

    static const float HDRMetaDataPool[4][4];
    static const UINT FrameCount = 2;

protected:
    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnSizeChanged(UINT width, UINT height, bool minimized);
    virtual void OnWindowMoved(int xPos, int yPos);
    virtual void OnDestroy();
    virtual void OnKeyDown(UINT8 key);
    virtual void OnDisplayChanged();
    virtual IDXGISwapChain* GetSwapchain() { return m_swapChain.Get(); }

private:
    static const float ClearColor[4];
    static const UINT TrianglesVertexCount = 18;

    // Vertex definitions.
    struct GradientVertex
    {
        XMFLOAT3 position;
        XMFLOAT3 color;
    };

    struct TrianglesVertex
    {
        XMFLOAT3 position;
        XMFLOAT2 uv;
    };

    struct PresentVertex
    {
        XMFLOAT3 position;
        XMFLOAT2 uv;
    };

    struct DisplayChromaticities
    {
        float RedX;
        float RedY;
        float GreenX;
        float GreenY;
        float BlueX;
        float BlueY;
        float WhiteX;
        float WhiteY;
    };

    enum PipelineStates
    {
        GradientPSO = 0,
        PalettePSO,
        Present8bitPSO,
        Present10bitPSO,
        Present16bitPSO,
        PipelineStateCount
    };

    enum SwapChainBitDepth
    {
        _8 = 0,
        _10,
        _16,
        SwapChainBitDepthCount
    };

    enum RootConstants
    {
        ReferenceWhiteNits = 0,
        DisplayCurve,
        RootConstantsCount
    };

    enum DisplayCurve
    {
        sRGB = 0,    // The display expects an sRGB signal.
        ST2084,        // The display expects an HDR10 signal.
        None        // The display expects a linear signal.
    };


    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGIFactory4> m_dxgiFactory;
    ComPtr<IDXGISwapChain4> m_swapChain;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12Resource> m_intermediateRenderTarget;
    ComPtr<ID3D12Resource> m_UIRenderTarget;
    ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_srvHeap;
    UINT m_rtvDescriptorSize;
    UINT m_srvDescriptorSize;
    DXGI_FORMAT m_swapChainFormats[SwapChainBitDepthCount];
    DXGI_COLOR_SPACE_TYPE m_currentSwapChainColorSpace;
    SwapChainBitDepth m_currentSwapChainBitDepth;
    bool m_swapChainFormatChanged;
    DXGI_FORMAT m_intermediateRenderTargetFormat;
    UINT m_dxgiFactoryFlags;

    // App resources.
    ComPtr<ID3D12PipelineState> m_pipelineStates[PipelineStateCount];
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    ComPtr<ID3D12Resource> m_vertexBuffer;
    ComPtr<ID3D12Resource> m_vertexBufferUpload;
    D3D12_VERTEX_BUFFER_VIEW m_gradientVertexBufferView;
    D3D12_VERTEX_BUFFER_VIEW m_trianglesVertexBufferView;
    D3D12_VERTEX_BUFFER_VIEW m_presentVertexBufferView;
    UINT m_rootConstants[RootConstantsCount];
    float* m_rootConstantsF;
    bool m_updateVertexBuffer;
    std::shared_ptr<UILayer> m_uiLayer;
    bool m_enableUI = true;
    UINT m_hdrMetaDataPoolIdx = 0;

    // Color.
    bool m_hdrSupport = false;
    bool m_enableST2084 = false;
    float m_referenceWhiteNits = 80.0f;    // The reference brightness level of the display.

    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValues[FrameCount];

    // Track the state of the window.
    // If it's minimized the app may decide not to render frames.
    bool m_windowVisible;
    bool m_windowedMode;

    void LoadPipeline();
    void LoadAssets();
    void LoadSizeDependentResources();
    XMFLOAT3 TransformVertex(XMFLOAT2 point, XMFLOAT2 offset);
    void UpdateVertexBuffer();
    void RenderScene();
    void WaitForGpu();
    void MoveToNextFrame();
    void EnsureSwapChainColorSpace(SwapChainBitDepth d, bool enableST2084);
    void CheckDisplayHDRSupport();
    void SetHDRMetaData(float MaxOutputNits = 1000.0f, float MinOutputNits = 0.001f, float MaxCLL = 2000.0f, float MaxFALL = 500.0f);
    void UpdateSwapChainBuffer(UINT width, UINT height, DXGI_FORMAT format);
};
