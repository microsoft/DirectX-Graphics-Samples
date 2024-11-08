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

#include "CompiledShaders/SDFGIProbeVizVS.h"
#include "CompiledShaders/SDFGIProbeVizPS.h"
#include "CompiledShaders/SDFGIProbeVizGS.h"

using namespace Graphics;

namespace SDFGI {
    BoolVar Enable("Graphics/Debug/SDFGI Enable", true);
    BoolVar DebugDraw("Graphics/Debug/SDFGI Debug Draw", false);

    GraphicsPSO s_ProbeVisualizationPSO;    
    RootSignature s_RootSignature;

    void Initialize(void) 
    {
        // 2 parameters (camera constant buffer and probe data structured buffer) and 1 static sampler.
        s_RootSignature.Reset(2, 1);

        // First root parameter is a constant buffer for camera data. Register b0.
        // Visibility ALL because VS and GS access it.
        s_RootSignature[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_ALL);

        // Second root parameter is a structured buffer SRV for probe buffer. Register t0.
        s_RootSignature[1].InitAsBufferSRV(0, D3D12_SHADER_VISIBILITY_VERTEX);

        s_RootSignature.InitStaticSampler(0, SamplerLinearClampDesc);

        s_RootSignature.Finalize(L"SDFGI Root Signature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        s_ProbeVisualizationPSO.SetRootSignature(s_RootSignature);
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

    void Render(GraphicsContext& context, const Math::Camera& camera, SDFGIManager* SDFGIManager)
    {
        if (!Enable)
            return;

        ScopedTimer _prof(L"SDFGI Rendering", context);

        context.SetPipelineState(s_ProbeVisualizationPSO);
        context.SetRootSignature(s_RootSignature);

        // D3D12_DESCRIPTOR_HEAP_TYPE heapTypes[] = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };
        // ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap };
        // context.SetDescriptorHeaps(_countof(descriptorHeaps), heapTypes, descriptorHeaps);

        CameraData camData = {};
        camData.viewProjMatrix = camera.GetViewProjMatrix();
        camData.position = camera.GetPosition();

        context.SetDynamicConstantBufferView(0, sizeof(camData), &camData);

        context.SetBufferSRV(1, SDFGIManager->probeBuffer);

        context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

        context.DrawInstanced(SDFGIManager->probeGrid.probes.size(), 1, 0, 0);
    }

    void UpdateProbeData(GraphicsContext& context)
    {

    }




    SDFGIProbeGrid::SDFGIProbeGrid(Vector3u count, Vector3f spacing) {
        // probeCount[0] = count[0];
        // probeCount[1] = count[1];
        // probeCount[2] = count[2];
        // probeSpacing[0] = spacing[0];
        // probeSpacing[1] = spacing[1];
        // probeSpacing[2] = spacing[2];

        probeCount[0] = 20;
        probeCount[1] = 10;
        probeCount[2] = 50;
        probeSpacing[0] = 100.0f;
        probeSpacing[1] = 100.0f;
        probeSpacing[2] = 100.0f;

        GenerateProbes();
    }

    void SDFGIProbeGrid::GenerateProbes() {
        probes.clear();
        for (uint32_t x = 0; x < probeCount[0]; ++x) {
            for (uint32_t y = 0; y < probeCount[1]; ++y) {
                for (uint32_t z = 0; z < probeCount[2]; ++z) {
                    Vector3 position = Vector3(
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

    SDFGIManager::SDFGIManager(Vector3u probeCount, Vector3f probeSpacing)
        : probeGrid(probeCount, probeSpacing) {
        InitializeTextures();
        InitializeProbeBuffer();
    };

    void SDFGIManager::InitializeTextures() {
        uint32_t width = probeGrid.probeCount[0];
        uint32_t height = probeGrid.probeCount[1];
        uint32_t depth = probeGrid.probeCount[2];

        size_t irradianceRowPitch = width * sizeof(float) * 4;
        size_t depthRowPitch = width * sizeof(float);

        irradianceTexture.Create3D(irradianceRowPitch, width, height, depth, DXGI_FORMAT_R16G16B16A16_FLOAT, nullptr);

        depthTexture.Create3D(depthRowPitch, width, height, depth, DXGI_FORMAT_R16_FLOAT, nullptr);
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
}
