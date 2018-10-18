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

#include "stdafx.h"
#include "D3D12RaytracingDynamicGeometry.h"
#include "GameInput.h"
#include "EngineTuning.h"
#include "CompiledShaders\Raytracing.hlsl.h"
#include "CompiledShaders\RNGVisualizer.hlsl.h"
#include "SquidRoom.h"

using namespace std;
using namespace DX;
using namespace DirectX;
using namespace SceneEnums;
using namespace GameCore;

D3D12RaytracingDynamicGeometry* g_pSample = nullptr;
HWND g_hWnd = 0;

// Shader entry points.
const wchar_t* D3D12RaytracingDynamicGeometry::c_rayGenShaderNames[] = 
{
	// ToDo reorder
	L"MyRayGenShader_GBuffer", L"MyRayGenShader_PrimaryAndAO", L"MyRayGenShader_AO"
};
const wchar_t* D3D12RaytracingDynamicGeometry::c_closestHitShaderNames[] =
{
    L"MyClosestHitShader", L"MyClosestHitShader_GBuffer"
};
const wchar_t* D3D12RaytracingDynamicGeometry::c_missShaderNames[] =
{
    L"MyMissShader", L"MyMissShader_ShadowRay", L"MyMissShader_GBuffer"
};
// Hit groups.
const wchar_t* D3D12RaytracingDynamicGeometry::c_hitGroupNames_TriangleGeometry[] = 
{ 
    L"MyHitGroup_Triangle", L"MyHitGroup_Triangle_ShadowRay", L"MyHitGroup_Triangle_GBuffer"
};
namespace SceneArgs
{
    void OnGeometryReinitializationNeeded(void* args)
    {
        g_pSample->RequestGeometryInitialization(true);
        g_pSample->RequestASInitialization(true);
    }

    void OnASReinitializationNeeded(void* args)
    {
        g_pSample->RequestASInitialization(true);
    }
    function<void(void*)> OnGeometryChange = OnGeometryReinitializationNeeded;
    function<void(void*)> OnASChange = OnASReinitializationNeeded;
	
	void OnSceneChange(void*)
	{
		g_pSample->RequestSceneInitialization();
	}

    enum RaytracingMode { FLDXR = 0, FL, DXR };
    const WCHAR* RaytracingModes[] = { L"FL-DXR", L"FL",L"DXR" };
    // ToDo EnumVar RaytracingMode(L"RaytracingMode", FLDXR, _countof(RaytracingModes), RaytracingModes);

    BoolVar EnableGeometryAndASBuildsAndUpdates(L"Enable geometry & AS builds and updates", true);

#if ONLY_SQUID_SCENE_BLAS
	EnumVar SceneType(L"Scene", Scene::Type::SquidRoom, Scene::Type::Count, Scene::Type::Names, OnSceneChange, nullptr);
#else
	EnumVar SceneType(L"Scene", Scene::Type::SingleObject, Scene::Type::Count, Scene::Type::Names, OnSceneChange, nullptr);
#endif

    enum UpdateMode { Build = 0, Update, Update_BuildEveryXFrames };
    const WCHAR* UpdateModes[] = { L"Build only", L"Update only", L"Update + build every X frames" };
    EnumVar ASUpdateMode(L"Acceleration structure/Update mode", Build, _countof(UpdateModes), UpdateModes);
    IntVar ASBuildFrequency(L"Acceleration structure/Rebuild frame frequency", 1, 1, 1200, 1);
    BoolVar ASMinimizeMemory(L"Acceleration structure/Minimize memory", false, OnASChange, nullptr);
    BoolVar ASAllowUpdate(L"Acceleration structure/Allow update", true, OnASChange, nullptr);
 
    // ToDo test tessFactor 16
	// ToDo fix alias on TessFactor 2
    IntVar GeometryTesselationFactor(L"Geometry/Tesselation factor", 14, 0, 80, 1, OnGeometryChange, nullptr);
    IntVar NumGeometriesPerBLAS(L"Geometry/# geometries per BLAS", 1, 1, 1000, 1, OnGeometryChange, nullptr);
    IntVar NumSphereBLAS(L"Geometry/# Sphere BLAS", 1, 1, D3D12RaytracingDynamicGeometry::MaxBLAS, 1, OnASChange, nullptr);
};


D3D12RaytracingDynamicGeometry::D3D12RaytracingDynamicGeometry(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_animateCamera(false),
    m_animateLight(false),
    m_animateScene(true),
    m_missShaderTableStrideInBytes(UINT_MAX),
    m_hitGroupShaderTableStrideInBytes(UINT_MAX),
    m_numTriangles(0),
    m_isGeometryInitializationRequested(true),
    m_isASinitializationRequested(true),
    m_isASrebuildRequested(true),
    m_ASmemoryFootprint(0),
    m_numFramesSinceASBuild(0),
	m_isCameraFrozen(false),
	m_timerID(0)
{
    g_pSample = this;
    UpdateForSizeChange(width, height);
    m_bottomLevelASdescritorHeapIndices.resize(MaxBLAS, UINT_MAX);
    m_bottomLevelASinstanceDescsDescritorHeapIndices.resize(MaxBLAS, UINT_MAX);
    m_topLevelASdescritorHeapIndex = UINT_MAX;
    m_geometryIBHeapIndices.resize(GeometryType::Count, UINT_MAX);
    m_geometryVBHeapIndices.resize(GeometryType::Count, UINT_MAX);
	m_raytracingOutput.descriptorHeapIndex = UINT_MAX;
	for (auto& gbufferResource : m_GBufferResources)
	{
		gbufferResource.descriptorHeapIndex = UINT_MAX;
	}
	m_generatorURNG.seed(1729);
}

void D3D12RaytracingDynamicGeometry::OnInit()
{
    m_deviceResources = std::make_unique<DeviceResources>(
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_UNKNOWN,
        FrameCount,
        D3D_FEATURE_LEVEL_11_0,
        // Sample shows handling of use cases with tearing support, which is OS dependent and has been supported since TH2.
        // Since the DXR requires October 2018 update, we don't need to handle non-tearing cases.
        DeviceResources::c_RequireTearingSupport,
        m_adapterIDoverride
        );
    m_deviceResources->RegisterDeviceNotify(this);
    m_deviceResources->SetWindow(Win32Application::GetHwnd(), m_width, m_height);

    g_hWnd = Win32Application::GetHwnd();
    GameInput::Initialize();
    EngineTuning::Initialize();

    m_deviceResources->InitializeDXGIAdapter();
	ThrowIfFalse(IsDirectXRaytracingSupported(m_deviceResources->GetAdapter()),
		L"ERROR: DirectX Raytracing is not supported by your GPU and driver.\n\n");
    
    // ToDo cleanup
    m_deviceResources->CreateDeviceResources();
	// Initialize scene ToDo
    InitializeScene();
    CreateDeviceDependentResources();
    m_deviceResources->CreateWindowSizeDependentResources();
}

D3D12RaytracingDynamicGeometry::~D3D12RaytracingDynamicGeometry()
{
    GameInput::Shutdown();
}

// Update camera matrices passed into the shader.
void D3D12RaytracingDynamicGeometry::UpdateCameraMatrices()
{
    m_sceneCB->cameraPosition = m_camera.Eye();

	float verticalFovAngle = 45.0f;
	XMMATRIX view, proj;
	// ToDo camera is creating fisheye in spehere scene
	m_camera.GetProj(&proj, verticalFovAngle, m_width, m_height);

	// Calculate view matrix as if the camera was at (0,0,0) to avoid 
	// precision issues when camera position is too far from (0,0,0).
	// GenerateCameraRay takes this into consideration in the raytracing shader.
	view = XMMatrixLookAtLH(XMVectorSet(0, 0, 0, 1), XMVectorSetW(m_camera.At() - m_camera.Eye(), 1), m_camera.Up());
	XMMATRIX viewProj = view * proj;
	m_sceneCB->projectionToWorldWithCameraEyeAtOrigin = XMMatrixInverse(nullptr, viewProj);
}

void D3D12RaytracingDynamicGeometry::UpdateBottomLevelASTransforms()
{
    float animationDuration = 24.0f;
    float curTime = static_cast<float>(m_timer.GetTotalSeconds());
    float t = CalculateAnimationInterpolant(curTime, animationDuration);
    t += -0.5f;
    //ToDo
    t = 0.0f;

    float baseAmplitude = 16.0f;
    for (auto& bottomLevelAS : m_vBottomLevelAS)
    {
        // Animate along Y coordinate.
        XMMATRIX transform = bottomLevelAS.GetTransform();
        float distFromOrigin = XMVectorGetX(XMVector4Length(transform.r[3]));
        float posY = t * (baseAmplitude + 0.35f * distFromOrigin);

        transform.r[3] = XMVectorSetByIndex(transform.r[3], posY, 1);
        bottomLevelAS.SetTransform(transform);
    }
}

void D3D12RaytracingDynamicGeometry::UpdateSphereGeometryTransforms()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    // Generate geometry desc transforms;
    int dim = static_cast<int>(ceil(cbrt(static_cast<double>(SceneArgs::NumGeometriesPerBLAS))));
    float distanceBetweenGeometry = m_geometryRadius;
    float geometryWidth = 2 * m_geometryRadius;
    float stepDistance = geometryWidth + distanceBetweenGeometry;

    float animationDuration = 12.0f;
    float curTime = static_cast<float>(m_timer.GetTotalSeconds());
    float t = CalculateAnimationInterpolant(curTime, animationDuration);
    //ToDo
    t = 0.0f;
    float rotAngle = XMConvertToRadians(t * 360.0f);

    // Rotate around offset center.
    XMMATRIX localTranslation = XMMatrixTranslation(0.0f, m_geometryRadius, 0.5f * m_geometryRadius);
    XMMATRIX localRotation = XMMatrixRotationY(XMConvertToRadians(rotAngle));
    XMMATRIX localTransform = localTranslation * localRotation;
    
    // ToDo
    localTransform = XMMatrixTranslation(0.0f, m_geometryRadius, 0.0f);

    for (int iY = 0, i = 0; iY < dim; iY++)
        for (int iX = 0; iX < dim; iX++)
            for (int iZ = 0; iZ < dim; iZ++, i++)
            {
                if (i >= SceneArgs::NumGeometriesPerBLAS)
                {
                    break;
                }

                // Translate within BLAS.
                XMFLOAT4 translationVector = XMFLOAT4(
                    static_cast<float>(iX - dim / 2),
                    static_cast<float>(iY - dim / 2),
                    static_cast<float>(iZ - dim / 2),
                    0.0f);
                XMMATRIX transformWithinBLAS= XMMatrixTranslationFromVector(stepDistance * XMLoadFloat4(&translationVector));
                XMMATRIX transform = localTransform * transformWithinBLAS;
                assert(BottomLevelASType::Sphere == 1);
                for (UINT j = BottomLevelASType::Sphere; j < m_vBottomLevelAS.size(); j++)
                {
                    UINT transformIndex = (j- BottomLevelASType::Sphere) * SceneArgs::NumGeometriesPerBLAS + i;
        			XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(m_geometryTransforms[transformIndex].transform3x4), transform);
                }
            }
}

// Initialize scene rendering parameters.
void D3D12RaytracingDynamicGeometry::InitializeScene()
{
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    // Setup materials.
    {
        auto SetAttributes = [&](
            UINT primitiveIndex, 
            const XMFLOAT4& albedo, 
            float reflectanceCoef = 0.0f,
            float diffuseCoef = 0.9f,
            float specularCoef = 0.7f,
            float specularPower = 50.0f,
            float stepScale = 1.0f )
        {
			// ToDo
            //auto& attributes = m_aabbMaterialCB[primitiveIndex];
            //attributes.albedo = albedo;
            //attributes.reflectanceCoef = reflectanceCoef;
            //attributes.diffuseCoef = diffuseCoef;
            //attributes.specularCoef = specularCoef;
            //attributes.specularPower = specularPower;
            //attributes.stepScale = stepScale;
        };

        m_planeMaterialCB = { XMFLOAT4(0.75f, 0.75f, 0.75f, 1.0f), 0.0f, 1, 0.4f, 50, 1};

        // Albedos
        XMFLOAT4 green = XMFLOAT4(0.1f, 1.0f, 0.5f, 1.0f);
        XMFLOAT4 red = XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f);
        XMFLOAT4 yellow = XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f);
    }

    // Setup camera.
	{
		// Initialize the view and projection inverse matrices.
		auto& camera = Scene::args[SceneArgs::SceneType].camera;
		m_camera.Set(camera.position.eye, camera.position.at, camera.position.up);
		m_cameraController = make_unique<CameraController>(m_camera);
		m_cameraController->SetBoundaries(camera.boundaries.min, camera.boundaries.max);
	}

    // Setup lights.
    {
        // Initialize the lighting parameters.
		// ToDo remove
        XMFLOAT4 lightPosition;
        XMFLOAT4 lightAmbientColor;
        XMFLOAT4 lightDiffuseColor;
		lightPosition = XMFLOAT4(0.0f, 50.0f, -60.0f, 0.0f);
		m_sceneCB->lightPosition = XMLoadFloat4(&lightPosition);

        lightAmbientColor = XMFLOAT4(0.45f, 0.45f, 0.45f, 1.0f);
        m_sceneCB->lightAmbientColor = XMLoadFloat4(&lightAmbientColor);

        float d = 0.6f;
        lightDiffuseColor = XMFLOAT4(d, d, d, 1.0f);
        m_sceneCB->lightDiffuseColor = XMLoadFloat4(&lightDiffuseColor);
    }
}

// Create constant buffers.
void D3D12RaytracingDynamicGeometry::CreateConstantBuffers()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto frameCount = m_deviceResources->GetBackBufferCount();

    m_sceneCB.Create(device, frameCount, L"Scene Constant Buffer");
}

void D3D12RaytracingDynamicGeometry::CreateSamplesRNG()
{
    auto device = m_deviceResources->GetD3DDevice(); 
    auto frameCount = m_deviceResources->GetBackBufferCount();

    m_randomSampler.Reset(c_sppAO, 83, Samplers::HemisphereDistribution::Cosine);

    // Create root signature
    {
        CD3DX12_DESCRIPTOR_RANGE ranges[1]; // Perfomance TIP: Order from most frequent to least frequent.
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 1 output texture

        CD3DX12_ROOT_PARAMETER rootParameters[RNGVisualizerRootSignature::Slot::Count];
        rootParameters[RNGVisualizerRootSignature::Slot::OutputView].InitAsDescriptorTable(1, &ranges[0]);
        rootParameters[RNGVisualizerRootSignature::Slot::SampleBuffers].InitAsShaderResourceView(1);
        rootParameters[RNGVisualizerRootSignature::Slot::SceneConstant].InitAsConstantBufferView(0);

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
        SerializeAndCreateRaytracingRootSignature(rootSignatureDesc, &m_csSamleVisualizerRootSignature, L"RNG Visualizer Compute Root Signature");
    }

    // Create compute pipeline state.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC descComputePSO = {};
        descComputePSO.pRootSignature = m_csSamleVisualizerRootSignature.Get();
        descComputePSO.CS = CD3DX12_SHADER_BYTECODE((void *)g_pRNGVisualizer, ARRAYSIZE(g_pRNGVisualizer));

        ThrowIfFailed(device->CreateComputePipelineState(&descComputePSO, IID_PPV_ARGS(&m_computePSO)));
        m_computePSO->SetName(L"Compute PSO");
    }

    // Create compute allocator, command queue and command list
    {
        D3D12_COMMAND_QUEUE_DESC descCommandQueue = { D3D12_COMMAND_LIST_TYPE_COMPUTE, 0, D3D12_COMMAND_QUEUE_FLAG_NONE };
        ThrowIfFailed(device->CreateCommandQueue(&descCommandQueue, IID_PPV_ARGS(&m_computeCommandQueue)));
        for (auto& computeAllocator: m_computeAllocators)
        {
            ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&computeAllocator)));
        }
        ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_computeAllocators[0].Get(), m_computePSO.Get(), IID_PPV_ARGS(&m_computeCommandList)));
        ThrowIfFailed(m_computeCommandList->Close());

        // Create a fence for tracking GPU execution progress.
        ThrowIfFailed(device->CreateFence(m_fenceValues[0], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValues[0]++;

		for (auto& fenceValue : m_fenceValues)
		{
			fenceValue = m_fenceValues[0];
		}

        m_fenceEvent.Attach(CreateEvent(nullptr, FALSE, FALSE, nullptr));
        if (!m_fenceEvent.IsValid())
        {
            ThrowIfFailed(E_FAIL, L"CreateEvent failed.\n");
        }
    }

    // Create shader resources
    {
        m_computeCB.Create(device, frameCount, L"GPU CB: RNG");
        m_samplesGPUBuffer.Create(device, m_randomSampler.NumSamples() * m_randomSampler.NumSampleSets(), frameCount, L"GPU buffer: Random unit square samples");
        m_hemisphereSamplesGPUBuffer.Create(device, m_randomSampler.NumSamples() * m_randomSampler.NumSampleSets(), frameCount, L"GPU buffer: Random hemisphere samples");

        for (UINT i = 0; i < m_randomSampler.NumSamples() * m_randomSampler.NumSampleSets(); i++)
        {
            //sample.value = m_randomSampler.GetSample2D();
            XMFLOAT3 p = m_randomSampler.GetHemisphereSample3D();
			// Convert [-1,1] to [0,1].
            m_samplesGPUBuffer[i].value = XMFLOAT2(p.x*0.5f + 0.5f, p.y*0.5f + 0.5f);
            m_hemisphereSamplesGPUBuffer[i].value = p;
        }
    }
}

// Create resources that depend on the device.
void D3D12RaytracingDynamicGeometry::CreateDeviceDependentResources()
{
	auto device = m_deviceResources->GetD3DDevice();

    CreateAuxilaryDeviceResources();

    // Initialize raytracing pipeline.

    // Create raytracing interfaces: raytracing device and commandlist.
    CreateRaytracingInterfaces();

    // Create a heap for descriptors.
	CreateDescriptorHeap();

    // Build geometry to be used in the sample.
    // ToDO
    m_isGeometryInitializationRequested = true;
    InitializeGeometry();
    m_isGeometryInitializationRequested = false;

    // Build raytracing acceleration structures from the generated geometry.
    m_isASinitializationRequested = true;

    // Create root signatures for the shaders.
    CreateRootSignatures();

    // Create a raytracing pipeline state object which defines the binding of shaders, state and resources to be used during raytracing.
    CreateRaytracingPipelineStateObject();

    // Create constant buffers for the geometry and the scene.
    CreateConstantBuffers();

    // Build shader tables, which define shaders and their local root arguments.
    BuildShaderTables();

    // Create an output 2D texture to store the raytracing result to.
	// ToDo remove
    CreateRaytracingOutputResource();

    CreateSamplesRNG();
}

void D3D12RaytracingDynamicGeometry::SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig, LPCWSTR resourceName)
{
    auto device = m_deviceResources->GetD3DDevice();
    ComPtr<ID3DBlob> blob;
	ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error), error ? static_cast<wchar_t*>(error->GetBufferPointer()) : nullptr);
    ThrowIfFailed(device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&(*rootSig))));
    
	// ToDo is this check needed?
    if (resourceName)
    {
        (*rootSig)->SetName(resourceName);
    }
}

void D3D12RaytracingDynamicGeometry::CreateRootSignatures()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Global Root Signature
    // This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
    {
        CD3DX12_DESCRIPTOR_RANGE ranges[2]; // Perfomance TIP: Order from most frequent to least frequent.
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 1 output textures
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 3, 1);  // 3 output textures

        CD3DX12_ROOT_PARAMETER rootParameters[GlobalRootSignature::Slot::Count];
        rootParameters[GlobalRootSignature::Slot::OutputView].InitAsDescriptorTable(1, &ranges[0]);
		rootParameters[GlobalRootSignature::Slot::GBufferResources].InitAsDescriptorTable(1, &ranges[1]);
        rootParameters[GlobalRootSignature::Slot::AccelerationStructure].InitAsShaderResourceView(0);
        rootParameters[GlobalRootSignature::Slot::SceneConstant].InitAsConstantBufferView(0);
        rootParameters[GlobalRootSignature::Slot::AABBattributeBuffer].InitAsShaderResourceView(3);
        rootParameters[GlobalRootSignature::Slot::SampleBuffers].InitAsShaderResourceView(4);
        
        CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
        SerializeAndCreateRaytracingRootSignature(globalRootSignatureDesc, &m_raytracingGlobalRootSignature);
    }

    // Local Root Signature
    // This is a root signature that enables a shader to have unique arguments that come from shader tables.
    {
        // Triangle geometry
        {
            CD3DX12_DESCRIPTOR_RANGE ranges[1]; // Perfomance TIP: Order from most frequent to least frequent.
            ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1);  // 2 static index and vertex buffers.

            namespace RootSignatureSlots = LocalRootSignature::Triangle::Slot;
            CD3DX12_ROOT_PARAMETER rootParameters[RootSignatureSlots::Count];
            rootParameters[RootSignatureSlots::MaterialConstant].InitAsConstants(SizeOfInUint32(PrimitiveConstantBuffer), 1);
            rootParameters[RootSignatureSlots::VertexBuffers].InitAsDescriptorTable(1, &ranges[0]);

            CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
            localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
            SerializeAndCreateRaytracingRootSignature(localRootSignatureDesc, &m_raytracingLocalRootSignature[LocalRootSignature::Type::Triangle]);
        }
    }
}

// Create raytracing device and command list.
void D3D12RaytracingDynamicGeometry::CreateRaytracingInterfaces()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();

   ThrowIfFailed(device->QueryInterface(IID_PPV_ARGS(&m_dxrDevice)), L"Couldn't get DirectX Raytracing interface for the device.\n");
}

// DXIL library
// This contains the shaders and their entrypoints for the state object.
// Since shaders are not considered a subobject, they need to be passed in via DXIL library subobjects.
void D3D12RaytracingDynamicGeometry::CreateDxilLibrarySubobject(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
{
    auto lib = raytracingPipeline->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void *)g_pRaytracing, ARRAYSIZE(g_pRaytracing));
    lib->SetDXILLibrary(&libdxil);
    // Use default shader exports for a DXIL library/collection subobject ~ surface all shaders.
}

// Hit groups
// A hit group specifies closest hit, any hit and intersection shaders 
// to be executed when a ray intersects the geometry.
void D3D12RaytracingDynamicGeometry::CreateHitGroupSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
{
    // Triangle geometry hit groups
    {
        for (UINT rayType = 0; rayType < RayType::Count; rayType++)
        {
            auto hitGroup = raytracingPipeline->CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
            
			switch (rayType)
			{
			case RayType::Radiance: hitGroup->SetClosestHitShaderImport(c_closestHitShaderNames[0]); break;
			case RayType::GBuffer: hitGroup->SetClosestHitShaderImport(c_closestHitShaderNames[1]); break;
            }
            hitGroup->SetHitGroupExport(c_hitGroupNames_TriangleGeometry[rayType]);
            hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
        }
    }
}

// Local root signature and shader association
// This is a root signature that enables a shader to have unique arguments that come from shader tables.
void D3D12RaytracingDynamicGeometry::CreateLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
{
    // Ray gen and miss shaders in this sample are not using a local root signature and thus one is not associated with them.

    // Hit groups
    // Triangle geometry
    {
        auto localRootSignature = raytracingPipeline->CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
        localRootSignature->SetRootSignature(m_raytracingLocalRootSignature[LocalRootSignature::Type::Triangle].Get());
        // Shader association
        auto rootSignatureAssociation = raytracingPipeline->CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
        rootSignatureAssociation->AddExports(c_hitGroupNames_TriangleGeometry);
    }
}

// Create a raytracing pipeline state object (RTPSO).
// An RTPSO represents a full set of shaders reachable by a DispatchRays() call,
// with all configuration options resolved, such as local signatures and other state.
void D3D12RaytracingDynamicGeometry::CreateRaytracingPipelineStateObject()
{
    // Create 18 subobjects that combine into a RTPSO:
    // Subobjects need to be associated with DXIL exports (i.e. shaders) either by way of default or explicit associations.
    // Default association applies to every exported shader entrypoint that doesn't have any of the same type of subobject associated with it.
    // This simple sample utilizes default shader association except for local root signature subobject
    // which has an explicit association specified purely for demonstration purposes.
    // 1 - DXIL library
    // 8 - Hit group types - 4 geometries (1 triangle, 3 aabb) x 2 ray types (ray, shadowRay)
    // 1 - Shader config
    // 6 - 3 x Local root signature and association
    // 1 - Global root signature
    // 1 - Pipeline config
    CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

    // DXIL library
    CreateDxilLibrarySubobject(&raytracingPipeline);

    // Hit groups
    CreateHitGroupSubobjects(&raytracingPipeline);

    // Shader config
    // Defines the maximum sizes in bytes for the ray rayPayload and attribute structure.
    auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    UINT payloadSize = max(max(sizeof(RayPayload), sizeof(ShadowRayPayload)), sizeof(GBufferRayPayload));
    UINT attributeSize = sizeof(struct ProceduralPrimitiveAttributes); // ToDo
    shaderConfig->Config(payloadSize, attributeSize);

    // Local root signature and shader association
    // This is a root signature that enables a shader to have unique arguments that come from shader tables.
    CreateLocalRootSignatureSubobjects(&raytracingPipeline);

    // Global root signature
    // This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
    auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
    globalRootSignature->SetRootSignature(m_raytracingGlobalRootSignature.Get());

    // Pipeline config
    // Defines the maximum TraceRay() recursion depth.
    auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
    // PERFOMANCE TIP: Set max recursion depth as low as needed
    // as drivers may apply optimization strategies for low recursion depths.
    UINT maxRecursionDepth = MAX_RAY_RECURSION_DEPTH;
    pipelineConfig->Config(maxRecursionDepth);

    PrintStateObjectDesc(raytracingPipeline);

    // Create the state object.
    ThrowIfFailed(m_dxrDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_dxrStateObject)), L"Couldn't create DirectX Raytracing state object.\n");
}

// Create a 2D output texture for raytracing.
void D3D12RaytracingDynamicGeometry::CreateRaytracingOutputResource()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto backbufferFormat = m_deviceResources->GetBackBufferFormat();

	CreateRenderTargetResource(device, backbufferFormat, m_width, m_height, m_descriptorHeap.get(), &m_raytracingOutput);
}


void D3D12RaytracingDynamicGeometry::CreateGBufferResources()
{
	auto device = m_deviceResources->GetD3DDevice();

	// ToDo tune formats
	CreateRenderTargetResource(device, DXGI_FORMAT_R32G32B32A32_FLOAT, m_width, m_height, m_descriptorHeap.get(), &m_GBufferResources[GBufferResource::Hit]);
	CreateRenderTargetResource(device, DXGI_FORMAT_R32G32B32A32_FLOAT, m_width, m_height, m_descriptorHeap.get(), &m_GBufferResources[GBufferResource::HitPosition]);
	CreateRenderTargetResource(device, DXGI_FORMAT_R32G32B32A32_FLOAT, m_width, m_height, m_descriptorHeap.get(), &m_GBufferResources[GBufferResource::SurfaceNormal]);
}

void D3D12RaytracingDynamicGeometry::CreateAuxilaryDeviceResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandQueue = m_deviceResources->GetCommandQueue();

    for (auto& gpuTimer : m_gpuTimers)
    {
        gpuTimer.RestoreDevice(device, commandQueue, FrameCount);
    }
}

void D3D12RaytracingDynamicGeometry::CreateDescriptorHeap()
{
    auto device = m_deviceResources->GetD3DDevice();

    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
    // Allocate a heap for descriptors:
    // 2 per geometry - vertex and index  buffer SRVs
    // 1 - raytracing output texture SRV
    // 2 per BLAS - one for the acceleration structure and one for its instance desc 
    // 1 - top level acceleration structure
    //ToDo
	UINT NumDescriptors = 2 * GeometryType::Count + 1 + 2 * MaxBLAS + 1 + ARRAYSIZE(SquidRoomAssets::Draws) * 2;
	m_descriptorHeap = make_unique<DescriptorHeap>(device, NumDescriptors);
}

void D3D12RaytracingDynamicGeometry::BuildPlaneGeometry()
{
    auto device = m_deviceResources->GetD3DDevice();
    // Plane indices.
    Index indices[] =
    {
        3, 1, 0,
        2, 1, 3

    };

    // Cube vertices positions and corresponding triangle normals.
    DirectX::VertexPositionNormalTexture vertices[] =
    {
        { XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) }
    };
    AllocateUploadBuffer(device, indices, sizeof(indices), &m_geometries[GeometryType::Plane].ib.resource);
    AllocateUploadBuffer(device, vertices, sizeof(vertices), &m_geometries[GeometryType::Plane].vb.resource);

    // Vertex buffer is passed to the shader along with index buffer as a descriptor range.

    // ToDo revise numElements calculation
    CreateBufferSRV(&m_geometries[GeometryType::Plane].ib, device, static_cast<UINT>(ceil((float)sizeof(indices) / sizeof(UINT))), 0, m_descriptorHeap.get(), &m_geometryIBHeapIndices[GeometryType::Plane]);
    CreateBufferSRV(&m_geometries[GeometryType::Plane].vb, device, ARRAYSIZE(vertices), sizeof(vertices[0]), m_descriptorHeap.get(), &m_geometryVBHeapIndices[GeometryType::Plane]);
    ThrowIfFalse(m_geometryVBHeapIndices[GeometryType::Plane] == m_geometryIBHeapIndices[GeometryType::Plane] + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index");


	m_geometryInstances[GeometryType::Plane].ib.startIndex = 0;
	m_geometryInstances[GeometryType::Plane].ib.count = ARRAYSIZE(indices);

	m_geometryInstances[GeometryType::Plane].vb.startIndex = 0;
	m_geometryInstances[GeometryType::Plane].vb.count = ARRAYSIZE(vertices);

	m_geometryInstances[GeometryType::Plane].ib.gpuDescriptorHandle = m_geometries[GeometryType::Plane].ib.gpuDescriptorHandle;
	m_geometryInstances[GeometryType::Plane].vb.gpuDescriptorHandle = m_geometries[GeometryType::Plane].vb.gpuDescriptorHandle;
}

void D3D12RaytracingDynamicGeometry::BuildTesselatedGeometry()
{
    auto device = m_deviceResources->GetD3DDevice();

    vector<GeometricPrimitive::VertexType> vertices;
    vector<Index> indices;

    const float GeometryRange = 10.f;
    const bool RhCoords = false;

    // ToDo option to reuse multiple geometries
    auto& geometry = m_geometries[GeometryType::Sphere];
    switch (SceneArgs::GeometryTesselationFactor)
    {
    case 0:
        // 24 indices
        GeometricPrimitive::CreateOctahedron(vertices, indices, m_geometryRadius, RhCoords);
        break;
    case 1:
        // 36 indices
        GeometricPrimitive::CreateDodecahedron(vertices, indices, m_geometryRadius, RhCoords);
        break;
    case 2:
        // 60 indices
        GeometricPrimitive::CreateIcosahedron(vertices, indices, m_geometryRadius, RhCoords);
        break;
    default:
        // Tesselation Factor - # Indices:
        // o 3  - 126
        // o 4  - 216
        // o 5  - 330
        // o 10 - 1260
        // o 16 - 3681
        // o 20 - 4920
        const float Diameter = 2 * m_geometryRadius;
        GeometricPrimitive::CreateSphere(vertices, indices, Diameter, SceneArgs::GeometryTesselationFactor, RhCoords);
    }
    AllocateUploadBuffer(device, indices.data(), indices.size() * sizeof(indices[0]), &geometry.ib.resource);
    AllocateUploadBuffer(device, vertices.data(), vertices.size() * sizeof(vertices[0]), &geometry.vb.resource);

    // Vertex buffer is passed to the shader along with index buffer as a descriptor range.
    CreateBufferSRV(&geometry.ib, device, static_cast<UINT>(ceil(static_cast<float>(indices.size() * sizeof(Index)) / sizeof(UINT))) , 0, m_descriptorHeap.get(), &m_geometryIBHeapIndices[GeometryType::Sphere]);
    CreateBufferSRV(&geometry.vb, device, static_cast<UINT>(vertices.size()), sizeof(vertices[0]), m_descriptorHeap.get(), &m_geometryVBHeapIndices[GeometryType::Sphere]);
    ThrowIfFalse(m_geometryVBHeapIndices[GeometryType::Sphere] == m_geometryIBHeapIndices[GeometryType::Sphere] + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index");

	m_geometryInstances[GeometryType::Sphere].ib.startIndex = 0;
	m_geometryInstances[GeometryType::Sphere].ib.count = static_cast<UINT>(indices.size());

	m_geometryInstances[GeometryType::Sphere].vb.startIndex = 0;
	m_geometryInstances[GeometryType::Sphere].vb.count = static_cast<UINT>(vertices.size());

	m_geometryInstances[GeometryType::Sphere].ib.gpuDescriptorHandle = geometry.ib.gpuDescriptorHandle;
	m_geometryInstances[GeometryType::Sphere].vb.gpuDescriptorHandle = geometry.vb.gpuDescriptorHandle;

    // ToDo
    m_numTriangles = static_cast<UINT>(indices.size()) / 3;
}

// ToDo move this out as a helper
void D3D12RaytracingDynamicGeometry::LoadSceneGeometry()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto commandList = m_deviceResources->GetCommandList();

	SquidRoomAssets::LoadGeometry(
		device,
		commandList,
		m_descriptorHeap.get(),
		GetAssetFullPath(SquidRoomAssets::DataFileName).c_str(),
		&m_geometries[GeometryType::SquidRoom].vb,
		m_vertexBufferUpload.Get(),
		&m_geometryVBHeapIndices[GeometryType::SquidRoom],
		&m_geometries[GeometryType::SquidRoom].ib,
		m_indexBufferUpload.Get(),
		&m_geometryIBHeapIndices[GeometryType::SquidRoom],
		&m_geometryInstances);
	ThrowIfFalse(m_geometryVBHeapIndices[GeometryType::SquidRoom] == m_geometryIBHeapIndices[GeometryType::SquidRoom] + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index");

	m_numTriangles = 0;
	for (auto& geometry : m_geometryInstances)
	{
		m_numTriangles += geometry.ib.count / 3;
	}
}

// Build geometry used in the sample.
void D3D12RaytracingDynamicGeometry::InitializeGeometry()
{
    m_geometries.resize(GeometryType::Count);
	m_geometryInstances.resize(GeometryType::Count);
    BuildTesselatedGeometry();
    BuildPlaneGeometry();   

	// Begin frame.
	m_deviceResources->ResetCommandAllocatorAndCommandlist();
#if ONLY_SQUID_SCENE_BLAS
	LoadSceneGeometry();
#endif

#if !RUNTIME_AS_UPDATES
	InitializeAccelerationStructures();

	UpdateSphereGeometryTransforms();
	UpdateBottomLevelASTransforms();

	UpdateAccelerationStructures(m_isASrebuildRequested);
#endif
	m_deviceResources->ExecuteCommandList();
}

void D3D12RaytracingDynamicGeometry::GenerateBottomLevelASInstanceTransforms()
{
#if ONLY_SQUID_SCENE_BLAS
	// Bottom-level AS with a single plane.
	int BLASindex = 0;
	{
		m_vBottomLevelAS[0].SetTransform(XMMatrixIdentity());
	}
#else
    // Bottom-level AS with a single plane.
    int BLASindex = 0;
    {
        // Scale in XZ dimensions.
        float width = 36.0f;
        XMMATRIX mScale = XMMatrixScaling(width, 1.0f, width);
        XMMATRIX mTranslation = XMMatrixTranslationFromVector(XMLoadFloat3(&XMFLOAT3(-width/2.0f, 0.0f, -width/2.0f)));
        XMMATRIX mTransform = mScale * mTranslation;
        m_vBottomLevelAS[BLASindex].SetTransform(mTransform);
        BLASindex += 1;
    }


    // Bottom-level AS with one or more spheres.
    {
        int geometryDim = static_cast<int>(ceil(cbrt(static_cast<double>(SceneArgs::NumGeometriesPerBLAS))));
        float distanceBetweenGeometry = m_geometryRadius;
        float geometryWidth = 2 * m_geometryRadius;

        int dim = static_cast<int>(ceil(sqrt(static_cast<double>(SceneArgs::NumSphereBLAS))));
        float blasWidth = geometryDim * geometryWidth + (geometryDim - 1) * distanceBetweenGeometry;
        float distanceBetweenBLAS = 3 * distanceBetweenGeometry;
        float stepDistance = blasWidth + distanceBetweenBLAS;

        for (int iX = 0; iX < dim; iX++)
            for (int iZ = 0; iZ < dim; iZ++, BLASindex++)
            {
                if (BLASindex - 1 >= SceneArgs::NumSphereBLAS)
                {
                    break;
                }

                XMFLOAT4 translationVector = XMFLOAT4(
                    static_cast<float>(iX),
                    0.0f,
                    static_cast<float>(iZ),
                    0.0f);
                XMMATRIX transform = XMMatrixTranslationFromVector(stepDistance * XMLoadFloat4(&translationVector));
                m_vBottomLevelAS[BLASindex].SetTransform(transform);
            }
    }
#endif

}

// Build acceleration structure needed for raytracing.
void D3D12RaytracingDynamicGeometry::InitializeAccelerationStructures()
{
    auto device = m_deviceResources->GetD3DDevice();
    
    // Build flags.
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

    // Initialize bottom-level AS.
    UINT64 maxScratchResourceSize = 0;
    m_ASmemoryFootprint = 0;
    {
#if ONLY_SQUID_SCENE_BLAS
		m_vBottomLevelAS.resize(1);
		// ToDo apply scale transform to make all scenes using same spatial unit lengths.
		m_vBottomLevelAS[0].Initialize(device, m_geometries[GeometryType::SquidRoom], static_cast<UINT>(m_geometryInstances.size()), buildFlags, DXGI_FORMAT_R32_UINT, sizeof(UINT), SquidRoomAssets::StandardVertexStride, m_geometryInstances);
		m_vBottomLevelAS[0].SetInstanceContributionToHitGroupIndex(0);
		maxScratchResourceSize = max(m_vBottomLevelAS[0].RequiredScratchSize(), maxScratchResourceSize);
		m_ASmemoryFootprint += m_vBottomLevelAS[0].RequiredResultDataSizeInBytes();
		UINT numGeometryTransforms = 1;
#else
        m_vBottomLevelAS.resize(SceneArgs::NumSphereBLAS + 1);
	
        for (UINT i = 0; i < m_vBottomLevelAS.size(); i++)
        {
            UINT numInstances = 0;
            switch (i) 
            {
            case GeometryType::Plane: numInstances = 1; break;
            case GeometryType::Sphere: numInstances = SceneArgs::NumGeometriesPerBLAS;
            };

			std::vector<GeometryInstance> geometryInstances;
			geometryInstances.resize(1, m_geometryInstances[GeometryType::Plane]);
			m_vBottomLevelAS[i].Initialize(device, m_geometries[i], numInstances, buildFlags, DXGI_FORMAT_R16_UINT, sizeof(Index), sizeof(DirectX::GeometricPrimitive::VertexType), geometryInstances);
			m_vBottomLevelAS[i].SetInstanceContributionToHitGroupIndex(i * RayType::Count);
            maxScratchResourceSize = max(m_vBottomLevelAS[i].RequiredScratchSize(), maxScratchResourceSize);
            m_ASmemoryFootprint += m_vBottomLevelAS[i].RequiredResultDataSizeInBytes();
        }
		UINT numGeometryTransforms = SceneArgs::NumSphereBLAS * SceneArgs::NumGeometriesPerBLAS;
#endif
        
        if (m_geometryTransforms.Size() != numGeometryTransforms)
        {
            m_geometryTransforms.Create(device, numGeometryTransforms, FrameCount, L"Geometry descs transforms");
        }
    }

    GenerateBottomLevelASInstanceTransforms();

    // Initialize top-level AS.
    {
        m_topLevelAS.Initialize(device, m_vBottomLevelAS, buildFlags, &m_bottomLevelASinstanceDescsDescritorHeapIndices);
        maxScratchResourceSize = max(m_topLevelAS.RequiredScratchSize(), maxScratchResourceSize);
        m_ASmemoryFootprint += m_topLevelAS.RequiredResultDataSizeInBytes();
    }

    // Create a scratch buffer.
    // ToDo: Compare build perf vs using per AS scratch
    AllocateUAVBuffer(device, maxScratchResourceSize, &m_accelerationStructureScratch, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"Acceleration structure scratch resource");

    m_isASrebuildRequested = true;
}

// Build shader tables.
// This encapsulates all shader records - shaders and the arguments for their local root signatures.
void D3D12RaytracingDynamicGeometry::BuildShaderTables()
{
    auto device = m_deviceResources->GetD3DDevice();

    void* rayGenShaderIDs[RayGenShaderType::Count];
    void* missShaderIDs[RayType::Count];
    void* hitGroupShaderIDs_TriangleGeometry[RayType::Count];

    // A shader name look-up table for shader table debug print out.
    unordered_map<void*, wstring> shaderIdToStringMap;

    auto GetShaderIDs = [&](auto* stateObjectProperties)
    {
		for (UINT i = 0; i < RayGenShaderType::Count; i++)
		{
			rayGenShaderIDs[i] = stateObjectProperties->GetShaderIdentifier(c_rayGenShaderNames[i]);
			shaderIdToStringMap[rayGenShaderIDs[i]] = c_rayGenShaderNames[i];
		}

        for (UINT i = 0; i < RayType::Count; i++)
        {
            missShaderIDs[i] = stateObjectProperties->GetShaderIdentifier(c_missShaderNames[i]);
            shaderIdToStringMap[missShaderIDs[i]] = c_missShaderNames[i];
        }
        for (UINT i = 0; i < RayType::Count; i++)
        {
            hitGroupShaderIDs_TriangleGeometry[i] = stateObjectProperties->GetShaderIdentifier(c_hitGroupNames_TriangleGeometry[i]);
            shaderIdToStringMap[hitGroupShaderIDs_TriangleGeometry[i]] = c_hitGroupNames_TriangleGeometry[i];
        }
    };

    // Get shader identifiers.
    UINT shaderIDSize;
    ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
    ThrowIfFailed(m_dxrStateObject.As(&stateObjectProperties));
    GetShaderIDs(stateObjectProperties.Get());
    shaderIDSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

    /*************--------- Shader table layout -------*******************
    | -------------------------------------------------------------------
	| -------------------------------------------------------------------
	|Shader table - RayGenShaderTable: 32 | 32 bytes
	| [0]: MyRaygenShader, 32 + 0 bytes
	| -------------------------------------------------------------------

	| -------------------------------------------------------------------
	|Shader table - MissShaderTable: 32 | 64 bytes
	| [0]: MyMissShader, 32 + 0 bytes
	| [1]: MyMissShader_ShadowRay, 32 + 0 bytes
	| -------------------------------------------------------------------

	| -------------------------------------------------------------------
	|Shader table - HitGroupShaderTable: 96 | 196800 bytes
	| [0]: MyHitGroup_Triangle, 32 + 56 bytes
	| [1]: MyHitGroup_Triangle_ShadowRay, 32 + 56 bytes
	| [2]: MyHitGroup_Triangle, 32 + 56 bytes
	| [3]: MyHitGroup_Triangle_ShadowRay, 32 + 56 bytes
	| ...
    | --------------------------------------------------------------------
    **********************************************************************/

     // RayGen shader tables.
    {
        UINT numShaderRecords = RayGenShaderType::Count;
		UINT shaderRecordSize = Align(shaderIDSize /*No root arguments */, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
        
		ShaderTable rayGenShaderTable(device, numShaderRecords, shaderRecordSize, L"RayGenShaderTable");
		for (UINT i = 0; i < RayGenShaderType::Count; i++)
		{
			rayGenShaderTable.push_back(ShaderRecord(rayGenShaderIDs[i], shaderIDSize, nullptr, 0));
		}
		rayGenShaderTable.DebugPrint(shaderIdToStringMap);
		m_rayGenShaderTableRecordSizeInBytes = rayGenShaderTable.GetShaderRecordSize();
		m_rayGenShaderTable = rayGenShaderTable.GetResource();
    }
    
    // Miss shader table.
    {
        UINT numShaderRecords = RayType::Count;
        UINT shaderRecordSize = shaderIDSize; // No root arguments

        ShaderTable missShaderTable(device, numShaderRecords, shaderRecordSize, L"MissShaderTable");
        for (UINT i = 0; i < RayType::Count; i++)
        {
            missShaderTable.push_back(ShaderRecord(missShaderIDs[i], shaderIDSize, nullptr, 0));
        }
        missShaderTable.DebugPrint(shaderIdToStringMap);
        m_missShaderTableStrideInBytes = missShaderTable.GetShaderRecordSize();
        m_missShaderTable = missShaderTable.GetResource();
    }

    // Hit group shader table.
    {
		UINT numShaderRecords = static_cast<UINT>(m_geometryInstances.size()) * RayType::Count;
        UINT shaderRecordSize = shaderIDSize + LocalRootSignature::MaxRootArgumentsSize();
        ShaderTable hitGroupShaderTable(device, numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");

#if ONLY_SQUID_SCENE_BLAS
		// Triangle geometry hit groups.
		for (UINT i = 0; i < m_geometryInstances.size(); i++)
		{
			LocalRootSignature::Triangle::RootArguments rootArgs;
			rootArgs.materialCb = m_planeMaterialCB;
			//float value = (float)i / m_geometryInstances.size();
			//rootArgs.materialCb.albedo = XMFLOAT4(value, value, value, value);
			memcpy(&rootArgs.vertexBufferGPUHandle, &m_geometryInstances[i].ib.gpuDescriptorHandle, sizeof(m_geometries[i].ib.gpuDescriptorHandle));
			for (auto& hitGroupShaderID : hitGroupShaderIDs_TriangleGeometry)
			{
				hitGroupShaderTable.push_back(ShaderRecord(hitGroupShaderID, shaderIDSize, &rootArgs, sizeof(rootArgs)));
			}
		}
#else
        // Triangle geometry hit groups.
        for (UINT i = 0; i < GeometryType::Count; i++)
        {
            LocalRootSignature::Triangle::RootArguments rootArgs;
            rootArgs.materialCb = m_planeMaterialCB;
            if (i == GeometryType::Sphere)
                rootArgs.materialCb.albedo = XMFLOAT4(1, 0, 0, 0);
            memcpy(&rootArgs.vertexBufferGPUHandle, &m_geometries[i].ib.gpuDescriptorHandle, sizeof(m_geometries[i].ib.gpuDescriptorHandle));
            for (auto& hitGroupShaderID : hitGroupShaderIDs_TriangleGeometry)
            {
                hitGroupShaderTable.push_back(ShaderRecord(hitGroupShaderID, shaderIDSize, &rootArgs, sizeof(rootArgs)));
            }
        }
#endif
        hitGroupShaderTable.DebugPrint(shaderIdToStringMap);
        m_hitGroupShaderTableStrideInBytes = hitGroupShaderTable.GetShaderRecordSize();
        m_hitGroupShaderTable = hitGroupShaderTable.GetResource();
    }
}

void D3D12RaytracingDynamicGeometry::OnKeyDown(UINT8 key)
{
	// ToDo
    switch (key)
    {
    case VK_ESCAPE:
        throw HrException(E_APPLICATION_EXITING);
    case 'L':
        m_animateLight = !m_animateLight;
        break;
    case 'C':
        m_animateCamera = !m_animateCamera;
        break;
    case 'A':
        m_animateScene = !m_animateScene;
        break;
    default:
        break;
    }
}

// Update frame-based values.
void D3D12RaytracingDynamicGeometry::OnUpdate()
{
    m_timer.Tick();
	// ToDo Switch to time range averaged timers
	m_timerID = (m_timerID + 1) % m_gpuTimers->c_maxTimers;

    float elapsedTime = static_cast<float>(m_timer.GetElapsedSeconds());
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();
    auto prevFrameIndex = m_deviceResources->GetPreviousFrameIndex();

	if (m_isSceneInitializationRequested)
	{
		m_isSceneInitializationRequested = false;
		m_deviceResources->WaitForGpu();
		OnInit();
	}

    CalculateFrameStats();

    GameInput::Update(elapsedTime);
    EngineTuning::Update(elapsedTime);
	
	if (GameInput::IsFirstPressed(GameInput::kKey_f))
	{
		m_isCameraFrozen = !m_isCameraFrozen;
	}

	if (!m_isCameraFrozen)
	{
		m_cameraController->Update(elapsedTime);
	}


    // Rotate the camera around Y axis.
    if (m_animateCamera)
    {
		// ToDo
#if 0
        float secondsToRotateAround = 48.0f;
        float angleToRotateBy = 360.0f * (elapsedTime / secondsToRotateAround);
        XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(angleToRotateBy));
        m_eye = m_at + XMVector3TransformCoord(m_eye - m_at, rotate);
        m_up = XMVector3TransformCoord(m_up, rotate);
#endif
    }
	UpdateCameraMatrices();

    // Rotate the second light around Y axis.
    if (m_animateLight)
    {
        float secondsToRotateAround = 8.0f;
        float angleToRotateBy = -360.0f * (elapsedTime / secondsToRotateAround);
        XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(angleToRotateBy));
        const XMVECTOR& prevLightPosition = m_sceneCB->lightPosition;
        m_sceneCB->lightPosition = XMVector3Transform(prevLightPosition, rotate);
    }
    m_sceneCB->elapsedTime = static_cast<float>(m_timer.GetTotalSeconds());

#if RUNTIME_AS_UPDATES
    // Lazy initialize and update geometries and acceleration structures.
    if (SceneArgs::EnableGeometryAndASBuildsAndUpdates &&
        (m_isGeometryInitializationRequested || m_isASinitializationRequested))
    {
        // Since we'll be recreating D3D resources, GPU needs to be done with the current ones.
		// ToDo
        m_deviceResources->WaitForGpu();

        m_deviceResources->ResetCommandAllocatorAndCommandlist();
        if (m_isGeometryInitializationRequested)
        {
            InitializeGeometry();
        }
        if (m_isASinitializationRequested)
        {
            InitializeAccelerationStructures();
        }

        m_isGeometryInitializationRequested = false;
        m_isASinitializationRequested = false;
        m_deviceResources->ExecuteCommandList();

		// ToDo remove CPU-GPU syncs
		m_deviceResources->WaitForGpu();
    }
    if (m_animateScene)
    {
        UpdateSphereGeometryTransforms();
        UpdateBottomLevelASTransforms();
    }
#endif

	if (m_enableUI)
    {
        UpdateUI();
    }
}

// Parse supplied command line args.
void D3D12RaytracingDynamicGeometry::ParseCommandLineArgs(WCHAR* argv[], int argc)
{
    DXSample::ParseCommandLineArgs(argv, argc);
}

void D3D12RaytracingDynamicGeometry::UpdateAccelerationStructures(bool forceBuild)
{
    auto commandList = m_deviceResources->GetCommandList();
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();
    m_numFramesSinceASBuild++;

    // ToDo move this next to TLAS build? But BLAS update resets its dirty flag
    m_topLevelAS.UpdateInstanceDescTransforms(m_vBottomLevelAS);
    
    BOOL bUpdate = false;    // ~ build or update
    if (!forceBuild)
    {
        switch (SceneArgs::ASUpdateMode)
        {
        case SceneArgs::Update:
            bUpdate = true;
            break;
        case SceneArgs::Build:
            bUpdate = false;
            break;
        case SceneArgs::Update_BuildEveryXFrames:
            bUpdate = m_numFramesSinceASBuild < SceneArgs::ASBuildFrequency;
        default: 
            break;
        };
    }

	m_gpuTimers[GpuTimers::UpdateBLAS].Start(commandList);
	{
#if ONLY_SQUID_SCENE_BLAS
		// ToDo this should be per scene
		// SquidRoom
		{
			// ToDo Heuristic to do an update based on transform amplitude
			D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGpuAddress = 0;
			m_vBottomLevelAS[0].Build(commandList, m_accelerationStructureScratch.Get(), m_descriptorHeap->GetHeap(), baseGeometryTransformGpuAddress, bUpdate);
		}
#else
		// Plane
		{
			D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGpuAddress = 0;
			m_vBottomLevelAS[BottomLevelASType::Plane].Build(commandList, m_accelerationStructureScratch.Get(), m_descriptorHeap->GetHeap(), baseGeometryTransformGpuAddress, bUpdate);
		}
		// Sphere
		{
            D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGpuAddress = 0;     
#if USE_GPU_TRANSFORM
            baseGeometryTransformGpuAddress = m_geometryTransforms.GpuVirtualAddress(frameIndex);
#endif

			m_geometryTransforms.CopyStagingToGpu(frameIndex);
			m_vBottomLevelAS[BottomLevelASType::Sphere].Build(commandList, m_accelerationStructureScratch.Get(), m_descriptorHeap->GetHeap(), baseGeometryTransformGpuAddress, bUpdate);
        }
#endif
    }
	m_gpuTimers[GpuTimers::UpdateBLAS].Stop(commandList);

    m_gpuTimers[GpuTimers::UpdateTLAS].Start(commandList);
    m_topLevelAS.Build(commandList, m_accelerationStructureScratch.Get(), m_descriptorHeap->GetHeap(), bUpdate);
    m_gpuTimers[GpuTimers::UpdateTLAS].Stop(commandList);

    if (!bUpdate)
    {
        m_numFramesSinceASBuild = 0;
    }
}

void D3D12RaytracingDynamicGeometry::DoRaytracing()
{
    auto commandList = m_deviceResources->GetCommandList();
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    auto DispatchRays = [&](auto* raytracingCommandList, auto* stateObject)
    {
		D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};

        dispatchDesc.HitGroupTable.StartAddress = m_hitGroupShaderTable->GetGPUVirtualAddress();
        dispatchDesc.HitGroupTable.SizeInBytes = m_hitGroupShaderTable->GetDesc().Width;
        dispatchDesc.HitGroupTable.StrideInBytes = m_hitGroupShaderTableStrideInBytes;
        dispatchDesc.MissShaderTable.StartAddress = m_missShaderTable->GetGPUVirtualAddress();
        dispatchDesc.MissShaderTable.SizeInBytes = m_missShaderTable->GetDesc().Width;
        dispatchDesc.MissShaderTable.StrideInBytes = m_missShaderTableStrideInBytes;
        dispatchDesc.RayGenerationShaderRecord.StartAddress = m_rayGenShaderTable->GetGPUVirtualAddress() + RayGenShaderType::PrimaryAndAO * m_rayGenShaderTableRecordSizeInBytes;
        dispatchDesc.RayGenerationShaderRecord.SizeInBytes = m_rayGenShaderTableRecordSizeInBytes;
        dispatchDesc.Width = m_width;
        dispatchDesc.Height = m_height;
		dispatchDesc.Depth = 1;
        raytracingCommandList->SetPipelineState1(stateObject);

        m_gpuTimers[GpuTimers::Raytracing].Start(commandList, m_timerID);
        raytracingCommandList->DispatchRays(&dispatchDesc);
        m_gpuTimers[GpuTimers::Raytracing].Stop(commandList, m_timerID);
    };

    auto SetCommonPipelineState = [&](auto* descriptorSetCommandList)
    {
        descriptorSetCommandList->SetDescriptorHeaps(1, m_descriptorHeap->GetAddressOf());
        // Set index and successive vertex buffer decriptor tables.

        commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::OutputView, m_raytracingOutput.gpuDescriptor);
		commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::GBufferResources, m_raytracingOutput.gpuDescriptor);
    };

    commandList->SetComputeRootSignature(m_raytracingGlobalRootSignature.Get());

	uniform_int_distribution<UINT> seedDistribution(0, UINT_MAX);
	
	static UINT seed = 0;
    m_sceneCB->seed = seedDistribution(m_generatorURNG);
    m_sceneCB->numSamples = m_randomSampler.NumSamples();
    m_sceneCB->numSampleSets = m_randomSampler.NumSampleSets();
#if 1
    m_sceneCB->numSamplesToUse = m_randomSampler.NumSamples();    UINT NumFramesPerIter = 400;
#else
    UINT NumFramesPerIter = 100;
    static UINT frameID = NumFramesPerIter * 4;
    m_sceneCB->numSamplesToUse = (frameID++ / NumFramesPerIter) % m_randomSampler.NumSamples();
#endif
    // Copy dynamic buffers to GPU.
    {
        m_hemisphereSamplesGPUBuffer.CopyStagingToGpu(frameIndex);
        commandList->SetComputeRootShaderResourceView(GlobalRootSignature::Slot::SampleBuffers, m_hemisphereSamplesGPUBuffer.GpuVirtualAddress(frameIndex));

        m_sceneCB.CopyStagingToGpu(frameIndex);
        commandList->SetComputeRootConstantBufferView(GlobalRootSignature::Slot::SceneConstant, m_sceneCB.GpuVirtualAddress(frameIndex));
    }

    // Bind the heaps, acceleration structure and dispatch rays. 
    SetCommonPipelineState(commandList);
    commandList->SetComputeRootShaderResourceView(GlobalRootSignature::Slot::AccelerationStructure, m_topLevelAS.GetResource()->GetGPUVirtualAddress());
    DispatchRays(commandList, m_dxrStateObject.Get());
}


void D3D12RaytracingDynamicGeometry::DoRaytracingGBufferAndAOPasses()
{
	auto commandList = m_deviceResources->GetCommandList();
	auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

	auto DispatchRays = [&](auto* raytracingCommandList, auto* stateObject)
	{
		D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};

		dispatchDesc.HitGroupTable.StartAddress = m_hitGroupShaderTable->GetGPUVirtualAddress();
		dispatchDesc.HitGroupTable.SizeInBytes = m_hitGroupShaderTable->GetDesc().Width;
		dispatchDesc.HitGroupTable.StrideInBytes = m_hitGroupShaderTableStrideInBytes;
		dispatchDesc.MissShaderTable.StartAddress = m_missShaderTable->GetGPUVirtualAddress();
		dispatchDesc.MissShaderTable.SizeInBytes = m_missShaderTable->GetDesc().Width;
		dispatchDesc.MissShaderTable.StrideInBytes = m_missShaderTableStrideInBytes;
		dispatchDesc.RayGenerationShaderRecord.StartAddress = m_rayGenShaderTable->GetGPUVirtualAddress() + RayGenShaderType::PrimaryAndAO * m_rayGenShaderTableRecordSizeInBytes;
		dispatchDesc.RayGenerationShaderRecord.SizeInBytes = m_rayGenShaderTableRecordSizeInBytes;
		dispatchDesc.Width = m_width;
		dispatchDesc.Height = m_height;
		dispatchDesc.Depth = 1;
		raytracingCommandList->SetPipelineState1(stateObject);

		m_gpuTimers[GpuTimers::Raytracing].Start(commandList, m_timerID);
		raytracingCommandList->DispatchRays(&dispatchDesc);
		m_gpuTimers[GpuTimers::Raytracing].Stop(commandList, m_timerID);
	};

	auto SetCommonPipelineState = [&](auto* descriptorSetCommandList)
	{
		descriptorSetCommandList->SetDescriptorHeaps(1, m_descriptorHeap->GetAddressOf());
		// Set index and successive vertex buffer decriptor tables.

		commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::OutputView, m_raytracingOutput.gpuDescriptor);
		commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::GBufferResources, m_raytracingOutput.gpuDescriptor);
	};

	commandList->SetComputeRootSignature(m_raytracingGlobalRootSignature.Get());

	uniform_int_distribution<UINT> seedDistribution(0, UINT_MAX);

	static UINT seed = 0;
	m_sceneCB->seed = seedDistribution(m_generatorURNG);
	m_sceneCB->numSamples = m_randomSampler.NumSamples();
	m_sceneCB->numSampleSets = m_randomSampler.NumSampleSets();
#if 1
	m_sceneCB->numSamplesToUse = m_randomSampler.NumSamples();    UINT NumFramesPerIter = 400;
#else
	UINT NumFramesPerIter = 100;
	static UINT frameID = NumFramesPerIter * 4;
	m_sceneCB->numSamplesToUse = (frameID++ / NumFramesPerIter) % m_randomSampler.NumSamples();
#endif
	// Copy dynamic buffers to GPU.
	{
		m_hemisphereSamplesGPUBuffer.CopyStagingToGpu(frameIndex);
		commandList->SetComputeRootShaderResourceView(GlobalRootSignature::Slot::SampleBuffers, m_hemisphereSamplesGPUBuffer.GpuVirtualAddress(frameIndex));

		m_sceneCB.CopyStagingToGpu(frameIndex);
		commandList->SetComputeRootConstantBufferView(GlobalRootSignature::Slot::SceneConstant, m_sceneCB.GpuVirtualAddress(frameIndex));
	}

	// Bind the heaps, acceleration structure and dispatch rays. 
	SetCommonPipelineState(commandList);
	commandList->SetComputeRootShaderResourceView(GlobalRootSignature::Slot::AccelerationStructure, m_topLevelAS.GetResource()->GetGPUVirtualAddress());
	DispatchRays(commandList, m_dxrStateObject.Get());
}


// Copy the raytracing output to the backbuffer.
void D3D12RaytracingDynamicGeometry::CopyRaytracingOutputToBackbuffer(D3D12_RESOURCE_STATES outRenderTargetState)
{
    auto commandList = m_deviceResources->GetCommandList();
    auto renderTarget = m_deviceResources->GetRenderTarget();

    D3D12_RESOURCE_BARRIER preCopyBarriers[2];
    preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
    preCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_raytracingOutput.resource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
    commandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);

    commandList->CopyResource(renderTarget, m_raytracingOutput.resource.Get());

    D3D12_RESOURCE_BARRIER postCopyBarriers[2];
    postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_COPY_DEST, outRenderTargetState);
    postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_raytracingOutput.resource.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    commandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
}

void D3D12RaytracingDynamicGeometry::UpdateUI()
{
	// ToDo average/smoothen numbers of 1/4 second.
    vector<wstring> labels;
#if 1
    // Main runtime information.
    {
        wstringstream wLabel;
        wLabel.precision(1);
        wLabel << L" GPU[" << m_deviceResources->GetAdapterID() << L"]: " 
               << m_deviceResources->GetAdapterDescription() << L"\n";
        wLabel << fixed << L" FPS: " << m_fps << L"\n";
		wLabel << fixed << L" DispatchRays: " << m_gpuTimers[GpuTimers::Raytracing].GetAverageMS()
			   << L"ms" << L"     ~Million Primary Rays/s: " << NumMRaysPerSecond()
   			   << L"   ~Million AO rays/s" << NumMRaysPerSecond() * c_sppAO
               << L"\n";
        wLabel << fixed << L" AS update (BLAS / TLAS / Total): "
               << m_gpuTimers[GpuTimers::UpdateBLAS].GetElapsedMS() << L"ms / "
               << m_gpuTimers[GpuTimers::UpdateTLAS].GetElapsedMS() << L"ms / "
               << m_gpuTimers[GpuTimers::UpdateBLAS].GetElapsedMS() +
                  m_gpuTimers[GpuTimers::UpdateTLAS].GetElapsedMS() << L"ms\n";
    
        labels.push_back(wLabel.str());
    }

    // Parameters.
    labels.push_back(L"\n");
    {
        wstringstream wLabel;
        wLabel << L"Scene:" << L"\n";
        wLabel << L" " << L"AS update mode: " << SceneArgs::ASUpdateMode << L"\n";
        wLabel.precision(2);
        wLabel << L" " << L"AS memory footprint: " << static_cast<double>(m_ASmemoryFootprint)/(1024*1024) << L"MB\n";
        wLabel << L" " << L" # triangles per geometry: " << m_numTriangles << L"\n";
        wLabel << L" " << L" # geometries per BLAS: " << SceneArgs::NumGeometriesPerBLAS << L"\n";
        wLabel << L" " << L" # Sphere BLAS: " << SceneArgs::NumSphereBLAS << L"\n";
        wLabel << L" " << L" # total triangles: " << SceneArgs::NumSphereBLAS * SceneArgs::NumGeometriesPerBLAS* m_numTriangles << L"\n";
        // ToDo AS memory
        labels.push_back(wLabel.str());
    }

    // Engine tuning.
    {
        wstringstream wLabel;
        wLabel << L"\n\n";
        EngineTuning::Display(&wLabel);
        labels.push_back(wLabel.str());
    }

	// Sampling info:
	{
		wstringstream wLabel;
		wLabel << L"\n";
		wLabel << L"Num samples: " << m_randomSampler.NumSamples() << L"\n";
		wLabel << L"Sample set: " << m_computeCB->sampleSetBase / m_randomSampler.NumSamples() << " / " << m_randomSampler.NumSampleSets() << L"\n";
		
		labels.push_back(wLabel.str());
	}
#endif
    wstring uiText = L"";
    for (auto s : labels)
    {
        uiText += s;
    }

	m_uiLayer->UpdateLabels(uiText);
}

// Create resources that are dependent on the size of the main window.
void D3D12RaytracingDynamicGeometry::CreateWindowSizeDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandQueue = m_deviceResources->GetCommandQueue();
    auto renderTargets = m_deviceResources->GetRenderTargets();

    // Create an output 2D texture to store the raytracing result to.
    CreateRaytracingOutputResource();

	CreateGBufferResources();
        
    if (m_enableUI)
    {
        if (!m_uiLayer)
        {
            m_uiLayer = make_unique<UILayer>(FrameCount, device, commandQueue);
        }
        m_uiLayer->Resize(renderTargets, m_width, m_height);
    }
}

// Release resources that are dependent on the size of the main window.
void D3D12RaytracingDynamicGeometry::ReleaseWindowSizeDependentResources()
{
    if (m_enableUI)
    {
        m_uiLayer.reset();
    }
    m_raytracingOutput.resource.Reset();
}

// Release all resources that depend on the device.
void D3D12RaytracingDynamicGeometry::ReleaseDeviceDependentResources()
{
    for (auto& gpuTimer : m_gpuTimers)
    {
        gpuTimer.ReleaseDevice();
    }
	if (m_enableUI)
	{
		m_uiLayer.reset();
	}

    m_raytracingGlobalRootSignature.Reset();
    ResetComPtrArray(&m_raytracingLocalRootSignature);

    m_dxrDevice.Reset();
    m_dxrStateObject.Reset();

    m_raytracingGlobalRootSignature.Reset();
    ResetComPtrArray(&m_raytracingLocalRootSignature);

	m_descriptorHeap.release();

    m_computeCB.Release();

    // ToDo
    for (auto& bottomLevelAS : m_vBottomLevelAS)
    {
        bottomLevelAS.ReleaseD3DResources();
    }
    m_topLevelAS.ReleaseD3DResources();

    m_raytracingOutput.resource.Reset();
	m_rayGenShaderTable.Reset();
    m_missShaderTable.Reset();
    m_hitGroupShaderTable.Reset();
}

void D3D12RaytracingDynamicGeometry::RecreateD3D()
{
    // Give GPU a chance to finish its execution in progress.
    try
    {
        m_deviceResources->WaitForGpu();
    }
    catch (HrException&)
    {
        // Do nothing, currently attached adapter is unresponsive.
    }
    m_deviceResources->HandleDeviceLost();
}

void D3D12RaytracingDynamicGeometry::RenderRNGVisualizations()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

	// Make sure execution for the current index FrameCount frames ago is finished.
	{
		// Schedule a Signal command in the queue.
		UINT prevFrameIndex = (frameIndex + (FrameCount - 1)) % FrameCount;
		const UINT64 currentFenceValue = m_fenceValues[prevFrameIndex];
		ThrowIfFailed(m_computeCommandQueue->Signal(m_fence.Get(), currentFenceValue));

		// If the next frame is not ready to be rendered yet, wait until it is ready.
		if (m_fence->GetCompletedValue() < m_fenceValues[frameIndex])
		{
			ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[frameIndex], m_fenceEvent.Get()));
			WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, FALSE);
		}

		// Set the fence value for the next frame.
		m_fenceValues[frameIndex] = currentFenceValue + 1;
	}

    m_computeAllocators[frameIndex]->Reset();
    m_computeCommandList->Reset(m_computeAllocators[frameIndex].Get(), m_computePSO.Get());

    commandList->SetDescriptorHeaps(1, m_descriptorHeap->GetAddressOf());
    commandList->SetComputeRootSignature(m_csSamleVisualizerRootSignature.Get());

    XMUINT2 rngWindowSize(256, 256);
    m_computeCB->dispatchDimensions = rngWindowSize;

    static UINT seed = 0;
	UINT NumFramesPerIter = 400;
	static UINT frameID = NumFramesPerIter * 4;
	m_computeCB->numSamplesToShow = c_sppAO;// (frameID++ / NumFramesPerIter) % m_randomSampler.NumSamples();
	m_computeCB->sampleSetBase = ((seed++ / NumFramesPerIter) % m_randomSampler.NumSampleSets()) * m_randomSampler.NumSamples();
    m_computeCB->stratums = XMUINT2(static_cast<UINT>(sqrt(m_randomSampler.NumSamples())), 
                                    static_cast<UINT>(sqrt(m_randomSampler.NumSamples())));
    m_computeCB->grid = XMUINT2(m_randomSampler.NumSamples(), m_randomSampler.NumSamples());
    m_computeCB->uavOffset = XMUINT2(0 /*m_width - rngWindowSize.x*/, m_height - rngWindowSize.y);
    m_computeCB->numSamples = m_randomSampler.NumSamples();
    m_computeCB->numSampleSets = m_randomSampler.NumSampleSets();

    // Copy dynamic buffers to GPU
    {
        m_computeCB.CopyStagingToGpu(frameIndex);
        m_samplesGPUBuffer.CopyStagingToGpu(frameIndex);
    }
    commandList->SetComputeRootConstantBufferView(RNGVisualizerRootSignature::Slot::SceneConstant, m_computeCB.GpuVirtualAddress(frameIndex));
    commandList->SetComputeRootShaderResourceView(RNGVisualizerRootSignature::Slot::SampleBuffers, m_samplesGPUBuffer.GpuVirtualAddress(frameIndex));
    commandList->SetComputeRootDescriptorTable(RNGVisualizerRootSignature::Slot::OutputView, m_raytracingOutput.gpuDescriptor);

    commandList->SetPipelineState(m_computePSO.Get());
 
    commandList->Dispatch(rngWindowSize.x, rngWindowSize.y, 1);

    // close and execute the command list
    m_computeCommandList->Close();
    ID3D12CommandList *tempList = m_computeCommandList.Get();
    m_computeCommandQueue->ExecuteCommandLists(1, &tempList);
	
}

// Render the scene.
void D3D12RaytracingDynamicGeometry::OnRender()
{
    if (!m_deviceResources->IsWindowVisible())
    {
        return;
    }

    auto commandList = m_deviceResources->GetCommandList();

    // Begin frame.
    m_deviceResources->Prepare();
    for (auto& gpuTimer : m_gpuTimers)
    {
        gpuTimer.BeginFrame(commandList);
    }

#if RUNTIME_AS_UPDATES
    // Update acceleration structures.
    if (m_isASrebuildRequested && SceneArgs::EnableGeometryAndASBuildsAndUpdates)
    {
        UpdateAccelerationStructures(m_isASrebuildRequested);
        m_isASrebuildRequested = false;
    }
#endif

#if DO_RAYTRACING
    // Render.
	DoRaytracing();
#endif

#if SAMPLES_CS_VISUALIZATION 
    RenderRNGVisualizations();
#endif

	// UILayer will transition backbuffer to a present state.
    CopyRaytracingOutputToBackbuffer(m_enableUI ? D3D12_RESOURCE_STATE_RENDER_TARGET : D3D12_RESOURCE_STATE_PRESENT);

    // End frame.
    for (auto& gpuTimer : m_gpuTimers)
    {
        gpuTimer.EndFrame(commandList);
    }
    m_deviceResources->ExecuteCommandList();

    // UI overlay.
    if (m_enableUI)
    {
        m_uiLayer->Render(m_deviceResources->GetCurrentFrameIndex());
    }
    
    m_deviceResources->Present(D3D12_RESOURCE_STATE_PRESENT);
}

// Compute the average frames per second and million rays per second.
void D3D12RaytracingDynamicGeometry::CalculateFrameStats()
{
    static int frameCnt = 0;
    static double prevTime = 0.0f;
    double totalTime = m_timer.GetTotalSeconds();

    frameCnt++;

    // Compute averages over one second period.
    if ((totalTime - prevTime) >= 1.0f)
    {
        float diff = static_cast<float>(totalTime - prevTime);
        m_fps = static_cast<float>(frameCnt) / diff; // Normalize to an exact second.

        frameCnt = 0;
        prevTime = totalTime;
        
        // Display partial UI on the window title bar if UI is disabled.
        if (1)//!m_enableUI)
        {
            wstringstream windowText;
            windowText << setprecision(2) << fixed
                << L"    fps: " << m_fps << L"     ~Million Primary Rays/s: " << NumMRaysPerSecond()
                << L"    GPU[" << m_deviceResources->GetAdapterID() << L"]: " << m_deviceResources->GetAdapterDescription();
            SetCustomWindowText(windowText.str().c_str());
        }
    }
}

// Handle OnSizeChanged message event.
void D3D12RaytracingDynamicGeometry::OnSizeChanged(UINT width, UINT height, bool minimized)
{
    UpdateForSizeChange(width, height);

    if (!m_deviceResources->WindowSizeChanged(width, height, minimized))
    {
        return;
    }
}