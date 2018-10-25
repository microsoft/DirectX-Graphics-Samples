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
#include "StepTimer.h"
#include "RaytracingSceneDefines.h"
#include "DirectXRaytracingHelper.h"
#include "CameraController.h"
#include "PerformanceTimers.h"
#include "Sampler.h"
#include "UILayer.h"
#include "GpuKernels.h"
#include "PBRTParser.h"

class D3D12RaytracingAmbientOcclusion : public DXSample
{
public:
	D3D12RaytracingAmbientOcclusion(UINT width, UINT height, std::wstring name);
	~D3D12RaytracingAmbientOcclusion();
	// IDeviceNotify
	virtual void OnReleaseWindowSizeDependentResources() override { ReleaseWindowSizeDependentResources(); };
	virtual void OnCreateWindowSizeDependentResources() override { CreateWindowSizeDependentResources(); };

	// Messages
	virtual void OnInit();
	virtual void OnKeyDown(UINT8 key);
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnSizeChanged(UINT width, UINT height, bool minimized);
	virtual IDXGISwapChain* GetSwapchain() { return m_deviceResources->GetSwapChain(); }

	const DX::DeviceResources& GetDeviceResources() { return *m_deviceResources; }
	ID3D12Device5* GetDxrDevice() { return m_dxrDevice.Get(); }
	ID3D12GraphicsCommandList4* GetDxrCommandList() { return m_deviceResources->GetCommandList(); }

	void RequestGeometryInitialization(bool bRequest) { m_isGeometryInitializationRequested = bRequest; }
	void RequestASInitialization(bool bRequest) { m_isASinitializationRequested = bRequest; }
	void RequestSceneInitialization() { m_isSceneInitializationRequested = true; }

	static const UINT MaxBLAS = 1000;

private:
	static const UINT FrameCount = 3;

	// ToDo change ID3D12Resourcs with views to RWGpuResource

	std::mt19937 m_generatorURNG;

	// Constants.
	const UINT NUM_BLAS = 2;          // Triangle + AABB bottom-level AS.
	const float c_aabbWidth = 2;      // AABB width.
	const float c_aabbDistance = 2;   // Distance between AABBs.

	// AmbientOcclusion
	std::vector<BottomLevelAccelerationStructure> m_vBottomLevelAS;
	std::vector<GeometryInstance>	m_geometryInstances[Scene::Type::Count];
	TopLevelAccelerationStructure	m_topLevelAS;
	ComPtr<ID3D12Resource>			m_accelerationStructureScratch;
	UINT64 m_ASmemoryFootprint;
	int m_numFramesSinceASBuild;
#if TESSELATED_GEOMETRY_BOX
#if TESSELATED_GEOMETRY_THIN
	const XMFLOAT3 m_boxSize = XMFLOAT3(0.01f, 0.1f, 0.01f);
#else
	const XMFLOAT3 m_boxSize = XMFLOAT3(1, 1, 1);
#endif
	const float m_geometryRadius = 2.0f;
#else
	const float m_geometryRadius = 3.0f;
#endif

	std::vector<UINT> m_bottomLevelASdescritorHeapIndices;
	std::vector<UINT> m_bottomLevelASinstanceDescsDescritorHeapIndices;
	UINT m_topLevelASdescritorHeapIndex;

	// DirectX Raytracing (DXR) attributes
	ComPtr<ID3D12Device5> m_dxrDevice;		// ToDo remove
	ComPtr<ID3D12StateObject> m_dxrStateObject;

	// Compute resources.
	Samplers::MultiJittered m_randomSampler;
	ConstantBuffer<RNGConstantBuffer>   m_csHemisphereVisualizationCB;
	ComPtr<ID3D12PipelineState>         m_computePSOs[ComputeShader::Type::Count];
	ComPtr<ID3D12RootSignature>         m_computeRootSigs[ComputeShader::Type::Count];

	GpuKernels::ReduceSum				m_reduceSumKernel;
	UINT								m_numRayGeometryHits[ReduceSumCalculations::Count];

	ComPtr<ID3D12Fence>                 m_fence;
	UINT64                              m_fenceValues[FrameCount];
	Microsoft::WRL::Wrappers::Event     m_fenceEvent;
	// Root signatures
	ComPtr<ID3D12RootSignature> m_raytracingGlobalRootSignature;
	ComPtr<ID3D12RootSignature> m_raytracingLocalRootSignature[LocalRootSignature::Type::Count];

	// ToDo move to deviceResources
	std::unique_ptr<DescriptorHeap> m_cbvSrvUavHeap;
	std::unique_ptr<DescriptorHeap> m_samplerHeap;

	// Raytracing scene
	ConstantBuffer<SceneConstantBuffer> m_sceneCB;

	// Root constants
	PrimitiveConstantBuffer m_planeMaterialCB;

	// Geometry

	DX::GPUTimer m_gpuTimers[GpuTimers::Count];

	// ToDo clean up buffer management
	// SquidRoom buffers
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_vertexBufferUpload;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	ComPtr<ID3D12Resource> m_indexBuffer;
	ComPtr<ID3D12Resource> m_indexBufferUpload;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;


	struct alignas(16) AlignedGeometryTransform3x4
	{
		float transform3x4[12];
	};

	SceneParser::Scene m_pbrtScene;
	std::vector<D3DGeometry> m_geometries[GeometryType::Count];
	StructuredBuffer<AlignedGeometryTransform3x4> m_geometryTransforms;

	StructuredBuffer<AlignedUnitSquareSample2D> m_samplesGPUBuffer;
	StructuredBuffer<AlignedHemisphereSample3D> m_hemisphereSamplesGPUBuffer;


	// Raytracing output
	// ToDo use the struct
	RWGpuResource m_raytracingOutput;
	RWGpuResource m_GBufferResources[GBufferResource::Count];
	RWGpuResource m_AORayHits;


	// Shader tables
	static const wchar_t* c_hitGroupNames_TriangleGeometry[RayType::Count];
	static const wchar_t* c_rayGenShaderNames[RayGenShaderType::Count];
	static const wchar_t* c_closestHitShaderNames[RayType::Count];
	static const wchar_t* c_missShaderNames[RayType::Count];

	ComPtr<ID3D12Resource> m_rayGenShaderTables[RayGenShaderType::Count];
	UINT m_rayGenShaderTableRecordSizeInBytes;
	ComPtr<ID3D12Resource> m_hitGroupShaderTable;
	UINT m_hitGroupShaderTableStrideInBytes;
	ComPtr<ID3D12Resource> m_missShaderTable;
	UINT m_missShaderTableStrideInBytes;

	// Application state
	StepTimer m_timer;
	bool m_animateCamera;
	bool m_animateLight;
	bool m_animateScene;
	bool m_isCameraFrozen;
	GameCore::Camera m_camera;
	std::unique_ptr<GameCore::CameraController> m_cameraController;
	
	// AO
	// ToDo fix artifacts at 4. Looks like selfshadowing on some AOrays in SquidScene
	const UINT c_sppAO = 16;	// Samples per pixel for Ambient Occlusion.

	// UI
	std::unique_ptr<UILayer> m_uiLayer;
	
	float m_fps;
	UINT m_numTrianglesInTheScene;
	UINT m_numTriangles[GeometryType::Count];
	bool m_isGeometryInitializationRequested;
	bool m_isASinitializationRequested;
	bool m_isASrebuildRequested;
	bool m_isSceneInitializationRequested;

	void ParseCommandLineArgs(WCHAR* argv[], int argc);
	void RecreateD3D();
	void LoadPBRTScene();
	void LoadSceneGeometry();
    void UpdateCameraMatrices();
	void UpdateBottomLevelASTransforms();
	void UpdateSphereGeometryTransforms();
    void UpdateGridGeometryTransforms();
    void InitializeScene();
	void UpdateAccelerationStructures(bool forceBuild = false);
	void DispatchRays(ID3D12Resource* rayGenShaderTable, DX::GPUTimer* gpuTimer);
    void DoRaytracing();
	void DoRaytracingGBufferAndAOPasses();
    void CreateConstantBuffers();
    void CreateSamplesRNG();
	void UpdateUI();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void ReleaseDeviceDependentResources();
    void ReleaseWindowSizeDependentResources();
    void RenderRNGVisualizations();
    void CreateRaytracingInterfaces();
    void CreateRootSignatures();
    void CreateDxilLibrarySubobject(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);
    void CreateHitGroupSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);
    void CreateLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);
    void CreateRaytracingPipelineStateObject();
    void CreateDescriptorHeaps();
    void CreateRaytracingOutputResource();
	void CreateGBufferResources();
	void CreateAuxilaryDeviceResources();
    void InitializeGeometry();
    void BuildPlaneGeometry();
    void BuildTesselatedGeometry();
	void GenerateBottomLevelASInstanceTransforms();
    void InitializeAccelerationStructures();
    void BuildShaderTables();
    void CopyRaytracingOutputToBackbuffer(D3D12_RESOURCE_STATES outRenderTargetState = D3D12_RESOURCE_STATE_PRESENT);
    void CalculateFrameStats();
#if GBUFFER_AO_SEPRATE_PATHS
	float NumCameraRaysPerSecond() { return NumMPixelsPerSecond(m_gpuTimers[GpuTimers::Raytracing_GBuffer].GetAverageMS(), m_width, m_height); }
	float NumRayGeometryHitsPerSecond(ReduceSumCalculations::Enum type) { return NumMPixelsPerSecond(m_gpuTimers[GpuTimers::Raytracing_AO].GetAverageMS(type), m_numRayGeometryHits[type], 1); }
#else
	float NumCameraRaysPerSecond() { return NumMPixelsPerSecond(m_gpuTimers[GpuTimers::Raytracing_PrimaryAndAO].GetAverageMS(), m_width, m_height); }
	float NumRayGeometryHitsPerSecond(ReduceSumCalculations::Enum type) { return NumMPixelsPerSecond(m_gpuTimers[GpuTimers::Raytracing_PrimaryAndAO].GetAverageMS(type), m_width, m_height); }
#endif
};
