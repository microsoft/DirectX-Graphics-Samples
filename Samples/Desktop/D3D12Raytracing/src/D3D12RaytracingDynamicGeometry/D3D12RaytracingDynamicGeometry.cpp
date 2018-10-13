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
const wchar_t* D3D12RaytracingDynamicGeometry::c_raygenShaderName = L"MyRaygenShader";
const wchar_t* D3D12RaytracingDynamicGeometry::c_intersectionShaderNames[] =
{
    L"MyIntersectionShader_AnalyticPrimitive",
    L"MyIntersectionShader_VolumetricPrimitive",
    L"MyIntersectionShader_SignedDistancePrimitive",
};
const wchar_t* D3D12RaytracingDynamicGeometry::c_closestHitShaderNames[] =
{
    L"MyClosestHitShader_Triangle"
};
const wchar_t* D3D12RaytracingDynamicGeometry::c_missShaderNames[] =
{
    L"MyMissShader", L"MyMissShader_ShadowRay"
};
// Hit groups.
const wchar_t* D3D12RaytracingDynamicGeometry::c_hitGroupNames_TriangleGeometry[] = 
{ 
    L"MyHitGroup_Triangle", L"MyHitGroup_Triangle_ShadowRay" 
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

    enum RaytracingMode { FLDXR = 0, FL, DXR };
    const WCHAR* RaytracingModes[] = { L"FL-DXR", L"FL",L"DXR" };
    // ToDo EnumVar RaytracingMode(L"RaytracingMode", FLDXR, _countof(RaytracingModes), RaytracingModes);

    BoolVar EnableGeometryAndASBuildsAndUpdates(L"Enable geometry & AS builds and updates", true);

    enum UpdateMode { Build = 0, Update, Update_BuildEveryXFrames };
    const WCHAR* UpdateModes[] = { L"Build only", L"Update only", L"Update + build every X frames" };
    EnumVar ASUpdateMode(L"Acceleration structure/Update mode", Build, _countof(UpdateModes), UpdateModes);
    IntVar ASBuildFrequency(L"Acceleration structure/Rebuild frame frequency", 1, 1, 1200, 1);
    BoolVar ASMinimizeMemory(L"Acceleration structure/Minimize memory", false, OnASChange, nullptr);
    BoolVar ASAllowUpdate(L"Acceleration structure/Allow update", true, OnASChange, nullptr);
    enum BuildFlag { Default = 0, FastTrace, FastBuild };
    const WCHAR* BuildFlags[] = { L"Default", L"Fast trace", L"Fast build" };
    EnumVar ASBuildFlag(L"Acceleration structure/Build quality", FastTrace, _countof(BuildFlags), BuildFlags, OnASChange, nullptr);

    // ToDo test tessFactor 16
	// ToDo fix alias on TessFactor 2
    IntVar GeometryTesselationFactor(L"Geometry/Tesselation factor", 14, 0, 80, 1, OnGeometryChange, nullptr);
    IntVar NumGeometriesPerBLAS(L"Geometry/# geometries per BLAS", 1, 1, 1000, 1, OnGeometryChange, nullptr);
    IntVar NumSphereBLAS(L"Geometry/# Sphere BLAS", 1, 1, D3D12RaytracingDynamicGeometry::MaxBLAS, 1, OnASChange, nullptr);
};


D3D12RaytracingDynamicGeometry::D3D12RaytracingDynamicGeometry(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_raytracingOutputResourceUAVDescriptorHeapIndex(UINT_MAX),
    m_animateCamera(false),
    m_animateLight(false),
    m_animateScene(true),
    m_isDxrSupported(false),
    m_descriptorsAllocated(0),
    m_descriptorSize(0),
    m_missShaderTableStrideInBytes(UINT_MAX),
    m_hitGroupShaderTableStrideInBytes(UINT_MAX),
    m_forceComputeFallback(false),
    m_numTrianglesPerGeometry(0),
    m_isGeometryInitializationRequested(true),
    m_isASinitializationRequested(true),
    m_isASrebuildRequested(true),
    m_ASmemoryFootprint(0),
    m_numFramesSinceASBuild(0),
	m_isCameraFrozen(false)
{
    g_pSample = this;
    m_forceComputeFallback = false;
    SelectRaytracingAPI(RaytracingAPI::FallbackLayer);
    UpdateForSizeChange(width, height);
    m_bottomLevelASdescritorHeapIndices.resize(MaxBLAS, UINT_MAX);
    m_bottomLevelASinstanceDescsDescritorHeapIndices.resize(MaxBLAS, UINT_MAX);
    m_topLevelASdescritorHeapIndex = UINT_MAX;
    m_geometryIBHeapIndices.resize(GeometryType::Count, UINT_MAX);
    m_geometryVBHeapIndices.resize(GeometryType::Count, UINT_MAX);
	
	m_generatorURNG.seed(1729);
}

void D3D12RaytracingDynamicGeometry::EnableDirectXRaytracing(IDXGIAdapter1* adapter)
{
    // Fallback Layer uses an experimental feature and needs to be enabled before creating a D3D12 device.
	bool isFallbackSupported = false;// EnableComputeRaytracingFallback(adapter);

    if (!isFallbackSupported)
    {
        OutputDebugString(
            L"Warning: Could not enable Compute Raytracing Fallback (D3D12EnableExperimentalFeatures() failed).\n" \
            L"         Possible reasons: your OS is not in developer mode.\n\n");
    }

    m_isDxrSupported = IsDirectXRaytracingSupported(adapter);

    if (!m_isDxrSupported)
    {
        OutputDebugString(L"Warning: DirectX Raytracing is not supported by your GPU and driver.\n\n");

        ThrowIfFalse(isFallbackSupported,
            L"Could not enable compute based fallback raytracing support (D3D12EnableExperimentalFeatures() failed).\n"\
            L"Possible reasons: your OS is not in developer mode.\n\n");
        m_raytracingAPI = RaytracingAPI::FallbackLayer;
    }
}

void D3D12RaytracingDynamicGeometry::OnInit()
{
    m_deviceResources = std::make_unique<DeviceResources>(
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_UNKNOWN,
        FrameCount,
        D3D_FEATURE_LEVEL_11_0,
        // Sample shows handling of use cases with tearing support, which is OS dependent and has been supported since TH2.
        // Since the Fallback Layer requires Fall Creator's update (RS3), we don't need to handle non-tearing cases.
        DeviceResources::c_RequireTearingSupport,
        m_adapterIDoverride
        );
    m_deviceResources->RegisterDeviceNotify(this);
    m_deviceResources->SetWindow(Win32Application::GetHwnd(), m_width, m_height);

    g_hWnd = Win32Application::GetHwnd();
    GameInput::Initialize();
    EngineTuning::Initialize();

    m_deviceResources->InitializeDXGIAdapter();
    EnableDirectXRaytracing(m_deviceResources->GetAdapter());
    
    // ToDo cleanup
    m_deviceResources->CreateDeviceResources();
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


	float fovAngleY = 90.0f;
	XMMATRIX view, proj;
	m_camera.GetProj(&proj, fovAngleY, m_width, m_height);

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
    m_geometryTransforms.CopyStagingToGpu(frameIndex);
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
            auto& attributes = m_aabbMaterialCB[primitiveIndex];
            attributes.albedo = albedo;
            attributes.reflectanceCoef = reflectanceCoef;
            attributes.diffuseCoef = diffuseCoef;
            attributes.specularCoef = specularCoef;
            attributes.specularPower = specularPower;
            attributes.stepScale = stepScale;
        };


        m_planeMaterialCB = { XMFLOAT4(0.75f, 0.75f, 0.75f, 1.0f), 0.0f, 1, 0.4f, 50, 1};


        // Albedos
        XMFLOAT4 green = XMFLOAT4(0.1f, 1.0f, 0.5f, 1.0f);
        XMFLOAT4 red = XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f);
        XMFLOAT4 yellow = XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f);

#if 0
        UINT offset = 0;
        // Analytic primitives.
        {
            using namespace AnalyticPrimitive;
            SetAttributes(offset + Spheres, ChromiumReflectance, 1);
            offset += AnalyticPrimitive::Count;
        }
#endif
    }

    // Setup camera.
    {
        // Initialize the view and projection inverse matrices.
#if ONLY_SQUID_SCENE_BLAS
#if 1
		XMVECTOR eye = XMVectorSet(0.0f, 80, -268.555980f, 1);
		XMVECTOR at = XMVectorSet(0, 80, 0, 1);
		XMVECTOR up = XMVectorSet(0, 1, 0, 0);
#elif 0
		XMVECTOR eye = XMVectorSet(215.815994, 1238.98132, -2074.55811, 1.00000000);
		XMVECTOR at = XMVectorSet(188.561310, 1094.73657, -1849.67334, 1.00000000);
		XMVECTOR up = XMVectorSet(-0.0206577014, 0.842688203, 0.538005412, 0.000000000);
#else
		XMVECTOR eye = XMVectorSet(0.0f, 80, -268.555980f, 1);
		XMVECTOR at = XMVectorSet(0, 80, 0, 1);
		XMVECTOR up = XMVectorSet(0, 1, 0, 0);
#endif
#if 0
		m_camera.SetEyeAtUp(eye, at, up);
		m_camera.SetZRange(1.0f, 10000.0f);
		m_camera.SetPerspectiveMatrix(XM_PIDIV4, static_cast<float>(m_height) / m_width, 0.1f, 1000.f);
#else
		m_camera.Set(eye, at, up);
#endif
#else
		m_eye = { 0.0f, 6.3f, -17.0f, 1.0f };
		m_at = { 0.0f, 1.0f, 0.0f, 1.0f };
		XMVECTOR right = { 1.0f, 0.0f, 0.0f, 0.0f };

		XMVECTOR direction = XMVector4Normalize(m_at - m_eye);
		m_up = XMVector3Normalize(XMVector3Cross(direction, right));

		// Rotate camera around Y axis.
		XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(45.0f));
		m_eye = XMVector3Transform(m_eye, rotate);
		m_up = XMVector3Transform(m_up, rotate);
#endif

        UpdateCameraMatrices();
    }
	m_cameraController = make_unique<CameraController>(m_camera);

    // Setup lights.
    {
        // Initialize the lighting parameters.
        XMFLOAT4 lightPosition;
        XMFLOAT4 lightAmbientColor;
        XMFLOAT4 lightDiffuseColor;
#if ONLY_SQUID_SCENE_BLAS
        lightPosition = XMFLOAT4(0.0f, 50.0f, -60.0f, 0.0f);
#else
		lightPosition = XMFLOAT4(0.0f, 50.0f, -60.0f, 0.0f);
#endif
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

// Create AABB primitive attributes buffers.
void D3D12RaytracingDynamicGeometry::CreateAABBPrimitiveAttributesBuffers()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto frameCount = m_deviceResources->GetBackBufferCount();
    m_aabbPrimitiveAttributeBuffer.Create(device, IntersectionShaderType::TotalPrimitiveCount, frameCount, L"AABB primitive attributes");
}

void D3D12RaytracingDynamicGeometry::CreateSamplesRNG()
{
    auto device = m_deviceResources->GetD3DDevice(); 
    auto frameCount = m_deviceResources->GetBackBufferCount();

    m_randomSampler.Reset(9, 83, Samplers::HemisphereDistribution::Cosine);

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

        m_fenceEvent.Attach(CreateEvent(nullptr, FALSE, FALSE, nullptr));
        if (!m_fenceEvent.IsValid())
        {
            ThrowIfFailed(E_FAIL, L"CreateEvent failed.\n");
        }
    }

    // Create shader resources
    {
        m_computeCB.Create(device, frameCount, L"GPU CB: RNG");
#if 1
        m_samplesGPUBuffer.Create(device, m_randomSampler.NumSamples() * m_randomSampler.NumSampleSets(), frameCount, L"GPU buffer: Random unit square samples");

        m_hemisphereSamplesGPUBuffer.Create(device, m_randomSampler.NumSamples() * m_randomSampler.NumSampleSets(), frameCount, L"GPU buffer: Random hemisphere samples");

        for (UINT i = 0; i < m_randomSampler.NumSamples() * m_randomSampler.NumSampleSets(); i++)
        {
            //sample.value = m_randomSampler.GetSample2D();
            XMFLOAT3 p = m_randomSampler.GetHemisphereSample3D();
            m_samplesGPUBuffer[i].value = XMFLOAT2(p.x*0.5f + 0.5f, p.y*0.5f + 0.5f);
            m_hemisphereSamplesGPUBuffer[i].value = p;
        }
#endif
    }
}

// Create resources that depend on the device.
void D3D12RaytracingDynamicGeometry::CreateDeviceDependentResources()
{
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

#if ENABLE_RAYTRACING
    // Create root signatures for the shaders.
    CreateRootSignatures();

    // Create a raytracing pipeline state object which defines the binding of shaders, state and resources to be used during raytracing.
    CreateRaytracingPipelineStateObject();
#endif

    // Create constant buffers for the geometry and the scene.
    CreateConstantBuffers();

    // Create AABB primitive attribute buffers.
    CreateAABBPrimitiveAttributesBuffers();

#if ENABLE_RAYTRACING
    // Build shader tables, which define shaders and their local root arguments.
    BuildShaderTables();
#endif

    // Create an output 2D texture to store the raytracing result to.
    CreateRaytracingOutputResource();

    CreateSamplesRNG();
}

void D3D12RaytracingDynamicGeometry::SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig, LPCWSTR resourceName)
{
    auto device = m_deviceResources->GetD3DDevice();
    ComPtr<ID3DBlob> blob;
    ComPtr<ID3DBlob> error;

    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        ThrowIfFailed(m_fallbackDevice->D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error), error ? static_cast<wchar_t*>(error->GetBufferPointer()) : nullptr);
        ThrowIfFailed(m_fallbackDevice->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&(*rootSig))));
    }
    else // DirectX Raytracing
    {
        ThrowIfFailed(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error), error ? static_cast<wchar_t*>(error->GetBufferPointer()) : nullptr);
        ThrowIfFailed(device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&(*rootSig))));
    }

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
        CD3DX12_DESCRIPTOR_RANGE ranges[1]; // Perfomance TIP: Order from most frequent to least frequent.
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 1 output texture
        
        CD3DX12_ROOT_PARAMETER rootParameters[GlobalRootSignature::Slot::Count];
        rootParameters[GlobalRootSignature::Slot::OutputView].InitAsDescriptorTable(1, &ranges[0]);
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

        // AABB geometry
        {
            namespace RootSignatureSlots = LocalRootSignature::AABB::Slot;
            CD3DX12_ROOT_PARAMETER rootParameters[RootSignatureSlots::Count];
            rootParameters[RootSignatureSlots::MaterialConstant].InitAsConstants(SizeOfInUint32(PrimitiveConstantBuffer), 1);
            rootParameters[RootSignatureSlots::GeometryIndex].InitAsConstants(SizeOfInUint32(PrimitiveInstanceConstantBuffer), 2);

            CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
            localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
            SerializeAndCreateRaytracingRootSignature(localRootSignatureDesc, &m_raytracingLocalRootSignature[LocalRootSignature::Type::AABB]);
        }
    }
}

// Create raytracing device and command list.
void D3D12RaytracingDynamicGeometry::CreateRaytracingInterfaces()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();

    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        CreateRaytracingFallbackDeviceFlags createDeviceFlags = m_forceComputeFallback ?
            CreateRaytracingFallbackDeviceFlags::ForceComputeFallback :
            CreateRaytracingFallbackDeviceFlags::None;
        ThrowIfFailed(D3D12CreateRaytracingFallbackDevice(device, createDeviceFlags, 0, IID_PPV_ARGS(&m_fallbackDevice)));
        m_fallbackDevice->QueryRaytracingCommandList(commandList, IID_PPV_ARGS(&m_fallbackCommandList));
    }
    else // DirectX Raytracing
    {
        ThrowIfFailed(device->QueryInterface(IID_PPV_ARGS(&m_dxrDevice)), L"Couldn't get DirectX Raytracing interface for the device.\n");
    }
}

// DXIL library
// This contains the shaders and their entrypoints for the state object.
// Since shaders are not considered a subobject, they need to be passed in via DXIL library subobjects.
void D3D12RaytracingDynamicGeometry::CreateDxilLibrarySubobject(CD3D12_STATE_OBJECT_DESC* raytracingPipeline)
{
    auto lib = raytracingPipeline->CreateSubobject<CD3D12_DXIL_LIBRARY_SUBOBJECT>();
    D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void *)g_pRaytracing, ARRAYSIZE(g_pRaytracing));
    lib->SetDXILLibrary(&libdxil);
    // Use default shader exports for a DXIL library/collection subobject ~ surface all shaders.
}

// Hit groups
// A hit group specifies closest hit, any hit and intersection shaders 
// to be executed when a ray intersects the geometry.
void D3D12RaytracingDynamicGeometry::CreateHitGroupSubobjects(CD3D12_STATE_OBJECT_DESC* raytracingPipeline)
{
    // Triangle geometry hit groups
    {
        for (UINT rayType = 0; rayType < RayType::Count; rayType++)
        {
            auto hitGroup = raytracingPipeline->CreateSubobject<CD3D12_HIT_GROUP_SUBOBJECT>();
            if (rayType == RayType::Radiance)
            {
                hitGroup->SetClosestHitShaderImport(c_closestHitShaderNames[0]);
            }
            hitGroup->SetHitGroupExport(c_hitGroupNames_TriangleGeometry[rayType]);
            hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
        }
    }
}

// Local root signature and shader association
// This is a root signature that enables a shader to have unique arguments that come from shader tables.
void D3D12RaytracingDynamicGeometry::CreateLocalRootSignatureSubobjects(CD3D12_STATE_OBJECT_DESC* raytracingPipeline)
{
    // Ray gen and miss shaders in this sample are not using a local root signature and thus one is not associated with them.

    // Hit groups
    // Triangle geometry
    {
        auto localRootSignature = raytracingPipeline->CreateSubobject<CD3D12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
        localRootSignature->SetRootSignature(m_raytracingLocalRootSignature[LocalRootSignature::Type::Triangle].Get());
        // Shader association
        auto rootSignatureAssociation = raytracingPipeline->CreateSubobject<CD3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
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
    CD3D12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

    // DXIL library
    CreateDxilLibrarySubobject(&raytracingPipeline);

    // Hit groups
    CreateHitGroupSubobjects(&raytracingPipeline);

    // Shader config
    // Defines the maximum sizes in bytes for the ray rayPayload and attribute structure.
    auto shaderConfig = raytracingPipeline.CreateSubobject<CD3D12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    UINT payloadSize = max(sizeof(RayPayload), sizeof(ShadowRayPayload));
    UINT attributeSize = sizeof(struct ProceduralPrimitiveAttributes);
    shaderConfig->Config(payloadSize, attributeSize);

    // Local root signature and shader association
    // This is a root signature that enables a shader to have unique arguments that come from shader tables.
    CreateLocalRootSignatureSubobjects(&raytracingPipeline);

    // Global root signature
    // This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
    auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3D12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
    globalRootSignature->SetRootSignature(m_raytracingGlobalRootSignature.Get());

    // Pipeline config
    // Defines the maximum TraceRay() recursion depth.
    auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3D12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
    // PERFOMANCE TIP: Set max recursion depth as low as needed
    // as drivers may apply optimization strategies for low recursion depths.
    UINT maxRecursionDepth = MAX_RAY_RECURSION_DEPTH;
    pipelineConfig->Config(maxRecursionDepth);

    PrintStateObjectDesc(raytracingPipeline);

    // Create the state object.
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        ThrowIfFailed(m_fallbackDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_fallbackStateObject)), L"Couldn't create DirectX Raytracing state object.\n");
    }
    else // DirectX Raytracing
    {
        ThrowIfFailed(m_dxrDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_dxrStateObject)), L"Couldn't create DirectX Raytracing state object.\n");
    }
}

// Create a 2D output texture for raytracing.
void D3D12RaytracingDynamicGeometry::CreateRaytracingOutputResource()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto backbufferFormat = m_deviceResources->GetBackBufferFormat();

    // Create the output resource. The dimensions and format should match the swap-chain.
    auto uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(backbufferFormat, m_width, m_height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(device->CreateCommittedResource(
        &defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_raytracingOutput)));
    NAME_D3D12_OBJECT(m_raytracingOutput);

    D3D12_CPU_DESCRIPTOR_HANDLE uavDescriptorHandle;
    m_raytracingOutputResourceUAVDescriptorHeapIndex = AllocateDescriptor(&uavDescriptorHandle, m_raytracingOutputResourceUAVDescriptorHeapIndex);
    D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
    UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    device->CreateUnorderedAccessView(m_raytracingOutput.Get(), nullptr, &UAVDesc, uavDescriptorHandle);
    m_raytracingOutputResourceUAVGpuDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), m_raytracingOutputResourceUAVDescriptorHeapIndex, m_descriptorSize);
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
	descriptorHeapDesc.NumDescriptors = 2 * GeometryType::Count + 1 + 2 * MaxBLAS + 1 + ARRAYSIZE(SampleAssets::Draws) * 2;
    descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    descriptorHeapDesc.NodeMask = 0;
    device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_descriptorHeap));
    NAME_D3D12_OBJECT(m_descriptorHeap);

    m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

// Build AABBs for procedural geometry within a bottom-level acceleration structure.
void D3D12RaytracingDynamicGeometry::BuildDynamicGeometryAABBs()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Set up AABBs on a grid.
    {
        XMINT3 aabbGrid = XMINT3(4, 1, 4);
        const XMFLOAT3 basePosition =
        {
            -(aabbGrid.x * c_aabbWidth + (aabbGrid.x - 1) * c_aabbDistance) / 2.0f,
            -(aabbGrid.y * c_aabbWidth + (aabbGrid.y - 1) * c_aabbDistance) / 2.0f,
            -(aabbGrid.z * c_aabbWidth + (aabbGrid.z - 1) * c_aabbDistance) / 2.0f,
        };

        XMFLOAT3 stride = XMFLOAT3(c_aabbWidth + c_aabbDistance, c_aabbWidth + c_aabbDistance, c_aabbWidth + c_aabbDistance);
        auto InitializeAABB = [&](auto& offsetIndex, auto& size)
        {
            return D3D12_RAYTRACING_AABB { 
                basePosition.x + offsetIndex.x * stride.x, 
                basePosition.y + offsetIndex.y * stride.y,
                basePosition.z + offsetIndex.z * stride.z,
                basePosition.x + offsetIndex.x * stride.x + size.x,
                basePosition.y + offsetIndex.y * stride.y + size.y,
                basePosition.z + offsetIndex.z * stride.z + size.z,
            };
        };
        m_aabbs.resize(IntersectionShaderType::TotalPrimitiveCount);
        UINT offset = 0;

        // Analytic primitives.
        {
            using namespace AnalyticPrimitive;
            m_aabbs[offset + AABB] = InitializeAABB(XMINT3(3, 0, 0), XMFLOAT3(2, 3, 2));
            m_aabbs[offset + Spheres] = InitializeAABB(XMFLOAT3(2.25f, 0, 0.75f), XMFLOAT3(3, 3, 3));
            offset += AnalyticPrimitive::Count;
        }

        // Volumetric primitives.
        {
            using namespace VolumetricPrimitive;
            m_aabbs[offset + Metaballs] = InitializeAABB(XMINT3(0, 0, 0), XMFLOAT3(3, 3, 3));
            offset += VolumetricPrimitive::Count;
        }

        // Signed distance primitives.
        {
            using namespace SignedDistancePrimitive;
            m_aabbs[offset + MiniSpheres] = InitializeAABB(XMINT3(2, 0, 0), XMFLOAT3(2, 2, 2));
            m_aabbs[offset + TwistedTorus] = InitializeAABB(XMINT3(0, 0, 1), XMFLOAT3(2, 2, 2));
            m_aabbs[offset + IntersectedRoundCube] = InitializeAABB(XMINT3(0, 0, 2), XMFLOAT3(2, 2, 2));
            m_aabbs[offset + SquareTorus] = InitializeAABB(XMFLOAT3(0.75f, -0.1f, 2.25f), XMFLOAT3(3, 3, 3));
            m_aabbs[offset + Cog] = InitializeAABB(XMINT3(1, 0, 0), XMFLOAT3(2, 2, 2));
            m_aabbs[offset + Cylinder] = InitializeAABB(XMINT3(0, 0, 3), XMFLOAT3(2, 3, 2));
            m_aabbs[offset + FractalPyramid] = InitializeAABB(XMINT3(2, 0, 2), XMFLOAT3(6, 6, 6));
        }
        AllocateUploadBuffer(device, m_aabbs.data(), m_aabbs.size()*sizeof(m_aabbs[0]), &m_aabbBuffer.resource);
    }
}

void D3D12RaytracingDynamicGeometry::BuildPlaneGeometry()
{
    auto device = m_deviceResources->GetD3DDevice();
    // Plane indices.
    Index indices[] =
    {
        3,1,0,
        2,1,3

    };

    // Cube vertices positions and corresponding triangle normals.
#if 1
    DirectX::VertexPositionNormalTexture vertices[] =
    {
        { XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) }
    };
#else
    GeometricPrimitive::VertexType vertices[] =
    {
        { XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 0.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) }
    };
#endif
    AllocateUploadBuffer(device, indices, sizeof(indices), &m_geometries[GeometryType::Plane].ib.resource);
    AllocateUploadBuffer(device, vertices, sizeof(vertices), &m_geometries[GeometryType::Plane].vb.resource);

    // Vertex buffer is passed to the shader along with index buffer as a descriptor range.

    // ToDo revise numElements calculation
    CreateBufferSRV(&m_geometries[GeometryType::Plane].ib, static_cast<UINT>(ceil((float)sizeof(indices) / sizeof(UINT))), 0, &m_geometryIBHeapIndices[GeometryType::Plane]);
    CreateBufferSRV(&m_geometries[GeometryType::Plane].vb, ARRAYSIZE(vertices), sizeof(vertices[0]), &m_geometryVBHeapIndices[GeometryType::Plane]);
    ThrowIfFalse(m_geometryVBHeapIndices[GeometryType::Plane] == m_geometryIBHeapIndices[GeometryType::Plane] + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index");
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
    CreateBufferSRV(&geometry.ib, static_cast<UINT>(ceil(static_cast<float>(indices.size() * sizeof(Index)) / sizeof(UINT))) , 0, &m_geometryIBHeapIndices[GeometryType::Sphere]);
    CreateBufferSRV(&geometry.vb, static_cast<UINT>(vertices.size()), sizeof(vertices[0]), &m_geometryVBHeapIndices[GeometryType::Sphere]);
    ThrowIfFalse(m_geometryVBHeapIndices[GeometryType::Sphere] == m_geometryIBHeapIndices[GeometryType::Sphere] + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index");

    // ToDo
    m_numTrianglesPerGeometry = static_cast<UINT>(indices.size()) / 3;
}


void D3D12RaytracingDynamicGeometry::LoadSceneGeometry()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto commandList = m_deviceResources->GetCommandList();

	// Load scene assets.
	UINT fileSize = 0;
	UINT8* pAssetData;
	ThrowIfFailed(ReadDataFromFile(GetAssetFullPath(SampleAssets::DataFileName).c_str(), &pAssetData, &fileSize));

	// Create the vertex buffer.
	{
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(SampleAssets::VertexDataSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer)));

		NAME_D3D12_OBJECT(m_vertexBuffer);

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(SampleAssets::VertexDataSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertexBufferUpload)));

		// Copy data to the upload heap and then schedule a copy 
		// from the upload heap to the vertex buffer.
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = pAssetData + SampleAssets::VertexDataOffset;
		vertexData.RowPitch = SampleAssets::VertexDataSize;
		vertexData.SlicePitch = vertexData.RowPitch;

#if CULL_SQUID_CONTAINER_SIDE_PANELS
		const UINT sidePanelsGeometryID = 848;
		auto& geometryDesc = SampleAssets::Draws[sidePanelsGeometryID];
		SquidVertex* geometryVertices = reinterpret_cast<SquidVertex*>(pAssetData + SampleAssets::VertexDataOffset) + geometryDesc.VertexBase;
		UINT* geometryIndices = reinterpret_cast<UINT*>(pAssetData + SampleAssets::IndexDataOffset) + geometryDesc.IndexStart;

		// Deactivate vertices by setting x-coordinate to NaN.
		for (UINT i = 0; i < geometryDesc.IndexCount; i++)
		{
			auto& vertex = geometryVertices[geometryIndices[i]];
			vertex.position.x = nanf("");
		}
#endif

		PIXBeginEvent(commandList, 0, L"Copy vertex buffer data to default resource...");

		UpdateSubresources<1>(commandList, m_vertexBuffer.Get(), m_vertexBufferUpload.Get(), 0, 0, 1, &vertexData);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		PIXEndEvent(commandList);

		// Initialize the vertex buffer view.
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.SizeInBytes = SampleAssets::VertexDataSize;
		m_vertexBufferView.StrideInBytes = SampleAssets::StandardVertexStride;
	}

	// Create the index buffer.
	{
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(SampleAssets::IndexDataSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_indexBuffer)));

		NAME_D3D12_OBJECT(m_indexBuffer);

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(SampleAssets::IndexDataSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_indexBufferUpload)));

		// Copy data to the upload heap and then schedule a copy 
		// from the upload heap to the index buffer.
		D3D12_SUBRESOURCE_DATA indexData = {};
		indexData.pData = pAssetData + SampleAssets::IndexDataOffset;
		indexData.RowPitch = SampleAssets::IndexDataSize;
		indexData.SlicePitch = indexData.RowPitch;

		PIXBeginEvent(commandList, 0, L"Copy index buffer data to default resource...");

		UpdateSubresources<1>(commandList, m_indexBuffer.Get(), m_indexBufferUpload.Get(), 0, 0, 1, &indexData);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

		PIXEndEvent(commandList);

		// Initialize the index buffer view.
		m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
		m_indexBufferView.SizeInBytes = SampleAssets::IndexDataSize;
		m_indexBufferView.Format = SampleAssets::StandardIndexFormat;
	}

	auto& geometry = m_geometries[GeometryType::SquidScene];
	geometry.vb.resource = m_vertexBuffer;
	geometry.ib.resource = m_indexBuffer;
	
	// ToDo revise numElements calculation
	CreateBufferSRV(&m_geometries[GeometryType::SquidScene].ib, m_indexBufferView.SizeInBytes / sizeof(UINT), 0, &m_geometryIBHeapIndices[GeometryType::SquidScene]);
	CreateBufferSRV(&m_geometries[GeometryType::SquidScene].vb, m_vertexBufferView.SizeInBytes / m_vertexBufferView.StrideInBytes, m_vertexBufferView.StrideInBytes, &m_geometryVBHeapIndices[GeometryType::SquidScene]);
	ThrowIfFalse(m_geometryVBHeapIndices[GeometryType::SquidScene] == m_geometryIBHeapIndices[GeometryType::SquidScene] + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index");

	// ToDo remove
	// Backup primary handles
	struct Handles {
		D3D12_CPU_DESCRIPTOR_HANDLE cpu;
		D3D12_GPU_DESCRIPTOR_HANDLE gpu;
	};
	Handles ib = { m_geometries[GeometryType::SquidScene].ib.cpuDescriptorHandle, m_geometries[GeometryType::SquidScene].ib.gpuDescriptorHandle };
	Handles vb = { m_geometries[GeometryType::SquidScene].vb.cpuDescriptorHandle, m_geometries[GeometryType::SquidScene].vb.gpuDescriptorHandle };

	m_numTrianglesPerGeometry = 0;
	m_geometryInstances.resize(ARRAYSIZE(SampleAssets::Draws));
	for (UINT i = 0; i < ARRAYSIZE(SampleAssets::Draws); i++)
	{
		m_geometryInstances[i].ib.startIndex = SampleAssets::Draws[i].IndexStart;
		m_geometryInstances[i].ib.count = SampleAssets::Draws[i].IndexCount;
		
		m_geometryInstances[i].vb.startIndex = SampleAssets::Draws[i].VertexBase;
		m_geometryInstances[i].vb.count = SampleAssets::VertexDataSize / SampleAssets::StandardVertexStride - SampleAssets::Draws[i].VertexBase;

		UINT geometryIBHeapIndex = UINT_MAX;
		UINT geometryVBHeapIndex = UINT_MAX;
		CreateBufferSRV(&m_geometries[GeometryType::SquidScene].ib, m_geometryInstances[i].ib.count, sizeof(UINT), &geometryIBHeapIndex, m_geometryInstances[i].ib.startIndex);
		CreateBufferSRV(&m_geometries[GeometryType::SquidScene].vb, m_geometryInstances[i].vb.count, m_vertexBufferView.StrideInBytes, &geometryVBHeapIndex, m_geometryInstances[i].vb.startIndex);

		m_geometryInstances[i].ib.gpuDescriptorHandle = m_geometries[GeometryType::SquidScene].ib.gpuDescriptorHandle;
		m_geometryInstances[i].vb.gpuDescriptorHandle = m_geometries[GeometryType::SquidScene].vb.gpuDescriptorHandle;
	
		m_numTrianglesPerGeometry += SampleAssets::Draws[i].IndexCount / 3;
	}

	// Revert
	m_geometries[GeometryType::SquidScene].ib.cpuDescriptorHandle = ib.cpu;
	m_geometries[GeometryType::SquidScene].ib.gpuDescriptorHandle = ib.gpu;
	m_geometries[GeometryType::SquidScene].vb.cpuDescriptorHandle = vb.cpu;
	m_geometries[GeometryType::SquidScene].vb.gpuDescriptorHandle = vb.gpu;

	free(pAssetData);
}

// Build geometry used in the sample.
void D3D12RaytracingDynamicGeometry::InitializeGeometry()
{
    m_geometries.resize(GeometryType::Count);
    BuildTesselatedGeometry();
    BuildPlaneGeometry();   

	// Begin frame.
	m_deviceResources->ResetCommandAllocatorAndCommandlist();
	LoadSceneGeometry();
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
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
    {
        switch (SceneArgs::ASBuildFlag)
        {
        case SceneArgs::FastBuild:
            buildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
            break;
        case SceneArgs::FastTrace:
            buildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
            break;
        default: break;
        };

        if (SceneArgs::ASAllowUpdate) buildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
        if (SceneArgs::ASMinimizeMemory) buildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
    }

    // Initialize bottom-level AS.
    UINT64 maxScratchResourceSize = 0;
    m_ASmemoryFootprint = 0;
    {
#if ONLY_SQUID_SCENE_BLAS
		m_vBottomLevelAS.resize(1);
		m_vBottomLevelAS[0].Initialize(device, m_geometries[GeometryType::SquidScene], m_geometryInstances.size(), buildFlags, DXGI_FORMAT_R32_UINT, sizeof(UINT), SampleAssets::StandardVertexStride, m_geometryInstances);
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

			m_vBottomLevelAS[i].Initialize(device, m_geometries[i], numInstances, buildFlags, DXGI_FORMAT_R16_UINT, sizeof(Index), sizeof(DirectX::GeometricPrimitive::VertexType));
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

    void* rayGenShaderID;
    void* missShaderIDs[RayType::Count];
    void* hitGroupShaderIDs_TriangleGeometry[RayType::Count];

    // A shader name look-up table for shader table debug print out.
    unordered_map<void*, wstring> shaderIdToStringMap;

    auto GetShaderIDs = [&](auto* stateObjectProperties)
    {
        rayGenShaderID = stateObjectProperties->GetShaderIdentifier(c_raygenShaderName);
        shaderIdToStringMap[rayGenShaderID] = c_raygenShaderName;

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
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        GetShaderIDs(m_fallbackStateObject.Get());
        shaderIDSize = m_fallbackDevice->GetShaderIdentifierSize();
    }
    else // DirectX Raytracing
    {
        ComPtr<ID3D12StateObjectPropertiesPrototype> stateObjectProperties;
        ThrowIfFailed(m_dxrStateObject.As(&stateObjectProperties));
        GetShaderIDs(stateObjectProperties.Get());
        shaderIDSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    }

    /*************--------- Shader table layout -------*******************
    | --------------------------------------------------------------------
    | Shader table - HitGroupShaderTable: 
    | [0] : MyHitGroup_Triangle
    | [1] : MyHitGroup_Triangle_ShadowRay
    | [2] : MyHitGroup_AABB_AnalyticPrimitive
    | [3] : MyHitGroup_AABB_AnalyticPrimitive_ShadowRay 
    | ...
    | [6] : MyHitGroup_AABB_VolumetricPrimitive
    | [7] : MyHitGroup_AABB_VolumetricPrimitive_ShadowRay
    | [8] : MyHitGroup_AABB_SignedDistancePrimitive 
    | [9] : MyHitGroup_AABB_SignedDistancePrimitive_ShadowRay,
    | ...
    | [20] : MyHitGroup_AABB_SignedDistancePrimitive
    | [21] : MyHitGroup_AABB_SignedDistancePrimitive_ShadowRay
    | --------------------------------------------------------------------
    **********************************************************************/

     // RayGen shader table.
    {
        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIDSize; // No root arguments
        
        ShaderTable rayGenShaderTable(device, numShaderRecords, shaderRecordSize, L"RayGenShaderTable" );
        rayGenShaderTable.push_back(ShaderRecord(rayGenShaderID, shaderRecordSize, nullptr, 0));
        rayGenShaderTable.DebugPrint(shaderIdToStringMap);
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
#if ONLY_SQUID_SCENE_BLAS
		UINT numShaderRecords = m_geometryInstances.size() * RayType::Count;
#else
		UINT numShaderRecords = RayType::Count + IntersectionShaderType::TotalPrimitiveCount * RayType::Count;
#endif
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

void D3D12RaytracingDynamicGeometry::SelectRaytracingAPI(RaytracingAPI type)
{
    if (type == RaytracingAPI::FallbackLayer)
    {
        m_raytracingAPI = type;
    }
    else // DirectX Raytracing
    {
        if (m_isDxrSupported)
        {
            m_raytracingAPI = type;
        }
        else
        {
            OutputDebugString(L"Invalid selection - DXR is not available.\n");
        }
    }
}

// ToDo move to UILayer
void D3D12RaytracingDynamicGeometry::ModifyActiveUIParameter(bool bIncreaseValue)
{
/*
    switch (m_activeUIparameter)
    {
    case UIParameters::RaytracingAPI:
    {
        m_raytracingAPIparameter += bIncreaseValue ? 1 : -1;
        m_raytracingAPIparameter = Clamp(m_raytracingAPIparameter, 0, 2);
    }
    break;

    case UIParameters::BuildQuality:
    {
        m_raytracingAPIparameter += bIncreaseValue ? 1 : -1;
        m_raytracingAPIparameter = Clamp(m_raytracingAPIparameter, 0, 1);
    }
    break;

    case UIParameters::UpdateAlgorithm:
    {
    }
    break;

    case UIParameters::TesselationQuality:
    {
    }
    break;

    case UIParameters::NumberOfObjects:
    {
    }
    break;
    default:
    }

    case '1': // Fallback Layer
        m_forceComputeFallback = false;
        SelectRaytracingAPI(RaytracingAPI::FallbackLayer);
        break;
    case VK_NUMPAD2:
    case '2': // Fallback Layer + force compute path
        m_forceComputeFallback = true;
        SelectRaytracingAPI(RaytracingAPI::FallbackLayer);
        break;
    case VK_NUMPAD3:
    case '3': // DirectX Raytracing
        SelectRaytracingAPI(RaytracingAPI::DirectXRaytracing);
        break;
    default:
        break;
        

if (m_raytracingAPI != previousRaytracingAPI ||
    m_forceComputeFallback != previousForceComputeFallback)
{
    // Raytracing API selection changed, recreate everything.
    RecreateD3D();
}
*/
}

void D3D12RaytracingDynamicGeometry::OnKeyDown(UINT8 key)
{
#if 1
    switch (key)
    {
    case VK_NUMPAD1:
    case '1': // Fallback Layer
        m_forceComputeFallback = false;
        SelectRaytracingAPI(RaytracingAPI::FallbackLayer);
        break;
    case VK_NUMPAD2:
    case '2': // Fallback Layer + force compute path
        m_forceComputeFallback = true;
        SelectRaytracingAPI(RaytracingAPI::FallbackLayer);
        break;
    case VK_NUMPAD3:
    case '3': // DirectX Raytracing
        SelectRaytracingAPI(RaytracingAPI::DirectXRaytracing);
        break;
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
#else
    // Store previous values.
    RaytracingAPI previousRaytracingAPI = m_raytracingAPI;
    bool previousForceComputeFallback = m_forceComputeFallback;
    
    switch (key)
    {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
        m_activeUIparameter = key - '1';
        break;
    case VK_NUMPAD1:
    case VK_NUMPAD2:
    case VK_NUMPAD3:
    case VK_NUMPAD4:
    case VK_NUMPAD5:
        m_activeUIparameter = key - VK_NUMPAD1;
        break;
    case VK_UP:
        ModifyActiveUIParameter(true);
        break;
    case VK_DOWN:
        ModifyActiveUIParameter(false);
        break;
    case 'L':
        m_animateLight = !m_animateLight;
        break;
    case 'C':
        m_animateCamera = !m_animateCamera;
        break;
    default:
        break;
    }
#endif
}

// Update frame-based values.
void D3D12RaytracingDynamicGeometry::OnUpdate()
{
    m_timer.Tick();

    float elapsedTime = static_cast<float>(m_timer.GetElapsedSeconds());
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();
    auto prevFrameIndex = m_deviceResources->GetPreviousFrameIndex();

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
		UpdateCameraMatrices();
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
        UpdateCameraMatrices();
    }

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

    // Lazy initialize and update geometries and acceleration structures.
    if (SceneArgs::EnableGeometryAndASBuildsAndUpdates &&
        (m_isGeometryInitializationRequested || m_isASinitializationRequested))
    {
        // Since we'll be recreating D3D resources, GPU needs to be done with the current ones.
        m_deviceResources->WaitForGpu();

        m_deviceResources->ResetCommandAllocatorAndCommandlist();
#if ENABLE_RAYTRACING
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
#endif
        m_deviceResources->ExecuteCommandList();
    }
#if ENABLE_RAYTRACING
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

	for (int i = 1; i < argc; ++i)
	{
		if (_wcsnicmp(argv[i], L"-FL", wcslen(argv[i])) == 0)
		{
			m_forceComputeFallback = true;
			m_raytracingAPI = RaytracingAPI::FallbackLayer;
		}
		else if (_wcsnicmp(argv[i], L"-DXR", wcslen(argv[i])) == 0)
		{
			m_raytracingAPI = RaytracingAPI::DirectXRaytracing;
		}
	}
}

void D3D12RaytracingDynamicGeometry::UpdateAccelerationStructures(bool forceBuild)
{
    auto commandList = m_deviceResources->GetCommandList();
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();
    bool isTopLevelASUpdateNeeded = false;
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

    {
        m_gpuTimers[GpuTimers::UpdateBLAS].Start(commandList);
        for (UINT i = 0; i < m_vBottomLevelAS.size(); i++)
        {
            auto& bottomLevelAS = m_vBottomLevelAS[i];

            // ToDo - there should be two dirty flags 
            // - one for geometry transform/VB changes 
            // - and one for BLAS transform change
            // For now, update everything every frame
            //if (bottomLevelAS.IsDirty() || forceBuild)
            {
                // ToDo Heuristic to do an update based on transform amplitude
                D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGpuAddress = 0;                
                if (i > 0)
                {
                    // ToDo
                    baseGeometryTransformGpuAddress = m_geometryTransforms.GpuVirtualAddress(frameIndex) + (i - GeometryType::Sphere) * SceneArgs::NumGeometriesPerBLAS;
                }
                bottomLevelAS.Build(commandList, m_accelerationStructureScratch.Get(), m_descriptorHeap.Get(), baseGeometryTransformGpuAddress, bUpdate);
                isTopLevelASUpdateNeeded = true;
            }
        }
        m_gpuTimers[GpuTimers::UpdateBLAS].Stop(commandList);
    }

    if (isTopLevelASUpdateNeeded)
    {
        m_gpuTimers[GpuTimers::UpdateTLAS].Start(commandList);
        m_topLevelAS.Build(commandList, m_accelerationStructureScratch.Get(), m_descriptorHeap.Get(), bUpdate);
        m_gpuTimers[GpuTimers::UpdateTLAS].Stop(commandList);
    }
    if (!bUpdate)
    {
        m_numFramesSinceASBuild = 0;
    }
}

void D3D12RaytracingDynamicGeometry::DoRaytracing()
{
    auto commandList = m_deviceResources->GetCommandList();
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    auto DispatchRays = [&](auto* raytracingCommandList, auto* stateObject, auto* dispatchDesc)
    {
        dispatchDesc->HitGroupTable.StartAddress = m_hitGroupShaderTable->GetGPUVirtualAddress();
        dispatchDesc->HitGroupTable.SizeInBytes = m_hitGroupShaderTable->GetDesc().Width;
        dispatchDesc->HitGroupTable.StrideInBytes = m_hitGroupShaderTableStrideInBytes;
        dispatchDesc->MissShaderTable.StartAddress = m_missShaderTable->GetGPUVirtualAddress();
        dispatchDesc->MissShaderTable.SizeInBytes = m_missShaderTable->GetDesc().Width;
        dispatchDesc->MissShaderTable.StrideInBytes = m_missShaderTableStrideInBytes;
        dispatchDesc->RayGenerationShaderRecord.StartAddress = m_rayGenShaderTable->GetGPUVirtualAddress();
        dispatchDesc->RayGenerationShaderRecord.SizeInBytes = m_rayGenShaderTable->GetDesc().Width;
        dispatchDesc->Width = m_width;
        dispatchDesc->Height = m_height;
        raytracingCommandList->SetPipelineState1(stateObject);

        m_gpuTimers[GpuTimers::Raytracing].Start(commandList);
        raytracingCommandList->DispatchRays(dispatchDesc);
        m_gpuTimers[GpuTimers::Raytracing].Stop(commandList);
    };

    auto SetCommonPipelineState = [&](auto* descriptorSetCommandList)
    {
        descriptorSetCommandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());
        // Set index and successive vertex buffer decriptor tables.

        commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::OutputView, m_raytracingOutputResourceUAVGpuDescriptor);
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
    D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        SetCommonPipelineState(m_fallbackCommandList.Get());
        m_fallbackCommandList->SetTopLevelAccelerationStructure(GlobalRootSignature::Slot::AccelerationStructure, m_topLevelAS.GetFallbackAccelerationStructurePointer());
        DispatchRays(m_fallbackCommandList.Get(), m_fallbackStateObject.Get(), &dispatchDesc);
    }
    else // DirectX Raytracing
    {
        SetCommonPipelineState(commandList);
        commandList->SetComputeRootShaderResourceView(GlobalRootSignature::Slot::AccelerationStructure, m_topLevelAS.GetResource()->GetGPUVirtualAddress());
        DispatchRays(commandList, m_dxrStateObject.Get(), &dispatchDesc);
    }
}

// Update the application state with the new resolution.
void D3D12RaytracingDynamicGeometry::UpdateForSizeChange(UINT width, UINT height)
{
    DXSample::UpdateForSizeChange(width, height);
}

// Copy the raytracing output to the backbuffer.
void D3D12RaytracingDynamicGeometry::CopyRaytracingOutputToBackbuffer(D3D12_RESOURCE_STATES outRenderTargetState)
{
    auto commandList = m_deviceResources->GetCommandList();
    auto renderTarget = m_deviceResources->GetRenderTarget();

    D3D12_RESOURCE_BARRIER preCopyBarriers[2];
    preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
    preCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_raytracingOutput.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
    commandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);

    commandList->CopyResource(renderTarget, m_raytracingOutput.Get());

    D3D12_RESOURCE_BARRIER postCopyBarriers[2];
    postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_COPY_DEST, outRenderTargetState);
    postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_raytracingOutput.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    commandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
}

void D3D12RaytracingDynamicGeometry::UpdateUI()
{
    vector<wstring> labels;
#if 1
    // Main runtime information.
    {
        wstringstream wLabel;
        wLabel.precision(1);
        wLabel << L" GPU[" << m_deviceResources->GetAdapterID() << L"]: " 
               << m_deviceResources->GetAdapterDescription() << L"\n";
        wLabel << fixed << L" FPS: " << m_fps << L"\n";
        wLabel << fixed << L" DispatchRays: " << m_gpuTimers[GpuTimers::Raytracing].GetElapsedMS()
               << L"ms" << L"     ~Million Primary Rays/s: " << NumMRaysPerSecond()
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
        wLabel << L" " << L" # triangles per geometry: " << m_numTrianglesPerGeometry << L"\n";
        wLabel << L" " << L" # geometries per BLAS: " << SceneArgs::NumGeometriesPerBLAS << L"\n";
        wLabel << L" " << L" # Sphere BLAS: " << SceneArgs::NumSphereBLAS << L"\n";
        wLabel << L" " << L" # total triangles: " << SceneArgs::NumSphereBLAS * SceneArgs::NumGeometriesPerBLAS* m_numTrianglesPerGeometry << L"\n";
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
    
    UpdateCameraMatrices();
    
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
    m_raytracingOutput.Reset();
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

    m_fallbackDevice.Reset();
    m_fallbackCommandList.Reset();
    m_fallbackStateObject.Reset();
    m_raytracingGlobalRootSignature.Reset();
    ResetComPtrArray(&m_raytracingLocalRootSignature);

    m_dxrDevice.Reset();
    m_dxrStateObject.Reset();

    m_raytracingGlobalRootSignature.Reset();
    ResetComPtrArray(&m_raytracingLocalRootSignature);

    m_descriptorHeap.Reset();
    m_descriptorsAllocated = 0;
    m_sceneCB.Release();

    m_computeCB.Release();

    // ToDo
    for (auto& bottomLevelAS : m_vBottomLevelAS)
    {
        bottomLevelAS.ReleaseD3DResources();
    }
    m_topLevelAS.ReleaseD3DResources();

    m_raytracingOutput.Reset();
    m_raytracingOutputResourceUAVDescriptorHeapIndex = UINT_MAX;
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

    m_computeAllocators[frameIndex]->Reset();
    m_computeCommandList->Reset(m_computeAllocators[frameIndex].Get(), m_computePSO.Get());

    commandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());
    commandList->SetComputeRootSignature(m_csSamleVisualizerRootSignature.Get());

    XMUINT2 rngWindowSize(256, 256);
    m_computeCB->dispatchDimensions = rngWindowSize;

    static UINT seed = 0;
	UINT NumFramesPerIter = 100;
	static UINT frameID = NumFramesPerIter * 4;
    m_computeCB->numSamplesToShow = (frameID++ / NumFramesPerIter) % m_randomSampler.NumSamples();
    m_computeCB->seed =  ((seed++ / (NumFramesPerIter * m_randomSampler.NumSamples())) % m_randomSampler.NumSampleSets()) * m_randomSampler.NumSamples();
    m_computeCB->stratums = XMUINT2(static_cast<UINT>(sqrt(m_randomSampler.NumSamples())), 
                                    static_cast<UINT>(sqrt(m_randomSampler.NumSamples())));
    m_computeCB->grid = XMUINT2(m_randomSampler.NumSamples(), m_randomSampler.NumSamples());
    m_computeCB->uavOffset = XMUINT2(m_width - rngWindowSize.x, m_height - rngWindowSize.y);
    m_computeCB->numSamples = m_randomSampler.NumSamples();
    m_computeCB->numSampleSets = m_randomSampler.NumSampleSets();

    // Copy dynamic buffers to GPU
    {
        m_computeCB.CopyStagingToGpu(frameIndex);
        m_samplesGPUBuffer.CopyStagingToGpu(frameIndex);
    }
    commandList->SetComputeRootConstantBufferView(RNGVisualizerRootSignature::Slot::SceneConstant, m_computeCB.GpuVirtualAddress(frameIndex));
    commandList->SetComputeRootShaderResourceView(RNGVisualizerRootSignature::Slot::SampleBuffers, m_samplesGPUBuffer.GpuVirtualAddress(frameIndex));
    commandList->SetComputeRootDescriptorTable(RNGVisualizerRootSignature::Slot::OutputView, m_raytracingOutputResourceUAVGpuDescriptor);

    commandList->SetPipelineState(m_computePSO.Get());
 
    commandList->Dispatch(rngWindowSize.x, rngWindowSize.y, 1);

    // close and execute the command list
    m_computeCommandList->Close();
    ID3D12CommandList *tempList = m_computeCommandList.Get();
    m_computeCommandQueue->ExecuteCommandLists(1, &tempList);

    if (m_computeCommandQueue && m_fence && m_fenceEvent.IsValid())
    {
        // Schedule a Signal command in the GPU queue.
        UINT64 fenceValue = m_fenceValues[frameIndex];
        if (SUCCEEDED(m_computeCommandQueue->Signal(m_fence.Get(), fenceValue)))
        {
            // Wait until the Signal has been processed.
            if (SUCCEEDED(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent.Get())))
            {
                WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, FALSE);

                // Increment the fence value for the current frame.
                m_fenceValues[frameIndex]++;
            }
        }
    }
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
#if !DEBUG_UI_DEVICE_HUNG
    for (auto& gpuTimer : m_gpuTimers)
    {
        gpuTimer.BeginFrame(commandList);
    }

#if ENABLE_RAYTRACING
    // Update acceleration structures.
    if (m_isASrebuildRequested && SceneArgs::EnableGeometryAndASBuildsAndUpdates)
    {
        UpdateAccelerationStructures(m_isASrebuildRequested);
        m_isASrebuildRequested = false;
    }
    // Render.
	DoRaytracing();
#endif

    RenderRNGVisualizations();

	// UILayer will transition backbuffer to a present state.
    CopyRaytracingOutputToBackbuffer(m_enableUI ? D3D12_RESOURCE_STATE_RENDER_TARGET : D3D12_RESOURCE_STATE_PRESENT);
    
    // End frame.
    for (auto& gpuTimer : m_gpuTimers)
    {
        gpuTimer.EndFrame(commandList);
    }
#endif
    m_deviceResources->ExecuteCommandList();

    // UI overlay.
    if (m_enableUI)
    {
		m_deviceResources->WaitForGpu();
        m_uiLayer->Render(m_deviceResources->GetCurrentFrameIndex());
    }
    
    m_deviceResources->Present(D3D12_RESOURCE_STATE_PRESENT);
}

void D3D12RaytracingDynamicGeometry::OnDestroy()
{
    // Let GPU finish before releasing D3D resources.
    m_deviceResources->WaitForGpu();
    OnDeviceLost();
}

// Release all device dependent resouces when a device is lost.
void D3D12RaytracingDynamicGeometry::OnDeviceLost()
{
    ReleaseWindowSizeDependentResources();
    ReleaseDeviceDependentResources();
}

// Create all device dependent resources when a device is restored.
void D3D12RaytracingDynamicGeometry::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

float D3D12RaytracingDynamicGeometry::NumMRaysPerSecond()
{
    float resolution = static_cast<float>(m_width * m_height);
    float raytracingTime = 0.001f * static_cast<float>(m_gpuTimers[GpuTimers::Raytracing].GetElapsedMS());
    return resolution / ( raytracingTime * static_cast<float>(1e6));
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
            if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
            {
#if ENABLE_RAYTRACING
                if (m_fallbackDevice->UsingRaytracingDriver())
                {
                    windowText << L"(FL-DXR)";
                }
                else
                {
                    windowText << L"(FL)";
                }
#endif
            }
            else
            {
                windowText << L"(DXR)";
            }
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

// Create a wrapped pointer for the Fallback Layer path.
WRAPPED_GPU_POINTER D3D12RaytracingDynamicGeometry::CreateFallbackWrappedPointer(ID3D12Resource* resource, UINT bufferNumElements, UINT* descriptorHeapIndex)
{
    auto device = m_deviceResources->GetD3DDevice();

    D3D12_UNORDERED_ACCESS_VIEW_DESC rawBufferUavDesc = {};
    rawBufferUavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    rawBufferUavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
    rawBufferUavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    rawBufferUavDesc.Buffer.NumElements = bufferNumElements;

    D3D12_CPU_DESCRIPTOR_HANDLE bottomLevelDescriptor;

    // Only compute fallback requires a valid descriptor index when creating a wrapped pointer.
    if (!m_fallbackDevice->UsingRaytracingDriver())
    {
        *descriptorHeapIndex = AllocateDescriptor(&bottomLevelDescriptor, *descriptorHeapIndex);
        device->CreateUnorderedAccessView(resource, nullptr, &rawBufferUavDesc, bottomLevelDescriptor);
    }
    return m_fallbackDevice->GetWrappedPointerSimple(*descriptorHeapIndex, resource->GetGPUVirtualAddress());
}

// Allocate a descriptor and return its index. 
// If the passed descriptorIndexToUse is valid, it will be used instead of allocating a new one.
UINT D3D12RaytracingDynamicGeometry::AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse)
{
    auto descriptorHeapCpuBase = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    if (descriptorIndexToUse >= m_descriptorHeap->GetDesc().NumDescriptors)
    {
        ThrowIfFalse(m_descriptorsAllocated < m_descriptorHeap->GetDesc().NumDescriptors, L"Ran out of descriptors on the heap!" );
        descriptorIndexToUse = m_descriptorsAllocated++;
    }
    *cpuDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeapCpuBase, descriptorIndexToUse, m_descriptorSize);
    return descriptorIndexToUse;
}

// Create a SRV for a buffer.
void D3D12RaytracingDynamicGeometry::CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize, UINT* descriptorHeapIndex, UINT firstElement)
{
    auto device = m_deviceResources->GetD3DDevice();

    // SRV
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.NumElements = numElements;
	srvDesc.Buffer.FirstElement = firstElement;
    if (elementSize == 0)
    {
        srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
        srvDesc.Buffer.StructureByteStride = 0;
    }
    else
    {
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
        srvDesc.Buffer.StructureByteStride = elementSize;
    }
    *descriptorHeapIndex = AllocateDescriptor(&buffer->cpuDescriptorHandle, *descriptorHeapIndex);
    device->CreateShaderResourceView(buffer->resource.Get(), &srvDesc, buffer->cpuDescriptorHandle);
    buffer->gpuDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), *descriptorHeapIndex, m_descriptorSize);
};
