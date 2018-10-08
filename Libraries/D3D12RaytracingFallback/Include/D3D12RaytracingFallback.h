///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// D3D12RaytracingFallback.h                                                 //
//                                                                           //
// Provides a simplified interface for the DX12 Ray Tracing interface that   //
// will use native DX12 ray tracing when available. For drivers that do not  //
// support ray tracing, a fallback compute-shader based solution will be     //
// used instead.                                                             //    
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include "d3d12_1.h"

struct EMULATED_GPU_POINTER
{
    UINT32 OffsetInBytes;
    UINT32 DescriptorHeapIndex;
};

struct WRAPPED_GPU_POINTER
{
    union
    {
        EMULATED_GPU_POINTER EmulatedGpuPtr;
        D3D12_GPU_VIRTUAL_ADDRESS GpuVA;
    };

    WRAPPED_GPU_POINTER operator+(UINT64 offset)
    {
        WRAPPED_GPU_POINTER pointer = *this;
        pointer.GpuVA += offset;
        return pointer;
    }
};

typedef struct D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC
{
    FLOAT Transform[3][4];
    UINT InstanceID : 24;
    UINT InstanceMask : 8;
    UINT InstanceContributionToHitGroupIndex : 24;
    UINT Flags : 8;
    WRAPPED_GPU_POINTER AccelerationStructure;
}     D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC;

class
_declspec(uuid("539e5c40-df25-4c7d-81d8-6537f54306ed"))
ID3D12RaytracingFallbackStateObject : public IUnknown
{
public:
    virtual ~ID3D12RaytracingFallbackStateObject() {};

    virtual void *STDMETHODCALLTYPE GetShaderIdentifier(
        _In_  LPCWSTR pExportName) = 0;

    virtual UINT64 STDMETHODCALLTYPE GetShaderStackSize(
        _In_  LPCWSTR pExportName) = 0;

    virtual UINT64 STDMETHODCALLTYPE GetPipelineStackSize(void) = 0;

    virtual void STDMETHODCALLTYPE SetPipelineStackSize(
        UINT64 PipelineStackSizeInBytes) = 0;

    virtual ID3D12StateObject *GetStateObject() = 0;
};

class
_declspec(uuid("348a2a6b-6760-4b78-a9a7-1758b6f78d46"))
ID3D12RaytracingFallbackCommandList : public IUnknown
{
public:
    virtual ~ID3D12RaytracingFallbackCommandList() {}

    virtual void BuildRaytracingAccelerationStructure(
        _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
        _In_  UINT NumPostbuildInfoDescs,
        _In_reads_opt_(NumPostbuildInfoDescs)  const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC *pPostbuildInfoDescs) = 0;

    virtual void STDMETHODCALLTYPE EmitRaytracingAccelerationStructurePostbuildInfo(
        _In_  const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC *pDesc,
        _In_  UINT NumSourceAccelerationStructures,
        _In_reads_(NumSourceAccelerationStructures)  const D3D12_GPU_VIRTUAL_ADDRESS *pSourceAccelerationStructureData) = 0;

    virtual void STDMETHODCALLTYPE CopyRaytracingAccelerationStructure(
        _In_  D3D12_GPU_VIRTUAL_ADDRESS DestAccelerationStructureData,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData,
        _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE Mode) = 0;

    virtual void STDMETHODCALLTYPE SetDescriptorHeaps(
        _In_  UINT NumDescriptorHeaps,
        _In_reads_(NumDescriptorHeaps)  ID3D12DescriptorHeap *const *ppDescriptorHeaps) = 0;

    virtual void STDMETHODCALLTYPE SetTopLevelAccelerationStructure(
        _In_  UINT RootParameterIndex,
        _In_  WRAPPED_GPU_POINTER  BufferLocation) = 0;

    virtual void STDMETHODCALLTYPE SetPipelineState1(
        _In_  ID3D12RaytracingFallbackStateObject *pStateObject) = 0;

    virtual void STDMETHODCALLTYPE DispatchRays(
        _In_  const D3D12_DISPATCH_RAYS_DESC *pDesc) = 0;
};

class
_declspec(uuid("0a662ea0-ab43-423a-848f-4824ae4b25ba"))
ID3D12RaytracingFallbackDevice : public IUnknown
{
public:
    virtual ~ID3D12RaytracingFallbackDevice() {};

    virtual bool UsingRaytracingDriver() = 0;

    // Automatically determine how to create WRAPPED_GPU_POINTER based on UsingRaytracingDriver()
    virtual WRAPPED_GPU_POINTER GetWrappedPointerSimple(UINT32 DescriptorHeapIndex, D3D12_GPU_VIRTUAL_ADDRESS GpuVA) = 0;

    // Pre-condition: UsingRaytracingDriver() must be false
    virtual WRAPPED_GPU_POINTER GetWrappedPointerFromDescriptorHeapIndex(UINT32 DescriptorHeapIndex, UINT32 OffsetInBytes = 0) = 0;

    // Pre-condition: UsingRaytracingDriver() must be true
    virtual WRAPPED_GPU_POINTER GetWrappedPointerFromGpuVA(D3D12_GPU_VIRTUAL_ADDRESS gpuVA) = 0;

    virtual D3D12_RESOURCE_STATES GetAccelerationStructureResourceState() = 0;
    
    virtual UINT STDMETHODCALLTYPE GetShaderIdentifierSize(void) = 0;

    virtual HRESULT STDMETHODCALLTYPE CreateStateObject(
        const D3D12_STATE_OBJECT_DESC *pDesc,
        REFIID riid,
        _COM_Outptr_  void **ppStateObject) = 0;

    virtual void STDMETHODCALLTYPE GetRaytracingAccelerationStructurePrebuildInfo(
        _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS *pDesc,
        _Out_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO *pInfo) = 0;

    virtual void QueryRaytracingCommandList(
        ID3D12GraphicsCommandList *pCommandList, 
        REFIID riid,
        _COM_Outptr_  void **ppRaytracingCommandList) = 0;

    virtual HRESULT STDMETHODCALLTYPE CreateRootSignature(
        _In_  UINT nodeMask,
        _In_reads_(blobLengthInBytes)  const void *pBlobWithRootSignature,
        _In_  SIZE_T blobLengthInBytes,
        REFIID riid,
        _COM_Outptr_  void **ppvRootSignature) = 0;

    virtual HRESULT D3D12SerializeVersionedRootSignature(
        _In_ const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignature,
        _Out_ ID3DBlob** ppBlob,
        _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob) = 0;

    virtual HRESULT WINAPI D3D12SerializeRootSignature(
        _In_ const D3D12_ROOT_SIGNATURE_DESC* pRootSignature,
        _In_ D3D_ROOT_SIGNATURE_VERSION Version,
        _Out_ ID3DBlob** ppBlob,
        _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob) = 0;
};

enum CreateRaytracingFallbackDeviceFlags
{
    None = 0x0,
    ForceComputeFallback = 0x1,
    EnableRootDescriptorsInShaderRecords = 0x2
};

HRESULT D3D12CreateRaytracingFallbackDevice(
    _In_ ID3D12Device *pDevice, 
    _In_ DWORD createRaytracingFallbackDeviceFlags,
    _In_ UINT NodeMask,
    _In_ REFIID riid,
    _COM_Outptr_opt_ void** ppDevice);