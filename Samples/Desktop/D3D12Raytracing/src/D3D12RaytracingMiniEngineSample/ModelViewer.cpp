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
#include "GameCore.h"
#include "GraphicsCore.h"
#include "CameraController.h"
#include "BufferManager.h"
#include "Camera.h"
#include "Model.h"
#include "GpuBuffer.h"
#include "CommandContext.h"
#include "SamplerManager.h"
#include "TemporalEffects.h"
#include "MotionBlur.h"
#include "DepthOfField.h"
#include "PostEffects.h"
#include "SSAO.h"
#include "FXAA.h"
#include "SystemTime.h"
#include "TextRenderer.h"
#include "ShadowCamera.h"
#include "ParticleEffectManager.h"
#include "GameInput.h"
#include "./ForwardPlusLighting.h"
#include <atlbase.h>
#include <atlbase.h>
#include "DXSampleHelper.h"

#include "CompiledShaders/DepthViewerVS.h"
#include "CompiledShaders/DepthViewerPS.h"
#include "CompiledShaders/ModelViewerVS.h"
#include "CompiledShaders/ModelViewerPS.h"
#include "CompiledShaders/WaveTileCountPS.h"
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

extern ByteAddressBuffer   g_bvh_bottomLevelAccelerationStructure;
ColorBuffer g_SceneNormalBuffer;

CComPtr<ID3D12Device5> g_pRaytracingDevice;

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
D3D12_CPU_DESCRIPTOR_HANDLE g_SceneIndices;

D3D12_GPU_DESCRIPTOR_HANDLE g_OutputUAV;
D3D12_GPU_DESCRIPTOR_HANDLE g_DepthAndNormalsTable;
D3D12_GPU_DESCRIPTOR_HANDLE g_SceneSrvs;

std::vector<CComPtr<ID3D12Resource>>   g_bvh_bottomLevelAccelerationStructures;
CComPtr<ID3D12Resource>   g_bvh_topLevelAccelerationStructure;

DynamicCB           g_dynamicCb;
CComPtr<ID3D12RootSignature> g_GlobalRaytracingRootSignature;
CComPtr<ID3D12RootSignature> g_LocalRaytracingRootSignature;

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
        ID3D12Device5 &device,
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
        dispatchRaysDesc.MissShaderTable.StrideInBytes = dispatchRaysDesc.MissShaderTable.SizeInBytes; // Only one entry
        dispatchRaysDesc.Width = DispatchWidth;
        dispatchRaysDesc.Height = DispatchHeight;
        dispatchRaysDesc.Depth = 1;
        return dispatchRaysDesc;
    }

    UINT m_HitGroupStride;
    CComPtr<ID3D12StateObject> m_pPSO;
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

    void SetCameraToPredefinedPosition(int cameraPosition);

private:

    void RenderLightShadows(GraphicsContext& gfxContext);

    enum eObjectFilter { kOpaque = 0x1, kCutout = 0x2, kTransparent = 0x4, kAll = 0xF, kNone = 0x0 };
    void RenderObjects( GraphicsContext& Context, const Matrix4& ViewProjMat, eObjectFilter Filter = kAll );
    void RaytraceDiffuse(GraphicsContext& context, const Math::Camera& camera, ColorBuffer& colorTarget);
    void RaytraceShadows(GraphicsContext& context, const Math::Camera& camera, ColorBuffer& colorTarget, DepthBuffer& depth);
    void RaytraceReflections(GraphicsContext& context, const Math::Camera& camera, ColorBuffer& colorTarget, DepthBuffer& depth, ColorBuffer& normals);

    Camera m_Camera;
    std::auto_ptr<CameraController> m_CameraController;
    Matrix4 m_ViewProjMatrix;
    D3D12_VIEWPORT m_MainViewport;
    D3D12_RECT m_MainScissor;

    RootSignature m_RootSig;
    GraphicsPSO m_DepthPSO[1];
    GraphicsPSO m_CutoutDepthPSO[1];
    GraphicsPSO m_ModelPSO[1];
    GraphicsPSO m_CutoutModelPSO[1];
    GraphicsPSO m_ShadowPSO;
    GraphicsPSO m_CutoutShadowPSO;
    GraphicsPSO m_WaveTileCountPSO;

    D3D12_CPU_DESCRIPTOR_HANDLE m_DefaultSampler;
    D3D12_CPU_DESCRIPTOR_HANDLE m_ShadowSampler;
    D3D12_CPU_DESCRIPTOR_HANDLE m_BiasedDefaultSampler;

    D3D12_CPU_DESCRIPTOR_HANDLE m_ExtraTextures[6];
    Model m_Model;
    std::vector<bool> m_pMaterialIsCutout;
    std::vector<bool> m_pMaterialIsReflective;

    Vector3 m_SunDirection;
    ShadowCamera m_SunShadow;

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

int wmain(int argc, wchar_t** argv)
{
#if _DEBUG
    CComPtr<ID3D12Debug> debugInterface;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
    {
        debugInterface->EnableDebugLayer();
    }
#endif

    CComPtr<ID3D12Device> pDevice;
    CComPtr<IDXGIAdapter1> pAdapter;
    CComPtr<IDXGIFactory2> pFactory;
    CreateDXGIFactory2(0, IID_PPV_ARGS(&pFactory));
    bool validDeviceFound = false;
    for (uint32_t Idx = 0; !validDeviceFound && DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(Idx, &pAdapter); ++Idx)
    {
        DXGI_ADAPTER_DESC1 desc;
        pAdapter->GetDesc1(&desc);

        if (IsDirectXRaytracingSupported(pAdapter))
        {
            validDeviceFound = true;
        }
        pAdapter = nullptr;
    }

    s_EnableVSync.Decrement();
    TargetResolution = k720p;
    g_DisplayWidth = 1280;
    g_DisplayHeight = 720;
    GameCore::RunApplication(D3D12RaytracingMiniEngineSample(), L"D3D12RaytracingMiniEngineSample"); 
    return 0;
}

ExpVar m_SunLightIntensity("Application/Lighting/Sun Light Intensity", 4.0f, 0.0f, 16.0f, 0.1f);
ExpVar m_AmbientIntensity("Application/Lighting/Ambient Intensity", 0.1f, -16.0f, 16.0f, 0.1f);
NumVar m_SunOrientation("Application/Lighting/Sun Orientation", -0.5f, -100.0f, 100.0f, 0.1f );
NumVar m_SunInclination("Application/Lighting/Sun Inclination", 0.75f, 0.0f, 1.0f, 0.01f );
NumVar ShadowDimX("Application/Lighting/Shadow Dim X", 5000, 1000, 10000, 100 );
NumVar ShadowDimY("Application/Lighting/Shadow Dim Y", 3000, 1000, 10000, 100 );
NumVar ShadowDimZ("Application/Lighting/Shadow Dim Z", 3000, 1000, 10000, 100 );

BoolVar ShowWaveTileCounts("Application/Forward+/Show Wave Tile Counts", false);

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

    ID3D12DescriptorHeap &GetDescriptorHeap() { return *m_pDescriptorHeap; }

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
    CComPtr<ID3D12DescriptorHeap> m_pDescriptorHeap;
    UINT m_descriptorsAllocated = 0;
    UINT m_descriptorSize;
    D3D12_CPU_DESCRIPTOR_HANDLE m_descriptorHeapCpuBase;
};

std::unique_ptr<DescriptorHeapStack> g_pRaytracingDescriptorHeap;

StructuredBuffer    g_hitShaderMeshInfoBuffer;

static
void InitializeSceneInfo(
    const Model& model)
{
    //
    // Mesh info
    //
    std::vector<RayTraceMeshInfo>   meshInfoData(model.m_Header.meshCount);
    for (UINT i=0; i < model.m_Header.meshCount; ++i)
    {
        meshInfoData[i].m_indexOffsetBytes = model.m_pMesh[i].indexDataByteOffset;
        meshInfoData[i].m_uvAttributeOffsetBytes = model.m_pMesh[i].vertexDataByteOffset + model.m_pMesh[i].attrib[Model::attrib_texcoord0].offset;
        meshInfoData[i].m_normalAttributeOffsetBytes = model.m_pMesh[i].vertexDataByteOffset + model.m_pMesh[i].attrib[Model::attrib_normal].offset;
        meshInfoData[i].m_positionAttributeOffsetBytes = model.m_pMesh[i].vertexDataByteOffset + model.m_pMesh[i].attrib[Model::attrib_position].offset;
        meshInfoData[i].m_tangentAttributeOffsetBytes = model.m_pMesh[i].vertexDataByteOffset + model.m_pMesh[i].attrib[Model::attrib_tangent].offset;
        meshInfoData[i].m_bitangentAttributeOffsetBytes = model.m_pMesh[i].vertexDataByteOffset + model.m_pMesh[i].attrib[Model::attrib_bitangent].offset;
        meshInfoData[i].m_attributeStrideBytes = model.m_pMesh[i].vertexStride;
        meshInfoData[i].m_materialInstanceId = model.m_pMesh[i].materialIndex;
        ASSERT(meshInfoData[i].m_materialInstanceId < 27);
    }

    g_hitShaderMeshInfoBuffer.Create(L"RayTraceMeshInfo",
        (UINT)meshInfoData.size(),
        sizeof(meshInfoData[0]),
        meshInfoData.data());

    g_SceneIndices = model.m_IndexBuffer.GetSRV();
    g_SceneMeshInfo = g_hitShaderMeshInfoBuffer.GetSRV();
}

static
void InitializeViews(const Model& model)
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
        Graphics::g_Device->CopyDescriptorsSimple(1, srvHandle, g_SceneIndices, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        g_pRaytracingDescriptorHeap->AllocateBufferSrv(*const_cast<ID3D12Resource*>(model.m_VertexBuffer.GetResource()));

        g_pRaytracingDescriptorHeap->AllocateDescriptor(srvHandle, unused);
        Graphics::g_Device->CopyDescriptorsSimple(1, srvHandle, g_ShadowBuffer.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        g_pRaytracingDescriptorHeap->AllocateDescriptor(srvHandle, unused);
        Graphics::g_Device->CopyDescriptorsSimple(1, srvHandle, g_SSAOFullScreen.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        for (UINT i = 0; i < model.m_Header.materialCount; i++)
        {
            UINT slot;
            g_pRaytracingDescriptorHeap->AllocateDescriptor(srvHandle, slot);
            Graphics::g_Device->CopyDescriptorsSimple(1, srvHandle, *model.GetSRVs(i), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            g_pRaytracingDescriptorHeap->AllocateDescriptor(srvHandle, unused);
            Graphics::g_Device->CopyDescriptorsSimple(1, srvHandle, model.GetSRVs(i)[3], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            
            g_GpuSceneMaterialSrvs[i] = g_pRaytracingDescriptorHeap->GetGpuHandle(slot);
        }
    }
}

D3D12_STATE_SUBOBJECT CreateDxilLibrary(LPCWSTR entrypoint, const void *pShaderByteCode, SIZE_T bytecodeLength, D3D12_DXIL_LIBRARY_DESC &dxilLibDesc, D3D12_EXPORT_DESC &exportDesc)
{
    exportDesc = { entrypoint, nullptr, D3D12_EXPORT_FLAG_NONE };
    D3D12_STATE_SUBOBJECT dxilLibSubObject = {};
    dxilLibDesc.DXILLibrary.pShaderBytecode = pShaderByteCode;
    dxilLibDesc.DXILLibrary.BytecodeLength = bytecodeLength;
    dxilLibDesc.NumExports = 1;
    dxilLibDesc.pExports = &exportDesc;
    dxilLibSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    dxilLibSubObject.pDesc = &dxilLibDesc;
    return dxilLibSubObject;
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

void InitializeRaytracingStateObjects(const Model &model, UINT numMeshes)
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
    auto globalRootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(ARRAYSIZE(globalRootSignatureParameters), globalRootSignatureParameters, ARRAYSIZE(staticSamplerDescs), staticSamplerDescs);

    CComPtr<ID3DBlob> pGlobalRootSignatureBlob;
    CComPtr<ID3DBlob> pErrorBlob;
    if (FAILED(D3D12SerializeVersionedRootSignature(&globalRootSignatureDesc, &pGlobalRootSignatureBlob, &pErrorBlob)))
    {
        OutputDebugStringA((LPCSTR)pErrorBlob->GetBufferPointer());
    }
    g_pRaytracingDevice->CreateRootSignature(0, pGlobalRootSignatureBlob->GetBufferPointer(), pGlobalRootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&g_GlobalRaytracingRootSignature));

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

    CComPtr<ID3DBlob> pLocalRootSignatureBlob;
    D3D12SerializeVersionedRootSignature(&localRootSignatureDesc, &pLocalRootSignatureBlob, nullptr);
    g_pRaytracingDevice->CreateRootSignature(0, pLocalRootSignatureBlob->GetBufferPointer(), pLocalRootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&g_LocalRaytracingRootSignature));

    std::vector<D3D12_STATE_SUBOBJECT> subObjects;
    D3D12_STATE_SUBOBJECT nodeMaskSubObject;
    UINT nodeMask = 1;
    nodeMaskSubObject.pDesc = &nodeMask;
    nodeMaskSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK;
    subObjects.push_back(nodeMaskSubObject);

    D3D12_STATE_SUBOBJECT rootSignatureSubObject;
    rootSignatureSubObject.pDesc = &g_GlobalRaytracingRootSignature.p;
    rootSignatureSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    subObjects.push_back(rootSignatureSubObject);

    D3D12_STATE_SUBOBJECT configurationSubObject;
    D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig;
    pipelineConfig.MaxTraceRecursionDepth = MaxRayRecursion;
    configurationSubObject.pDesc = &pipelineConfig;
    configurationSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
    subObjects.push_back(configurationSubObject);

    std::vector<LPCWSTR> shadersToAssociate;

    // Ray Gen shader stuff
    // ----------------------------------------------------------------//
    LPCWSTR rayGenShaderExportName = L"RayGen";
    subObjects.push_back(D3D12_STATE_SUBOBJECT{});
    D3D12_STATE_SUBOBJECT &rayGenDxilLibSubobject = subObjects.back();
    D3D12_EXPORT_DESC rayGenExportDesc;
    D3D12_DXIL_LIBRARY_DESC rayGenDxilLibDesc = {};
    rayGenDxilLibSubobject = CreateDxilLibrary(rayGenShaderExportName, g_pRayGenerationShaderLib, sizeof(g_pRayGenerationShaderLib), rayGenDxilLibDesc, rayGenExportDesc);
    shadersToAssociate.push_back(rayGenShaderExportName);

    // Hit Group shader stuff
    // ----------------------------------------------------------------//
    LPCWSTR closestHitExportName = L"Hit";
    D3D12_EXPORT_DESC hitGroupExportDesc;
    D3D12_DXIL_LIBRARY_DESC hitGroupDxilLibDesc = {};
    D3D12_STATE_SUBOBJECT hitGroupLibSubobject = CreateDxilLibrary(closestHitExportName, g_phitShaderLib, sizeof(g_phitShaderLib), hitGroupDxilLibDesc, hitGroupExportDesc);

    subObjects.push_back(hitGroupLibSubobject);
    shadersToAssociate.push_back(closestHitExportName);

    LPCWSTR missExportName = L"Miss";
    D3D12_EXPORT_DESC missExportDesc;
    D3D12_DXIL_LIBRARY_DESC missDxilLibDesc = {};
    D3D12_STATE_SUBOBJECT missLibSubobject = CreateDxilLibrary(missExportName, g_pmissShaderLib, sizeof(g_pmissShaderLib), missDxilLibDesc, missExportDesc);

    subObjects.push_back(missLibSubobject);
    D3D12_STATE_SUBOBJECT &missDxilLibSubobject = subObjects.back();

    shadersToAssociate.push_back(missExportName);

    D3D12_STATE_SUBOBJECT shaderConfigStateObject;
    D3D12_RAYTRACING_SHADER_CONFIG shaderConfig;
    shaderConfig.MaxAttributeSizeInBytes = 8;
    shaderConfig.MaxPayloadSizeInBytes = 8;
    shaderConfigStateObject.pDesc = &shaderConfig;
    shaderConfigStateObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
    subObjects.push_back(shaderConfigStateObject);
    D3D12_STATE_SUBOBJECT &shaderConfigSubobject = subObjects.back();

    LPCWSTR hitGroupExportName = L"HitGroup";
    D3D12_HIT_GROUP_DESC hitGroupDesc = {};
    hitGroupDesc.ClosestHitShaderImport = closestHitExportName;
    hitGroupDesc.HitGroupExport = hitGroupExportName;
    D3D12_STATE_SUBOBJECT hitGroupSubobject = {};
    hitGroupSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
    hitGroupSubobject.pDesc = &hitGroupDesc;
    subObjects.push_back(hitGroupSubobject);

    D3D12_STATE_SUBOBJECT localRootSignatureSubObject;
    localRootSignatureSubObject.pDesc = &g_LocalRaytracingRootSignature.p;
    localRootSignatureSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
    subObjects.push_back(localRootSignatureSubObject);
    D3D12_STATE_SUBOBJECT &rootSignatureSubobject = subObjects.back();

    D3D12_STATE_OBJECT_DESC stateObject;
    stateObject.NumSubobjects = (UINT)subObjects.size();
    stateObject.pSubobjects = subObjects.data();
    stateObject.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

    const UINT shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
#define ALIGN(alignment, num) ((((num) + alignment - 1) / alignment) * alignment)
    const UINT offsetToDescriptorHandle = ALIGN(sizeof(D3D12_GPU_DESCRIPTOR_HANDLE), shaderIdentifierSize);
    const UINT offsetToMaterialConstants = ALIGN(sizeof(UINT32), offsetToDescriptorHandle + sizeof(D3D12_GPU_DESCRIPTOR_HANDLE));
    const UINT shaderRecordSizeInBytes = ALIGN(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT, offsetToMaterialConstants + sizeof(MaterialRootConstant));
    
    std::vector<byte> pHitShaderTable(shaderRecordSizeInBytes * numMeshes);
    auto GetShaderTable = [=](const Model &model, ID3D12StateObject *pPSO, byte *pShaderTable)
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

    {
        CComPtr<ID3D12StateObject> pbarycentricPSO;
        g_pRaytracingDevice->CreateStateObject(&stateObject, IID_PPV_ARGS(&pbarycentricPSO));
        GetShaderTable(model, pbarycentricPSO, pHitShaderTable.data());
        g_RaytracingInputs[Primarybarycentric] = RaytracingDispatchRayInputs(*g_pRaytracingDevice, pbarycentricPSO, pHitShaderTable.data(), shaderRecordSizeInBytes, (UINT)pHitShaderTable.size(), rayGenShaderExportName, missExportName);
    }

    {
        rayGenDxilLibSubobject = CreateDxilLibrary(rayGenShaderExportName, g_pRayGenerationShaderSSRLib, sizeof(g_pRayGenerationShaderSSRLib), rayGenDxilLibDesc, rayGenExportDesc);
        CComPtr<ID3D12StateObject> pReflectionbarycentricPSO;
        g_pRaytracingDevice->CreateStateObject(&stateObject, IID_PPV_ARGS(&pReflectionbarycentricPSO));
        GetShaderTable(model, pReflectionbarycentricPSO, pHitShaderTable.data());
        g_RaytracingInputs[Reflectionbarycentric] = RaytracingDispatchRayInputs(*g_pRaytracingDevice, pReflectionbarycentricPSO, pHitShaderTable.data(), shaderRecordSizeInBytes, (UINT)pHitShaderTable.size(), rayGenShaderExportName, missExportName);
    }

    {
        rayGenDxilLibSubobject = CreateDxilLibrary(rayGenShaderExportName, g_pRayGenerationShadowsLib, sizeof(g_pRayGenerationShadowsLib), rayGenDxilLibDesc, rayGenExportDesc);
        missDxilLibSubobject = CreateDxilLibrary(missExportName, g_pmissShadowsLib, sizeof(g_pmissShadowsLib), missDxilLibDesc, missExportDesc);

        CComPtr<ID3D12StateObject> pShadowsPSO;
        g_pRaytracingDevice->CreateStateObject(&stateObject, IID_PPV_ARGS(&pShadowsPSO));
        GetShaderTable(model, pShadowsPSO, pHitShaderTable.data());
        g_RaytracingInputs[Shadows] = RaytracingDispatchRayInputs(*g_pRaytracingDevice, pShadowsPSO, pHitShaderTable.data(), shaderRecordSizeInBytes, (UINT)pHitShaderTable.size(), rayGenShaderExportName, missExportName);
    }

    {
        rayGenDxilLibSubobject = CreateDxilLibrary(rayGenShaderExportName, g_pRayGenerationShaderLib, sizeof(g_pRayGenerationShaderLib), rayGenDxilLibDesc, rayGenExportDesc);
        hitGroupLibSubobject = CreateDxilLibrary(closestHitExportName, g_pDiffuseHitShaderLib, sizeof(g_pDiffuseHitShaderLib), hitGroupDxilLibDesc, hitGroupExportDesc);
        missDxilLibSubobject = CreateDxilLibrary(missExportName, g_pmissShaderLib, sizeof(g_pmissShaderLib), missDxilLibDesc, missExportDesc);

        CComPtr<ID3D12StateObject> pDiffusePSO;
        g_pRaytracingDevice->CreateStateObject(&stateObject, IID_PPV_ARGS(&pDiffusePSO));
        GetShaderTable(model, pDiffusePSO, pHitShaderTable.data());
        g_RaytracingInputs[DiffuseHitShader] = RaytracingDispatchRayInputs(*g_pRaytracingDevice, pDiffusePSO, pHitShaderTable.data(), shaderRecordSizeInBytes, (UINT)pHitShaderTable.size(), rayGenShaderExportName, missExportName);
    }

   {
        rayGenDxilLibSubobject = CreateDxilLibrary(rayGenShaderExportName, g_pRayGenerationShaderSSRLib, sizeof(g_pRayGenerationShaderSSRLib), rayGenDxilLibDesc, rayGenExportDesc);
        hitGroupLibSubobject = CreateDxilLibrary(closestHitExportName, g_pDiffuseHitShaderLib, sizeof(g_pDiffuseHitShaderLib), hitGroupDxilLibDesc, hitGroupExportDesc);
        missDxilLibSubobject = CreateDxilLibrary(missExportName, g_pmissShaderLib, sizeof(g_pmissShaderLib), missDxilLibDesc, missExportDesc);

        CComPtr<ID3D12StateObject> pReflectionPSO;
        g_pRaytracingDevice->CreateStateObject(&stateObject, IID_PPV_ARGS(&pReflectionPSO));
        GetShaderTable(model, pReflectionPSO, pHitShaderTable.data());
        g_RaytracingInputs[Reflection] = RaytracingDispatchRayInputs(*g_pRaytracingDevice, pReflectionPSO, pHitShaderTable.data(), shaderRecordSizeInBytes, (UINT)pHitShaderTable.size(), rayGenShaderExportName, missExportName);
    }

   for (auto &raytracingPipelineState : g_RaytracingInputs)
   {
        WCHAR hitGroupExportNameClosestHitType[64];
        swprintf_s(hitGroupExportNameClosestHitType, L"%s::closesthit", hitGroupExportName );
        SetPipelineStateStackSize(rayGenShaderExportName, hitGroupExportNameClosestHitType, missExportName, MaxRayRecursion, raytracingPipelineState.m_pPSO);
   }
}

void D3D12RaytracingMiniEngineSample::Startup( void )
{
    ThrowIfFailed(g_Device->QueryInterface(IID_PPV_ARGS(&g_pRaytracingDevice)), L"Couldn't get DirectX Raytracing interface for the device.\n");
    g_SceneNormalBuffer.Create(L"Main Normal Buffer", g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight(), 1, DXGI_FORMAT_R16G16B16A16_FLOAT);

    g_pRaytracingDescriptorHeap = std::unique_ptr<DescriptorHeapStack>(
        new DescriptorHeapStack(*g_Device, 200, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0));

    D3D12_FEATURE_DATA_D3D12_OPTIONS1 options1;
    HRESULT hr = g_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &options1, sizeof(options1));

    SamplerDesc DefaultSamplerDesc;
    DefaultSamplerDesc.MaxAnisotropy = 8;

    m_RootSig.Reset(6, 2);
    m_RootSig.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig.InitStaticSampler(1, SamplerShadowDesc, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
    m_RootSig[1].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 6, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 64, 6, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig[4].InitAsConstants(1, 2, D3D12_SHADER_VISIBILITY_VERTEX);
    m_RootSig[5].InitAsConstants(1, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig.Finalize(L"D3D12RaytracingMiniEngineSample", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    DXGI_FORMAT ColorFormat = g_SceneColorBuffer.GetFormat();
    DXGI_FORMAT NormalFormat = g_SceneNormalBuffer.GetFormat();
    DXGI_FORMAT DepthFormat = g_SceneDepthBuffer.GetFormat();
    DXGI_FORMAT ShadowFormat = g_ShadowBuffer.GetFormat();

    D3D12_INPUT_ELEMENT_DESC vertElem[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // Depth-only (2x rate)
    m_DepthPSO[0].SetRootSignature(m_RootSig);
    m_DepthPSO[0].SetRasterizerState(RasterizerDefault);
    m_DepthPSO[0].SetBlendState(BlendNoColorWrite);
    m_DepthPSO[0].SetDepthStencilState(DepthStateReadWrite);
    m_DepthPSO[0].SetInputLayout(_countof(vertElem), vertElem);
    m_DepthPSO[0].SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    m_DepthPSO[0].SetRenderTargetFormats(0, nullptr, DepthFormat);
    m_DepthPSO[0].SetVertexShader(g_pDepthViewerVS, sizeof(g_pDepthViewerVS));

    // Make a copy of the desc before we mess with it
    m_CutoutDepthPSO[0] = m_DepthPSO[0];
    m_ShadowPSO = m_DepthPSO[0];

    m_DepthPSO[0].Finalize();

    // Depth-only shading but with alpha testing

    m_CutoutDepthPSO[0].SetPixelShader(g_pDepthViewerPS, sizeof(g_pDepthViewerPS));
    m_CutoutDepthPSO[0].SetRasterizerState(RasterizerTwoSided);
    m_CutoutDepthPSO[0].Finalize();

    // Depth-only but with a depth bias and/or render only backfaces

    m_ShadowPSO.SetRasterizerState(RasterizerShadow);
    m_ShadowPSO.SetRenderTargetFormats(0, nullptr, g_ShadowBuffer.GetFormat());
    m_ShadowPSO.Finalize();

    // Shadows with alpha testing
    m_CutoutShadowPSO = m_ShadowPSO;
    m_CutoutShadowPSO.SetPixelShader(g_pDepthViewerPS, sizeof(g_pDepthViewerPS));
    m_CutoutShadowPSO.SetRasterizerState(RasterizerShadowTwoSided);
    m_CutoutShadowPSO.Finalize();

    // Full color pass
    m_ModelPSO[0] = m_DepthPSO[0];
    m_ModelPSO[0].SetBlendState(BlendDisable);
    m_ModelPSO[0].SetDepthStencilState(DepthStateTestEqual);
    DXGI_FORMAT formats[] { ColorFormat, NormalFormat };
    m_ModelPSO[0].SetRenderTargetFormats(_countof(formats), formats, DepthFormat);
    m_ModelPSO[0].SetVertexShader( g_pModelViewerVS, sizeof(g_pModelViewerVS) );
    m_ModelPSO[0].SetPixelShader( g_pModelViewerPS, sizeof(g_pModelViewerPS) );
    m_ModelPSO[0].Finalize();

    m_CutoutModelPSO[0] = m_ModelPSO[0];
    m_CutoutModelPSO[0].SetRasterizerState(RasterizerTwoSided);
    m_CutoutModelPSO[0].Finalize();

    // A debug shader for counting lights in a tile
    m_WaveTileCountPSO = m_ModelPSO[0];
    m_WaveTileCountPSO.SetPixelShader(g_pWaveTileCountPS, sizeof(g_pWaveTileCountPS));
    m_WaveTileCountPSO.Finalize();

    Lighting::InitializeResources();

    m_ExtraTextures[0] = g_SSAOFullScreen.GetSRV();
    m_ExtraTextures[1] = g_ShadowBuffer.GetSRV();

#define ASSET_DIRECTORY "../../../../../MiniEngine/ModelViewer/"
    TextureManager::Initialize(ASSET_DIRECTORY L"Textures/");
    bool bModelLoadSuccess = m_Model.Load(ASSET_DIRECTORY "Models/sponza.h3d");
    ASSERT(bModelLoadSuccess, "Failed to load model");
    ASSERT(m_Model.m_Header.meshCount > 0, "Model contains no meshes");

    // The caller of this function can override which materials are considered cutouts
    m_pMaterialIsCutout.resize(m_Model.m_Header.materialCount);
    m_pMaterialIsReflective.resize(m_Model.m_Header.materialCount);
    for (uint32_t i = 0; i < m_Model.m_Header.materialCount; ++i)
    {
        const Model::Material& mat = m_Model.m_pMaterial[i];
        if (std::string(mat.texDiffusePath).find("thorn") != std::string::npos ||
            std::string(mat.texDiffusePath).find("plant") != std::string::npos ||
            std::string(mat.texDiffusePath).find("chain") != std::string::npos)
        {
            m_pMaterialIsCutout[i] = true;
        }
        else
        {
            m_pMaterialIsCutout[i] = false;
        }

        if (std::string(mat.texDiffusePath).find("floor") != std::string::npos)
        {
            m_pMaterialIsReflective[i] = true;
        }
        else
        {
            m_pMaterialIsReflective[i] = false;
        }
    }

    g_hitConstantBuffer.Create(L"Hit Constant Buffer", 1, sizeof(HitShaderConstants));
    g_dynamicConstantBuffer.Create(L"Dynamic Constant Buffer", 1, sizeof(DynamicCB));

    InitializeSceneInfo(m_Model);
    InitializeViews(m_Model);
    UINT numMeshes = m_Model.m_Header.meshCount;

    const UINT numBottomLevels = 1;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo;
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelAccelerationStructureDesc = {};
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &topLevelInputs = topLevelAccelerationStructureDesc.Inputs;
    topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    topLevelInputs.NumDescs = numBottomLevels;
    topLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    topLevelInputs.pGeometryDescs = nullptr;
    topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    g_pRaytracingDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
    
    const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlag = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs(m_Model.m_Header.meshCount);
    UINT64 scratchBufferSizeNeeded = topLevelPrebuildInfo.ScratchDataSizeInBytes;
    for (UINT i = 0; i < numMeshes; i++)
    {
        auto &mesh = m_Model.m_pMesh[i];

        D3D12_RAYTRACING_GEOMETRY_DESC &desc = geometryDescs[i];
        desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
        desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

        D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC &trianglesDesc = desc.Triangles;
        trianglesDesc.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        trianglesDesc.VertexCount = mesh.vertexCount;
        trianglesDesc.VertexBuffer.StartAddress = m_Model.m_VertexBuffer.GetGpuVirtualAddress() + (mesh.vertexDataByteOffset + mesh.attrib[Model::attrib_position].offset);
        trianglesDesc.IndexBuffer = m_Model.m_IndexBuffer.GetGpuVirtualAddress() + mesh.indexDataByteOffset;
        trianglesDesc.VertexBuffer.StrideInBytes = mesh.vertexStride;
        trianglesDesc.IndexCount = mesh.indexCount;
        trianglesDesc.IndexFormat = DXGI_FORMAT_R16_UINT;
        trianglesDesc.Transform3x4 = 0;
    }

    std::vector<UINT64> bottomLevelAccelerationStructureSize(numBottomLevels);
    std::vector<D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC> bottomLevelAccelerationStructureDescs(numBottomLevels);
    for (UINT i = 0; i < numBottomLevels; i++)
    {
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC &bottomLevelAccelerationStructureDesc = bottomLevelAccelerationStructureDescs[i];
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &bottomLevelInputs = bottomLevelAccelerationStructureDesc.Inputs;
        bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        bottomLevelInputs.NumDescs = numMeshes;
        bottomLevelInputs.pGeometryDescs = &geometryDescs[i];
        bottomLevelInputs.Flags = buildFlag;
        bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelprebuildInfo;
        g_pRaytracingDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelprebuildInfo);

        bottomLevelAccelerationStructureSize[i] = bottomLevelprebuildInfo.ResultDataMaxSizeInBytes;
        scratchBufferSizeNeeded = std::max(bottomLevelprebuildInfo.ScratchDataSizeInBytes, scratchBufferSizeNeeded);
    }

    ByteAddressBuffer scratchBuffer;
    scratchBuffer.Create(L"Acceleration Structure Scratch Buffer", (UINT)scratchBufferSizeNeeded, 1);

    D3D12_HEAP_PROPERTIES defaultHeapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto topLevelDesc = CD3DX12_RESOURCE_DESC::Buffer(topLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    g_Device->CreateCommittedResource(
        &defaultHeapDesc,
        D3D12_HEAP_FLAG_NONE,
        &topLevelDesc,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        nullptr,
        IID_PPV_ARGS(&g_bvh_topLevelAccelerationStructure));

    topLevelAccelerationStructureDesc.DestAccelerationStructureData = g_bvh_topLevelAccelerationStructure->GetGPUVirtualAddress();
    topLevelAccelerationStructureDesc.ScratchAccelerationStructureData = scratchBuffer.GetGpuVirtualAddress();

    std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDescs(numBottomLevels);
    g_bvh_bottomLevelAccelerationStructures.resize(numBottomLevels);
    for (UINT i = 0; i < bottomLevelAccelerationStructureDescs.size(); i++)
    {
        auto &bottomLevelStructure = g_bvh_bottomLevelAccelerationStructures[i];

        auto bottomLevelDesc = CD3DX12_RESOURCE_DESC::Buffer(bottomLevelAccelerationStructureSize[i], D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        g_Device->CreateCommittedResource(
            &defaultHeapDesc,
            D3D12_HEAP_FLAG_NONE, 
            &bottomLevelDesc, 
            D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
            nullptr, 
            IID_PPV_ARGS(&bottomLevelStructure));

        bottomLevelAccelerationStructureDescs[i].DestAccelerationStructureData = bottomLevelStructure->GetGPUVirtualAddress();
        bottomLevelAccelerationStructureDescs[i].ScratchAccelerationStructureData = scratchBuffer.GetGpuVirtualAddress();

        D3D12_RAYTRACING_INSTANCE_DESC &instanceDesc = instanceDescs[i];
        UINT descriptorIndex = g_pRaytracingDescriptorHeap->AllocateBufferUav(*bottomLevelStructure);
        
        // Identity matrix
        ZeroMemory(instanceDesc.Transform, sizeof(instanceDesc.Transform));
        instanceDesc.Transform[0][0] = 1.0f;
        instanceDesc.Transform[1][1] = 1.0f;
        instanceDesc.Transform[2][2] = 1.0f;
        
        instanceDesc.AccelerationStructure = g_bvh_bottomLevelAccelerationStructures[i]->GetGPUVirtualAddress();
        instanceDesc.Flags = 0;
        instanceDesc.InstanceID = 0;
        instanceDesc.InstanceMask = 1;
        instanceDesc.InstanceContributionToHitGroupIndex = i;
    }

    ByteAddressBuffer instanceDataBuffer;
    instanceDataBuffer.Create(L"Instance Data Buffer", numBottomLevels, sizeof(D3D12_RAYTRACING_INSTANCE_DESC), instanceDescs.data());

    topLevelInputs.InstanceDescs = instanceDataBuffer.GetGpuVirtualAddress();
    topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;

    GraphicsContext& gfxContext = GraphicsContext::Begin(L"Create Acceleration Structure");
    ID3D12GraphicsCommandList *pCommandList = gfxContext.GetCommandList();

    CComPtr<ID3D12GraphicsCommandList4> pRaytracingCommandList;
    pCommandList->QueryInterface(IID_PPV_ARGS(&pRaytracingCommandList));

    ID3D12DescriptorHeap *descriptorHeaps[] = { &g_pRaytracingDescriptorHeap->GetDescriptorHeap() };
    pRaytracingCommandList->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);

    auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
    for (UINT i = 0; i < bottomLevelAccelerationStructureDescs.size(); i++)
    {
        pRaytracingCommandList->BuildRaytracingAccelerationStructure(&bottomLevelAccelerationStructureDescs[i], 0, nullptr);
    }
    pCommandList->ResourceBarrier(1, &uavBarrier);

    pRaytracingCommandList->BuildRaytracingAccelerationStructure(&topLevelAccelerationStructureDesc, 0, nullptr);
    
    gfxContext.Finish(true);

    InitializeRaytracingStateObjects(m_Model, numMeshes);

    float modelRadius = Length(m_Model.m_Header.boundingBox.max - m_Model.m_Header.boundingBox.min) * .5f;
    const Vector3 eye = (m_Model.m_Header.boundingBox.min + m_Model.m_Header.boundingBox.max) * .5f + Vector3(modelRadius * .5f, 0.0f, 0.0f);
    m_Camera.SetEyeAtUp( eye, Vector3(kZero), Vector3(kYUnitVector) );
    
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

    m_Camera.SetZRange( 1.0f, 10000.0f );

    m_CameraController.reset(new CameraController(m_Camera, Vector3(kYUnitVector)));
    
    MotionBlur::Enable = false;//true;
    TemporalEffects::EnableTAA = false;//true;
    FXAA::Enable = false;
    PostEffects::EnableHDR = false;//true;
    PostEffects::EnableAdaptation = false;//true;
    SSAO::Enable = true;

    Lighting::CreateRandomLights(m_Model.GetBoundingBox().min, m_Model.GetBoundingBox().max);

    m_ExtraTextures[2] = Lighting::m_LightBuffer.GetSRV();
    m_ExtraTextures[3] = Lighting::m_LightShadowArray.GetSRV();
    m_ExtraTextures[4] = Lighting::m_LightGrid.GetSRV();
    m_ExtraTextures[5] = Lighting::m_LightGridBitMask.GetSRV();
}

void D3D12RaytracingMiniEngineSample::Cleanup( void )
{
    m_Model.Clear();
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

    m_ViewProjMatrix = m_Camera.GetViewProjMatrix();

    float costheta = cosf(m_SunOrientation);
    float sintheta = sinf(m_SunOrientation);
    float cosphi = cosf(m_SunInclination * 3.14159f * 0.5f);
    float sinphi = sinf(m_SunInclination * 3.14159f * 0.5f);
    m_SunDirection = Normalize(Vector3( costheta * cosphi, sinphi, sintheta * cosphi ));

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

void D3D12RaytracingMiniEngineSample::RenderObjects( GraphicsContext& gfxContext, const Matrix4& ViewProjMat, eObjectFilter Filter )
{
    struct VSConstants
    {
        Matrix4 modelToProjection;
        Matrix4 modelToShadow;
        XMFLOAT3 viewerPos;
    } vsConstants;
    vsConstants.modelToProjection = ViewProjMat;
    vsConstants.modelToShadow = m_SunShadow.GetShadowMatrix();
    XMStoreFloat3(&vsConstants.viewerPos, m_Camera.GetPosition());

    gfxContext.SetDynamicConstantBufferView(0, sizeof(vsConstants), &vsConstants);

    uint32_t materialIdx = 0xFFFFFFFFul;

    uint32_t VertexStride = m_Model.m_VertexStride;

    for (uint32_t meshIndex = 0; meshIndex < m_Model.m_Header.meshCount; meshIndex++)
    {
        const Model::Mesh& mesh = m_Model.m_pMesh[meshIndex];

        uint32_t indexCount = mesh.indexCount;
        uint32_t startIndex = mesh.indexDataByteOffset / sizeof(uint16_t);
        uint32_t baseVertex = mesh.vertexDataByteOffset / VertexStride;

        if (mesh.materialIndex != materialIdx)
        {
            if ( m_pMaterialIsCutout[mesh.materialIndex] && !(Filter & kCutout) ||
                !m_pMaterialIsCutout[mesh.materialIndex] && !(Filter & kOpaque) )
                continue;

            materialIdx = mesh.materialIndex;
            gfxContext.SetDynamicDescriptors(2, 0, 6, m_Model.GetSRVs(materialIdx) );
        }
        uint32_t areNormalsNeeded = 1;// (rayTracingMode != RTM_REFLECTIONS) || m_pMaterialIsReflective[mesh.materialIndex];
        gfxContext.SetConstants(4, baseVertex, materialIdx);
        gfxContext.SetConstants(5, areNormalsNeeded);

        gfxContext.DrawIndexed(indexCount, startIndex, baseVertex);
    }
}


void D3D12RaytracingMiniEngineSample::SetCameraToPredefinedPosition(int cameraPosition) 
{
    if (cameraPosition < 0 || cameraPosition >= c_NumCameraPositions)
        return;
    
    m_CameraController->SetCurrentHeading(m_CameraPosArray[m_CameraPosArrayCurrentPosition].heading);
    m_CameraController->SetCurrentPitch(m_CameraPosArray[m_CameraPosArrayCurrentPosition].pitch);

    Matrix3 neworientation = Matrix3(m_CameraController->GetWorldEast(), m_CameraController->GetWorldUp(), -m_CameraController->GetWorldNorth()) 
                           * Matrix3::MakeYRotation(m_CameraController->GetCurrentHeading())
                           * Matrix3::MakeXRotation(m_CameraController->GetCurrentPitch());
    m_Camera.SetTransform(AffineTransform(neworientation, m_CameraPosArray[m_CameraPosArrayCurrentPosition].position));
    m_Camera.Update();
}

void D3D12RaytracingMiniEngineSample::RenderLightShadows(GraphicsContext& gfxContext)
{
    using namespace Lighting;

    ScopedTimer _prof(L"RenderLightShadows", gfxContext);

    static uint32_t LightIndex = 0;
    if (LightIndex >= MaxLights)
        return;
    
    m_LightShadowTempBuffer.BeginRendering(gfxContext);
    {
        gfxContext.SetPipelineState(m_ShadowPSO);
        RenderObjects(gfxContext, m_LightShadowMatrix[LightIndex], kOpaque);
        gfxContext.SetPipelineState(m_CutoutShadowPSO);
        RenderObjects(gfxContext, m_LightShadowMatrix[LightIndex], kCutout);
    }
    m_LightShadowTempBuffer.EndRendering(gfxContext);

    gfxContext.TransitionResource(m_LightShadowTempBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
    gfxContext.TransitionResource(m_LightShadowArray, D3D12_RESOURCE_STATE_COPY_DEST);

    gfxContext.CopySubresource(m_LightShadowArray, LightIndex, m_LightShadowTempBuffer, 0);

    gfxContext.TransitionResource(m_LightShadowArray, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    ++LightIndex;
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

    static bool s_ShowLightCounts = false;
    if (ShowWaveTileCounts != s_ShowLightCounts)
    {
        static bool EnableHDR;
        if (ShowWaveTileCounts)
        {
            EnableHDR = PostEffects::EnableHDR;
            PostEffects::EnableHDR = false;
        }
        else
        {
            PostEffects::EnableHDR = EnableHDR;
        }
        s_ShowLightCounts = ShowWaveTileCounts;
    }

    GraphicsContext& gfxContext = GraphicsContext::Begin(L"Scene Render");

    ParticleEffects::Update(gfxContext.GetComputeContext(), Graphics::GetFrameTime());

    uint32_t FrameIndex = TemporalEffects::GetFrameIndexMod2();

    __declspec(align(16)) struct
    {
        Vector3 sunDirection;
        Vector3 sunLight;
        Vector3 ambientLight;
        float ShadowTexelSize[4];

        float InvTileDim[4];
        uint32_t TileCount[4];
        uint32_t FirstLightIndex[4];
        uint32_t FrameIndexMod2;
    } psConstants;

    psConstants.sunDirection = m_SunDirection;
    psConstants.sunLight = Vector3(1.0f, 1.0f, 1.0f) * m_SunLightIntensity;
    psConstants.ambientLight = Vector3(1.0f, 1.0f, 1.0f) * m_AmbientIntensity;
    psConstants.ShadowTexelSize[0] = 1.0f / g_ShadowBuffer.GetWidth();
    psConstants.InvTileDim[0] = 1.0f / Lighting::LightGridDim;
    psConstants.InvTileDim[1] = 1.0f / Lighting::LightGridDim;
    psConstants.TileCount[0] = Math::DivideByMultiple(g_SceneColorBuffer.GetWidth(), Lighting::LightGridDim);
    psConstants.TileCount[1] = Math::DivideByMultiple(g_SceneColorBuffer.GetHeight(), Lighting::LightGridDim);
    psConstants.FirstLightIndex[0] = Lighting::m_FirstConeLight;
    psConstants.FirstLightIndex[1] = Lighting::m_FirstConeShadowedLight;
    psConstants.FrameIndexMod2 = FrameIndex;

    // Set the default state for command lists
    auto& pfnSetupGraphicsState = [&](void)
    {
        gfxContext.SetRootSignature(m_RootSig);
        gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        gfxContext.SetIndexBuffer(m_Model.m_IndexBuffer.IndexBufferView());
        gfxContext.SetVertexBuffer(0, m_Model.m_VertexBuffer.VertexBufferView());
    };

    pfnSetupGraphicsState();

        RenderLightShadows(gfxContext);

    {
        ScopedTimer _prof(L"Z PrePass", gfxContext);

        gfxContext.SetDynamicConstantBufferView(1, sizeof(psConstants), &psConstants);

        {
            ScopedTimer _prof(L"Opaque", gfxContext);
            {
                gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
                gfxContext.ClearDepth(g_SceneDepthBuffer);

                gfxContext.SetPipelineState(m_DepthPSO[0]);
                gfxContext.SetDepthStencilTarget(g_SceneDepthBuffer.GetDSV());

                gfxContext.SetViewportAndScissor(m_MainViewport, m_MainScissor);
            }

            RenderObjects(gfxContext, m_ViewProjMatrix, kOpaque);
        }

        {
            ScopedTimer _prof(L"Cutout", gfxContext);
            {
                gfxContext.SetPipelineState(m_CutoutDepthPSO[0]);
            }
            RenderObjects(gfxContext, m_ViewProjMatrix, kCutout);
        }
    }

    SSAO::Render(gfxContext, m_Camera);

    if (!skipDiffusePass)
    {
        Lighting::FillLightGrid(gfxContext, m_Camera);

        if (!SSAO::DebugDraw)
        {
            ScopedTimer _prof(L"Main Render", gfxContext);
            {
                gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
                gfxContext.TransitionResource(g_SceneNormalBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
                gfxContext.ClearColor(g_SceneColorBuffer);
            }
        }
    }

    if (!skipShadowMap)
    {
        if (!SSAO::DebugDraw)
        {
            pfnSetupGraphicsState();
            {
                ScopedTimer _prof(L"Render Shadow Map", gfxContext);

                m_SunShadow.UpdateMatrix(-m_SunDirection, Vector3(0, -500.0f, 0), Vector3(ShadowDimX, ShadowDimY, ShadowDimZ),
                    (uint32_t)g_ShadowBuffer.GetWidth(), (uint32_t)g_ShadowBuffer.GetHeight(), 16);

                g_ShadowBuffer.BeginRendering(gfxContext);
                gfxContext.SetPipelineState(m_ShadowPSO);
                RenderObjects(gfxContext, m_SunShadow.GetViewProjMatrix(), kOpaque);
                gfxContext.SetPipelineState(m_CutoutShadowPSO);
                RenderObjects(gfxContext, m_SunShadow.GetViewProjMatrix(), kCutout);
                g_ShadowBuffer.EndRendering(gfxContext);
            }
        }
    }

    if (!skipDiffusePass)
    {
        if (!SSAO::DebugDraw)
        {
            if (SSAO::AsyncCompute)
            {
                gfxContext.Flush();
                pfnSetupGraphicsState();

                // Make the 3D queue wait for the Compute queue to finish SSAO
                g_CommandManager.GetGraphicsQueue().StallForProducer(g_CommandManager.GetComputeQueue());
            }

            {
                ScopedTimer _prof(L"Render Color", gfxContext);

                gfxContext.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

                gfxContext.SetDynamicDescriptors(3, 0, ARRAYSIZE(m_ExtraTextures), m_ExtraTextures);
                gfxContext.SetDynamicConstantBufferView(1, sizeof(psConstants), &psConstants);

                bool RenderIDs = !TemporalEffects::EnableTAA;

                {
                    gfxContext.SetPipelineState(ShowWaveTileCounts ? m_WaveTileCountPSO : m_ModelPSO[0]);
                    gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
                    D3D12_CPU_DESCRIPTOR_HANDLE rtvs[]{ g_SceneColorBuffer.GetRTV(), g_SceneNormalBuffer.GetRTV() };
                    gfxContext.SetRenderTargets(ARRAYSIZE(rtvs), rtvs, g_SceneDepthBuffer.GetDSV_DepthReadOnly());
                    gfxContext.SetViewportAndScissor(m_MainViewport, m_MainScissor);
                }

                RenderObjects(gfxContext, m_ViewProjMatrix, kOpaque);

                if (!ShowWaveTileCounts)
                {
                    gfxContext.SetPipelineState(m_CutoutModelPSO[0]);
                    RenderObjects(gfxContext, m_ViewProjMatrix, kCutout);
                }
            }

        }

        // Some systems generate a per-pixel velocity buffer to better track dynamic and skinned meshes.  Everything
        // is static in our scene, so we generate velocity from camera motion and the depth buffer.  A velocity buffer
        // is necessary for all temporal effects (and motion blur).
        MotionBlur::GenerateCameraVelocityBuffer(gfxContext, m_Camera, true);

        TemporalEffects::ResolveImage(gfxContext);

        ParticleEffects::Render(gfxContext, m_Camera, g_SceneColorBuffer, g_SceneDepthBuffer, g_LinearDepth[FrameIndex]);

        // Until I work out how to couple these two, it's "either-or".
        if (DepthOfField::Enable)
            DepthOfField::Render(gfxContext, m_Camera.GetNearClip(), m_Camera.GetFarClip());
        else
            MotionBlur::RenderObjectBlur(gfxContext, g_VelocityBuffer);
    }
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

    CComPtr<ID3D12GraphicsCommandList4> pRaytracingCommandList;
    pCommandList->QueryInterface(IID_PPV_ARGS(&pRaytracingCommandList));

    ID3D12DescriptorHeap *pDescriptorHeaps[] = { &g_pRaytracingDescriptorHeap->GetDescriptorHeap() };
    pRaytracingCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);

    pCommandList->SetComputeRootSignature(g_GlobalRaytracingRootSignature);
    pCommandList->SetComputeRootDescriptorTable(0, g_SceneSrvs);
    pCommandList->SetComputeRootConstantBufferView(1, g_hitConstantBuffer.GetGpuVirtualAddress());
    pCommandList->SetComputeRootConstantBufferView(2, g_dynamicConstantBuffer.GetGpuVirtualAddress());
    pCommandList->SetComputeRootDescriptorTable(4, g_OutputUAV);
    pRaytracingCommandList->SetComputeRootShaderResourceView(7, g_bvh_topLevelAccelerationStructure->GetGPUVirtualAddress());

    D3D12_DISPATCH_RAYS_DESC dispatchRaysDesc = g_RaytracingInputs[Primarybarycentric].GetDispatchRayDesc(colorTarget.GetWidth(), colorTarget.GetHeight());
    pRaytracingCommandList->SetPipelineState1(g_RaytracingInputs[Primarybarycentric].m_pPSO);
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

    CComPtr<ID3D12GraphicsCommandList4> pRaytracingCommandList;
    pCommandList->QueryInterface(IID_PPV_ARGS(&pRaytracingCommandList));

    ID3D12DescriptorHeap *pDescriptorHeaps[] = { &g_pRaytracingDescriptorHeap->GetDescriptorHeap() };
    pRaytracingCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);

    pCommandList->SetComputeRootSignature(g_GlobalRaytracingRootSignature);
    pCommandList->SetComputeRootConstantBufferView(1, g_hitConstantBuffer.GetGpuVirtualAddress());
    pCommandList->SetComputeRootConstantBufferView(2, g_dynamicConstantBuffer.GetGpuVirtualAddress());
    pCommandList->SetComputeRootDescriptorTable(4, g_OutputUAV);
    pCommandList->SetComputeRootDescriptorTable(3, g_DepthAndNormalsTable);
    pRaytracingCommandList->SetComputeRootShaderResourceView(7, g_bvh_topLevelAccelerationStructure->GetGPUVirtualAddress());

    D3D12_DISPATCH_RAYS_DESC dispatchRaysDesc = g_RaytracingInputs[Reflectionbarycentric].GetDispatchRayDesc(colorTarget.GetWidth(), colorTarget.GetHeight());
    pRaytracingCommandList->SetPipelineState1(g_RaytracingInputs[Reflectionbarycentric].m_pPSO);
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
    hitShaderConstants.sunDirection = m_SunDirection;
    hitShaderConstants.sunLight = Vector3(1.0f, 1.0f, 1.0f) * m_SunLightIntensity;
    hitShaderConstants.ambientLight = Vector3(1.0f, 1.0f, 1.0f) * m_AmbientIntensity;
    hitShaderConstants.ShadowTexelSize[0] = 1.0f / g_ShadowBuffer.GetWidth();
    hitShaderConstants.modelToShadow = m_SunShadow.GetShadowMatrix();
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

    CComPtr<ID3D12GraphicsCommandList4> pRaytracingCommandList;
    pCommandList->QueryInterface(IID_PPV_ARGS(&pRaytracingCommandList));

    ID3D12DescriptorHeap *pDescriptorHeaps[] = { &g_pRaytracingDescriptorHeap->GetDescriptorHeap() };
    pRaytracingCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);

    pCommandList->SetComputeRootSignature(g_GlobalRaytracingRootSignature);
    pCommandList->SetComputeRootConstantBufferView(1, g_hitConstantBuffer.GetGpuVirtualAddress());
    pCommandList->SetComputeRootConstantBufferView(2, g_dynamicConstantBuffer->GetGPUVirtualAddress());
    pCommandList->SetComputeRootDescriptorTable(4, g_OutputUAV);
    pCommandList->SetComputeRootDescriptorTable(3, g_DepthAndNormalsTable);
    pRaytracingCommandList->SetComputeRootShaderResourceView(7, g_bvh_topLevelAccelerationStructure->GetGPUVirtualAddress());

    D3D12_DISPATCH_RAYS_DESC dispatchRaysDesc = g_RaytracingInputs[Shadows].GetDispatchRayDesc(colorTarget.GetWidth(), colorTarget.GetHeight());
    pRaytracingCommandList->SetPipelineState1(g_RaytracingInputs[Shadows].m_pPSO);
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
    hitShaderConstants.sunDirection = m_SunDirection;
    hitShaderConstants.sunLight = Vector3(1.0f, 1.0f, 1.0f) * m_SunLightIntensity;
    hitShaderConstants.ambientLight = Vector3(1.0f, 1.0f, 1.0f) * m_AmbientIntensity;
    hitShaderConstants.ShadowTexelSize[0] = 1.0f / g_ShadowBuffer.GetWidth();
    hitShaderConstants.modelToShadow = Transpose(m_SunShadow.GetShadowMatrix());
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

    CComPtr<ID3D12GraphicsCommandList4> pRaytracingCommandList;
    pCommandList->QueryInterface(IID_PPV_ARGS(&pRaytracingCommandList));

    ID3D12DescriptorHeap *pDescriptorHeaps[] = { &g_pRaytracingDescriptorHeap->GetDescriptorHeap() };
    pRaytracingCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);

    pCommandList->SetComputeRootSignature(g_GlobalRaytracingRootSignature);
    pCommandList->SetComputeRootDescriptorTable(0, g_SceneSrvs);
    pCommandList->SetComputeRootConstantBufferView(1, g_hitConstantBuffer.GetGpuVirtualAddress());
    pCommandList->SetComputeRootConstantBufferView(2, g_dynamicConstantBuffer.GetGpuVirtualAddress());
    pCommandList->SetComputeRootDescriptorTable(4, g_OutputUAV);
    pRaytracingCommandList->SetComputeRootShaderResourceView(7, g_bvh_topLevelAccelerationStructure->GetGPUVirtualAddress());

    D3D12_DISPATCH_RAYS_DESC dispatchRaysDesc = g_RaytracingInputs[DiffuseHitShader].GetDispatchRayDesc(colorTarget.GetWidth(), colorTarget.GetHeight());
    pRaytracingCommandList->SetPipelineState1(g_RaytracingInputs[DiffuseHitShader].m_pPSO);
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
    hitShaderConstants.sunDirection = m_SunDirection;
    hitShaderConstants.sunLight = Vector3(1.0f, 1.0f, 1.0f) * m_SunLightIntensity;
    hitShaderConstants.ambientLight = Vector3(1.0f, 1.0f, 1.0f) * m_AmbientIntensity;
    hitShaderConstants.ShadowTexelSize[0] = 1.0f / g_ShadowBuffer.GetWidth();
    hitShaderConstants.modelToShadow = Transpose(m_SunShadow.GetShadowMatrix());
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

    CComPtr<ID3D12GraphicsCommandList4> pRaytracingCommandList;
    pCommandList->QueryInterface(IID_PPV_ARGS(&pRaytracingCommandList));

    ID3D12DescriptorHeap *pDescriptorHeaps[] = { &g_pRaytracingDescriptorHeap->GetDescriptorHeap() };
    pRaytracingCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);

    pCommandList->SetComputeRootSignature(g_GlobalRaytracingRootSignature);
    pCommandList->SetComputeRootDescriptorTable(0, g_SceneSrvs);
    pCommandList->SetComputeRootConstantBufferView(1, g_hitConstantBuffer.GetGpuVirtualAddress());
    pCommandList->SetComputeRootConstantBufferView(2, g_dynamicConstantBuffer.GetGpuVirtualAddress());
    pCommandList->SetComputeRootDescriptorTable(3, g_DepthAndNormalsTable);
    pCommandList->SetComputeRootDescriptorTable(4, g_OutputUAV);
    pRaytracingCommandList->SetComputeRootShaderResourceView(7, g_bvh_topLevelAccelerationStructure->GetGPUVirtualAddress());

    D3D12_DISPATCH_RAYS_DESC dispatchRaysDesc = g_RaytracingInputs[Reflection].GetDispatchRayDesc(colorTarget.GetWidth(), colorTarget.GetHeight());
    pRaytracingCommandList->SetPipelineState1(g_RaytracingInputs[Reflection].m_pPSO);
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

    uint32_t FrameIndex = TemporalEffects::GetFrameIndexMod2();

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
