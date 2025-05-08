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
#include "D3D12RaytracingTree.h"
#include "DirectXRaytracingHelper.h"
#include "CompiledShaders\Raytracing.hlsl.h"
#include "DDSTextureLoader.h"

#include "SharedCode.h"


using namespace std;
using namespace DX;

const wchar_t* D3D12RaytracingTree::c_hitGroupName = L"MyHitGroup";
const wchar_t* D3D12RaytracingTree::c_raygenShaderName = L"MyRaygenShader";
const wchar_t* D3D12RaytracingTree::c_closestHitShaderName = L"MyClosestHitShader";
const wchar_t* D3D12RaytracingTree::c_anyHitShaderName = L"MyAnyHitShader";
const wchar_t* D3D12RaytracingTree::c_missShaderName = L"MyMissShader";

D3D12RaytracingTree::D3D12RaytracingTree(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_curRotationAngleRad(0.0f)
{
    UpdateForSizeChange(width, height);
}

void D3D12RaytracingTree::OnInit()
{
    m_deviceResources = std::make_unique<DeviceResources>(
        DXGI_FORMAT_R10G10B10A2_UNORM,
        DXGI_FORMAT_UNKNOWN,
        FrameCount,
        D3D_FEATURE_LEVEL_11_0,
        DeviceResources::c_RequireTearingSupport,
        m_adapterIDoverride
        );

    m_deviceResources->RegisterDeviceNotify(this);
    m_deviceResources->SetWindow(Win32Application::GetHwnd(), m_width, m_height);
    m_deviceResources->InitializeDXGIAdapter();

    ThrowIfFalse(IsDirectXRaytracingSupported(m_deviceResources->GetAdapter(), D3D12_RAYTRACING_TIER_1_2),
        L"ERROR: DirectX Raytracing Tier 1.2 is not supported by your OS, GPU and/or driver.\n\n");

    m_deviceResources->CreateDeviceResources();
    m_deviceResources->CreateWindowSizeDependentResources();

    InitializeScene();

    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

// Update camera matrices passed into the shader.
void D3D12RaytracingTree::UpdateCameraMatrices()
{
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    m_sceneCB[frameIndex].cameraPosition = m_eye;
    float fovAngleY = 40.0f;
    XMMATRIX view = XMMatrixLookAtLH(m_eye, m_at, m_up);
    XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), m_aspectRatio, 1.0f, 1000.0f);
    XMMATRIX viewProj = view * proj;

    m_sceneCB[frameIndex].projectionToWorld = XMMatrixInverse(nullptr, viewProj);
}

// Initialize scene rendering parameters.
void D3D12RaytracingTree::InitializeScene()
{
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    // Setup materials.
    {
        m_treeCB.albedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    // Setup camera.
    {
        // Initialize the view and projection inverse matrices.
        m_eye = { 0.0f, 7.0f, -24.0f, 1.0f };
        m_at = { 0.0f, 8.7f, 0.0f, 1.0f };
        XMVECTOR right = { 1.0f, 0.0f, 0.0f, 0.0f };

        XMVECTOR direction = XMVector4Normalize(m_at - m_eye);
        m_up = XMVector3Normalize(XMVector3Cross(direction, right));

        // Rotate camera around Y axis.
        XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(45.0f));
        m_eye = XMVector3Transform(m_eye, rotate);
        m_up = XMVector3Transform(m_up, rotate);
        
        UpdateCameraMatrices();
    }

    // Apply the initial values to all frames' buffer instances.
    for (auto& sceneCB : m_sceneCB)
    {
        sceneCB = m_sceneCB[frameIndex];
    }
}

// Create constant buffers.
void D3D12RaytracingTree::CreateConstantBuffers()
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

    // Create two SRVs, one for each frame at the respective offsets
    for (UINT n = 0; n < frameCount; n++)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        cbvSrvHandle.ptr += (Descriptors::SCENE_CBV_0 + n) * m_descriptorSize;

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = m_perFrameConstants->GetGPUVirtualAddress() + n * sizeof(AlignedSceneConstantBuffer);
        cbvDesc.SizeInBytes = sizeof(AlignedSceneConstantBuffer);
        device->CreateConstantBufferView(&cbvDesc, cbvSrvHandle);
    }
}

// Create resources that depend on the device.
void D3D12RaytracingTree::CreateDeviceDependentResources()
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

    // Load a model and build raytracing acceleration structures from the generated geometry.
    LoadAndBuildAccelerationStructures();

    // Create constant buffers for the geometry and the scene.
    CreateConstantBuffers();

    // Build shader tables, which define shaders and their local root arguments.
    BuildShaderTables();

    // Create an output 2D texture to store the raytracing result to.
    CreateRaytracingOutputResource();
}

void D3D12RaytracingTree::LoadTexture(const wchar_t* path, ID3D12Resource** resource, ID3D12Resource** uploadResource)
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();

    std::unique_ptr<uint8_t[]> ddsData;
    std::vector<D3D12_SUBRESOURCE_DATA> subresouceData;
    HRESULT hr = LoadDDSTextureFromFile(device, path, resource, ddsData, subresouceData);
	if (FAILED(hr))
	{
		printf("Failed to load texture %ls\n", path);
		exit(1);
	}

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(*resource, 0, static_cast<UINT>(subresouceData.size()));

	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	ThrowIfFailed(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadResource)));

	UpdateSubresources(commandList, *resource, *uploadResource, 0, 0, static_cast<UINT>(subresouceData.size()), subresouceData.data());
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(*resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void D3D12RaytracingTree::LoadTextures()
{
    auto device = m_deviceResources->GetD3DDevice();

    const wchar_t* texturesToLoad[3] =
    {
        L"jacaranda_tree_leaves_alpha_4k.dds",
        L"jacaranda_tree_leaves_diff_4k.dds",
        L"jacaranda_tree_trunk_diff_4k.dds"
    };

    for (int i = 0; i < ARRAYSIZE(texturesToLoad); i++)
    {
        LoadTexture(texturesToLoad[i], &m_textureDefault[i], &m_textureUpload[i]);

        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        srvHandle.ptr += ((Descriptors::MODEL_TEXTURES_START + i) * m_descriptorSize);

        device->CreateShaderResourceView(m_textureDefault[i].Get(), nullptr, srvHandle);
    }
}

void D3D12RaytracingTree::LoadModel(const char* modelPath, const char* ommPath)
{
    HANDLE fh = CreateFileA(modelPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE fhOMM = CreateFileA(ommPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (fh == INVALID_HANDLE_VALUE || fhOMM == INVALID_HANDLE_VALUE)
    {
        printf("Failed to open model file(s)\n");
        exit(1);
    }

    auto device = m_deviceResources->GetD3DDevice();

    DWORD bytesRead;

    UINT ommCounts[5];
    ReadFile(fhOMM, ommCounts, sizeof(ommCounts), &bytesRead, NULL);

    UINT arrayDataSize = ommCounts[0];
    UINT descArrayCount = ommCounts[1];
    m_numHistogramEntries = ommCounts[2];
    UINT indexFormat = ommCounts[3];
    UINT indexCount = ommCounts[4];

    size_t descArraySize = descArrayCount * sizeof(D3D12_RAYTRACING_OPACITY_MICROMAP_DESC);
    size_t ommIndexBufferSize = indexCount * (indexFormat == 0 ? 2 : 4);

    AllocateUploadableBuffer(device, nullptr, descArraySize,
        &m_ommDescBuffer.uploadResource, &m_ommDescBuffer.defaultResource, L"OMM Desc Buffer");
    AllocateUploadableBuffer(device, nullptr, arrayDataSize,
        &m_ommArrayBuffer.uploadResource, &m_ommArrayBuffer.defaultResource, L"OMM Array Buffer");
    AllocateUploadableBuffer(device, nullptr, ommIndexBufferSize,
        &m_ommIndexBuffer.uploadResource, &m_ommIndexBuffer.defaultResource, L"OMM Index Buffer");

    D3D12_RAYTRACING_OPACITY_MICROMAP_DESC* descArrayPtr;
    void* ommArrayPtr;
    UINT* ommIndexBufferPtr;
    m_ommDescBuffer.uploadResource->Map(0, nullptr, (void**)&descArrayPtr);
    m_ommArrayBuffer.uploadResource->Map(0, nullptr, &ommArrayPtr);
    m_ommIndexBuffer.uploadResource->Map(0, nullptr, (void**)&ommIndexBufferPtr);

    size_t histogramSize = m_numHistogramEntries * sizeof(D3D12_RAYTRACING_OPACITY_MICROMAP_HISTOGRAM_ENTRY);
    m_ommHistogramBuffer = new D3D12_RAYTRACING_OPACITY_MICROMAP_HISTOGRAM_ENTRY[m_numHistogramEntries];

    ReadFile(fhOMM, descArrayPtr, descArraySize, &bytesRead, NULL);
    ReadFile(fhOMM, ommArrayPtr, arrayDataSize, &bytesRead, NULL);
    ReadFile(fhOMM, m_ommHistogramBuffer, histogramSize, &bytesRead, NULL);
    ReadFile(fhOMM, ommIndexBufferPtr, ommIndexBufferSize, &bytesRead, NULL);

    CloseHandle(fhOMM);


    UINT counts[3];
    ReadFile(fh, counts, sizeof(counts), &bytesRead, NULL);

    UINT positionCount = counts[0];
    UINT normalCount = counts[1];
    UINT texcoordCount = counts[2];

    AllocateUploadableBuffer(device, nullptr, positionCount * sizeof(XMFLOAT3), 
        &m_positionBuffer.uploadResource, &m_positionBuffer.defaultResource, L"Position Buffer");
    AllocateUploadableBuffer(device, nullptr, normalCount * sizeof(XMFLOAT3),
        &m_normalBuffer.uploadResource, &m_normalBuffer.defaultResource, L"Normal Buffer");
    AllocateUploadableBuffer(device, nullptr, texcoordCount * sizeof(XMFLOAT3),
        &m_texCoordBuffer.uploadResource, &m_texCoordBuffer.defaultResource, L"TexCoord Buffer");

    void* positionData = nullptr;
    void* normalData = nullptr;
    void* texCoordData = nullptr;

    m_positionBuffer.uploadResource->Map(0, nullptr, &positionData);
    m_normalBuffer.uploadResource->Map(0, nullptr, &normalData);
    m_texCoordBuffer.uploadResource->Map(0, nullptr, &texCoordData);

    ReadFile(fh, positionData, positionCount * sizeof(XMFLOAT3), &bytesRead, NULL);
    ReadFile(fh, normalData, normalCount * sizeof(XMFLOAT3), &bytesRead, NULL);
    ReadFile(fh, texCoordData, texcoordCount * sizeof(XMFLOAT2), &bytesRead, NULL);

    ReadFile(fh, &m_numGeoms, sizeof(m_numGeoms), &bytesRead, NULL);

    m_indicesPerGeom = new UINT[m_numGeoms];
    ReadFile(fh, m_indicesPerGeom, m_numGeoms * sizeof(UINT), &bytesRead, NULL);

    UINT totalIndices;
    ReadFile(fh, &totalIndices, sizeof(totalIndices), &bytesRead, NULL);

    size_t indexBufferSize = totalIndices * sizeof(UINT);
    AllocateUploadableBuffer(device, nullptr, indexBufferSize,
        &m_positionIndexBuffer.uploadResource, &m_positionIndexBuffer.defaultResource, L"Position Index Buffer");
    AllocateUploadableBuffer(device, nullptr, indexBufferSize,
        &m_normalIndexBuffer.uploadResource, &m_normalIndexBuffer.defaultResource, L"Normal Index Buffer");
    AllocateUploadableBuffer(device, nullptr, indexBufferSize,
        &m_texCoordIndexBuffer.uploadResource, &m_texCoordIndexBuffer.defaultResource, L"TexCoord Index Buffer");

    void* positionIndexData = nullptr;
    void* normalIndexData = nullptr;
    void* texCoordIndexData = nullptr;

    m_positionIndexBuffer.uploadResource->Map(0, nullptr, &positionIndexData);
    m_normalIndexBuffer.uploadResource->Map(0, nullptr, &normalIndexData);
    m_texCoordIndexBuffer.uploadResource->Map(0, nullptr, &texCoordIndexData);

    ReadFile(fh, positionIndexData, indexBufferSize, &bytesRead, NULL);
    ReadFile(fh, normalIndexData, indexBufferSize, &bytesRead, NULL);
    ReadFile(fh, texCoordIndexData, indexBufferSize, &bytesRead, NULL);

    CloseHandle(fh);

	// Create a buffer to hold each geometry's offset in the index buffer
    {
		D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(m_numGeoms * sizeof(GeometryInfo));
		ThrowIfFailed(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_geometryOffsetBuffer)));

        GeometryInfo* geomInfos = nullptr;
		m_geometryOffsetBuffer->Map(0, nullptr, (void**)&geomInfos);
		
        UINT primOffset = 0;

		for (UINT i = 0; i < m_numGeoms; i++)
		{
            // TODO. the indices, do it properly
            geomInfos[i].primitiveOffset = primOffset;
			geomInfos[i].diffuseTextureIndex = (i == 0 || i == 6) ? 2 : 1;
			geomInfos[i].alphaTextureIndex = (i == 0 || i == 6) ? 0xFFFFFFFF : 0;

            primOffset += (m_indicesPerGeom[i] / 3);
		}

		D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        srvHandle.ptr += (Descriptors::GEOMETRY_INFO_BUFFER) * m_descriptorSize;
		
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Buffer.NumElements = m_numGeoms;
		srvDesc.Buffer.StructureByteStride = sizeof(GeometryInfo);

		device->CreateShaderResourceView(m_geometryOffsetBuffer.Get(), &srvDesc, srvHandle);
    }

    {
        // Create an SRV for the 6 buffers
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

        ID3D12Resource* bufferResources[] = {
            m_positionBuffer.defaultResource.Get(),
            m_normalBuffer.defaultResource.Get(),
            m_texCoordBuffer.defaultResource.Get(),
            m_positionIndexBuffer.defaultResource.Get(),
            m_normalIndexBuffer.defaultResource.Get(),
            m_texCoordIndexBuffer.defaultResource.Get()
        };

		UINT bufferSizes[] = {
			positionCount * sizeof(XMFLOAT3),
			normalCount * sizeof(XMFLOAT3),
			texcoordCount * sizeof(XMFLOAT2),
			totalIndices * sizeof(UINT),
			totalIndices * sizeof(UINT),
			totalIndices * sizeof(UINT)
		};

		for (UINT i = 0; i < ARRAYSIZE(bufferResources); i++)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
			srvHandle.ptr += (Descriptors::POSITION_BUFFER + i) * m_descriptorSize;

			srvDesc.Buffer.NumElements = bufferSizes[i] / 4;
			srvDesc.Buffer.StructureByteStride = 0;
			srvDesc.Buffer.FirstElement = 0;

			device->CreateShaderResourceView(bufferResources[i], &srvDesc, srvHandle);
		}
    }
}

void D3D12RaytracingTree::BuildAccelerationStructures()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();

    // Copy all the uploadable buffers to default memory if they haven't been already
    static bool uploadedUploadableBuffers = false;

    if (!uploadedUploadableBuffers)
    {
        commandList->CopyResource(m_positionBuffer.defaultResource.Get(), m_positionBuffer.uploadResource.Get());
        commandList->CopyResource(m_normalBuffer.defaultResource.Get(), m_normalBuffer.uploadResource.Get());
        commandList->CopyResource(m_texCoordBuffer.defaultResource.Get(), m_texCoordBuffer.uploadResource.Get());
        commandList->CopyResource(m_positionIndexBuffer.defaultResource.Get(), m_positionIndexBuffer.uploadResource.Get());
        commandList->CopyResource(m_normalIndexBuffer.defaultResource.Get(), m_normalIndexBuffer.uploadResource.Get());
        commandList->CopyResource(m_texCoordIndexBuffer.defaultResource.Get(), m_texCoordIndexBuffer.uploadResource.Get());

        commandList->CopyResource(m_ommDescBuffer.defaultResource.Get(), m_ommDescBuffer.uploadResource.Get());
        commandList->CopyResource(m_ommArrayBuffer.defaultResource.Get(), m_ommArrayBuffer.uploadResource.Get());
        commandList->CopyResource(m_ommIndexBuffer.defaultResource.Get(), m_ommIndexBuffer.uploadResource.Get());

        // Transition all default resources from COPY_DEST to GENERIC_READ
        D3D12_RESOURCE_BARRIER barrier[9];
        barrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_positionBuffer.defaultResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
        barrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_normalBuffer.defaultResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
        barrier[2] = CD3DX12_RESOURCE_BARRIER::Transition(m_texCoordBuffer.defaultResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
        barrier[3] = CD3DX12_RESOURCE_BARRIER::Transition(m_positionIndexBuffer.defaultResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
        barrier[4] = CD3DX12_RESOURCE_BARRIER::Transition(m_normalIndexBuffer.defaultResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
        barrier[5] = CD3DX12_RESOURCE_BARRIER::Transition(m_texCoordIndexBuffer.defaultResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
        barrier[6] = CD3DX12_RESOURCE_BARRIER::Transition(m_ommDescBuffer.defaultResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
        barrier[7] = CD3DX12_RESOURCE_BARRIER::Transition(m_ommArrayBuffer.defaultResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
        barrier[8] = CD3DX12_RESOURCE_BARRIER::Transition(m_ommIndexBuffer.defaultResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

        commandList->ResourceBarrier(ARRAYSIZE(barrier), barrier);

        uploadedUploadableBuffers = true;
    }

    // Build a BLAS
    UINT vertexCount = m_positionBuffer.defaultResource->GetDesc().Width / sizeof(XMFLOAT3);

	auto linkageDescs = (D3D12_RAYTRACING_GEOMETRY_OMM_LINKAGE_DESC*)alloca(m_numGeoms * sizeof(D3D12_RAYTRACING_GEOMETRY_OMM_LINKAGE_DESC));
    auto geomDescs = (D3D12_RAYTRACING_GEOMETRY_DESC*)alloca(m_numGeoms * sizeof(D3D12_RAYTRACING_GEOMETRY_DESC));
    auto triDescs = (D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC*)alloca(m_numGeoms * sizeof(D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC));

	ZeroMemory(linkageDescs, m_numGeoms * sizeof(D3D12_RAYTRACING_GEOMETRY_OMM_LINKAGE_DESC));
	ZeroMemory(geomDescs, m_numGeoms * sizeof(D3D12_RAYTRACING_GEOMETRY_DESC));
	ZeroMemory(triDescs, m_numGeoms * sizeof(D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC));

    UINT indexOffset = 0;
	UINT ommOffset = 0;

    for (UINT i = 0; i < m_numGeoms; i++)
    {
        D3D12_RAYTRACING_GEOMETRY_DESC& geomDesc = geomDescs[i];
        D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC* triDesc;

        geomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;

        bool isLeaves = (i >= 1 && i <= 5);

		// TODO, per geometry buffers, or a single buffer with offsets
        if (isLeaves)
        {
            geomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_OMM_TRIANGLES;
            geomDesc.OmmTriangles.pTriangles = &triDescs[i];
            geomDesc.OmmTriangles.pOmmLinkage = &linkageDescs[i];

			D3D12_RAYTRACING_GEOMETRY_OMM_LINKAGE_DESC& linkageDesc = linkageDescs[i];

            linkageDesc.OpacityMicromapIndexBuffer = { m_ommIndexBuffer.defaultResource->GetGPUVirtualAddress() + (ommOffset * sizeof(UINT)), sizeof(UINT)};
			linkageDesc.OpacityMicromapIndexFormat = DXGI_FORMAT_R32_UINT;
            linkageDesc.OpacityMicromapBaseLocation = 0;

            triDesc = &triDescs[i];
            ommOffset += (m_indicesPerGeom[i] / 3);
        }
        else
        {
            geomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;

            triDesc = &geomDesc.Triangles;
        }

        geomDesc.Flags = isLeaves ? D3D12_RAYTRACING_GEOMETRY_FLAG_NONE : D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
        
                
        triDesc->Transform3x4 = 0;
        triDesc->IndexFormat = DXGI_FORMAT_R32_UINT;
        triDesc->VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        triDesc->IndexCount = m_indicesPerGeom[i];
        triDesc->VertexCount = vertexCount;
        triDesc->IndexBuffer = m_positionIndexBuffer.defaultResource->GetGPUVirtualAddress() + indexOffset * sizeof(UINT);
        triDesc->VertexBuffer.StartAddress = m_positionBuffer.defaultResource->GetGPUVirtualAddress();
        triDesc->VertexBuffer.StrideInBytes = sizeof(XMFLOAT3);

        indexOffset += m_indicesPerGeom[i];
    }

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

    D3D12_RAYTRACING_OPACITY_MICROMAP_ARRAY_DESC ommArrayDesc = {};
    ommArrayDesc.InputBuffer = m_ommArrayBuffer.defaultResource->GetGPUVirtualAddress();
    ommArrayDesc.NumOmmHistogramEntries = m_numHistogramEntries;
    ommArrayDesc.pOmmHistogram = (D3D12_RAYTRACING_OPACITY_MICROMAP_HISTOGRAM_ENTRY*)m_ommHistogramBuffer;
    ommArrayDesc.PerOmmDescs = { m_ommDescBuffer.defaultResource->GetGPUVirtualAddress(), sizeof(D3D12_RAYTRACING_OPACITY_MICROMAP_DESC) };

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC ommBuildDesc = {};
    ommBuildDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_ARRAY;
    ommBuildDesc.Inputs.Flags = buildFlags;
    ommBuildDesc.Inputs.NumDescs = 1;
    ommBuildDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    ommBuildDesc.Inputs.pOpacityMicromapArrayDesc = &ommArrayDesc;

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC blasBuildDesc = {};
    blasBuildDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    blasBuildDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    blasBuildDesc.Inputs.Flags = buildFlags;
    blasBuildDesc.Inputs.NumDescs = m_numGeoms;
    blasBuildDesc.Inputs.pGeometryDescs = geomDescs;

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasBuildDesc = {};
    tlasBuildDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    tlasBuildDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    tlasBuildDesc.Inputs.Flags = buildFlags;
    tlasBuildDesc.Inputs.NumDescs = 1;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO ommPrebuildInfo = {};
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO blasPrebuildInfo = {};
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO tlasPrebuildInfo = {};

    m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&ommBuildDesc.Inputs, &ommPrebuildInfo);
    m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&blasBuildDesc.Inputs, &blasPrebuildInfo);
    m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&tlasBuildDesc.Inputs, &tlasPrebuildInfo);

    size_t scratchSize = max(max(blasPrebuildInfo.ScratchDataSizeInBytes, tlasPrebuildInfo.ScratchDataSizeInBytes), ommPrebuildInfo.ScratchDataSizeInBytes);

    if (m_scratchResource.Get() == nullptr)
        AllocateUAVBuffer(device, scratchSize, &m_scratchResource, D3D12_RESOURCE_STATE_COMMON, L"ScratchResource");

    if (m_ommAccelerationStructure.Get() == nullptr)
        AllocateUAVBuffer(device, ommPrebuildInfo.ResultDataMaxSizeInBytes, &m_ommAccelerationStructure, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, L"OMMAccelerationStructure");

    if (m_bottomLevelAccelerationStructure.Get() == nullptr)
        AllocateUAVBuffer(device, blasPrebuildInfo.ResultDataMaxSizeInBytes, &m_bottomLevelAccelerationStructure, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, L"BottomLevelAccelerationStructure");

    if (m_topLevelAccelerationStructure.Get() == nullptr)
    {
        AllocateUAVBuffer(device, tlasPrebuildInfo.ResultDataMaxSizeInBytes, &m_topLevelAccelerationStructure, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, L"TopLevelAccelerationStructure");

        // Create an SRV for the TLAS so the shaders can access the acceleration structure
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.RaytracingAccelerationStructure.Location = m_topLevelAccelerationStructure->GetGPUVirtualAddress();

        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        srvHandle.ptr += (Descriptors::TLAS * m_descriptorSize);
        device->CreateShaderResourceView(nullptr, &srvDesc, srvHandle);
    }

    D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
    instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
    instanceDesc.InstanceMask = 1;
    instanceDesc.AccelerationStructure = m_bottomLevelAccelerationStructure->GetGPUVirtualAddress();

    if (m_instanceDescs.Get() == nullptr)
        AllocateUploadBuffer(device, &instanceDesc, sizeof(instanceDesc), &m_instanceDescs, L"InstanceDescs");

    for (int i = 0; i < m_numGeoms; i++)
    {
		linkageDescs[i].OpacityMicromapArray = m_ommAccelerationStructure->GetGPUVirtualAddress();
    }

    //blasBuildDesc.Inputs.NumDescs = 2;

    ommBuildDesc.ScratchAccelerationStructureData = m_scratchResource->GetGPUVirtualAddress();
    ommBuildDesc.DestAccelerationStructureData = m_ommAccelerationStructure->GetGPUVirtualAddress();

    blasBuildDesc.ScratchAccelerationStructureData = m_scratchResource->GetGPUVirtualAddress();
    blasBuildDesc.DestAccelerationStructureData = m_bottomLevelAccelerationStructure->GetGPUVirtualAddress();

    tlasBuildDesc.ScratchAccelerationStructureData = m_scratchResource->GetGPUVirtualAddress();
    tlasBuildDesc.DestAccelerationStructureData = m_topLevelAccelerationStructure->GetGPUVirtualAddress();
    tlasBuildDesc.Inputs.InstanceDescs = m_instanceDescs->GetGPUVirtualAddress();

    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
    
    m_dxrCommandList->BuildRaytracingAccelerationStructure(&ommBuildDesc, 0, nullptr);
    commandList->ResourceBarrier(1, &uavBarrier);
    m_dxrCommandList->BuildRaytracingAccelerationStructure(&blasBuildDesc, 0, nullptr);
    commandList->ResourceBarrier(1, &uavBarrier);
    m_dxrCommandList->BuildRaytracingAccelerationStructure(&tlasBuildDesc, 0, nullptr);
    commandList->ResourceBarrier(1, &uavBarrier);
}

void D3D12RaytracingTree::LoadAndBuildAccelerationStructures()
{
    const char* modelPath = "treeModel.bin";
    const char* ommPath = "treeOMM_SubD8_2State.bin";

    // Load the model.
    LoadModel(modelPath, ommPath);
}

void D3D12RaytracingTree::SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig)
{
    auto device = m_deviceResources->GetD3DDevice();
    ComPtr<ID3DBlob> blob;
    ComPtr<ID3DBlob> error;

    ThrowIfFailed(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error), error ? static_cast<wchar_t*>(error->GetBufferPointer()) : nullptr);
    ThrowIfFailed(device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&(*rootSig))));
}

void D3D12RaytracingTree::CreateRootSignatures()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Global Root Signature
    // This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
    {
        CD3DX12_ROOT_PARAMETER rootParameters[GlobalRootSignatureParams::Count] = {};
        rootParameters[0].InitAsConstants(1, 0);

        CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc = {};
        globalRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
        globalRootSignatureDesc.NumParameters = ARRAYSIZE(rootParameters);
        globalRootSignatureDesc.pParameters = rootParameters;

		D3D12_STATIC_SAMPLER_DESC staticSampler = CD3DX12_STATIC_SAMPLER_DESC(0);
        globalRootSignatureDesc.NumStaticSamplers = 1;
		globalRootSignatureDesc.pStaticSamplers = &staticSampler;

        SerializeAndCreateRaytracingRootSignature(globalRootSignatureDesc, &m_raytracingGlobalRootSignature);
    }
}

// Create raytracing device and command list.
void D3D12RaytracingTree::CreateRaytracingInterfaces()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();

    ThrowIfFailed(device->QueryInterface(IID_PPV_ARGS(&m_dxrDevice)), L"Couldn't get DirectX Raytracing interface for the device.\n");
    ThrowIfFailed(commandList->QueryInterface(IID_PPV_ARGS(&m_dxrCommandList)), L"Couldn't get DirectX Raytracing interface for the command list.\n");
}

// Create a raytracing pipeline state object (RTPSO).
// An RTPSO represents a full set of shaders reachable by a DispatchRays() call,
// with all configuration options resolved, such as local signatures and other state.
void D3D12RaytracingTree::CreateRaytracingPipelineStateObject()
{
    // Create 7 subobjects that combine into a RTPSO:
    // Subobjects need to be associated with DXIL exports (i.e. shaders) either by way of default or explicit associations.
    // Default association applies to every exported shader entrypoint that doesn't have any of the same type of subobject associated with it.
    // This simple sample utilizes default shader association except for local root signature subobject
    // which has an explicit association specified purely for demonstration purposes.
    // 1 - DXIL library
    // 1 - Triangle hit group
    // 1 - Shader config
    // 1 - Global root signature
    // 1 - Pipeline config
    CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };
    
    // DXIL library
    // This contains the shaders and their entrypoints for the state object.
    // Since shaders are not considered a subobject, they need to be passed in via DXIL library subobjects.
    auto lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void *)g_pRaytracing, ARRAYSIZE(g_pRaytracing));
    lib->SetDXILLibrary(&libdxil);
    // Define which shader exports to surface from the library.
    // If no shader exports are defined for a DXIL library subobject, all shaders will be surfaced.
    // In this sample, this could be ommited for convenience since the sample uses all shaders in the library. 
    {
        lib->DefineExport(c_raygenShaderName);
        lib->DefineExport(c_closestHitShaderName);
        lib->DefineExport(c_anyHitShaderName);
        lib->DefineExport(c_missShaderName);
    }
    
    // Triangle hit group
    // A hit group specifies closest hit, any hit and intersection shaders to be executed when a ray intersects the geometry's triangle/AABB.
    // In this sample, we only use triangle geometry with a closest hit shader, so others are not set.
    auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    hitGroup->SetClosestHitShaderImport(c_closestHitShaderName);
    hitGroup->SetHitGroupExport(c_hitGroupName);
	hitGroup->SetAnyHitShaderImport(c_anyHitShaderName);
    hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
    
    // Shader config
    // Defines the maximum sizes in bytes for the ray payload and attribute structure.
    auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    UINT payloadSize = sizeof(XMFLOAT4) + sizeof(UINT);    // float4 pixelColor
    UINT attributeSize = sizeof(XMFLOAT2);  // float2 barycentrics
    shaderConfig->Config(payloadSize, attributeSize);

    // Global root signature
    // This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
    auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
    globalRootSignature->SetRootSignature(m_raytracingGlobalRootSignature.Get());

    // Pipeline config
    // Defines the maximum TraceRay() recursion depth.
    auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT>();
    // PERFOMANCE TIP: Set max recursion depth as low as needed 
    // as drivers may apply optimization strategies for low recursion depths.
    UINT maxRecursionDepth = 2; // primary rays + shadow ray 

    pipelineConfig->Config(maxRecursionDepth, D3D12_RAYTRACING_PIPELINE_FLAG_ALLOW_OPACITY_MICROMAPS);
    //pipelineConfig->Config(maxRecursionDepth, D3D12_RAYTRACING_PIPELINE_FLAG_NONE);

#if _DEBUG
    PrintStateObjectDesc(raytracingPipeline);
#endif

    // Create the state object.
    ThrowIfFailed(m_dxrDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_dxrStateObject)), L"Couldn't create DirectX Raytracing state object.\n");
}

// Create 2D output texture for raytracing.
void D3D12RaytracingTree::CreateRaytracingOutputResource()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto backbufferFormat = m_deviceResources->GetBackBufferFormat();

    // Create the output resource. The dimensions and format should match the swap-chain.
    auto uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(backbufferFormat, m_width, m_height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(device->CreateCommittedResource(
        &defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_raytracingOutput)));
    NAME_D3D12_OBJECT(m_raytracingOutput);

    D3D12_CPU_DESCRIPTOR_HANDLE uavDescriptorHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    uavDescriptorHandle.ptr += (Descriptors::RENDER_TARGET * m_descriptorSize);

    device->CreateUnorderedAccessView(m_raytracingOutput.Get(), nullptr, nullptr, uavDescriptorHandle);
}

void D3D12RaytracingTree::CreateDescriptorHeap()
{
    auto device = m_deviceResources->GetD3DDevice();

    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
    descriptorHeapDesc.NumDescriptors = Descriptors::COUNT;
    descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    descriptorHeapDesc.NodeMask = 0;
    device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_descriptorHeap));
    NAME_D3D12_OBJECT(m_descriptorHeap);

    m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

// Build shader tables.
// This encapsulates all shader records - shaders and the arguments for their local root signatures.
void D3D12RaytracingTree::BuildShaderTables()
{
    auto device = m_deviceResources->GetD3DDevice();

    void* rayGenShaderIdentifier;
    void* missShaderIdentifier;
    void* hitGroupShaderIdentifier;

    auto GetShaderIdentifiers = [&](auto* stateObjectProperties)
    {
        rayGenShaderIdentifier = stateObjectProperties->GetShaderIdentifier(c_raygenShaderName);
        missShaderIdentifier = stateObjectProperties->GetShaderIdentifier(c_missShaderName);
        hitGroupShaderIdentifier = stateObjectProperties->GetShaderIdentifier(c_hitGroupName);
    };

    // Get shader identifiers.
    UINT shaderIdentifierSize;
    {
        ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
        ThrowIfFailed(m_dxrStateObject.As(&stateObjectProperties));
        GetShaderIdentifiers(stateObjectProperties.Get());
        shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    }

    // Ray gen shader table
    {
        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIdentifierSize;
        ShaderTable rayGenShaderTable(device, numShaderRecords, shaderRecordSize, L"RayGenShaderTable");
        rayGenShaderTable.push_back(ShaderRecord(rayGenShaderIdentifier, shaderIdentifierSize));
        m_rayGenShaderTable = rayGenShaderTable.GetResource();
    }

    // Miss shader table
    {
        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIdentifierSize;
        ShaderTable missShaderTable(device, numShaderRecords, shaderRecordSize, L"MissShaderTable");
        missShaderTable.push_back(ShaderRecord(missShaderIdentifier, shaderIdentifierSize));
        m_missShaderTable = missShaderTable.GetResource();
    }

    // Hit group shader table
    {
        struct RootArguments {
            TreeConstantBuffer cb;
        } rootArguments;
        rootArguments.cb = m_treeCB;

        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIdentifierSize + sizeof(rootArguments);
        ShaderTable hitGroupShaderTable(device, numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");
        hitGroupShaderTable.push_back(ShaderRecord(hitGroupShaderIdentifier, shaderIdentifierSize, &rootArguments, sizeof(rootArguments)));
        m_hitGroupShaderTable = hitGroupShaderTable.GetResource();
    }
}

// Update frame-based values.
void D3D12RaytracingTree::OnUpdate()
{
    m_timer.Tick();
    CalculateFrameStats();
    float elapsedTime = static_cast<float>(m_timer.GetElapsedSeconds());
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();
    auto prevFrameIndex = m_deviceResources->GetPreviousFrameIndex();

    // Rotate the camera around Y axis.
    {
        float secondsToRotateAround = 120.0f;
        float angleToRotateBy = 0;// 360.0f * (elapsedTime / secondsToRotateAround);
        XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(angleToRotateBy));
        m_eye = XMVector3Transform(m_eye, rotate);
        m_up = XMVector3Transform(m_up, rotate);
        m_at = XMVector3Transform(m_at, rotate);
        UpdateCameraMatrices();
    }
}

void D3D12RaytracingTree::DoRaytracing()
{
    auto commandList = m_deviceResources->GetCommandList();
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    static bool done = false;

    if (!done)
    {
        LoadTextures(); // TODO, put this somewhere sensible
        BuildAccelerationStructures();
    }

    done = true;
    
    auto DispatchRays = [&](auto* commandList, auto* stateObject, auto* dispatchDesc)
    {
        // Since each shader table has only one shader record, the stride is same as the size.
        dispatchDesc->HitGroupTable.StartAddress = m_hitGroupShaderTable->GetGPUVirtualAddress();
        dispatchDesc->HitGroupTable.SizeInBytes = m_hitGroupShaderTable->GetDesc().Width;
        dispatchDesc->HitGroupTable.StrideInBytes = dispatchDesc->HitGroupTable.SizeInBytes;
        dispatchDesc->MissShaderTable.StartAddress = m_missShaderTable->GetGPUVirtualAddress();
        dispatchDesc->MissShaderTable.SizeInBytes = m_missShaderTable->GetDesc().Width;
        dispatchDesc->MissShaderTable.StrideInBytes = dispatchDesc->MissShaderTable.SizeInBytes;
        dispatchDesc->RayGenerationShaderRecord.StartAddress = m_rayGenShaderTable->GetGPUVirtualAddress();
        dispatchDesc->RayGenerationShaderRecord.SizeInBytes = m_rayGenShaderTable->GetDesc().Width;
        dispatchDesc->Width = m_width;
        dispatchDesc->Height = m_height;
        dispatchDesc->Depth = 1;
        commandList->SetPipelineState1(stateObject);
        commandList->DispatchRays(dispatchDesc);
    };

    auto SetCommonPipelineState = [&](auto* descriptorSetCommandList)
    {
        descriptorSetCommandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());
        // Set index and successive vertex buffer decriptor tables
        //commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::VertexBuffersSlot, m_indexBuffer.gpuDescriptorHandle);
        //commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::OutputViewSlot, m_raytracingOutputResourceUAVGpuDescriptor);
    };

    

    // Copy the updated scene constant buffer to GPU.
    memcpy(&m_mappedConstantData[frameIndex].constants, &m_sceneCB[frameIndex], sizeof(m_sceneCB[frameIndex]));
   
    // Bind the heaps, acceleration structure and dispatch rays.
    D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
    SetCommonPipelineState(commandList);

    commandList->SetComputeRootSignature(m_raytracingGlobalRootSignature.Get());
    commandList->SetComputeRoot32BitConstant(GlobalRootSignatureParams::FrameParams, frameIndex, 0);

    DispatchRays(m_dxrCommandList.Get(), m_dxrStateObject.Get(), &dispatchDesc);
}

// Update the application state with the new resolution.
void D3D12RaytracingTree::UpdateForSizeChange(UINT width, UINT height)
{
    DXSample::UpdateForSizeChange(width, height);
}

// Copy the raytracing output to the backbuffer.
void D3D12RaytracingTree::CopyRaytracingOutputToBackbuffer()
{
    auto commandList= m_deviceResources->GetCommandList();
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
void D3D12RaytracingTree::CreateWindowSizeDependentResources()
{
    CreateRaytracingOutputResource(); 
    UpdateCameraMatrices();
}

// Release resources that are dependent on the size of the main window.
void D3D12RaytracingTree::ReleaseWindowSizeDependentResources()
{
    m_raytracingOutput.Reset();
}

// Release all resources that depend on the device.
void D3D12RaytracingTree::ReleaseDeviceDependentResources()
{
    m_raytracingGlobalRootSignature.Reset();

    m_dxrDevice.Reset();
    m_dxrCommandList.Reset();
    m_dxrStateObject.Reset();

    m_descriptorHeap.Reset();
    m_perFrameConstants.Reset();
    m_rayGenShaderTable.Reset();
    m_missShaderTable.Reset();
    m_hitGroupShaderTable.Reset();

    m_bottomLevelAccelerationStructure.Reset();
    m_topLevelAccelerationStructure.Reset();

}

void D3D12RaytracingTree::RecreateD3D()
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
void D3D12RaytracingTree::OnRender()
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

void D3D12RaytracingTree::OnDestroy()
{
    // Let GPU finish before releasing D3D resources.
    m_deviceResources->WaitForGpu();
    OnDeviceLost();
}

// Release all device dependent resouces when a device is lost.
void D3D12RaytracingTree::OnDeviceLost()
{
    ReleaseWindowSizeDependentResources();
    ReleaseDeviceDependentResources();
}

// Create all device dependent resources when a device is restored.
void D3D12RaytracingTree::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

// Compute the average frames per second and million rays per second.
void D3D12RaytracingTree::CalculateFrameStats()
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
        windowText << setprecision(2) << fixed
            << L"    fps: " << fps << L"     ~Million Primary Rays/s: " << MRaysPerSecond
            << L"    GPU[" << m_deviceResources->GetAdapterID() << L"]: " << m_deviceResources->GetAdapterDescription();
        SetCustomWindowText(windowText.str().c_str());
    }
}

// Handle OnSizeChanged message event.
void D3D12RaytracingTree::OnSizeChanged(UINT width, UINT height, bool minimized)
{
    if (!m_deviceResources->WindowSizeChanged(width, height, minimized))
    {
        return;
    }

    UpdateForSizeChange(width, height);

    ReleaseWindowSizeDependentResources();
    CreateWindowSizeDependentResources();
}

// Create SRV for a buffer.
UINT D3D12RaytracingTree::CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize)
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
    __debugbreak();
    UINT descriptorIndex = 0;// AllocateDescriptor(&buffer->cpuDescriptorHandle);
    //device->CreateShaderResourceView(buffer->defaultResource.Get(), &srvDesc, buffer->cpuDescriptorHandle);
    //buffer->gpuDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), descriptorIndex, m_descriptorSize);
    return descriptorIndex;
}