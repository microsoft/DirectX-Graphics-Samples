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
#include "va/va.h"
#include "va/va_win32.h"

class VAException : public std::runtime_error
{
public:
    VAException(VAStatus vas) : std::runtime_error(VAStatusToString(vas)), m_vas(vas) {}
    VAStatus Error() const { return m_vas; }
private:
    inline std::string VAStatusToString(VAStatus vas)
    {
        char s_str[64] = {};
        sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(vas));
        return std::string(s_str);
    }
    const VAStatus m_vas;
};

inline void ThrowIfFailed(VAStatus va_status, const char* func)
{
    if (va_status != VA_STATUS_SUCCESS)
    {
        printf("%s:%s (%d) failed with VAStatus %x,exit\n", __func__, func, __LINE__, va_status);   \
        throw VAException(va_status);
    }
}

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

class D3D12HelloVADecode : public DXSample
{
public:
    D3D12HelloVADecode(UINT width, UINT height, std::wstring name);

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();

private:
    //
    // D3D12 Objects
    //
    static const UINT FrameCount = 2;

    // Pipeline objects.
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<IDXGIAdapter1> m_adapter;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    UINT m_rtvDescriptorSize = 0;

    // Synchronization objects.
    UINT m_frameIndex = 0;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue = 0;

    void LoadD3D12Pipeline();
    void LoadAssets();
    void PopulateCommandList();
    void WaitForPreviousFrame();

    //
    // VA Objects
    //

    // Common VA objects / methods
    VADisplay m_vaDisplay = { };
    VASurfaceID m_VARenderTargets[FrameCount] = { };
    VASurfaceID m_VASurfaceNV12 = 0;
    VAProcPipelineCaps m_ProcPipelineCaps = { };
    void LoadVAPipeline();
    void InitVADisplay();
    void ImportRenderTargetsToVA();
    void PerformVAWorkload();
    void DestroyVA();
    void CreateVASurfaces();
    
    // Video Processor
    VAConfigID m_VAProcConfigId = 0;
    void EnsureVAProcSupport();
    void InitVAProcContext();

    // Context for color rgb to yuv conversion
    VAContextID m_vaColorConvCtx = 0;
    VABufferID m_vaColorConvBuf = 0;

    void PerformVABlit(
        VAContextID context,
        VABufferID buffer,
        VASurfaceID* pInSurfaces,
        UINT inSurfacesCount,
        VARectangle* pSrcRegions,
        VARectangle* pDstRegions,
        VASurfaceID dstSurface,
        float alpha);
    void DestroyVAProc();

    // Video Decode
    VAConfigID m_VADecConfigId = 0;
    VAContextID m_VADecContextId = 0;
    VABufferID m_VADecPipelineBufferId[4];
    static const VABufferID VA_H264DEC_BUFFER_INDEX_PIC = 0;
    static const VABufferID VA_H264DEC_BUFFER_INDEX_QMATRIX = 1;
    static const VABufferID VA_H264DEC_BUFFER_INDEX_COMPRESSED_BIT = 2;
    static const VABufferID VA_H264DEC_BUFFER_INDEX_SLICE = 3;

    void EnsureVADecSupport();
    void InitVADecContext();
    void PerformVADecodeFrame(VASurfaceID surface);
    void DestroyVADec();
};
