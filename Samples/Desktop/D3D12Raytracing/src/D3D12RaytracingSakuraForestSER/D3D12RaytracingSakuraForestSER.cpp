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
#include "D3D12RaytracingSakuraForestSER.h"
#include "DirectXRaytracingHelper.h"
#include "dxcapi.h"
#include <atlbase.h>
#include <vector>
#include <wincodec.h>

#include <DirectXTex.h>
#include <WICTextureLoader.h>
#include "SharedCode.h"
#include <random>

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 717; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\"; }

using namespace std;
using namespace DX;

const wchar_t* D3D12RaytracingSakuraForestSER::c_floorHitGroupName = L"MyHitGroup";
const wchar_t* D3D12RaytracingSakuraForestSER::c_trunkHitGroupName = L"TrunkHitGroup";
const wchar_t* D3D12RaytracingSakuraForestSER::c_leavesHitGroupName = L"LeavesHitGroup";
const wchar_t* D3D12RaytracingSakuraForestSER::c_bushHitGroupName = L"BushHitGroup";
const wchar_t* D3D12RaytracingSakuraForestSER::c_reflectiveCubeHitGroupName = L"TCubeHitGroup";
const wchar_t* D3D12RaytracingSakuraForestSER::c_raygenShaderName = L"MyRaygenShader";
const wchar_t* D3D12RaytracingSakuraForestSER::c_floorClosestHitShaderName = L"FloorClosestHitShader";
const wchar_t* D3D12RaytracingSakuraForestSER::c_trunkClosestHitShaderName = L"TrunkClosestHitShader";
const wchar_t* D3D12RaytracingSakuraForestSER::c_leavesClosestHitShaderName = L"LeavesClosestHitShader";
const wchar_t* D3D12RaytracingSakuraForestSER::c_bushClosestHitShaderName = L"BushClosestHitShader";
const wchar_t* D3D12RaytracingSakuraForestSER::c_refCubeClosestHitShaderName = L"TCubeClosestHitShader";
const wchar_t* D3D12RaytracingSakuraForestSER::c_missShaderName = L"MyMissShader";

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

D3D12RaytracingSakuraForestSER::D3D12RaytracingSakuraForestSER(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_raytracingOutputResourceUAVDescriptorHeapIndex(UINT_MAX),
    m_curRotationAngleRad(0.0f),
    m_serEnabled(true),
    m_sortByHit(true),
    m_sortByMaterial(false),
    m_sortByBoth(false),
    rotateCamera(true)
{
    UpdateForSizeChange(width, height);
}

void D3D12RaytracingSakuraForestSER::OnInit()
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
void D3D12RaytracingSakuraForestSER::UpdateCameraMatrices()
{
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    m_sceneCB[frameIndex].cameraPosition = m_eye;
    float fovAngleY = 45.0f;
    XMMATRIX view = XMMatrixLookAtLH(m_eye, m_at, m_up);
    XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), m_aspectRatio, 1.0f, 125.0f);
    XMMATRIX viewProj = view * proj;

    m_sceneCB[frameIndex].projectionToWorld = XMMatrixInverse(nullptr, viewProj);
}

// Initialize scene rendering parameters.
void D3D12RaytracingSakuraForestSER::InitializeScene()
{
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();
    // Setup camera.
    {
        // Initialize the view and projection inverse matrices.
        // m_eye currently at the middle of the forest
        m_eye = { 0.0f, 2.2f, -2.0f, 1.0f };
        m_at = { 1.0f, 2.5f, -6.0f, 1.0f };
        XMVECTOR right = { 1.0f, 0.0f, 0.0f, 0.0f };

        XMVECTOR direction = XMVector4Normalize(m_at - m_eye);
        m_up = XMVector3Normalize(XMVector3Cross(direction, right));

        // Rotate camera around Y axis.
        XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(45.0f));
        m_eye = XMVector3Transform(m_eye, rotate);

        // Keep the camera upright
        m_up = { 0.0f, 1.0f, 0.0f, 0.0f };
        UpdateCameraMatrices();
    }

    // Setup lights.
    {
        // Initialize the lighting parameters.
        XMFLOAT4 lightPosition;
        XMFLOAT4 lightAmbientColor;
        XMFLOAT4 lightDiffuseColor;

        lightPosition = XMFLOAT4(0.0f, 15.8f, 233.0f, 0.0f);
        m_sceneCB[frameIndex].lightPosition = XMLoadFloat4(&lightPosition);

        lightAmbientColor = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f); // Brighter ambient light
        m_sceneCB[frameIndex].lightAmbientColor = XMLoadFloat4(&lightAmbientColor);

        lightDiffuseColor = XMFLOAT4(1.2f, 1.0f, 1.0f, 1.0f); // Stronger diffuse light
        m_sceneCB[frameIndex].lightDiffuseColor = XMLoadFloat4(&lightDiffuseColor);
    }

    // Apply the initial values to all frames' buffer instances.
    for (auto& sceneCB : m_sceneCB)
    {
        sceneCB = m_sceneCB[frameIndex];
    }
}

// Create constant buffers.
void D3D12RaytracingSakuraForestSER::CreateConstantBuffers()
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
void D3D12RaytracingSakuraForestSER::CreateDeviceDependentResources()
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

    // Load gometry from OBJ files.
    m_ObjModelLoader.Load(L"trunk.obj");
    m_ObjModelLoader.Load(L"leaves.obj");
    m_ObjModelLoader.Load(L"bush.obj");

    // Build geometry for cubes (that will make the reflective cubes & the floor)
    BuildCubeGeometry();

    // Build geometry for trees, trunks, and bushes
    BuildTreeGeometry();

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

void D3D12RaytracingSakuraForestSER::CreateTexture()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandQueue = m_deviceResources->GetCommandQueue();

    // Begin a resource upload batch
    DirectX::ResourceUploadBatch resourceUpload(device);
    resourceUpload.Begin();

    // Load trunk texture from file using DirectXTK
    ComPtr<ID3D12Resource> trunkTexture;

    HRESULT hr = DirectX::CreateWICTextureFromFile(
        device,
        resourceUpload,
        L"tree-trunk.jpg",
        trunkTexture.GetAddressOf()
    );

    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to load texture using CreateWICTextureFromFile.\n");
        return;
    }

    // Load bush texture from file using DirectXTK
    ComPtr<ID3D12Resource> bushTexture;

    hr = DirectX::CreateWICTextureFromFile(
        device,
        resourceUpload,
        L"bush-texture.jpg",
        bushTexture.GetAddressOf()
    );

    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to load texture using CreateWICTextureFromFile.\n");
        return;
    }

    // Schedule the upload and wait for it to complete
    auto uploadResourcesFinished = resourceUpload.End(commandQueue);
    uploadResourcesFinished.wait();

    m_trunkTexture = trunkTexture;
    m_bushTexture = bushTexture;

    // Create SRV for the trunk texture
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDescTrunk = {};
    srvDescTrunk.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDescTrunk.Format = m_trunkTexture->GetDesc().Format;
    srvDescTrunk.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDescTrunk.Texture2D.MipLevels = 1;

    D3D12_CPU_DESCRIPTOR_HANDLE srvHandleTrunk;
    UINT descriptorIndexTrunk = AllocateDescriptor(&srvHandleTrunk);
    device->CreateShaderResourceView(m_trunkTexture.Get(), &srvDescTrunk, srvHandleTrunk);

    m_textureSrvGpuDescriptorTrunk = CD3DX12_GPU_DESCRIPTOR_HANDLE(
        m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
        descriptorIndexTrunk,
        m_descriptorSize);

    // Create SRV for the bush texture
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDescBush = {};
    srvDescBush.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDescBush.Format = m_bushTexture->GetDesc().Format;
    srvDescBush.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDescBush.Texture2D.MipLevels = 1;

    D3D12_CPU_DESCRIPTOR_HANDLE srvHandleBush;
    UINT descriptorIndexBush = AllocateDescriptor(&srvHandleBush);
    device->CreateShaderResourceView(m_bushTexture.Get(), &srvDescBush, srvHandleBush);

    m_textureSrvGpuDescriptorBush = CD3DX12_GPU_DESCRIPTOR_HANDLE(
        m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
        descriptorIndexBush,
        m_descriptorSize);
}

void D3D12RaytracingSakuraForestSER::CreateUIFont()
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

void D3D12RaytracingSakuraForestSER::SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig)
{
    auto device = m_deviceResources->GetD3DDevice();
    ComPtr<ID3DBlob> blob;
    ComPtr<ID3DBlob> error;

    ThrowIfFailed(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error), error ? static_cast<wchar_t*>(error->GetBufferPointer()) : nullptr);
    ThrowIfFailed(device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&(*rootSig))));
}

void D3D12RaytracingSakuraForestSER::CreateRootSignatures()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Global Root Signature
    // This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
    {
        CD3DX12_DESCRIPTOR_RANGE ranges[2]; // Perfomance TIP: Order from most frequent to least frequent.
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 1 output texture
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 10, 1);  // 4 static index and 4 vertex buffers + 2 textures 

        CD3DX12_ROOT_PARAMETER rootParameters[GlobalRootSignatureParams::Count];
        rootParameters[GlobalRootSignatureParams::OutputViewSlot].InitAsDescriptorTable(1, &ranges[0]);
        rootParameters[GlobalRootSignatureParams::AccelerationStructureSlot].InitAsShaderResourceView(0);
        rootParameters[GlobalRootSignatureParams::SceneConstantSlot].InitAsConstantBufferView(0);
        rootParameters[GlobalRootSignatureParams::VertexBuffersSlot].InitAsDescriptorTable(1, &ranges[1]);

        // Static sampler
        D3D12_STATIC_SAMPLER_DESC samplers[2] = {};

        // Sampler for s0
        samplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        samplers[0].MipLODBias = 0;
        samplers[0].MaxAnisotropy = 0;
        samplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        samplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        samplers[0].MinLOD = 0.0f;
        samplers[0].MaxLOD = D3D12_FLOAT32_MAX;
        samplers[0].ShaderRegister = 0;
        samplers[0].RegisterSpace = 0;
        samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        // Sampler for s1
        samplers[1] = samplers[0]; // Copy settings
        samplers[1].ShaderRegister = 1;

        CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(
            ARRAYSIZE(rootParameters), rootParameters,
            ARRAYSIZE(samplers), samplers);

        SerializeAndCreateRaytracingRootSignature(globalRootSignatureDesc, &m_raytracingGlobalRootSignature);
    }

    // Local Root Signature
    // This is a root signature that enables a shader to have unique arguments that come from shader tables.
    {
        CD3DX12_ROOT_PARAMETER rootParameters[LocalRootSignatureParams::Count];
        rootParameters[LocalRootSignatureParams::CubeConstantSlot].InitAsConstants(SizeOfInUint32(m_objectCB), 1);
        CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
        localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
        SerializeAndCreateRaytracingRootSignature(localRootSignatureDesc, &m_raytracingLocalRootSignature);
    }
}

// Create raytracing device and command list.
void D3D12RaytracingSakuraForestSER::CreateRaytracingInterfaces()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();

    ThrowIfFailed(device->QueryInterface(IID_PPV_ARGS(&m_dxrDevice)), L"Couldn't get DirectX Raytracing interface for the device.\n");
    ThrowIfFailed(commandList->QueryInterface(IID_PPV_ARGS(&m_dxrCommandList)), L"Couldn't get DirectX Raytracing interface for the command list.\n");
}

// Local root signature and shader association
// This is a root signature that enables a shader to have unique arguments that come from shader tables.
void D3D12RaytracingSakuraForestSER::CreateLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
{
    // Local root signature to be used in a hit group.
    auto localRootSignature = raytracingPipeline->CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
    localRootSignature->SetRootSignature(m_raytracingLocalRootSignature.Get());
    // Define explicit shader association for the local root signature. 
    {
        auto rootSignatureAssociation = raytracingPipeline->CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
        rootSignatureAssociation->AddExport(c_floorHitGroupName);
        rootSignatureAssociation->AddExport(c_trunkHitGroupName);
        rootSignatureAssociation->AddExport(c_leavesHitGroupName);
        rootSignatureAssociation->AddExport(c_bushHitGroupName);
        rootSignatureAssociation->AddExport(c_reflectiveCubeHitGroupName);
        rootSignatureAssociation->AddExport(c_raygenShaderName);
    }
}

// Create a raytracing pipeline state object (RTPSO).
// An RTPSO represents a full set of shaders reachable by a DispatchRays() call,
// with all configuration options resolved, such as local signatures and other state.
void D3D12RaytracingSakuraForestSER::CreateRaytracingPipelineStateObject()
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
        lib->DefineExport(c_floorClosestHitShaderName);
        lib->DefineExport(c_trunkClosestHitShaderName);
        lib->DefineExport(c_leavesClosestHitShaderName);
        lib->DefineExport(c_bushClosestHitShaderName);
        lib->DefineExport(c_refCubeClosestHitShaderName);
        lib->DefineExport(c_missShaderName);
    }

    // Triangle hit group
    // A hit group specifies closest hit, any hit and intersection shaders to be executed when a ray intersects the geometry's triangle/AABB.
    // In this sample, we only use triangle geometry with a closest hit shader, so others are not set.
    auto floorHitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    floorHitGroup->SetClosestHitShaderImport(c_floorClosestHitShaderName);
    floorHitGroup->SetHitGroupExport(c_floorHitGroupName);
    floorHitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

    auto trunkHitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    trunkHitGroup->SetClosestHitShaderImport(c_trunkClosestHitShaderName);
    trunkHitGroup->SetHitGroupExport(c_trunkHitGroupName);
    trunkHitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

    auto leavesHitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    leavesHitGroup->SetClosestHitShaderImport(c_leavesClosestHitShaderName);
    leavesHitGroup->SetHitGroupExport(c_leavesHitGroupName);
    leavesHitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

    auto bushHitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    bushHitGroup->SetClosestHitShaderImport(c_bushClosestHitShaderName);
    bushHitGroup->SetHitGroupExport(c_bushHitGroupName);
    bushHitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

    auto refCubeHitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    refCubeHitGroup->SetClosestHitShaderImport(c_refCubeClosestHitShaderName);
    refCubeHitGroup->SetHitGroupExport(c_reflectiveCubeHitGroupName);
    refCubeHitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

    // Shader config
   //  Defines the maximum sizes in bytes for the ray payload and attribute structure.
    auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
	UINT payloadSize = sizeof(XMFLOAT4) + sizeof(UINT) + sizeof(UINT); // Since payload is a float4 color, an UINT for recursion depth, and an UINT for reflectHint.

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
    UINT maxRecursionDepth = MAX_RECURSION_DEPTH;
    pipelineConfig->Config(maxRecursionDepth);

#if _DEBUG
    PrintStateObjectDesc(raytracingPipeline);
#endif

    // Create the state object.
    ThrowIfFailed(m_dxrDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_dxrStateObject)), L"Couldn't create DirectX Raytracing state object.\n");
}

// Create 2D output texture for raytracing.
void D3D12RaytracingSakuraForestSER::CreateRaytracingOutputResource()
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

void D3D12RaytracingSakuraForestSER::CreateDescriptorHeap()
{
    auto device = m_deviceResources->GetD3DDevice();

    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
    // Allocate a heap for 11 descriptors:
    // 2 - vertex and index buffer SRVs for cube
    // 2 - vertex and index buffer SRVs for trunk
    // 2 - vertex and index buffer SRVs for leaves
    // 2 - vertex and index buffer SRVs for bushes
    // 1 - raytracing output texture UAV
    // 2 - texture SRV

    descriptorHeapDesc.NumDescriptors = 11;
    descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    descriptorHeapDesc.NodeMask = 0;
    device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_descriptorHeap));
    NAME_D3D12_OBJECT(m_descriptorHeap);

    m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

// Build geometry used in the sample.
void D3D12RaytracingSakuraForestSER::BuildCubeGeometry()
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


// Build geometry for trees, trunks, and bushes
void D3D12RaytracingSakuraForestSER::BuildTreeGeometry()
{
    auto device = m_deviceResources->GetD3DDevice();
    std::vector<Vertex> trunkVertices;
    std::vector<Index> trunkIndices;
    std::vector<Vertex> leavesVertices;
    std::vector<Index> leavesIndices;
    std::vector<Vertex> bushVertices;
    std::vector<Index> bushIndices;

    {
        XMVECTOR xAxis = { 1, 0, 0 };
        XMVECTOR yAxis = { 0, 1, 0 };
        XMVECTOR zAxis = { 0, 0, 1 };
        XMMATRIX transform = XMMatrixRotationAxis(xAxis, 3.14159f / 2.0f) * XMMatrixRotationAxis(yAxis, 3.14159f / 12.0f) * XMMatrixRotationAxis(zAxis, 3.14159f) * XMMatrixTranslation(-1.5f, 0, 0);
        m_ObjModelLoader.GetObjectVerticesAndIndices(
            "tree",
            0.007f,
            &trunkVertices,
            &trunkIndices);
        m_ObjModelLoader.GetObjectVerticesAndIndices(
            "leaves",
            0.007f,
            &leavesVertices,
            &leavesIndices);
        m_ObjModelLoader.GetObjectVerticesAndIndices(
            "bush",
            0.007f,
            &bushVertices,
            &bushIndices);
    }

    int trunkIndexBufferSize = static_cast<int>(trunkIndices.size()) * sizeof(Index);
    AllocateUploadBuffer(device, trunkIndices.data(), trunkIndexBufferSize, &m_trunkIndexBuffer.resource);
    AllocateUploadBuffer(device, trunkVertices.data(), trunkVertices.size() * sizeof(trunkVertices[0]), &m_trunkVertexBuffer.resource);

    int leavesIndexBufferSize = static_cast<int>(leavesIndices.size()) * sizeof(Index);
    AllocateUploadBuffer(device, leavesIndices.data(), leavesIndexBufferSize, &m_leavesIndexBuffer.resource);
    AllocateUploadBuffer(device, leavesVertices.data(), leavesVertices.size() * sizeof(leavesVertices[0]), &m_leavesVertexBuffer.resource);

    int bushIndexBufferSize = static_cast<int>(bushIndices.size()) * sizeof(Index);
    AllocateUploadBuffer(device, bushIndices.data(), bushIndexBufferSize, &m_bushIndexBuffer.resource);
    AllocateUploadBuffer(device, bushVertices.data(), bushVertices.size() * sizeof(bushVertices[0]), &m_bushVertexBuffer.resource);

    // Vertex buffer is passed to the shader along with index buffer as a descriptor table.
    // Vertex buffer descriptor must follow index buffer descriptor in the descriptor heap.
    m_totalTrunkVertexCount = static_cast<int>(trunkVertices.size());
    UINT trunkDescriptorIndexIB = CreateBufferSRV(
        &m_trunkIndexBuffer,
        static_cast<UINT>(trunkIndices.size() * sizeof(Index) / 4),
        0
    );

    UINT trunkDescriptorIndexVB = CreateBufferSRV(
        &m_trunkVertexBuffer, 
        static_cast<UINT>(trunkVertices.size()), 
        sizeof(trunkVertices[0])
    );
    ThrowIfFalse(trunkDescriptorIndexVB == trunkDescriptorIndexIB + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index!");

    m_totalLeavesVertexCount = static_cast<int>(leavesVertices.size());
    UINT leavesDescriptorIndexIB = CreateBufferSRV(
        &m_leavesIndexBuffer,
        static_cast<UINT>(leavesIndices.size() * sizeof(Index) / 4),
        0
    );

    UINT leavesDescriptorIndexVB = CreateBufferSRV(
        &m_leavesVertexBuffer, 
        static_cast<UINT>(leavesVertices.size()), 
        sizeof(leavesVertices[0])
    );
    ThrowIfFalse(leavesDescriptorIndexVB == leavesDescriptorIndexIB + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index!");

    m_totalBushVertexCount = static_cast<int>(bushVertices.size());
    UINT bushDescriptorIndexIB = CreateBufferSRV(
        &m_bushIndexBuffer,
        static_cast<UINT>(bushIndices.size() * sizeof(Index) / 4),
        0
    );

    UINT bushDescriptorIndexVB = CreateBufferSRV(
        &m_bushVertexBuffer, 
        static_cast<UINT>(bushVertices.size()), 
        sizeof(bushVertices[0])
    );
    ThrowIfFalse(bushDescriptorIndexVB == bushDescriptorIndexIB + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index!");
}


// Build acceleration structures needed for raytracing.
void D3D12RaytracingSakuraForestSER::BuildAccelerationStructures()
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
    cubeGeometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
    cubeGeometryDesc.Triangles.Transform3x4 = 0;
    cubeGeometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    cubeGeometryDesc.Triangles.VertexCount = static_cast<UINT>(m_vertexBuffer.resource->GetDesc().Width) / sizeof(Vertex);
    cubeGeometryDesc.Triangles.VertexBuffer.StartAddress = m_vertexBuffer.resource->GetGPUVirtualAddress();
    cubeGeometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);

    // Mark the geometry as opaque. 
    // PERFORMANCE TIP: mark geometry as opaque whenever applicable as it can enable important ray processing optimizations.
    // Note: When rays encounter opaque geometry an any hit shader will not be executed whether it is present or not.
    cubeGeometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

    // Setup trunk geometry desc
    D3D12_RAYTRACING_GEOMETRY_DESC trunkGeometryDesc = {};
    trunkGeometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    trunkGeometryDesc.Triangles.IndexBuffer = m_trunkIndexBuffer.resource->GetGPUVirtualAddress();
    trunkGeometryDesc.Triangles.IndexCount = static_cast<UINT>(m_trunkIndexBuffer.resource->GetDesc().Width) / sizeof(Index);
    trunkGeometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
    trunkGeometryDesc.Triangles.Transform3x4 = 0;
    trunkGeometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    trunkGeometryDesc.Triangles.VertexCount = static_cast<UINT>(m_trunkVertexBuffer.resource->GetDesc().Width) / sizeof(Vertex);
    trunkGeometryDesc.Triangles.VertexBuffer.StartAddress = m_trunkVertexBuffer.resource->GetGPUVirtualAddress();
    trunkGeometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
    trunkGeometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

    // Setup leaves shape geometry desc
    D3D12_RAYTRACING_GEOMETRY_DESC leavesGeometryDesc = {};
    leavesGeometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    leavesGeometryDesc.Triangles.IndexBuffer = m_leavesIndexBuffer.resource->GetGPUVirtualAddress();
    leavesGeometryDesc.Triangles.IndexCount = static_cast<UINT>(m_leavesIndexBuffer.resource->GetDesc().Width) / sizeof(Index);
    leavesGeometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
    leavesGeometryDesc.Triangles.Transform3x4 = 0;
    leavesGeometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    leavesGeometryDesc.Triangles.VertexCount = static_cast<UINT>(m_leavesVertexBuffer.resource->GetDesc().Width) / sizeof(Vertex);
    leavesGeometryDesc.Triangles.VertexBuffer.StartAddress = m_leavesVertexBuffer.resource->GetGPUVirtualAddress();
    leavesGeometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
    leavesGeometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

    // Setup bush shape geometry desc 
    D3D12_RAYTRACING_GEOMETRY_DESC bushGeometryDesc = {};
    bushGeometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    bushGeometryDesc.Triangles.IndexBuffer = m_bushIndexBuffer.resource->GetGPUVirtualAddress();
    bushGeometryDesc.Triangles.IndexCount = static_cast<UINT>(m_bushIndexBuffer.resource->GetDesc().Width) / sizeof(Index);
    bushGeometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
    bushGeometryDesc.Triangles.Transform3x4 = 0;
    bushGeometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    bushGeometryDesc.Triangles.VertexCount = static_cast<UINT>(m_bushVertexBuffer.resource->GetDesc().Width) / sizeof(Vertex);
    bushGeometryDesc.Triangles.VertexBuffer.StartAddress = m_bushVertexBuffer.resource->GetGPUVirtualAddress();
    bushGeometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
    bushGeometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

    // Get required sizes for an acceleration structure.
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

    // Get prebuild info for the bottom-level acceleration structure.
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS cubeBLASInputs = {};
    cubeBLASInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    cubeBLASInputs.Flags = buildFlags;
    cubeBLASInputs.NumDescs = 1; // 1 geometry desc
    cubeBLASInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    cubeBLASInputs.pGeometryDescs = &cubeGeometryDesc;

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS trunkBLASInputs = {};
    trunkBLASInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    trunkBLASInputs.Flags = buildFlags;
    trunkBLASInputs.NumDescs = 1; // 1 geometry desc
    trunkBLASInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    trunkBLASInputs.pGeometryDescs = &trunkGeometryDesc;

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS leavesBLASInputs = {};
    leavesBLASInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    leavesBLASInputs.Flags = buildFlags;
    leavesBLASInputs.NumDescs = 1; // 1 geometry desc
    leavesBLASInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    leavesBLASInputs.pGeometryDescs = &leavesGeometryDesc;

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bushBLASInputs = {};
    bushBLASInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    bushBLASInputs.Flags = buildFlags;
    bushBLASInputs.NumDescs = 1; // 1 geometry desc
    bushBLASInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    bushBLASInputs.pGeometryDescs = &bushGeometryDesc;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO cubeBLASPrebuildInfo = {};
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO trunkBLASPrebuildInfo = {};
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO leavesBLASPrebuildInfo = {};
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bushBLASPrebuildInfo = {};

    m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&cubeBLASInputs, &cubeBLASPrebuildInfo);
    m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&trunkBLASInputs, &trunkBLASPrebuildInfo);
    m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&leavesBLASInputs, &leavesBLASPrebuildInfo);
    m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bushBLASInputs, &bushBLASPrebuildInfo);

    // Tp-level acceleration structure.
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
    topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    topLevelInputs.Flags = buildFlags;
    topLevelInputs.NumDescs = numTopLevelInstances;
    topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
    m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
    ThrowIfFalse(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

    ComPtr<ID3D12Resource> scratchResource;
    SIZE_T scratchSize = max(
        leavesBLASPrebuildInfo.ScratchDataSizeInBytes,
        max(
            cubeBLASPrebuildInfo.ScratchDataSizeInBytes,
            max(
                trunkBLASPrebuildInfo.ScratchDataSizeInBytes,
                max(
                    topLevelPrebuildInfo.ScratchDataSizeInBytes,
                    bushBLASPrebuildInfo.ScratchDataSizeInBytes
                )
            )
        )
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
        AllocateUAVBuffer(device, trunkBLASPrebuildInfo.ResultDataMaxSizeInBytes, &m_bottomLevelAccelerationStructureTrunk, initialResourceState, L"BottomLevelAccelerationStructureTrunk");
        AllocateUAVBuffer(device, leavesBLASPrebuildInfo.ResultDataMaxSizeInBytes, &m_bottomLevelAccelerationStructureLeaves, initialResourceState, L"BottomLevelAccelerationStructureLeaves");
        AllocateUAVBuffer(device, bushBLASPrebuildInfo.ResultDataMaxSizeInBytes, &m_bottomLevelAccelerationStructureBushes, initialResourceState, L"BottomLevelAccelerationStructureBush");
        AllocateUAVBuffer(device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_topLevelAccelerationStructure, initialResourceState, L"TopLevelAccelerationStructure");
    }

    // Create an instance desc for the bottom-level acceleration structure.
    ComPtr<ID3D12Resource> instanceDescsResource;
    std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDesc;

    int cubesPerRow = 20; // Number of cubes per row along Z and X axis that make up the floor
    float largerCubeSpacing = 4.0f; // Spacing between larger cubes
    float randomCubeSpacing = 0.1f; // Spacing between random smaller cubes
	int treesPerRow = 30; // Number of trees per row along Z and X axis
	float treeSpacing = 1.9f; // Spacing between trees

	// Create random number generator for random offsets
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> randomOffset(-0.1f, 0.7f);

    std::random_device rdBush;
    std::mt19937 genBush(rdBush());
    std::uniform_real_distribution<float> randomOffsetBush(0.7f, 0.75f);

    // Larger cubes for the floor
    for (int x = -cubesPerRow / 2; x <= cubesPerRow / 2; ++x)
    {
        for (int z = -cubesPerRow / 2; z <= cubesPerRow / 2; ++z)
        {
            float posX = x * largerCubeSpacing + 10.0f;
            float posY = 0.0f;
            float posZ = z * largerCubeSpacing;

            D3D12_RAYTRACING_INSTANCE_DESC desc = {};
            float scale = 2.0f; // Larger cube scale
            desc.Transform[0][0] = scale;
            desc.Transform[1][1] = scale;
            desc.Transform[2][2] = scale;

            desc.Transform[0][3] = posX;
            desc.Transform[1][3] = posY;
            desc.Transform[2][3] = posZ;

            desc.InstanceMask = 1;
            desc.AccelerationStructure = m_bottomLevelAccelerationStructureCube->GetGPUVirtualAddress();
            desc.InstanceID = instanceDesc.size();
            desc.InstanceContributionToHitGroupIndex = static_cast<UINT>(instanceDesc.size());
            instanceDesc.push_back(desc);
        }
    }

    // Smaller reflective cubes that are placed randomly on the floor
	int reflectiveCubesPerRow = 30;
    for (int x = -reflectiveCubesPerRow / 2; x <= reflectiveCubesPerRow / 2; ++x)
    {
        for (int z = -reflectiveCubesPerRow / 2; z <= reflectiveCubesPerRow / 2; ++z)
        {
            float randomYOffset = randomOffset(gen);

            float posX = 0.5f + x * randomCubeSpacing;
            float posY = 1.7f + randomYOffset;
            float posZ = z * randomCubeSpacing;

            D3D12_RAYTRACING_INSTANCE_DESC desc = {};
            float scale = 0.01f; // Smaller cube scale 
            desc.Transform[0][0] = scale;
            desc.Transform[1][1] = scale;
            desc.Transform[2][2] = scale;

            desc.Transform[0][3] = posX; 
            desc.Transform[1][3] = posY; 
            desc.Transform[2][3] = posZ; 

            desc.InstanceMask = 1;
            desc.AccelerationStructure = m_bottomLevelAccelerationStructureCube->GetGPUVirtualAddress();
            desc.InstanceID = static_cast<UINT>(instanceDesc.size());
            desc.InstanceContributionToHitGroupIndex = static_cast<UINT>(instanceDesc.size());
            instanceDesc.push_back(desc);
        }
    }

    // Trunk and leaves
    // Store random positions for trunks
    std::vector<std::tuple<float, float, float>> trunkPositions;

    // First loop: Initialize trunks
    for (int x = -treesPerRow / 2; x <= treesPerRow / 2; ++x)
    {
        for (int z = -treesPerRow / 2; z <= treesPerRow / 2; ++z)
        {
            float randomXOffset = randomOffset(gen);
            float randomYOffset = randomOffset(gen);
            float randomZOffset = randomOffset(gen);

            float posX = x * treeSpacing + (randomXOffset / 6);
            float posY = 2.0f - (randomYOffset / 3);
            float posZ = z * treeSpacing + (randomZOffset / 6);
            trunkPositions.emplace_back(posX, posY, posZ);

            // Trunk instance
            D3D12_RAYTRACING_INSTANCE_DESC desc = {};
            float scale = 35.0f;
            desc.Transform[0][0] = scale;
            desc.Transform[1][1] = scale;
            desc.Transform[2][2] = scale;

            desc.Transform[0][3] = posX;
            desc.Transform[1][3] = posY;
            desc.Transform[2][3] = posZ;

            desc.InstanceMask = 1;
            desc.AccelerationStructure = m_bottomLevelAccelerationStructureTrunk->GetGPUVirtualAddress();
            desc.InstanceID = static_cast<UINT>(instanceDesc.size());
            desc.InstanceContributionToHitGroupIndex = static_cast<UINT>(instanceDesc.size());
            instanceDesc.push_back(desc);
        }
    }


    // Second loop: Initialize leaves
    for (size_t i = 0; i < trunkPositions.size(); ++i)
    {
        // Leaves instance
        D3D12_RAYTRACING_INSTANCE_DESC desc = {};
        float scale = 35.0f;
        desc.Transform[0][0] = scale;
        desc.Transform[1][1] = scale;
        desc.Transform[2][2] = scale;

        // Use the same position as the corresponding trunk
        desc.Transform[0][3] = std::get<0>(trunkPositions[i]); 
        desc.Transform[1][3] = std::get<1>(trunkPositions[i]); 
        desc.Transform[2][3] = std::get<2>(trunkPositions[i]); 

        desc.InstanceMask = 1;
        desc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE;
        desc.AccelerationStructure = m_bottomLevelAccelerationStructureLeaves->GetGPUVirtualAddress();
        desc.InstanceID = instanceDesc.size();
        desc.InstanceContributionToHitGroupIndex = static_cast<UINT>(instanceDesc.size());
        instanceDesc.push_back(desc);
    }

    // Bushes placed randomly on the floor
    int bushesPerRow = 100;
    for (int x = -bushesPerRow / 2; x <= bushesPerRow / 2; ++x)
    {
        for (int z = -bushesPerRow / 2; z <= bushesPerRow / 2; ++z)
        {
            float randomXOffset = randomOffset(gen);
            float randomYOffset = randomOffsetBush(genBush);
            float randomZOffset = randomOffset(gen);

            float posX = x * 0.2f + randomXOffset;
            float posY = 1.7f + (randomYOffset / 3);
            float posZ = z * 0.2f + randomZOffset;

            D3D12_RAYTRACING_INSTANCE_DESC desc = {};
            float scale = 2.1f; // Bush scale
            desc.Transform[0][0] = scale;
            desc.Transform[1][1] = scale;
            desc.Transform[2][2] = scale;

            desc.Transform[0][3] = posX; 
            desc.Transform[1][3] = posY; 
            desc.Transform[2][3] = posZ; 
            desc.InstanceMask = 1;
            desc.AccelerationStructure = m_bottomLevelAccelerationStructureBushes->GetGPUVirtualAddress();
            desc.InstanceID = static_cast<UINT>(instanceDesc.size());
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

    // Build bottom-level acceleration structures for trunk
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC trunkBLASDesc = {};
    trunkBLASDesc.Inputs = trunkBLASInputs;
    trunkBLASDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
    trunkBLASDesc.DestAccelerationStructureData = m_bottomLevelAccelerationStructureTrunk->GetGPUVirtualAddress();

    // Build bottom-level acceleration structures for leaves 
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC leavesBLASDesc = {};
    leavesBLASDesc.Inputs = leavesBLASInputs;
    leavesBLASDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
    leavesBLASDesc.DestAccelerationStructureData = m_bottomLevelAccelerationStructureLeaves->GetGPUVirtualAddress();

    // Build bottom-level acceleration structures for bushes 
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bushBLASDesc = {};
    bushBLASDesc.Inputs = bushBLASInputs;
    bushBLASDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
    bushBLASDesc.DestAccelerationStructureData = m_bottomLevelAccelerationStructureBushes->GetGPUVirtualAddress();

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
    topLevelBuildDesc.Inputs = topLevelInputs;
    topLevelBuildDesc.Inputs.InstanceDescs = instanceDescsResource->GetGPUVirtualAddress();
    topLevelBuildDesc.DestAccelerationStructureData = m_topLevelAccelerationStructure->GetGPUVirtualAddress();
    topLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();

    auto BuildAccelerationStructure = [&](auto* raytracingCommandList)
        {
            raytracingCommandList->BuildRaytracingAccelerationStructure(&cubeBLASDesc, 0, nullptr);
            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_bottomLevelAccelerationStructureCube.Get()));
            raytracingCommandList->BuildRaytracingAccelerationStructure(&trunkBLASDesc, 0, nullptr);
            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_bottomLevelAccelerationStructureTrunk.Get()));
            raytracingCommandList->BuildRaytracingAccelerationStructure(&leavesBLASDesc, 0, nullptr);
            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_bottomLevelAccelerationStructureLeaves.Get()));
            raytracingCommandList->BuildRaytracingAccelerationStructure(&bushBLASDesc, 0, nullptr);
            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_bottomLevelAccelerationStructureBushes.Get()));
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
void D3D12RaytracingSakuraForestSER::BuildShaderTables()
{
    auto device = m_deviceResources->GetD3DDevice();

    void* rayGenShaderIdentifier;
    void* missShaderIdentifier;
    void* floorHitGroupShaderIdentifier;
    void* trunkHitGroupShaderIdentifier;
    void* leavesHitGroupShaderIdentifier;
    void* bushHitGroupShaderIdentifier;
    void* refCubeHitGroupShaderIdentifier;

    auto GetShaderIdentifiers = [&](auto* stateObjectProperties)
    {
        rayGenShaderIdentifier = stateObjectProperties->GetShaderIdentifier(c_raygenShaderName);
        missShaderIdentifier = stateObjectProperties->GetShaderIdentifier(c_missShaderName);
        floorHitGroupShaderIdentifier = stateObjectProperties->GetShaderIdentifier(c_floorHitGroupName);
        trunkHitGroupShaderIdentifier = stateObjectProperties->GetShaderIdentifier(c_trunkHitGroupName);
        leavesHitGroupShaderIdentifier = stateObjectProperties->GetShaderIdentifier(c_leavesHitGroupName);
        bushHitGroupShaderIdentifier = stateObjectProperties->GetShaderIdentifier(c_bushHitGroupName);
        refCubeHitGroupShaderIdentifier = stateObjectProperties->GetShaderIdentifier(c_reflectiveCubeHitGroupName);
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
        struct RootArguments 
        {
            ObjectConstantBuffer cb;
        } rootArguments;
        rootArguments.cb = m_objectCB;

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
        struct RootArguments 
        {
            ObjectConstantBuffer cb;
        };

		// Total number of shader records in the hit group shader table
        UINT numShaderRecords = numTopLevelInstances;
		UINT CUBE_NUMER_RECORDS = 441; // Number of larger cubes
		UINT REFLECTIVE_CUBE_NUMER_RECORDS = 961; // Number of smaller reflective cubes
		UINT TRUNK_NUMER_RECORDS = 961; // Number of tree trunks
		UINT LEAVES_NUMER_RECORDS = 961; // Number of tree leaves
		UINT BUSH_NUMER_RECORDS = 10201; // Number of bushes
        UINT shaderRecordSize = shaderIdentifierSize + sizeof(RootArguments);
        ShaderTable hitGroupShaderTable(device, numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");

        // Larger cube shader records as the ground
        for (UINT i = 0; i < CUBE_NUMER_RECORDS; ++i)
        {
            RootArguments argument;
            argument.cb = m_cubeCB;
            argument.cb.albedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
            argument.cb.materialID = 0;
            hitGroupShaderTable.push_back(ShaderRecord(floorHitGroupShaderIdentifier, shaderIdentifierSize, &argument, sizeof(argument)));
        }

		// Reflective cube shader records randomly placed around the scene
        for (UINT i = 0; i < REFLECTIVE_CUBE_NUMER_RECORDS; ++i)
        {
            RootArguments argument;
            argument.cb = m_reflectiveCubeCB;
            argument.cb.albedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); 
            argument.cb.materialID = 1;
            hitGroupShaderTable.push_back(ShaderRecord(refCubeHitGroupShaderIdentifier, shaderIdentifierSize, &argument, sizeof(argument)));
        }

        // Tree trunk shader records
        for (UINT i = 0; i < TRUNK_NUMER_RECORDS; ++i)
        {
            RootArguments argument;
            const void* shaderIdentifier = nullptr;
            argument.cb = m_trunkCB;
            argument.cb.albedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); 
            argument.cb.materialID = 2;
            hitGroupShaderTable.push_back(ShaderRecord(trunkHitGroupShaderIdentifier, shaderIdentifierSize, &argument, sizeof(argument)));
        }

        // Tree leaves shader records
        for (UINT i = 0; i < LEAVES_NUMER_RECORDS; ++i)
        {
            RootArguments argument;
            const void* shaderIdentifier = nullptr;
            argument.cb = m_leavesCB;
            argument.cb.albedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); 
            argument.cb.materialID = 3;
            hitGroupShaderTable.push_back(ShaderRecord(leavesHitGroupShaderIdentifier, shaderIdentifierSize, &argument, sizeof(argument)));
        }


        // Bush shader records 
        for (UINT i = 0; i < BUSH_NUMER_RECORDS; ++i)
        {
            RootArguments argument;
            argument.cb = m_bushCB;
            argument.cb.albedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
            argument.cb.materialID = 4;
            hitGroupShaderTable.push_back(ShaderRecord(bushHitGroupShaderIdentifier, shaderIdentifierSize, &argument, sizeof(argument)));
        }

        m_hitGroupShaderTable = hitGroupShaderTable.GetResource();
    }
}

// Update frame-based values.
void D3D12RaytracingSakuraForestSER::OnUpdate()
{
    m_timer.Tick();
    CalculateFrameStats();
    float elapsedTime = static_cast<float>(m_timer.GetElapsedSeconds());
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();
    auto prevFrameIndex = m_deviceResources->GetPreviousFrameIndex();

    auto kb = m_keyboard->GetState();
    m_keyboardButtons.Update(kb);

	// If press P, we toggle SER 
    if (m_keyboardButtons.IsKeyPressed(Keyboard::Keys::P))
    {
        OutputDebugStringA("P key pressed!\n");
        m_serEnabled = !m_serEnabled;
        if (m_serEnabled)
        {
            // Default to sorting by HitObject when SER is enabled
            m_sortByHit = true;
            m_sortByMaterial = false;
            m_sortByBoth = false;
        }
        else
        {
            // Disable all sorting when SER is off
            m_sortByHit = false;
            m_sortByMaterial = false;
            m_sortByBoth = false;
        }
    }

	// If press H, we sort by HitObject.
    if (m_keyboardButtons.IsKeyPressed(Keyboard::Keys::H))
    {
        OutputDebugStringA("H key pressed!\n");
        m_serEnabled = true;
        m_sortByHit = true;
        m_sortByMaterial = false;
        m_sortByBoth = false;
    }

	// If press M, we sort by reflectHint
    if (m_keyboardButtons.IsKeyPressed(Keyboard::Keys::M))
    {
        OutputDebugStringA("M key pressed!\n");
        m_serEnabled = true;
        m_sortByHit = false;
        m_sortByMaterial = true;
        m_sortByBoth = false;
    }

	// If press B, we sort by both HitObject and reflectHint
    if (m_keyboardButtons.IsKeyPressed(Keyboard::Keys::B))
    {
        OutputDebugStringA("B key pressed!\n");
        m_serEnabled = true;
        m_sortByHit = false;
        m_sortByMaterial = false;
        m_sortByBoth = true;
    }

    // Camera movement speed
    float movementSpeed = 5.0f * elapsedTime;

    // Moving forward and backward (W/S)
    if (kb.W)
    {
        XMVECTOR forward = XMVector3Normalize(m_at - m_eye);
        m_eye += forward * movementSpeed;
        m_at += forward * movementSpeed;
    }
    if (kb.S)
    {
        XMVECTOR backward = XMVector3Normalize(m_eye - m_at);
        m_eye += backward * movementSpeed;
        m_at += backward * movementSpeed;
    }

    // Moving left and right (A/D)
    if (kb.A)
    {
        XMVECTOR forward = XMVector3Normalize(m_at - m_eye);
        XMVECTOR left = XMVector3Normalize(XMVector3Cross(forward, m_up));
        m_eye += left * movementSpeed;
        m_at += left * movementSpeed;
    }
    if (kb.D)
    {
        XMVECTOR forward = XMVector3Normalize(m_at - m_eye);
        XMVECTOR right = XMVector3Normalize(XMVector3Cross(m_up, forward));
        m_eye += right * movementSpeed;
        m_at += right * movementSpeed;
    }

    // Moving up and down 
    if (kb.Q)
    {
        m_eye += m_up * movementSpeed;
        m_at += m_up * movementSpeed;
    }
    if (kb.E)
    {
        m_eye -= m_up * movementSpeed;
        m_at -= m_up * movementSpeed;
    }

    // Looking down and up
    if (kb.Down)
    {
        XMVECTOR forward = XMVector3Normalize(m_at - m_eye);
        XMVECTOR right = XMVector3Normalize(XMVector3Cross(m_up, forward));

        float pitchSpeed = XMConvertToRadians(30.0f) * elapsedTime; // degrees/sec

        XMMATRIX pitchMatrix = XMMatrixRotationAxis(right, pitchSpeed);
        XMVECTOR newForward = XMVector3TransformNormal(forward, pitchMatrix);

        m_at = m_eye + newForward;
    }
    if (kb.Up)
    {
        XMVECTOR forward = XMVector3Normalize(m_at - m_eye);
        XMVECTOR right = XMVector3Normalize(XMVector3Cross(m_up, forward));

        float pitchSpeed = XMConvertToRadians(-30.0f) * elapsedTime;

        XMMATRIX pitchMatrix = XMMatrixRotationAxis(right, pitchSpeed);
        XMVECTOR newForward = XMVector3TransformNormal(forward, pitchMatrix);

        m_at = m_eye + newForward;
    }

	// Looking left and right
    if (kb.Left)
    {
        XMVECTOR forward = XMVector3Normalize(m_at - m_eye);
        float yawSpeed = XMConvertToRadians(-30.0f) * elapsedTime; // negative = left
        XMMATRIX yawMatrix = XMMatrixRotationAxis(m_up, yawSpeed);
        XMVECTOR newForward = XMVector3TransformNormal(forward, yawMatrix);
        m_at = m_eye + newForward;
    }
    if (kb.Right)
    {
        XMVECTOR forward = XMVector3Normalize(m_at - m_eye);
        float yawSpeed = XMConvertToRadians(30.0f) * elapsedTime; // positive = right
        XMMATRIX yawMatrix = XMMatrixRotationAxis(m_up, yawSpeed);
        XMVECTOR newForward = XMVector3TransformNormal(forward, yawMatrix);
        m_at = m_eye + newForward;
    }

    const float minCameraHeight = 2.02f; // Minimum height above ground
    float currentHeight = XMVectorGetY(m_eye);
    if (currentHeight < minCameraHeight)
    {
        // Clamp the camera height
        m_eye = XMVectorSetY(m_eye, minCameraHeight);

        // Adjust the look-at point to maintain the same viewing angle
        float atHeight = XMVectorGetY(m_at);
        float heightDifference = minCameraHeight - currentHeight;
        m_at = XMVectorSetY(m_at, atHeight + heightDifference);
    }

    UpdateCameraMatrices();

    // Rotate the second light around Y axis.
    {
        float secondsToRotateAround = 8.0f;
        float angleToRotateBy = -360.0f * (elapsedTime / secondsToRotateAround);
        XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(angleToRotateBy));
        const XMVECTOR& prevLightPosition = m_sceneCB[prevFrameIndex].lightPosition;
        m_sceneCB[frameIndex].lightPosition = XMVector3Transform(prevLightPosition, rotate);
    }
    if (!m_serEnabled)
    {
        m_sceneCB[frameIndex].sortMode = SORTMODE_OFF;
    }
    else if (m_sortByHit)
    {
        m_sceneCB[frameIndex].sortMode = SORTMODE_BY_HIT;
    }
    else if (m_sortByMaterial)
    {
        m_sceneCB[frameIndex].sortMode = SORTMODE_BY_MATERIAL;
    }
    else if (m_sortByBoth)
    {
        m_sceneCB[frameIndex].sortMode = SORTMODE_BY_BOTH;
    }
}

void D3D12RaytracingSakuraForestSER::DoRaytracing()
{
    auto commandList = m_deviceResources->GetCommandList();
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    auto DispatchRays = [&](auto* commandList, auto* stateObject, auto* dispatchDesc)
    {
        // Since each shader table has only one shader record, the stride is same as the size.
        dispatchDesc->HitGroupTable.StartAddress = m_hitGroupShaderTable->GetGPUVirtualAddress();
        dispatchDesc->HitGroupTable.SizeInBytes = m_hitGroupShaderTable->GetDesc().Width;
        dispatchDesc->HitGroupTable.StrideInBytes = m_hitGroupShaderTable->GetDesc().Width / numTopLevelInstances;
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
void D3D12RaytracingSakuraForestSER::UpdateForSizeChange(UINT width, UINT height)
{
    DXSample::UpdateForSizeChange(width, height);
}

// Render the UI
void D3D12RaytracingSakuraForestSER::RenderUI()
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
    XMVECTOR textColor = XMVectorSet(1, 1, 0, 1);

    wchar_t buffer[256];

    // Draw the titles of UI
    m_smallFont->DrawString(m_spriteBatch.get(), L"D3D12: Shader Execution Reordering", textPos, textColor);
    textPos.y += m_smallFont->GetLineSpacing() * 2;

    swprintf_s(buffer, ARRAYSIZE(buffer), L"Toggle SER: %s - Press 'P'", m_serEnabled ? L"Enabled" : L"Disabled");
    m_smallFont->DrawString(m_spriteBatch.get(), buffer, textPos, textColor);
    textPos.y += m_smallFont->GetLineSpacing();

    swprintf_s(buffer, ARRAYSIZE(buffer), L"Sort by HitObject: %s - Press 'H'", m_sortByHit ? L"Enabled" : L"Disabled");
    m_smallFont->DrawString(m_spriteBatch.get(), buffer, textPos, textColor);
    textPos.y += m_smallFont->GetLineSpacing();

    swprintf_s(buffer, ARRAYSIZE(buffer), L"Sort by reflectHint: %s - Press 'M'", m_sortByMaterial ? L"Enabled" : L"Disabled");
    m_smallFont->DrawString(m_spriteBatch.get(), buffer, textPos, textColor);
    textPos.y += m_smallFont->GetLineSpacing();

    swprintf_s(buffer, ARRAYSIZE(buffer), L"Sort by both: %s - Press 'B'", m_sortByBoth ? L"Enabled" : L"Disabled");
    m_smallFont->DrawString(m_spriteBatch.get(), buffer, textPos, textColor);
    textPos.y += m_smallFont->GetLineSpacing();

    swprintf_s(buffer, ARRAYSIZE(buffer), L"Use WASD to move around");
    m_smallFont->DrawString(m_spriteBatch.get(), buffer, textPos, textColor);
    textPos.y += m_smallFont->GetLineSpacing();

    swprintf_s(buffer, ARRAYSIZE(buffer), L"Use Arrow keys to look around");
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
void D3D12RaytracingSakuraForestSER::CopyRaytracingOutputToBackbuffer()
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
void D3D12RaytracingSakuraForestSER::CreateWindowSizeDependentResources()
{
    CreateRaytracingOutputResource();
    UpdateCameraMatrices();
}

// Release resources that are dependent on the size of the main window.
void D3D12RaytracingSakuraForestSER::ReleaseWindowSizeDependentResources()
{
    m_raytracingOutput.Reset();
}

// Release all resources that depend on the device.
void D3D12RaytracingSakuraForestSER::ReleaseDeviceDependentResources()
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
    m_bottomLevelAccelerationStructureTrunk.Reset();
    m_bottomLevelAccelerationStructureLeaves.Reset();
    m_bottomLevelAccelerationStructureCube.Reset();
    m_topLevelAccelerationStructure.Reset();

}

void D3D12RaytracingSakuraForestSER::RecreateD3D()
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
void D3D12RaytracingSakuraForestSER::OnRender()
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

void D3D12RaytracingSakuraForestSER::OnDestroy()
{
    // Let GPU finish before releasing D3D resources.
    m_deviceResources->WaitForGpu();
    OnDeviceLost();
}

// Release all device dependent resouces when a device is lost.
void D3D12RaytracingSakuraForestSER::OnDeviceLost()
{
    ReleaseWindowSizeDependentResources();
    ReleaseDeviceDependentResources();
}

// Create all device dependent resources when a device is restored.
void D3D12RaytracingSakuraForestSER::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

// Compute the average frames per second and million rays per second.
void D3D12RaytracingSakuraForestSER::CalculateFrameStats()
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
void D3D12RaytracingSakuraForestSER::OnSizeChanged(UINT width, UINT height, bool minimized)
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
UINT D3D12RaytracingSakuraForestSER::AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse)
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
UINT D3D12RaytracingSakuraForestSER::CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize)
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