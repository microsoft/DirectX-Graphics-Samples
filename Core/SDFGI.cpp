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

#include "CompiledShaders/SDFGIProbeVizVS.h"
#include "CompiledShaders/SDFGIProbeVizPS.h"
#include "CompiledShaders/SDFGIProbeVizGS.h"
#include "CompiledShaders/SDFGIProbeUpdateCS.h"
#include "CompiledShaders/SDFGIProbeIrradianceDepthVizPS.h"
#include "CompiledShaders/SDFGIProbeIrradianceDepthVizVS.h"

using namespace Graphics;

namespace SDFGI {
    BoolVar Enable("Graphics/Debug/SDFGI Enable", true);
    BoolVar DebugDraw("Graphics/Debug/SDFGI Debug Draw", false);

    GraphicsPSO s_ProbeVisualizationPSO;    
    RootSignature s_ProbeVisualizationRootSignature;

    void Initialize(void) 
    {
        // 2 parameters (camera constant buffer and probe data structured buffer) and 1 static sampler.
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

    }

    void Render(GraphicsContext& context, const Math::Camera& camera, SDFGIManager* sdfgiManager)
    {
        if (!Enable)
            return;

        ScopedTimer _prof(L"SDFGI Rendering", context);

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

        sdfgiManager->RenderIrradianceDepthViz(context, camera, 10, 50);
    }

    void UpdateProbeData(GraphicsContext& context)
    {

    }




    SDFGIProbeGrid::SDFGIProbeGrid(Vector3 &sceneSize, Vector3 &sceneMin) {
        probeSpacing[0] = 100.0f;
        probeSpacing[1] = 100.0f;
        probeSpacing[2] = 100.0f;

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

    SDFGIManager::SDFGIManager(Vector3u probeCount, Vector3f probeSpacing, const Math::AxisAlignedBox &sceneBounds)
        : probeGrid(sceneBounds.GetDimensions(), sceneBounds.GetMin()), sceneBounds(sceneBounds) {
        InitializeTextures();
        InitializeViews();
        InitializeProbeBuffer();
        InitializeProbeUpdateShader();
        InitializeTextureVizShader();
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

        irradianceAtlas.Create2D(rowPitchBytes, atlasWidth, atlasHeight, DXGI_FORMAT_R16G16B16A16_FLOAT, nullptr);

        depthTexture.Create3D(depthRowPitch, width, height, depth, DXGI_FORMAT_R16_FLOAT, nullptr);

        depthAtlas.Create2D(rowPitchBytes / 2, atlasWidth, atlasHeight, DXGI_FORMAT_R16_FLOAT, nullptr);
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
        probeUpdateComputeRootSignature.Reset(6, 0);

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

        computeContext.TransitionResource(irradianceTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        computeContext.TransitionResource(depthTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        computeContext.TransitionResource(irradianceAtlas, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        computeContext.TransitionResource(depthAtlas, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        struct ProbeData {
            unsigned int ProbeCount;
            float ProbeMaxDistance;
            Vector3 GridSize;
            Vector3 ProbeSpacing;
            Vector3 SceneMinBounds;
        } probeData;

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

    void SDFGIManager::InitializeTextureVizShader()
    {
        textureVisualizationRootSignature.Reset(5, 1);
        // Irradiance.
        textureVisualizationRootSignature[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
        // Depth.
        textureVisualizationRootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL);
        // Probe grid info.
        textureVisualizationRootSignature[2].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_ALL);
        textureVisualizationRootSignature[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, D3D12_SHADER_VISIBILITY_PIXEL);
        textureVisualizationRootSignature[4].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 1, D3D12_SHADER_VISIBILITY_PIXEL);
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

    void SDFGIManager::RenderIrradianceDepthViz(GraphicsContext& context, const Math::Camera& camera, int sliceIndex, float maxDepthDistance)
    {
        ScopedTimer _prof(L"Visualize SDFGI Textures", context);

        context.SetPipelineState(textureVisualizationPSO);
        context.SetRootSignature(textureVisualizationRootSignature);

        context.SetDynamicDescriptor(0, 0, irradianceTexture.GetSRV());
        context.SetDynamicDescriptor(1, 0, depthTexture.GetSRV());

        context.SetDynamicDescriptor(3, 0, irradianceAtlas.GetSRV());
        context.SetDynamicDescriptor(4, 0, depthAtlas.GetSRV());

        int totalProbes = probeGrid.probeCount[0] * probeGrid.probeCount[1] * probeGrid.probeCount[2];
        int atlasColumns = static_cast<int>(ceil(sqrt(static_cast<float>(totalProbes))));
        int atlasRows = (totalProbes + atlasColumns - 1) / atlasColumns;
        float cellSize = 1.0f / static_cast<float>(std::max(atlasColumns, atlasRows));


        __declspec(align(16)) struct VisualizationData {
            int sliceIndex;
            int depthDimension;
            float maxDepthDistance;
            int atlasColumns;
            int atlasRows;
            float cellSize;
        } data = { sliceIndex,  probeGrid.probeCount[2], maxDepthDistance, atlasColumns, atlasRows, cellSize };
        context.SetDynamicConstantBufferView(2, sizeof(data), &data);

        context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        context.Draw(4);
    }

}
