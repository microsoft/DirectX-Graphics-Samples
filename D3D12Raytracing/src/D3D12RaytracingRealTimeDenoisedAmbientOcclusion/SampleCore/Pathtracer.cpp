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
#include "Pathtracer.h"
#include "GameInput.h"
#include "EngineTuning.h"
#include "EngineProfiling.h"
#include "GpuTimeManager.h"
#include "D3D12RaytracingRealTimeDenoisedAmbientOcclusion.h"
#include "CompiledShaders\Pathtracer.hlsl.h"

using namespace std;
using namespace DX;
using namespace DirectX;
using namespace SceneEnums;


namespace GlobalRootSignature {
    namespace Slot {
        enum Enum {
            Output = 0,
            GBufferResources,
            AccelerationStructure,
            ConstantBuffer,
            MaterialBuffer,
            SampleBuffers,
            EnvironmentMap,
            GbufferNormalRGB,
            PrevFrameBottomLevelASIstanceTransforms,
            MotionVector,
            ReprojectedNormalDepth,
            Color,
            AOSurfaceAlbedo,
            Debug1,
            Debug2,
            Count
        };
    }
}


namespace LocalRootSignature {
    namespace Slot {
        enum Enum {
            ConstantBuffer = 0,
            IndexBuffer,
            VertexBuffer,
            PreviousFrameVertexBuffer,
            DiffuseTexture,
            NormalTexture,
            Count
        };
    }
    struct RootArguments {
        PrimitiveConstantBuffer cb;
        // Bind each resource via a descriptor.
        // This design was picked for simplicity, but one could optimize for shader record size by:
        //    1) Binding multiple descriptors via a range descriptor instead.
        //    2) Storing 4 Byte indices (instead of 8 Byte descriptors) to a global pool resources.
        D3D12_GPU_DESCRIPTOR_HANDLE indexBufferGPUHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE vertexBufferGPUHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE previousFrameVertexBufferGPUHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE diffuseTextureGPUHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE normalTextureGPUHandle;
    };
}

// Shader entry points.
const wchar_t* Pathtracer::c_rayGenShaderNames[] = { L"MyRayGenShader_RadianceRay" };
const wchar_t* Pathtracer::c_closestHitShaderNames[] = { L"MyClosestHitShader_RadianceRay", L"MyClosestHitShader_ShadowRay" };
const wchar_t* Pathtracer::c_missShaderNames[] = { L"MyMissShader_RadianceRay", L"MyMissShader_ShadowRay" };

// Hit groups.
const wchar_t* Pathtracer::c_hitGroupNames[] = { L"MyHitGroup_Triangle_RadianceRay", L"MyHitGroup_Triangle_ShadowRay" };

// Singleton instance.
Pathtracer* g_pPathracer = nullptr;

Pathtracer* instance()
{
    return g_pPathracer;
}

void OnRecreateRTAORaytracingResources(void*)
{
    g_pPathracer->RequestRecreateRaytracingResources();
}

void OnRecreateSampleRaytracingResources(void*)
{
    Sample::instance().RequestRecreateRaytracingResources();
}

namespace Pathtracer_Args
{

    // Default ambient intensity for hitPositions that don't have a calculated Ambient coefficient.
    // Calculating AO just for a single hitPosition per pixel can cause visible visual differences
    // in bounces off surfaces that have non-zero Albedo, such as reflection on car paint at sharp angles. 
    // With default Ambient coefficient added to every hit along the ray, the visual difference is decreased.
    NumVar DefaultAmbientIntensity(L"Render/PathTracing/Default ambient intensity", 0.4f, 0, 1, 0.01f);

#define MAX_RAY_RECURSION_DEPTH 5
    IntVar MaxRadianceRayRecursionDepth(L"Render/PathTracing/Max Radiance Ray recursion depth", 3, 1, MAX_RAY_RECURSION_DEPTH, 1);
    IntVar MaxShadowRayRecursionDepth(L"Render/PathTracing/Max Shadow Ray recursion depth", 4, 1, MAX_RAY_RECURSION_DEPTH, 1);
       
    BoolVar RTAOUseNormalMaps(L"Render/PathTracing/Normal maps", false);
    const WCHAR* FloatingPointFormatsRG[TextureResourceFormatRG::Count] = { L"R32G32_FLOAT", L"R16G16_FLOAT", L"R8G8_SNORM" };
    EnumVar RTAO_PartialDepthDerivativesResourceFormat(L"Render/Texture Formats/PartialDepthDerivatives", TextureResourceFormatRG::R16G16_FLOAT, TextureResourceFormatRG::Count, FloatingPointFormatsRG, Sample::OnRecreateRaytracingResources);
    EnumVar RTAO_MotionVectorResourceFormat(L"Render/Texture Formats/AO/RTAO/Temporal Supersampling/Motion Vector", TextureResourceFormatRG::R16G16_FLOAT, TextureResourceFormatRG::Count, FloatingPointFormatsRG, Sample::OnRecreateRaytracingResources);
}

GpuResource(&Pathtracer::GBufferResources(bool getQuarterResResources))[GBufferResource::Count]
{
    if (getQuarterResResources)
        return m_GBufferQuarterResResources;
    else
        return m_GBufferResources;
}
    
Pathtracer::Pathtracer()
{
    ThrowIfFalse(g_pPathracer == nullptr, L"There can be only one Pathtracer instance.");
    g_pPathracer = this;

    for (auto& rayGenShaderTableRecordSizeInBytes : m_rayGenShaderTableRecordSizeInBytes)
    {
        rayGenShaderTableRecordSizeInBytes = UINT_MAX;
    }
}

void Pathtracer::Setup(shared_ptr<DeviceResources> deviceResources, shared_ptr<DX::DescriptorHeap> descriptorHeap, Scene& scene)
{
    m_deviceResources = deviceResources;
    m_cbvSrvUavHeap = descriptorHeap;

    CreateDeviceDependentResources(scene);
}

// Create resources that depend on the device.
void Pathtracer::CreateDeviceDependentResources(Scene& scene)
{
    CreateAuxilaryDeviceResources();

    // Initialize raytracing pipeline.

    // Create root signatures for the shaders.
    CreateRootSignatures();

    // Create a raytracing pipeline state object which defines the binding of shaders, state and resources to be used during raytracing.
    CreateRaytracingPipelineStateObject();

    // Create constant buffers for the geometry and the scene.
    CreateConstantBuffers();

    // Build shader tables, which define shaders and their local root arguments.
    BuildShaderTables(scene);
}

void Pathtracer::CreateAuxilaryDeviceResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandQueue = m_deviceResources->GetCommandQueue();
    auto commandList = m_deviceResources->GetCommandList();
    auto FrameCount = m_deviceResources->GetBackBufferCount();

    m_calculatePartialDerivativesKernel.Initialize(device, FrameCount);
    m_downsampleGBufferBilateralFilterKernel.Initialize(device, FrameCount);

    // Create null resource descriptor for the unused second VB in non-animated geometry.
    D3D12_CPU_DESCRIPTOR_HANDLE nullCPUhandle;
    UINT nullHeapIndex = UINT_MAX;
    CreateBufferSRV(nullptr, device, 0, sizeof(VertexPositionNormalTextureTangent), m_cbvSrvUavHeap.get(), &nullCPUhandle, &m_nullVertexBufferGPUhandle, &nullHeapIndex);
}

// Create constant buffers.
void Pathtracer::CreateConstantBuffers()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto FrameCount = m_deviceResources->GetBackBufferCount();

    m_CB.Create(device, FrameCount, L"Pathtracer Constant Buffer");
}


void Pathtracer::CreateRootSignatures()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Global Root Signature
    // This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
    {
        using namespace GlobalRootSignature;

        CD3DX12_DESCRIPTOR_RANGE ranges[Slot::Count]; // Perfomance TIP: Order from most frequent to least frequent.
        ranges[Slot::Output].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 1 output textures
        ranges[Slot::GBufferResources].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 3, 7);  // 3 output GBuffer textures
        ranges[Slot::GbufferNormalRGB].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 14);  // 1 output normal texture
        ranges[Slot::MotionVector].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 17);  // 1 output texture space motion vector.
        ranges[Slot::ReprojectedNormalDepth].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 18);  // 1 output texture reprojected hit position
        ranges[Slot::Color].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 19);  // 1 output texture shaded color
        ranges[Slot::AOSurfaceAlbedo].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 20);  // 1 output texture AO diffuse
        ranges[Slot::Debug1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 21); 
        ranges[Slot::Debug2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 22); 

        ranges[Slot::EnvironmentMap].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 12);  // 1 input environment map texture

        CD3DX12_ROOT_PARAMETER rootParameters[Slot::Count];
        rootParameters[Slot::Output].InitAsDescriptorTable(1, &ranges[Slot::Output]);
        rootParameters[Slot::GBufferResources].InitAsDescriptorTable(1, &ranges[Slot::GBufferResources]);
        rootParameters[Slot::EnvironmentMap].InitAsDescriptorTable(1, &ranges[Slot::EnvironmentMap]);
        rootParameters[Slot::GbufferNormalRGB].InitAsDescriptorTable(1, &ranges[Slot::GbufferNormalRGB]);
        rootParameters[Slot::MotionVector].InitAsDescriptorTable(1, &ranges[Slot::MotionVector]);
        rootParameters[Slot::ReprojectedNormalDepth].InitAsDescriptorTable(1, &ranges[Slot::ReprojectedNormalDepth]);
        rootParameters[Slot::Color].InitAsDescriptorTable(1, &ranges[Slot::Color]);
        rootParameters[Slot::AOSurfaceAlbedo].InitAsDescriptorTable(1, &ranges[Slot::AOSurfaceAlbedo]);
        rootParameters[Slot::Debug1].InitAsDescriptorTable(1, &ranges[Slot::Debug1]);
        rootParameters[Slot::Debug2].InitAsDescriptorTable(1, &ranges[Slot::Debug2]);

        rootParameters[Slot::AccelerationStructure].InitAsShaderResourceView(0);
        rootParameters[Slot::ConstantBuffer].InitAsConstantBufferView(0);
        rootParameters[Slot::MaterialBuffer].InitAsShaderResourceView(3);
        rootParameters[Slot::SampleBuffers].InitAsShaderResourceView(4);
        rootParameters[Slot::PrevFrameBottomLevelASIstanceTransforms].InitAsShaderResourceView(15);

        CD3DX12_STATIC_SAMPLER_DESC staticSamplers[] =
        {
            // LinearWrapSampler
            CD3DX12_STATIC_SAMPLER_DESC(0, SAMPLER_FILTER),
        };

        CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters, ARRAYSIZE(staticSamplers), staticSamplers);
        SerializeAndCreateRootSignature(device, globalRootSignatureDesc, &m_raytracingGlobalRootSignature, L"Global root signature");
    }

    // Local Root Signature
    // This is a root signature that enables a shader to have unique arguments that come from shader tables.
    {
        // Triangle geometry
        {
            using namespace LocalRootSignature;

            CD3DX12_DESCRIPTOR_RANGE ranges[Slot::Count]; // Perfomance TIP: Order from most frequent to least frequent.
            ranges[Slot::IndexBuffer].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);  // 1 buffer - index buffer.
            ranges[Slot::VertexBuffer].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 1);  // 1 buffer - current frame vertex buffer.
            ranges[Slot::PreviousFrameVertexBuffer].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 1);  // 1 buffer - previous frame vertex buffer.
            ranges[Slot::DiffuseTexture].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 1);  // 1 diffuse texture
            ranges[Slot::NormalTexture].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 1);  // 1 normal texture

            CD3DX12_ROOT_PARAMETER rootParameters[Slot::Count];
            rootParameters[Slot::ConstantBuffer].InitAsConstants(SizeOfInUint32(PrimitiveConstantBuffer), 0, 1);
            rootParameters[Slot::IndexBuffer].InitAsDescriptorTable(1, &ranges[Slot::IndexBuffer]);
            rootParameters[Slot::VertexBuffer].InitAsDescriptorTable(1, &ranges[Slot::VertexBuffer]);
            rootParameters[Slot::PreviousFrameVertexBuffer].InitAsDescriptorTable(1, &ranges[Slot::PreviousFrameVertexBuffer]);
            rootParameters[Slot::DiffuseTexture].InitAsDescriptorTable(1, &ranges[Slot::DiffuseTexture]);
            rootParameters[Slot::NormalTexture].InitAsDescriptorTable(1, &ranges[Slot::NormalTexture]);

            CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
            localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
            SerializeAndCreateRootSignature(device, localRootSignatureDesc, &m_raytracingLocalRootSignature, L"Local root signature");
        }
    }
}


// DXIL library
// This contains the shaders and their entrypoints for the state object.
// Since shaders are not considered a subobject, they need to be passed in via DXIL library subobjects.
void Pathtracer::CreateDxilLibrarySubobject(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
{
    auto lib = raytracingPipeline->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)g_pPathtracer, ARRAYSIZE(g_pPathtracer));
    lib->SetDXILLibrary(&libdxil);
    // Use default shader exports for a DXIL library/collection subobject ~ surface all shaders.
}

// Hit groups
// A hit group specifies closest hit, any hit and intersection shaders 
// to be executed when a ray intersects the geometry.
void Pathtracer::CreateHitGroupSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
{
    // Triangle geometry hit groups
    {
        for (UINT rayType = 0; rayType < PathtracerRayType::Count; rayType++)
        {
            auto hitGroup = raytracingPipeline->CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();

            if (c_closestHitShaderNames[rayType])
            {

                hitGroup->SetClosestHitShaderImport(c_closestHitShaderNames[rayType]);
            }
            hitGroup->SetHitGroupExport(c_hitGroupNames[rayType]);
            hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
        }
    }
}

// Local root signature and shader association
// This is a root signature that enables a shader to have unique arguments that come from shader tables.
void Pathtracer::CreateLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
{
    // Ray gen and miss shaders in this sample are not using a local root signature and thus one is not associated with them.

    // Hit groups
    // Triangle geometry
    {
        auto localRootSignature = raytracingPipeline->CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
        localRootSignature->SetRootSignature(m_raytracingLocalRootSignature.Get());
        // Shader association
        auto rootSignatureAssociation = raytracingPipeline->CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
        rootSignatureAssociation->AddExports(c_hitGroupNames);
    }
}

// Create a raytracing pipeline state object (RTPSO).
// An RTPSO represents a full set of shaders reachable by a DispatchRays() call,
// with all configuration options resolved, such as local root signatures and other state.
void Pathtracer::CreateRaytracingPipelineStateObject()
{
    auto device = m_deviceResources->GetD3DDevice();
    // Pathracing state object.
    {
        CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

        // DXIL library
        CreateDxilLibrarySubobject(&raytracingPipeline);

        // Hit groups
        CreateHitGroupSubobjects(&raytracingPipeline);

        // Shader config
        // Defines the maximum sizes in bytes for the ray rayPayload and attribute structure.
        auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
        UINT payloadSize = static_cast<UINT>(max(sizeof(ShadowRayPayload), sizeof(PathtracerRayPayload)));

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
        UINT maxRecursionDepth = MAX_RAY_RECURSION_DEPTH;
        pipelineConfig->Config(maxRecursionDepth);

        PrintStateObjectDesc(raytracingPipeline);

        // Create the state object.
        ThrowIfFailed(device->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_dxrStateObject)), L"Couldn't create DirectX Raytracing state object.\n");
    }
}

// Build shader tables.
// This encapsulates all shader records - shaders and the arguments for their local root signatures.
void Pathtracer::BuildShaderTables(Scene& scene)
{
    auto device = m_deviceResources->GetD3DDevice();

    void* rayGenShaderIDs[RayGenShaderType::Count];
    void* missShaderIDs[PathtracerRayType::Count];
    void* hitGroupShaderIDs_TriangleGeometry[PathtracerRayType::Count];

    // A shader name look-up table for shader table debug print out.
    unordered_map<void*, wstring> shaderIdToStringMap;

    auto GetShaderIDs = [&](auto* stateObjectProperties)
    {
        for (UINT i = 0; i < RayGenShaderType::Count; i++)
        {
            rayGenShaderIDs[i] = stateObjectProperties->GetShaderIdentifier(c_rayGenShaderNames[i]);
            shaderIdToStringMap[rayGenShaderIDs[i]] = c_rayGenShaderNames[i];
        }

        for (UINT i = 0; i < PathtracerRayType::Count; i++)
        {
            missShaderIDs[i] = stateObjectProperties->GetShaderIdentifier(c_missShaderNames[i]);
            shaderIdToStringMap[missShaderIDs[i]] = c_missShaderNames[i];
        }

        for (UINT i = 0; i < PathtracerRayType::Count; i++)
        {
            hitGroupShaderIDs_TriangleGeometry[i] = stateObjectProperties->GetShaderIdentifier(c_hitGroupNames[i]);
            shaderIdToStringMap[hitGroupShaderIDs_TriangleGeometry[i]] = c_hitGroupNames[i];
        }
    };

    // Get shader identifiers.
    UINT shaderIDSize;
    ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
    ThrowIfFailed(m_dxrStateObject.As(&stateObjectProperties));
    GetShaderIDs(stateObjectProperties.Get());
    shaderIDSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    
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
        UINT numShaderRecords = PathtracerRayType::Count;
        UINT shaderRecordSize = shaderIDSize; // No root arguments

        ShaderTable missShaderTable(device, numShaderRecords, shaderRecordSize, L"MissShaderTable");
        for (UINT i = 0; i < PathtracerRayType::Count; i++)
        {
            missShaderTable.push_back(ShaderRecord(missShaderIDs[i], shaderIDSize, nullptr, 0));
        }
        missShaderTable.DebugPrint(shaderIdToStringMap);
        m_missShaderTableStrideInBytes = missShaderTable.GetShaderRecordSize();
        m_missShaderTable = missShaderTable.GetResource();
    }

    // Hit group shader table.
    {
        auto& bottomLevelASGeometries = scene.BottomLevelASGeometries();
        auto& accelerationStructure = *scene.AccelerationStructure();
        auto& grassPatchVB = scene.GrassPatchVB();

        UINT numShaderRecords = 0;
        for (auto& bottomLevelASGeometryPair : bottomLevelASGeometries)
        {
            auto& bottomLevelASGeometry = bottomLevelASGeometryPair.second;
            numShaderRecords += static_cast<UINT>(bottomLevelASGeometry.m_geometryInstances.size()) * PathtracerRayType::Count;
        }
        UINT numGrassGeometryShaderRecords = 2 * UIParameters::NumGrassGeometryLODs * 3 * PathtracerRayType::Count;
        numShaderRecords += numGrassGeometryShaderRecords;

        UINT shaderRecordSize = shaderIDSize + sizeof(LocalRootSignature::RootArguments);
        ShaderTable hitGroupShaderTable(device, numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");

        // Triangle geometry hit groups.
        for (auto& bottomLevelASGeometryPair : bottomLevelASGeometries)
        {
            auto& bottomLevelASGeometry = bottomLevelASGeometryPair.second;
            auto& name = bottomLevelASGeometry.GetName();

            UINT shaderRecordOffset = hitGroupShaderTable.GeNumShaderRecords();
            accelerationStructure.GetBottomLevelAS(bottomLevelASGeometryPair.first).SetInstanceContributionToHitGroupIndex(shaderRecordOffset);

            // Grass Patch LOD shader recods
            if (name.find(L"Grass Patch LOD") != wstring::npos)
            {
                UINT LOD = stoi(name.data() + wcsnlen_s(L"Grass Patch LOD", 15));

                ThrowIfFalse(bottomLevelASGeometry.m_geometryInstances.size() == 1, L"The implementation assumes a single geometry instance per BLAS for dynamic/grass geometry");
                auto& geometryInstance = bottomLevelASGeometry.m_geometryInstances[0];

                LocalRootSignature::RootArguments rootArgs;
                rootArgs.cb.materialID = geometryInstance.materialID;
                rootArgs.cb.isVertexAnimated = geometryInstance.isVertexAnimated;

                memcpy(&rootArgs.indexBufferGPUHandle, &geometryInstance.ib.gpuDescriptorHandle, sizeof(geometryInstance.ib.gpuDescriptorHandle));
                memcpy(&rootArgs.diffuseTextureGPUHandle, &geometryInstance.diffuseTexture, sizeof(geometryInstance.diffuseTexture));
                memcpy(&rootArgs.normalTextureGPUHandle, &geometryInstance.normalTexture, sizeof(geometryInstance.normalTexture));

                // Dynamic geometry with multiple LODs is handled by creating shader records
                // for all cases. Then, on geometry/instance updates, a BLAS instance updates
                // its InstanceContributionToHitGroupIndex to point to the corresponding 
                // shader records for that LOD. 
                // 
                // The LOD selection can change from a frame to frame depending on distance
                // to the camera. For simplicity, we assume the LOD index difference from frame to frame 
                // is no greater than 1. This can be false if camera moves fast, but in that case 
                // temporal reprojection would fail for the most part anyway yielding 
                // diminishing returns supporting that scenario.
                // Reprojection consistency checks will prevent blending in from non-similar geometry.
                //
                // Given multiple LODs and LOD delta being 1 at most, we create the records as follows:
                // 2 * 3 Shader Records per LOD
                //  2 - ping-pong frame to frame
                //  3 - transition types
                //      Transition from lower LOD in previous frame
                //      Same LOD as previous frame
                //      Transition from higher LOD in previous frame

                struct VertexBufferHandles {
                    D3D12_GPU_DESCRIPTOR_HANDLE prevFrameVertexBuffer;
                    D3D12_GPU_DESCRIPTOR_HANDLE vertexBuffer;
                };

                VertexBufferHandles vbHandles[2][3];
                for (UINT frameID = 0; frameID < 2; frameID++)
                {
                    UINT prevFrameID = (frameID + 1) % 2;
                   

                    // Transitioning from lower LOD.
                    vbHandles[frameID][0].vertexBuffer = grassPatchVB[LOD][frameID].gpuDescriptorReadAccess;
                    vbHandles[frameID][0].prevFrameVertexBuffer = LOD > 0 ? grassPatchVB[LOD - 1][prevFrameID].gpuDescriptorReadAccess
                        : grassPatchVB[LOD][prevFrameID].gpuDescriptorReadAccess;

                    // Same LOD as previous frame.
                    vbHandles[frameID][1].vertexBuffer = grassPatchVB[LOD][frameID].gpuDescriptorReadAccess;
                    vbHandles[frameID][1].prevFrameVertexBuffer = grassPatchVB[LOD][prevFrameID].gpuDescriptorReadAccess;

                    // Transitioning from higher LOD.
                    vbHandles[frameID][2].vertexBuffer = grassPatchVB[LOD][frameID].gpuDescriptorReadAccess;
                    vbHandles[frameID][2].prevFrameVertexBuffer = LOD < UIParameters::NumGrassGeometryLODs - 1 ? grassPatchVB[LOD + 1][prevFrameID].gpuDescriptorReadAccess
                        : grassPatchVB[LOD][prevFrameID].gpuDescriptorReadAccess;
                }

                for (UINT frameID = 0; frameID < 2; frameID++)
                    for (UINT transitionType = 0; transitionType < 3; transitionType++)
                    {
                        memcpy(&rootArgs.vertexBufferGPUHandle, &vbHandles[frameID][transitionType].vertexBuffer, sizeof(vbHandles[frameID][transitionType].vertexBuffer));
                        memcpy(&rootArgs.previousFrameVertexBufferGPUHandle, &vbHandles[frameID][transitionType].prevFrameVertexBuffer, sizeof(vbHandles[frameID][transitionType].prevFrameVertexBuffer));

                        for (auto& hitGroupShaderID : hitGroupShaderIDs_TriangleGeometry)
                        {
                            hitGroupShaderTable.push_back(ShaderRecord(hitGroupShaderID, shaderIDSize, &rootArgs, sizeof(rootArgs)));
                        }
                    }
            }
            else // Non-vertex buffer animated geometry with 1 shader record per ray-type per bottom-level AS
            {
                for (auto& geometryInstance : bottomLevelASGeometry.m_geometryInstances)
                {
                    LocalRootSignature::RootArguments rootArgs;
                    rootArgs.cb.materialID = geometryInstance.materialID;
                    rootArgs.cb.isVertexAnimated = geometryInstance.isVertexAnimated;

                    memcpy(&rootArgs.indexBufferGPUHandle, &geometryInstance.ib.gpuDescriptorHandle, sizeof(geometryInstance.ib.gpuDescriptorHandle));
                    memcpy(&rootArgs.vertexBufferGPUHandle, &geometryInstance.vb.gpuDescriptorHandle, sizeof(geometryInstance.ib.gpuDescriptorHandle));
                    memcpy(&rootArgs.previousFrameVertexBufferGPUHandle, &m_nullVertexBufferGPUhandle, sizeof(m_nullVertexBufferGPUhandle));
                    memcpy(&rootArgs.diffuseTextureGPUHandle, &geometryInstance.diffuseTexture, sizeof(geometryInstance.diffuseTexture));
                    memcpy(&rootArgs.normalTextureGPUHandle, &geometryInstance.normalTexture, sizeof(geometryInstance.normalTexture));


                    for (auto& hitGroupShaderID : hitGroupShaderIDs_TriangleGeometry)
                    {
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

void Pathtracer::DispatchRays(ID3D12Resource* rayGenShaderTable, UINT width, UINT height)
{
    auto commandList = m_deviceResources->GetCommandList();
    auto resourceStateTracker = m_deviceResources->GetGpuResourceStateTracker();
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    ScopedTimer _prof(L"DispatchRays", commandList);

    D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
    dispatchDesc.HitGroupTable.StartAddress = m_hitGroupShaderTable->GetGPUVirtualAddress();
    dispatchDesc.HitGroupTable.SizeInBytes = m_hitGroupShaderTable->GetDesc().Width;
    dispatchDesc.HitGroupTable.StrideInBytes = m_hitGroupShaderTableStrideInBytes;
    dispatchDesc.MissShaderTable.StartAddress = m_missShaderTable->GetGPUVirtualAddress();
    dispatchDesc.MissShaderTable.SizeInBytes = m_missShaderTable->GetDesc().Width;
    dispatchDesc.MissShaderTable.StrideInBytes = m_missShaderTableStrideInBytes;
    dispatchDesc.RayGenerationShaderRecord.StartAddress = rayGenShaderTable->GetGPUVirtualAddress();
    dispatchDesc.RayGenerationShaderRecord.SizeInBytes = rayGenShaderTable->GetDesc().Width;
    dispatchDesc.Width = width != 0 ? width : m_raytracingWidth;
    dispatchDesc.Height = height != 0 ? height : m_raytracingHeight;
    dispatchDesc.Depth = 1;
    commandList->SetPipelineState1(m_dxrStateObject.Get());

    resourceStateTracker->FlushResourceBarriers();
    commandList->DispatchRays(&dispatchDesc);
}

void Pathtracer::SetCamera(const GameCore::Camera& camera)
{
    XMMATRIX worldWithCameraEyeAtOrigin, proj;
    camera.GetProj(&proj, m_raytracingWidth, m_raytracingHeight);

    worldWithCameraEyeAtOrigin = XMMatrixLookAtLH(XMVectorSet(0, 0, 0, 1), XMVectorSetW(camera.At() - camera.Eye(), 1), camera.Up());
    XMMATRIX viewProj = worldWithCameraEyeAtOrigin * proj;
    m_CB->projectionToWorldWithCameraAtOrigin = XMMatrixInverse(nullptr, viewProj);
    XMStoreFloat3(&m_CB->cameraPosition, camera.Eye());
    m_CB->Znear = camera.ZMin;
    m_CB->Zfar = camera.ZMax;
}

void Pathtracer::UpdateConstantBuffer(Scene& scene)
{
    XMStoreFloat3(&m_CB->lightPosition, scene.m_lightPosition);
    m_CB->lightColor = scene.m_lightColor;

    SetCamera(scene.Camera());

    m_CB->maxRadianceRayRecursionDepth = Pathtracer_Args::MaxRadianceRayRecursionDepth;

    if (Composition_Args::CompositionMode == PBRShading)
        m_CB->maxShadowRayRecursionDepth = Pathtracer_Args::MaxShadowRayRecursionDepth;
    else
        // Casting shadow rays at multiple TraceRay recursion depths is expensive. Skip if it the result is not getting rendered at composition.
        m_CB->maxShadowRayRecursionDepth = 0;

    m_CB->useNormalMaps = Pathtracer_Args::RTAOUseNormalMaps;
    m_CB->defaultAmbientIntensity = Pathtracer_Args::DefaultAmbientIntensity;
    m_CB->useBaseAlbedoFromMaterial = Composition_Args::CompositionMode == CompositionType::BaseMaterialAlbedo;

    auto& prevFrameCamera = scene.PrevFrameCamera();
    XMMATRIX prevView, prevProj;
    prevFrameCamera.GetViewProj(&prevView, &prevProj, m_raytracingWidth, m_raytracingHeight);
    m_CB->prevFrameViewProj = prevView * prevProj;
    XMStoreFloat3(&m_CB->prevFrameCameraPosition, prevFrameCamera.Eye());

    XMMATRIX prevViewCameraAtOrigin = XMMatrixLookAtLH(XMVectorSet(0, 0, 0, 1), XMVectorSetW(prevFrameCamera.At() - prevFrameCamera.Eye(), 1), prevFrameCamera.Up());
    XMMATRIX viewProjCameraAtOrigin = prevViewCameraAtOrigin * prevProj;
    m_CB->prevFrameProjToViewCameraAtOrigin = XMMatrixInverse(nullptr, viewProjCameraAtOrigin);
}

void Pathtracer::Run(Scene& scene)
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();
    auto resourceStateTracker = m_deviceResources->GetGpuResourceStateTracker();
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    // TODO: this should be called before any rendering in a frame
    if (m_isRecreateRaytracingResourcesRequested)
    {
        m_isRecreateRaytracingResourcesRequested = false;
        m_deviceResources->WaitForGpu();

        CreateResolutionDependentResources();
        CreateAuxilaryDeviceResources();
    }

    ScopedTimer _prof(L"Pathtracing", commandList);
    UpdateConstantBuffer(scene);

    auto& MaterialBuffer = scene.MaterialBuffer();
    auto& EnvironmentMap = scene.EnvironmentMap();
    auto& PrevFrameBottomLevelASInstanceTransforms = scene.PrevFrameBottomLevelASInstanceTransforms();

    commandList->SetDescriptorHeaps(1, m_cbvSrvUavHeap->GetAddressOf());
    commandList->SetComputeRootSignature(m_raytracingGlobalRootSignature.Get());

    // Copy dynamic buffers to GPU.
    {
        m_CB.CopyStagingToGpu(frameIndex);
    }

    // Transition all output resources to UAV state.
    {
        resourceStateTracker->TransitionResource(&m_GBufferResources[GBufferResource::HitPosition], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&m_GBufferResources[GBufferResource::SurfaceNormalDepth], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&m_GBufferResources[GBufferResource::Depth], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&m_GBufferResources[GBufferResource::PartialDepthDerivatives], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&m_GBufferResources[GBufferResource::MotionVector], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&m_GBufferResources[GBufferResource::ReprojectedNormalDepth], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&m_GBufferResources[GBufferResource::Color], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&m_GBufferResources[GBufferResource::AOSurfaceAlbedo], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    }


    // Bind inputs.
    commandList->SetComputeRootShaderResourceView(GlobalRootSignature::Slot::AccelerationStructure, scene.AccelerationStructure()->GetTopLevelASResource()->GetGPUVirtualAddress());
    commandList->SetComputeRootConstantBufferView(GlobalRootSignature::Slot::ConstantBuffer, m_CB.GpuVirtualAddress(frameIndex));
    commandList->SetComputeRootShaderResourceView(GlobalRootSignature::Slot::MaterialBuffer, scene.MaterialBuffer().GpuVirtualAddress());
    commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::EnvironmentMap, EnvironmentMap.gpuDescriptorHandle);
    commandList->SetComputeRootShaderResourceView(GlobalRootSignature::Slot::PrevFrameBottomLevelASIstanceTransforms, PrevFrameBottomLevelASInstanceTransforms.GpuVirtualAddress(frameIndex));


    // Bind output RTs.
    commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::GBufferResources, m_GBufferResources[0].gpuDescriptorWriteAccess);
    commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::MotionVector, m_GBufferResources[GBufferResource::MotionVector].gpuDescriptorWriteAccess);
    commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::ReprojectedNormalDepth, m_GBufferResources[GBufferResource::ReprojectedNormalDepth].gpuDescriptorWriteAccess);
    commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::Color, m_GBufferResources[GBufferResource::Color].gpuDescriptorWriteAccess);
    commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::AOSurfaceAlbedo, m_GBufferResources[GBufferResource::AOSurfaceAlbedo].gpuDescriptorWriteAccess);

    GpuResource* debugResources = Sample::g_debugOutput;
    commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::Debug1, debugResources[0].gpuDescriptorWriteAccess);
    commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::Debug2, debugResources[1].gpuDescriptorWriteAccess); 
	
    // Dispatch Rays.
    DispatchRays(m_rayGenShaderTables[RayGenShaderType::Pathtracer].Get());

    // Transition GBuffer resources to shader resource state.
    {
        resourceStateTracker->TransitionResource(&m_GBufferResources[GBufferResource::HitPosition], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->TransitionResource(&m_GBufferResources[GBufferResource::SurfaceNormalDepth], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->TransitionResource(&m_GBufferResources[GBufferResource::Depth], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->TransitionResource(&m_GBufferResources[GBufferResource::MotionVector], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->TransitionResource(&m_GBufferResources[GBufferResource::ReprojectedNormalDepth], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->TransitionResource(&m_GBufferResources[GBufferResource::Color], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->TransitionResource(&m_GBufferResources[GBufferResource::AOSurfaceAlbedo], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }

    // Calculate partial derivatives.
    {
        ScopedTimer _prof(L"Calculate Partial Depth Derivatives", commandList);
        resourceStateTracker->FlushResourceBarriers();
        m_calculatePartialDerivativesKernel.Run(
            commandList,
            m_cbvSrvUavHeap->GetHeap(),
            m_raytracingWidth,
            m_raytracingHeight,
            m_GBufferResources[GBufferResource::Depth].gpuDescriptorReadAccess,
            m_GBufferResources[GBufferResource::PartialDepthDerivatives].gpuDescriptorWriteAccess);

        resourceStateTracker->TransitionResource(&m_GBufferResources[GBufferResource::PartialDepthDerivatives], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }
    if (RTAO_Args::QuarterResAO)
    {
        DownsampleGBuffer();
    }
}

void Pathtracer::CreateResolutionDependentResources()
{
    CreateTextureResources();
}

void Pathtracer::SetResolution(UINT GBufferWidth, UINT GBufferHeight, UINT RTAOWidth, UINT RTAOHeight)
{
    m_raytracingWidth = GBufferWidth;
    m_raytracingHeight = GBufferHeight;
    m_quarterResWidth = RTAOWidth;
    m_quarterResHeight = RTAOHeight;

    CreateResolutionDependentResources();
}

void Pathtracer::CreateTextureResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto backbufferFormat = m_deviceResources->GetBackBufferFormat();

    DXGI_FORMAT hitPositionFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
    DXGI_FORMAT debugFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
    D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    
    // Full-res GBuffer resources.
    {
        // Preallocate subsequent descriptor indices for both SRV and UAV groups.
        m_GBufferResources[0].uavDescriptorHeapIndex = m_cbvSrvUavHeap->AllocateDescriptorIndices(GBufferResource::Count);
        m_GBufferResources[0].srvDescriptorHeapIndex = m_cbvSrvUavHeap->AllocateDescriptorIndices(GBufferResource::Count);
        for (UINT i = 0; i < GBufferResource::Count; i++)
        {
            m_GBufferResources[i].uavDescriptorHeapIndex = m_GBufferResources[0].uavDescriptorHeapIndex + i;
            m_GBufferResources[i].srvDescriptorHeapIndex = m_GBufferResources[0].srvDescriptorHeapIndex + i;
        }
        CreateRenderTargetResource(device, hitPositionFormat, m_raytracingWidth, m_raytracingHeight, m_cbvSrvUavHeap.get(), &m_GBufferResources[GBufferResource::HitPosition], initialResourceState, L"GBuffer HitPosition");
        CreateRenderTargetResource(device, COMPACT_NORMAL_DEPTH_DXGI_FORMAT, m_raytracingWidth, m_raytracingHeight, m_cbvSrvUavHeap.get(), &m_GBufferResources[GBufferResource::SurfaceNormalDepth], initialResourceState, L"GBuffer Normal Depth");
        CreateRenderTargetResource(device, DXGI_FORMAT_R16_FLOAT, m_raytracingWidth, m_raytracingHeight, m_cbvSrvUavHeap.get(), &m_GBufferResources[GBufferResource::Depth], initialResourceState, L"GBuffer Distance");
        CreateRenderTargetResource(device, TextureResourceFormatRG::ToDXGIFormat(Pathtracer_Args::RTAO_PartialDepthDerivativesResourceFormat), m_raytracingWidth, m_raytracingHeight, m_cbvSrvUavHeap.get(), &m_GBufferResources[GBufferResource::PartialDepthDerivatives], initialResourceState, L"GBuffer Partial Depth Derivatives");
        CreateRenderTargetResource(device, TextureResourceFormatRG::ToDXGIFormat(Pathtracer_Args::RTAO_MotionVectorResourceFormat), m_raytracingWidth, m_raytracingHeight, m_cbvSrvUavHeap.get(), &m_GBufferResources[GBufferResource::MotionVector], initialResourceState, L"GBuffer Texture Space Motion Vector");
        CreateRenderTargetResource(device, COMPACT_NORMAL_DEPTH_DXGI_FORMAT, m_raytracingWidth, m_raytracingHeight, m_cbvSrvUavHeap.get(), &m_GBufferResources[GBufferResource::ReprojectedNormalDepth], initialResourceState, L"GBuffer Reprojected Hit Position");
        CreateRenderTargetResource(device, backbufferFormat, m_raytracingWidth, m_raytracingHeight, m_cbvSrvUavHeap.get(), &m_GBufferResources[GBufferResource::Color], initialResourceState, L"GBuffer Color");
        CreateRenderTargetResource(device, DXGI_FORMAT_R11G11B10_FLOAT, m_raytracingWidth, m_raytracingHeight, m_cbvSrvUavHeap.get(), &m_GBufferResources[GBufferResource::AOSurfaceAlbedo], initialResourceState, L"GBuffer AO Surface Albedo");
    }

    // Low-res GBuffer resources.
    {
        // Preallocate subsequent descriptor indices for both SRV and UAV groups.
        m_GBufferQuarterResResources[0].uavDescriptorHeapIndex = m_cbvSrvUavHeap->AllocateDescriptorIndices(GBufferResource::Count);
        m_GBufferQuarterResResources[0].srvDescriptorHeapIndex = m_cbvSrvUavHeap->AllocateDescriptorIndices(GBufferResource::Count);
        for (UINT i = 0; i < GBufferResource::Count; i++)
        {
            m_GBufferQuarterResResources[i].uavDescriptorHeapIndex = m_GBufferQuarterResResources[0].uavDescriptorHeapIndex + i;
            m_GBufferQuarterResResources[i].srvDescriptorHeapIndex = m_GBufferQuarterResResources[0].srvDescriptorHeapIndex + i;
        }
        CreateRenderTargetResource(device, hitPositionFormat, m_quarterResWidth, m_quarterResHeight, m_cbvSrvUavHeap.get(), &m_GBufferQuarterResResources[GBufferResource::HitPosition], initialResourceState, L"GBuffer LowRes HitPosition");
        CreateRenderTargetResource(device, COMPACT_NORMAL_DEPTH_DXGI_FORMAT, m_quarterResWidth, m_quarterResHeight, m_cbvSrvUavHeap.get(), &m_GBufferQuarterResResources[GBufferResource::SurfaceNormalDepth], initialResourceState, L"GBuffer LowRes Normal");
        CreateRenderTargetResource(device, DXGI_FORMAT_R16_FLOAT, m_quarterResWidth, m_quarterResHeight, m_cbvSrvUavHeap.get(), &m_GBufferQuarterResResources[GBufferResource::Depth], initialResourceState, L"GBuffer LowRes Distance");
        CreateRenderTargetResource(device, TextureResourceFormatRG::ToDXGIFormat(Pathtracer_Args::RTAO_PartialDepthDerivativesResourceFormat), m_quarterResWidth, m_quarterResHeight, m_cbvSrvUavHeap.get(), &m_GBufferQuarterResResources[GBufferResource::PartialDepthDerivatives], initialResourceState, L"GBuffer LowRes Partial Depth Derivatives");
        CreateRenderTargetResource(device, TextureResourceFormatRG::ToDXGIFormat(Pathtracer_Args::RTAO_MotionVectorResourceFormat), m_quarterResWidth, m_quarterResHeight, m_cbvSrvUavHeap.get(), &m_GBufferQuarterResResources[GBufferResource::MotionVector], initialResourceState, L"GBuffer LowRes Texture Space Motion Vector");

        CreateRenderTargetResource(device, COMPACT_NORMAL_DEPTH_DXGI_FORMAT, m_quarterResWidth, m_quarterResHeight, m_cbvSrvUavHeap.get(), &m_GBufferQuarterResResources[GBufferResource::ReprojectedNormalDepth], initialResourceState, L"GBuffer LowRes Reprojected Normal Depth");
        CreateRenderTargetResource(device, DXGI_FORMAT_R11G11B10_FLOAT, m_quarterResWidth, m_quarterResHeight, m_cbvSrvUavHeap.get(), &m_GBufferQuarterResResources[GBufferResource::AOSurfaceAlbedo], initialResourceState, L"GBuffer LowRes AO Surface Albedo");
    }
}

void Pathtracer::DownsampleGBuffer()
{
    auto commandList = m_deviceResources->GetCommandList();
    auto resourceStateTracker = m_deviceResources->GetGpuResourceStateTracker();
    ScopedTimer _prof(L"DownsampleGBuffer", commandList);

    // Transition all output resources to UAV state.
    {
        resourceStateTracker->TransitionResource(&m_GBufferQuarterResResources[GBufferResource::HitPosition], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&m_GBufferQuarterResResources[GBufferResource::PartialDepthDerivatives], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&m_GBufferQuarterResResources[GBufferResource::MotionVector], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&m_GBufferQuarterResResources[GBufferResource::ReprojectedNormalDepth], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&m_GBufferQuarterResResources[GBufferResource::Depth], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&m_GBufferQuarterResResources[GBufferResource::SurfaceNormalDepth], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceStateTracker->TransitionResource(&m_GBufferQuarterResResources[GBufferResource::AOSurfaceAlbedo], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    }

    resourceStateTracker->FlushResourceBarriers();
    m_downsampleGBufferBilateralFilterKernel.Run(
        commandList,
        m_raytracingWidth,
        m_raytracingHeight,
        m_cbvSrvUavHeap->GetHeap(),
        m_GBufferResources[GBufferResource::SurfaceNormalDepth].gpuDescriptorReadAccess,
        m_GBufferResources[GBufferResource::HitPosition].gpuDescriptorReadAccess,
        m_GBufferResources[GBufferResource::PartialDepthDerivatives].gpuDescriptorReadAccess,
        m_GBufferResources[GBufferResource::MotionVector].gpuDescriptorReadAccess,
        m_GBufferResources[GBufferResource::ReprojectedNormalDepth].gpuDescriptorReadAccess,
        m_GBufferResources[GBufferResource::Depth].gpuDescriptorReadAccess,
        m_GBufferResources[GBufferResource::AOSurfaceAlbedo].gpuDescriptorReadAccess,
        m_GBufferQuarterResResources[GBufferResource::SurfaceNormalDepth].gpuDescriptorWriteAccess,
        m_GBufferQuarterResResources[GBufferResource::HitPosition].gpuDescriptorWriteAccess,
        m_GBufferQuarterResResources[GBufferResource::PartialDepthDerivatives].gpuDescriptorWriteAccess,
        m_GBufferQuarterResResources[GBufferResource::MotionVector].gpuDescriptorWriteAccess,
        m_GBufferQuarterResResources[GBufferResource::ReprojectedNormalDepth].gpuDescriptorWriteAccess,
        m_GBufferQuarterResResources[GBufferResource::Depth].gpuDescriptorWriteAccess,
        m_GBufferQuarterResResources[GBufferResource::AOSurfaceAlbedo].gpuDescriptorWriteAccess);

    // Transition GBuffer resources to shader resource state.
    {
        resourceStateTracker->TransitionResource(&m_GBufferQuarterResResources[GBufferResource::HitPosition], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->TransitionResource(&m_GBufferQuarterResResources[GBufferResource::SurfaceNormalDepth], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->TransitionResource(&m_GBufferQuarterResResources[GBufferResource::PartialDepthDerivatives], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->TransitionResource(&m_GBufferQuarterResResources[GBufferResource::MotionVector], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->TransitionResource(&m_GBufferQuarterResResources[GBufferResource::ReprojectedNormalDepth], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->TransitionResource(&m_GBufferQuarterResResources[GBufferResource::Depth], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        resourceStateTracker->TransitionResource(&m_GBufferQuarterResResources[GBufferResource::AOSurfaceAlbedo], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }
}