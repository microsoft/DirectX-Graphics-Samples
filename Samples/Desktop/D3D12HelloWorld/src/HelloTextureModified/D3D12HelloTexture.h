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
#include <chrono>
#include "DXSample.h"

using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

class D3D12HelloTexture : public DXSample
{
public:
    D3D12HelloTexture(UINT width, UINT height, std::wstring name);

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();

private:
    static const UINT FrameCount = 2;
    static const UINT TextureWidth = 256;
    static const UINT TextureHeight = 256;
    static const UINT TexturePixelSize = 4;    // The number of bytes used to represent a pixel in the texture.

	static const UINT TextureCount = 1023;
    static const UINT TextureTypes = 1000; // Color Type : 0-9

    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT2 uv;
    };

    struct Material
    {
        UINT textureIndex;
		float padding[3]; // 16byte alignment
	};

	struct SceneConstantBuffer
	{
        XMFLOAT4 offset;
		Material material;
        float padding[64-4-4];
	};
	static_assert(sizeof(SceneConstantBuffer) % 256 == 0, "CB size must be 256-byte aligned.");

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_heap; // CBV/SRV/UAV heap
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    UINT m_rtvDescriptorSize;
	UINT m_descriptorSize;
    UINT m_nextFrameIndex = 0;

    UINT m_texIndex[TextureCount] = {};
    UINT m_nextFreeIndex = 0;

    SceneConstantBuffer m_constantBufferData;

    // App resources.
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    std::vector<ComPtr<ID3D12Resource>> m_texture;
    ComPtr<ID3D12Resource> m_constantBuffer;
    UINT8* m_pCbvDataBegin;

    std::chrono::steady_clock::time_point m_prevTime;
    UINT m_texIndexId = 0;

    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;

    void LoadPipeline();
    void LoadAssets();
    std::vector<UINT8> GenerateTextureData();
    void PopulateCommandList();
    void WaitForPreviousFrame();

	UINT AllocateTextureSRV(ID3D12Resource* texture);
};
