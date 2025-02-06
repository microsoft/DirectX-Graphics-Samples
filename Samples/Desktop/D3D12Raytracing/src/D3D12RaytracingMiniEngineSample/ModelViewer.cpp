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

#define NOMINMAX

#include "d3d12.h"
#include "d3d12video.h"
#include <d3d12.h>
#include "dxgi1_3.h"
#include "d3dx12.h"
#include "GameCore.h"
#include "CameraController.h"
#include "BufferManager.h"
#include "Camera.h"
#include "CommandContext.h"
#include "Display.h"
#include "TemporalEffects.h"
#include "MotionBlur.h"
#include "DepthOfField.h"
#include "PostEffects.h"
#include "ParticleEffectManager.h"
#include "SSAO.h"
#include "FXAA.h"
#include "SystemTime.h"
#include "TextRenderer.h"
#include "ShadowCamera.h"
#include "ParticleEffectManager.h"
#include "GameInput.h"
#include "SponzaRenderer.h"
#include "ModelH3D.h"
#include "Renderer.h"

#include "DXSampleHelper.h"

#include "CompiledShaders/RayGenerationShaderLib.h"
#include "CompiledShaders/RayGenerationShaderSSRLib.h"
#include "CompiledShaders/HitShaderLib.h"
#include "CompiledShaders/MissShaderLib.h"
#include "CompiledShaders/DiffuseHitShaderLib.h"
#include "CompiledShaders/RayGenerationShadowsLib.h"
#include "CompiledShaders/MissShadowsLib.h"

#include "RaytracingHlslCompat.h"
#include "ModelViewerRayTracing.h"

using namespace GameCore;
using namespace Math;
using namespace Graphics;

using Microsoft::WRL::ComPtr;

ComPtr<ID3D12Device5> g_pRaytracingDevice;

__declspec(align(16)) struct HitShaderConstants
{
    Vector3 sunDirection;
    Vector3 sunLight;
    Vector3 ambientLight;
    float ShadowTexelSize[4];
    Matrix4 modelToShadow;
    UINT32 IsReflection;
    UINT32 UseShadowRays;
};

ByteAddressBuffer          g_hitConstantBuffer;
ByteAddressBuffer          g_dynamicConstantBuffer;

D3D12_GPU_DESCRIPTOR_HANDLE g_GpuSceneMaterialSrvs[27];
D3D12_CPU_DESCRIPTOR_HANDLE g_SceneMeshInfo;

D3D12_GPU_DESCRIPTOR_HANDLE g_OutputUAV;
D3D12_GPU_DESCRIPTOR_HANDLE g_DepthAndNormalsTable;
D3D12_GPU_DESCRIPTOR_HANDLE g_SceneSrvs;

std::vector<ComPtr<ID3D12Resource>>   g_bvh_bottomLevelAccelerationStructures;
ComPtr<ID3D12Resource>   g_bvh_topLevelAccelerationStructure;

DynamicCB           g_dynamicCb;
ComPtr<ID3D12RootSignature> g_GlobalRaytracingRootSignature;
ComPtr<ID3D12RootSignature> g_LocalRaytracingRootSignature;

enum RaytracingTypes
{
    Primarybarycentric = 0,
    Reflectionbarycentric,
    Shadows,
    DiffuseHitShader,
    Reflection,
    NumTypes
};

const static UINT MaxRayRecursion = 2;

const static UINT c_NumCameraPositions = 5;

struct RaytracingDispatchRayInputs
{
    RaytracingDispatchRayInputs() {}
    RaytracingDispatchRayInputs(
        ID3D12Device5& /*device*/,
        ID3D12StateObject *pPSO,
        void *pHitGroupShaderTable,
        UINT HitGroupStride,
        UINT HitGroupTableSize,
        LPCWSTR rayGenExportName,
        LPCWSTR missExportName) : m_pPSO(pPSO)
    {
        const UINT shaderTableSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        ID3D12StateObjectProperties* stateObjectProperties = nullptr;
        ThrowIfFailed(pPSO->QueryInterface(IID_PPV_ARGS(&stateObjectProperties)));
        void *pRayGenShaderData = stateObjectProperties->GetShaderIdentifier(rayGenExportName);
        void *pMissShaderData = stateObjectProperties->GetShaderIdentifier(missExportName);

        m_HitGroupStride = HitGroupStride;

        // MiniEngine requires that all initial data be aligned to 16 bytes
        UINT alignment = 16;
        std::vector<BYTE> alignedShaderTableData(shaderTableSize + alignment - 1);
        BYTE *pAlignedShaderTableData = alignedShaderTableData.data() + ((UINT64)alignedShaderTableData.data() % alignment);
        memcpy(pAlignedShaderTableData, pRayGenShaderData, shaderTableSize);
        m_RayGenShaderTable.Create(L"Ray Gen Shader Table", 1, shaderTableSize, alignedShaderTableData.data());
        
        memcpy(pAlignedShaderTableData, pMissShaderData, shaderTableSize);
        m_MissShaderTable.Create(L"Miss Shader Table", 1, shaderTableSize, alignedShaderTableData.data());
        
        m_HitShaderTable.Create(L"Hit Shader Table", 1, HitGroupTableSize, pHitGroupShaderTable);
    }

    D3D12_DISPATCH_RAYS_DESC GetDispatchRayDesc(UINT DispatchWidth, UINT DispatchHeight)
    {
        D3D12_DISPATCH_RAYS_DESC dispatchRaysDesc = {};

        dispatchRaysDesc.RayGenerationShaderRecord.StartAddress = m_RayGenShaderTable.GetGpuVirtualAddress();
        dispatchRaysDesc.RayGenerationShaderRecord.SizeInBytes = m_RayGenShaderTable.GetBufferSize();
        dispatchRaysDesc.HitGroupTable.StartAddress = m_HitShaderTable.GetGpuVirtualAddress();
        dispatchRaysDesc.HitGroupTable.SizeInBytes = m_HitShaderTable.GetBufferSize();
        dispatchRaysDesc.HitGroupTable.StrideInBytes = m_HitGroupStride;
        dispatchRaysDesc.MissShaderTable.StartAddress = m_MissShaderTable.GetGpuVirtualAddress();
        dispatchRaysDesc.MissShaderTable.SizeInBytes = m_MissShaderTable.GetBufferSize();
        dispatchRaysDesc.MissShaderTable.StrideInBytes = dispatchRaysDesc.MissShaderTable.StrideInBytes;
        dispatchRaysDesc.Width = DispatchWidth;
        dispatchRaysDesc.Height = DispatchHeight;
        dispatchRaysDesc.Depth = 1;
        return dispatchRaysDesc;
    }

    UINT m_HitGroupStride;
    ComPtr<ID3D12StateObject> m_pPSO;
    ByteAddressBuffer   m_RayGenShaderTable;
    ByteAddressBuffer   m_MissShaderTable;
    ByteAddressBuffer   m_HitShaderTable;
};

struct MaterialRootConstant
{
    UINT MaterialID;
};

RaytracingDispatchRayInputs g_RaytracingInputs[RaytracingTypes::NumTypes];
D3D12_CPU_DESCRIPTOR_HANDLE g_bvh_attributeSrvs[34];

class D3D12RaytracingMiniEngineSample : public GameCore::IGameApp
{
public:

    D3D12RaytracingMiniEngineSample( void ) {}

    virtual void Startup( void ) override;
    virtual void Cleanup( void ) override;

    virtual void Update( float deltaT ) override;
    virtual void RenderScene( void ) override;
    virtual void RenderUI(class GraphicsContext&) override;
    virtual void Raytrace(class GraphicsContext&);

    virtual bool RequiresRaytracingSupport() const override { return true; }

    void SetCameraToPredefinedPosition(int cameraPosition);

private:

    void RaytraceDiffuse(GraphicsContext& context, const Math::Camera& camera, ColorBuffer& colorTarget);
    void RaytraceShadows(GraphicsContext& context, const Math::Camera& camera, ColorBuffer& colorTarget, DepthBuffer& depth);
    void RaytraceReflections(GraphicsContext& context, const Math::Camera& camera, ColorBuffer& colorTarget, DepthBuffer& depth, ColorBuffer& normals);

    Camera m_Camera;
    std::unique_ptr<FlyingFPSCamera> m_CameraController;
    D3D12_VIEWPORT m_MainViewport;
    D3D12_RECT m_MainScissor;

    struct CameraPosition
    {
        Vector3 position;
        float heading;
        float pitch;
    };

    CameraPosition m_CameraPosArray[c_NumCameraPositions];
    UINT m_CameraPosArrayCurrentPosition;

};


// Returns bool whether the device supports DirectX Raytracing tier.
inline bool IsDirectXRaytracingSupported(IDXGIAdapter1* adapter)
{
    ComPtr<ID3D12Device> testDevice;
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupportData = {};

    return SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&testDevice)))
        && SUCCEEDED(testDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureSupportData, sizeof(featureSupportData)))
        && featureSupportData.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPWSTR /*lpCmdLine*/, _In_ int nCmdShow)
{
#if _DEBUG
    ComPtr<ID3D12Debug> debugInterface;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
    {
        debugInterface->EnableDebugLayer();
    }
#endif

    ComPtr<ID3D12Device> pDevice;
    ComPtr<IDXGIAdapter1> pAdapter;
    ComPtr<IDXGIFactory2> pFactory;
    CreateDXGIFactory2(0, IID_PPV_ARGS(&pFactory));
    bool validDeviceFound = false;
    for (uint32_t Idx = 0; !validDeviceFound && DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(Idx, &pAdapter); ++Idx)
    {
        DXGI_ADAPTER_DESC1 desc;
        pAdapter->GetDesc1(&desc);

        if (IsDirectXRaytracingSupported(pAdapter.Get()))
        {
            validDeviceFound = true;
        }
        pAdapter = nullptr;
    }

    //s_EnableVSync.Decrement();
    //TargetResolution = k720p;
    //g_DisplayWidth = 1280;
    //g_DisplayHeight = 720;
    GameCore::RunApplication(D3D12RaytracingMiniEngineSample(), L"D3D12RaytracingMiniEngineSample", hInstance, nCmdShow); 
    return 0;
}

const char* rayTracingModes[] = {
    "Off", 
    "Bary Rays", 
    "Refl Bary", 
    "Shadow Rays", 
    "Diffuse&ShadowMaps",
    "Diffuse&ShadowRays",
    "Reflection Rays"};
enum RaytracingMode
{
    RTM_OFF,
    RTM_TRAVERSAL,
    RTM_SSR,
    RTM_SHADOWS,
    RTM_DIFFUSE_WITH_SHADOWMAPS,
    RTM_DIFFUSE_WITH_SHADOWRAYS,
    RTM_REFLECTIONS,
};
EnumVar rayTracingMode("Application/Raytracing/RayTraceMode", RTM_DIFFUSE_WITH_SHADOWMAPS, _countof(rayTracingModes), rayTracingModes);

class DescriptorHeapStack
{
public:
    DescriptorHeapStack(ID3D12Device &device, UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT NodeMask) :
        m_device(device)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = numDescriptors;
        desc.Type = type;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask = NodeMask;
        device.CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pDescriptorHeap));

        m_descriptorSize = device.GetDescriptorHandleIncrementSize(type);
        m_descriptorHeapCpuBase = m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    }

    ID3D12DescriptorHeap &GetDescriptorHeap() { return *m_pDescriptorHeap.Get(); }

    void AllocateDescriptor(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE &cpuHandle, _Out_ UINT &descriptorHeapIndex)
    {
        descriptorHeapIndex = m_descriptorsAllocated;
        cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_descriptorHeapCpuBase, descriptorHeapIndex, m_descriptorSize);
        m_descriptorsAllocated++;
    }

    UINT AllocateBufferSrv(_In_ ID3D12Resource &resource)
    {
        UINT descriptorHeapIndex;
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
        AllocateDescriptor(cpuHandle, descriptorHeapIndex);
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.NumElements = (UINT)(resource.GetDesc().Width / sizeof(UINT32));
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
        srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        m_device.CreateShaderResourceView(&resource, &srvDesc, cpuHandle);
        return descriptorHeapIndex;
    }

    UINT AllocateBufferUav(_In_ ID3D12Resource &resource)
    {
        UINT descriptorHeapIndex;
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
        AllocateDescriptor(cpuHandle, descriptorHeapIndex);
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.NumElements = (UINT)(resource.GetDesc().Width / sizeof(UINT32));
        uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
        uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;

        m_device.CreateUnorderedAccessView(&resource, nullptr, &uavDesc, cpuHandle);
        return descriptorHeapIndex;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(UINT descriptorIndex)
    {
        return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), descriptorIndex, m_descriptorSize);
    }
private:
    ID3D12Device & m_device;
    ComPtr<ID3D12DescriptorHeap> m_pDescriptorHeap;
    UINT m_descriptorsAllocated = 0;
    UINT m_descriptorSize;
    D3D12_CPU_DESCRIPTOR_HANDLE m_descriptorHeapCpuBase;
};

std::unique_ptr<DescriptorHeapStack> g_pRaytracingDescriptorHeap;

StructuredBuffer    g_hitShaderMeshInfoBuffer;

static
void InitializeSceneInfo(
    const ModelH3D& model)
{
    //
    // Mesh info
    //
    std::vector<RayTraceMeshInfo>   meshInfoData(model.m_Header.meshCount);
    for (UINT i=0; i < model.m_Header.meshCount; ++i)
    {
        meshInfoData[i].m_indexOffsetBytes = model.m_pMesh[i].indexDataByteOffset;
        meshInfoData[i].m_uvAttributeOffsetBytes = model.m_pMesh[i].vertexDataByteOffset + model.m_pMesh[i].attrib[ModelH3D::attrib_texcoord0].offset;
        meshInfoData[i].m_normalAttributeOffsetBytes = model.m_pMesh[i].vertexDataByteOffset + model.m_pMesh[i].attrib[ModelH3D::attrib_normal].offset;
        meshInfoData[i].m_positionAttributeOffsetBytes = model.m_pMesh[i].vertexDataByteOffset + model.m_pMesh[i].attrib[ModelH3D::attrib_position].offset;
        meshInfoData[i].m_tangentAttributeOffsetBytes = model.m_pMesh[i].vertexDataByteOffset + model.m_pMesh[i].attrib[ModelH3D::attrib_tangent].offset;
        meshInfoData[i].m_bitangentAttributeOffsetBytes = model.m_pMesh[i].vertexDataByteOffset + model.m_pMesh[i].attrib[ModelH3D::attrib_bitangent].offset;
        meshInfoData[i].m_attributeStrideBytes = model.m_pMesh[i].vertexStride;
        meshInfoData[i].m_materialInstanceId = model.m_pMesh[i].materialIndex;
        ASSERT(meshInfoData[i].m_materialInstanceId < 27);
    }

    g_hitShaderMeshInfoBuffer.Create(L"RayTraceMeshInfo",
        (UINT)meshInfoData.size(),
        sizeof(meshInfoData[0]),
        meshInfoData.data());

    g_SceneMeshInfo = g_hitShaderMeshInfoBuffer.GetSRV();
}

static
void InitializeViews(const ModelH3D& model)
{
    D3D12_CPU_DESCRIPTOR_HANDLE uavHandle;
    UINT uavDescriptorIndex;
    g_pRaytracingDescriptorHeap->AllocateDescriptor(uavHandle, uavDescriptorIndex);
    Graphics::g_Device->CopyDescriptorsSimple(1, uavHandle, g_SceneColorBuffer.GetUAV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    g_OutputUAV = g_pRaytracingDescriptorHeap->GetGpuHandle(uavDescriptorIndex);

    {
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
        UINT srvDescriptorIndex;
        g_pRaytracingDescriptorHeap->AllocateDescriptor(srvHandle, srvDescriptorIndex);
        Graphics::g_Device->CopyDescriptorsSimple(1, srvHandle, g_SceneDepthBuffer.GetDepthSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        g_DepthAndNormalsTable = g_pRaytracingDescriptorHeap->GetGpuHandle(srvDescriptorIndex);

        UINT unused;
        g_pRaytracingDescriptorHeap->AllocateDescriptor(srvHandle, unused);
        Graphics::g_Device->CopyDescriptorsSimple(1, srvHandle, g_SceneNormalBuffer.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    {
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
        UINT srvDescriptorIndex;
        g_pRaytracingDescriptorHeap->AllocateDescriptor(srvHandle, srvDescriptorIndex);
        Graphics::g_Device->CopyDescriptorsSimple(1, srvHandle, g_SceneMeshInfo, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        g_SceneSrvs = g_pRaytracingDescriptorHeap->GetGpuHandle(srvDescriptorIndex);

        UINT unused;
        g_pRaytracingDescriptorHeap->AllocateDescriptor(srvHandle, unused);
        Sponza::GetModel().CreateIndexBufferSRV(srvHandle);

        g_pRaytracingDescriptorHeap->AllocateDescriptor(srvHandle, unused);
        Sponza::GetModel().CreateVertexBufferSRV(srvHandle);

        g_pRaytracingDescriptorHeap->AllocateDescriptor(srvHandle, unused);
        Graphics::g_Device->CopyDescriptorsSimple(1, srvHandle, g_ShadowBuffer.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        g_pRaytracingDescriptorHeap->AllocateDescriptor(srvHandle, unused);
        Graphics::g_Device->CopyDescriptorsSimple(1, srvHandle, g_SSAOFullScreen.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        for (UINT i = 0; i < model.m_Header.materialCount; i++)
        {
            const TextureRef* textures = model.GetMaterialTextures(i);

            UINT slot;
            g_pRaytracingDescriptorHeap->AllocateDescriptor(srvHandle, slot);       // Diffuse
            Graphics::g_Device->CopyDescriptorsSimple(1, srvHandle, textures[0].GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            g_pRaytracingDescriptorHeap->AllocateDescriptor(srvHandle, unused);     // Normal
            Graphics::g_Device->CopyDescriptorsSimple(1, srvHandle, textures[2].GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            
            g_GpuSceneMaterialSrvs[i] = g_pRaytracingDescriptorHeap->GetGpuHandle(slot);
        }
    }
}

template <size_t bytecodeLength>
void SetDxilLibrary(CD3DX12_STATE_OBJECT_DESC& stateObjectDesc, const unsigned char (&pShaderByteCode)[bytecodeLength], LPCWSTR entrypoint)
{
    auto dxilLibSubobject = stateObjectDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();

    D3D12_SHADER_BYTECODE shader = CD3DX12_SHADER_BYTECODE(pShaderByteCode, bytecodeLength);
    dxilLibSubobject->SetDXILLibrary(&shader);
    dxilLibSubobject->DefineExport(entrypoint);
}

template <size_t bytecodeLength>
void ReplaceDxilLibrary(D3D12_STATE_OBJECT_DESC* pStateObjectDesc, const unsigned char(&pShaderByteCode)[bytecodeLength], LPCWSTR entrypoint)
{
    for (UINT i = 0; i < pStateObjectDesc->NumSubobjects; ++i)
    {
        const D3D12_STATE_SUBOBJECT* pSubobject = &pStateObjectDesc->pSubobjects[i];

        if (pSubobject->Type == D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY)
        {
            D3D12_DXIL_LIBRARY_DESC* pLibDesc = (D3D12_DXIL_LIBRARY_DESC*)pSubobject->pDesc;

            for (UINT j = 0; j < pLibDesc->NumExports; ++j)
            {
                if (wcscmp(entrypoint, pLibDesc->pExports[j].Name) == 0)
                {
                    pLibDesc->DXILLibrary = CD3DX12_SHADER_BYTECODE(pShaderByteCode, bytecodeLength);
                }
            }
        }
    }
}

void SetPipelineStateStackSize(LPCWSTR raygen, LPCWSTR closestHit, LPCWSTR miss, UINT maxRecursion, ID3D12StateObject *pStateObject)
{
    ID3D12StateObjectProperties* stateObjectProperties = nullptr;
    ThrowIfFailed(pStateObject->QueryInterface(IID_PPV_ARGS(&stateObjectProperties)));
    UINT64 closestHitStackSize = stateObjectProperties->GetShaderStackSize(closestHit);
    UINT64 missStackSize = stateObjectProperties->GetShaderStackSize(miss);
    UINT64 raygenStackSize = stateObjectProperties->GetShaderStackSize(raygen);

    UINT64 totalStackSize = raygenStackSize + std::max(missStackSize, closestHitStackSize) * maxRecursion;
    stateObjectProperties->SetPipelineStackSize(totalStackSize);
}

void InitializeRaytracingStateObjects(const ModelH3D &model, UINT numMeshes)
{
    ZeroMemory(&g_dynamicCb, sizeof(g_dynamicCb));

    D3D12_STATIC_SAMPLER_DESC staticSamplerDescs[2] = {};
    D3D12_STATIC_SAMPLER_DESC &defaultSampler = staticSamplerDescs[0];
    defaultSampler.Filter = D3D12_FILTER_ANISOTROPIC;
    defaultSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    defaultSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    defaultSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    defaultSampler.MipLODBias = 0.0f;
    defaultSampler.MaxAnisotropy = 16;
    defaultSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    defaultSampler.MinLOD = 0.0f;
    defaultSampler.MaxLOD = D3D12_FLOAT32_MAX;
    defaultSampler.MaxAnisotropy = 8;
    defaultSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
    defaultSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    defaultSampler.ShaderRegister = 0;

    D3D12_STATIC_SAMPLER_DESC &shadowSampler = staticSamplerDescs[1];
    shadowSampler = staticSamplerDescs[0];
    shadowSampler.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    shadowSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
    shadowSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    shadowSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    shadowSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    shadowSampler.ShaderRegister = 1;

    D3D12_DESCRIPTOR_RANGE1 sceneBuffersDescriptorRange = {};
    sceneBuffersDescriptorRange.BaseShaderRegister = 1;
    sceneBuffersDescriptorRange.NumDescriptors = 5;
    sceneBuffersDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    sceneBuffersDescriptorRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;

    D3D12_DESCRIPTOR_RANGE1 srvDescriptorRange = {};
    srvDescriptorRange.BaseShaderRegister = 12;
    srvDescriptorRange.NumDescriptors = 2;
    srvDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvDescriptorRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;

    D3D12_DESCRIPTOR_RANGE1 uavDescriptorRange = {};
    uavDescriptorRange.BaseShaderRegister = 2;
    uavDescriptorRange.NumDescriptors = 10;
    uavDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    uavDescriptorRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;

    CD3DX12_ROOT_PARAMETER1 globalRootSignatureParameters[8];
    globalRootSignatureParameters[0].InitAsDescriptorTable(1, &sceneBuffersDescriptorRange);
    globalRootSignatureParameters[1].InitAsConstantBufferView(0);
    globalRootSignatureParameters[2].InitAsConstantBufferView(1);
    globalRootSignatureParameters[3].InitAsDescriptorTable(1, &srvDescriptorRange);
    globalRootSignatureParameters[4].InitAsDescriptorTable(1, &uavDescriptorRange);
    globalRootSignatureParameters[5].InitAsUnorderedAccessView(0);
    globalRootSignatureParameters[6].InitAsUnorderedAccessView(1);
    globalRootSignatureParameters[7].InitAsShaderResourceView(0);
    auto globalRootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(
        ARRAYSIZE(globalRootSignatureParameters), globalRootSignatureParameters,
        ARRAYSIZE(staticSamplerDescs), staticSamplerDescs);

    ComPtr<ID3DBlob> pGlobalRootSignatureBlob;
    ComPtr<ID3DBlob> pErrorBlob;
    if (FAILED(D3D12SerializeVersionedRootSignature(&globalRootSignatureDesc, &pGlobalRootSignatureBlob, &pErrorBlob)))
    {
        OutputDebugStringA((LPCSTR)pErrorBlob->GetBufferPointer());
    }
    ASSERT_SUCCEEDED(g_pRaytracingDevice->CreateRootSignature(
        0,
        pGlobalRootSignatureBlob->GetBufferPointer(),
        pGlobalRootSignatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&g_GlobalRaytracingRootSignature)));

    D3D12_DESCRIPTOR_RANGE1 localTextureDescriptorRange = {};
    localTextureDescriptorRange.BaseShaderRegister = 6;
    localTextureDescriptorRange.NumDescriptors = 2;
    localTextureDescriptorRange.RegisterSpace = 0;
    localTextureDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    localTextureDescriptorRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;

    CD3DX12_ROOT_PARAMETER1 localRootSignatureParameters[2];
    UINT sizeOfRootConstantInDwords = (sizeof(MaterialRootConstant) - 1) / sizeof(DWORD) + 1;
    localRootSignatureParameters[0].InitAsDescriptorTable(1, &localTextureDescriptorRange);
    localRootSignatureParameters[1].InitAsConstants(sizeOfRootConstantInDwords, 3);
    auto localRootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(ARRAYSIZE(localRootSignatureParameters), localRootSignatureParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);

    ComPtr<ID3DBlob> pLocalRootSignatureBlob;
    D3D12SerializeVersionedRootSignature(&localRootSignatureDesc, &pLocalRootSignatureBlob, nullptr);
    g_pRaytracingDevice->CreateRootSignature(0, pLocalRootSignatureBlob->GetBufferPointer(), pLocalRootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&g_LocalRaytracingRootSignature));

    CD3DX12_STATE_OBJECT_DESC stateObjectDesc(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);

    auto nodeMaskSubobject = stateObjectDesc.CreateSubobject<CD3DX12_NODE_MASK_SUBOBJECT>();
    nodeMaskSubobject->SetNodeMask(1);

    auto rootSignatureSubObject = stateObjectDesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
    rootSignatureSubObject->SetRootSignature(g_GlobalRaytracingRootSignature.Get());

    auto configurationSubObject = stateObjectDesc.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
    configurationSubObject->Config(MaxRayRecursion);

    LPCWSTR rayGenExportName = L"RayGen";
    SetDxilLibrary(stateObjectDesc, g_pRayGenerationShaderLib, rayGenExportName);

    LPCWSTR hitExportName = L"Hit";
    SetDxilLibrary(stateObjectDesc, g_phitShaderLib, hitExportName);

    LPCWSTR missExportName = L"Miss";
    SetDxilLibrary(stateObjectDesc, g_pmissShaderLib, missExportName);

    auto shaderConfigStateObject = stateObjectDesc.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    shaderConfigStateObject->Config(8, 8);

    LPCWSTR hitGroupExportName = L"HitGroup";
    auto hitGroupSubobject = stateObjectDesc.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    hitGroupSubobject->SetHitGroupExport(hitGroupExportName);
    hitGroupSubobject->SetClosestHitShaderImport(hitExportName);

    auto localRootSignatureSubObject = stateObjectDesc.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
    localRootSignatureSubObject->SetRootSignature(g_LocalRaytracingRootSignature.Get());

    const UINT shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
#define ALIGN(alignment, num) ((((num) + alignment - 1) / alignment) * alignment)
    const UINT offsetToDescriptorHandle = ALIGN(sizeof(D3D12_GPU_DESCRIPTOR_HANDLE), shaderIdentifierSize);
    const UINT offsetToMaterialConstants = ALIGN(sizeof(UINT32), offsetToDescriptorHandle + sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
    const UINT shaderRecordSizeInBytes = ALIGN(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT, offsetToMaterialConstants + sizeof(MaterialRootConstant));
    
    std::vector<byte> pHitShaderTable(shaderRecordSizeInBytes * numMeshes);
    auto GetShaderTable = [=](const ModelH3D &model, ID3D12StateObject *pPSO, byte *pShaderTable)
    {
        ID3D12StateObjectProperties* stateObjectProperties = nullptr;
        ThrowIfFailed(pPSO->QueryInterface(IID_PPV_ARGS(&stateObjectProperties)));
        void *pHitGroupIdentifierData = stateObjectProperties->GetShaderIdentifier(hitGroupExportName);
        for (UINT i = 0; i < numMeshes; i++)
        {
            byte *pShaderRecord = i * shaderRecordSizeInBytes + pShaderTable;
            memcpy(pShaderRecord, pHitGroupIdentifierData, shaderIdentifierSize);

            UINT materialIndex = model.m_pMesh[i].materialIndex;
            memcpy(pShaderRecord + offsetToDescriptorHandle, &g_GpuSceneMaterialSrvs[materialIndex].ptr, sizeof(g_GpuSceneMaterialSrvs[materialIndex].ptr));

            MaterialRootConstant material;
            material.MaterialID = i;
            memcpy(pShaderRecord + offsetToMaterialConstants, &material, sizeof(material));
        }
    };

    // This causes the CD3DX12_STATE_OBJECT_DESC to generate the final D3D12_STATE_OBJECT_DESC, after which we aren't supposed to make any more changes.
    // But we *are* going to make changes to override the shader library bytecode to make various permutations of the state object. For this reason, we
    // must const_cast the state object description. ReplaceDxilLibrary() will not accept a const D3D12_STATE_OBJECT_DESC.
    D3D12_STATE_OBJECT_DESC* pStateObjectDesc = const_cast<D3D12_STATE_OBJECT_DESC*>((const D3D12_STATE_OBJECT_DESC*)stateObjectDesc);

    {
        ComPtr<ID3D12StateObject> pbarycentricPSO;
        g_pRaytracingDevice->CreateStateObject(pStateObjectDesc, IID_PPV_ARGS(&pbarycentricPSO));

        GetShaderTable(model, pbarycentricPSO.Get(), pHitShaderTable.data());
        g_RaytracingInputs[Primarybarycentric] = RaytracingDispatchRayInputs(*g_pRaytracingDevice.Get(), pbarycentricPSO.Get(), pHitShaderTable.data(), shaderRecordSizeInBytes, (UINT)pHitShaderTable.size(), rayGenExportName, missExportName);
    }

    {
        ReplaceDxilLibrary(pStateObjectDesc, g_pRayGenerationShaderSSRLib, rayGenExportName);

        ComPtr<ID3D12StateObject> pReflectionbarycentricPSO;
        g_pRaytracingDevice->CreateStateObject(pStateObjectDesc, IID_PPV_ARGS(&pReflectionbarycentricPSO));

        GetShaderTable(model, pReflectionbarycentricPSO.Get(), pHitShaderTable.data());
        g_RaytracingInputs[Reflectionbarycentric] = RaytracingDispatchRayInputs(*g_pRaytracingDevice.Get(), pReflectionbarycentricPSO.Get(), pHitShaderTable.data(), shaderRecordSizeInBytes, (UINT)pHitShaderTable.size(), rayGenExportName, missExportName);
    }

    {
        ReplaceDxilLibrary(pStateObjectDesc, g_pRayGenerationShadowsLib, rayGenExportName);
        ReplaceDxilLibrary(pStateObjectDesc, g_pmissShadowsLib, missExportName);

        ComPtr<ID3D12StateObject> pShadowsPSO;
        g_pRaytracingDevice->CreateStateObject(pStateObjectDesc, IID_PPV_ARGS(&pShadowsPSO));

        GetShaderTable(model, pShadowsPSO.Get(), pHitShaderTable.data());
        g_RaytracingInputs[Shadows] = RaytracingDispatchRayInputs(*g_pRaytracingDevice.Get(), pShadowsPSO.Get(), pHitShaderTable.data(), shaderRecordSizeInBytes, (UINT)pHitShaderTable.size(), rayGenExportName, missExportName);
    }

    {
        ReplaceDxilLibrary(pStateObjectDesc, g_pRayGenerationShaderLib, rayGenExportName);
        ReplaceDxilLibrary(pStateObjectDesc, g_pDiffuseHitShaderLib, hitExportName);
        ReplaceDxilLibrary(pStateObjectDesc, g_pmissShaderLib, missExportName);

        ComPtr<ID3D12StateObject> pDiffusePSO;
        g_pRaytracingDevice->CreateStateObject(pStateObjectDesc, IID_PPV_ARGS(&pDiffusePSO));

        GetShaderTable(model, pDiffusePSO.Get(), pHitShaderTable.data());
        g_RaytracingInputs[DiffuseHitShader] = RaytracingDispatchRayInputs(*g_pRaytracingDevice.Get(), pDiffusePSO.Get(), pHitShaderTable.data(), shaderRecordSizeInBytes, (UINT)pHitShaderTable.size(), rayGenExportName, missExportName);
    }

   {
        ReplaceDxilLibrary(pStateObjectDesc, g_pRayGenerationShaderSSRLib, rayGenExportName);
        ReplaceDxilLibrary(pStateObjectDesc, g_pDiffuseHitShaderLib, hitExportName);
        ReplaceDxilLibrary(pStateObjectDesc, g_pmissShaderLib, missExportName);

        ComPtr<ID3D12StateObject> pReflectionPSO;
        g_pRaytracingDevice->CreateStateObject(pStateObjectDesc, IID_PPV_ARGS(&pReflectionPSO));

        GetShaderTable(model, pReflectionPSO.Get(), pHitShaderTable.data());
        g_RaytracingInputs[Reflection] = RaytracingDispatchRayInputs(*g_pRaytracingDevice.Get(), pReflectionPSO.Get(), pHitShaderTable.data(), shaderRecordSizeInBytes, (UINT)pHitShaderTable.size(), rayGenExportName, missExportName);
    }

   for (auto &raytracingPipelineState : g_RaytracingInputs)
   {
        WCHAR hitGroupExportNameClosestHitType[64];
        swprintf_s(hitGroupExportNameClosestHitType, L"%s::closesthit", hitGroupExportName );
        SetPipelineStateStackSize(rayGenExportName, hitGroupExportNameClosestHitType, missExportName, MaxRayRecursion, raytracingPipelineState.m_pPSO.Get());
   }
}

void D3D12RaytracingMiniEngineSample::Startup( void )
{
    MotionBlur::Enable = false;//true;
    TemporalEffects::EnableTAA = false;//true;
    FXAA::Enable = false;
    PostEffects::EnableHDR = false;//true;
    PostEffects::EnableAdaptation = false;//true;
    SSAO::Enable = true;

    Renderer::Initialize();

    Sponza::Startup(m_Camera);

    m_Camera.SetZRange( 1.0f, 10000.0f );
    m_CameraController.reset(new FlyingFPSCamera(m_Camera, Vector3(kYUnitVector)));


    ThrowIfFailed(g_Device->QueryInterface(IID_PPV_ARGS(&g_pRaytracingDevice)), L"Couldn't get DirectX Raytracing interface for the device.\n");

    g_pRaytracingDescriptorHeap = std::unique_ptr<DescriptorHeapStack>(
        new DescriptorHeapStack(*g_Device, 200, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0));

    D3D12_FEATURE_DATA_D3D12_OPTIONS1 options1;
    g_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &options1, sizeof(options1));

    g_hitConstantBuffer.Create(L"Hit Constant Buffer", 1, sizeof(HitShaderConstants));
    g_dynamicConstantBuffer.Create(L"Dynamic Constant Buffer", 1, sizeof(DynamicCB));

    //
    // Load the model
    //
    const ModelH3D& model = Sponza::GetModel();

    InitializeSceneInfo(model);
    InitializeViews(model);

    //
    // Define the top level acceleration structure
    //
    const UINT numMeshes = model.m_Header.meshCount;
    const UINT numInstances = numMeshes;

    // You can toggle between all meshes in one BLAS or one instance per mesh. Typically, to save memory, you would instance a BLAS multiple
    // times when geometry is duplicated.
    ASSERT(numInstances == 1 || numInstances == numMeshes);

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo;
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelAccelerationStructureDesc = {};
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &topLevelInputs = topLevelAccelerationStructureDesc.Inputs;
    topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    topLevelInputs.NumDescs = numInstances;
    topLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    topLevelInputs.pGeometryDescs = nullptr;
    topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    g_pRaytracingDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
    
    const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlag = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs(model.m_Header.meshCount);
    UINT64 scratchBufferSizeNeeded = topLevelPrebuildInfo.ScratchDataSizeInBytes;

    ByteAddressBuffer tlasScratchBuffer;
    tlasScratchBuffer.Create(L"Acceleration Structure Scratch Buffer", (UINT)scratchBufferSizeNeeded, 1);

    D3D12_HEAP_PROPERTIES defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto tlasBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(topLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    g_Device->CreateCommittedResource(
        &defaultHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &tlasBufferDesc,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        nullptr,
        IID_PPV_ARGS(&g_bvh_topLevelAccelerationStructure));

    topLevelAccelerationStructureDesc.DestAccelerationStructureData = g_bvh_topLevelAccelerationStructure->GetGPUVirtualAddress();
    topLevelAccelerationStructureDesc.ScratchAccelerationStructureData = tlasScratchBuffer.GetGpuVirtualAddress();

    //
    // Define the bottom level acceleration structures
    //

    for (UINT i = 0; i < numMeshes; i++)
    {
        auto &mesh = model.m_pMesh[i];

        D3D12_RAYTRACING_GEOMETRY_DESC &desc = geometryDescs[i];
        desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
        desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

        D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC &trianglesDesc = desc.Triangles;
        trianglesDesc.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        trianglesDesc.VertexCount = mesh.vertexCount;
        trianglesDesc.VertexBuffer.StartAddress = model.GetVertexBuffer().BufferLocation + (mesh.vertexDataByteOffset + (UINT)mesh.attrib[ModelH3D::attrib_position].offset);
        trianglesDesc.IndexBuffer = model.GetIndexBuffer().BufferLocation + mesh.indexDataByteOffset;
        trianglesDesc.VertexBuffer.StrideInBytes = mesh.vertexStride;
        trianglesDesc.IndexCount = mesh.indexCount;
        trianglesDesc.IndexFormat = DXGI_FORMAT_R16_UINT;
        trianglesDesc.Transform3x4 = 0;
    }

    g_bvh_bottomLevelAccelerationStructures.resize(numInstances);
    std::vector<D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC> blasDescs(numInstances);
    std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDescs(numInstances);
    std::vector<ByteAddressBuffer> blasScratchBuffers(numInstances);

    for (UINT i = 0; i < numInstances; i++)
    {
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC& blasDesc = blasDescs[i];
        blasDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        blasDesc.Inputs.NumDescs = (numInstances == numMeshes) ? 1 : numMeshes;
        blasDesc.Inputs.pGeometryDescs = &geometryDescs[i];
        blasDesc.Inputs.Flags = buildFlag;
        blasDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo;
        g_pRaytracingDevice->GetRaytracingAccelerationStructurePrebuildInfo(&blasDesc.Inputs, &bottomLevelPrebuildInfo);

        auto &blas = g_bvh_bottomLevelAccelerationStructures[i];

        auto blasBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        g_Device->CreateCommittedResource(
            &defaultHeapProps,
            D3D12_HEAP_FLAG_NONE, 
            &blasBufferDesc,
            D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
            nullptr, 
            IID_PPV_ARGS(&blas));

        blasDesc.DestAccelerationStructureData = blas->GetGPUVirtualAddress();

        blasScratchBuffers[i].Create(L"BLAS build scratch buffer", (UINT)bottomLevelPrebuildInfo.ScratchDataSizeInBytes, 1);
        blasDesc.ScratchAccelerationStructureData = blasScratchBuffers[i].GetGpuVirtualAddress();

        D3D12_RAYTRACING_INSTANCE_DESC &instanceDesc = instanceDescs[i];
        g_pRaytracingDescriptorHeap->AllocateBufferUav(*blas.Get());
        
        // Identity matrix
        ZeroMemory(instanceDesc.Transform, sizeof(instanceDesc.Transform));
        instanceDesc.Transform[0][0] = 1.0f;
        instanceDesc.Transform[1][1] = 1.0f;
        instanceDesc.Transform[2][2] = 1.0f;
        
        instanceDesc.AccelerationStructure = blas->GetGPUVirtualAddress();
        instanceDesc.Flags = 0;
        instanceDesc.InstanceID = 0;
        instanceDesc.InstanceMask = 1;
        instanceDesc.InstanceContributionToHitGroupIndex = i;
    }

    //
    // Upload the instance data
    //
    ByteAddressBuffer instanceDataBuffer;
    instanceDataBuffer.Create(L"Instance Data Buffer", numInstances, sizeof(D3D12_RAYTRACING_INSTANCE_DESC), instanceDescs.data());

    topLevelInputs.InstanceDescs = instanceDataBuffer.GetGpuVirtualAddress();
    topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;

    //
    // Build the acceleration structures
    //
    GraphicsContext& gfxContext = GraphicsContext::Begin(L"Build Acceleration Structures");

    ComPtr<ID3D12GraphicsCommandList4> pRaytracingCommandList;
    gfxContext.GetCommandList()->QueryInterface(IID_PPV_ARGS(&pRaytracingCommandList));

    ID3D12DescriptorHeap *descriptorHeaps[] = { &g_pRaytracingDescriptorHeap->GetDescriptorHeap() };
    pRaytracingCommandList->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);

    auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
    for (UINT i = 0; i < blasDescs.size(); i++)
    {
        pRaytracingCommandList->BuildRaytracingAccelerationStructure(&blasDescs[i], 0, nullptr);

        // If each BLAS build reuses the scratch buffer, you would need a UAV barrier between each. But without
        // barriers, the driver may be able to batch these BLAS builds together. This maximizes GPU utilization
        // and should execute more quickly.
    }
    pRaytracingCommandList->ResourceBarrier(1, &uavBarrier);
    pRaytracingCommandList->BuildRaytracingAccelerationStructure(&topLevelAccelerationStructureDesc, 0, nullptr);
    
    gfxContext.Finish(true);

    //
    // Build the RTPSO
    //
    InitializeRaytracingStateObjects(model, numMeshes);

    m_CameraPosArrayCurrentPosition = 0;
    
    // Lion's head
    m_CameraPosArray[0].position = Vector3(-1100.0f, 170.0f, -30.0f);
    m_CameraPosArray[0].heading = 1.5707f;
    m_CameraPosArray[0].pitch = 0.0f;

    // View of columns
    m_CameraPosArray[1].position = Vector3(299.0f, 208.0f, -202.0f);
    m_CameraPosArray[1].heading = -3.1111f;
    m_CameraPosArray[1].pitch = 0.5953f;

    // Bottom-up view from the floor
    m_CameraPosArray[2].position = Vector3(-1237.61f, 80.60f, -26.02f);
    m_CameraPosArray[2].heading = -1.5707f;
    m_CameraPosArray[2].pitch = 0.268f;

    // Top-down view from the second floor
    m_CameraPosArray[3].position = Vector3(-977.90f, 595.05f, -194.97f);
    m_CameraPosArray[3].heading = -2.077f;
    m_CameraPosArray[3].pitch =  - 0.450f;

    // View of corridors on the second floor
    m_CameraPosArray[4].position = Vector3(-1463.0f, 600.0f, 394.52f);
    m_CameraPosArray[4].heading = -1.236f;
    m_CameraPosArray[4].pitch = 0.0f;
}

void D3D12RaytracingMiniEngineSample::Cleanup( void )
{
    Sponza::Cleanup();
    Renderer::Shutdown();
}

namespace Graphics
{
    extern EnumVar DebugZoom;
}

void D3D12RaytracingMiniEngineSample::Update( float deltaT )
{
    ScopedTimer _prof(L"Update State");

    if (GameInput::IsFirstPressed(GameInput::kLShoulder))
        DebugZoom.Decrement();
    else if (GameInput::IsFirstPressed(GameInput::kRShoulder))
        DebugZoom.Increment();
    else if(GameInput::IsFirstPressed(GameInput::kKey_1))
      rayTracingMode = RTM_OFF;
    else if(GameInput::IsFirstPressed(GameInput::kKey_2))
      rayTracingMode = RTM_TRAVERSAL;
    else if(GameInput::IsFirstPressed(GameInput::kKey_3))
      rayTracingMode = RTM_SSR;
    else if(GameInput::IsFirstPressed(GameInput::kKey_4))
      rayTracingMode = RTM_SHADOWS;
    else if(GameInput::IsFirstPressed(GameInput::kKey_5))
      rayTracingMode = RTM_DIFFUSE_WITH_SHADOWMAPS;
    else if(GameInput::IsFirstPressed(GameInput::kKey_6))
      rayTracingMode = RTM_DIFFUSE_WITH_SHADOWRAYS;
    else if(GameInput::IsFirstPressed(GameInput::kKey_7))
      rayTracingMode = RTM_REFLECTIONS;
    
    static bool freezeCamera = false;
    
    if (GameInput::IsFirstPressed(GameInput::kKey_f))
    {
        freezeCamera = !freezeCamera;
    }

    if (GameInput::IsFirstPressed(GameInput::kKey_left))
    {
        m_CameraPosArrayCurrentPosition = (m_CameraPosArrayCurrentPosition + c_NumCameraPositions - 1) % c_NumCameraPositions;
        SetCameraToPredefinedPosition(m_CameraPosArrayCurrentPosition);
    }
    else if (GameInput::IsFirstPressed(GameInput::kKey_right))
    {
        m_CameraPosArrayCurrentPosition = (m_CameraPosArrayCurrentPosition + 1) % c_NumCameraPositions;
        SetCameraToPredefinedPosition(m_CameraPosArrayCurrentPosition);
    }

    if (!freezeCamera) 
    {
        m_CameraController->Update(deltaT);
    }

    // We use viewport offsets to jitter sample positions from frame to frame (for TAA.)
    // D3D has a design quirk with fractional offsets such that the implicit scissor
    // region of a viewport is floor(TopLeftXY) and floor(TopLeftXY + WidthHeight), so
    // having a negative fractional top left, e.g. (-0.25, -0.25) would also shift the
    // BottomRight corner up by a whole integer.  One solution is to pad your viewport
    // dimensions with an extra pixel.  My solution is to only use positive fractional offsets,
    // but that means that the average sample position is +0.5, which I use when I disable
    // temporal AA.
    TemporalEffects::GetJitterOffset(m_MainViewport.TopLeftX, m_MainViewport.TopLeftY);

    m_MainViewport.Width = (float)g_SceneColorBuffer.GetWidth();
    m_MainViewport.Height = (float)g_SceneColorBuffer.GetHeight();
    m_MainViewport.MinDepth = 0.0f;
    m_MainViewport.MaxDepth = 1.0f;

    m_MainScissor.left = 0;
    m_MainScissor.top = 0;
    m_MainScissor.right = (LONG)g_SceneColorBuffer.GetWidth();
    m_MainScissor.bottom = (LONG)g_SceneColorBuffer.GetHeight();
}

void D3D12RaytracingMiniEngineSample::SetCameraToPredefinedPosition(int cameraPosition) 
{
    if (cameraPosition < 0 || cameraPosition >= c_NumCameraPositions)
        return;

    m_CameraController->SetHeadingPitchAndPosition(
        m_CameraPosArray[m_CameraPosArrayCurrentPosition].heading,
        m_CameraPosArray[m_CameraPosArrayCurrentPosition].pitch,
        m_CameraPosArray[m_CameraPosArrayCurrentPosition].position);
}

void D3D12RaytracingMiniEngineSample::RenderScene(void)
{
    const bool skipDiffusePass = 
        rayTracingMode == RTM_DIFFUSE_WITH_SHADOWMAPS ||
        rayTracingMode == RTM_DIFFUSE_WITH_SHADOWRAYS ||
        rayTracingMode == RTM_TRAVERSAL;
        
    const bool skipShadowMap = 
        rayTracingMode == RTM_DIFFUSE_WITH_SHADOWRAYS ||
        rayTracingMode == RTM_TRAVERSAL ||
        rayTracingMode == RTM_SSR;

    GraphicsContext& gfxContext = GraphicsContext::Begin(L"Scene Render");

    uint32_t FrameIndex = TemporalEffects::GetFrameIndexMod2();
    const D3D12_VIEWPORT& viewport = m_MainViewport;
    const D3D12_RECT& scissor = m_MainScissor;

    ParticleEffectManager::Update(gfxContext.GetComputeContext(), Graphics::GetFrameTime());

    Sponza::RenderScene(gfxContext, m_Camera, viewport, scissor, skipDiffusePass, skipShadowMap);

    // Some systems generate a per-pixel velocity buffer to better track dynamic and skinned meshes.  Everything
    // is static in our scene, so we generate velocity from camera motion and the depth buffer.  A velocity buffer
    // is necessary for all temporal effects (and motion blur).
    MotionBlur::GenerateCameraVelocityBuffer(gfxContext, m_Camera, true);

    TemporalEffects::ResolveImage(gfxContext);

    ParticleEffectManager::Render(gfxContext, m_Camera, g_SceneColorBuffer, g_SceneDepthBuffer, g_LinearDepth[FrameIndex]);

    // Until I work out how to couple these two, it's "either-or".
    if (DepthOfField::Enable)
        DepthOfField::Render(gfxContext, m_Camera.GetNearClip(), m_Camera.GetFarClip());
    else
        MotionBlur::RenderObjectBlur(gfxContext, g_VelocityBuffer);

    Raytrace(gfxContext);
    gfxContext.Finish();
}

//
// Tests traversal
//

void Raytracebarycentrics(
    CommandContext& context,
    const Math::Camera& camera,
    ColorBuffer& colorTarget)
{
    ScopedTimer _p0(L"Raytracing barycentrics", context);

    // Prepare constants
    DynamicCB inputs = g_dynamicCb;
    auto m0 = camera.GetViewProjMatrix();
    auto m1 = Transpose(Invert(m0));
    memcpy(&inputs.cameraToWorld, &m1, sizeof(inputs.cameraToWorld));
    memcpy(&inputs.worldCameraPosition, &camera.GetPosition(), sizeof(inputs.worldCameraPosition));
    inputs.resolution.x = (float)colorTarget.GetWidth();
    inputs.resolution.y = (float)colorTarget.GetHeight();

    HitShaderConstants hitShaderConstants = {};
    hitShaderConstants.IsReflection = false;
    context.WriteBuffer(g_hitConstantBuffer, 0, &hitShaderConstants, sizeof(hitShaderConstants));

    context.WriteBuffer(g_dynamicConstantBuffer, 0, &inputs, sizeof(inputs));

    context.TransitionResource(g_hitConstantBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    context.TransitionResource(g_dynamicConstantBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    context.TransitionResource(colorTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    context.FlushResourceBarriers();

    ID3D12GraphicsCommandList * pCommandList = context.GetCommandList();

    ComPtr<ID3D12GraphicsCommandList4> pRaytracingCommandList;
    pCommandList->QueryInterface(IID_PPV_ARGS(&pRaytracingCommandList));

    ID3D12DescriptorHeap *pDescriptorHeaps[] = { &g_pRaytracingDescriptorHeap->GetDescriptorHeap() };
    pRaytracingCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);

    pCommandList->SetComputeRootSignature(g_GlobalRaytracingRootSignature.Get());
    pCommandList->SetComputeRootDescriptorTable(0, g_SceneSrvs);
    pCommandList->SetComputeRootConstantBufferView(1, g_hitConstantBuffer.GetGpuVirtualAddress());
    pCommandList->SetComputeRootConstantBufferView(2, g_dynamicConstantBuffer.GetGpuVirtualAddress());
    pCommandList->SetComputeRootDescriptorTable(4, g_OutputUAV);
    pRaytracingCommandList->SetComputeRootShaderResourceView(7, g_bvh_topLevelAccelerationStructure->GetGPUVirtualAddress());

    D3D12_DISPATCH_RAYS_DESC dispatchRaysDesc = g_RaytracingInputs[Primarybarycentric].GetDispatchRayDesc(colorTarget.GetWidth(), colorTarget.GetHeight());
    pRaytracingCommandList->SetPipelineState1(g_RaytracingInputs[Primarybarycentric].m_pPSO.Get());
    pRaytracingCommandList->DispatchRays(&dispatchRaysDesc);
}

void RaytracebarycentricsSSR(
    CommandContext& context,
    const Math::Camera& camera,
    ColorBuffer& colorTarget,
    DepthBuffer& depth,
    ColorBuffer& normals)
{
    ScopedTimer _p0(L"Raytracing SSR barycentrics", context);

    DynamicCB inputs = g_dynamicCb;
    auto m0 = camera.GetViewProjMatrix();
    auto m1 = Transpose(Invert(m0));
    memcpy(&inputs.cameraToWorld, &m1, sizeof(inputs.cameraToWorld));
    memcpy(&inputs.worldCameraPosition, &camera.GetPosition(), sizeof(inputs.worldCameraPosition));
    inputs.resolution.x = (float)colorTarget.GetWidth();
    inputs.resolution.y = (float)colorTarget.GetHeight();

    HitShaderConstants hitShaderConstants = {};
    hitShaderConstants.IsReflection = false;
    context.WriteBuffer(g_hitConstantBuffer, 0, &hitShaderConstants, sizeof(hitShaderConstants));

    ComputeContext& ctx = context.GetComputeContext();
    ID3D12GraphicsCommandList *pCommandList = context.GetCommandList();

    ctx.WriteBuffer(g_dynamicConstantBuffer, 0, &inputs, sizeof(inputs));
    ctx.TransitionResource(g_dynamicConstantBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    ctx.TransitionResource(g_hitConstantBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    ctx.TransitionResource(normals, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    ctx.TransitionResource(depth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    ctx.TransitionResource(g_ShadowBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    ctx.TransitionResource(colorTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    ctx.FlushResourceBarriers();

    ComPtr<ID3D12GraphicsCommandList4> pRaytracingCommandList;
    pCommandList->QueryInterface(IID_PPV_ARGS(&pRaytracingCommandList));

    ID3D12DescriptorHeap *pDescriptorHeaps[] = { &g_pRaytracingDescriptorHeap->GetDescriptorHeap() };
    pRaytracingCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);

    pCommandList->SetComputeRootSignature(g_GlobalRaytracingRootSignature.Get());
    pCommandList->SetComputeRootConstantBufferView(1, g_hitConstantBuffer.GetGpuVirtualAddress());
    pCommandList->SetComputeRootConstantBufferView(2, g_dynamicConstantBuffer.GetGpuVirtualAddress());
    pCommandList->SetComputeRootDescriptorTable(4, g_OutputUAV);
    pCommandList->SetComputeRootDescriptorTable(3, g_DepthAndNormalsTable);
    pRaytracingCommandList->SetComputeRootShaderResourceView(7, g_bvh_topLevelAccelerationStructure->GetGPUVirtualAddress());

    D3D12_DISPATCH_RAYS_DESC dispatchRaysDesc = g_RaytracingInputs[Reflectionbarycentric].GetDispatchRayDesc(colorTarget.GetWidth(), colorTarget.GetHeight());
    pRaytracingCommandList->SetPipelineState1(g_RaytracingInputs[Reflectionbarycentric].m_pPSO.Get());
    pRaytracingCommandList->DispatchRays(&dispatchRaysDesc);
}

void D3D12RaytracingMiniEngineSample::RaytraceShadows(
    GraphicsContext& context,
    const Math::Camera& camera,
    ColorBuffer& colorTarget,
    DepthBuffer& depth)
{
    ScopedTimer _p0(L"Raytracing Shadows", context);

    DynamicCB inputs = g_dynamicCb;
    auto m0 = camera.GetViewProjMatrix();
    auto m1 = Transpose(Invert(m0));
    memcpy(&inputs.cameraToWorld, &m1, sizeof(inputs.cameraToWorld));
    memcpy(&inputs.worldCameraPosition, &camera.GetPosition(), sizeof(inputs.worldCameraPosition));
    inputs.resolution.x = (float)colorTarget.GetWidth();
    inputs.resolution.y = (float)colorTarget.GetHeight();

    HitShaderConstants hitShaderConstants = {};
    hitShaderConstants.sunDirection = Sponza::m_SunDirection;
    hitShaderConstants.sunLight = Vector3(1.0f, 1.0f, 1.0f) * Sponza::m_SunLightIntensity;
    hitShaderConstants.ambientLight = Vector3(1.0f, 1.0f, 1.0f) * Sponza::m_AmbientIntensity;
    hitShaderConstants.ShadowTexelSize[0] = 1.0f / g_ShadowBuffer.GetWidth();
    hitShaderConstants.modelToShadow = Sponza::m_SunShadow.GetShadowMatrix();
    hitShaderConstants.IsReflection = false;
    hitShaderConstants.UseShadowRays = false;
    context.WriteBuffer(g_hitConstantBuffer, 0, &hitShaderConstants, sizeof(hitShaderConstants));

    ComputeContext& ctx = context.GetComputeContext();
    ID3D12GraphicsCommandList *pCommandList = context.GetCommandList();

    ctx.WriteBuffer(g_dynamicConstantBuffer, 0, &inputs, sizeof(inputs));
    ctx.TransitionResource(g_dynamicConstantBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    ctx.TransitionResource(g_SceneNormalBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    ctx.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    ctx.TransitionResource(g_hitConstantBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    ctx.TransitionResource(depth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    ctx.TransitionResource(g_ShadowBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    ctx.TransitionResource(colorTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    ctx.FlushResourceBarriers();

    ComPtr<ID3D12GraphicsCommandList4> pRaytracingCommandList;
    pCommandList->QueryInterface(IID_PPV_ARGS(&pRaytracingCommandList));

    ID3D12DescriptorHeap *pDescriptorHeaps[] = { &g_pRaytracingDescriptorHeap->GetDescriptorHeap() };
    pRaytracingCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);

    pCommandList->SetComputeRootSignature(g_GlobalRaytracingRootSignature.Get());
    pCommandList->SetComputeRootConstantBufferView(1, g_hitConstantBuffer.GetGpuVirtualAddress());
    pCommandList->SetComputeRootConstantBufferView(2, g_dynamicConstantBuffer->GetGPUVirtualAddress());
    pCommandList->SetComputeRootDescriptorTable(4, g_OutputUAV);
    pCommandList->SetComputeRootDescriptorTable(3, g_DepthAndNormalsTable);
    pRaytracingCommandList->SetComputeRootShaderResourceView(7, g_bvh_topLevelAccelerationStructure->GetGPUVirtualAddress());

    D3D12_DISPATCH_RAYS_DESC dispatchRaysDesc = g_RaytracingInputs[Shadows].GetDispatchRayDesc(colorTarget.GetWidth(), colorTarget.GetHeight());
    pRaytracingCommandList->SetPipelineState1(g_RaytracingInputs[Shadows].m_pPSO.Get());
    pRaytracingCommandList->DispatchRays(&dispatchRaysDesc);
}

void D3D12RaytracingMiniEngineSample::RaytraceDiffuse(
    GraphicsContext& context,
    const Math::Camera& camera,
    ColorBuffer& colorTarget)
{
    ScopedTimer _p0(L"RaytracingWithHitShader", context);

    // Prepare constants
    DynamicCB inputs = g_dynamicCb;
    auto m0 = camera.GetViewProjMatrix();
    auto m1 = Transpose(Invert(m0));
    memcpy(&inputs.cameraToWorld, &m1, sizeof(inputs.cameraToWorld));
    memcpy(&inputs.worldCameraPosition, &camera.GetPosition(), sizeof(inputs.worldCameraPosition));
    inputs.resolution.x = (float)colorTarget.GetWidth();
    inputs.resolution.y = (float)colorTarget.GetHeight();

    HitShaderConstants hitShaderConstants = {};
    hitShaderConstants.sunDirection = Sponza::m_SunDirection;
    hitShaderConstants.sunLight = Vector3(1.0f, 1.0f, 1.0f) * Sponza::m_SunLightIntensity;
    hitShaderConstants.ambientLight = Vector3(1.0f, 1.0f, 1.0f) * Sponza::m_AmbientIntensity;
    hitShaderConstants.ShadowTexelSize[0] = 1.0f / g_ShadowBuffer.GetWidth();
    hitShaderConstants.modelToShadow = Transpose(Sponza::m_SunShadow.GetShadowMatrix());
    hitShaderConstants.IsReflection = false;
    hitShaderConstants.UseShadowRays = rayTracingMode == RTM_DIFFUSE_WITH_SHADOWRAYS;
    context.WriteBuffer(g_hitConstantBuffer, 0, &hitShaderConstants, sizeof(hitShaderConstants));
    context.WriteBuffer(g_dynamicConstantBuffer, 0, &inputs, sizeof(inputs));

    context.TransitionResource(g_dynamicConstantBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    context.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    context.TransitionResource(g_hitConstantBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    context.TransitionResource(g_ShadowBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    context.TransitionResource(colorTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    context.FlushResourceBarriers();

    ID3D12GraphicsCommandList * pCommandList = context.GetCommandList();

    ComPtr<ID3D12GraphicsCommandList4> pRaytracingCommandList;
    pCommandList->QueryInterface(IID_PPV_ARGS(&pRaytracingCommandList));

    ID3D12DescriptorHeap *pDescriptorHeaps[] = { &g_pRaytracingDescriptorHeap->GetDescriptorHeap() };
    pRaytracingCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);

    pCommandList->SetComputeRootSignature(g_GlobalRaytracingRootSignature.Get());
    pCommandList->SetComputeRootDescriptorTable(0, g_SceneSrvs);
    pCommandList->SetComputeRootConstantBufferView(1, g_hitConstantBuffer.GetGpuVirtualAddress());
    pCommandList->SetComputeRootConstantBufferView(2, g_dynamicConstantBuffer.GetGpuVirtualAddress());
    pCommandList->SetComputeRootDescriptorTable(4, g_OutputUAV);
    pRaytracingCommandList->SetComputeRootShaderResourceView(7, g_bvh_topLevelAccelerationStructure->GetGPUVirtualAddress());

    D3D12_DISPATCH_RAYS_DESC dispatchRaysDesc = g_RaytracingInputs[DiffuseHitShader].GetDispatchRayDesc(colorTarget.GetWidth(), colorTarget.GetHeight());
    pRaytracingCommandList->SetPipelineState1(g_RaytracingInputs[DiffuseHitShader].m_pPSO.Get());
    pRaytracingCommandList->DispatchRays(&dispatchRaysDesc);
}

void D3D12RaytracingMiniEngineSample::RaytraceReflections(
    GraphicsContext& context,
    const Math::Camera& camera,
    ColorBuffer& colorTarget,
    DepthBuffer& depth, 
    ColorBuffer& normals)
{
    ScopedTimer _p0(L"RaytracingWithHitShader", context);

    // Prepare constants
    DynamicCB inputs = g_dynamicCb;
    auto m0 = camera.GetViewProjMatrix();
    auto m1 = Transpose(Invert(m0));
    memcpy(&inputs.cameraToWorld, &m1, sizeof(inputs.cameraToWorld));
    memcpy(&inputs.worldCameraPosition, &camera.GetPosition(), sizeof(inputs.worldCameraPosition));
    inputs.resolution.x = (float)colorTarget.GetWidth();
    inputs.resolution.y = (float)colorTarget.GetHeight();

    HitShaderConstants hitShaderConstants = {};
    hitShaderConstants.sunDirection = Sponza::m_SunDirection;
    hitShaderConstants.sunLight = Vector3(1.0f, 1.0f, 1.0f) * Sponza::m_SunLightIntensity;
    hitShaderConstants.ambientLight = Vector3(1.0f, 1.0f, 1.0f) * Sponza::m_AmbientIntensity;
    hitShaderConstants.ShadowTexelSize[0] = 1.0f / g_ShadowBuffer.GetWidth();
    hitShaderConstants.modelToShadow = Transpose(Sponza::m_SunShadow.GetShadowMatrix());
    hitShaderConstants.IsReflection = true;
    hitShaderConstants.UseShadowRays = false;
    context.WriteBuffer(g_hitConstantBuffer, 0, &hitShaderConstants, sizeof(hitShaderConstants));
    context.WriteBuffer(g_dynamicConstantBuffer, 0, &inputs, sizeof(inputs));

    context.TransitionResource(g_dynamicConstantBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    context.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    context.TransitionResource(depth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    context.TransitionResource(g_ShadowBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    context.TransitionResource(normals, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    context.TransitionResource(g_hitConstantBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    context.TransitionResource(colorTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    context.FlushResourceBarriers();

    ID3D12GraphicsCommandList * pCommandList = context.GetCommandList();

    ComPtr<ID3D12GraphicsCommandList4> pRaytracingCommandList;
    pCommandList->QueryInterface(IID_PPV_ARGS(&pRaytracingCommandList));

    ID3D12DescriptorHeap *pDescriptorHeaps[] = { &g_pRaytracingDescriptorHeap->GetDescriptorHeap() };
    pRaytracingCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);

    pCommandList->SetComputeRootSignature(g_GlobalRaytracingRootSignature.Get());
    pCommandList->SetComputeRootDescriptorTable(0, g_SceneSrvs);
    pCommandList->SetComputeRootConstantBufferView(1, g_hitConstantBuffer.GetGpuVirtualAddress());
    pCommandList->SetComputeRootConstantBufferView(2, g_dynamicConstantBuffer.GetGpuVirtualAddress());
    pCommandList->SetComputeRootDescriptorTable(3, g_DepthAndNormalsTable);
    pCommandList->SetComputeRootDescriptorTable(4, g_OutputUAV);
    pRaytracingCommandList->SetComputeRootShaderResourceView(7, g_bvh_topLevelAccelerationStructure->GetGPUVirtualAddress());

    D3D12_DISPATCH_RAYS_DESC dispatchRaysDesc = g_RaytracingInputs[Reflection].GetDispatchRayDesc(colorTarget.GetWidth(), colorTarget.GetHeight());
    pRaytracingCommandList->SetPipelineState1(g_RaytracingInputs[Reflection].m_pPSO.Get());
    pRaytracingCommandList->DispatchRays(&dispatchRaysDesc);
}

void D3D12RaytracingMiniEngineSample::RenderUI(class GraphicsContext& gfxContext)
{
    const UINT framesToAverage = 20;
    static float frameRates[framesToAverage] = {};
    frameRates[Graphics::GetFrameCount() % framesToAverage] = Graphics::GetFrameRate();
    float rollingAverageFrameRate = 0.0;
    for (auto frameRate : frameRates)
    {
        rollingAverageFrameRate += frameRate / framesToAverage;
    }

    float primaryRaysPerSec = g_SceneColorBuffer.GetWidth() * g_SceneColorBuffer.GetHeight() * rollingAverageFrameRate / (1000000.0f);
    TextContext text(gfxContext);
    text.Begin();
    text.DrawFormattedString("\nMillion Primary Rays/s: %7.3f", primaryRaysPerSec);
    text.End();
}

void D3D12RaytracingMiniEngineSample::Raytrace(class GraphicsContext& gfxContext)
{
    ScopedTimer _prof(L"Raytrace", gfxContext);

    gfxContext.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    switch (rayTracingMode)
    {
    case RTM_TRAVERSAL:
        Raytracebarycentrics(gfxContext, m_Camera, g_SceneColorBuffer);
        break;

    case RTM_SSR:
        RaytracebarycentricsSSR(gfxContext, m_Camera, g_SceneColorBuffer, g_SceneDepthBuffer, g_SceneNormalBuffer);
        break;

    case RTM_SHADOWS:
        RaytraceShadows(gfxContext, m_Camera, g_SceneColorBuffer, g_SceneDepthBuffer);
        break;

    case RTM_DIFFUSE_WITH_SHADOWMAPS:
    case RTM_DIFFUSE_WITH_SHADOWRAYS:
        RaytraceDiffuse(gfxContext, m_Camera, g_SceneColorBuffer);
        break;

    case RTM_REFLECTIONS:
        RaytraceReflections(gfxContext, m_Camera, g_SceneColorBuffer, g_SceneDepthBuffer, g_SceneNormalBuffer);
        break;
    }

    // Clear the gfxContext's descriptor heap since ray tracing changes this underneath the sheets
    gfxContext.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, nullptr);
}
