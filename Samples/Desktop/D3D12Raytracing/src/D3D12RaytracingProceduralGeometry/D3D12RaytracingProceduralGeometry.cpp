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
#include "D3D12RaytracingProceduralGeometry.h"
#include "DirectXRaytracingHelper.h"
#include "CompiledShaders\Raytracing.hlsl.h"

using namespace std;
using namespace DX;

const wchar_t* D3D12RaytracingProceduralGeometry::c_hitGroupNames[] = { L"MyHitGroup_Triangle", L"MyHitGroup_AABB", L"MyHitGroup_ShadowRayAABB" };
const wchar_t* D3D12RaytracingProceduralGeometry::c_raygenShaderName = L"MyRaygenShader";
const wchar_t* D3D12RaytracingProceduralGeometry::c_intersectionShaderName = L"MyIntersectionShader_AABB";
const wchar_t* D3D12RaytracingProceduralGeometry::c_closestHitShaderNames[] = { L"MyClosestHitShader_Triangle", L"MyClosestHitShader_AABB", L"MyClosestHitShader_ShadowAABB" };
const wchar_t* D3D12RaytracingProceduralGeometry::c_missShaderNames[] = { L"MyMissShader", L"MyMissShader_Shadow" };

D3D12RaytracingProceduralGeometry::D3D12RaytracingProceduralGeometry(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_raytracingOutputResourceUAVDescriptorHeapIndex(UINT_MAX),
    m_curRotationAngleRad(0.0f)
{
    // DXR is an experimental feature and needs to be enabled before creating a D3D12 device.
    m_isDxrSupported = EnableRaytracing();

    if (!m_isDxrSupported)
    {
        OutputDebugString(
            L"Could not enable raytracing driver (D3D12EnableExperimentalFeatures() failed).\n" \
            L"Possible reasons:\n" \
            L"  1) your OS is not in developer mode.\n" \
            L"  2) your GPU driver doesn't match the D3D12 runtime loaded by the app (d3d12.dll and friends).\n" \
            L"  3) your D3D12 runtime doesn't match the D3D12 headers used by your app (in particular, the GUID passed to D3D12EnableExperimentalFeatures).\n\n");

        OutputDebugString(L"Enabling compute based fallback raytracing support.\n");
        ThrowIfFalse(EnableComputeRaytracingFallback(), L"Could not enable compute based fallback raytracing support (D3D12EnableExperimentalFeatures() failed).\n");
    }

    m_forceComputeFallback = false;
    SelectRaytracingAPI(RaytracingAPI::FallbackLayer);

    m_deviceResources = std::make_unique<DeviceResources>(
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_UNKNOWN,
        FrameCount,
        D3D_FEATURE_LEVEL_11_0,
        DeviceResources::c_AllowTearing
        );
    m_deviceResources->RegisterDeviceNotify(this);

    // Sample shows handling of use cases with tearing support, which is OS dependent and has been supported since Threshold II.
    // Since the Fallback Layer requires Fall Creator's update (RS3), we don't need to handle non-tearing cases.
    if (!m_deviceResources->IsTearingSupported())
    {
        OutputDebugString(L"Sample must be run on an OS with tearing support.\n");
        exit(EXIT_FAILURE);
    }

    UpdateForSizeChange(width, height);
}

void D3D12RaytracingProceduralGeometry::OnInit()
{
    m_deviceResources->SetWindow(Win32Application::GetHwnd(), m_width, m_height);

    m_deviceResources->CreateDeviceResources();
    m_deviceResources->CreateWindowSizeDependentResources();

    InitializeScene();

    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

// Update camera matrices passed into the shader.
void D3D12RaytracingProceduralGeometry::UpdateCameraMatrices()
{
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    m_sceneCB[frameIndex].cameraPosition = m_eye;
    float fovAngleY = 45.0f;
    XMMATRIX view = XMMatrixLookAtLH(m_eye, m_at, m_up);
    XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), m_aspectRatio, 1.0f, 125.0f);
    XMMATRIX viewProj = view * proj;

    m_sceneCB[frameIndex].projectionToWorld = XMMatrixInverse(nullptr, viewProj);
}

// Update AABB primite attributes buffers passed into the shader.
void D3D12RaytracingProceduralGeometry::UpdateAABBPrimitiveAttributes()
{
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    const float aabbDefaultWidth = 2;    // Default AABB is <-1,1>^3
    //const float c_aabbWidth  = 2;// 1 / sqrt(2.0f);// Width of each AABB - scaled down to fit any AABB rotation within the default width
    const float c_aabbDistance = 2;        // Distance between AABBs
    const float aabbDistanceStride = c_aabbWidth  + c_aabbDistance;
    const XMVECTOR vAABBstride = XMLoadFloat3(&XMFLOAT3(aabbDistanceStride, aabbDistanceStride, aabbDistanceStride));

    float scaleRatio = c_aabbWidth  / aabbDefaultWidth;
    XMMATRIX mScale = XMMatrixScaling(scaleRatio, scaleRatio, scaleRatio);
   
    const XMVECTOR vBasePosition = XMLoadFloat3(&XMFLOAT3(
        -((NUM_AABB_X-1) * c_aabbWidth  + (NUM_AABB_X - 1) * c_aabbDistance) / 2.0f,
        -((NUM_AABB_Y-1) * c_aabbWidth  + (NUM_AABB_Y - 1) * c_aabbDistance) / 2.0f,
        -((NUM_AABB_Z-1) * c_aabbWidth  + (NUM_AABB_Z - 1) * c_aabbDistance) / 2.0f));


    for (UINT z = 0; z < NUM_AABB_Z; z++)
    {
        for (UINT y = 0; y < NUM_AABB_Y; y++)
        {
            for (UINT x = 0; x < NUM_AABB_X; x++)
            {
                float _x = static_cast<float>(x);
                float _y = static_cast<float>(y);
                float _z = static_cast<float>(z);

                auto& aabbAttribute = m_aabbPrimitiveAttributeBuffer[frameIndex][z][y][x];
                XMVECTOR vIndex = XMLoadFloat3(&XMFLOAT3(_x, _y, _z));
                XMVECTOR vTranslation = vBasePosition + vIndex * vAABBstride;
                XMMATRIX mRotation = XMMatrixIdentity();// XMMatrixRotationY((x + y + z) * XM_2PI / NUM_AABB);// XMConvertToRadians(XMVectorGetX(XMVector3Length(vTranslation))));
                XMMATRIX mTranslation = XMMatrixTranslationFromVector(vTranslation);
                aabbAttribute.bottomLevelASToLocalSpace = XMMatrixInverse(nullptr, mRotation * mTranslation);
                
                aabbAttribute.albedo = XMFLOAT3((_x + 1) / NUM_AABB_X, (_y + 1) / NUM_AABB_Y, (_z + 1) / NUM_AABB_Z);
            }
        }
    }
}

// Initialize scene rendering parameters.
void D3D12RaytracingProceduralGeometry::InitializeScene()
{
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    // Setup materials.
    {
        XMFLOAT4 cubeDiffuseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        m_cubeCB.diffuseColor = XMLoadFloat4(&cubeDiffuseColor);
    }

    // Setup camera.
    {
        // Initialize the view and projection inverse matrices.
        m_eye = { 0.0f, 7.0f, -18.0f, 1.0f };
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

        lightPosition = XMFLOAT4(0.0f, 18.0f, -30.0f, 0.0f);
        m_sceneCB[frameIndex].lightPosition = XMLoadFloat4(&lightPosition);

        lightAmbientColor = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
        m_sceneCB[frameIndex].lightAmbientColor = XMLoadFloat4(&lightAmbientColor);

        lightDiffuseColor = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
        m_sceneCB[frameIndex].lightDiffuseColor = XMLoadFloat4(&lightDiffuseColor);
    }

    // Apply the initial values to all frames' buffer instances.
    for (auto& sceneCB : m_sceneCB)
    {
        sceneCB = m_sceneCB[frameIndex];
    }
}

// Create constant buffers.
void D3D12RaytracingProceduralGeometry::CreateConstantBuffers()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto frameCount = m_deviceResources->GetBackBufferCount();

    // Create the constant buffer memory and map the CPU and GPU addresses
    const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    // Allocate one constant buffer per frame, since it gets updated every frame.
    size_t cbSize = frameCount * sizeof(AlignedSceneConstantBuffer);
    const D3D12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(cbSize);

    ThrowIfFailed(device->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &constantBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_perFrameConstants)));

    // Map the constant buffer and cache its heap pointers.
    // We don't unmap this until the app closes. Keeping buffer mapped for the lifetime of the resource is okay.
    CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(m_perFrameConstants->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedConstantData)));
}

// Create AABB primitive attributes buffers.
void D3D12RaytracingProceduralGeometry::CreateAABBPrimitiveAttributesBuffers()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto frameCount = m_deviceResources->GetBackBufferCount();

    // Create the buffer memory and map the CPU and GPU addresses
    const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    // Allocate one buffer per frame, since it gets updated every frame.
    size_t bufferSize = frameCount * AABB_BUFFER_SIZE;
    const D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    ThrowIfFailed(device->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_perFrameAABBPrimitiveAttributes)));

    // Map the constant buffer and cache its heap pointers.
    // We don't unmap this until the app closes. Keeping buffer mapped for the lifetime of the resource is okay.
    CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(m_perFrameAABBPrimitiveAttributes->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedAABBPrimitiveAttributes)));
}

// Create resources that depend on the device.
void D3D12RaytracingProceduralGeometry::CreateDeviceDependentResources()
{
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
    BuildGeometry();

    // Build raytracing acceleration structures from the generated geometry.
    BuildAccelerationStructures();

    // Create constant buffers for the geometry and the scene.
    CreateConstantBuffers();

    // Create AABB primitive attribute buffers.
    CreateAABBPrimitiveAttributesBuffers();

    // Build shader tables, which define shaders and their local root arguments.
    BuildShaderTables();

    // Create an output 2D texture to store the raytracing result to.
    CreateRaytracingOutputResource();
}

void D3D12RaytracingProceduralGeometry::SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig)
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

void D3D12RaytracingProceduralGeometry::CreateRootSignatures()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Global Root Signature
    // This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
    {
        CD3DX12_DESCRIPTOR_RANGE ranges[2]; // Perfomance TIP: Order from most frequent to least frequent.
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 1 output texture
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1);  // 2 static index and vertex buffers.

        CD3DX12_ROOT_PARAMETER rootParameters[GlobalRootSignatureParams::Count];
        rootParameters[GlobalRootSignatureParams::OutputViewSlot].InitAsDescriptorTable(1, &ranges[0]);
        rootParameters[GlobalRootSignatureParams::AccelerationStructureSlot].InitAsShaderResourceView(0);
        rootParameters[GlobalRootSignatureParams::SceneConstantSlot].InitAsConstantBufferView(0);
        rootParameters[GlobalRootSignatureParams::AABBattributeBufferSlot].InitAsShaderResourceView(3);
        rootParameters[GlobalRootSignatureParams::VertexBuffersSlot].InitAsDescriptorTable(1, &ranges[1]);
        CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
        SerializeAndCreateRaytracingRootSignature(globalRootSignatureDesc, &m_raytracingGlobalRootSignature);
    }

    // Local Root Signature
    // This is a root signature that enables a shader to have unique arguments that come from shader tables.
    {
        CD3DX12_DESCRIPTOR_RANGE UAVDescriptor;
        CD3DX12_ROOT_PARAMETER rootParameters[LocalRootSignatureParams::Count];
        rootParameters[LocalRootSignatureParams::CubeConstantSlot].InitAsConstants(SizeOfInUint32(m_cubeCB), 1);
        CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
        localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
        SerializeAndCreateRaytracingRootSignature(localRootSignatureDesc, &m_raytracingLocalRootSignature);
    }
}

// Create raytracing device and command list.
void D3D12RaytracingProceduralGeometry::CreateRaytracingInterfaces()
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
        ThrowIfFailed(device->QueryInterface(__uuidof(ID3D12DeviceRaytracingPrototype), &m_dxrDevice), L"Couldn't get DirectX Raytracing interface for the device.\n");
        ThrowIfFailed(commandList->QueryInterface(__uuidof(ID3D12CommandListRaytracingPrototype), &m_dxrCommandList), L"Couldn't get DirectX Raytracing interface for the command list.\n");
    }
}

// Create a raytracing pipeline state object (RTPSO).
// An RTPSO represents a full set of shaders reachable by a DispatchRays() call,
// with all configuration options resolved, such as local signatures and other state.
void D3D12RaytracingProceduralGeometry::CreateRaytracingPipelineStateObject()
{
    // Create 8 subobjects that combine into a RTPSO:
    // Subobjects need to be associated with DXIL exports (i.e. shaders) either by way of default or explicit associations.
    // Default association applies to every exported shader entrypoint that doesn't have any of the same type of subobject associated with it.
    // This simple sample utilizes default shader association except for local root signature subobject
    // which has an explicit association specified purely for demonstration purposes.
    // 1 - DXIL library
    // 2 - Hit groups - triangle and AABB
    // 1 - Shader config
    // 2 - Local root signature and association
    // 1 - Global root signature
    // 1 - Pipeline config
    CD3D12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };


    // DXIL library
    // This contains the shaders and their entrypoints for the state object.
    // Since shaders are not considered a subobject, they need to be passed in via DXIL library subobjects.
    auto lib = raytracingPipeline.CreateSubobject<CD3D12_DXIL_LIBRARY_SUBOBJECT>();
    D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void *)g_pRaytracing, ARRAYSIZE(g_pRaytracing));
    lib->SetDXILLibrary(&libdxil);
    // Define which shader exports to surface from the library.
    // If no shader exports are defined for a DXIL library subobject, all shaders will be surfaced.
    // In this sample, this could be ommited for convenience since the sample uses all shaders in the library. 
    {
        lib->DefineExport(c_raygenShaderName);
        lib->DefineExport(c_intersectionShaderName);
        for (auto& closestHitShaderName : c_closestHitShaderNames)
        {
            lib->DefineExport(closestHitShaderName);
        }
        for (auto& missShaderName : c_missShaderNames)
        {
            lib->DefineExport(missShaderName);
        }
    }

    // Hit groups
    // A hit group specifies closest hit, any hit and intersection shaders to be executed when a ray intersects the geometry's triangle/AABB.
    // In this sample, we use two hit groups: triangle and AABB primitive geometry. Both of which use a closest hit shader, so others are not set.
    {
        // Triangle hit group
        {
            auto hitGroup = raytracingPipeline.CreateSubobject<CD3D12_HIT_GROUP_SUBOBJECT>();
            hitGroup->SetClosestHitShaderImport(c_closestHitShaderNames[ClosestHitRayType::Triangle]);
            hitGroup->SetHitGroupExport(c_hitGroupNames[HitGroupType::Triangle]);
        }

        // AABB hit group
        {
            auto hitGroup = raytracingPipeline.CreateSubobject<CD3D12_HIT_GROUP_SUBOBJECT>();
            hitGroup->SetIntersectionShaderImport(c_intersectionShaderName);
            hitGroup->SetClosestHitShaderImport(c_closestHitShaderNames[ClosestHitRayType::AABB]);
            hitGroup->SetHitGroupExport(c_hitGroupNames[HitGroupType::AABB]);
        }


        // ToDo add hit group for triangle geometry
        // Shadow ray hit group
        {
            auto hitGroup = raytracingPipeline.CreateSubobject<CD3D12_HIT_GROUP_SUBOBJECT>();
            hitGroup->SetIntersectionShaderImport(c_intersectionShaderName);
            hitGroup->SetClosestHitShaderImport(c_closestHitShaderNames[ClosestHitRayType::ShadowAABB]);
            hitGroup->SetHitGroupExport(c_hitGroupNames[HitGroupType::ShadowAABB]);
        }
    }

    // Shader config
    // Defines the maximum sizes in bytes for the ray payload and attribute structure.
    auto shaderConfig = raytracingPipeline.CreateSubobject<CD3D12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    UINT payloadSize = sizeof(XMFLOAT4);    // float4 pixelColor
    // ToDo fix attribute Size
    UINT attributeSize = sizeof(XMFLOAT2) + sizeof(XMFLOAT4);  // float2 barycentrics, float4 normal
    shaderConfig->Config(payloadSize, attributeSize);

    // Local root signature and shader association
    // This is a root signature that enables a shader to have unique arguments that come from shader tables.
    auto localRootSignature = raytracingPipeline.CreateSubobject<CD3D12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
    localRootSignature->SetRootSignature(m_raytracingLocalRootSignature.Get());
    // Define explicit shader association for the local root signature. 
    // In this sample, this could be ommited for convenience since it matches the default association.
    {
        auto rootSignatureAssociation = raytracingPipeline.CreateSubobject<CD3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
        // ToDo set local root sig association to empty root sig where it's not needed/
        rootSignatureAssociation->AddExport(c_raygenShaderName);
        for (auto& hitGroupShaderName : c_hitGroupNames)
        {
            rootSignatureAssociation->AddExport(hitGroupShaderName);
        }
        for (auto& missShaderName : c_missShaderNames)
        {
            rootSignatureAssociation->AddExport(missShaderName);
        }
    }

    // Global root signature
    // This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
    auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3D12_ROOT_SIGNATURE_SUBOBJECT>();
    globalRootSignature->SetRootSignature(m_raytracingGlobalRootSignature.Get());

    // Pipeline config
    // Defines the maximum TraceRay() recursion depth.
    auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3D12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
    // Setting max recursion depth at 2 ~ primary rays + shadow rays. 
    // Drivers may apply optimization strategies for low recursion depths, 
    // so it is recommended to set max recursion depth as low as needed. 
    pipelineConfig->Config(2);

#if _DEBUG
    PrintStateObjectDesc(raytracingPipeline);
#endif

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

// Create 2D output texture for raytracing.
void D3D12RaytracingProceduralGeometry::CreateRaytracingOutputResource()
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

void D3D12RaytracingProceduralGeometry::CreateDescriptorHeap()
{
    auto device = m_deviceResources->GetD3DDevice();

    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
    // Allocate a heap for 5 descriptors:
    // 2 - vertex and index buffer SRVs
    // 1 - raytracing output texture SRV
    // 2 - bottom and top level acceleration structure fallback wrapped pointer UAVs
    descriptorHeapDesc.NumDescriptors = 5;
    descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    descriptorHeapDesc.NodeMask = 0;
    device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_descriptorHeap));
    NAME_D3D12_OBJECT(m_descriptorHeap);

    m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

// Build AABBs for procedural geometry within a bottom-level acceleration structure.
void D3D12RaytracingProceduralGeometry::BuildProceduralGeometryAABBs()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Create a grid of AABBs.
    {
        const float aabbBaseWidth = 2;                  // Width of each AABB
        const float aabbRotationBloat = 1;// sqrt(2.0f);     // Size bloat for AABB to encapsule any rotations of base AABB
        const float c_aabbWidth  = aabbBaseWidth * aabbRotationBloat;

        const XMFLOAT3 basePosition =
        {
            -(NUM_AABB_X * c_aabbWidth  + (NUM_AABB_X - 1) * c_aabbDistance) / 2.0f,
            -(NUM_AABB_Y * c_aabbWidth  + (NUM_AABB_Y - 1) * c_aabbDistance) / 2.0f,
            -(NUM_AABB_Z * c_aabbWidth  + (NUM_AABB_Z - 1) * c_aabbDistance) / 2.0f,
        };

        // ToDo This is calculated twice - here and in update AABB
        D3D12_RAYTRACING_AABB aabb[NUM_AABB_Z][NUM_AABB_Y][NUM_AABB_X];
        for (UINT z = 0; z < NUM_AABB_Z; z++)
        {
            FLOAT minZ = basePosition.z + z * (c_aabbWidth  + c_aabbDistance);
            for (UINT y = 0; y < NUM_AABB_Y; y++)
            {
                FLOAT minY = basePosition.y + y * (c_aabbWidth  + c_aabbDistance);
                for (UINT x = 0; x < NUM_AABB_X; x++)
                {
                    FLOAT minX = basePosition.x + x * (c_aabbWidth  + c_aabbDistance);
                    aabb[z][y][x] =
                    {
                        minX, minY, minZ, minX + c_aabbWidth , minY + c_aabbWidth , minZ + c_aabbWidth 
                    };
                }
            }
        }
        AllocateUploadBuffer(device, &aabb, sizeof(aabb), &m_aabbBuffer.resource);
    }
}

// Build geometry used in the sample.
void D3D12RaytracingProceduralGeometry::BuildGeometry()
{
    auto device = m_deviceResources->GetD3DDevice();
    
    BuildProceduralGeometryAABBs();

    // Plane indices.
    Index indices[] =
    {
        3,1,0,
        2,1,3,

    };

    // Cube vertices positions and corresponding triangle normals.
    // ToDo use scale transformation
    Vertex vertices[] =
    {
        { XMFLOAT3(-10.0f, -1.0f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(10.0f, -1.0f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(10.0f, -1.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(-10.0f, -1.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
    };

    AllocateUploadBuffer(device, indices, sizeof(indices), &m_indexBuffer.resource);
    AllocateUploadBuffer(device, vertices, sizeof(vertices), &m_vertexBuffer.resource);

    // Vertex buffer is passed to the shader along with index buffer as a descriptor range.
    UINT descriptorIndexIB = CreateBufferSRV(&m_indexBuffer, sizeof(indices) / 4, sizeof(UINT));
    UINT descriptorIndexVB = CreateBufferSRV(&m_vertexBuffer, ARRAYSIZE(vertices), sizeof(vertices[0]));
    ThrowIfFalse(descriptorIndexVB == descriptorIndexIB + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index");
}

// Build acceleration structures needed for raytracing.
void D3D12RaytracingProceduralGeometry::BuildBottomLevelGeometryDescs(D3D12_RAYTRACING_GEOMETRY_DESC geometryDescs[BottomLevelASType::Count])
{
    // Mark the geometry as opaque. 
    // Note: when rays encounter this geometry an any hit shader will not be executed whether it is present or not. 
    // It is recommended to use this flag liberally, as it can enable important ray processing optimizations.
    D3D12_RAYTRACING_GEOMETRY_FLAGS geometryFlags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

    // Triangle geometry desc
    {
        auto& geometryDesc = geometryDescs[BottomLevelASType::Triangle];
        geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
        geometryDesc.Triangles.IndexBuffer = m_indexBuffer.resource->GetGPUVirtualAddress();
        geometryDesc.Triangles.IndexCount = static_cast<UINT>(m_indexBuffer.resource->GetDesc().Width) / sizeof(Index);
        geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
        geometryDesc.Triangles.Transform = 0;
        geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        geometryDesc.Triangles.VertexCount = static_cast<UINT>(m_vertexBuffer.resource->GetDesc().Width) / sizeof(Vertex);
        geometryDesc.Triangles.VertexBuffer.StartAddress = m_vertexBuffer.resource->GetGPUVirtualAddress();
        geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
        geometryDesc.Flags = geometryFlags;
    }

    // AABB geometry desc
    {
        auto& geometryDesc = geometryDescs[BottomLevelASType::AABB];
        geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
        geometryDesc.AABBs.AABBCount = static_cast<UINT>(m_aabbBuffer.resource->GetDesc().Width) / sizeof(D3D12_RAYTRACING_AABB);
        geometryDesc.AABBs.AABBs.StartAddress = m_aabbBuffer.resource->GetGPUVirtualAddress();
        geometryDesc.AABBs.AABBs.StrideInBytes = sizeof(D3D12_RAYTRACING_AABB);
        geometryDesc.Flags = geometryFlags;
    }
}

AccelerationStructureBuffers D3D12RaytracingProceduralGeometry::BuildBottomLevelAS(const D3D12_RAYTRACING_GEOMETRY_DESC& geometryDesc, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags)
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();
    ComPtr<ID3D12Resource> scratch;
    ComPtr<ID3D12Resource> bottomLevelAS;

    // Get the size requirements for the scratch and AS buffers.
    D3D12_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_DESC prebuildInfoDesc = {};
    prebuildInfoDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    prebuildInfoDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    prebuildInfoDesc.Flags = buildFlags;
    prebuildInfoDesc.NumDescs = 1;
    prebuildInfoDesc.pGeometryDescs = &geometryDesc;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        m_fallbackDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfoDesc, &bottomLevelPrebuildInfo);
    }
    else // DirectX Raytracing
    {
        m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfoDesc, &bottomLevelPrebuildInfo);
    }
    ThrowIfFalse(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

    // Create a scratch buffer.
    AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ScratchDataSizeInBytes, &scratch, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");

    // Allocate resources for acceleration structures.
    // Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
    // Default heap is OK since the application doesn’t need CPU read/write access to them. 
    // The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
    // and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
    //  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
    //  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.
    {
        D3D12_RESOURCE_STATES initialResourceState;
        if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
        {
            initialResourceState = m_fallbackDevice->GetAccelerationStructureResourceState();
        }
        else // DirectX Raytracing
        {
            initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        }
        AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &bottomLevelAS, initialResourceState, L"BottomLevelAccelerationStructure");
    }

    // Bottom-level AS desc.
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
    {
        bottomLevelBuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        bottomLevelBuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        bottomLevelBuildDesc.Flags = buildFlags;
        bottomLevelBuildDesc.ScratchAccelerationStructureData = { scratch->GetGPUVirtualAddress(), scratch->GetDesc().Width };
        bottomLevelBuildDesc.DestAccelerationStructureData = { bottomLevelAS->GetGPUVirtualAddress(), bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes };
        bottomLevelBuildDesc.NumDescs = 1;
        bottomLevelBuildDesc.pGeometryDescs = &geometryDesc;
    }
    
    // Build the AS.
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        // Set the descriptor heaps to be used during acceleration structure build for the Fallback Layer.
        ID3D12DescriptorHeap *pDescriptorHeaps[] = { m_descriptorHeap.Get() };
        m_fallbackCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);
        m_fallbackCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc);
    }
    else // DirectX Raytracing
    {
        m_dxrCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc);
    }

    AccelerationStructureBuffers bottomLevelASBuffers;
    bottomLevelASBuffers.accelerationStructure = bottomLevelAS;
    bottomLevelASBuffers.scratch = scratch;
    bottomLevelASBuffers.ResultDataMaxSizeInBytes = bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes;
    return bottomLevelASBuffers;
}


// ToDo should the comptr be passed by value?
template <class InstanceDescType, class BLASPtrType>
void D3D12RaytracingProceduralGeometry::BuildBotomLevelASInstanceDescs(BLASPtrType *bottomLevelASaddresses, ComPtr<ID3D12Resource>* instanceDescsResource)
{
    auto device = m_deviceResources->GetD3DDevice();

    static const UINT NUM_X = 1;
    static const UINT NUM_Y = 1;
    static const UINT NUM_Z = 1;
    const UINT numTriangleDescs = 1;
    const UINT numAABBDescs = NUM_X * NUM_Y * NUM_Z;
    const UINT numInstanceDescs = numTriangleDescs + numAABBDescs;

    vector<InstanceDescType> instanceDescs;
    instanceDescs.resize(numInstanceDescs);

    // Bottom-level AS with a single plane.
    {
        auto& instanceDesc = instanceDescs[0];

        // Calculate transformation matrix.
        float planeWidth = 10.0f;
        const XMVECTOR vBasePosition = XMLoadFloat3(&XMFLOAT3(-planeWidth/2.0f, 0.0f, planeWidth /2.0f));
        XMMATRIX mScale = XMMatrixScaling(planeWidth, 1.0f, planeWidth);
        XMMATRIX mTranslation = XMMatrixTranslationFromVector(vBasePosition);
        XMMATRIX mTransform = mScale * mTranslation;                
        XMStoreFloat4x3(reinterpret_cast<XMFLOAT4X3*>(instanceDesc.Transform), mTransform);

        instanceDesc.InstanceMask = 1;
        instanceDesc.InstanceContributionToHitGroupIndex = BottomLevelASType::Triangle;
        instanceDesc.AccelerationStructure = bottomLevelASaddresses[BottomLevelASType::Triangle];
    }

    // Create instanced bottom-level AS with procedural geometry AABBs.
    // Instances share all the data, except for a transform.
    {
        // Width of a bottom-level AS geometry
        const XMFLOAT3 fWidth = XMFLOAT3(
            NUM_AABB_X * c_aabbWidth + (NUM_AABB_X - 1) * c_aabbDistance,
            NUM_AABB_Y * c_aabbWidth + (NUM_AABB_Y - 1) * c_aabbDistance,
            NUM_AABB_Z * c_aabbWidth + (NUM_AABB_Z - 1) * c_aabbDistance);
        const XMVECTOR vWidth = XMLoadFloat3(&fWidth);
        const XMVECTOR vStride = vWidth + XMLoadFloat3(&XMFLOAT3(c_aabbDistance, c_aabbDistance, c_aabbDistance));
        const XMVECTOR vBasePosition = XMLoadFloat3(&XMFLOAT3(
            -((NUM_X - 1) * (fWidth.x + c_aabbDistance) / 2.0f),
            1.0f,
            -((NUM_Z - 1) * (fWidth.z + c_aabbDistance) / 2.0f)));
        
        InstanceDescType instanceDescTemplate; 
        instanceDescTemplate.InstanceMask = 1;
        instanceDescTemplate.InstanceContributionToHitGroupIndex = BottomLevelASType::AABB;
        instanceDescTemplate.AccelerationStructure = bottomLevelASaddresses[BottomLevelASType::AABB];

        UINT blasIndex = 1;
        for (UINT x = 0; x < NUM_X; x++)
            for (UINT y = 0; y < NUM_Y; y++)
                for (UINT z = 0; z < NUM_Z; z++, blasIndex++)
                {
                    auto& instanceDesc = instanceDescs[blasIndex];
                    instanceDescTemplate.InstanceMask = 1;
                    instanceDescTemplate.InstanceContributionToHitGroupIndex = BottomLevelASType::AABB;
                    instanceDescTemplate.AccelerationStructure = bottomLevelASaddresses[BottomLevelASType::AABB];

                    XMVECTOR vIndex = XMLoadUInt3(&XMUINT3(x, y, z));
                    XMVECTOR vTranslation = vBasePosition + vIndex * vStride;
                    XMMATRIX mTranslation = XMMatrixTranslationFromVector(vBasePosition);
                    XMStoreFloat4x3(reinterpret_cast<XMFLOAT4X3*>(instanceDesc.Transform), mTranslation);
                }
    }
    UINT64 bufferSize = static_cast<UINT64>(instanceDescs.size() * sizeof(instanceDescs[0]));
    AllocateUploadBuffer(device, instanceDescs.data(), bufferSize, &(*instanceDescsResource), L"InstanceDescs");
};


AccelerationStructureBuffers D3D12RaytracingProceduralGeometry::BuildTopLevelAS(AccelerationStructureBuffers bottomLevelAS[BottomLevelASType::Count], D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags)
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();
    ComPtr<ID3D12Resource> scratch;
    ComPtr<ID3D12Resource> topLevelAS;

    // Get required sizes for an acceleration structure.
    D3D12_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_DESC prebuildInfoDesc = {};
    prebuildInfoDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    prebuildInfoDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    prebuildInfoDesc.Flags = buildFlags;
    prebuildInfoDesc.NumDescs = BottomLevelASType::Count;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        m_fallbackDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfoDesc, &topLevelPrebuildInfo);
    }
    else // DirectX Raytracing
    {
        m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfoDesc, &topLevelPrebuildInfo);
    }
    ThrowIfFalse(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

    AllocateUAVBuffer(device, topLevelPrebuildInfo.ScratchDataSizeInBytes, &scratch, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");

    // Allocate resources for acceleration structures.
    // Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
    // Default heap is OK since the application doesn’t need CPU read/write access to them. 
    // The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
    // and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
    //  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
    //  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.
    {
        D3D12_RESOURCE_STATES initialResourceState;
        if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
        {
            initialResourceState = m_fallbackDevice->GetAccelerationStructureResourceState();
        }
        else // DirectX Raytracing
        {
            initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        }

        AllocateUAVBuffer(device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &topLevelAS, initialResourceState, L"TopLevelAccelerationStructure");
    }

    // Note on Emulated GPU pointers (AKA Wrapped pointers) requirement in Fallback Layer:
    // The primary point of divergence between the DXR API and the compute-based Fallback layer is the handling of GPU pointers. 
    // DXR fundamentally requires that GPUs be able to dynamically read from arbitrary addresses in GPU memory. 
    // The existing Direct Compute API today is more rigid than DXR and requires apps to explicitly inform the GPU what blocks of memory it will access with SRVs/UAVs.
    // In order to handle the requirements of DXR, the Fallback Layer uses the concept of Emulated GPU pointers, 
    // which requires apps to create views around all memory they will access for raytracing, 
    // but retains the DXR-like flexibility of only needing to bind the top level acceleration structure at DispatchRays.
    //
    // The Fallback Layer interface uses WRAPPED_GPU_POINTER to encapsulate the underlying pointer
    // which will either be an emulated GPU pointer for the compute - based path or a GPU_VIRTUAL_ADDRESS for the DXR path.

    // Create an instance desc for the bottom-level acceleration structures.
    ComPtr<ID3D12Resource> instanceDescsResource;
    auto InitializeBotomLevelASInstanceDescs = [&](auto* bottomLevelASaddresses, auto* instanceDescs, auto instanceDescsSize)
    {
        for (UINT i = 0; i < BottomLevelASType::Count; i++)
        {
            auto& instanceDesc = instanceDescs[i];
            // Define identity matrix for the transformation.
            instanceDesc.Transform[0] = instanceDesc.Transform[5] = instanceDesc.Transform[10] = 1;
            instanceDesc.InstanceMask = 1;
            // ToDo describe indexing
            instanceDesc.InstanceContributionToHitGroupIndex = i;
            instanceDesc.AccelerationStructure = bottomLevelASaddresses[i];
        }
        AllocateUploadBuffer(device, instanceDescs, instanceDescsSize, &instanceDescsResource, L"InstanceDescs");
    };

    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC instanceDescs[BottomLevelASType::Count] = {};
        WRAPPED_GPU_POINTER bottomLevelASaddresses[BottomLevelASType::Count] = 
        {
            CreateFallbackWrappedPointer(bottomLevelAS[0].accelerationStructure.Get(), static_cast<UINT>(bottomLevelAS[0].ResultDataMaxSizeInBytes) / sizeof(UINT32)),
            CreateFallbackWrappedPointer(bottomLevelAS[1].accelerationStructure.Get(), static_cast<UINT>(bottomLevelAS[1].ResultDataMaxSizeInBytes) / sizeof(UINT32))
        };
        InitializeBotomLevelASInstanceDescs(bottomLevelASaddresses, instanceDescs, sizeof(instanceDescs));
        //BuildBotomLevelASInstanceDescs<D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC>(bottomLevelASaddresses, &instanceDescsResource);
    }
    else // DirectX Raytracing
    {
        D3D12_RAYTRACING_INSTANCE_DESC instanceDescs[BottomLevelASType::Count] = {};
        D3D12_GPU_VIRTUAL_ADDRESS bottomLevelASaddresses[BottomLevelASType::Count] =
        {
            bottomLevelAS[0].accelerationStructure->GetGPUVirtualAddress(),
            bottomLevelAS[1].accelerationStructure->GetGPUVirtualAddress()
        };
        InitializeBotomLevelASInstanceDescs(bottomLevelASaddresses, instanceDescs, sizeof(instanceDescs));
        //BuildBotomLevelASInstanceDescs<D3D12_RAYTRACING_INSTANCE_DESC>(bottomLevelASaddresses, &instanceDescsResource);
    }

    // Create a wrapped pointer to the acceleration structure.
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        UINT numBufferElements = static_cast<UINT>(topLevelPrebuildInfo.ResultDataMaxSizeInBytes) / sizeof(UINT32);
        m_fallbackTopLevelAccelerationStructurePointer = CreateFallbackWrappedPointer(topLevelAS.Get(), numBufferElements);
    }

    // Top-level AS desc
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
    {
        topLevelBuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
        topLevelBuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        topLevelBuildDesc.Flags = buildFlags;
        topLevelBuildDesc.DestAccelerationStructureData = { topLevelAS->GetGPUVirtualAddress(), topLevelPrebuildInfo.ResultDataMaxSizeInBytes };
        topLevelBuildDesc.NumDescs = BottomLevelASType::Count;
        topLevelBuildDesc.pGeometryDescs = nullptr;
        topLevelBuildDesc.InstanceDescs = instanceDescsResource->GetGPUVirtualAddress();
        topLevelBuildDesc.ScratchAccelerationStructureData = { scratch->GetGPUVirtualAddress(), scratch->GetDesc().Width };
    }

    // Build acceleration structure.
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        // Set the descriptor heaps to be used during acceleration structure build for the Fallback Layer.
        ID3D12DescriptorHeap *pDescriptorHeaps[] = { m_descriptorHeap.Get() };
        m_fallbackCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);
        m_fallbackCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc);
    }
    else // DirectX Raytracing
    {
        m_dxrCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc);
    }

    AccelerationStructureBuffers topLevelASBuffers;
    topLevelASBuffers.accelerationStructure = topLevelAS;
    topLevelASBuffers.instanceDesc = instanceDescsResource;
    topLevelASBuffers.scratch = scratch;
    topLevelASBuffers.ResultDataMaxSizeInBytes = topLevelPrebuildInfo.ResultDataMaxSizeInBytes;
    return topLevelASBuffers;
}

// Build acceleration structure needed for raytracing.
void D3D12RaytracingProceduralGeometry::BuildAccelerationStructures()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();
    auto commandQueue = m_deviceResources->GetCommandQueue();
    auto commandAllocator = m_deviceResources->GetCommandAllocator();

    // Reset the command list for the acceleration structure construction.
    commandList->Reset(commandAllocator, nullptr);

    // Build bottom-level AS.
    AccelerationStructureBuffers bottomLevelAS[BottomLevelASType::Count];
    {
        D3D12_RAYTRACING_GEOMETRY_DESC geometryDescs[BottomLevelASType::Count];
        BuildBottomLevelGeometryDescs(geometryDescs);

        // Build all bottom-level AS.
        for (UINT i = 0; i < BottomLevelASType::Count; i++)
        {
            bottomLevelAS[i] = BuildBottomLevelAS(geometryDescs[i]);
        }
    }

    // Batch all resource barriers for bottom-level AS builds.
    D3D12_RESOURCE_BARRIER resourceBarriers[BottomLevelASType::Count];
    for (UINT i = 0; i < BottomLevelASType::Count; i++)
    {
        resourceBarriers[i] = CD3DX12_RESOURCE_BARRIER::UAV(bottomLevelAS[i].accelerationStructure.Get());
    }
    commandList->ResourceBarrier(BottomLevelASType::Count, resourceBarriers);

    // Build top-level AS.
    AccelerationStructureBuffers topLevelAS = BuildTopLevelAS(bottomLevelAS);
    
    // Kick off acceleration structure construction.
    m_deviceResources->ExecuteCommandList();

    // Wait for GPU to finish as the locally created temporary GPU resources will get released once we go out of scope.
    m_deviceResources->WaitForGpu();

    // Store the AS buffers. The rest of the buffers will be released once we exit the function.
    for (UINT i = 0; i < BottomLevelASType::Count; i++)
    {
        m_bottomLevelAS[i] = bottomLevelAS[i].accelerationStructure;
    }
    m_topLevelAS = topLevelAS.accelerationStructure;
}

// Build shader tables.
// This encapsulates all shader records - shaders and the arguments for their local root signatures.
void D3D12RaytracingProceduralGeometry::BuildShaderTables()
{
    auto device = m_deviceResources->GetD3DDevice();

    void* rayGenShaderIdentifier;
    void* missShaderIdentifiers[RayType::Count];
    void* hitGroupShaderIdentifiers[HitGroupType::Count];

    auto GetShaderIdentifiers = [&](auto* stateObjectProperties)
    {
        rayGenShaderIdentifier = stateObjectProperties->GetShaderIdentifier(c_raygenShaderName);
        for (UINT i = 0; i < RayType::Count; i++)
        {
            missShaderIdentifiers[i] = stateObjectProperties->GetShaderIdentifier(c_missShaderNames[i]);
        }
        for (UINT i = 0; i < HitGroupType::Count; i++)
        {
            hitGroupShaderIdentifiers[i] = stateObjectProperties->GetShaderIdentifier(c_hitGroupNames[i]);
        }
    };

    // Get shader identifiers.
    UINT shaderIdentifierSize;
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        GetShaderIdentifiers(m_fallbackStateObject.Get());
        shaderIdentifierSize = m_fallbackDevice->GetShaderIdentifierSize();
    }
    else // DirectX Raytracing
    {
        ComPtr<ID3D12StateObjectPropertiesPrototype> stateObjectProperties;
        ThrowIfFailed(m_dxrStateObject.As(&stateObjectProperties));
        GetShaderIdentifiers(stateObjectProperties.Get());
        shaderIdentifierSize = m_dxrDevice->GetShaderIdentifierSize();
    }

    // Initialize shader tables.
    // Shader table = {{ ShaderRecord 1}, {ShaderRecord 2}, ...}
    // Shader record = {{ Shader ID }, { RootArguments }}

    // RayGen shader table.
    {
        ShaderTable rayGenShaderTable;
        rayGenShaderTable.push_back(ShaderRecord(rayGenShaderIdentifier, shaderIdentifierSize, nullptr, 0));
        rayGenShaderTable.AllocateAsUploadBuffer(device, &m_rayGenShaderTable, L"RayGenShaderTable");
    }
    
    // Miss shader table.
    {
        ShaderTable missShaderTable;
        for (UINT i = 0; i < RayType::Count; i++)
        {
            missShaderTable.push_back(ShaderRecord(missShaderIdentifiers[i], shaderIdentifierSize, nullptr, 0));
        }
        missShaderTable.AllocateAsUploadBuffer(device, &m_missShaderTable, L"MissShaderTable");
        m_missShaderTableStrideInBytes = missShaderTable.GetMaxShaderRecordSize();
    }

    // Hit group shader table.
    // Only hit group shader record requires root arguments initialization for the closest hit shader's cube CB access.
    {
        // ToDo Rename to plane
        static_assert(LocalRootSignatureParams::CubeConstantSlot == 0 && LocalRootSignatureParams::Count == 1, "Checking the local root signature parameters definition here.");
        struct RootArguments {
            CubeConstantBuffer cb;
        } rootArguments;
        rootArguments.cb = m_cubeCB;

        ShaderTable hitGroupShaderTable;
        // ToDo describe layout
        for (UINT i = 0; i < HitGroupType::Count; i++)
        {
            hitGroupShaderTable.push_back(ShaderRecord(hitGroupShaderIdentifiers[i], shaderIdentifierSize, &rootArguments, sizeof(rootArguments)));
        }
        hitGroupShaderTable.AllocateAsUploadBuffer(device, &m_hitGroupShaderTable, L"HitGroupShaderTable");
        m_hitGroupShaderTableStrideInBytes = hitGroupShaderTable.GetMaxShaderRecordSize();
    }
}

void D3D12RaytracingProceduralGeometry::SelectRaytracingAPI(RaytracingAPI type)
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

void D3D12RaytracingProceduralGeometry::OnKeyDown(UINT8 key)
{
    // Store previous values.
    RaytracingAPI previousRaytracingAPI = m_raytracingAPI;
    bool previousForceComputeFallback = m_forceComputeFallback;

    switch (key)
    {
    case '1': // Fallback Layer
        m_forceComputeFallback = false;
        SelectRaytracingAPI(RaytracingAPI::FallbackLayer);
        break;
    case '2': // Fallback Layer + force compute path
        m_forceComputeFallback = true;
        SelectRaytracingAPI(RaytracingAPI::FallbackLayer);
        break;
    case '3': // DirectX Raytracing
        SelectRaytracingAPI(RaytracingAPI::DirectXRaytracing);
        break;
    default:
        break;
    }

    if (m_raytracingAPI != previousRaytracingAPI ||
        m_forceComputeFallback != previousForceComputeFallback)
    {
        // Raytracing API selection changed, recreate everything.
        RecreateD3D();
    }
}

// Update frame-based values.
void D3D12RaytracingProceduralGeometry::OnUpdate()
{
    m_timer.Tick();
    CalculateFrameStats();
    float elapsedTime = static_cast<float>(m_timer.GetElapsedSeconds());
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();
    auto prevFrameIndex = m_deviceResources->GetPreviousFrameIndex();

    // Rotate the camera around Y axis.
    {
        float secondsToRotateAround = 24.0f;
        float angleToRotateBy = 360.0f * (elapsedTime / secondsToRotateAround);
        XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(angleToRotateBy));
        m_eye = XMVector3Transform(m_eye, rotate);
        m_up = XMVector3Transform(m_up, rotate);
        m_at = XMVector3Transform(m_at, rotate);
        UpdateCameraMatrices();
    }

    // Rotate the second light around Y axis.
    {
        float secondsToRotateAround = 8.0f;
        float angleToRotateBy = -360.0f * (elapsedTime / secondsToRotateAround);
        XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(angleToRotateBy));
        const XMVECTOR& prevLightPosition = m_sceneCB[prevFrameIndex].lightPosition;
        m_sceneCB[frameIndex].lightPosition = XMVector3Transform(prevLightPosition, rotate);
    }

    UpdateAABBPrimitiveAttributes();
}


// Parse supplied command line args.
void D3D12RaytracingProceduralGeometry::ParseCommandLineArgs(WCHAR* argv[], int argc)
{
    if (argc > 1)
    {
        if (_wcsnicmp(argv[1], L"-FL", wcslen(argv[1])) == 0)
        {
            m_forceComputeFallback = true;
            SelectRaytracingAPI(RaytracingAPI::FallbackLayer);
        }
        else if (_wcsnicmp(argv[1], L"-DXR", wcslen(argv[1])) == 0)
        {
            SelectRaytracingAPI(RaytracingAPI::DirectXRaytracing);
        }
    }
}

void D3D12RaytracingProceduralGeometry::DoRaytracing()
{
    auto commandList = m_deviceResources->GetCommandList();
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    auto DispatchRays = [&](auto* commandList, auto* stateObject, auto* dispatchDesc)
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
        commandList->DispatchRays(stateObject, dispatchDesc);
    };

    auto SetCommonPipelineState = [&](auto* descriptorSetCommandList)
    {
        descriptorSetCommandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());
        // Set index and successive vertex buffer decriptor tables
        commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::VertexBuffersSlot, m_indexBuffer.gpuDescriptorHandle);
        commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::OutputViewSlot, m_raytracingOutputResourceUAVGpuDescriptor);
    };

    commandList->SetComputeRootSignature(m_raytracingGlobalRootSignature.Get());

    // Copy the updated scene constant buffer to GPU.
    {
        memcpy(&m_mappedConstantData[frameIndex].constants, &m_sceneCB[frameIndex], sizeof(m_sceneCB[frameIndex]));
        auto cbGpuAddress = m_perFrameConstants->GetGPUVirtualAddress() + frameIndex * sizeof(m_mappedConstantData[0]);
        commandList->SetComputeRootConstantBufferView(GlobalRootSignatureParams::SceneConstantSlot, cbGpuAddress);
    }

    // Copy the updated AABB primitive attribute buffer to GPU.
    {
        // ToDo use per frame buffers
        UINT frameIndex = 0;
        memcpy(&m_mappedAABBPrimitiveAttributes[frameIndex], &m_aabbPrimitiveAttributeBuffer[frameIndex][0][0][0], AABB_BUFFER_SIZE);
        auto bufferGpuAddress = m_perFrameAABBPrimitiveAttributes->GetGPUVirtualAddress() + frameIndex * AABB_BUFFER_SIZE;
        commandList->SetComputeRootShaderResourceView(GlobalRootSignatureParams::AABBattributeBufferSlot, bufferGpuAddress);
    }

    // Bind the heaps, acceleration structure and dispatch rays.    
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        D3D12_FALLBACK_DISPATCH_RAYS_DESC dispatchDesc = {};
        SetCommonPipelineState(m_fallbackCommandList.Get());
        m_fallbackCommandList->SetTopLevelAccelerationStructure(GlobalRootSignatureParams::AccelerationStructureSlot, m_fallbackTopLevelAccelerationStructurePointer);
        DispatchRays(m_fallbackCommandList.Get(), m_fallbackStateObject.Get(), &dispatchDesc);
    }
    else // DirectX Raytracing
    {
        D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
        SetCommonPipelineState(commandList);
        commandList->SetComputeRootShaderResourceView(GlobalRootSignatureParams::AccelerationStructureSlot, m_topLevelAS->GetGPUVirtualAddress());
        DispatchRays(m_dxrCommandList.Get(), m_dxrStateObject.Get(), &dispatchDesc);
    }
}

// Update the application state with the new resolution.
void D3D12RaytracingProceduralGeometry::UpdateForSizeChange(UINT width, UINT height)
{
    DXSample::UpdateForSizeChange(width, height);
}

// Copy the raytracing output to the backbuffer.
void D3D12RaytracingProceduralGeometry::CopyRaytracingOutputToBackbuffer()
{
    auto commandList = m_deviceResources->GetCommandList();
    auto renderTarget = m_deviceResources->GetRenderTarget();

    D3D12_RESOURCE_BARRIER preCopyBarriers[2];
    preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
    preCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_raytracingOutput.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
    commandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);

    commandList->CopyResource(renderTarget, m_raytracingOutput.Get());

    D3D12_RESOURCE_BARRIER postCopyBarriers[2];
    postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
    postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_raytracingOutput.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    commandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
}

// Create resources that are dependent on the size of the main window.
void D3D12RaytracingProceduralGeometry::CreateWindowSizeDependentResources()
{
    CreateRaytracingOutputResource();
    UpdateCameraMatrices();
}

// Release resources that are dependent on the size of the main window.
void D3D12RaytracingProceduralGeometry::ReleaseWindowSizeDependentResources()
{
    m_raytracingOutput.Reset();
}

// Release all resources that depend on the device.
void D3D12RaytracingProceduralGeometry::ReleaseDeviceDependentResources()
{
    m_fallbackDevice.Reset();
    m_fallbackCommandList.Reset();
    m_fallbackStateObject.Reset();
    m_raytracingGlobalRootSignature.Reset();
    m_raytracingLocalRootSignature.Reset();

    m_dxrDevice.Reset();
    m_dxrCommandList.Reset();
    m_dxrStateObject.Reset();

    m_descriptorHeap.Reset();
    m_descriptorsAllocated = 0;
    m_raytracingOutputResourceUAVDescriptorHeapIndex = UINT_MAX;
    m_indexBuffer.resource.Reset();
    m_vertexBuffer.resource.Reset();
    m_perFrameConstants.Reset();
    m_rayGenShaderTable.Reset();
    m_missShaderTable.Reset();
    m_hitGroupShaderTable.Reset();

    ResetComPtrArray(&m_bottomLevelAS);
    m_topLevelAS.Reset();
}

void D3D12RaytracingProceduralGeometry::RecreateD3D()
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
void D3D12RaytracingProceduralGeometry::OnRender()
{
    if (!m_deviceResources->IsWindowVisible())
    {
        return;
    }

    m_deviceResources->Prepare();

    DoRaytracing();

    CopyRaytracingOutputToBackbuffer();

    m_deviceResources->Present(D3D12_RESOURCE_STATE_PRESENT);
}

void D3D12RaytracingProceduralGeometry::OnDestroy()
{
    OnDeviceLost();
}

// Release all device dependent resouces when a device is lost.
void D3D12RaytracingProceduralGeometry::OnDeviceLost()
{
    ReleaseWindowSizeDependentResources();
    ReleaseDeviceDependentResources();
}

// Create all device dependent resources when a device is restored.
void D3D12RaytracingProceduralGeometry::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

// Compute the average frames per second and million rays per second.
void D3D12RaytracingProceduralGeometry::CalculateFrameStats()
{
    static int frameCnt = 0;
    static double elapsedTime = 0.0f;
    double totalTime = m_timer.GetTotalSeconds();
    frameCnt++;

    // Compute averages over one second period.
    if ((totalTime - elapsedTime) >= 1.0f)
    {
        float diff = static_cast<float>(totalTime - elapsedTime);
        float fps = static_cast<float>(frameCnt) / diff; // Normalize to an exact second.

        frameCnt = 0;
        elapsedTime = totalTime;

        float MRaysPerSecond = (m_width * m_height * fps) / static_cast<float>(1e6);

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
            << L"    fps: " << fps << L"     ~Million Primary Rays/s: " << MRaysPerSecond;
        SetCustomWindowText(windowText.str().c_str());
    }
}

// Handle OnSizeChanged message event.
void D3D12RaytracingProceduralGeometry::OnSizeChanged(UINT width, UINT height, bool minimized)
{
    if (!m_deviceResources->WindowSizeChanged(width, height, minimized))
    {
        return;
    }

    UpdateForSizeChange(width, height);

    ReleaseWindowSizeDependentResources();
    CreateWindowSizeDependentResources();
}

// Create a wrapped pointer for the Fallback Layer path.
WRAPPED_GPU_POINTER D3D12RaytracingProceduralGeometry::CreateFallbackWrappedPointer(ID3D12Resource* resource, UINT bufferNumElements)
{
    auto device = m_deviceResources->GetD3DDevice();

    D3D12_UNORDERED_ACCESS_VIEW_DESC rawBufferUavDesc = {};
    rawBufferUavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    rawBufferUavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
    rawBufferUavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    rawBufferUavDesc.Buffer.NumElements = bufferNumElements;

    D3D12_CPU_DESCRIPTOR_HANDLE bottomLevelDescriptor;

    // Only compute fallback requires a valid descriptor index when creating a wrapped pointer.
    UINT descriptorHeapIndex = 0;
    if (!m_fallbackDevice->UsingRaytracingDriver())
    {
        descriptorHeapIndex = AllocateDescriptor(&bottomLevelDescriptor);
        device->CreateUnorderedAccessView(resource, nullptr, &rawBufferUavDesc, bottomLevelDescriptor);
    }
    return m_fallbackDevice->GetWrappedPointerSimple(descriptorHeapIndex, resource->GetGPUVirtualAddress());
}

// Allocate a descriptor and return its index. 
// If the passed descriptorIndexToUse is valid, it will be used instead of allocating a new one.
UINT D3D12RaytracingProceduralGeometry::AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse)
{
    auto descriptorHeapCpuBase = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    if (descriptorIndexToUse >= m_descriptorHeap->GetDesc().NumDescriptors)
    {
        descriptorIndexToUse = m_descriptorsAllocated++;
    }
    *cpuDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeapCpuBase, descriptorIndexToUse, m_descriptorSize);
    return descriptorIndexToUse;
}

// Create SRV for a buffer.
UINT D3D12RaytracingProceduralGeometry::CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize)
{
    auto device = m_deviceResources->GetD3DDevice();

    // SRV
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    srvDesc.Buffer.NumElements = numElements;
    srvDesc.Buffer.StructureByteStride = elementSize;
    UINT descriptorIndex = AllocateDescriptor(&buffer->cpuDescriptorHandle);
    device->CreateShaderResourceView(buffer->resource.Get(), &srvDesc, buffer->cpuDescriptorHandle);
    buffer->gpuDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), descriptorIndex, m_descriptorSize);
    return descriptorIndex;
};