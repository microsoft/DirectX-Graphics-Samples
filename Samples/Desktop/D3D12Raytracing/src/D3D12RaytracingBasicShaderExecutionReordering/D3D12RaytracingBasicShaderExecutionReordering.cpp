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
#include "D3D12RaytracingBasicShaderExecutionReordering.h"
#include "DirectXRaytracingHelper.h"
#include "dxcapi.h"
#include <atlbase.h>
#include <vector>

#include "SharedCode.h"

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 717; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\"; }

using namespace std;
using namespace DX;

const wchar_t* D3D12RaytracingBasicShaderExecutionReordering::c_hitGroupName = L"MyHitGroup";
const wchar_t* D3D12RaytracingBasicShaderExecutionReordering::c_raygenShaderName = L"MyRaygenShader";
const wchar_t* D3D12RaytracingBasicShaderExecutionReordering::c_closestHitShaderName = L"MyClosestHitShader";
const wchar_t* D3D12RaytracingBasicShaderExecutionReordering::c_missShaderName = L"MyMissShader";

#define PRINT(text) OutputDebugStringA(text);

//=================================================================================================================================
HRESULT CompileDxilLibraryFromFile(
    _In_ LPCWSTR pFile,
    _In_ LPCWSTR pTarget,
    _In_reads_(cDefines) DxcDefine* pDefines,
    _In_ UINT cDefines,
    _Out_ ID3DBlob** ppCode)
{
    HRESULT hr = S_OK;
    *ppCode = nullptr;

    static HMODULE s_hmod = 0;
    static HMODULE s_hmodDxil = 0;
    static DxcCreateInstanceProc s_pDxcCreateInstanceProc = nullptr;
    if (s_hmodDxil == 0)
    {
        s_hmodDxil = LoadLibrary(L"dxil.dll");
        if (s_hmodDxil == 0)
        {
            PRINT("dxil.dll missing or wrong architecture");
            return E_FAIL;
        }
    }
    if (s_hmod == 0)
    {
        s_hmod = LoadLibrary(L"dxcompiler.dll");
        if (s_hmod == 0)
        {
            PRINT("dxcompiler.dll missing or wrong architecture");
            return E_FAIL;
        }

        if (s_pDxcCreateInstanceProc == nullptr)
        {
            s_pDxcCreateInstanceProc = (DxcCreateInstanceProc)GetProcAddress(s_hmod, "DxcCreateInstance");
            if (s_pDxcCreateInstanceProc == nullptr)
            {
                PRINT("Unable to find dxcompiler!DxcCreateInstance");
                return E_FAIL;
            }
        }
    }

    CComPtr<IDxcCompiler> compiler;
    CComPtr<IDxcLibrary> library;
    CComPtr<IDxcBlobEncoding> source;
    CComPtr<IDxcOperationResult> operationResult;
    CComPtr<IDxcIncludeHandler> includeHandler;
    hr = s_pDxcCreateInstanceProc(CLSID_DxcLibrary, __uuidof(IDxcLibrary), reinterpret_cast<LPVOID*>(&library));
    if (FAILED(hr))
    {
        PRINT("Failed to instantiate compiler.");
        return hr;
    }

    HRESULT createBlobHr = library->CreateBlobFromFile(pFile, nullptr, &source);
    if (createBlobHr != S_OK)
    {
        PRINT("Create Blob From File Failed - perhaps file is missing?");
        return E_FAIL;
    }

    hr = library->CreateIncludeHandler(&includeHandler);
    if (FAILED(hr))
    {
        PRINT("Failed to create include handler.");
        return hr;
    }
    hr = s_pDxcCreateInstanceProc(CLSID_DxcCompiler, __uuidof(IDxcCompiler), reinterpret_cast<LPVOID*>(&compiler));
    if (FAILED(hr))
    {
        PRINT("Failed to instantiate compiler.");
        return hr;
    }

    LPCWSTR args[] = { L"" };
    UINT cArgs = 0;
    hr = compiler->Compile(
        source,
        nullptr,
        nullptr,
        pTarget,
        args, cArgs,
        pDefines, cDefines,
        includeHandler,
        &operationResult);
    if (FAILED(hr))
    {
        PRINT("Failed to compile.");
        return hr;
    }

    operationResult->GetStatus(&hr);
    if (SUCCEEDED(hr))
    {
        hr = operationResult->GetResult((IDxcBlob**)ppCode);
        if (FAILED(hr))
        {
            PRINT("Failed to retrieve compiled code.");
        }
    }
    CComPtr<IDxcBlobEncoding> pErrors;
    if (SUCCEEDED(operationResult->GetErrorBuffer(&pErrors)))
    {
        auto pText = pErrors->GetBufferPointer();
        if (pText)
        {
            PRINT((char*)pText);
        }
    }

    return hr;
}

D3D12RaytracingBasicShaderExecutionReordering::D3D12RaytracingBasicShaderExecutionReordering(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_raytracingOutputResourceUAVDescriptorHeapIndex(UINT_MAX),
    m_curRotationAngleRad(0.0f),
    m_serEnabled(false)               
{
    UpdateForSizeChange(width, height);
}

void D3D12RaytracingBasicShaderExecutionReordering::OnInit()
{
    UUID Features[] = { D3D12ExperimentalShaderModels };
    ThrowIfFailed(D3D12EnableExperimentalFeatures(_countof(Features), Features, nullptr, nullptr));

    m_deviceResources = std::make_unique<DeviceResources>(
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_UNKNOWN,
        FrameCount,
        D3D_FEATURE_LEVEL_11_0,
        // Sample shows handling of use cases with tearing support, which is OS dependent and has been supported since TH2.
        // Since the sample requires build 1809 (RS5) or higher, we don't need to handle non-tearing cases.
        DeviceResources::c_RequireTearingSupport,
        m_adapterIDoverride
    );
    m_deviceResources->RegisterDeviceNotify(this);
    m_deviceResources->SetWindow(Win32Application::GetHwnd(), m_width, m_height);
    m_deviceResources->InitializeDXGIAdapter();

    ThrowIfFalse(IsDirectXRaytracingSupported(m_deviceResources->GetAdapter()),
        L"ERROR: DirectX Raytracing is not supported by your OS, GPU and/or driver.\n\n");

    m_deviceResources->CreateDeviceResources();
    m_deviceResources->CreateWindowSizeDependentResources();

    InitializeScene();

    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();

    m_keyboard = std::make_unique<Keyboard>();
}

// Update camera matrices passed into the shader.
void D3D12RaytracingBasicShaderExecutionReordering::UpdateCameraMatrices()
{
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    m_sceneCB[frameIndex].cameraPosition = m_eye;
    float fovAngleY = 40.0f;
    XMMATRIX view = XMMatrixLookAtLH(m_eye, m_at, m_up);
    XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), m_aspectRatio, 1.0f, 125.0f);
    XMMATRIX viewProj = view * proj;

    m_sceneCB[frameIndex].projectionToWorld = XMMatrixInverse(nullptr, viewProj);
}

// Initialize scene rendering parameters.
void D3D12RaytracingBasicShaderExecutionReordering::InitializeScene()
{
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();
    // Setup camera.
    {
        // Initialize the view and projection inverse matrices.
        m_eye = { 0.0f, 50.0f, -4.0f, 1.0f };
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

        lightPosition = XMFLOAT4(0.0f, 15.8f, -3.0f, 0.0f);
        m_sceneCB[frameIndex].lightPosition = XMLoadFloat4(&lightPosition);

        lightAmbientColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
        m_sceneCB[frameIndex].lightAmbientColor = XMLoadFloat4(&lightAmbientColor);

        lightDiffuseColor = XMFLOAT4(1.0f, 0.75f, 0.85f, 1.0f);
        m_sceneCB[frameIndex].lightDiffuseColor = XMLoadFloat4(&lightDiffuseColor);
    }

    // Apply the initial values to all frames' buffer instances.
    for (auto& sceneCB : m_sceneCB)
    {
        sceneCB = m_sceneCB[frameIndex];
    }
}

// Create constant buffers.
void D3D12RaytracingBasicShaderExecutionReordering::CreateConstantBuffers()
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

// Create resources that depend on the device.
void D3D12RaytracingBasicShaderExecutionReordering::CreateDeviceDependentResources()
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

	// Build complex geometry (the torus knot to illustrate the use of multiple geometries in the scene)
    BuildComplexGeometry();

    // Create texture
    CreateTexture();

    // Build raytracing acceleration structures from the generated geometry.
    BuildAccelerationStructures();

    // Create constant buffers for the geometry and the scene.
    CreateConstantBuffers();

    // Build shader tables, which define shaders and their local root arguments.
    BuildShaderTables();

    // Create an output 2D texture to store the raytracing result to.
    CreateRaytracingOutputResource();

    // Load fonts
    CreateUIFont();
}

void D3D12RaytracingBasicShaderExecutionReordering::CreateTexture()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandAllocator = m_deviceResources->GetCommandAllocator();
    auto commandList = m_deviceResources->GetCommandList();
    auto commandQueue = m_deviceResources->GetCommandQueue();

    // Reset the command list before using
    commandList->Reset(commandAllocator, nullptr);

    // Create the texture resource
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width = TextureWidth;
    textureDesc.Height = TextureHeight;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_texture)));

    // Generate texture data
    std::vector<UINT8> textureData = GenerateTextureData();
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);

    // Create the upload heap
    ComPtr<ID3D12Resource> textureUploadHeap;
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&textureUploadHeap)));

    // Copy data to upload heap
    D3D12_SUBRESOURCE_DATA textureSubresourceData = {};
    textureSubresourceData.pData = &textureData[0];
    textureSubresourceData.RowPitch = TextureWidth * TexturePixelSize;
    textureSubresourceData.SlicePitch = textureData.size();

    UpdateSubresources(
        commandList,
        m_texture.Get(),
        textureUploadHeap.Get(),
        0, 0, 1,
        &textureSubresourceData);

    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        m_texture.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    // Execute the command list to upload the texture
    m_deviceResources->ExecuteCommandList();
    m_deviceResources->WaitForGpu(); // Wait for upload to complete

    // Create SRV for the texture (can be done without command list)
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    // Create the SRV in descriptor heap
    D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
    UINT descriptorIndex = AllocateDescriptor(&srvHandle);
    device->CreateShaderResourceView(m_texture.Get(), &srvDesc, srvHandle);
    m_textureSrvGpuDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(
        m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
        descriptorIndex,
        m_descriptorSize);
}

void D3D12RaytracingBasicShaderExecutionReordering::CreateUIFont()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto size = m_deviceResources->GetOutputSize();

    m_graphicsMemory = std::make_unique<GraphicsMemory>(device);

    ResourceUploadBatch resourceUpload(device);

    resourceUpload.Begin();

    {
        RenderTargetState rtState(m_deviceResources->GetBackBufferFormat(), m_deviceResources->GetDepthBufferFormat());
        SpriteBatchPipelineStateDescription pd(rtState);
        m_spriteBatch = std::make_unique<SpriteBatch>(device, resourceUpload, pd);
    }

    auto uploadResourcesFinished = resourceUpload.End(m_deviceResources->GetCommandQueue());
    uploadResourcesFinished.wait();

    D3D12_CPU_DESCRIPTOR_HANDLE fontHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    fontHandle.ptr += (Descriptors::FONT * m_descriptorSize);

    D3D12_GPU_DESCRIPTOR_HANDLE fontGpuHandle = m_descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    fontGpuHandle.ptr += (Descriptors::FONT * m_descriptorSize);


    // Begin uploading texture resources
    {
        ResourceUploadBatch resourceUpload(device);
        resourceUpload.Begin();

        m_smallFont = std::make_unique<SpriteFont>(device, resourceUpload,
            L"SegoeUI_18.spritefont",
            fontHandle,
            fontGpuHandle);

        auto finished = resourceUpload.End(m_deviceResources->GetCommandQueue());
        finished.wait();
    }
}

void D3D12RaytracingBasicShaderExecutionReordering::SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig)
{
    auto device = m_deviceResources->GetD3DDevice();
    ComPtr<ID3DBlob> blob;
    ComPtr<ID3DBlob> error;

    ThrowIfFailed(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error), error ? static_cast<wchar_t*>(error->GetBufferPointer()) : nullptr);
    ThrowIfFailed(device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&(*rootSig))));
}

void D3D12RaytracingBasicShaderExecutionReordering::CreateRootSignatures()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Global Root Signature
    // This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
    {
        CD3DX12_DESCRIPTOR_RANGE ranges[2]; // Perfomance TIP: Order from most frequent to least frequent.
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 1 output texture
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 1);  // 2 static index and 2 vertex buffers + 1 texture

        CD3DX12_ROOT_PARAMETER rootParameters[GlobalRootSignatureParams::Count];
        rootParameters[GlobalRootSignatureParams::OutputViewSlot].InitAsDescriptorTable(1, &ranges[0]);
        rootParameters[GlobalRootSignatureParams::AccelerationStructureSlot].InitAsShaderResourceView(0);
        rootParameters[GlobalRootSignatureParams::SceneConstantSlot].InitAsConstantBufferView(0);
        rootParameters[GlobalRootSignatureParams::VertexBuffersSlot].InitAsDescriptorTable(1, &ranges[1]);

        // Static sampler
        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0; 
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters,
            1, &sampler); 
        SerializeAndCreateRaytracingRootSignature(globalRootSignatureDesc, &m_raytracingGlobalRootSignature);
    }

    // Local Root Signature
    // This is a root signature that enables a shader to have unique arguments that come from shader tables.
    {
        CD3DX12_ROOT_PARAMETER rootParameters[LocalRootSignatureParams::Count];
        rootParameters[LocalRootSignatureParams::CubeConstantSlot].InitAsConstants(SizeOfInUint32(m_cubeCB), 1);
        CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
        localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
        SerializeAndCreateRaytracingRootSignature(localRootSignatureDesc, &m_raytracingLocalRootSignature);
    }
}

// Create raytracing device and command list.
void D3D12RaytracingBasicShaderExecutionReordering::CreateRaytracingInterfaces()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();

    ThrowIfFailed(device->QueryInterface(IID_PPV_ARGS(&m_dxrDevice)), L"Couldn't get DirectX Raytracing interface for the device.\n");
    ThrowIfFailed(commandList->QueryInterface(IID_PPV_ARGS(&m_dxrCommandList)), L"Couldn't get DirectX Raytracing interface for the command list.\n");
}

// Local root signature and shader association
// This is a root signature that enables a shader to have unique arguments that come from shader tables.
void D3D12RaytracingBasicShaderExecutionReordering::CreateLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
{
    // Ray gen and miss shaders in this sample are not using a local root signature and thus one is not associated with them.

    // Local root signature to be used in a hit group.
    auto localRootSignature = raytracingPipeline->CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
    localRootSignature->SetRootSignature(m_raytracingLocalRootSignature.Get());
    // Define explicit shader association for the local root signature. 
    {
        auto rootSignatureAssociation = raytracingPipeline->CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
        rootSignatureAssociation->AddExport(c_hitGroupName);
        rootSignatureAssociation->AddExport(c_raygenShaderName);
    }
}

// Create a raytracing pipeline state object (RTPSO).
// An RTPSO represents a full set of shaders reachable by a DispatchRays() call,
// with all configuration options resolved, such as local signatures and other state.
void D3D12RaytracingBasicShaderExecutionReordering::CreateRaytracingPipelineStateObject()
{
    D3D12_FEATURE_DATA_SHADER_MODEL SM;
    SM.HighestShaderModel = D3D_SHADER_MODEL_6_9;
    m_dxrDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &SM, sizeof(SM));
    ThrowIfFalse(SM.HighestShaderModel >= D3D_SHADER_MODEL_6_9,
        L"ERROR: Device doesn't support Shader Model 6.9.\n\n");

    // Create 7 subobjects that combine into a RTPSO:
    // Subobjects need to be associated with DXIL exports (i.e. shaders) either by way of default or explicit associations.
    // Default association applies to every exported shader entrypoint that doesn't have any of the same type of subobject associated with it.
    // This simple sample utilizes default shader association except for local root signature subobject
    // which has an explicit association specified purely for demonstration purposes.
    // 1 - DXIL library
    // 1 - Triangle hit group
    // 1 - Shader config
    // 2 - Local root signature and association
    // 1 - Global root signature
    // 1 - Pipeline config
    CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

    CComPtr<ID3DBlob> library;
    ThrowIfFailed(CompileDxilLibraryFromFile(L"Raytracing.hlsl", L"lib_6_9", nullptr, 0, &library));

    auto lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    CD3DX12_SHADER_BYTECODE libCode(library);
    lib->SetDXILLibrary(&libCode);
    {
        lib->DefineExport(c_raygenShaderName);
        lib->DefineExport(c_closestHitShaderName);
        lib->DefineExport(c_missShaderName);
    }

    // Triangle hit group
    // A hit group specifies closest hit, any hit and intersection shaders to be executed when a ray intersects the geometry's triangle/AABB.
    // In this sample, we only use triangle geometry with a closest hit shader, so others are not set.
    auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    hitGroup->SetClosestHitShaderImport(c_closestHitShaderName);
    hitGroup->SetHitGroupExport(c_hitGroupName);
    hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

    // Shader config
   //  Defines the maximum sizes in bytes for the ray payload and attribute structure.
    auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    UINT payloadSize = sizeof(XMFLOAT4);    // float4 color
    UINT attributeSize = sizeof(XMFLOAT2);  // float2 barycentrics
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
    UINT maxRecursionDepth = 1; // ~ primary rays only. 
    pipelineConfig->Config(maxRecursionDepth);

#if _DEBUG
    PrintStateObjectDesc(raytracingPipeline);
#endif

    // Create the state object.
    ThrowIfFailed(m_dxrDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_dxrStateObject)), L"Couldn't create DirectX Raytracing state object.\n");
}

// Create 2D output texture for raytracing.
void D3D12RaytracingBasicShaderExecutionReordering::CreateRaytracingOutputResource()
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

void D3D12RaytracingBasicShaderExecutionReordering::CreateDescriptorHeap()
{
    auto device = m_deviceResources->GetD3DDevice();

    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
    // Allocate a heap for 6 descriptors:
    // 2 - vertex and index buffer SRVs for cube
    // 2 - vertex and index buffer SRVs for complex shape
    // 1 - raytracing output texture UAV
    // 1 - texture SRV

    descriptorHeapDesc.NumDescriptors = 6;
    descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    descriptorHeapDesc.NodeMask = 0;
    device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_descriptorHeap));
    NAME_D3D12_OBJECT(m_descriptorHeap);

    m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

// Build geometry used in the sample.
void D3D12RaytracingBasicShaderExecutionReordering::BuildGeometry()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Cube indices.
    Index indices[] =
    {
        3,1,0,
        2,1,3,

        6,4,5,
        7,4,6,

        11,9,8,
        10,9,11,

        14,12,13,
        15,12,14,

        19,17,16,
        18,17,19,

        22,20,21,
        23,20,22
    };

    // Cube vertices positions and corresponding triangle normals.
    Vertex vertices[] =
    {
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },

        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },

        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },

        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
    };

    AllocateUploadBuffer(device, indices, sizeof(indices), &m_indexBuffer.resource);
    AllocateUploadBuffer(device, vertices, sizeof(vertices), &m_vertexBuffer.resource);

    // Vertex buffer is passed to the shader along with index buffer as a descriptor table.
    // Vertex buffer descriptor must follow index buffer descriptor in the descriptor heap.
    UINT descriptorIndexIB = CreateBufferSRV(&m_indexBuffer, sizeof(indices) / 4, 0);
    UINT descriptorIndexVB = CreateBufferSRV(&m_vertexBuffer, ARRAYSIZE(vertices), sizeof(vertices[0]));
    ThrowIfFalse(descriptorIndexVB == descriptorIndexIB + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index!");
}


// Build geometry for a complex torus knot shape
void D3D12RaytracingBasicShaderExecutionReordering::BuildComplexGeometry()
{
    auto device = m_deviceResources->GetD3DDevice();
    // Torus knot parameters
    const int tubularSegments = 7000;
    const int radialSegments = 42;    // Number of segments around radius
    const float p = 2.0f;             // Number of times around the circle
    const float q = 3.0f;             // Number of times through the circle
    const float radius = 1.0f;        // Radius of entire knot
    const float tubeRadius = 0.3f;    // Radius of the tube


    const int vertexCount = (tubularSegments + 1) * (radialSegments + 1);
    const int indexCount = tubularSegments * radialSegments * 6;

    std::vector<Vertex> vertices(vertexCount);
    std::vector<Index> indices(indexCount);

    // Generate vertices
    for (int i = 0; i < tubularSegments; i++) {
        for (int j = 0; j < radialSegments; j++) {
            const int index = i * radialSegments + j;
            // Calculate position on the knot curve
            float u = i / float(tubularSegments) * 2.0f * p * XM_PI;
            float v = j / float(radialSegments) * 2.0f * XM_PI;
            // Core curve of the knot
            float x = (radius + tubeRadius * cos(v)) * cos(q * u);
            float y = (radius + tubeRadius * cos(v)) * sin(q * u);
            float z = tubeRadius * sin(v) + sin(p * u) * 0.5f;
            // Calculate normals using partial derivatives
            float nx = cos(v) * cos(q * u);
            float ny = cos(v) * sin(q * u);
            float nz = sin(v);
            // Normalize the normal
            float len = sqrt(nx * nx + ny * ny + nz * nz);
            nx /= len; ny /= len; nz /= len;
            // Store the vertex
            vertices[index].position = XMFLOAT3(x, y, z);
            vertices[index].normal = XMFLOAT3(nx, ny, nz);
        }
    }

    // Generate indices for triangles
    int indexOffset = 0;
    for (int i = 0; i < tubularSegments; i++) {
        for (int j = 0; j < radialSegments; j++) {
            const int a = i * radialSegments + j;
            const int b = i * radialSegments + ((j + 1) % radialSegments);
            const int c = ((i + 1) % tubularSegments) * radialSegments + ((j + 1) % radialSegments);
            const int d = ((i + 1) % tubularSegments) * radialSegments + j;
            // Triangle 1
            indices[indexOffset++] = a;
            indices[indexOffset++] = b;
            indices[indexOffset++] = d;
            // Triangle 2
            indices[indexOffset++] = b;
            indices[indexOffset++] = c;
            indices[indexOffset++] = d;
        }
    }


    AllocateUploadBuffer(device, indices.data(), indices.size() * sizeof(Index), &m_complexIndexBuffer.resource);
    AllocateUploadBuffer(device, vertices.data(), vertices.size() * sizeof(Vertex), &m_complexVertexBuffer.resource);

    // Create SRVs for the complex shape buffers
    UINT descriptorIndexIB = CreateBufferSRV(&m_complexIndexBuffer, (UINT)indices.size(), sizeof(Index));
    UINT descriptorIndexVB = CreateBufferSRV(&m_complexVertexBuffer, (UINT)vertices.size(), sizeof(Vertex));
    ThrowIfFalse(descriptorIndexVB == descriptorIndexIB + 1, L"Complex shape vertex buffer descriptor index must follow that of index buffer descriptor index!");
}


// Build acceleration structures needed for raytracing.
void D3D12RaytracingBasicShaderExecutionReordering::BuildAccelerationStructures()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();
    auto commandQueue = m_deviceResources->GetCommandQueue();
    auto commandAllocator = m_deviceResources->GetCommandAllocator();

    // Reset the command list for the acceleration structure construction.
    commandList->Reset(commandAllocator, nullptr);

    // Setup cube geometry desc
    D3D12_RAYTRACING_GEOMETRY_DESC cubeGeometryDesc = {};
    cubeGeometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    cubeGeometryDesc.Triangles.IndexBuffer = m_indexBuffer.resource->GetGPUVirtualAddress();
    cubeGeometryDesc.Triangles.IndexCount = static_cast<UINT>(m_indexBuffer.resource->GetDesc().Width) / sizeof(Index);
    cubeGeometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
    cubeGeometryDesc.Triangles.Transform3x4 = 0;
    cubeGeometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    cubeGeometryDesc.Triangles.VertexCount = static_cast<UINT>(m_vertexBuffer.resource->GetDesc().Width) / sizeof(Vertex);
    cubeGeometryDesc.Triangles.VertexBuffer.StartAddress = m_vertexBuffer.resource->GetGPUVirtualAddress();
    cubeGeometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);

    // Mark the geometry as opaque. 
    // PERFORMANCE TIP: mark geometry as opaque whenever applicable as it can enable important ray processing optimizations.
    // Note: When rays encounter opaque geometry an any hit shader will not be executed whether it is present or not.
    cubeGeometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

    // Setup complex shape geometry desc
    D3D12_RAYTRACING_GEOMETRY_DESC complexGeometryDesc = {};
    complexGeometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    complexGeometryDesc.Triangles.IndexBuffer = m_complexIndexBuffer.resource->GetGPUVirtualAddress();
    complexGeometryDesc.Triangles.IndexCount = static_cast<UINT>(m_complexIndexBuffer.resource->GetDesc().Width) / sizeof(Index);
    complexGeometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
    complexGeometryDesc.Triangles.Transform3x4 = 0;
    complexGeometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    complexGeometryDesc.Triangles.VertexCount = static_cast<UINT>(m_complexVertexBuffer.resource->GetDesc().Width) / sizeof(Vertex);
    complexGeometryDesc.Triangles.VertexBuffer.StartAddress = m_complexVertexBuffer.resource->GetGPUVirtualAddress();
    complexGeometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);

    // Mark the geometry as opaque. 
    // PERFORMANCE TIP: mark geometry as opaque whenever applicable as it can enable important ray processing optimizations.
    // Note: When rays encounter opaque geometry an any hit shader will not be executed whether it is present or not.
    complexGeometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;

    // Get required sizes for an acceleration structure.
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

    // Get prebuild info for the bottom-level acceleration structure.
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS cubeBLASInputs = {};
    cubeBLASInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    cubeBLASInputs.Flags = buildFlags;
    cubeBLASInputs.NumDescs = 1; // 1 geometry desc
    cubeBLASInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    cubeBLASInputs.pGeometryDescs = &cubeGeometryDesc;

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS complexBLASInputs = {};
    complexBLASInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    complexBLASInputs.Flags = buildFlags;
    complexBLASInputs.NumDescs = 1; // 1 geometry desc
    complexBLASInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    complexBLASInputs.pGeometryDescs = &complexGeometryDesc;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO cubeBLASPrebuildInfo = {};
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO complexBLASPrebuildInfo = {};

    m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&cubeBLASInputs, &cubeBLASPrebuildInfo);
    m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&complexBLASInputs, &complexBLASPrebuildInfo);

    // Tp-level acceleration structure - 3 instances (two cubes, one complex shape).
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
    topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    topLevelInputs.Flags = buildFlags;
    topLevelInputs.NumDescs = 10082;
    topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
    m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
    ThrowIfFalse(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

    ComPtr<ID3D12Resource> scratchResource;
    SIZE_T scratchSize = max(
        cubeBLASPrebuildInfo.ScratchDataSizeInBytes,
        max(complexBLASPrebuildInfo.ScratchDataSizeInBytes, topLevelPrebuildInfo.ScratchDataSizeInBytes)
    );
    AllocateUAVBuffer(device, scratchSize, &scratchResource, D3D12_RESOURCE_STATE_COMMON, L"ScratchResource");

    // Allocate resources for acceleration structures.
    // Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
    // Default heap is OK since the application doesnt need CPU read/write access to them. 
    // The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
    // and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
    //  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
    //  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.
    {
        D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

        AllocateUAVBuffer(device, cubeBLASPrebuildInfo.ResultDataMaxSizeInBytes, &m_bottomLevelAccelerationStructureCube, initialResourceState, L"BottomLevelAccelerationStructureCube");
        AllocateUAVBuffer(device, complexBLASPrebuildInfo.ResultDataMaxSizeInBytes, &m_bottomLevelAccelerationStructureComplex, initialResourceState, L"BottomLevelAccelerationStructureComplex");
        AllocateUAVBuffer(device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_topLevelAccelerationStructure, initialResourceState, L"TopLevelAccelerationStructure");
    }

    // Create an instance desc for the bottom-level acceleration structure.
    ComPtr<ID3D12Resource> instanceDescsResource;
    std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDesc;
    int cubesPerRow = 70;     // Cubes per row along X and Z axes
    float cubeSpacing = 2.0f; // Spacing between cubes

    // Loop through each position in the XZ plane to create cubes and complex shapes.
    for (int x = -cubesPerRow / 2; x <= cubesPerRow / 2; ++x) {
        for (int z = -cubesPerRow / 2; z <= cubesPerRow / 2; ++z) {
            D3D12_RAYTRACING_INSTANCE_DESC desc = {};
            desc.Transform[0][0] = desc.Transform[1][1] = desc.Transform[2][2] = 1.0f;

            // Position the cubes in the XZ plane, Y remains 0 (flat around 0, 0, 0)
            desc.Transform[0][3] = x * cubeSpacing; // Position along X axis
            desc.Transform[1][3] = 0.0f;           // Y remains at 0 for all cubes (on the ground)
            desc.Transform[2][3] = z * cubeSpacing; // Position along Z axis

            desc.InstanceMask = 1;
            desc.AccelerationStructure = m_bottomLevelAccelerationStructureCube->GetGPUVirtualAddress();
            desc.InstanceID = instanceDesc.size(); 
            desc.InstanceContributionToHitGroupIndex = static_cast<UINT>(instanceDesc.size());
            instanceDesc.push_back(desc);
        }
    }


    float complexShapeZ = -15.0f;
    float complexShapeSpacing = 1.6f;
    for (int x = -cubesPerRow / 2; x <= cubesPerRow / 2; ++x) {
        for (int z = -cubesPerRow / 2; z <= cubesPerRow / 2; ++z) {
            D3D12_RAYTRACING_INSTANCE_DESC desc = {};
            desc.Transform[0][0] = desc.Transform[1][1] = desc.Transform[2][2] = 1.0f;

            // Position the complex shapes directly above the cubes
            desc.Transform[0][3] = x * complexShapeSpacing; // X position same as cubes
            desc.Transform[1][3] = 2.0f;           // Y position set to 2.0f (above cubes)
            desc.Transform[2][3] = z * complexShapeSpacing; // Z position same as cubes

            desc.InstanceMask = 1;
            desc.AccelerationStructure = m_bottomLevelAccelerationStructureComplex->GetGPUVirtualAddress();
            desc.InstanceID = instanceDesc.size(); 
            desc.InstanceContributionToHitGroupIndex = static_cast<UINT>(instanceDesc.size());
            instanceDesc.push_back(desc);
        }
    }

    AllocateUploadBuffer(device, instanceDesc.data(), instanceDesc.size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC), &instanceDescsResource, L"InstanceDesc");

    // Build bottom-level acceleration structures for cube
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC cubeBLASDesc = {};
    cubeBLASDesc.Inputs = cubeBLASInputs;
    cubeBLASDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
    cubeBLASDesc.DestAccelerationStructureData = m_bottomLevelAccelerationStructureCube->GetGPUVirtualAddress();

    // Build bottom-level acceleration structures for complex shape
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC complexBLASDesc = {};
    complexBLASDesc.Inputs = complexBLASInputs;
    complexBLASDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
    complexBLASDesc.DestAccelerationStructureData = m_bottomLevelAccelerationStructureComplex->GetGPUVirtualAddress();

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
    topLevelBuildDesc.Inputs = topLevelInputs;
    topLevelBuildDesc.Inputs.InstanceDescs = instanceDescsResource->GetGPUVirtualAddress();
    topLevelBuildDesc.DestAccelerationStructureData = m_topLevelAccelerationStructure->GetGPUVirtualAddress();
    topLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();

    auto BuildAccelerationStructure = [&](auto* raytracingCommandList)
        {
            raytracingCommandList->BuildRaytracingAccelerationStructure(&cubeBLASDesc, 0, nullptr);
            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_bottomLevelAccelerationStructureCube.Get()));
            raytracingCommandList->BuildRaytracingAccelerationStructure(&complexBLASDesc, 0, nullptr);
            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_bottomLevelAccelerationStructureComplex.Get()));
            raytracingCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
        };

    // Build acceleration structure.
    BuildAccelerationStructure(m_dxrCommandList.Get());

    // Kick off acceleration structure construction.
    m_deviceResources->ExecuteCommandList();

    // Wait for GPU to finish as the locally created temporary GPU resources will get released once we go out of scope.
    m_deviceResources->WaitForGpu();
}

// Build shader tables.
// This encapsulates all shader records - shaders and the arguments for their local root signatures.
void D3D12RaytracingBasicShaderExecutionReordering::BuildShaderTables()
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
        struct RootArguments {
            ObjectConstantBuffer cb;
        } rootArguments;
        rootArguments.cb = m_cubeCB;

        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIdentifierSize + sizeof(rootArguments);
        ShaderTable rayGenShaderTable(device, numShaderRecords, shaderRecordSize, L"RayGenShaderTable");
        rayGenShaderTable.push_back(ShaderRecord(rayGenShaderIdentifier, shaderIdentifierSize, &rootArguments, sizeof(rootArguments)));
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
            ObjectConstantBuffer cb;
        };

        UINT numShaderRecords = 10082;
        UINT shaderRecordSize = shaderIdentifierSize + sizeof(RootArguments);
        ShaderTable hitGroupShaderTable(device, numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");


        for (int i = 0; i < 5041; ++i) {
            RootArguments argument;
            argument.cb = m_cubeCB;
            argument.cb.albedo = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f); // 16 bytes (4 floats × 4 bytes)
            argument.cb.materialID = 0;
            hitGroupShaderTable.push_back(ShaderRecord(

            hitGroupShaderIdentifier, shaderIdentifierSize, &argument, sizeof(argument)));
        }

        for (int i = 0; i < 5041; ++i) {
            RootArguments argument;
            argument.cb = m_complexShapeCB;
            argument.cb.albedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // 16 bytes (4 floats × 4 bytes)
            argument.cb.materialID = 1;
            hitGroupShaderTable.push_back(ShaderRecord(

            hitGroupShaderIdentifier, shaderIdentifierSize, &argument, sizeof(argument)));
        }

        // Add this line to fix the null pointer issue:
        m_hitGroupShaderTable = hitGroupShaderTable.GetResource();
    }
}

// Update frame-based values.
void D3D12RaytracingBasicShaderExecutionReordering::OnUpdate()
{
    m_timer.Tick();
    CalculateFrameStats();
    float elapsedTime = static_cast<float>(m_timer.GetElapsedSeconds());
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();
    auto prevFrameIndex = m_deviceResources->GetPreviousFrameIndex();

    auto kb = m_keyboard->GetState();
    m_keyboardButtons.Update(kb);

    if (m_keyboardButtons.IsKeyPressed(Keyboard::Keys::S))
    {
        OutputDebugStringA("S key pressed!\n");
        m_serEnabled = !m_serEnabled;
    }

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
    m_sceneCB[frameIndex].enableSER = m_serEnabled ? 1 : 0;
}

void D3D12RaytracingBasicShaderExecutionReordering::DoRaytracing()
{
    auto commandList = m_deviceResources->GetCommandList();
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    auto DispatchRays = [&](auto* commandList, auto* stateObject, auto* dispatchDesc)
        {
            // Since each shader table has only one shader record, the stride is same as the size.
            dispatchDesc->HitGroupTable.StartAddress = m_hitGroupShaderTable->GetGPUVirtualAddress();
            dispatchDesc->HitGroupTable.SizeInBytes = m_hitGroupShaderTable->GetDesc().Width;
            dispatchDesc->HitGroupTable.StrideInBytes = m_hitGroupShaderTable->GetDesc().Width / 10082;
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
            commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::VertexBuffersSlot, m_indexBuffer.gpuDescriptorHandle);
            commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::OutputViewSlot, m_raytracingOutputResourceUAVGpuDescriptor);
        };

    commandList->SetComputeRootSignature(m_raytracingGlobalRootSignature.Get());

    // Copy the updated scene constant buffer to GPU.
    memcpy(&m_mappedConstantData[frameIndex].constants, &m_sceneCB[frameIndex], sizeof(m_sceneCB[frameIndex]));
    auto cbGpuAddress = m_perFrameConstants->GetGPUVirtualAddress() + frameIndex * sizeof(m_mappedConstantData[0]);
    commandList->SetComputeRootConstantBufferView(GlobalRootSignatureParams::SceneConstantSlot, cbGpuAddress);

    // Bind the heaps, acceleration structure and dispatch rays.
    D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
    SetCommonPipelineState(commandList);
    commandList->SetComputeRootShaderResourceView(GlobalRootSignatureParams::AccelerationStructureSlot, m_topLevelAccelerationStructure->GetGPUVirtualAddress());
    DispatchRays(m_dxrCommandList.Get(), m_dxrStateObject.Get(), &dispatchDesc);
}

// Update the application state with the new resolution.
void D3D12RaytracingBasicShaderExecutionReordering::UpdateForSizeChange(UINT width, UINT height)
{
    DXSample::UpdateForSizeChange(width, height);
}

// Render the UI
void D3D12RaytracingBasicShaderExecutionReordering::RenderUI()
{
    auto commandList = m_deviceResources->GetCommandList();
    auto renderTarget = m_deviceResources->GetRenderTarget();
    auto viewport = m_deviceResources->GetScreenViewport();
    auto scissorRect = m_deviceResources->GetScissorRect();
    auto rtvDescriptor = m_deviceResources->GetRenderTargetView();


    // Transition render target to RENDER_TARGET state
    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTarget,
        D3D12_RESOURCE_STATE_PRESENT, 
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    commandList->ResourceBarrier(1, &barrier);


    // Set the render target
    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, nullptr);
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    m_spriteBatch->SetViewport(viewport);
    m_spriteBatch->Begin(commandList);

    XMFLOAT2 textPos = XMFLOAT2(30, 30);
    XMVECTOR textColor = XMVectorSet(1, 1, 1, 1);

    wchar_t buffer[256];

    m_smallFont->DrawString(m_spriteBatch.get(), L"D3D12: Basic Shader Execution Reordering Scene - Reordering based on HitObject + MaterialID", textPos, textColor);
    textPos.y += m_smallFont->GetLineSpacing() * 2;

    swprintf_s(buffer, ARRAYSIZE(buffer), L"SER: %s - Press 'S'", m_serEnabled ? L"Enabled" : L"Disabled");
    m_smallFont->DrawString(m_spriteBatch.get(), buffer, textPos, textColor);
    textPos.y += m_smallFont->GetLineSpacing();

    m_spriteBatch->End();


    // Transition render target back to PRESENT state
    D3D12_RESOURCE_BARRIER barrierBack = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTarget,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    );
    commandList->ResourceBarrier(1, &barrierBack);
}


// Copy the raytracing output to the backbuffer.
void D3D12RaytracingBasicShaderExecutionReordering::CopyRaytracingOutputToBackbuffer()
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
void D3D12RaytracingBasicShaderExecutionReordering::CreateWindowSizeDependentResources()
{
    CreateRaytracingOutputResource();
    UpdateCameraMatrices();
}

// Release resources that are dependent on the size of the main window.
void D3D12RaytracingBasicShaderExecutionReordering::ReleaseWindowSizeDependentResources()
{
    m_raytracingOutput.Reset();
}

// Release all resources that depend on the device.
void D3D12RaytracingBasicShaderExecutionReordering::ReleaseDeviceDependentResources()
{
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
    m_bottomLevelAccelerationStructureComplex.Reset();
    m_bottomLevelAccelerationStructureCube.Reset();
    m_topLevelAccelerationStructure.Reset();

}

void D3D12RaytracingBasicShaderExecutionReordering::RecreateD3D()
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
void D3D12RaytracingBasicShaderExecutionReordering::OnRender()
{
    if (!m_deviceResources->IsWindowVisible())
    {
        return;
    }

    m_deviceResources->Prepare();
    DoRaytracing();
    CopyRaytracingOutputToBackbuffer();
    RenderUI();

    m_deviceResources->Present(D3D12_RESOURCE_STATE_PRESENT);
    m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());
}

void D3D12RaytracingBasicShaderExecutionReordering::OnDestroy()
{
    // Let GPU finish before releasing D3D resources.
    m_deviceResources->WaitForGpu();
    OnDeviceLost();
}

// Release all device dependent resouces when a device is lost.
void D3D12RaytracingBasicShaderExecutionReordering::OnDeviceLost()
{
    ReleaseWindowSizeDependentResources();
    ReleaseDeviceDependentResources();
}

// Create all device dependent resources when a device is restored.
void D3D12RaytracingBasicShaderExecutionReordering::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

// Compute the average frames per second and million rays per second.
void D3D12RaytracingBasicShaderExecutionReordering::CalculateFrameStats()
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
            << L"    GPU[" << m_deviceResources->GetAdapterID() << L"]: " << m_deviceResources->GetAdapterDescription()
            << L"    SER: " << (m_serEnabled ? L"ON" : L"OFF");
        SetCustomWindowText(windowText.str().c_str());
    }
}

// Handle OnSizeChanged message event.
void D3D12RaytracingBasicShaderExecutionReordering::OnSizeChanged(UINT width, UINT height, bool minimized)
{
    if (!m_deviceResources->WindowSizeChanged(width, height, minimized))
    {
        return;
    }

    UpdateForSizeChange(width, height);

    ReleaseWindowSizeDependentResources();
    CreateWindowSizeDependentResources();
}

// Allocate a descriptor and return its index. 
// If the passed descriptorIndexToUse is valid, it will be used instead of allocating a new one.
UINT D3D12RaytracingBasicShaderExecutionReordering::AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse)
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
UINT D3D12RaytracingBasicShaderExecutionReordering::CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize)
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
    UINT descriptorIndex = AllocateDescriptor(&buffer->cpuDescriptorHandle);
    device->CreateShaderResourceView(buffer->resource.Get(), &srvDesc, buffer->cpuDescriptorHandle);
    buffer->gpuDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), descriptorIndex, m_descriptorSize);
    return descriptorIndex;
}

std::vector<UINT8> D3D12RaytracingBasicShaderExecutionReordering::GenerateTextureData()
{
    const UINT rowPitch = TextureWidth * TexturePixelSize;
    const UINT cellSize = 32; // size of each checker cell
    const UINT textureSize = rowPitch * TextureHeight;

    std::vector<UINT8> data(textureSize);
    UINT8* pData = &data[0];

    for (UINT y = 0; y < TextureHeight; y++)
    {
        for (UINT x = 0; x < TextureWidth; x++)
        {
            UINT pixelOffset = (y * rowPitch) + (x * TexturePixelSize);

            // Determine if this pixel is in a white or black cell
            bool isWhite = ((x / cellSize) % 2) == ((y / cellSize) % 2);

            UINT8 color = isWhite ? 255 : 0;
            pData[pixelOffset + 0] = color; // R
            pData[pixelOffset + 1] = color; // G
            pData[pixelOffset + 2] = color; // B
            pData[pixelOffset + 3] = 255;   // A
        }
    }
    return data;
}
