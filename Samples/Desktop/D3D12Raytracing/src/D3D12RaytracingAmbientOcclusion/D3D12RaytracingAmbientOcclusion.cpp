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
#include "D3D12RaytracingAmbientOcclusion.h"
#include "GameInput.h"
#include "EngineTuning.h"
#include "CompiledShaders\Raytracing.hlsl.h"
#include "CompiledShaders\RNGVisualizerCS.hlsl.h"
#include "SquidRoom.h"

using namespace std;
using namespace DX;
using namespace DirectX;
using namespace SceneEnums;
using namespace GameCore;

D3D12RaytracingAmbientOcclusion* g_pSample = nullptr;
HWND g_hWnd = 0;

// Shader entry points.
const wchar_t* D3D12RaytracingAmbientOcclusion::c_rayGenShaderNames[] = 
{
	// ToDo reorder
	L"MyRayGenShader_GBuffer", L"MyRayGenShader_PrimaryAndAO", L"MyRayGenShader_AO"
};
const wchar_t* D3D12RaytracingAmbientOcclusion::c_closestHitShaderNames[] =
{
    L"MyClosestHitShader", nullptr, L"MyClosestHitShader_GBuffer"
};
const wchar_t* D3D12RaytracingAmbientOcclusion::c_missShaderNames[] =
{
    L"MyMissShader", L"MyMissShader_ShadowRay", L"MyMissShader_GBuffer"
};
// Hit groups.
const wchar_t* D3D12RaytracingAmbientOcclusion::c_hitGroupNames_TriangleGeometry[] = 
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
    IntVar GeometryTesselationFactor(L"Geometry/Tesselation factor", 0/*14*/, 0, 80, 1, OnGeometryChange, nullptr);
    IntVar NumGeometriesPerBLAS(L"Geometry/# geometries per BLAS", // ToDo
#if ONLY_SQUID_SCENE_BLAS
		1,
#else
#if NUM_GEOMETRIES_1000
		1000, 
#elif NUM_GEOMETRIES_100000
		100000,
#elif NUM_GEOMETRIES_1000000
		1000000,
#endif
#endif
		1, 1000000, 1, OnGeometryChange, nullptr);
    IntVar NumSphereBLAS(L"Geometry/# Sphere BLAS", 1, 1, D3D12RaytracingAmbientOcclusion::MaxBLAS, 1, OnASChange, nullptr);
};


// ToDo move
void D3D12RaytracingAmbientOcclusion::LoadPBRTScene()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto commandList = m_deviceResources->GetCommandList();
	auto commandQueue = m_deviceResources->GetCommandQueue();
	auto commandAllocator = m_deviceResources->GetCommandAllocator();

	auto Vec3ToXMFLOAT3 = [](SceneParser::Vector3 v)
	{
		return XMFLOAT3(v.x, v.y, v.z);
	};


	auto Vec3ToXMVECTOR = [](SceneParser::Vector3 v)
	{
		return XMLoadFloat3(&XMFLOAT3(v.x, v.y, v.z));
	};

	auto Vec3ToXMFLOAT2 = [](SceneParser::Vector2 v)
	{
		return XMFLOAT2(v.x, v.y);
	};

	PBRTParser::PBRTParser().Parse("Assets\\dragon\\scene.pbrt", m_pbrtScene);

	m_camera.Set(
		Vec3ToXMVECTOR(m_pbrtScene.m_Camera.m_Position),
		Vec3ToXMVECTOR(m_pbrtScene.m_Camera.m_LookAt),
		Vec3ToXMVECTOR(m_pbrtScene.m_Camera.m_Up));
	m_camera.fov = m_pbrtScene.m_Camera.m_FieldOfView;

	UINT numGeometries = static_cast<UINT>(m_pbrtScene.m_Meshes.size());

	auto& geometries = m_geometries[GeometryType::PBRT];
	geometries.resize(numGeometries);

	auto& geometryInstances = m_geometryInstances[GeometryType::PBRT];
	geometryInstances.reserve(numGeometries);

	// ToDo
	m_numTriangles[GeometryType::PBRT] = 0;
	for (UINT i = 0; i < m_pbrtScene.m_Meshes.size(); i++)
	{
		auto &mesh = m_pbrtScene.m_Meshes[i];
		vector<VertexPositionNormalTextureTangent> vertexBuffer;
		vector<Index> indexBuffer;
		vertexBuffer.reserve(mesh.m_VertexBuffer.size());
		indexBuffer.reserve(mesh.m_IndexBuffer.size());

		GeometryDescriptor desc;
		desc.ib.count = static_cast<UINT>(mesh.m_IndexBuffer.size());
		desc.vb.count = static_cast<UINT>(mesh.m_VertexBuffer.size());

		for (auto &parseIndex : mesh.m_IndexBuffer)
		{
			Index index = parseIndex;
			indexBuffer.push_back(index);
		}
		desc.ib.indices = indexBuffer.data();
		
		for (auto &parserVertex : mesh.m_VertexBuffer)
		{
			VertexPositionNormalTextureTangent vertex;
			vertex.normal = Vec3ToXMFLOAT3(parserVertex.Normal);
			vertex.position = Vec3ToXMFLOAT3(parserVertex.Position);
			vertex.tangent = Vec3ToXMFLOAT3(parserVertex.Tangents);
			vertex.textureCoordinate = Vec3ToXMFLOAT2(parserVertex.UV);
			vertexBuffer.push_back(vertex);
		}
		desc.vb.vertices = vertexBuffer.data();

		auto IsTriangleClockwiseWinded = [&](UINT index0)
		{
			UINT indices[3] = { mesh.m_IndexBuffer[index0], mesh.m_IndexBuffer[index0 + 1], mesh.m_IndexBuffer[index0 + 2] };
			SceneParser::Vertex vertices[3] = { mesh.m_VertexBuffer[indices[0]], mesh.m_VertexBuffer[indices[1]], mesh.m_VertexBuffer[indices[2]] };
			XMVECTOR v01 = XMLoadFloat3(&vertexBuffer[indices[1]].position) - XMLoadFloat3(&vertexBuffer[indices[0]].position);
			XMVECTOR v02 = XMLoadFloat3(&vertexBuffer[indices[2]].position) - XMLoadFloat3(&vertexBuffer[indices[0]].position);
			XMVECTOR normal = XMLoadFloat3(&vertexBuffer[indices[0]].normal) +
				XMLoadFloat3(&vertexBuffer[indices[1]].normal) +
				XMLoadFloat3(&vertexBuffer[indices[2]].normal);
			return XMVectorGetX(XMVector3Dot(XMVector3Cross(v01, v02), normal)) > 0;
		};

#if 0 //ToDo scene is mirrored 
		for (UINT j = 0; j < vertexBuffer.size(); j += 1)
		{
			vertexBuffer[j].position.z = -vertexBuffer[j].position.z;
		}
#endif
		// Make sure the triangles are in LH clockwise order 
		if (!IsTriangleClockwiseWinded(0))
			for (UINT j = 0; j < indexBuffer.size(); j += 3)
			{
				swap(indexBuffer[j], indexBuffer[j + 2]);
			}
		auto& geometry = geometries[i];
		CreateGeometry(device, commandList, m_cbvSrvUavHeap.get(), desc, &geometry);
		ThrowIfFalse(geometry.vb.buffer.heapIndex == geometry.ib.buffer.heapIndex + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index");

		geometryInstances.push_back(GeometryInstance(geometry));

		m_numTriangles[GeometryType::PBRT] += desc.ib.count / 3;
	}
}


D3D12RaytracingAmbientOcclusion::D3D12RaytracingAmbientOcclusion(UINT width, UINT height, wstring name) :
    DXSample(width, height, name),
    m_animateCamera(false),
    m_animateLight(false),
    m_animateScene(true),
    m_missShaderTableStrideInBytes(UINT_MAX),
    m_hitGroupShaderTableStrideInBytes(UINT_MAX),
    m_isGeometryInitializationRequested(true),
    m_isASinitializationRequested(true),
    m_isASrebuildRequested(true),
    m_ASmemoryFootprint(0),
    m_numFramesSinceASBuild(0),
	m_isCameraFrozen(false)
{
    g_pSample = this;
    UpdateForSizeChange(width, height);
    m_bottomLevelASdescritorHeapIndices.resize(MaxBLAS, UINT_MAX);
    m_bottomLevelASinstanceDescsDescritorHeapIndices.resize(MaxBLAS, UINT_MAX);
    m_topLevelASdescritorHeapIndex = UINT_MAX;
	m_raytracingOutput.descriptorHeapIndex = UINT_MAX;
	for (auto& gbufferResource : m_GBufferResources)
	{
		gbufferResource.descriptorHeapIndex = UINT_MAX;
	}
	m_AORayHits.descriptorHeapIndex = UINT_MAX;
	m_generatorURNG.seed(1729);
}

// ToDo worth moving some common member vars and fncs to DxSampleRaytracing base class?
void D3D12RaytracingAmbientOcclusion::OnInit()
{
    m_deviceResources = make_unique<DeviceResources>(
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
#if ENABLE_RAYTRACING
	ThrowIfFalse(IsDirectXRaytracingSupported(m_deviceResources->GetAdapter()),
		L"ERROR: DirectX Raytracing is not supported by your GPU and driver.\n\n");
#endif
    // ToDo cleanup
    m_deviceResources->CreateDeviceResources();
	// Initialize scene ToDo
    InitializeScene();
    CreateDeviceDependentResources();
    m_deviceResources->CreateWindowSizeDependentResources();
}

D3D12RaytracingAmbientOcclusion::~D3D12RaytracingAmbientOcclusion()
{
    GameInput::Shutdown();
}

// Update camera matrices passed into the shader.
void D3D12RaytracingAmbientOcclusion::UpdateCameraMatrices()
{
    m_sceneCB->cameraPosition = m_camera.Eye();

	XMMATRIX view, proj;
	// ToDo camera is creating fisheye in spehere scene
	m_camera.GetProj(&proj, m_width, m_height);

	// Calculate view matrix as if the camera was at (0,0,0) to avoid 
	// precision issues when camera position is too far from (0,0,0).
	// GenerateCameraRay takes this into consideration in the raytracing shader.
	view = XMMatrixLookAtLH(XMVectorSet(0, 0, 0, 1), XMVectorSetW(m_camera.At() - m_camera.Eye(), 1), m_camera.Up());
	XMMATRIX viewProj = view * proj;
	m_sceneCB->projectionToWorldWithCameraEyeAtOrigin = XMMatrixInverse(nullptr, viewProj);
}

void D3D12RaytracingAmbientOcclusion::UpdateBottomLevelASTransforms()
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

void D3D12RaytracingAmbientOcclusion::UpdateSphereGeometryTransforms()
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

                for (UINT j = BottomLevelASType::Sphere; j < m_vBottomLevelAS.size(); j++)
                {
                    UINT transformIndex = j + 1;	// + plane which is first. ToDo break geometries apart.
        			XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(m_geometryTransforms[transformIndex].transform3x4), transform);
                }
            }
}

// ToDo move to CS.
void D3D12RaytracingAmbientOcclusion::UpdateGridGeometryTransforms()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

	// Generate geometry desc transforms;
#if TESSELATED_GEOMETRY_ASPECT_RATIO_DIMENSIONS
	int dimX =static_cast<int>(ceil(sqrt(SceneArgs::NumGeometriesPerBLAS * m_aspectRatio)));
#else
	int dimX = static_cast<int>(ceil(sqrt(static_cast<double>(SceneArgs::NumGeometriesPerBLAS))));
#endif
	XMUINT3 dim(dimX, 1, CeilDivide(SceneArgs::NumGeometriesPerBLAS, dimX));

	float spacing = 0.4f * max(m_boxSize.x, m_boxSize.z);
	XMVECTOR stepDistance = XMLoadFloat3(&m_boxSize) + XMVectorSet(spacing, spacing, spacing, 0);
	XMVECTOR offset = - XMLoadUInt3(&dim) / 2 * stepDistance;
	offset = XMVectorSetY(offset, m_boxSize.y / 2);

	// ToDo

	uniform_real_distribution<float> elevationDistribution(-0.4f*m_boxSize.y, 0);
	uniform_real_distribution<float> jitterDistribution(-spacing, spacing);
	uniform_real_distribution<float> rotationDistribution(-XM_PI, 180);

	for (UINT iY = 0, i = 0; iY < dim.y; iY++)
		for (UINT iX = 0; iX < dim.x; iX++)
			for (UINT iZ = 0; iZ < dim.z; iZ++, i++)
			{
				if (static_cast<int>(i) >= SceneArgs::NumGeometriesPerBLAS )
				{
					break;
				}
				const UINT X_TILE_WIDTH = 20;
				const UINT X_TILE_SPACING = X_TILE_WIDTH * 2;
				const UINT Z_TILE_WIDTH = 6;
				const UINT Z_TILE_SPACING = Z_TILE_WIDTH * 2;

				XMVECTOR translationVector = offset + stepDistance * 
					XMVectorSet(
#if TESSELATED_GEOMETRY_TILES
						static_cast<float>((iX / X_TILE_WIDTH) * X_TILE_SPACING + iX % X_TILE_WIDTH),
						static_cast<float>(iY),
						static_cast<float>((iZ/ Z_TILE_WIDTH) * Z_TILE_SPACING + iZ % Z_TILE_WIDTH),
#else
						static_cast<float>(iX),
						static_cast<float>(iY),
						static_cast<float>(iZ),
#endif
						0);
				// Break up Moire alias patterns by jittering the position.
				translationVector += XMVectorSet(
					jitterDistribution(m_generatorURNG),
					elevationDistribution(m_generatorURNG), 
					jitterDistribution(m_generatorURNG),
					0);
				XMMATRIX translation = XMMatrixTranslationFromVector(translationVector);
				XMMATRIX rotation = XMMatrixIdentity();// ToDo - need to rotate normals too XMMatrixRotationY(rotationDistribution(m_generatorURNG));
				XMMATRIX transform = rotation * translation;
				
				// ToDO remove - skip past plane transform
				XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(m_geometryTransforms[i + 1].transform3x4), transform);
			}

	// Update the plane transform.
	XMVECTOR size = XMVectorSetY(1.1f*XMLoadUInt3(&dim) * stepDistance, 1);
	XMMATRIX scale = XMMatrixScalingFromVector(size);
	XMMATRIX translation = XMMatrixTranslationFromVector(XMVectorSetY (- size / 2, 0));
	XMMATRIX transform = scale * translation;
	XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(m_geometryTransforms[0].transform3x4), transform);
}

// Initialize scene rendering parameters.
void D3D12RaytracingAmbientOcclusion::InitializeScene()
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
void D3D12RaytracingAmbientOcclusion::CreateConstantBuffers()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto frameCount = m_deviceResources->GetBackBufferCount();

    m_sceneCB.Create(device, frameCount, L"Scene Constant Buffer");
}

// ToDo rename, move
void D3D12RaytracingAmbientOcclusion::CreateSamplesRNG()
{
    auto device = m_deviceResources->GetD3DDevice(); 
    auto frameCount = m_deviceResources->GetBackBufferCount();

    m_randomSampler.Reset(c_sppAO, 83, Samplers::HemisphereDistribution::Cosine);

    // Create root signature.
    {
		using namespace ComputeShader::RootSignature;

        CD3DX12_DESCRIPTOR_RANGE ranges[1]; // Perfomance TIP: Order from most frequent to least frequent.
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 1 output texture

        CD3DX12_ROOT_PARAMETER rootParameters[HemisphereSampleSetVisualization::Slot::Count];
        rootParameters[HemisphereSampleSetVisualization::Slot::OutputView].InitAsDescriptorTable(1, &ranges[0]);

        rootParameters[HemisphereSampleSetVisualization::Slot::SampleBuffers].InitAsShaderResourceView(1);
        rootParameters[HemisphereSampleSetVisualization::Slot::SceneConstant].InitAsConstantBufferView(0);

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
		SerializeAndCreateRootSignature(device, rootSignatureDesc, &m_computeRootSigs[ComputeShader::Type::HemisphereSampleSetVisualization], L"Root signature: CS hemisphere sample set visualization");
    }

    // Create compute pipeline state.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC descComputePSO = {};
        descComputePSO.pRootSignature = m_computeRootSigs[ComputeShader::Type::HemisphereSampleSetVisualization].Get();
        descComputePSO.CS = CD3DX12_SHADER_BYTECODE((void *)g_pRNGVisualizerCS, ARRAYSIZE(g_pRNGVisualizerCS));

        ThrowIfFailed(device->CreateComputePipelineState(&descComputePSO, IID_PPV_ARGS(&m_computePSOs[ComputeShader::Type::HemisphereSampleSetVisualization])));
        m_computePSOs[ComputeShader::Type::HemisphereSampleSetVisualization]->SetName(L"PSO: CS hemisphere sample set visualization");
    }


    // Create shader resources
    {
        m_csHemisphereVisualizationCB.Create(device, frameCount, L"GPU CB: RNG");
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
void D3D12RaytracingAmbientOcclusion::CreateDeviceDependentResources()
{
	auto device = m_deviceResources->GetD3DDevice();

    CreateAuxilaryDeviceResources();

	// Create a heap for descriptors.
	CreateDescriptorHeaps();

    // Initialize raytracing pipeline.

    // Create raytracing interfaces: raytracing device and commandlist.
    CreateRaytracingInterfaces();

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

#if ENABLE_RAYTRACING
	// Build shader tables, which define shaders and their local root arguments.
    BuildShaderTables();
#endif
    // Create an output 2D texture to store the raytracing result to.
	// ToDo remove
    CreateRaytracingOutputResource();

	// ToDo move
    CreateSamplesRNG();

}

void D3D12RaytracingAmbientOcclusion::CreateRootSignatures()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Global Root Signature
    // This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
    {
        CD3DX12_DESCRIPTOR_RANGE ranges[4]; // Perfomance TIP: Order from most frequent to least frequent.
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 1 output textures
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 3, 5);  // 3 output textures
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 8);  // 3 input textures
		ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 8);  // 1 output textures


        CD3DX12_ROOT_PARAMETER rootParameters[GlobalRootSignature::Slot::Count];
        rootParameters[GlobalRootSignature::Slot::OutputView].InitAsDescriptorTable(1, &ranges[0]);
		rootParameters[GlobalRootSignature::Slot::GBufferResources].InitAsDescriptorTable(1, &ranges[1]);
		rootParameters[GlobalRootSignature::Slot::GBufferResourcesIn].InitAsDescriptorTable(1, &ranges[2]);
		rootParameters[GlobalRootSignature::Slot::AOResourcesOut].InitAsDescriptorTable(1, &ranges[3]);
        rootParameters[GlobalRootSignature::Slot::AccelerationStructure].InitAsShaderResourceView(0);
        rootParameters[GlobalRootSignature::Slot::SceneConstant].InitAsConstantBufferView(0);
        rootParameters[GlobalRootSignature::Slot::AABBattributeBuffer].InitAsShaderResourceView(3);
        rootParameters[GlobalRootSignature::Slot::SampleBuffers].InitAsShaderResourceView(4);
        
        CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
		SerializeAndCreateRootSignature(device, globalRootSignatureDesc, &m_raytracingGlobalRootSignature, L"Global root signature");
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
			SerializeAndCreateRootSignature(device, localRootSignatureDesc, &m_raytracingLocalRootSignature[LocalRootSignature::Type::Triangle], L"Local root signature: triangle geometry");
        }
    }
}

// Create raytracing device and command list.
void D3D12RaytracingAmbientOcclusion::CreateRaytracingInterfaces()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();

   ThrowIfFailed(device->QueryInterface(IID_PPV_ARGS(&m_dxrDevice)), L"Couldn't get DirectX Raytracing interface for the device.\n");
}

// DXIL library
// This contains the shaders and their entrypoints for the state object.
// Since shaders are not considered a subobject, they need to be passed in via DXIL library subobjects.
void D3D12RaytracingAmbientOcclusion::CreateDxilLibrarySubobject(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
{
    auto lib = raytracingPipeline->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void *)g_pRaytracing, ARRAYSIZE(g_pRaytracing));
    lib->SetDXILLibrary(&libdxil);
    // Use default shader exports for a DXIL library/collection subobject ~ surface all shaders.
}

// Hit groups
// A hit group specifies closest hit, any hit and intersection shaders 
// to be executed when a ray intersects the geometry.
void D3D12RaytracingAmbientOcclusion::CreateHitGroupSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
{
    // Triangle geometry hit groups
    {
        for (UINT rayType = 0; rayType < RayType::Count; rayType++)
        {
            auto hitGroup = raytracingPipeline->CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
            
			if (c_closestHitShaderNames[rayType])
			{
				hitGroup->SetClosestHitShaderImport(c_closestHitShaderNames[rayType]);
			}
            hitGroup->SetHitGroupExport(c_hitGroupNames_TriangleGeometry[rayType]);
            hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
        }
    }
}

// Local root signature and shader association
// This is a root signature that enables a shader to have unique arguments that come from shader tables.
void D3D12RaytracingAmbientOcclusion::CreateLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
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
void D3D12RaytracingAmbientOcclusion::CreateRaytracingPipelineStateObject()
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
void D3D12RaytracingAmbientOcclusion::CreateRaytracingOutputResource()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto backbufferFormat = m_deviceResources->GetBackBufferFormat();
	m_raytracingOutput.rwFlags = ResourceRWFlags::AllowWrite | ResourceRWFlags::AllowRead;
	CreateRenderTargetResource(device, backbufferFormat, m_width, m_height, m_cbvSrvUavHeap.get(), &m_raytracingOutput, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}


void D3D12RaytracingAmbientOcclusion::CreateGBufferResources()
{
	auto device = m_deviceResources->GetD3DDevice();

	// ToDo tune formats
	D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	for (auto& resource : m_GBufferResources)
	{
		resource.rwFlags = ResourceRWFlags::AllowWrite;
	}
	CreateRenderTargetResource(device, DXGI_FORMAT_R32_UINT, m_width, m_height, m_cbvSrvUavHeap.get(), &m_GBufferResources[GBufferResource::Hit], initialResourceState);
	CreateRenderTargetResource(device, DXGI_FORMAT_R32G32B32A32_FLOAT, m_width, m_height, m_cbvSrvUavHeap.get(), &m_GBufferResources[GBufferResource::HitPosition], initialResourceState);
	CreateRenderTargetResource(device, DXGI_FORMAT_R32G32B32A32_FLOAT, m_width, m_height, m_cbvSrvUavHeap.get(), &m_GBufferResources[GBufferResource::SurfaceNormal], initialResourceState);
	ThrowIfFalse(m_GBufferResources[GBufferResource::HitPosition].descriptorHeapIndex == m_GBufferResources[GBufferResource::Hit].descriptorHeapIndex + 1, 
		L"GBuffer RT resources are expecected to be subsequent in the heap.");
	ThrowIfFalse(m_GBufferResources[GBufferResource::SurfaceNormal].descriptorHeapIndex == m_GBufferResources[GBufferResource::Hit].descriptorHeapIndex + 2,
		L"GBuffer RT resources are expecected to be subsequent in the heap.");

	// ToDo move
	m_AORayHits.rwFlags = ResourceRWFlags::AllowWrite | ResourceRWFlags::AllowRead;
	CreateRenderTargetResource(device, DXGI_FORMAT_R32_UINT, m_width, m_height, m_cbvSrvUavHeap.get(), &m_AORayHits, initialResourceState);

	// Create SRVs
	// ToDo store these with the resource? Or make them optional in Create...()
	D3D12_CPU_DESCRIPTOR_HANDLE dummyHandle;
	UINT heapIndices[3] = { UINT_MAX, UINT_MAX, UINT_MAX };
	CreateTextureSRV(device, m_GBufferResources[GBufferResource::Hit].resource.Get(), m_cbvSrvUavHeap.get(), &heapIndices[0], &dummyHandle, &m_GBufferResources[GBufferResource::Hit].gpuDescriptorReadAccess);
	CreateTextureSRV(device, m_GBufferResources[GBufferResource::HitPosition].resource.Get(), m_cbvSrvUavHeap.get(), &heapIndices[1], &dummyHandle, &m_GBufferResources[GBufferResource::HitPosition].gpuDescriptorReadAccess);
	CreateTextureSRV(device, m_GBufferResources[GBufferResource::SurfaceNormal].resource.Get(), m_cbvSrvUavHeap.get(), &heapIndices[2], &dummyHandle, &m_GBufferResources[GBufferResource::SurfaceNormal].gpuDescriptorReadAccess);
	ThrowIfFalse(heapIndices[1] == heapIndices[0] + 1,
		L"GBuffer RT resources are expecected to be subsequent in the heap.");
	ThrowIfFalse(heapIndices[2] == heapIndices[0] + 2,
		L"GBuffer RT resources are expecected to be subsequent in the heap.");

	// ToDo cleanup?
	for (auto& resource : m_GBufferResources)
	{
		resource.rwFlags |= ResourceRWFlags::AllowRead;
	}

	// ToDo
	// Describe and create the point clamping sampler used for reading from the GBuffer resources.
	//CD3DX12_CPU_DESCRIPTOR_HANDLE samplerHandle(m_samplerHeap->GetHeap()->GetCPUDescriptorHandleForHeapStart());
	//D3D12_SAMPLER_DESC clampSamplerDesc = {};
	//clampSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	//clampSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	//clampSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	//clampSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	//clampSamplerDesc.MipLODBias = 0.0f;
	//clampSamplerDesc.MaxAnisotropy = 1;
	//clampSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	//clampSamplerDesc.MinLOD = 0;
	//clampSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	//device->CreateSampler(&clampSamplerDesc, samplerHandle);
}

void D3D12RaytracingAmbientOcclusion::CreateAuxilaryDeviceResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandQueue = m_deviceResources->GetCommandQueue();
	auto commandList = m_deviceResources->GetCommandList();

    for (auto& gpuTimer : m_gpuTimers)
    {
		gpuTimer.SetAvgRefreshPeriod(500);
        gpuTimer.RestoreDevice(device, commandQueue, FrameCount);
    }

	// ToDo move?
	m_reduceSumKernel.Initialize(device);
}

void D3D12RaytracingAmbientOcclusion::CreateDescriptorHeaps()
{
    auto device = m_deviceResources->GetD3DDevice();

	// CBV SRV UAV heap.
	{
		// Allocate a heap for descriptors:
		// 2 per geometry - vertex and index  buffer SRVs
		// 1 - raytracing output texture SRV
		// 2 per BLAS - one for the acceleration structure and one for its instance desc 
		// 1 - top level acceleration structure
		//ToDo
		UINT NumDescriptors = 2 * GeometryType::Count + 1 + 2 * MaxBLAS + 1 + ARRAYSIZE(SquidRoomAssets::Draws) * 2;
		m_cbvSrvUavHeap = make_unique<DescriptorHeap>(device, NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	// Sampler heap.
	{
		UINT NumDescriptors = 1;
		m_samplerHeap = make_unique<DescriptorHeap>(device, NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	}

}

void D3D12RaytracingAmbientOcclusion::BuildPlaneGeometry()
{
    auto device = m_deviceResources->GetD3DDevice();

	m_geometries[GeometryType::Plane].resize(1);

	auto& geometry = m_geometries[GeometryType::Plane][0];

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
	
	// A ByteAddressBuffer SRV is created with a ElementSize = 0 and NumElements = number of 32 - bit words.
	UINT indexBufferSize = CeilDivide(sizeof(indices), sizeof(UINT)) * sizeof(UINT);	// Pad the buffer to fit NumElements of 32bit words.
	UINT numIndexBufferElements = indexBufferSize / sizeof(UINT);

    AllocateUploadBuffer(device, indices, indexBufferSize, &geometry.ib.buffer.resource);
    AllocateUploadBuffer(device, vertices, sizeof(vertices), &geometry.vb.buffer.resource);

    // Vertex buffer is passed to the shader along with index buffer as a descriptor range.

    // ToDo revise numElements calculation
	CreateBufferSRV(device, numIndexBufferElements, 0, m_cbvSrvUavHeap.get(), &geometry.ib.buffer);
    CreateBufferSRV(device, ARRAYSIZE(vertices), sizeof(vertices[0]), m_cbvSrvUavHeap.get(), &geometry.vb.buffer);
    ThrowIfFalse(geometry.vb.buffer.heapIndex == geometry.ib.buffer.heapIndex + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index");

	m_geometryInstances[GeometryType::Plane].push_back(GeometryInstance(geometry));
}

void D3D12RaytracingAmbientOcclusion::BuildTesselatedGeometry()
{
    auto device = m_deviceResources->GetD3DDevice();

    const bool RhCoords = false;

    // ToDo option to reuse multiple geometries
	m_geometries[GeometryType::Sphere].resize(1);
    auto& geometry = m_geometries[GeometryType::Sphere][0];

#if	TESSELATED_GEOMETRY_BOX_TETRAHEDRON
	// Plane indices.
	array<Index, 12> indices =
	{ {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,

#if !TESSELATED_GEOMETRY_BOX_TETRAHEDRON_REMOVE_BOTTOM_TRIANGLE
		0, 1, 2
#endif
	} };

	const float edgeLength = 0.707f;
	const float e2 = edgeLength / 2;
	// Cube vertices positions and corresponding triangle normals.
	array<DirectX::VertexPositionNormalTexture, 4> vertices =
	{ {
#if 1
		{ XMFLOAT3(-e2, -e2, -e2), XMFLOAT3(0, 0, 0), XMFLOAT2(FLT_MAX, FLT_MAX) },
		{ XMFLOAT3(e2, -e2, -e2), XMFLOAT3(0, 0, 0), XMFLOAT2(FLT_MAX, FLT_MAX) },
		{ XMFLOAT3(0, -e2, e2), XMFLOAT3(0, 0, 0), XMFLOAT2(FLT_MAX, FLT_MAX) },
		{ XMFLOAT3(0, e2, 0), XMFLOAT3(0, 0, 0), XMFLOAT2(FLT_MAX, FLT_MAX) }
#else
		{ XMFLOAT3(e2, -e2, -e2), XMFLOAT3(1, 0, 0), XMFLOAT2(FLT_MAX, FLT_MAX) },
		{ XMFLOAT3(e2, -e2, e2), XMFLOAT3(1, 0, 0), XMFLOAT2(FLT_MAX, FLT_MAX) },
		{ XMFLOAT3(-e2, -e2, e2), XMFLOAT3(1, 0, 0), XMFLOAT2(FLT_MAX, FLT_MAX) },
		{ XMFLOAT3(e2, e2, e2), XMFLOAT3(1,0, 0), XMFLOAT2(FLT_MAX, FLT_MAX) }
#endif
		} };

#if 1
	for (auto& vertex : vertices)
	{
		auto& scale = m_boxSize;
		vertex.position.x *= (m_boxSize.x / e2);
		vertex.position.y *= (m_boxSize.y / e2);
		vertex.position.z *= (m_boxSize.z / e2);
	}
#endif

	auto Edge = [&](UINT v0, UINT v1)
	{
		return XMLoadFloat3(&vertices[v1].position) - XMLoadFloat3(&vertices[v0].position);
	};

	// ToDo, normalizing face or vertex normals skews normal in the shader.?
	XMVECTOR faceNormals[4] =
	{
		XMVector3Cross(Edge(0, 3), Edge(0, 1)),
		XMVector3Cross(Edge(1, 3), Edge(1, 2)),
		XMVector3Cross(Edge(2, 3), Edge(2, 0)),
		XMVector3Cross(Edge(0, 1), Edge(0, 2))
	};

	
#if 1 // ToDo
	XMStoreFloat3(&vertices[0].normal, (faceNormals[0] + faceNormals[2] + faceNormals[3]));
	XMStoreFloat3(&vertices[1].normal, (faceNormals[0] + faceNormals[1] + faceNormals[3]));
	XMStoreFloat3(&vertices[2].normal, (faceNormals[1] + faceNormals[2] + faceNormals[3]));
#if AO_OVERDOSE_BEND_NORMALS_DOWN
	XMStoreFloat3(&vertices[3].normal, (faceNormals[0] + faceNormals[1] + faceNormals[2]) * XMVectorSet(1, 0.01f, 1, 0));
#else
	XMStoreFloat3(&vertices[3].normal, (faceNormals[0] + faceNormals[1] + faceNormals[2]) * XMVectorSet(1, 0.01f, 1, 0));
#endif
	float a = 2;
#endif
#else
	vector<GeometricPrimitive::VertexType> vertices;
	vector<Index> indices;
	switch (SceneArgs::GeometryTesselationFactor)
    {
    case 0:
        // 24 indices
#if TESSELATED_GEOMETRY_BOX_TETRAHEDRON
		GeometricPrimitive::CreateTetrahedron(vertices, indices, m_boxSize.x, RhCoords);
#elif TESSELATED_GEOMETRY_BOX
		GeometricPrimitive::CreateBox(vertices, indices, m_boxSize, RhCoords);
#else
		GeometricPrimitive::CreateOctahedron(vertices, indices, m_geometryRadius, RhCoords);
#endif
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
#endif
#if TESSELATED_GEOMETRY_THIN
#if TESSELATED_GEOMETRY_BOX_TETRAHEDRON
	for (auto& vertex : vertices)
	{
		if (vertex.position.y > 0)
		{
			//vertex.position.y = m_boxSize.y;
		}
	}
#else
	for (auto& vertex : vertices)
	{
		if (vertex.position.y > 0)
		{
			vertex.position.x *= 0;
			vertex.position.z *= 0;
		}
	}
#endif
#endif

	// Index buffer is created with a ByteAddressBuffer SRV. 
	// ByteAddressBuffer SRV is created with an ElementSize = 0 and NumElements = number of 32 - bit words.
	UINT indexBufferSize = CeilDivide(static_cast<UINT>(indices.size() * sizeof(indices[0])), sizeof(UINT)) * sizeof(UINT);	// Pad the buffer to fit NumElements of 32bit words.
	UINT numIndexBufferElements = indexBufferSize / sizeof(UINT);

    AllocateUploadBuffer(device, indices.data(), indexBufferSize, &geometry.ib.buffer.resource);
    AllocateUploadBuffer(device, vertices.data(), vertices.size() * sizeof(vertices[0]), &geometry.vb.buffer.resource);

    // Vertex buffer is passed to the shader along with index buffer as a descriptor table.
    CreateBufferSRV(device, numIndexBufferElements, 0, m_cbvSrvUavHeap.get(), &geometry.ib.buffer);
    CreateBufferSRV(device, static_cast<UINT>(vertices.size()), sizeof(vertices[0]), m_cbvSrvUavHeap.get(), &geometry.vb.buffer);
    ThrowIfFalse(geometry.vb.buffer.heapIndex == geometry.ib.buffer.heapIndex + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index");
	m_geometryInstances[GeometryType::Sphere].resize(SceneArgs::NumGeometriesPerBLAS, GeometryInstance(geometry));

	m_numTriangles[GeometryType::Sphere] = static_cast<UINT>(indices.size()) / 3;
}

// ToDo move this out as a helper
void D3D12RaytracingAmbientOcclusion::LoadSceneGeometry()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto commandList = m_deviceResources->GetCommandList();
	
	m_geometries[GeometryType::SquidRoom].resize(1);
	auto& geometry = m_geometries[GeometryType::SquidRoom][0];

	SquidRoomAssets::LoadGeometry(
		device,
		commandList,
		m_cbvSrvUavHeap.get(),
		GetAssetFullPath(SquidRoomAssets::DataFileName).c_str(),
		&geometry,
		&m_geometryInstances[GeometryType::SquidRoom]);

	m_numTriangles[GeometryType::SquidRoom] = 0;
	for (auto& geometry : m_geometryInstances[GeometryType::SquidRoom])
	{
		m_numTriangles[GeometryType::SquidRoom] += geometry.ib.count / 3;
	}
#if PBRT_SCENE
	LoadPBRTScene();
#endif
}

// Build geometry used in the sample.
void D3D12RaytracingAmbientOcclusion::InitializeGeometry()
{
    BuildTesselatedGeometry();
    BuildPlaneGeometry();   

	// Begin frame.
	m_deviceResources->ResetCommandAllocatorAndCommandlist();
#if ONLY_SQUID_SCENE_BLAS
	LoadSceneGeometry();
#endif

#if !RUNTIME_AS_UPDATES
	InitializeAccelerationStructures();

#if !ONLY_SQUID_SCENE_BLAS
#if TESSELATED_GEOMETRY_BOX
	UpdateGridGeometryTransforms();
#else 
	UpdateSphereGeometryTransforms();
#endif
#endif
	UpdateBottomLevelASTransforms();

	UpdateAccelerationStructures(m_isASrebuildRequested);
#endif
	m_deviceResources->ExecuteCommandList();
}

void D3D12RaytracingAmbientOcclusion::GenerateBottomLevelASInstanceTransforms()
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
#if 0
        float width = 50.0f;
        XMMATRIX mScale = XMMatrixScaling(width, 1.0f, width);
        XMMATRIX mTranslation = XMMatrixTranslationFromVector(XMLoadFloat3(&XMFLOAT3(-width/2.0f, 0.0f, -width/2.0f)));
        XMMATRIX mTransform = mScale * mTranslation;
        m_vBottomLevelAS[BLASindex].SetTransform(mTransform);
#endif
		BLASindex += 1;
    }
#if DEBUG_AS
	return;
#endif 

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
void D3D12RaytracingAmbientOcclusion::InitializeAccelerationStructures()
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
#if PBRT_SCENE
		auto geometryType = GeometryType::PBRT;
#else
		auto geometryType = GeometryType::SquidRoom;
#endif
		m_vBottomLevelAS.resize(1);
		m_vBottomLevelAS[0].Initialize(device, buildFlags, SquidRoomAssets::StandardIndexFormat, SquidRoomAssets::StandardIndexStride, SquidRoomAssets::StandardVertexStride, m_geometryInstances[geometryType]);
		m_numTrianglesInTheScene = m_numTriangles[geometryType];
		
		m_vBottomLevelAS[0].SetInstanceContributionToHitGroupIndex(0);	// ToDo fix hack
		maxScratchResourceSize = max(m_vBottomLevelAS[0].RequiredScratchSize(), maxScratchResourceSize);
		m_ASmemoryFootprint += m_vBottomLevelAS[0].RequiredResultDataSizeInBytes();

		UINT numGeometryTransforms = SceneArgs::NumGeometriesPerBLAS;
#else
		m_numTrianglesInTheScene = 0;
#if DEBUG_AS
		m_vBottomLevelAS.resize(1);
		for (UINT i = 0; i < 1; i++)
#else
		m_vBottomLevelAS.resize(2);
		for (UINT i = 0; i < m_vBottomLevelAS.size(); i++)
#endif
		{
			UINT instanceContributionHitGroupIndex;
			GeometryType::Enum geometryType;
            switch (i) 
            {
			case 0: geometryType = GeometryType::Plane;
				instanceContributionHitGroupIndex = 0;
				break;
			case 1: geometryType = GeometryType::Sphere;
				instanceContributionHitGroupIndex = static_cast<UINT>(m_geometryInstances[GeometryType::Plane].size()) * RayType::Count;
				break;
			default:
				assert(0);
				break;
            };
			auto& geometryInstances = m_geometryInstances[geometryType];

			// ToDo pass IB stride from a geometryInstance object
			m_vBottomLevelAS[i].Initialize(device, buildFlags, DXGI_FORMAT_R16_UINT, sizeof(Index), sizeof(DirectX::GeometricPrimitive::VertexType), geometryInstances);
			m_numTrianglesInTheScene += m_numTriangles[geometryType];
			
			m_vBottomLevelAS[i].SetInstanceContributionToHitGroupIndex(instanceContributionHitGroupIndex);
            maxScratchResourceSize = max(m_vBottomLevelAS[i].RequiredScratchSize(), maxScratchResourceSize);
            m_ASmemoryFootprint += m_vBottomLevelAS[i].RequiredResultDataSizeInBytes();
        }
		UINT numGeometryTransforms = 1 + SceneArgs::NumSphereBLAS * SceneArgs::NumGeometriesPerBLAS;
#endif

		m_geometryTransforms.Create(device, numGeometryTransforms, FrameCount, L"Geometry descs transforms");
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
void D3D12RaytracingAmbientOcclusion::BuildShaderTables()
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
		UINT numShaderRecords = 1;
		UINT shaderRecordSize = shaderIDSize;

		for (UINT i = 0; i < RayGenShaderType::Count; i++)
		{
			ShaderTable rayGenShaderTable(device, numShaderRecords, shaderRecordSize, L"RayGenShaderTable");
			rayGenShaderTable.push_back(ShaderRecord(rayGenShaderIDs[i], shaderIDSize, nullptr, 0));
			rayGenShaderTable.DebugPrint(shaderIdToStringMap);
			m_rayGenShaderTables[i] = rayGenShaderTable.GetResource();
		}
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

	// ToDo remove
	vector<vector<GeometryInstance>*> geometryInstancesArray;
#if ONLY_SQUID_SCENE_BLAS
#if PBRT_SCENE
	geometryInstancesArray.push_back(&m_geometryInstances[GeometryType::PBRT]);
#else
	geometryInstancesArray.push_back(&m_geometryInstances[GeometryType::SquidRoom]);
#endif
#else

	geometryInstancesArray.push_back(&m_geometryInstances[GeometryType::Plane]);

#if !DEBUG_AS
	geometryInstancesArray.push_back(&m_geometryInstances[GeometryType::Sphere]);
#endif
#endif

	// Hit group shader table.
	{
		UINT numShaderRecords = 0;
		for (auto& geometryInstances : geometryInstancesArray)
		{
			numShaderRecords += static_cast<UINT>(geometryInstances->size()) * RayType::Count;
		}
		UINT shaderRecordSize = shaderIDSize + LocalRootSignature::MaxRootArgumentsSize();
		ShaderTable hitGroupShaderTable(device, numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");

		// Triangle geometry hit groups.
		for (auto& geometryInstances : geometryInstancesArray)
			for (auto& geometryInstance: *geometryInstances)
			{
				LocalRootSignature::Triangle::RootArguments rootArgs;
				rootArgs.materialCb = m_planeMaterialCB;	// ToDo
				memcpy(&rootArgs.vertexBufferGPUHandle, &geometryInstance.ib.gpuDescriptorHandle, sizeof(geometryInstance.ib.gpuDescriptorHandle));

				for (auto& hitGroupShaderID : hitGroupShaderIDs_TriangleGeometry)
				{
					hitGroupShaderTable.push_back(ShaderRecord(hitGroupShaderID, shaderIDSize, &rootArgs, sizeof(rootArgs)));
				}
			}

        hitGroupShaderTable.DebugPrint(shaderIdToStringMap);
        m_hitGroupShaderTableStrideInBytes = hitGroupShaderTable.GetShaderRecordSize();
        m_hitGroupShaderTable = hitGroupShaderTable.GetResource();
    }
}

void D3D12RaytracingAmbientOcclusion::OnKeyDown(UINT8 key)
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
void D3D12RaytracingAmbientOcclusion::OnUpdate()
{
    m_timer.Tick();

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

#if ENABLE_RAYTRACING
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
#if TESSELATED_GEOMETRY_BOX
		UpdateGridGeometryTransforms();
#else
        UpdateSphereGeometryTransforms();
#endif
		UpdateBottomLevelASTransforms();
    }
#endif
#endif

	if (m_enableUI)
    {
        UpdateUI();
    }
}

// Parse supplied command line args.
void D3D12RaytracingAmbientOcclusion::ParseCommandLineArgs(WCHAR* argv[], int argc)
{
    DXSample::ParseCommandLineArgs(argv, argc);
}

void D3D12RaytracingAmbientOcclusion::UpdateAccelerationStructures(bool forceBuild)
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
			m_vBottomLevelAS[0].Build(commandList, m_accelerationStructureScratch.Get(), m_cbvSrvUavHeap->GetHeap(), baseGeometryTransformGpuAddress, bUpdate);
		}
#else
		m_geometryTransforms.CopyStagingToGpu(frameIndex);
		// Plane
		{
			D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGpuAddress = 0;
#if USE_GPU_TRANSFORM // ToDo either place in same blas or move transform to blas?
			baseGeometryTransformGpuAddress = m_geometryTransforms.GpuVirtualAddress(frameIndex);
#endif
			m_vBottomLevelAS[BottomLevelASType::Plane].Build(commandList, m_accelerationStructureScratch.Get(), m_cbvSrvUavHeap->GetHeap(), baseGeometryTransformGpuAddress, bUpdate);
		}
#if DEBUG_AS
		if (0)
#endif
		// Sphere
		{
            D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGpuAddress = 0;     
#if USE_GPU_TRANSFORM
			// ToDo - remove: skip past plane transform
            baseGeometryTransformGpuAddress = m_geometryTransforms.GpuVirtualAddress(frameIndex) + 1 * m_geometryTransforms.ElementSize();
#endif
			m_vBottomLevelAS[BottomLevelASType::Sphere].Build(commandList, m_accelerationStructureScratch.Get(), m_cbvSrvUavHeap->GetHeap(), baseGeometryTransformGpuAddress, bUpdate);
        }
#endif
    }
	m_gpuTimers[GpuTimers::UpdateBLAS].Stop(commandList);

    m_gpuTimers[GpuTimers::UpdateTLAS].Start(commandList);
    m_topLevelAS.Build(commandList, m_accelerationStructureScratch.Get(), m_cbvSrvUavHeap->GetHeap(), bUpdate);
    m_gpuTimers[GpuTimers::UpdateTLAS].Stop(commandList);

    if (!bUpdate)
    {
        m_numFramesSinceASBuild = 0;
    }
}

void D3D12RaytracingAmbientOcclusion::DispatchRays(ID3D12Resource* rayGenShaderTable, GPUTimer* gpuTimer)
{
	auto commandList = m_deviceResources->GetCommandList();
	auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

	D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
	dispatchDesc.HitGroupTable.StartAddress = m_hitGroupShaderTable->GetGPUVirtualAddress();
	dispatchDesc.HitGroupTable.SizeInBytes = m_hitGroupShaderTable->GetDesc().Width;
	dispatchDesc.HitGroupTable.StrideInBytes = m_hitGroupShaderTableStrideInBytes;
	dispatchDesc.MissShaderTable.StartAddress = m_missShaderTable->GetGPUVirtualAddress();
	dispatchDesc.MissShaderTable.SizeInBytes = m_missShaderTable->GetDesc().Width;
	dispatchDesc.MissShaderTable.StrideInBytes = m_missShaderTableStrideInBytes;
	dispatchDesc.RayGenerationShaderRecord.StartAddress = rayGenShaderTable->GetGPUVirtualAddress();
	dispatchDesc.RayGenerationShaderRecord.SizeInBytes = rayGenShaderTable->GetDesc().Width;
	dispatchDesc.Width = m_width;
	dispatchDesc.Height = m_height;
	dispatchDesc.Depth = 1;
	commandList->SetPipelineState1(m_dxrStateObject.Get());

	gpuTimer->Start(commandList);
	commandList->DispatchRays(&dispatchDesc);
	gpuTimer->Stop(commandList);
};


void D3D12RaytracingAmbientOcclusion::DoRaytracing()
{
    auto commandList = m_deviceResources->GetCommandList();
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();
	
	// ToDo dedupe
    auto SetCommonPipelineState = [&](auto* descriptorSetCommandList)
    {
        descriptorSetCommandList->SetDescriptorHeaps(1, m_cbvSrvUavHeap->GetAddressOf());
        // Set index and successive vertex buffer decriptor tables.

        commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::OutputView, m_raytracingOutput.gpuDescriptorWriteAccess);
    };

    commandList->SetComputeRootSignature(m_raytracingGlobalRootSignature.Get());

	uniform_int_distribution<UINT> seedDistribution(0, UINT_MAX);
	
	static UINT seed = 0;
    m_sceneCB->seed = seedDistribution(m_generatorURNG);
    m_sceneCB->numSamples = m_randomSampler.NumSamples();
    m_sceneCB->numSampleSets = m_randomSampler.NumSampleSets();
#if 1
    m_sceneCB->numSamplesToUse = m_randomSampler.NumSamples();    
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
	DispatchRays(m_rayGenShaderTables[RayGenShaderType::PrimaryAndAO].Get(), &m_gpuTimers[GpuTimers::Raytracing_PrimaryAndAO]);
}

void D3D12RaytracingAmbientOcclusion::DoRaytracingGBufferAndAOPasses()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto commandList = m_deviceResources->GetCommandList();
	auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

	auto SetCommonPipelineState = [&]()
	{
		commandList->SetDescriptorHeaps(1, m_cbvSrvUavHeap->GetAddressOf());
	};

	auto CalculateRayHitCount = [&](ReduceSumCalculations::Enum type)
	{
		RWGpuResource* inputResource = nullptr;
		switch (type)
		{
		case ReduceSumCalculations::CameraRayHits: inputResource = &m_GBufferResources[GBufferResource::Hit]; break;
		case ReduceSumCalculations::AORayHits: inputResource = &m_AORayHits; break;
		}

		m_gpuTimers[GpuTimers::ReduceSum].Start(commandList, type);
		m_reduceSumKernel.Execute(
			device,
			commandList,
			m_cbvSrvUavHeap->GetHeap(),
			frameIndex,
			inputResource->gpuDescriptorReadAccess,
			type,
			&m_numRayGeometryHits[type]);
		m_gpuTimers[GpuTimers::ReduceSum].Stop(commandList, type);
	};

	uniform_int_distribution<UINT> seedDistribution(0, UINT_MAX);

	// ToDo remove?
	static UINT seed = 0;
	m_sceneCB->seed = seedDistribution(m_generatorURNG);
	m_sceneCB->numSamples = m_randomSampler.NumSamples();
	m_sceneCB->numSampleSets = m_randomSampler.NumSampleSets();
	m_sceneCB->numSamplesToUse = m_randomSampler.NumSamples();    
	UINT NumFramesPerIter = 400;

	commandList->SetComputeRootSignature(m_raytracingGlobalRootSignature.Get());
	SetCommonPipelineState();

	// Copy dynamic buffers to GPU.
	{
		m_hemisphereSamplesGPUBuffer.CopyStagingToGpu(frameIndex);
		m_sceneCB.CopyStagingToGpu(frameIndex);
	}

	// Transition GBuffer resources to render target state.
	{
		D3D12_RESOURCE_BARRIER barriers[GBufferResource::Count + 1];
		D3D12_RESOURCE_STATES before = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		D3D12_RESOURCE_STATES after = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_GBufferResources[GBufferResource::Hit].resource.Get(), before, after);
		barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_GBufferResources[GBufferResource::HitPosition].resource.Get(), before, after);
		barriers[2] = CD3DX12_RESOURCE_BARRIER::Transition(m_GBufferResources[GBufferResource::SurfaceNormal].resource.Get(), before, after);
		barriers[3] = CD3DX12_RESOURCE_BARRIER::Transition(m_AORayHits.resource.Get(), before, after);
		commandList->ResourceBarrier(ARRAYSIZE(barriers), barriers);

	}


	// Bind inputs.
	commandList->SetComputeRootShaderResourceView(GlobalRootSignature::Slot::AccelerationStructure, m_topLevelAS.GetResource()->GetGPUVirtualAddress());
	commandList->SetComputeRootShaderResourceView(GlobalRootSignature::Slot::SampleBuffers, m_hemisphereSamplesGPUBuffer.GpuVirtualAddress(frameIndex));
	commandList->SetComputeRootConstantBufferView(GlobalRootSignature::Slot::SceneConstant, m_sceneCB.GpuVirtualAddress(frameIndex));

	// Bind output RTs.
	commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::GBufferResources, m_GBufferResources[GBufferResource::Hit].gpuDescriptorWriteAccess);

#if RAYGEN_SINGLE_COLOR_SHADING
	commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::OutputView, m_raytracingOutput.gpuDescriptorWriteAccess);
#endif

	// Dispatch Rays.
	DispatchRays(m_rayGenShaderTables[RayGenShaderType::GBuffer].Get(), &m_gpuTimers[GpuTimers::Raytracing_GBuffer]);

	// Transition GBuffer resources to shader resource state.
	{
		D3D12_RESOURCE_BARRIER barriers[GBufferResource::Count];
		D3D12_RESOURCE_STATES before = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		D3D12_RESOURCE_STATES after = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_GBufferResources[GBufferResource::Hit].resource.Get(), before, after);
		barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_GBufferResources[GBufferResource::HitPosition].resource.Get(), before, after);
		barriers[2] = CD3DX12_RESOURCE_BARRIER::Transition(m_GBufferResources[GBufferResource::SurfaceNormal].resource.Get(), before, after);
		commandList->ResourceBarrier(ARRAYSIZE(barriers), barriers);
	}

	CalculateRayHitCount(ReduceSumCalculations::CameraRayHits);

	//*************************
	// AO pass
	//*************************

	commandList->SetComputeRootSignature(m_raytracingGlobalRootSignature.Get());

	// Bind inputs.
	commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::GBufferResourcesIn, m_GBufferResources[GBufferResource::Hit].gpuDescriptorReadAccess);
	commandList->SetComputeRootShaderResourceView(GlobalRootSignature::Slot::SampleBuffers, m_hemisphereSamplesGPUBuffer.GpuVirtualAddress(frameIndex));
	commandList->SetComputeRootConstantBufferView(GlobalRootSignature::Slot::SceneConstant, m_sceneCB.GpuVirtualAddress(frameIndex));

	// Bind output RT.
	commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::OutputView, m_raytracingOutput.gpuDescriptorWriteAccess);
	commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::AOResourcesOut, m_AORayHits.gpuDescriptorWriteAccess);

	// Bind the heaps, acceleration structure and dispatch rays. 
	commandList->SetComputeRootShaderResourceView(GlobalRootSignature::Slot::AccelerationStructure, m_topLevelAS.GetResource()->GetGPUVirtualAddress());
	DispatchRays(m_rayGenShaderTables[RayGenShaderType::AO].Get(), &m_gpuTimers[GpuTimers::Raytracing_AO]);

	// Transition AORayHits resource to shader resource state.
	{
		D3D12_RESOURCE_STATES before = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		D3D12_RESOURCE_STATES after = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_AORayHits.resource.Get(), before, after));
	}

	CalculateRayHitCount(ReduceSumCalculations::AORayHits);
}

// Copy the raytracing output to the backbuffer.
void D3D12RaytracingAmbientOcclusion::CopyRaytracingOutputToBackbuffer(D3D12_RESOURCE_STATES outRenderTargetState)
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

void D3D12RaytracingAmbientOcclusion::UpdateUI()
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
#if GBUFFER_AO_SEPRATE_PATHS
		wLabel << fixed << L" CameraRay DispatchRays: " << m_gpuTimers[GpuTimers::Raytracing_GBuffer].GetAverageMS() << L"ms\n";
		wLabel << fixed << L" AORay DispatchRays: " << m_gpuTimers[GpuTimers::Raytracing_AO].GetAverageMS() << L"ms\n";
		wLabel << fixed << L" DispatchRays: " << m_gpuTimers[GpuTimers::Raytracing_GBuffer].GetAverageMS() << L"ms"
			   << L"     ~Million Primary Rays/s: " << NumCameraRaysPerSecond()
   			   << L"   ~Million AO rays/s" << NumRayGeometryHitsPerSecond(ReduceSumCalculations::CameraRayHits) * c_sppAO
               << L"\n";
#else
		wLabel << fixed << L" CameraRay DispatchRays: " << m_gpuTimers[GpuTimers::Raytracing_PrimaryAndAO].GetAverageMS() << L"ms\n";
		wLabel << fixed << L" AORay DispatchRays: " << m_gpuTimers[GpuTimers::Raytracing_PrimaryAndAO].GetAverageMS() << L"ms\n";
		wLabel << fixed << L" DispatchRays: " << m_gpuTimers[GpuTimers::Raytracing_PrimaryAndAO].GetAverageMS() << L"ms"
			<< L"     ~Million Primary Rays/s: " << NumCameraRaysPerSecond()
			<< L"   ~Million AO rays/s" << NumRayGeometryHitsPerSecond(ReduceSumCalculations::CameraRayHits) * c_sppAO
			<< L"\n";
#endif
        wLabel << fixed << L" AS update (BLAS / TLAS / Total): "
               << m_gpuTimers[GpuTimers::UpdateBLAS].GetElapsedMS() << L"ms / "
               << m_gpuTimers[GpuTimers::UpdateTLAS].GetElapsedMS() << L"ms / "
               << m_gpuTimers[GpuTimers::UpdateBLAS].GetElapsedMS() +
                  m_gpuTimers[GpuTimers::UpdateTLAS].GetElapsedMS() << L"ms\n";
		wLabel << fixed << L" CameraRayGeometryHits: #/%%/time " 
			   << m_numRayGeometryHits[ReduceSumCalculations::CameraRayHits] << "/"
			   << ((m_width * m_height) > 0 ? (100.f * m_numRayGeometryHits[ReduceSumCalculations::CameraRayHits]) / (m_width*m_height) : 0) << "%%/"
			   << 1000 * m_gpuTimers[GpuTimers::ReduceSum].GetAverageMS(ReduceSumCalculations::CameraRayHits) << L"us \n";
		wLabel << fixed << L" AORayGeometryHits: #/%%/time "
			   << m_numRayGeometryHits[ReduceSumCalculations::AORayHits] << "/"
			   << ((m_numRayGeometryHits[ReduceSumCalculations::CameraRayHits] * c_sppAO) > 0 ?
				   (100.f * m_numRayGeometryHits[ReduceSumCalculations::AORayHits]) / (m_numRayGeometryHits[ReduceSumCalculations::CameraRayHits] * c_sppAO) : 0) << "%%/"
			   << 1000 * m_gpuTimers[GpuTimers::ReduceSum].GetAverageMS(ReduceSumCalculations::AORayHits) << L"us \n";
    
        labels.push_back(wLabel.str());
    }

    // Parameters.
    labels.push_back(L"\n");
    {
        wstringstream wLabel;
        wLabel << L"Scene:" << L"\n";
        wLabel << L" " << L"AS update mode: " << SceneArgs::ASUpdateMode << L"\n";
        wLabel.precision(3);
        wLabel << L" " << L"AS memory footprint: " << static_cast<double>(m_ASmemoryFootprint)/(1024*1024) << L"MB\n";
        wLabel << L" " << L" # triangles per geometry: " << m_numTrianglesInTheScene << L"\n";
        wLabel << L" " << L" # geometries per BLAS: " << SceneArgs::NumGeometriesPerBLAS << L"\n";
        wLabel << L" " << L" # Sphere BLAS: " << SceneArgs::NumSphereBLAS << L"\n";	// ToDo fix
		wLabel << L" " << L" # total triangles: " << m_numTrianglesInTheScene << L"\n";// SceneArgs::NumSphereBLAS * SceneArgs::NumGeometriesPerBLAS* m_numTriangles[SceneArgs::SceneType] << L"\n";
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
		wLabel << L"Sample set: " << m_csHemisphereVisualizationCB->sampleSetBase / m_randomSampler.NumSamples() << " / " << m_randomSampler.NumSampleSets() << L"\n";
		
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
void D3D12RaytracingAmbientOcclusion::CreateWindowSizeDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandQueue = m_deviceResources->GetCommandQueue();
    auto renderTargets = m_deviceResources->GetRenderTargets();

    // Create an output 2D texture to store the raytracing result to.
    CreateRaytracingOutputResource();

	CreateGBufferResources();
	m_reduceSumKernel.CreateInputResourceSizeDependentResources(
		device,
		m_cbvSrvUavHeap.get(), 
		FrameCount, 
		m_width, 
		m_height, 
		ReduceSumCalculations::Count);
        
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
void D3D12RaytracingAmbientOcclusion::ReleaseWindowSizeDependentResources()
{
    if (m_enableUI)
    {
        m_uiLayer.reset();
    }
    m_raytracingOutput.resource.Reset();
}

// Release all resources that depend on the device.
void D3D12RaytracingAmbientOcclusion::ReleaseDeviceDependentResources()
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

	m_cbvSrvUavHeap.release();

    m_csHemisphereVisualizationCB.Release();

    // ToDo
    for (auto& bottomLevelAS : m_vBottomLevelAS)
    {
        bottomLevelAS.ReleaseD3DResources();
    }
    m_topLevelAS.ReleaseD3DResources();

    m_raytracingOutput.resource.Reset();
	ResetComPtrArray(&m_rayGenShaderTables);
    m_missShaderTable.Reset();
    m_hitGroupShaderTable.Reset();
}

void D3D12RaytracingAmbientOcclusion::RecreateD3D()
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

void D3D12RaytracingAmbientOcclusion::RenderRNGVisualizations()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

	// Update constant buffer.
	XMUINT2 rngWindowSize(256, 256);
	{
		m_csHemisphereVisualizationCB->dispatchDimensions = rngWindowSize;

		static UINT seed = 0;
		UINT NumFramesPerIter = 400;
		static UINT frameID = NumFramesPerIter * 4;
		m_csHemisphereVisualizationCB->numSamplesToShow = c_sppAO;// (frameID++ / NumFramesPerIter) % m_randomSampler.NumSamples();
		m_csHemisphereVisualizationCB->sampleSetBase = ((seed++ / NumFramesPerIter) % m_randomSampler.NumSampleSets()) * m_randomSampler.NumSamples();
		m_csHemisphereVisualizationCB->stratums = XMUINT2(static_cast<UINT>(sqrt(m_randomSampler.NumSamples())),
			static_cast<UINT>(sqrt(m_randomSampler.NumSamples())));
		m_csHemisphereVisualizationCB->grid = XMUINT2(m_randomSampler.NumSamples(), m_randomSampler.NumSamples());
		m_csHemisphereVisualizationCB->uavOffset = XMUINT2(0 /*ToDo remove m_width - rngWindowSize.x*/, m_height - rngWindowSize.y);
		m_csHemisphereVisualizationCB->numSamples = m_randomSampler.NumSamples();
		m_csHemisphereVisualizationCB->numSampleSets = m_randomSampler.NumSampleSets();
	}

    // Copy dynamic buffers to GPU
    {
        m_csHemisphereVisualizationCB.CopyStagingToGpu(frameIndex);
        m_samplesGPUBuffer.CopyStagingToGpu(frameIndex);
    }

	// Set pipeline state.
	{
		using namespace ComputeShader::RootSignature::HemisphereSampleSetVisualization;

		commandList->SetDescriptorHeaps(1, m_cbvSrvUavHeap->GetAddressOf());
		commandList->SetComputeRootSignature(m_computeRootSigs[ComputeShader::Type::HemisphereSampleSetVisualization].Get());

		commandList->SetComputeRootConstantBufferView(Slot::SceneConstant, m_csHemisphereVisualizationCB.GpuVirtualAddress(frameIndex));
		commandList->SetComputeRootShaderResourceView(Slot::SampleBuffers, m_samplesGPUBuffer.GpuVirtualAddress(frameIndex));
		commandList->SetComputeRootDescriptorTable(Slot::OutputView, m_raytracingOutput.gpuDescriptorWriteAccess);

		commandList->SetPipelineState(m_computePSOs[ComputeShader::Type::HemisphereSampleSetVisualization].Get());
	}

	// Dispatch.
    commandList->Dispatch(rngWindowSize.x, rngWindowSize.y, 1);
}

// Render the scene.
void D3D12RaytracingAmbientOcclusion::OnRender()
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

#if ENABLE_RAYTRACING
#if RUNTIME_AS_UPDATES
    // Update acceleration structures.
    if (m_isASrebuildRequested && SceneArgs::EnableGeometryAndASBuildsAndUpdates)
    {
        UpdateAccelerationStructures(m_isASrebuildRequested);
        m_isASrebuildRequested = false;
    }
#endif

    // Render.
#if GBUFFER_AO_SEPRATE_PATHS
	DoRaytracingGBufferAndAOPasses();
#else
	DoRaytracing();
#endif
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
void D3D12RaytracingAmbientOcclusion::CalculateFrameStats()
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
                << L"    fps: " << m_fps << L"     ~Million Primary Rays/s: " << NumCameraRaysPerSecond()
                << L"    GPU[" << m_deviceResources->GetAdapterID() << L"]: " << m_deviceResources->GetAdapterDescription();
            SetCustomWindowText(windowText.str().c_str());
        }
    }
}

// Handle OnSizeChanged message event.
void D3D12RaytracingAmbientOcclusion::OnSizeChanged(UINT width, UINT height, bool minimized)
{
    UpdateForSizeChange(width, height);

    if (!m_deviceResources->WindowSizeChanged(width, height, minimized))
    {
        return;
    }
}