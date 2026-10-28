//*********************************************************
//
// Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once
#include <windows.h>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <SimpleMath.h>
#include "RaytracingHlslCompat.h"
#include "DeviceResources.h"
#include "SampleUI.h"
#include "ShaderCompiler.h"
#include <array>
#include <chrono>
#include <filesystem>
#include <exception>
#include <fstream>
#include <string>
#include <vector>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;
using Microsoft::WRL::ComPtr;

namespace RayRoot
{
enum Slot : UINT
{
    Scene = 0,
    Images,
    Frame,
    InstancePartitions,
    PartitionActivity,
    PhysicsState,
    TriangleData,
    PickResult,
    Count
};
} // namespace RayRoot

namespace AnimationRoot
{
enum Slot : UINT
{
    OriginalRegular = 0,
    OriginalPartitioned,
    PhysicsPrevious,
    PartitionActivity,
    CurrentRegular,
    DynamicWrites,
    Operation,
    InstancePartitions,
    PhysicsCurrent,
    PartitionActivityOutput,
    Constants,
    SeedVelocities,
    Count
};
} // namespace AnimationRoot

namespace CompositeRoot
{
enum Slot : UINT
{
    Images = 0,
    Pass,
    Count
};
} // namespace CompositeRoot

// Per-frame resources are reused only after the GPU has finished that frame.
struct FrameContext
{
    ComPtr<ID3D12CommandAllocator> allocator;
    ComPtr<ID3D12Resource> constants;
    ComPtr<ID3D12Resource> timings;
    ComPtr<ID3D12Resource> operationCount;
    ComPtr<ID3D12Resource> pickedInstance;
    UINT64 fenceValue{};
    UINT64 number{};
    UINT mode{};
    D3D12_RTAS_PARTITIONED_TLAS_FLAGS hint{D3D12_RTAS_PARTITIONED_TLAS_FLAG_NONE};
    bool pending{};
    bool ptlas{};
    bool pickPending{};
};

// Keep a working pipeline set alive until all shader reload steps succeed.
struct PipelineResources
{
    ComPtr<ID3D12RootSignature> raytracingRoot;
    ComPtr<ID3D12RootSignature> compositingRoot;
    ComPtr<ID3D12PipelineState> compositing;
    ComPtr<ID3D12RootSignature> animationRoot;
    ComPtr<ID3D12PipelineState> animation;
    ComPtr<ID3D12PipelineState> physics;
    ComPtr<ID3D12PipelineState> markPartitions;
    ComPtr<ID3D12StateObject> raytracing;
    ComPtr<ID3D12Resource> shaderTable;
};

class D3D12RaytracingPartitionedTLAS : public IDeviceNotify
{
public:
    void ParseCommandLineArgs();
    void Run(HINSTANCE instance, int showCommand);
    ~D3D12RaytracingPartitionedTLAS();
    void OnInit();
    void OnUpdate();
    void OnRender();
    void OnSizeChanged(UINT width, UINT height, bool minimized);
    void OnDestroy();
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

private:
    // Own the window-independent D3D state; the sample owns its PTLAS resources.
    DeviceResources m_deviceResources;
    std::exception_ptr m_windowException;
    ComPtr<ID3D12Device5> m_device;
    ComPtr<ID3D12GraphicsCommandList4> m_commandList;
    ComPtr<ID3D12DeviceRaytracing2> m_raytracingDevice;
    ComPtr<ID3D12CommandListRaytracing2> m_raytracingCommandList;
    void CreateDeviceDependentResources();
    void ReloadShadersIfRequested();
    void CaptureStateDump();
    void ReadbackPtlasWriteCount(FrameContext& frame);
    void DoRaytracing(FrameContext& frame, UINT queryBase);
    void Composite();
    void CopyRaytracingOutputToBackbuffer();
    void RenderUI();
    void PresentFrame(FrameContext& frame);
    static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
    ComPtr<ID3D12Resource> CreateBuffer(UINT64 bytes, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_FLAGS flags,
                                        D3D12_RESOURCE_STATES state);
    template <typename T> ComPtr<ID3D12Resource> Upload(const std::vector<T>& values);
    void CreateWindowSizeDependentResources();
    void BuildGeometryAndBLAS();
    void CreateSceneAndTopLevels();
    void UploadSeedVelocities(const std::vector<D3D12_RAYTRACING_INSTANCE_DESC>& instances);
    void CreateRegularTLAS(const std::vector<D3D12_RAYTRACING_INSTANCE_DESC>& tlasInstances);
    void CreatePartitionedTLAS(
        const std::vector<D3D12_RTAS_PARTITIONED_TLAS_OPERATION_WRITE_INSTANCE_ARGS>& ptlasInstances);
    void CreateAnimationPipelines(const ShaderBytecodes& shaders);
    void CreateAnimationResources();
    void CreateRaytracingPipelineStateObject(const ShaderBytecodes& shaders);
    void CreateCompositingPipeline(const ShaderBytecodes& shaders);
    ShaderBytecodes m_shaderBytecodes; // Last validated shaders, retained across device recovery.
    void CreateFrameConstants();
    void UpdateCamera();
    void UpdateInput();
    void RenderFrame();
    void RecoverDevice();
    void RegenerateScene();
    bool m_regenerateSceneRequested{};
    void DrawInterface();
    void CreateTimingQueries();
    void InspectDebugMessages();
    void UpdateAnimation();
    void GenerateInstanceUpdates();
    void UpdateAccelerationStructure();
    void ReloadShaders();
    void CollectFrame(FrameContext& frame);
    void BeginFrame();
    void WriteStateDump();
    void ExecuteAndWait();
    void UpdateTitle();
    HWND m_window{};
    UINT64 m_timestampFrequency{};
    UINT64 m_frameNumber{};
    UINT64 m_simulationFrame{};
    UINT64 m_testDeviceLossFrame{UINT64_MAX};
    bool m_deviceRecoveryRequested{};
    bool m_cameraInitialized{};
    float m_fps{};
    SampleUI m_ui;
    DirectX::Keyboard m_keyboard;
    DirectX::Keyboard::KeyboardStateTracker m_keyboardButtons;
    UINT m_frameSlot{};
    std::array<FrameContext, kFrameCount> m_frames;
    UINT64 m_frameLimit{};
    UINT m_width{1280};
    UINT m_height{720};
    UINT m_renderScale{2};
    UINT m_adapterIndex{UINT_MAX};
    UINT m_dominoesPerAxis{16};
    UINT m_partitionCountPerAxis{4};
    UINT m_dominoCount{256};
    UINT m_staticCount{};
    UINT m_staticCountLimit{UINT_MAX};
    UINT m_totalInstanceCount{256};
    UINT m_previewDominoCount{256};
    UINT m_partitionCount{16};
    UINT m_maxInstancesPerPartition{16};
    UINT m_lastChangedCount{};
    UINT m_updateMode{};
    UINT m_selfToppleCount{UINT_MAX};
    bool m_markAllInPartition{};
    bool m_vsync{true};
    double m_gpuUpdateMs{};
    double m_gpuAnimationMs{};
    double m_gpuAsMs{};
    double m_gpuTraceMs{};
    double m_gpuCompositeMs{};
    double m_gpuTotalMs{};
    float m_cameraYaw{};
    float m_cameraPitch{};
    float m_cameraFov{60.0f};
    float m_sunAzimuth{};
    float m_sunElevation{DirectX::XMConvertToRadians(45.0f)};
    float m_sceneSize{1000.0f};
    float m_sceneSizeInput{1000.0f};
    int m_partitionInput{4};
    int m_seedInput{1};
    int m_dominoAxisInput{16};
    float m_modeDistance{100.0f};
    float m_metallic{0.1f};
    float m_roughness{1.0f};
    float m_intensity{5.0f};
    int m_maxRayDepth{10};
    int m_lastMouseX{};
    int m_lastMouseY{};
    int m_pickX{};
    int m_pickY{};
    int m_toppleInput{};
    UINT m_toppleIndex{};
    bool m_draggingCamera{};
    bool m_pickRequested{};
    bool m_pickThisFrame{};
    bool m_reloadShadersRequested{};
    std::string m_shaderReloadError;
    bool m_manualTopple{};
    bool m_hasManualTopple{};
    std::chrono::steady_clock::time_point m_lastCameraTime{};
    bool m_usePtlas{true};
    bool m_showPartitionColors{true};
    bool m_paused{};
    bool m_step{};
    bool m_reset{};
    bool m_advancePhysics{};
    bool m_physicsResetPending{true};
    bool m_tlasRefit{true};
    bool m_debug{};
    bool m_minimized{};
    bool m_deviceReady{};
    bool m_fullScene{};
    D3D12_RTAS_PARTITIONED_TLAS_FLAGS m_ptlasHint{D3D12_RTAS_PARTITIONED_TLAS_FLAG_NONE};
    bool m_tlasNeedsSync{true};
    bool m_ptlasNeedsSync{true};
    ComPtr<ID3D12Resource> m_output;
    ComPtr<ID3D12Resource> m_renderOutput;
    std::array<ComPtr<ID3D12Resource>, 3> m_traceImages;
    ComPtr<ID3D12DescriptorHeap> m_descriptors;
    ComPtr<ID3D12Resource> m_vertices;
    ComPtr<ID3D12Resource> m_indices;
    ComPtr<ID3D12Resource> m_triangleData;
    ComPtr<ID3D12Resource> m_pickResult;
    ComPtr<ID3D12Resource> m_blas;
    std::array<ComPtr<ID3D12Resource>, 4> m_staticBlas;
    ComPtr<ID3D12Resource> m_tlas;
    ComPtr<ID3D12Resource> m_ptlas;
    D3D12_RTAS_PARTITIONED_TLAS_INPUTS_DESC m_ptlasInputs{};
    ComPtr<ID3D12Resource> m_tlasScratch;
    ComPtr<ID3D12Resource> m_ptlasScratch;
    ComPtr<ID3D12Resource> m_originalTlasUpload;
    ComPtr<ID3D12Resource> m_ptlasInstances;
    ComPtr<ID3D12Resource> m_gpuTlasInstances;
    ComPtr<ID3D12Resource> m_gpuOriginalRegular;
    ComPtr<ID3D12Resource> m_gpuOriginalPartitioned;
    ComPtr<ID3D12Resource> m_seedAngularVelocities;
    ComPtr<ID3D12Resource> m_gpuPtlasWrites;
    ComPtr<ID3D12Resource> m_gpuPtlasOperation;
    ComPtr<ID3D12Resource> m_instancePartitions;
    std::array<ComPtr<ID3D12Resource>, 2> m_physicsState;
    ComPtr<ID3D12Resource> m_partitionActivity;
    ComPtr<ID3D12Resource> m_partitionActivityZero;
    UINT m_physicsCurrentIndex{};
    D3D12_RESOURCE_STATES m_partitionActivityState{D3D12_RESOURCE_STATE_COPY_DEST};
    D3D12_RESOURCE_STATES m_instancePartitionState{D3D12_RESOURCE_STATE_UNORDERED_ACCESS};
    D3D12_RESOURCE_STATES m_gpuTlasInstancesState{D3D12_RESOURCE_STATE_UNORDERED_ACCESS};
    D3D12_RESOURCE_STATES m_gpuPtlasWritesState{D3D12_RESOURCE_STATE_UNORDERED_ACCESS};
    D3D12_RESOURCE_STATES m_gpuOperationState{D3D12_RESOURCE_STATE_COPY_DEST};
    ComPtr<ID3D12Resource> m_ptlasOperations;
    ComPtr<ID3D12Resource> m_ptlasZeroOperation;
    ComPtr<ID3D12Resource> m_ptlasOperationCount;
    ComPtr<ID3D12QueryHeap> m_timingQueries;
    Vector3 m_cameraEye{};
    Matrix m_viewProjection;
    std::ofstream m_csv;
    std::filesystem::path m_stateDumpPath;
    std::filesystem::path m_screenshotPath;
    std::filesystem::path m_imagePath;
    bool m_screenshotRequested{};
    ComPtr<ID3D12Resource> m_stateDumpReadback;
    bool m_stateDumpPending{};
    PipelineResources m_pipelines;
};
