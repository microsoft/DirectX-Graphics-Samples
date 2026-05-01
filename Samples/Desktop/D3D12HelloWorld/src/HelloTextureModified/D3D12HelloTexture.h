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
    static constexpr UINT kFrameCount = 2;
    static constexpr UINT kTextureWidth = 256;
    static constexpr UINT kTextureHeight = 256;
    static constexpr UINT kTexturePixelSize = 4;    // The number of bytes used to represent a pixel in the texture.

	static constexpr UINT kHeapDescriptorCount = 1024;
    static constexpr UINT kTextureCount = 1021;
    static constexpr UINT kTextureTypes = 100; // Color Type : 0-9

    static constexpr float kTranslationSpeed = 0.005f;
    static constexpr float kOffsetBounds = 1.25f;

    static constexpr UINT kInstanceCount = 10;
	static constexpr UINT kMaterialCount = 10;

	float calculateOffsetX(int instanceId) {
		return -kOffsetBounds + (float)instanceId / (float)kInstanceCount * kOffsetBounds * 2.0f;
	}

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

	struct InstanceData
	{
		XMFLOAT4 offset;
		UINT materialId;
		float padding[3]; // 16byte alignment
	};

    struct FrameResource
    {
        ComPtr<ID3D12CommandAllocator> commandAllocator;
        ComPtr<ID3D12Resource> instanceBuffer;
        InstanceData* pSrvDataBegin = nullptr;
        UINT64 fenceValue = 0;
    };

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[kFrameCount];
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_heap; // CBV/SRV/UAV heap
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    UINT m_rtvDescriptorSize;
	UINT m_descriptorSize;
    UINT m_nextFrameIndex = 0;

    UINT m_texIndex[kTextureCount] = {};
    UINT m_nextFreeIndex = 0;

    std::vector<InstanceData> m_instanceData;
	std::vector<Material> m_materialData;

    // App resources.
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    std::vector<ComPtr<ID3D12Resource>> m_texture;

    ComPtr<ID3D12Resource> m_materialBuffer;
    Material* pMaterialDataBegin = nullptr;

    std::chrono::steady_clock::time_point m_prevTime;
    UINT m_texIndexId = 0;

    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;

    FrameResource m_frameResources[kFrameCount];

    void LoadPipeline();
    void LoadAssets();
    std::vector<UINT8> GenerateTextureData();
    void PopulateCommandList();

	void WaitForGpu();
	void MoveToNextFrame();

	UINT AllocateTextureSRV(ID3D12Resource* texture);
};
