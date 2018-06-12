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

using namespace std;
using namespace DX;
using namespace DirectX;

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
    L"MyClosestHitShader_Triangle",
    L"MyClosestHitShader_AABB",
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
const wchar_t* D3D12RaytracingDynamicGeometry::c_hitGroupNames_AABBGeometry[][RayType::Count] = 
{
    { L"MyHitGroup_AABB_AnalyticPrimitive", L"MyHitGroup_AABB_AnalyticPrimitive_ShadowRay" },
    { L"MyHitGroup_AABB_VolumetricPrimitive", L"MyHitGroup_AABB_VolumetricPrimitive_ShadowRay" },
    { L"MyHitGroup_AABB_SignedDistancePrimitive", L"MyHitGroup_AABB_SignedDistancePrimitive_ShadowRay" },
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
	EnumVar RaytracingMode(L"RaytracingMode", FLDXR, _countof(RaytracingModes), RaytracingModes);

	BoolVar EnableGeometryAndASRebuilds(L"Enable geometry & AS rebuilds", true);

	enum UpdateMode { Build = 0, Update, Mix };
	const WCHAR* UpdateModes[] = { L"Build only", L"Update only", L"Update + rebuild every X frames" };
	EnumVar ASUpdateMode(L"Acceleration structure/Update mode", Build, _countof(UpdateModes), UpdateModes);
	IntVar ASUpdateFrequency(L"Acceleration structure/Rebuild frame frequency", 1, 1, 1200, 1);
	BoolVar ASMinimizeMemory(L"Acceleration structure/Minimize memory", false, OnASChange, nullptr);
	BoolVar ASAllowUpdate(L"Acceleration structure/Allow update", true);
	enum BuildFlag { Default = 0,FastTrace, FastBuild };
	const WCHAR* BuildFlags[] = { L"Default", L"Fast trace", L"Fast build" };
	EnumVar ASBuildFlag(L"Acceleration structure/Build quality", FastTrace, _countof(BuildFlags), BuildFlags, OnASChange, nullptr);

	IntVar GeometryTesselationFactor(L"Geometry/Tesselation factor", 2, 0, 80, 1, OnGeometryChange, nullptr);
	IntVar NumGeometriesPerBLAS(L"Geometry/# geometries per BLAS", 1, 1, 1000, 1, OnGeometryChange, nullptr);
	IntVar NumBLAS(L"Geometry/# BLAS", 1, 1, D3D12RaytracingDynamicGeometry::MaxBLAS, 1, OnGeometryChange, nullptr);
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
	m_isASrebuildRequested(true)
{
	g_pSample = this;
    m_forceComputeFallback = false;
    SelectRaytracingAPI(RaytracingAPI::FallbackLayer);
    UpdateForSizeChange(width, height);
	m_bottomLevelASdescritorHeapIndices.resize(MaxBLAS, UINT_MAX);
	m_bottomLevelASinstanceDescsDescritorHeapIndices.resize(MaxBLAS, UINT_MAX);
	m_topLevelASdescritorHeapIndex = UINT_MAX;
	m_geometryIBHeapIndices.resize(MaxGeometries, UINT_MAX);
	m_geometryVBHeapIndices.resize(MaxGeometries, UINT_MAX);
}

void D3D12RaytracingDynamicGeometry::EnableDXRExperimentalFeatures(IDXGIAdapter1* adapter)
{
    // DXR is an experimental feature and needs to be enabled before creating a D3D12 device.
    m_isDxrSupported = EnableRaytracing(adapter);

    if (!m_isDxrSupported)
    {
        OutputDebugString(
            L"Could not enable raytracing driver (D3D12EnableExperimentalFeatures() failed).\n" \
            L"Possible reasons:\n" \
            L"  1) your OS is not in developer mode.\n" \
            L"  2) your GPU driver doesn't match the D3D12 runtime loaded by the app (d3d12.dll and friends).\n" \
            L"  3) your D3D12 runtime doesn't match the D3D12 headers used by your app (in particular, the GUID passed to D3D12EnableExperimentalFeatures).\n\n");

        OutputDebugString(L"Enabling compute based fallback raytracing support.\n");
        ThrowIfFalse(EnableComputeRaytracingFallback(adapter), L"Could not enable compute based fallback raytracing support (D3D12EnableExperimentalFeatures() failed).\n");
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
    EnableDXRExperimentalFeatures(m_deviceResources->GetAdapter());
    
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
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    m_sceneCB->cameraPosition = m_eye;
    float fovAngleY = 45.0f;
    XMMATRIX view = XMMatrixLookAtLH(m_eye, m_at, m_up);
    XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), m_aspectRatio, 0.01f, 125.0f);
    XMMATRIX viewProj = view * proj;
    m_sceneCB->projectionToWorld = XMMatrixInverse(nullptr, viewProj);
}

void D3D12RaytracingDynamicGeometry::UpdateGeometries()
{
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

	float animationDuration = 24.0f;
    float t = CalculateAnimationInterpolant(static_cast<float>(m_timer.GetTotalSeconds()), animationDuration);
	t += -0.5f;

	float amplitude = 12.0f;
	XMFLOAT4 translationVector = XMFLOAT4(0, 1, 0, 0);
	XMMATRIX transform = XMMatrixTranslationFromVector(t * amplitude * XMLoadFloat4(&translationVector));
	m_vBottomLevelAS[0].SetTransform(transform);
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


        m_planeMaterialCB = { XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f), 0.25f, 1, 0.4f, 50, 1};

        // Albedos
        XMFLOAT4 green = XMFLOAT4(0.1f, 1.0f, 0.5f, 1.0f);
        XMFLOAT4 red = XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f);
        XMFLOAT4 yellow = XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f);
        
        UINT offset = 0;
        // Analytic primitives.
        {
            using namespace AnalyticPrimitive;
            SetAttributes(offset + AABB, red);
            SetAttributes(offset + Spheres, ChromiumReflectance, 1);
            offset += AnalyticPrimitive::Count;
        }

        // Volumetric primitives.
        {
            using namespace VolumetricPrimitive;
            SetAttributes(offset + Metaballs, ChromiumReflectance, 1);
            offset += VolumetricPrimitive::Count;
        }

        // Signed distance primitives.
        {
            using namespace SignedDistancePrimitive;
            SetAttributes(offset + MiniSpheres, green);
            SetAttributes(offset + IntersectedRoundCube, green);
            SetAttributes(offset + SquareTorus, ChromiumReflectance, 1);
            SetAttributes(offset + TwistedTorus, yellow, 0, 1.0f, 0.7f, 50, 0.5f );
            SetAttributes(offset + Cog, yellow, 0, 1.0f, 0.1f, 2);
            SetAttributes(offset + Cylinder, red);
            SetAttributes(offset + FractalPyramid, green, 0, 1, 0.1f, 4, 0.8f);
        }
    }

    // Setup camera.
    {
        // Initialize the view and projection inverse matrices.
        m_eye = { 0.0f, 20.3f, -65.0f, 1.0f }; 
        m_at = { 0.0f, 0.0f, 0.0f, 1.0f };
        XMVECTOR right = { 1.0f, 0.0f, 0.0f, 0.0f };

        XMVECTOR direction = XMVector4Normalize(m_at - m_eye);
        m_up = XMVector3Normalize(XMVector3Cross(direction, right));

        // Rotate camera around Y axis.
        XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(45.0f));
        m_eye = XMVector3Transform(m_eye, rotate);
        m_up = XMVector3Transform(m_up, rotate);

        UpdateCameraMatrices();
    }

    // Setup lights.
    {
        // Initialize the lighting parameters.
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

// Create AABB primitive attributes buffers.
void D3D12RaytracingDynamicGeometry::CreateAABBPrimitiveAttributesBuffers()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto frameCount = m_deviceResources->GetBackBufferCount();
    m_aabbPrimitiveAttributeBuffer.Create(device, IntersectionShaderType::TotalPrimitiveCount, frameCount, L"AABB primitive attributes");
}

// Create resources that depend on the device.
void D3D12RaytracingDynamicGeometry::CreateDeviceDependentResources()
{
	CreateAuxilaryDeviceResources();

    // Initialize raytracing pipeline.

    // Create raytracing interfaces: raytracing device and commandlist.
    CreateRaytracingInterfaces();

    // Create root signatures for the shaders.
    CreateRootSignatures();

    // Create a raytracing pipeline state object which defines the binding of shaders, state and resources to be used during raytracing.
    CreateRaytracingPipelineStateObject();

    // Create a heap for descriptors.
    CreateDescriptorHeap();

    // Build geometry to be used in the sample.
	m_isGeometryInitializationRequested = true;
	
    // Build raytracing acceleration structures from the generated geometry.
	m_isASinitializationRequested = true;
	
    // Create constant buffers for the geometry and the scene.
    CreateConstantBuffers();

    // Create AABB primitive attribute buffers.
    CreateAABBPrimitiveAttributesBuffers();

    // Build shader tables, which define shaders and their local root arguments.
    BuildShaderTables();

    // Create an output 2D texture to store the raytracing result to.
    CreateRaytracingOutputResource();
}

void D3D12RaytracingDynamicGeometry::SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig)
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
}

void D3D12RaytracingDynamicGeometry::CreateRootSignatures()
{
	auto device = m_deviceResources->GetD3DDevice();

	// Global Root Signature
	// This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
	{
		CD3DX12_DESCRIPTOR_RANGE ranges[2]; // Perfomance TIP: Order from most frequent to least frequent.
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 1 output texture
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1);  // 2 static index and vertex buffers.

		CD3DX12_ROOT_PARAMETER rootParameters[GlobalRootSignature::Slot::Count];
		rootParameters[GlobalRootSignature::Slot::OutputView].InitAsDescriptorTable(1, &ranges[0]);
		rootParameters[GlobalRootSignature::Slot::AccelerationStructure].InitAsShaderResourceView(0);
		rootParameters[GlobalRootSignature::Slot::SceneConstant].InitAsConstantBufferView(0);
		rootParameters[GlobalRootSignature::Slot::AABBattributeBuffer].InitAsShaderResourceView(3);
		rootParameters[GlobalRootSignature::Slot::VertexBuffers].InitAsDescriptorTable(1, &ranges[1]);
		CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
		SerializeAndCreateRaytracingRootSignature(globalRootSignatureDesc, &m_raytracingGlobalRootSignature);
	}

	// Local Root Signature
	// This is a root signature that enables a shader to have unique arguments that come from shader tables.
	{
#if USE_NON_NULL_LOCAL_ROOT_SIG 
		// Empty root signature
		{
			CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(D3D12_DEFAULT);
			localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
			SerializeAndCreateRaytracingRootSignature(localRootSignatureDesc, &m_raytracingLocalRootSignature[LocalRootSignature::Type::Empty]);
		}
#endif
		// Triangle geometry
		{
			namespace RootSignatureSlots = LocalRootSignature::Triangle::Slot;
			CD3DX12_ROOT_PARAMETER rootParameters[RootSignatureSlots::Count];
			rootParameters[RootSignatureSlots::MaterialConstant].InitAsConstants(SizeOfInUint32(PrimitiveConstantBuffer), 1);

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
		ThrowIfFailed(commandList->QueryInterface(IID_PPV_ARGS(&m_dxrCommandList)), L"Couldn't get DirectX Raytracing interface for the command list.\n");
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
#if DEFINE_EXPLICIT_SHADER_EXPORTS
    // Define which shader exports to surface from the library.
    // If no shader exports are defined for a DXIL library subobject, all shaders will be surfaced.
    // In this sample, this could be ommited for convenience since the sample uses all shaders in the library. 
    {
        lib->DefineExport(c_raygenShaderName);
        DefineExports(lib, c_intersectionShaderNames);
        DefineExports(lib, c_closestHitShaderNames);
        DefineExports(lib, c_missShaderNames);
    }
#else
    // Use default shader exports for a DXIL library/collection subobject ~ surface all shaders.
#endif
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
                hitGroup->SetClosestHitShaderImport(c_closestHitShaderNames[GeometryType::Triangle]);
            }
            hitGroup->SetHitGroupExport(c_hitGroupNames_TriangleGeometry[rayType]);
        }
    }

    // AABB geometry hit groups
    {
        // Create hit groups for each intersection shader.
        for (UINT t = 0; t < IntersectionShaderType::Count; t++)
            for (UINT rayType = 0; rayType < RayType::Count; rayType++)
            {
                auto hitGroup = raytracingPipeline->CreateSubobject<CD3D12_HIT_GROUP_SUBOBJECT>();
                hitGroup->SetIntersectionShaderImport(c_intersectionShaderNames[t]);
                if (rayType == RayType::Radiance)
                {
                    hitGroup->SetClosestHitShaderImport(c_closestHitShaderNames[GeometryType::AABB]);
                }
                hitGroup->SetHitGroupExport(c_hitGroupNames_AABBGeometry[t][rayType]);
            }
    }
}

// Local root signature and shader association
// This is a root signature that enables a shader to have unique arguments that come from shader tables.
void D3D12RaytracingDynamicGeometry::CreateLocalRootSignatureSubobjects(CD3D12_STATE_OBJECT_DESC* raytracingPipeline)
{
#if USE_NON_NULL_LOCAL_ROOT_SIG 
    // Empty
    {
        auto localRootSignature = raytracingPipeline->CreateSubobject<CD3D12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
        localRootSignature->SetRootSignature(m_raytracingLocalRootSignature[LocalRootSignature::Type::Empty].Get());
        // Shader association
        auto rootSignatureAssociation = raytracingPipeline->CreateSubobject<CD3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
        rootSignatureAssociation->AddExport(c_raygenShaderName);
        rootSignatureAssociation->AddExports(c_missShaderNames);
    }
#endif
    // Triangle geometry
    {
        auto localRootSignature = raytracingPipeline->CreateSubobject<CD3D12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
        localRootSignature->SetRootSignature(m_raytracingLocalRootSignature[LocalRootSignature::Type::Triangle].Get());
        // Shader association
        auto rootSignatureAssociation = raytracingPipeline->CreateSubobject<CD3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
        rootSignatureAssociation->AddExports(c_hitGroupNames_TriangleGeometry);
    }

    // AABB geometry
    {
        auto localRootSignature = raytracingPipeline->CreateSubobject<CD3D12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
        localRootSignature->SetRootSignature(m_raytracingLocalRootSignature[LocalRootSignature::Type::AABB].Get());
        // Shader association
        auto rootSignatureAssociation = raytracingPipeline->CreateSubobject<CD3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
        for (auto& hitGroupsForIntersectionShaderType : c_hitGroupNames_AABBGeometry)
        {
            rootSignatureAssociation->AddExports(hitGroupsForIntersectionShaderType);
        }
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
    auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3D12_ROOT_SIGNATURE_SUBOBJECT>();
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
		gpuTimer.RestoreDevice(device, commandQueue);
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
	descriptorHeapDesc.NumDescriptors = 2 * MaxGeometries + 1 + 2 * MaxBLAS + 1;
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
#if 0
	auto device = m_deviceResources->GetD3DDevice();
    // Plane indices.
    Index indices[] =
    {
        3,1,0,
        2,1,3,

    };

    // Cube vertices positions and corresponding triangle normals.
    Vertex vertices[] =
    {
        { XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
    };

    AllocateUploadBuffer(device, indices, sizeof(indices), &m_indexBuffer.resource);
    AllocateUploadBuffer(device, vertices, sizeof(vertices), &m_vertexBuffer.resource);

    // Vertex buffer is passed to the shader along with index buffer as a descriptor range.
    UINT descriptorIndexIB = CreateBufferSRV(&m_indexBuffer, sizeof(indices) / 4, 0);
    UINT descriptorIndexVB = CreateBufferSRV(&m_vertexBuffer, ARRAYSIZE(vertices), sizeof(vertices[0]));
    ThrowIfFalse(descriptorIndexVB == descriptorIndexIB + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index");
#endif
}

void D3D12RaytracingDynamicGeometry::BuildTesselatedGeometry()
{
    auto device = m_deviceResources->GetD3DDevice();

    vector<GeometricPrimitive::VertexType> vertices;
    vector<Index> indices;

    const float GeometryRange = 10.f;
    const bool RhCoords = false;

	// ToDo option to reuse geometry
	m_geometries.resize(1);
	auto& geometry = m_geometries[0];
	const float radius = 3.0f;
	switch (SceneArgs::GeometryTesselationFactor)
	{
	case 0:
		// 24 indices
		GeometricPrimitive::CreateOctahedron(vertices, indices, radius, RhCoords);
		break;
	case 1:
		// 36 indices
		GeometricPrimitive::CreateDodecahedron(vertices, indices, radius, RhCoords);
		break;
	case 2:
		// 60 indices
		GeometricPrimitive::CreateIcosahedron(vertices, indices, radius, RhCoords);
		break;
	default:
		// Tesselation Factor - # Indices:
		// o 3  - 126
		// o 4  - 216
		// o 5  - 330
		// o 10 - 1260
		// o 16 - 3681
		// o 20 - 4920
		const float Diameter = 2 * radius;
		GeometricPrimitive::CreateSphere(vertices, indices, Diameter, SceneArgs::GeometryTesselationFactor, RhCoords);
	}
	AllocateUploadBuffer(device, indices.data(), indices.size() * sizeof(indices[0]), &geometry.ib.resource);
    AllocateUploadBuffer(device, vertices.data(), vertices.size() * sizeof(vertices[0]), &geometry.vb.resource);

	// Vertex buffer is passed to the shader along with index buffer as a descriptor range.
	CreateBufferSRV(&geometry.ib, static_cast<UINT>(indices.size()) / sizeof(UINT) * sizeof(Index), 0, &m_geometryIBHeapIndices[0]);
	CreateBufferSRV(&geometry.vb, static_cast<UINT>(vertices.size()), sizeof(vertices[0]), &m_geometryVBHeapIndices[0]);
	ThrowIfFalse(m_geometryVBHeapIndices[0] == m_geometryIBHeapIndices[0] + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index");

	m_numTrianglesPerGeometry = static_cast<UINT>(indices.size()) / 3;

	// Generate geometry desc transforms;
	int dim = static_cast<int>(ceil(cbrt(static_cast<double>(SceneArgs::NumGeometriesPerBLAS))));
	float distanceBetweenGeometry = radius;
	float stepDistance = 2 * radius + distanceBetweenGeometry;
	m_geometryTransforms.Create(device, SceneArgs::NumGeometriesPerBLAS, 1, L"Geometry build desc transforms");


	for (int iY = 0, i = 0; iY < dim; iY++)
		for (int iX = 0; iX < dim; iX++)
			for (int iZ = 0; iZ < dim; iZ++, i++)
			{
				if (i >= static_cast<UINT>(SceneArgs::NumGeometriesPerBLAS))
				{
					break;
				}

				XMFLOAT4 translationVector = XMFLOAT4(
					static_cast<float>(iX - dim/2), 
					static_cast<float>(iY - dim/2), 
					static_cast<float>(iZ - dim/2), 
					0.0f);
				XMMATRIX transform = XMMatrixTranslationFromVector(stepDistance * XMLoadFloat4(&translationVector));
				StoreXMMatrixAsTransform3x4(m_geometryTransforms[i].transform3x4, transform);
			}
	m_geometryTransforms.CopyStagingToGpu();
}

// Build geometry used in the sample.
void D3D12RaytracingDynamicGeometry::InitializeGeometry()
{
	BuildTesselatedGeometry();
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
	{
		m_vBottomLevelAS.resize(SceneArgs::NumBLAS);
		m_vBottomLevelAS[0].Initialize(device, m_geometries[0], SceneArgs::NumGeometriesPerBLAS, m_geometryTransforms.GpuVirtualAddress(), buildFlags);
		maxScratchResourceSize = max(m_vBottomLevelAS[0].RequiredScratchSize(), maxScratchResourceSize);
	}

	// Initialize top-level AS.
	{
		m_topLevelAS.Initialize(device, m_vBottomLevelAS, buildFlags, &m_bottomLevelASinstanceDescsDescritorHeapIndices);
		maxScratchResourceSize = max(m_topLevelAS.RequiredScratchSize(), maxScratchResourceSize);
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
    void* hitGroupShaderIDs_AABBGeometry[IntersectionShaderType::Count][RayType::Count];

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
        for (UINT r = 0; r < IntersectionShaderType::Count; r++)
            for (UINT c = 0; c < RayType::Count; c++)        
            {
                hitGroupShaderIDs_AABBGeometry[r][c] = stateObjectProperties->GetShaderIdentifier(c_hitGroupNames_AABBGeometry[r][c]); 
                shaderIdToStringMap[hitGroupShaderIDs_AABBGeometry[r][c]] = c_hitGroupNames_AABBGeometry[r][c];
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
        shaderIDSize = m_dxrDevice->GetShaderIdentifierSize();
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
        UINT numShaderRecords = RayType::Count + IntersectionShaderType::TotalPrimitiveCount * RayType::Count;
        UINT shaderRecordSize = shaderIDSize + LocalRootSignature::MaxRootArgumentsSize();
        ShaderTable hitGroupShaderTable(device, numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");

        // Triangle geometry hit groups.
        {
            LocalRootSignature::Triangle::RootArguments rootArgs;
            rootArgs.materialCb = m_planeMaterialCB;

            for (auto& hitGroupShaderID : hitGroupShaderIDs_TriangleGeometry)
            {
                hitGroupShaderTable.push_back(ShaderRecord(hitGroupShaderID, shaderIDSize, &rootArgs, sizeof(rootArgs)));
            }
        }
      
        // AABB geometry hit groups.
        {
            LocalRootSignature::AABB::RootArguments rootArgs;
            UINT instanceIndex = 0;

            // Create a shader record for each primitive.
            for (UINT iShader = 0, instanceIndex = 0; iShader < IntersectionShaderType::Count; iShader++)
            {
                UINT numPrimitiveTypes = IntersectionShaderType::PerPrimitiveTypeCount(static_cast<IntersectionShaderType::Enum>(iShader));
                
                // Primitives for each intersection shader.
                for (UINT primitiveIndex = 0; primitiveIndex < numPrimitiveTypes; primitiveIndex++, instanceIndex++)
                {
                    rootArgs.materialCb = m_aabbMaterialCB[instanceIndex];
                    rootArgs.aabbCB.instanceIndex = instanceIndex;
                    rootArgs.aabbCB.primitiveType = primitiveIndex;
                    
                    // Ray types.
                    for (UINT r = 0; r < RayType::Count; r++)
                    {
                        auto& hitGroupShaderID = hitGroupShaderIDs_AABBGeometry[iShader][r];
                        hitGroupShaderTable.push_back(ShaderRecord(hitGroupShaderID, shaderIDSize, &rootArgs, sizeof(rootArgs)));
                    }
                }
            }
        }
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

    // Rotate the camera around Y axis.
    if (m_animateCamera)
    {
        float secondsToRotateAround = 48.0f;
        float angleToRotateBy = 360.0f * (elapsedTime / secondsToRotateAround);
        XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(angleToRotateBy));
        m_eye = XMVector3Transform(m_eye, rotate);
        m_up = XMVector3Transform(m_up, rotate);
        m_at = XMVector3Transform(m_at, rotate);
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
	if (SceneArgs::EnableGeometryAndASRebuilds &&
		(m_isGeometryInitializationRequested || m_isASinitializationRequested))
	{
		// Since we'll be recreating D3D resources, GPU needs to be done with the current ones.
		m_deviceResources->WaitForGpu();

		m_deviceResources->ResetCommandAllocatorAndCommandlist();
		if (m_isGeometryInitializationRequested)
		{
			InitializeGeometry();
			m_isGeometryInitializationRequested = false;
		}
		if (m_isASinitializationRequested)
		{
			InitializeAccelerationStructures();
			m_isASinitializationRequested = false;
		}
		m_deviceResources->ExecuteCommandList();
		m_deviceResources->WaitForGpu();
	}

	if (m_animateScene)
	{
		UpdateGeometries();
	}


	if (m_enableUI)
	{
		UpdateUI();
	}
}


// Parse supplied command line args.
void D3D12RaytracingDynamicGeometry::ParseCommandLineArgs(WCHAR* argv[], int argc)
{
    DXSample::ParseCommandLineArgs(argv, argc);

    if (argc > 1)
    {
        if (_wcsnicmp(argv[1], L"-FL", wcslen(argv[1])) == 0)
        {
            m_forceComputeFallback = true;
			m_raytracingAPI = RaytracingAPI::FallbackLayer;
        }
        else if (_wcsnicmp(argv[1], L"-DXR", wcslen(argv[1])) == 0)
        {
			m_raytracingAPI = RaytracingAPI::DirectXRaytracing;
        }
    }
}

void D3D12RaytracingDynamicGeometry::UpdateAccelerationStructures(bool forceBuild)
{
	auto commandList = m_deviceResources->GetCommandList();
	bool isTopLevelASUpdateNeeded = false;

	// ToDo move this next to TLAS build? But BLAS update resets dirty flag
	m_topLevelAS.UpdateInstanceDescTransforms(m_vBottomLevelAS);

	{
		m_gpuTimers[GpuTimers::UpdateBLAS].Start(commandList);
		for (auto& bottomLevelAS : m_vBottomLevelAS)
		{
			if (bottomLevelAS.IsDirty() || forceBuild)
			{
				// ToDo Heuristic to do an update instead
				bottomLevelAS.Build(commandList, m_accelerationStructureScratch.Get(), m_descriptorHeap.Get());
				isTopLevelASUpdateNeeded = true;
			}
		}
		m_gpuTimers[GpuTimers::UpdateBLAS].Stop(commandList);
	}
	if (isTopLevelASUpdateNeeded)
	{
		m_gpuTimers[GpuTimers::UpdateTLAS].Start(commandList);
		m_topLevelAS.Build(commandList, m_accelerationStructureScratch.Get(), m_descriptorHeap.Get());
		m_gpuTimers[GpuTimers::UpdateTLAS].Stop(commandList);
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

		m_gpuTimers[GpuTimers::Raytracing].Start(commandList);
		raytracingCommandList->DispatchRays(stateObject, dispatchDesc);
		m_gpuTimers[GpuTimers::Raytracing].Stop(commandList);
    };

    auto SetCommonPipelineState = [&](auto* descriptorSetCommandList)
    {
        descriptorSetCommandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());
        // Set index and successive vertex buffer decriptor tables.
#if RENDER_SPHERES
        commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::VertexBuffers, m_geometries[0].ib.gpuDescriptorHandle);
#else
		commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::VertexBuffers, m_indexBuffer.gpuDescriptorHandle);
#endif
		commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::OutputView, m_raytracingOutputResourceUAVGpuDescriptor);
    };

    commandList->SetComputeRootSignature(m_raytracingGlobalRootSignature.Get());

    // Copy dynamic buffers to GPU.
    {
        m_sceneCB.CopyStagingToGpu(frameIndex);
        commandList->SetComputeRootConstantBufferView(GlobalRootSignature::Slot::SceneConstant, m_sceneCB.GpuVirtualAddress(frameIndex));

        m_aabbPrimitiveAttributeBuffer.CopyStagingToGpu(frameIndex);
        commandList->SetComputeRootShaderResourceView(GlobalRootSignature::Slot::AABBattributeBuffer, m_aabbPrimitiveAttributeBuffer.GpuVirtualAddress(frameIndex));
    }

    // Bind the heaps, acceleration structure and dispatch rays.    
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        D3D12_FALLBACK_DISPATCH_RAYS_DESC dispatchDesc = {};
        SetCommonPipelineState(m_fallbackCommandList.Get());
        m_fallbackCommandList->SetTopLevelAccelerationStructure(GlobalRootSignature::Slot::AccelerationStructure, m_topLevelAS.GetFallbackAccelerationStructurePointer());
        DispatchRays(m_fallbackCommandList.Get(), m_fallbackStateObject.Get(), &dispatchDesc);
    }
    else // DirectX Raytracing
    {
        D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
        SetCommonPipelineState(commandList);
        commandList->SetComputeRootShaderResourceView(GlobalRootSignature::Slot::AccelerationStructure, m_topLevelAS.GetResource()->GetGPUVirtualAddress());
        DispatchRays(m_dxrCommandList.Get(), m_dxrStateObject.Get(), &dispatchDesc);
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
		wLabel << L" " << L" # triangles per geometry: " << m_numTrianglesPerGeometry << L"\n";
		wLabel << L" " << L" # geometries per BLAS: " << SceneArgs::NumGeometriesPerBLAS << L"\n";
		wLabel << L" " << L" # BLAS: " << SceneArgs::NumBLAS << L"\n";
		wLabel << L" " << L" # total triangles: " << SceneArgs::NumBLAS * SceneArgs::NumGeometriesPerBLAS* m_numTrianglesPerGeometry << L"\n";
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

    m_fallbackDevice.Reset();
    m_fallbackCommandList.Reset();
    m_fallbackStateObject.Reset();
    m_raytracingGlobalRootSignature.Reset();
    ResetComPtrArray(&m_raytracingLocalRootSignature);

    m_dxrDevice.Reset();
    m_dxrCommandList.Reset();
    m_dxrStateObject.Reset();

    m_raytracingGlobalRootSignature.Reset();
    ResetComPtrArray(&m_raytracingLocalRootSignature);

    m_descriptorHeap.Reset();
    m_descriptorsAllocated = 0;
    m_sceneCB.Release();
    m_aabbPrimitiveAttributeBuffer.Release();
    m_indexBuffer.resource.Reset();
    m_vertexBuffer.resource.Reset();
    m_aabbBuffer.resource.Reset();

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

	// Update acceleration structures.
	UpdateAccelerationStructures(m_isASrebuildRequested);
	m_isASrebuildRequested = false;
	
	// Render.
    DoRaytracing();
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
				if (m_fallbackDevice->UsingRaytracingDriver())
				{
					windowText << L"(FL-DXR)";
				}
				else
				{
					windowText << L"(FL)";
				}
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
void D3D12RaytracingDynamicGeometry::CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize, UINT* descriptorHeapIndex)
{
    auto device = m_deviceResources->GetD3DDevice();

    // SRV
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.NumElements = numElements;
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
