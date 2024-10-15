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

namespace GpuKernels
{
	class ReduceSum
	{
	public:
        enum Type {
            Uint = 0,
            Float
        };

		void Initialize(ID3D12Device5* device, Type type);
		void CreateInputResourceSizeDependentResources(
			ID3D12Device5* device,
			DX::DescriptorHeap* descriptorHeap,
			UINT frameCount,
			UINT width,
			UINT height,
			UINT numInvocationsPerFrame = 1);
		void Run(
			ID3D12GraphicsCommandList4* commandList,
			ID3D12DescriptorHeap* descriptorHeap,
			UINT frameIndex,
            D3D12_GPU_DESCRIPTOR_HANDLE inputResourceHandle,
			void* resultSum,
            UINT invocationIndex = 0 );

	private:
        Type                                m_resultType;
        UINT                                m_resultSize;
		ComPtr<ID3D12RootSignature>         m_rootSignature;
		ComPtr<ID3D12PipelineState>         m_pipelineStateObject;
		std::vector<GpuResource>			m_csReduceSumOutputs;
		std::vector<ComPtr<ID3D12Resource>>	m_readbackResources;
	};

    class DownsampleGBufferDataBilateralFilter
    {
    public:
        void Initialize(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame = 1);
        void Run(
            ID3D12GraphicsCommandList4* commandList,
            UINT width,
            UINT height,
            ID3D12DescriptorHeap* descriptorHeap,
            D3D12_GPU_DESCRIPTOR_HANDLE inputNormalResourceHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE inputPositionResourceHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE inputPartialDistanceDerivativesResourceHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE inputMotionVectorResourceHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE inputPrevFrameHitPositionResourceHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE inputDepthResourceHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE inputSurfaceAlbedoResourceHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE outputNormalResourceHandle,            
            D3D12_GPU_DESCRIPTOR_HANDLE outputPositionResourceHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE outputPartialDistanceDerivativesResourceHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE outputMotionVectorResourceHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE outputPrevFrameHitPositionResourceHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE outputDepthResourceHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE outputSurfaceAlbedoResourceHandle);

    private:
        ComPtr<ID3D12RootSignature>         m_rootSignature;
        ComPtr<ID3D12PipelineState>         m_pipelineStateObject;
        ConstantBuffer<TextureDimConstantBuffer> m_CB;
        UINT                                m_CBinstanceID = 0;
    };

    class UpsampleBilateralFilter
    {
    public:
        enum FilterType {
            Filter2x2FloatR = 0,
            Filter2x2UintR,
            Filter2x2FloatRG,
            Count
        };

        void Initialize(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame = 1);
        void Run(
            ID3D12GraphicsCommandList4* commandList,
            UINT width,
            UINT height,
            FilterType type,
            ID3D12DescriptorHeap* descriptorHeap,
            D3D12_GPU_DESCRIPTOR_HANDLE inputResourceHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE inputLowResNormalResourceHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE inputHiResNormalResourceHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE inputHiResPartialDistanceDerivativeResourceHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE outputResourceHandle);

    private:
        ComPtr<ID3D12RootSignature>         m_rootSignature;
        ComPtr<ID3D12PipelineState>         m_pipelineStateObjects[FilterType::Count];
        ConstantBuffer<DownAndUpsampleFilterConstantBuffer> m_CB;
        UINT                                m_CBinstanceID = 0;
    };

    class CalculatePartialDerivatives
    {
    public:
        void Initialize(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame = 1);
        void Run(
            ID3D12GraphicsCommandList4* commandList,
            ID3D12DescriptorHeap* descriptorHeap,
            UINT width,
            UINT height,
            D3D12_GPU_DESCRIPTOR_HANDLE inputResourceHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE outputResourceHandle);

    private:
        ComPtr<ID3D12RootSignature>         m_rootSignature;
        ComPtr<ID3D12PipelineState>         m_pipelineStateObject;
        ConstantBuffer<AtrousWaveletTransformFilterConstantBuffer> m_CB;
        UINT                                m_CBinstanceID = 0;
    };
   
    class GenerateGrassPatch
    {
    public:
        void Initialize(ID3D12Device5* device, const wchar_t* windTexturePath, DX::DescriptorHeap* descriptorHeap, ResourceUploadBatch* resourceUpload, UINT frameCount, UINT numCallsPerFrame = 1);
        void Run(
            ID3D12GraphicsCommandList4* commandList,
            const GenerateGrassStrawsConstantBuffer_AppParams& appParams,
            ID3D12DescriptorHeap* descriptorHeap,
            D3D12_GPU_DESCRIPTOR_HANDLE outputVertexBufferResourceHandle);

        UINT GetVertexBufferSize(UINT grassStrawsX, UINT grassStrawsY)
        {
            return grassStrawsX * grassStrawsY * N_GRASS_VERTICES * sizeof(VertexPositionNormalTextureTangent);
        }

    private:
        ComPtr<ID3D12RootSignature>         m_rootSignature;
        ComPtr<ID3D12PipelineState>         m_pipelineStateObject;

        ConstantBuffer<GenerateGrassStrawsConstantBuffer> m_CB;
        UINT                                m_CBinstanceID = 0;
        D3DTexture                          m_windTexture;
    };
}

