#include "pch.h"
#include "SDFGI.h"
#include "BufferManager.h"
#include "GraphicsCore.h"
#include "CommandContext.h"
#include "Camera.h"
#include "GraphicsCore.h"
#include "CommandContext.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "Utility.h"
#include <random>

#include "CompiledShaders/SDFGIProbeVizVS.h"
#include "CompiledShaders/SDFGIProbeVizPS.h"
#include "CompiledShaders/SDFGIProbeVizGS.h"
#include "CompiledShaders/SDFGIProbeUpdateCS.h"
#include "CompiledShaders/SDFGIProbeIrradianceDepthVizPS.h"
#include "CompiledShaders/SDFGIProbeIrradianceDepthVizVS.h"
#include "CompiledShaders/SDFGIProbeCubemapVizVS.h"
#include "CompiledShaders/SDFGIProbeCubemapVizPS.h"
#include "CompiledShaders/SDFGIProbeCubemapDownsampleCS.h"

using namespace Graphics;
using namespace DirectX;

namespace SDFGI {
    BoolVar Enable("Graphics/Debug/SDFGI Enable", true);
    BoolVar DebugDraw("Graphics/Debug/SDFGI Debug Draw", false);

    GraphicsPSO s_ProbeVisualizationPSO;    
    RootSignature s_ProbeVisualizationRootSignature;

    float GenerateRandomNumber(float min, float max) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(min, max);
        return dis(gen);
    }

    // Used to rotate spherical fibonacci directions.
    XMMATRIX GenerateRandomRotationMatrix(float rotation_scaler) {
        float randomX = GenerateRandomNumber(-1.0f, 1.0f) * rotation_scaler;
        float randomY = GenerateRandomNumber(-1.0f, 1.0f) * rotation_scaler;
        float randomZ = GenerateRandomNumber(-1.0f, 1.0f) * rotation_scaler;
        return XMMatrixRotationRollPitchYaw(randomX, randomY, randomZ);
    }

    void Initialize(void) 
    {
        s_ProbeVisualizationRootSignature.Reset(2, 1);

        // First root parameter is a constant buffer for camera data. Register b0.
        // Visibility ALL because VS and GS access it.
        s_ProbeVisualizationRootSignature[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_ALL);

        // Second root parameter is a structured buffer SRV for probe buffer. Register t0.
        s_ProbeVisualizationRootSignature[1].InitAsBufferSRV(0, D3D12_SHADER_VISIBILITY_VERTEX);

        s_ProbeVisualizationRootSignature.InitStaticSampler(0, SamplerLinearClampDesc);

        s_ProbeVisualizationRootSignature.Finalize(L"SDFGI Root Signature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        s_ProbeVisualizationPSO.SetRootSignature(s_ProbeVisualizationRootSignature);
        s_ProbeVisualizationPSO.SetRasterizerState(RasterizerDefault);
        s_ProbeVisualizationPSO.SetBlendState(BlendDisable);
        s_ProbeVisualizationPSO.SetDepthStencilState(DepthStateReadWrite);
        s_ProbeVisualizationPSO.SetVertexShader(g_pSDFGIProbeVizVS, sizeof(g_pSDFGIProbeVizVS));
        s_ProbeVisualizationPSO.SetGeometryShader(g_pSDFGIProbeVizGS, sizeof(g_pSDFGIProbeVizGS));
        s_ProbeVisualizationPSO.SetPixelShader(g_pSDFGIProbeVizPS, sizeof(g_pSDFGIProbeVizPS));
        s_ProbeVisualizationPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
        s_ProbeVisualizationPSO.SetRenderTargetFormat(g_SceneColorBuffer.GetFormat(), g_SceneDepthBuffer.GetFormat());
        s_ProbeVisualizationPSO.Finalize();
    }


    void Shutdown(void)
    {
        // TODO.
    }

    void Render(GraphicsContext& context, const Math::Camera& camera, SDFGIManager* sdfgiManager, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor)
    {
        if (!Enable)
            return;

        ScopedTimer _prof(L"SDFGI Rendering", context);

        sdfgiManager->RenderCubemapsForProbes(context, camera, viewport, scissor);

        sdfgiManager->CaptureIrradianceAndDepth(context);

        context.SetPipelineState(s_ProbeVisualizationPSO);
        context.SetRootSignature(s_ProbeVisualizationRootSignature);

        CameraData camData = {};
        camData.viewProjMatrix = camera.GetViewProjMatrix();
        camData.position = camera.GetPosition();

        context.SetDynamicConstantBufferView(0, sizeof(camData), &camData);

        context.SetBufferSRV(1, sdfgiManager->probeBuffer);

        context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

        context.DrawInstanced(sdfgiManager->probeGrid.probes.size(), 1, 0, 0);

        // sdfgiManager->RenderIrradianceDepthViz(context, camera, 10, 50);
        sdfgiManager->RenderCubemapViz(context, camera);
    }

    SDFGIProbeGrid::SDFGIProbeGrid(Vector3 &sceneSize, Vector3 &sceneMin) {
        probeSpacing[0] = 350.0f;
        probeSpacing[1] = 350.0f;
        probeSpacing[2] = 350.0f;

        probeCount[0] = std::max(1u, static_cast<uint32_t>(sceneSize.GetX() / probeSpacing[0]));
        probeCount[1] = std::max(1u, static_cast<uint32_t>(sceneSize.GetY() / probeSpacing[1]));
        probeCount[2] = std::max(1u, static_cast<uint32_t>(sceneSize.GetZ() / probeSpacing[2]));

        GenerateProbes(sceneMin);
    }

    void SDFGIProbeGrid::GenerateProbes(Vector3 &sceneMin) {
        probes.clear();
        for (uint32_t x = 0; x < probeCount[0]; ++x) {
            for (uint32_t y = 0; y < probeCount[1]; ++y) {
                for (uint32_t z = 0; z < probeCount[2]; ++z) {
                    Vector3 position = sceneMin + Vector3(
                        x * probeSpacing[0],
                        y * probeSpacing[1],
                        z * probeSpacing[2]
                    );
                     // Initialize position, irradiance and depth.
                    probes.push_back({position, 0.0f, 1.0f});
                }
            }
        }
    }

    SDFGIManager::SDFGIManager(
        Vector3f probeSpacing, const Math::AxisAlignedBox &sceneBounds, 
        std::function<void(GraphicsContext&, const Math::Camera&, const D3D12_VIEWPORT&, const D3D12_RECT&, D3D12_CPU_DESCRIPTOR_HANDLE*, Texture*)> renderFunc
    )
        : probeGrid(sceneBounds.GetDimensions(), sceneBounds.GetMin()), sceneBounds(sceneBounds), renderFunc(renderFunc) {
        InitializeProbeBuffer();
        InitializeTextures();
        InitializeViews();
        InitializeProbeUpdateShader();
        InitializeProbeAtlasVizShader();
        InitializeCubemapVizShader();
        InitializeDownsampleShader();
    };

    void SDFGIManager::InitializeTextures() {
        uint32_t width = probeGrid.probeCount[0];
        uint32_t height = probeGrid.probeCount[1];
        uint32_t depth = probeGrid.probeCount[2];

        size_t irradianceRowPitch = width * sizeof(float) * 4;
        size_t depthRowPitch = width * sizeof(float);

        // 4x4 pixels per probe for octahedral mapping.
        uint32_t probeBlockSize = 4;
        // Padding between probes
        uint32_t gutterSize = 1; 

        uint32_t atlasWidth = (width * probeBlockSize) + (width + 1) * gutterSize;
        uint32_t atlasHeight = (height * probeBlockSize) + (height + 1) * gutterSize;
        size_t rowPitchBytes = atlasWidth * 8;

        irradianceTexture.Create3D(irradianceRowPitch, width, height, depth, DXGI_FORMAT_R16G16B16A16_FLOAT, nullptr);

        irradianceAtlas.Create2D(rowPitchBytes, atlasWidth, atlasHeight, DXGI_FORMAT_R16G16B16A16_FLOAT, nullptr, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

        depthTexture.Create3D(depthRowPitch, width, height, depth, DXGI_FORMAT_R16_FLOAT, nullptr);

        depthAtlas.Create2D(rowPitchBytes / 2, atlasWidth, atlasHeight, DXGI_FORMAT_R16_FLOAT, nullptr, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

        size_t cubeRowPitchBytes = faceResolution * sizeof(float) * 4;
        probeIrradianceCubemap.CreateCube(cubeRowPitchBytes, faceResolution, faceResolution, DXGI_FORMAT_R16G16B16A16_FLOAT, nullptr);

        probeCount = width * height * depth;
        probeCubemapTextures = new Texture*[probeCount];
        for (uint32_t probe  = 0; probe < probeCount; ++probe) {
            probeCubemapTextures[probe] = new Texture[6];

            for (int face = 0; face < 6; ++face)
            {
                probeCubemapTextures[probe][face].Create2D(
                    faceResolution * sizeof(float) * 4,
                    faceResolution, faceResolution,
                    DXGI_FORMAT_R11G11B10_FLOAT,
                    nullptr,
                    D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
                );
            }
        }


        D3D12_RESOURCE_DESC texArrayDesc = {};
        texArrayDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texArrayDesc.Width = 64;
        texArrayDesc.Height = 64;
        texArrayDesc.DepthOrArraySize = probeCount * 6;
        texArrayDesc.MipLevels = 1;
        texArrayDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
        texArrayDesc.SampleDesc.Count = 1;
        texArrayDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texArrayDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        D3D12_HEAP_PROPERTIES HeapProps;
        HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        HeapProps.CreationNodeMask = 1;
        HeapProps.VisibleNodeMask = 1;

        g_Device->CreateCommittedResource(
            &HeapProps,
            D3D12_HEAP_FLAG_NONE,
            &texArrayDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&textureArrayResource)
        );

        textureArrayGpuResource = new GpuResource(textureArrayResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST);

    };

    void SDFGIManager::InitializeViews() {
        irradianceUAV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
        uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        uavDesc.Texture3D.MipSlice = 0;
        uavDesc.Texture3D.FirstWSlice = 0;
        uavDesc.Texture3D.WSize = -1;   
        g_Device->CreateUnorderedAccessView(irradianceTexture.GetResource(), nullptr, &uavDesc, irradianceUAV);

        depthUAV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
        uavDesc.Format = DXGI_FORMAT_R16_FLOAT;
        g_Device->CreateUnorderedAccessView(depthTexture.GetResource(), nullptr, &uavDesc, depthUAV);

        irradianceAtlasUAV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        D3D12_UNORDERED_ACCESS_VIEW_DESC atlasUavDesc = {};
        atlasUavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        atlasUavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        atlasUavDesc.Texture2D.MipSlice = 0;
        atlasUavDesc.Texture2D.PlaneSlice = 0;
        g_Device->CreateUnorderedAccessView(irradianceAtlas.GetResource(), nullptr, &atlasUavDesc, irradianceAtlasUAV);

        depthAtlasUAV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        atlasUavDesc.Format = DXGI_FORMAT_R16_FLOAT;
        g_Device->CreateUnorderedAccessView(depthAtlas.GetResource(), nullptr, &atlasUavDesc, depthAtlasUAV);

        for (int face = 0; face < 6; ++face)
        {
            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
            rtvDesc.Texture2DArray.MipSlice = 0;
            rtvDesc.Texture2DArray.FirstArraySlice = face;
            rtvDesc.Texture2DArray.ArraySize = 1;

            cubemapRTVs[face] = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            g_Device->CreateRenderTargetView(probeIrradianceCubemap.GetResource(), &rtvDesc, cubemapRTVs[face]);
        }

        probeCubemapRTVs = new D3D12_CPU_DESCRIPTOR_HANDLE*[probeCount];
        for (int probe = 0; probe < probeCount; ++probe)
        {
            probeCubemapRTVs[probe] = new D3D12_CPU_DESCRIPTOR_HANDLE[6];
            for (int face = 0; face < 6; ++face)
            {
                probeCubemapRTVs[probe][face] = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
                D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
                g_Device->CreateRenderTargetView(probeCubemapTextures[probe][face].GetResource(), &rtvDesc, probeCubemapRTVs[probe][face]);
            }
        }

        probeCubemapUAVs = new D3D12_CPU_DESCRIPTOR_HANDLE*[probeCount];
        for (int probe = 0; probe < probeCount; ++probe)
        {
            probeCubemapUAVs[probe] = new D3D12_CPU_DESCRIPTOR_HANDLE[6];
            for (int face = 0; face < 6; ++face)
            {
                probeCubemapUAVs[probe][face] = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                D3D12_UNORDERED_ACCESS_VIEW_DESC  uavDesc = {};
                uavDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                uavDesc.Texture2D.MipSlice = 0;
                g_Device->CreateUnorderedAccessView(probeCubemapTextures[probe][face].GetResource(), nullptr, &uavDesc, probeCubemapUAVs[probe][face]);
            }
        }

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        srvDesc.Texture2DArray.MostDetailedMip = 0;
        srvDesc.Texture2DArray.MipLevels = 1;
        srvDesc.Texture2DArray.FirstArraySlice = 0;
        srvDesc.Texture2DArray.ArraySize = probeCount * 6;
        srvDesc.Texture2DArray.PlaneSlice = 0;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        probeCubemapArraySRV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        g_Device->CreateShaderResourceView(textureArrayGpuResource->GetResource(), &srvDesc, probeCubemapArraySRV);

    };

    void SDFGIManager::InitializeProbeBuffer() {
        std::vector<float> probeData;

        for (const auto& probe : probeGrid.probes) {
            probeData.push_back(probe.position.GetX());
            probeData.push_back(probe.position.GetY());
            probeData.push_back(probe.position.GetZ());
            probeData.push_back(probe.irradiance);
        }

        probeBuffer.Create(L"Probe Data Buffer", probeData.size(), sizeof(float), probeData.data());
    }

    void SDFGIManager::InitializeProbeUpdateShader()
    {
        probeUpdateComputeRootSignature.Reset(8, 1);

        // probeBuffer.
        probeUpdateComputeRootSignature[0].InitAsBufferSRV(0, D3D12_SHADER_VISIBILITY_ALL);

        // Irradiance.
        probeUpdateComputeRootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);

        // Depth.
        probeUpdateComputeRootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);

        // Probe grid info.
        probeUpdateComputeRootSignature[3].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_ALL);

        // Irradiance atlas.
        probeUpdateComputeRootSignature[4].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 2, 1);

        // Depth atlas.
        probeUpdateComputeRootSignature[5].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 3, 1);

        probeUpdateComputeRootSignature[6].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6, D3D12_SHADER_VISIBILITY_ALL); 

        probeUpdateComputeRootSignature[7].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 1, D3D12_SHADER_VISIBILITY_ALL);

        probeUpdateComputeRootSignature.InitStaticSampler(0, SamplerLinearClampDesc, D3D12_SHADER_VISIBILITY_ALL);

        probeUpdateComputeRootSignature.Finalize(L"DDGI Compute Root Signature");

        probeUpdateComputePSO.SetRootSignature(probeUpdateComputeRootSignature);
        probeUpdateComputePSO.SetComputeShader(g_pSDFGIProbeUpdateCS, sizeof(g_pSDFGIProbeUpdateCS));
        probeUpdateComputePSO.Finalize();
    }


    void SDFGIManager::CaptureIrradianceAndDepth(GraphicsContext& context) {
        if (irradianceCaptured) return;

        ComputeContext& computeContext = context.GetComputeContext();

        ScopedTimer _prof(L"Capture Irradiance and Depth", context);

        computeContext.SetPipelineState(probeUpdateComputePSO);
        computeContext.SetRootSignature(probeUpdateComputeRootSignature);

        computeContext.SetBufferSRV(0, probeBuffer);
        computeContext.SetDynamicDescriptor(1, 0, irradianceUAV);
        computeContext.SetDynamicDescriptor(2, 0, depthUAV);
        computeContext.SetDynamicDescriptor(4, 0, irradianceAtlasUAV);
        computeContext.SetDynamicDescriptor(5, 0, depthAtlasUAV);

        computeContext.SetDynamicDescriptor(6, 0, probeCubemapTextures[32][0].GetSRV());
        computeContext.SetDynamicDescriptor(6, 1, probeCubemapTextures[32][1].GetSRV());
        computeContext.SetDynamicDescriptor(6, 2, probeCubemapTextures[32][2].GetSRV());
        computeContext.SetDynamicDescriptor(6, 3, probeCubemapTextures[32][3].GetSRV());
        computeContext.SetDynamicDescriptor(6, 4, probeCubemapTextures[32][4].GetSRV());
        computeContext.SetDynamicDescriptor(6, 5, probeCubemapTextures[32][5].GetSRV());

        computeContext.SetDynamicDescriptor(7, 0, probeCubemapArraySRV);

        computeContext.TransitionResource(irradianceTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        computeContext.TransitionResource(depthTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        computeContext.TransitionResource(irradianceAtlas, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        computeContext.TransitionResource(depthAtlas, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);



        struct ProbeData {
            XMFLOAT4X4 randomRotation;
            unsigned int ProbeCount;
            float ProbeMaxDistance;
            Vector3 GridSize;
            Vector3 ProbeSpacing;
            Vector3 SceneMinBounds;
            int ProbeIndex;
        } probeData;

        float rotation_scaler = 3.14159f / 7.0f;
        XMMATRIX randomRotation = GenerateRandomRotationMatrix(rotation_scaler);
        XMStoreFloat4x4(&probeData.randomRotation, randomRotation);

        probeData.ProbeCount = probeGrid.probes.size();
        probeData.ProbeMaxDistance = 50;
        probeData.GridSize = Vector3(probeGrid.probeCount[0], probeGrid.probeCount[1], probeGrid.probeCount[2]);
        probeData.ProbeSpacing = Vector3(probeGrid.probeSpacing[0], probeGrid.probeSpacing[1], probeGrid.probeSpacing[2]);
        probeData.SceneMinBounds = sceneBounds.GetMin();

        computeContext.SetDynamicConstantBufferView(3, sizeof(probeData), &probeData);

        // One thread per probe.
        computeContext.Dispatch(probeGrid.probeCount[0], probeGrid.probeCount[1], probeGrid.probeCount[2]);

        computeContext.TransitionResource(irradianceTexture, D3D12_RESOURCE_STATE_GENERIC_READ);
        computeContext.TransitionResource(depthTexture, D3D12_RESOURCE_STATE_GENERIC_READ);
        computeContext.TransitionResource(irradianceAtlas, D3D12_RESOURCE_STATE_GENERIC_READ);
        computeContext.TransitionResource(depthAtlas, D3D12_RESOURCE_STATE_GENERIC_READ);

        irradianceCaptured = true;
    }

    void SDFGIManager::InitializeProbeAtlasVizShader() {
        textureVisualizationRootSignature.Reset(2, 1);
        // Irradiance atlas.
        textureVisualizationRootSignature[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, D3D12_SHADER_VISIBILITY_PIXEL);
        // Depth atlas.
        textureVisualizationRootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 1, D3D12_SHADER_VISIBILITY_PIXEL);
        textureVisualizationRootSignature.InitStaticSampler(0, SamplerLinearClampDesc);
        textureVisualizationRootSignature.Finalize(L"SDFGI Visualization Root Signature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        textureVisualizationPSO.SetRootSignature(textureVisualizationRootSignature);
        textureVisualizationPSO.SetVertexShader(g_pSDFGIProbeIrradianceDepthVizVS, sizeof(g_pSDFGIProbeIrradianceDepthVizVS));
        textureVisualizationPSO.SetPixelShader(g_pSDFGIProbeIrradianceDepthVizPS, sizeof(g_pSDFGIProbeIrradianceDepthVizPS));
        textureVisualizationPSO.SetRasterizerState(RasterizerDefault);
        textureVisualizationPSO.SetBlendState(BlendDisable);
        textureVisualizationPSO.SetDepthStencilState(DepthStateReadWrite);
        textureVisualizationPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
        textureVisualizationPSO.SetRenderTargetFormat(g_SceneColorBuffer.GetFormat(), g_SceneDepthBuffer.GetFormat());
        textureVisualizationPSO.Finalize();
    }

    void SDFGIManager::RenderIrradianceDepthViz(GraphicsContext& context, const Math::Camera& camera, int sliceIndex, float maxDepthDistance) {
        ScopedTimer _prof(L"Visualize SDFGI Textures", context);

        context.SetPipelineState(textureVisualizationPSO);
        context.SetRootSignature(textureVisualizationRootSignature);

        context.SetDynamicDescriptor(0, 0, irradianceAtlas.GetSRV());
        context.SetDynamicDescriptor(1, 0, depthAtlas.GetSRV());

        context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        context.Draw(4);
    }

    void SDFGIManager::InitializeDownsampleShader() {
        downsampleRootSignature.Reset(3, 1);
        
        downsampleRootSignature[0].InitAsConstantBuffer(0);
        
        // Source texture, typically g_SceneColorBuffer.
        downsampleRootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
        
        // Destination texture, some probe's cubemap face.
        downsampleRootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
        
        downsampleRootSignature.InitStaticSampler(0, SamplerBilinearClampDesc);
        
        downsampleRootSignature.Finalize(L"Downsample Root Signature", D3D12_ROOT_SIGNATURE_FLAG_NONE);

        downsamplePSO.SetRootSignature(downsampleRootSignature);
        downsamplePSO.SetComputeShader(g_pSDFGIProbeCubemapDownsampleCS, sizeof(g_pSDFGIProbeCubemapDownsampleCS)); 
        downsamplePSO.Finalize();
    }


    void SDFGIManager::RenderToCubemapFace(
        GraphicsContext& context, DepthBuffer& depthBuffer, int probe, int face, const Math::Camera& faceCamera, Vector3 &probePosition, const D3D12_VIEWPORT& mainViewport, const D3D12_RECT& mainScissor
    ) {
        // Render to g_SceneColorBuffer (the main render target) using the given cubemap face camera.
        renderFunc(context, faceCamera, mainViewport, mainScissor, &probeCubemapRTVs[probe][face], &probeCubemapTextures[probe][face]);

        // Now copy and downsample g_SceneColorBuffer into probeCubemapTextures (which are square, typically 64x64).
        // Why not render the scene directly to probeCubemapTextures? It was hard to make renderFunc invoke the scene
        // rendering function with an arbitrary render target (i.e. other than g_SceneColorBuffer).

        ComputeContext& computeContext = context.GetComputeContext();

        computeContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, true);
        computeContext.TransitionResource(probeCubemapTextures[probe][face], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

        DownsampleCB downsampleCB;
        downsampleCB.srcSize = Vector3(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight(), 0.0f);
        downsampleCB.dstSize = Vector3(probeCubemapTextures[probe][face].GetWidth(), probeCubemapTextures[probe][face].GetHeight(), 0.0f);
        downsampleCB.scale = Vector3(
            downsampleCB.srcSize.GetX() / downsampleCB.dstSize.GetX(),
            downsampleCB.srcSize.GetY() / downsampleCB.dstSize.GetY(), 0.0f
        );

        computeContext.SetRootSignature(downsampleRootSignature);
        computeContext.SetPipelineState(downsamplePSO);
        computeContext.SetDynamicDescriptor(1, 0, g_SceneColorBuffer.GetSRV()); 
        computeContext.SetDynamicDescriptor(2, 0, probeCubemapUAVs[probe][face]);
        computeContext.SetDynamicConstantBufferView(0, sizeof(downsampleCB), &downsampleCB);
        
        uint32_t dispatchX = (uint32_t)ceil(downsampleCB.dstSize.GetX() / 8.0f);
        uint32_t dispatchY = (uint32_t)ceil(downsampleCB.dstSize.GetY() / 8.0f);
        computeContext.Dispatch(dispatchX, dispatchY, 1);

        computeContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
        computeContext.TransitionResource(probeCubemapTextures[probe][face], D3D12_RESOURCE_STATE_COPY_SOURCE, true);
    }

    void SDFGIManager::RenderCubemapsForProbes(GraphicsContext& context, const Math::Camera& camera, const D3D12_VIEWPORT& mainViewport, const D3D12_RECT& mainScissor)
    {
        // Only render the cubemaps once.
        if (cubeMapsRendered) return;

        std::array<Camera, 6> cubemapCameras;

        Vector3 lookDirections[6] = {
            Vector3(1.0f, 0.0f, 0.0f), 
            Vector3(-1.0f, 0.0f, 0.0f),
            Vector3(0.0f, 1.0f, 0.0f), 
            Vector3(0.0f, -1.0f, 0.0f),
            Vector3(0.0f, 0.0f, 1.0f), 
            Vector3(0.0f, 0.0f, -1.0f) 
        };

        Vector3 upVectors[6] = {
            Vector3(0.0f, 1.0f, 0.0f), 
            Vector3(0.0f, 1.0f, 0.0f), 
            Vector3(0.0f, 0.0f, -1.0f),
            Vector3(0.0f, 0.0f, 1.0f), 
            Vector3(0.0f, 1.0f, 0.0f), 
            Vector3(0.0f, -1.0f, 0.0f) 
        };

        // Render 6 views of the scene for each probe to probeCubemapTextures.
        for (size_t probe = 0; probe < probeGrid.probes.size(); ++probe) {
            Vector3& probePosition = probeGrid.probes[probe].position;

            for (int face = 0; face < 6; ++face)
            {
                Camera faceCamera;
                faceCamera.SetPosition(probePosition);
                faceCamera.SetLookDirection(lookDirections[face], upVectors[face]);
                faceCamera.SetPerspectiveMatrix(XM_PI / 2.0f, 1.0f, camera.GetNearClip(), camera.GetFarClip());
                faceCamera.ReverseZ(camera.GetReverseZ());
                faceCamera.Update();

                RenderToCubemapFace(context, g_SceneDepthBuffer, probe, face, faceCamera, probePosition, mainViewport, mainScissor);
            }
        }

        // Copy each of the textures in probeCubemapTextures to a texture array.
        // TODO: can we render directly to the texture array?
        // TODO: the shader seems to see the same texture at every index in the array.
        for (int probe = 0; probe < probeCount; ++probe) {
            for (int face = 0; face < 6; ++face) {
                D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
                srcLocation.pResource = probeCubemapTextures[probe][face].GetResource();
                srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                srcLocation.SubresourceIndex = 0;

                D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
                dstLocation.pResource = textureArrayGpuResource->GetResource();
                dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                dstLocation.SubresourceIndex = probe * 6 + face;

                D3D12_BOX srcBox = { 0, 0, 0, 64, 64, 1 };
                context.GetCommandList()->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, &srcBox);
            }
        }

        context.TransitionResource(*textureArrayGpuResource, D3D12_RESOURCE_STATE_GENERIC_READ);

        cubeMapsRendered = true;
    }

    // This shader renders the 6 faces of the cubemap of a single probe to a fullscreen quad. See RenderCubemapViz.
    void SDFGIManager::InitializeCubemapVizShader() {
        cubemapVisualizationRootSignature.Reset(2, 1);

        cubemapVisualizationRootSignature[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 6, D3D12_SHADER_VISIBILITY_PIXEL);
        cubemapVisualizationRootSignature[1].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
        cubemapVisualizationRootSignature.InitStaticSampler(0, SamplerLinearClampDesc);
        cubemapVisualizationRootSignature.Finalize(L"Cubemap Visualization Root Signature");

        cubemapVisualizationPSO.SetRootSignature(cubemapVisualizationRootSignature);
        cubemapVisualizationPSO.SetVertexShader(g_pSDFGIProbeCubemapVizVS, sizeof(g_pSDFGIProbeCubemapVizVS));
        cubemapVisualizationPSO.SetPixelShader(g_pSDFGIProbeCubemapVizPS, sizeof(g_pSDFGIProbeCubemapVizPS));
        cubemapVisualizationPSO.SetRasterizerState(RasterizerDefault);
        cubemapVisualizationPSO.SetBlendState(BlendDisable);
        cubemapVisualizationPSO.SetDepthStencilState(DepthStateReadWrite);
        cubemapVisualizationPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
        cubemapVisualizationPSO.SetRenderTargetFormat(g_SceneColorBuffer.GetFormat(), g_SceneDepthBuffer.GetFormat());
        cubemapVisualizationPSO.Finalize();
    }

    // Renders the 6 faces of the cubemap of a single probe to a fullscreen quad.
    void SDFGIManager::RenderCubemapViz(GraphicsContext& context, const Math::Camera& camera) {
        ScopedTimer _prof(L"Visualize Cubemap Faces", context);

        context.SetPipelineState(cubemapVisualizationPSO);
        context.SetRootSignature(cubemapVisualizationRootSignature);

        for (int face = 0; face < 6; ++face) {
            context.SetDynamicDescriptor(0, face, probeCubemapTextures[179][face].GetSRV());
        }

        int GridColumns = 3;
        int GridRows = 2;
        float CellSize = 1.0f / std::max(GridColumns, GridRows);

        __declspec(align(16)) struct GridConfig {
            int GridColumns;
            int GridRows;
            float CellSize;
            float pad;
        } config = { GridColumns, GridRows, CellSize, 0.0f };
        context.SetDynamicConstantBufferView(1, sizeof(config), &config);

        context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        context.Draw(4);
    }
}
