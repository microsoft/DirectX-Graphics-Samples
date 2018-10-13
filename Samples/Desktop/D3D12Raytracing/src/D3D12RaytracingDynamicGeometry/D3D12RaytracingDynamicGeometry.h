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

// The sample supports both Raytracing Fallback Layer and DirectX Raytracing APIs. 
// This is purely for demonstration purposes to show where the API differences are. 
// Real-world applications will implement only one or the other. 
// Fallback Layer uses DirectX Raytracing if a driver and OS supports it. 
// Otherwise, it falls back to compute pipeline to emulate raytracing.
// Developers aiming for a wider HW support should target Fallback Layer.
class D3D12RaytracingDynamicGeometry : public DXSample
{
public:
	D3D12RaytracingDynamicGeometry(UINT width, UINT height, std::wstring name);
	~D3D12RaytracingDynamicGeometry();
	// IDeviceNotify
	virtual void OnDeviceLost() override;
	virtual void OnDeviceRestored() override;
	virtual void OnReleaseWindowSizeDependentResources() override { ReleaseWindowSizeDependentResources(); };
	virtual void OnCreateWindowSizeDependentResources() override { CreateWindowSizeDependentResources(); };

	// Messages
	virtual void OnInit();
	virtual void OnKeyDown(UINT8 key);
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnSizeChanged(UINT width, UINT height, bool minimized);
	virtual void OnDestroy();
	virtual IDXGISwapChain* GetSwapchain() { return m_deviceResources->GetSwapChain(); }

	UINT AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse = UINT_MAX);
	void CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize, UINT* descriptorHeapIndex, UINT firstElement = 0);
	WRAPPED_GPU_POINTER CreateFallbackWrappedPointer(ID3D12Resource* resource, UINT bufferNumElements, UINT* descriptorHeapIndex);

	const DX::DeviceResources& GetDeviceResources() { return *m_deviceResources; }
	RaytracingAPI GetRaytracingAPI() { return m_raytracingAPI; }
	ID3D12RaytracingFallbackDevice* GetFallbackDevice() { return m_fallbackDevice.Get(); }
	ID3D12RaytracingFallbackCommandList* GetFallbackCommandList() { return m_fallbackCommandList.Get(); }
    ID3D12Device5* GetDxrDevice() { return m_dxrDevice.Get(); }
    ID3D12GraphicsCommandList5* GetDxrCommandList() { return m_deviceResources->GetCommandList(); }

	void RequestGeometryInitialization(bool bRequest) { m_isGeometryInitializationRequested = bRequest; }
	void RequestASInitialization(bool bRequest) { m_isASinitializationRequested = bRequest; }

	static const UINT MaxBLAS = 1000;

private:
	static const UINT FrameCount = 3;


	std::mt19937 m_generatorURNG;

	// Constants.
	const UINT NUM_BLAS = 2;          // Triangle + AABB bottom-level AS.
	const float c_aabbWidth = 2;      // AABB width.
	const float c_aabbDistance = 2;   // Distance between AABBs.

	// DynamicGeometry
	std::vector<BottomLevelAccelerationStructure> m_vBottomLevelAS;
	std::vector<GeometryInstance> m_geometryInstances;
	TopLevelAccelerationStructure m_topLevelAS;
	ComPtr<ID3D12Resource> m_accelerationStructureScratch;
	UINT64 m_ASmemoryFootprint;
	int m_numFramesSinceASBuild;
	const float m_geometryRadius = 3.0f;

	// Raytracing Fallback Layer (FL) attributes
	ComPtr<ID3D12RaytracingFallbackDevice> m_fallbackDevice;
	ComPtr<ID3D12RaytracingFallbackCommandList> m_fallbackCommandList;
	ComPtr<ID3D12RaytracingFallbackStateObject> m_fallbackStateObject;
	std::vector<UINT> m_bottomLevelASdescritorHeapIndices;
	std::vector<UINT> m_bottomLevelASinstanceDescsDescritorHeapIndices;
	UINT m_topLevelASdescritorHeapIndex;

	// DirectX Raytracing (DXR) attributes
	ComPtr<ID3D12Device5> m_dxrDevice;
	ComPtr<ID3D12StateObjectPrototype> m_dxrStateObject;
	bool m_isDxrSupported;

    // Compute resources.
    Samplers::MultiJittered m_randomSampler;
    ConstantBuffer<RNGConstantBuffer>   m_computeCB;
    ComPtr<ID3D12PipelineState>         m_computePSO;
    ComPtr<ID3D12RootSignature>         m_csSamleVisualizerRootSignature;
    ComPtr<ID3D12CommandAllocator>      m_computeAllocators[FrameCount];
    ComPtr<ID3D12CommandQueue>          m_computeCommandQueue;
	ComPtr<ID3D12GraphicsCommandList>   m_computeCommandList;
	
    ComPtr<ID3D12Fence>                 m_fence;
    UINT64                              m_fenceValues[FrameCount];
    Microsoft::WRL::Wrappers::Event     m_fenceEvent;
	// Root signatures
    ComPtr<ID3D12RootSignature> m_raytracingGlobalRootSignature;
	ComPtr<ID3D12RootSignature> m_raytracingLocalRootSignature[LocalRootSignature::Type::Count];

	// Descriptors
	ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
	UINT m_descriptorsAllocated;
	UINT m_descriptorSize;

	// Raytracing scene
	ConstantBuffer<SceneConstantBuffer> m_sceneCB;
	StructuredBuffer<PrimitiveInstancePerFrameBuffer> m_aabbPrimitiveAttributeBuffer;
	std::vector<D3D12_RAYTRACING_AABB> m_aabbs;

	// Root constants
	PrimitiveConstantBuffer m_planeMaterialCB;
	PrimitiveConstantBuffer m_aabbMaterialCB[IntersectionShaderType::TotalPrimitiveCount];

	// Geometry
	D3DBuffer m_aabbBuffer;
	std::vector<UINT> m_geometryIBHeapIndices;
	std::vector<UINT> m_geometryVBHeapIndices;


	// ToDo clean up buffer management
	// SquidScene buffers
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_vertexBufferUpload;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	ComPtr<ID3D12Resource> m_indexBuffer;
	ComPtr<ID3D12Resource> m_indexBufferUpload;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

	DX::GPUTimer m_gpuTimers[GpuTimers::Count];

	struct alignas(16) AlignedGeometryTransform3x4
	{
		float transform3x4[12];
	};

	std::vector<TriangleGeometryBuffer> m_geometries;
    StructuredBuffer<AlignedGeometryTransform3x4> m_geometryTransforms;

    StructuredBuffer<AlignedUnitSquareSample2D> m_samplesGPUBuffer; 
    StructuredBuffer<AlignedHemisphereSample3D> m_hemisphereSamplesGPUBuffer;

	// Raytracing output
	ComPtr<ID3D12Resource> m_raytracingOutput;
	D3D12_GPU_DESCRIPTOR_HANDLE m_raytracingOutputResourceUAVGpuDescriptor;
	UINT m_raytracingOutputResourceUAVDescriptorHeapIndex;

	// Shader tables
	static const wchar_t* c_hitGroupNames_TriangleGeometry[RayType::Count];
	static const wchar_t* c_raygenShaderName;
	static const wchar_t* c_intersectionShaderNames[IntersectionShaderType::Count];
	static const wchar_t* c_closestHitShaderNames[1];
	static const wchar_t* c_missShaderNames[RayType::Count];

	ComPtr<ID3D12Resource> m_missShaderTable;
	UINT m_missShaderTableStrideInBytes;
	ComPtr<ID3D12Resource> m_hitGroupShaderTable;
	UINT m_hitGroupShaderTableStrideInBytes;
	ComPtr<ID3D12Resource> m_rayGenShaderTable;

	// Application state
	RaytracingAPI m_raytracingAPI;
	bool m_forceComputeFallback;
	StepTimer m_timer;
	bool m_animateCamera;
	bool m_animateLight;
	bool m_animateScene;
	bool m_isCameraFrozen;
	GameCore::Camera m_camera;
	std::unique_ptr<GameCore::CameraController> m_cameraController;

	// AO
	const UINT c_sppAO = 16;	// Samples per pixel for Ambient Occlusion.

	// UI
	std::unique_ptr<UILayer> m_uiLayer;
	bool m_bCtrlKeyIsPressed;
	float m_fps;
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS m_ASBuildQuality;
	UINT m_activeUIparameter;
	UINT m_numTrianglesPerGeometry;
	bool m_isGeometryInitializationRequested;
	bool m_isASinitializationRequested;
	bool m_isASrebuildRequested;

	void ConvertRHtoLHGeometry(UINT8* pAssetData);
	void LoadSceneGeometry();
    void EnableDirectXRaytracing(IDXGIAdapter1* adapter);
    void ParseCommandLineArgs(WCHAR* argv[], int argc);
    void UpdateCameraMatrices();
	void UpdateBottomLevelASTransforms();
    void UpdateSphereGeometryTransforms();
    void InitializeScene();
    void RecreateD3D();
	void UpdateAccelerationStructures(bool forceBuild = false);
    void DoRaytracing();
    void CreateConstantBuffers();
    void CreateSamplesRNG();
    void CreateAABBPrimitiveAttributesBuffers();
	void ModifyActiveUIParameter(bool bIncreaseValue);
	void UpdateUI();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void ReleaseDeviceDependentResources();
    void ReleaseWindowSizeDependentResources();
    void RenderRNGVisualizations();
    void CreateRaytracingInterfaces();
    void SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig, LPCWSTR resournceName = nullptr);
    void CreateRootSignatures();
    void CreateDxilLibrarySubobject(CD3D12_STATE_OBJECT_DESC* raytracingPipeline);
    void CreateHitGroupSubobjects(CD3D12_STATE_OBJECT_DESC* raytracingPipeline);
    void CreateLocalRootSignatureSubobjects(CD3D12_STATE_OBJECT_DESC* raytracingPipeline);
    void CreateRaytracingPipelineStateObject();
    void CreateDescriptorHeap();
    void CreateRaytracingOutputResource();
	void CreateAuxilaryDeviceResources();
    void BuildDynamicGeometryAABBs();
    void InitializeGeometry();
    void BuildPlaneGeometry();
    void BuildTesselatedGeometry();
	void GenerateBottomLevelASInstanceTransforms();
    void InitializeAccelerationStructures();
    void BuildShaderTables();
    void SelectRaytracingAPI(RaytracingAPI type);
    void UpdateForSizeChange(UINT clientWidth, UINT clientHeight);
    void CopyRaytracingOutputToBackbuffer(D3D12_RESOURCE_STATES outRenderTargetState = D3D12_RESOURCE_STATE_PRESENT);
    void CalculateFrameStats();
	float NumMRaysPerSecond();
};
