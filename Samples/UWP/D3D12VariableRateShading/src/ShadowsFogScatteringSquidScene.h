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

// ShadowsFogScatteringSquidScene
// Renders a scene with a squid and illumination from a point light. A postprocess pass 
// applies a fog effect and an analytical in-scattering model. 
// Uses multithreading to draw scene geometry.
// Limitations\future work: 
// - shadow for a point light is generated for one face only - i.e. same as for a spotlight.
// - no volumetric shadows

#pragma once

#include "SquidRoom.h"
#include "Camera.h"
#include "DXSampleHelper.h"

using namespace DirectX;
class FrameResource;
class DXSample;

#define SINGLETHREADED FALSE

static const UINT NumNullSrvs = 2; // Null descriptors at the start of the heap.
static const UINT NumContexts = 3;

// Currently the rendering code can only handle a single point light.
static const UINT NumLights = 1; // Keep this in sync with "ShadowsAndScenePass.hlsl".

// Command list submissions from main thread.
static const int CommandListCount = 3;
static const int CommandListPre = 0;
static const int CommandListMid = 1;
static const int CommandListPost = 2;

namespace SceneEnums
{
    namespace RenderPass {
        enum Value { Scene = 0, Shadow, Postprocess, Count };
    }

    namespace DepthGenPass {
        enum Value { Scene = 0, Shadow, Count };
    }

    namespace RootSignature {
        enum { ShadowPass = 0, ScenePass, PostprocessPass, Count };
    };

    namespace VertexBuffer {
        enum Value { SceneGeometry = 0, ScreenQuad, Count };
    }

    namespace Timestamp {
        enum Value { ScenePass = 0, PostprocessPass, Count };
    }
}

struct LightState
{
    XMFLOAT4 position;
    XMFLOAT4 direction;
    XMFLOAT4 color;
    XMFLOAT4 falloff;
    XMFLOAT4X4 view;
    XMFLOAT4X4 projection;
};

struct SceneConstantBuffer
{
    XMFLOAT4X4 model;
    XMFLOAT4X4 view;
    XMFLOAT4X4 projection;
    XMFLOAT4 ambientColor;
    BOOL sampleShadowMap;
    BOOL padding[3];        // Must be aligned to be made up of N float4s.
    LightState lights[NumLights];
    XMFLOAT4 viewport;
    XMFLOAT4 clipPlane;
};

struct PostprocessConstantBuffer
{
    XMFLOAT4 lightPosition;
    XMFLOAT4 cameraPosition;
    XMFLOAT4X4 viewInverse;
    XMFLOAT4X4 projInverse;
    XMFLOAT4X4 viewProjInverseAtNearZ1;
    float fogDensity;
};

struct InputState
{
    bool rightArrowPressed;
    bool leftArrowPressed;
    bool upArrowPressed;
    bool downArrowPressed;
    bool animate;
};

class ShadowsFogScatteringSquidScene
{
public:
    ShadowsFogScatteringSquidScene(UINT frameCount, DXSample* pSample);
    virtual ~ShadowsFogScatteringSquidScene();

    virtual void Initialize(ID3D12Device* pDevice, ID3D12CommandQueue* pDirectCommandQueue, ID3D12GraphicsCommandList* pCommandList, UINT frameIndex);
    virtual void LoadSizeDependentResources(ID3D12Device* pDevice, ComPtr<ID3D12Resource>* ppRenderTargets, UINT width, UINT height);
    virtual void ReleaseSizeDependentResources();
    void SetFrameIndex(UINT frameIndex);
    void ReleaseD3DObjects();    
    void KeyDown(UINT8 key);
    void KeyUp(UINT8 key);
    virtual void Update(double elapsedTime);
    virtual void Render(ID3D12CommandQueue* pCommandQueue, bool setBackbufferReadyForPresent);
    static ShadowsFogScatteringSquidScene* Get() { return s_app; }

    float m_fogDensity;

    float GetScenePassGPUTimeInMs() const;
    float GetPostprocessPassGPUTimeInMs() const;

protected:
    UINT m_frameCount;

    struct Vertex
    {
        XMFLOAT4 position;
    };

    ID3D12CommandList * m_batchSubmit[NumContexts * 2 + CommandListCount];   // *2: shadowCommandLists, sceneCommandLists

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;

    // D3D objects.
    ComPtr<ID3D12GraphicsCommandList> m_commandLists[CommandListCount];
    ComPtr<ID3D12GraphicsCommandList> m_shadowCommandLists[NumContexts];
    ComPtr<ID3D12GraphicsCommandList> m_sceneCommandLists[NumContexts];
    std::vector<ComPtr<ID3D12Resource>> m_renderTargets;
    ComPtr<ID3D12Resource> m_depthTextures[SceneEnums::DepthGenPass::Count];
    ComPtr<ID3D12RootSignature> m_rootSignatures[SceneEnums::RootSignature::Count];
    ComPtr<ID3D12PipelineState> m_pipelineStates[SceneEnums::RenderPass::Count];
    ComPtr<ID3D12Resource> m_vertexBuffers[SceneEnums::VertexBuffer::Count];
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferViews[SceneEnums::VertexBuffer::Count];
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
    ComPtr<ID3D12Resource> m_textures[_countof(SampleAssets::Textures)];
    ComPtr<ID3D12Resource> m_textureUploads[_countof(SampleAssets::Textures)];
    ComPtr<ID3D12Resource> m_indexBuffer;
    ComPtr<ID3D12Resource> m_indexBufferUpload;
    ComPtr<ID3D12Resource> m_vertexBufferUpload;

    // Heap objects.
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    ComPtr<ID3D12DescriptorHeap> m_cbvSrvHeap;
    ComPtr<ID3D12DescriptorHeap> m_samplerHeap;
    UINT m_rtvDescriptorSize;
    UINT m_cbvSrvDescriptorSize;
    D3D12_CPU_DESCRIPTOR_HANDLE m_depthDsvs[SceneEnums::DepthGenPass::Count];
    D3D12_CPU_DESCRIPTOR_HANDLE m_depthSrvCpuHandles[SceneEnums::DepthGenPass::Count];
    D3D12_GPU_DESCRIPTOR_HANDLE m_depthSrvGpuHandles[SceneEnums::DepthGenPass::Count];
    
    // Frame resources.
    std::vector<std::unique_ptr<FrameResource>> m_frameResources;
    FrameResource* m_pCurrentFrameResource;
    UINT m_frameIndex;
    SceneConstantBuffer m_shadowConstantBuffer; // Shadow copy.
    SceneConstantBuffer m_sceneConstantBuffer; // Shadow copy.
    PostprocessConstantBuffer m_postprocessConstantBuffer; // Shadow copy.

    // App resources.
    DXSample*  m_pSample;
    InputState m_keyboardInput;
    LightState m_lights[NumLights];
    Camera m_lightCameras[NumLights];
    Camera m_camera;
    static const float s_clearColor[4];

    // Window state
    bool m_windowVisible;
    bool m_windowedMode;

    // Thread synchronization objects.
    struct ThreadParameter
    {
        int threadIndex;
    };
    ThreadParameter m_threadParameters[NumContexts];
    HANDLE m_workerBeginRenderFrame[NumContexts];
    HANDLE m_workerFinishShadowPass[NumContexts];
    HANDLE m_workerFinishedScenePass[NumContexts];
    HANDLE m_threadHandles[NumContexts];
    static ShadowsFogScatteringSquidScene* s_app;        // Singleton object so that worker threads can share members.

    virtual void UpdateConstantBuffers(); // Updates the shadow copies of the constant buffers.
    virtual void CommitConstantBuffers(); // Commits the shadows copies of the constant buffers to GPU-visible memory for the current frame.
    
    virtual void DrawInScattering(ID3D12GraphicsCommandList* pCommandList, const D3D12_CPU_DESCRIPTOR_HANDLE& renderTargetHandle);

    void WorkerThread(int threadIndex);
    void LoadContexts();

    virtual void ShadowPass(ID3D12GraphicsCommandList* pCommandList, int threadIndex);
    virtual void ScenePass(ID3D12GraphicsCommandList* pCommandList, int threadIndex);
    virtual void PostprocessPass(ID3D12GraphicsCommandList* pCommandList);

    virtual void BeginFrame();
    virtual void MidFrame();
    virtual void EndFrame(bool setBackbufferReadyForPresent);
    virtual void InitializeCameraAndLights();

    virtual void CreateDescriptorHeaps(ID3D12Device* pDevice);
    virtual void CreateCommandLists(ID3D12Device* pDevice);
    virtual void CreateRootSignatures(ID3D12Device* pDevice);
    virtual void CreatePipelineStates(ID3D12Device* pDevice);
    virtual void CreateFrameResources(ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue);
    virtual void CreateAssetResources(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
    virtual void CreatePostprocessPassResources(ID3D12Device* pDevice);
    virtual void CreateSamplers(ID3D12Device* pDevice);

    inline HRESULT CreateDepthStencilTexture2D(
        ID3D12Device* pDevice,
        UINT width,
        UINT height,
        DXGI_FORMAT typelessFormat,
        DXGI_FORMAT dsvFormat,
        DXGI_FORMAT srvFormat,
        ID3D12Resource** ppResource,
        D3D12_CPU_DESCRIPTOR_HANDLE cpuDsvHandle,
        D3D12_CPU_DESCRIPTOR_HANDLE cpuSrvHandle,
        D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_DEPTH_WRITE,
        float initDepthValue = 1.0f,
        UINT8 initStencilValue = 0)
    {
        try
        {
            *ppResource = nullptr;
    
            CD3DX12_RESOURCE_DESC texDesc(
                D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                0,
                width,
                height,
                1,
                1,
                typelessFormat,
                1,
                0,
                D3D12_TEXTURE_LAYOUT_UNKNOWN,
                D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
    
            ThrowIfFailed(pDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                D3D12_HEAP_FLAG_NONE,
                &texDesc,
                initState,
                &CD3DX12_CLEAR_VALUE(dsvFormat, initDepthValue, initStencilValue), // Performance tip: Tell the runtime at resource creation the desired clear value.
                IID_PPV_ARGS(ppResource)));
    
            // Create a depth stencil view (DSV).
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = dsvFormat;
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = 0;
            pDevice->CreateDepthStencilView(*ppResource, &dsvDesc, cpuDsvHandle);
    
            // Create a shader resource view (SRV).
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = srvFormat;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            pDevice->CreateShaderResourceView(*ppResource, &srvDesc, cpuSrvHandle);
        }
        catch (HrException& e)
        {
            SAFE_RELEASE(*ppResource);
            return e.Error();
        }
        return S_OK;
    }

    inline CD3DX12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferRtvCpuHandle()
    {
        return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    }

    // How much to scale each dimension in the world.
    inline float GetWorldScale() const
    {
        return 0.1f;
    }

    virtual UINT GetNumRtvDescriptors() const
    {
        return m_frameCount;
    }

    virtual UINT GetNumCbvSrvUavDescriptors() const
    {
        return NumNullSrvs + _countof(m_depthSrvCpuHandles) + _countof(SampleAssets::Textures);
    }
};