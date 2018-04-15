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

#pragma once

#include "DXSample.h"
#include "StepTimer.h"
#include "RaytracingHlslCompat.h"

namespace GlobalRootSignatureParams {
    enum Value {
        OutputViewSlot = 0,
        AccelerationStructureSlot,
        SceneConstantSlot,
        AABBattributeBufferSlot,
        VertexBuffersSlot,
        Count
    };
}

// Gate use of Empty on NV as a current workaround

namespace LocalRootSignatures {
    enum Value {
        Triangle = 0,
        AABB,
#if USE_NON_NULL_LOCAL_ROOT_SIG
        Empty,
#endif
        Count
    };
}

namespace LocalRootSignatureParamsTriangle {
    enum Value {
        MaterialConstantSlot = 0,
        Count
    };
}

namespace LocalRootSignature {
    namespace Triangle {
        enum Enum {
            MaterialConstantSlot = 0,
            Count
        };

        struct RootArguments {
            MaterialConstantBuffer materialCb;
        };
    }

    namespace AABB {
        enum Enum {
            MaterialConstantSlot = 0,
            // ToDo rename GeometryIndexSlot
            GeometryIndexSlot,
            Count
        };
        struct RootArguments {
            MaterialConstantBuffer materialCb;
#if USE_LOCAL_ROOT_CONSTANTS
            AABBConstantBuffer aabbCB;
#else
            D3D12_GPU_DESCRIPTOR_HANDLE aabbCBdescriptorHandle;
#endif
        };
    }
    inline UINT MaxRootArgumentsSize()
    {
        return max(sizeof(Triangle::RootArguments), sizeof(AABB::RootArguments));
    }
}

namespace LocalRootSignatureParamsAABB {
    enum Value {
        MaterialConstantSlot = 0,
        GeometryIndexSlot,
        Count
    };
}

// Bottom level acceleration structures (BottomLevelASType).
// This sample uses two BottomLevelASType, one for AABB and one for Triangle geometry.
// ToDo desc why the sample uses two - can a BLAS mix geometry types?
namespace BottomLevelASType {
    enum Value {
        Triangle = 0,
        AABB,
        Count
    };
}

namespace TriangleHitGroupType {
    enum Value {
        Triangle = 0,
        Count
    };
}

namespace AABBHitGroupType {
    enum Value {
        AABB = 0,
        ShadowAABB,
        Count
    };
}

namespace RayType {
    enum Value {
        Regular = 0,
        Shadow,
        Count
    };
}

namespace ClosestHitRayType {
    enum Value {
        Triangle = 0,
        AABB,
        ShadowAABB,
        Count
    };
}

// ToDo rename to IntersectionGeometryType ?
namespace IntersectionShaderType {
    enum Value {
        AABB = 0,
        Sphere,
        Spheres,
        Count
    };
}


struct AccelerationStructureBuffers
{
    ComPtr<ID3D12Resource> scratch;
    ComPtr<ID3D12Resource> accelerationStructure;
    ComPtr<ID3D12Resource> instanceDesc;    // Used only for top-level AS
    UINT64                 ResultDataMaxSizeInBytes;
};

// The sample supports both Raytracing Fallback Layer and DirectX Raytracing APIs. 
// This is purely for demonstration purposes to show where the API differences are. 
// Real-world applications will implement only one or the other. 
// Fallback Layer uses DirectX Raytracing if a driver and OS supports it. 
// Otherwise, it falls back to compute pipeline to emulate raytracing.
// Developers aiming for a wider HW support should target Fallback Layer.
class D3D12RaytracingProceduralGeometry : public DXSample
{
    enum class RaytracingAPI {
        FallbackLayer,
        DirectXRaytracing,
    };

public:
    D3D12RaytracingProceduralGeometry(UINT width, UINT height, std::wstring name);

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    virtual void OnInit();
    virtual void OnKeyDown(UINT8 key);
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnSizeChanged(UINT width, UINT height, bool minimized);
    virtual void OnDestroy();
    virtual IDXGISwapChain* GetSwapchain() { return m_deviceResources->GetSwapChain(); }

private:
    static const UINT FrameCount = 3;

    // Number of AABB BLAS instances
    static const UINT NUM_INSTANCE_X = 2;
    static const UINT NUM_INSTANCE_Y = 1;
    static const UINT NUM_INSTANCE_Z = 2;
    static const UINT NUM_INSTANCES = NUM_INSTANCE_X * NUM_INSTANCE_Y * NUM_INSTANCE_Z;

    // Number of AABBs in a BLAS
    static const UINT NUM_AABB_X = 2;
    static const UINT NUM_AABB_Y = 1;
    static const UINT NUM_AABB_Z = 2;
    static const UINT NUM_AABB = NUM_AABB_X * NUM_AABB_Y * NUM_AABB_Z;

    static const UINT NUM_BLAS = 1 + NUM_INSTANCES; // Triangle BLAS + AABB BLAS instances

    const float c_aabbWidth = 2;      // AABB width
    const float c_aabbDistance = 2;   // Distance between AABBs

    // Raytracing Fallback Layer (FL) attributes
    ComPtr<ID3D12RaytracingFallbackDevice> m_fallbackDevice;
    ComPtr<ID3D12RaytracingFallbackCommandList> m_fallbackCommandList;
    ComPtr<ID3D12RaytracingFallbackStateObject> m_fallbackStateObject;
    WRAPPED_GPU_POINTER m_fallbackTopLevelAccelerationStructurePointer;

    // DirectX Raytracing (DXR) attributes
    ComPtr<ID3D12DeviceRaytracingPrototype> m_dxrDevice;
    ComPtr<ID3D12CommandListRaytracingPrototype> m_dxrCommandList;
    ComPtr<ID3D12StateObjectPrototype> m_dxrStateObject;
    bool m_isDxrSupported;

    // Root signatures
    ComPtr<ID3D12RootSignature> m_raytracingGlobalRootSignature;
    ComPtr<ID3D12RootSignature> m_raytracingLocalRootSignature[LocalRootSignatures::Count];

    // Descriptors
    ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
    UINT m_descriptorsAllocated;
    UINT m_descriptorSize;

    // ToDo use root constants instead of CBVs

    // Raytracing scene
    ConstantBuffer<SceneConstantBuffer> m_sceneCB;
    StructuredBuffer<AABBPrimitiveAttributes> m_aabbPrimitiveAttributeBuffer;
    
    // Root constants
    MaterialConstantBuffer m_planeMaterialCB;
    MaterialConstantBuffer m_aabbMaterialCB[IntersectionShaderType::Count];

    // Geometry
    D3DBuffer m_indexBuffer;
    D3DBuffer m_vertexBuffer;
    D3DBuffer m_aabbBuffer;
#if !USE_LOCAL_ROOT_CONSTANTS
    D3DBuffer m_geometryIndexBuffer[IntersectionShaderType::Count];
#endif

    // Acceleration structure
    ComPtr<ID3D12Resource> m_bottomLevelAS[BottomLevelASType::Count];
    ComPtr<ID3D12Resource> m_topLevelAS;

    // Raytracing output
    ComPtr<ID3D12Resource> m_raytracingOutput;
    D3D12_GPU_DESCRIPTOR_HANDLE m_raytracingOutputResourceUAVGpuDescriptor;
    UINT m_raytracingOutputResourceUAVDescriptorHeapIndex;

    // Shader tables
    static const wchar_t* c_hitGroupNames_TriangleGeometry[TriangleHitGroupType::Count];
    static const wchar_t* c_hitGroupNames_AABBGeometry[IntersectionShaderType::Count][AABBHitGroupType::Count];
    static const wchar_t* c_raygenShaderName;
    static const wchar_t* c_intersectionShaderNames[IntersectionShaderType::Count];
    static const wchar_t* c_closestHitShaderNames[ClosestHitRayType::Count];
    static const wchar_t* c_missShaderNames[RayType::Count];
    ComPtr<ID3D12Resource> m_missShaderTable;
    UINT m_missShaderTableStrideInBytes;
    ComPtr<ID3D12Resource> m_hitGroupShaderTable;
    UINT m_hitGroupShaderTableStrideInBytes;
    ComPtr<ID3D12Resource> m_rayGenShaderTable;

    // Application state
    RaytracingAPI m_raytracingAPI;
    bool m_forceComputeFallback;
    StepTimer m_timer;
    float m_curRotationAngleRad;
    XMVECTOR m_eye;
    XMVECTOR m_at;
    XMVECTOR m_up;

    void ParseCommandLineArgs(WCHAR* argv[], int argc);
    void UpdateCameraMatrices();
    void UpdateAABBPrimitiveAttributes();
    void InitializeScene();
    void RecreateD3D();
    void DoRaytracing();
    void CreateConstantBuffers();
    void CreateAABBPrimitiveAttributesBuffers();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void ReleaseDeviceDependentResources();
    void ReleaseWindowSizeDependentResources();
    void CreateRaytracingInterfaces();
    void SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig);
    void CreateRootSignatures();
    void CreateRaytracingPipelineStateObject();
    void CreateDescriptorHeap();
    void CreateRaytracingOutputResource();
    void BuildProceduralGeometryAABBs();
    void BuildGeometry();
    void BuildPlaneGeometry();
    void BuildGeometryDescsForBottomLevelAS(std::array<std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>, BottomLevelASType::Count>& geometryDescs);
    template <class InstanceDescType, class BLASPtrType>
    void BuildBotomLevelASInstanceDescs(BLASPtrType *bottomLevelASaddresses, ComPtr<ID3D12Resource>* instanceDescsResource);
    AccelerationStructureBuffers BuildBottomLevelAS(const std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometryDesc, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE);
    AccelerationStructureBuffers BuildTopLevelAS(AccelerationStructureBuffers bottomLevelAS[BottomLevelASType::Count], D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE);
    void BuildAccelerationStructures();
    void BuildShaderTables();
    void SelectRaytracingAPI(RaytracingAPI type);
    void UpdateForSizeChange(UINT clientWidth, UINT clientHeight);
    void CopyRaytracingOutputToBackbuffer();
    void CalculateFrameStats();
    UINT AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse = UINT_MAX);
    UINT CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize);
    WRAPPED_GPU_POINTER CreateFallbackWrappedPointer(ID3D12Resource* resource, UINT bufferNumElements);
};
