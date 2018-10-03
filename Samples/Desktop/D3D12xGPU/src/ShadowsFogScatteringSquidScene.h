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

static const UINT NumNullSrvs = 2;        // Null descriptors at the start of the heap.
static const UINT NumContexts = 3;

// Currently the rendering code can only handle a single point light.
static const UINT NumLights = 1;        // Keep this in sync with "ShadowsAndScenePass.hlsl".
            

static const UINT NumDepthBuffers = 2;      // Shadow + Scene pass
static const UINT NumConstantBuffers = 3;   // Scene, shadows and postprocess

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
        enum { SceneAndShadowPass = 0, PostprocessPass, Count };
    };

    namespace VertexBuffer {
        enum Value { SceneGeometry = 0, ScreenQuad, Count };
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
};


struct PostprocessConstantBuffer
{
    XMFLOAT4 lightPosition;
    XMFLOAT4 cameraPosition;
    XMFLOAT4X4 viewInverse;
    XMFLOAT4X4 projInverse;
    XMFLOAT4X4 viewProjInverseAtNearZ1;
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
    ~ShadowsFogScatteringSquidScene();

    void Initialize(ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue, UINT frameIndex);
    void LoadSizeDependentResources(ID3D12Device* pDevice, ComPtr<ID3D12Resource>* ppRenderTargets, UINT width, UINT height);
    void ReleaseSizeDependentResources();
    void SetFrameIndex(UINT frameIndex);
    void ReleaseD3DObjects();    
    void KeyDown(UINT8 key);
    void KeyUp(UINT8 key);
    void Update(double elapsedTime);
    void Render(ID3D12CommandQueue* pCommandQueue, bool setBackbufferReadyForPresent);
    static ShadowsFogScatteringSquidScene* Get() { return s_app; }

private:
    UINT m_frameCount;

    struct Vertex
    {
        XMFLOAT4 position;
    };
    // Pipeline objects
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;

    // D3D objects.
    std::vector<ComPtr<ID3D12Resource>> m_renderTargets;
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
    
    // Frame resources.
    std::vector<std::unique_ptr<FrameResource>> m_frameResources;
    FrameResource* m_pCurrentFrameResource;
    UINT m_frameIndex;

    // App resources.
    DXSample*  m_pSample;
    InputState m_keyboardInput;
    LightState m_lights[NumLights];
    Camera m_lightCameras[NumLights];
    Camera m_camera;

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
    HANDLE m_workerFinishedRenderFrame[NumContexts];
    HANDLE m_threadHandles[NumContexts];
    static ShadowsFogScatteringSquidScene* s_app;        // Singleton object so that worker threads can share members.

    void WorkerThread(int threadIndex);
    void SetCommonPipelineState(ID3D12GraphicsCommandList* pCommandList);
    void LoadContexts();
    void PostprocessPass(ID3D12CommandQueue* pCommandQueue, bool setBackbufferReadyForPresent);    
    void BeginFrame();
    void MidFrame();
    void EndFrame();
    void InitializeCameraAndLights();
    void CreateDescriptorHeaps(ID3D12Device* pDevice);
    void CreateRootSignatures(ID3D12Device* pDevice);
    void CreatePipelineStates(ID3D12Device* pDevice);
    void CreateFrameResources(ID3D12Device* pDevice);
    void CreateAssetResources(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pAssetLoadingCmdList);
    void CreatePostprocessPassResources(ID3D12Device* pDevice);
    void CreateSamplers(ID3D12Device* pDevice);
};