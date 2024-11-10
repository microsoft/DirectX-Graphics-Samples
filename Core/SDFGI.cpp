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
#include "CompiledShaders/SDFGIProbeCubemapVizVS.h"
#include "CompiledShaders/SDFGIProbeCubemapVizPS.h"
#include "CompiledShaders/SDFGISimpleVS.h"
#include "CompiledShaders/SDFGISimplePS.h"

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

    void UpdateProbeData(GraphicsContext& context)
    {

    }




    SDFGIProbeGrid::SDFGIProbeGrid(Vector3 &sceneSize, Vector3 &sceneMin) {
        probeSpacing[0] = 250.0f;
        probeSpacing[1] = 250.0f;
        probeSpacing[2] = 250.0f;

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
        Vector3u probeCount, Vector3f probeSpacing, const Math::AxisAlignedBox &sceneBounds, 
        std::function<void(GraphicsContext&, const Math::Camera&, const D3D12_VIEWPORT&, const D3D12_RECT&, D3D12_CPU_DESCRIPTOR_HANDLE*, Texture*)> renderFunc
    )
        : probeGrid(sceneBounds.GetDimensions(), sceneBounds.GetMin()), sceneBounds(sceneBounds), renderFunc(renderFunc) {
        InitializeProbeBuffer();
        InitializeTextures();
        InitializeViews();
        InitializeProbeUpdateShader();
        InitializeTextureVizShader();
        InitializeCubemapVisualizationShader();
        InitializePentagon();
        InitializeSimpleQuadPipelineState();
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

        size_t cubeRowPitchBytes = faceResolution * sizeof(float) * 4;
        probeIrradianceCubemap.CreateCube(cubeRowPitchBytes, faceResolution, faceResolution, DXGI_FORMAT_R16G16B16A16_FLOAT, nullptr);

        probeCount = width * height * depth;
        intermediateTextures = new Texture*[probeCount];
        for (uint32_t probe  = 0; probe < probeCount; ++probe) {
            intermediateTextures[probe] = new Texture[6];

            for (int face = 0; face < 6; ++face)
            {
                intermediateTextures[probe][face].Create2D(
                    faceResolution * sizeof(float) * 4,
                    faceResolution, faceResolution,
                    DXGI_FORMAT_R11G11B10_FLOAT,
                    nullptr
                );
            }
        }
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

        intermediateRTVs = new D3D12_CPU_DESCRIPTOR_HANDLE*[probeCount];
        for (int probe = 0; probe < probeCount; ++probe)
        {
            intermediateRTVs[probe] = new D3D12_CPU_DESCRIPTOR_HANDLE[6];
            for (int face = 0; face < 6; ++face)
            {
                intermediateRTVs[probe][face] = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
                D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
                g_Device->CreateRenderTargetView(intermediateTextures[probe][face].GetResource(), &rtvDesc, intermediateRTVs[probe][face]);
            }
        }
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

    Matrix4 SDFGIManager::GetViewMatrixForCubemapFace(int faceIndex, const Vector3& probePosition) {
        static const Vector3 lookDirections[6] = {
            Vector3(1.0f, 0.0f, 0.0f),  // +X
            Vector3(-1.0f, 0.0f, 0.0f), // -X
            Vector3(0.0f, 1.0f, 0.0f),  // +Y
            Vector3(0.0f, -1.0f, 0.0f), // -Y
            Vector3(0.0f, 0.0f, 1.0f),  // +Z
            Vector3(0.0f, 0.0f, -1.0f)  // -Z
        };

        static const Vector3 upVectors[6] = {
            Vector3(0.0f, 1.0f, 0.0f),  // +X
            Vector3(0.0f, 1.0f, 0.0f),  // -X
            Vector3(0.0f, 0.0f, -1.0f), // +Y
            Vector3(0.0f, 0.0f, 1.0f),  // -Y
            Vector3(0.0f, 1.0f, 0.0f),  // +Z
            Vector3(0.0f, 1.0f, 0.0f)   // -Z
        };

        assert(faceIndex >= 0 && faceIndex < 6);

        Vector3 forward = Normalize(lookDirections[faceIndex]);
        Vector3 right = Normalize(Cross(upVectors[faceIndex], forward));
        Vector3 up = Cross(forward, right);

        Vector4 rowX = Vector4(right.GetX(), up.GetX(), forward.GetX(), 0.0f);
        Vector4 rowY = Vector4(right.GetY(), up.GetY(), forward.GetY(), 0.0f);
        Vector4 rowZ = Vector4(right.GetY(), up.GetY(), forward.GetZ(), 0.0f);
        Vector4 rowW = Vector4(-Dot(right, probePosition), -Dot(up, probePosition), -Dot(forward, probePosition), 1.0f);

        Matrix4 viewMatrix;
        viewMatrix.SetX(rowX);
        viewMatrix.SetY(rowY);
        viewMatrix.SetZ(rowZ);
        viewMatrix.SetW(rowW);

        return viewMatrix;
    }

    static Matrix4 MakeLookAt(Vector3 eye, Vector3 target, Vector3 up)
    {
        Vector3 forward = Math::Normalize(target - eye);
        Vector3 right = Math::Normalize(Math::Cross(up, forward));
        Vector3 camUp = Math::Cross(forward, right);

        return Matrix4(
            Vector4(right.GetX(), camUp.GetX(), forward.GetX(), 0.0f),
            Vector4(right.GetY(), camUp.GetY(), forward.GetY(), 0.0f),
            Vector4(right.GetZ(), camUp.GetZ(), forward.GetZ(), 0.0f),
            Vector4(-Math::Dot(right, eye), -Math::Dot(camUp, eye), -Math::Dot(forward, eye), 1.0f)
        );
    }

    void SDFGIManager::InitializePentagon() {
        
    }

    void SDFGIManager::SimpleRenderFunc(GraphicsContext& context, const Math::Camera& camera, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor) {
        context.SetPipelineState(BasicPipelineState);
        context.SetRootSignature(BasicRootSignature);
        context.SetVertexBuffer(0, pentagonVertexBufferView);
        context.SetIndexBuffer(pentagonIndexBufferView);
        context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context.SetViewportAndScissor(viewport, scissor);
        context.DrawIndexed(9); 
    }


    void SDFGIManager::InitializeSimpleQuadPipelineState()
    {
        // BasicRootSignature.Reset(1, 0);
        // BasicRootSignature[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
        // BasicRootSignature.Finalize(L"Simple Quad Root Signature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        // D3D12_INPUT_ELEMENT_DESC inputLayout[] =
        // {
        //     { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        //     { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        // };

        // BasicPipelineState.SetInputLayout(_countof(inputLayout), inputLayout);
        // BasicPipelineState.SetRootSignature(BasicRootSignature);
        // BasicPipelineState.SetVertexShader(g_pSDFGISimpleVS, sizeof(g_pSDFGISimpleVS));
        // BasicPipelineState.SetPixelShader(g_pSDFGISimplePS, sizeof(g_pSDFGISimplePS));
        // BasicPipelineState.SetRasterizerState(RasterizerDefault);
        // BasicPipelineState.SetBlendState(BlendDisable);
        // BasicPipelineState.SetDepthStencilState(DepthStateDisabled);
        // BasicPipelineState.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
        // BasicPipelineState.SetRenderTargetFormat(g_SceneColorBuffer.GetFormat(), DXGI_FORMAT_UNKNOWN);
        // BasicPipelineState.Finalize();

        BasicRootSignature.Reset(1, 0);  // Assuming one CBV in the root signature
        BasicRootSignature[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);  // Register b0
        BasicRootSignature.Finalize(L"Fullscreen Quad Root Signature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        BasicPipelineState.SetRootSignature(BasicRootSignature);
        BasicPipelineState.SetVertexShader(g_pSDFGISimpleVS, sizeof(g_pSDFGISimpleVS));
        BasicPipelineState.SetPixelShader(g_pSDFGISimplePS, sizeof(g_pSDFGISimplePS));
        BasicPipelineState.SetRasterizerState(RasterizerDefault);
        BasicPipelineState.SetBlendState(BlendDisable);
        BasicPipelineState.SetDepthStencilState(DepthStateDisabled);
        BasicPipelineState.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
        BasicPipelineState.SetRenderTargetFormat(g_SceneColorBuffer.GetFormat(), DXGI_FORMAT_UNKNOWN);
        BasicPipelineState.Finalize();

    }

    void SetupCubemapCameras(const Vector3& probePosition, std::array<Camera, 6>& cubemapCameras)
    {
        
    }


    void SDFGIManager::RenderToCubemapFace(
        GraphicsContext& context, DepthBuffer& depthBuffer, int probe, int face, const Math::Camera& camera, Vector3 &probePosition, const D3D12_VIEWPORT& mainViewport, const D3D12_RECT& mainScissor
    ) {
        // context.TransitionResource(intermediateTextures[probe][face], D3D12_RESOURCE_STATE_RENDER_TARGET, true);

        // context.TransitionResource(depthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);

        // context.SetRenderTarget(intermediateRTVs[probe][face], depthBuffer.GetDSV());

        // context.SetViewportAndScissor(0, 0, cubemapResolution, cubemapResolution);

        // Matrix4 projMatrix = Matrix4::MakePerspectiveMatrix(camera.GetFOV(), camera.GetAspectRatio(), camera.GetNearClip(), camera.GetFarClip());
        std::array<Camera, 6> cubemapCameras;
        Vector3 lookDirections[6] = {
            Vector3(1.0f, 0.0f, 0.0f),   // +X
            Vector3(-1.0f, 0.0f, 0.0f),  // -X
            Vector3(0.0f, 1.0f, 0.0f),   // +Y
            Vector3(0.0f, -1.0f, 0.0f),  // -Y
            Vector3(0.0f, 0.0f, 1.0f),   // +Z
            Vector3(0.0f, 0.0f, -1.0f)   // -Z
        };
        Vector3 upVectors[6] = {
            Vector3(0.0f, 1.0f, 0.0f),   // +X
            Vector3(0.0f, 1.0f, 0.0f),   // -X
            Vector3(0.0f, 0.0f, -1.0f),  // +Y
            Vector3(0.0f, 0.0f, 1.0f),   // -Y
            Vector3(0.0f, 1.0f, 0.0f),   // +Z
            Vector3(0.0f, -1.0f, 0.0f)   // -Z
        };
        for (int face = 0; face < 6; ++face)
        {
            cubemapCameras[face].SetPosition(probePosition);
            cubemapCameras[face].SetLookDirection(lookDirections[face], upVectors[face]);
            cubemapCameras[face].SetPerspectiveMatrix(XM_PI / 2.0f, 1.0f, camera.GetNearClip(), camera.GetFarClip());
            cubemapCameras[face].ReverseZ(camera.GetReverseZ());
            cubemapCameras[face].Update();
        }

        // // Matrix4 staticViewMatrix = MakeLookAt(Vector3(0, 0, 0), Vector3(0, 0, 1), Vector3(0, 1, 0));
        // // camData.viewProjMatrix = projMatrix * staticViewMatrix;
        // Matrix4 viewMatrix = GetViewMatrixForCubemapFace(face, probePosition);
        
        // CameraData camData;
        // camData.viewProjMatrix = projMatrix * viewMatrix;
        // context.SetDynamicConstantBufferView(0, sizeof(camData), &camData);

        float clearColors[6][4] = {
            {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}
        };
        float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        // context.GetCommandList()->ClearRenderTargetView(intermediateRTVs[probe][face], clearColors[face], 0, nullptr);
        // context.ClearDepth(depthBuffer);

        // auto renderLambda = [&]() {
        //     context.SetPipelineState(BasicPipelineState);
        //     context.SetRootSignature(BasicRootSignature);
        //     context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        //     context.SetDynamicConstantBufferView(0, sizeof(uint32_t), &face);
        //     context.Draw(4);
        // };

        // renderLambda();

        // D3D12_VIEWPORT cubemapViewport = {0.0f, 0.0f, faceResolution.0, faceResolution.0, 0.0f, 1.0f};
        // D3D12_RECT cubemapScissorRect = {0, 0, faceResolution, faceResolution};
        renderFunc(context, cubemapCameras[face], mainViewport, mainScissor, &intermediateRTVs[probe][face], &intermediateTextures[probe][face]);
        // // renderFunc(context, camera, viewport, scissor, &intermediateRTVs[probe][face]);
        // // SimpleRenderFunc(context, camera, viewport, scissor);

        context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);
        context.TransitionResource(intermediateTextures[probe][face], D3D12_RESOURCE_STATE_COPY_DEST, true);

        D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
        srcLocation.pResource = g_SceneColorBuffer.GetResource();
        srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        srcLocation.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
        dstLocation.pResource = intermediateTextures[probe][face].GetResource();
        dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dstLocation.SubresourceIndex = 0;

        D3D12_BOX srcBox = {};
        srcBox.left = 0;
        srcBox.top = 0;
        srcBox.front = 0;
        srcBox.right = faceResolution;  
        srcBox.bottom = faceResolution;
        srcBox.back = 1;
        context.GetCommandList()->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, &srcBox);

        context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
        context.TransitionResource(intermediateTextures[probe][face], D3D12_RESOURCE_STATE_GENERIC_READ, true);

        // context.TransitionResource(depthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ, true);

        // context.TransitionResource(intermediateTextures[probe][face], D3D12_RESOURCE_STATE_GENERIC_READ, true);
    }


    void SDFGIManager::RenderCubemapsForProbes(GraphicsContext& context, const Math::Camera& camera, const D3D12_VIEWPORT& mainViewport, const D3D12_RECT& mainScissor)
    {
        if (cubeMapsRendered) return;
        
        // D3D12_VIEWPORT cubemapViewport = {0.0f, 0.0f, faceResolution.0, faceResolution.0, 0.0f, 1.0f};
        // D3D12_RECT cubemapScissorRect = {0, 0, faceResolution, faceResolution};

         for (size_t probe = 0; probe < probeGrid.probes.size(); ++probe)
        {
            Vector3& probePosition = probeGrid.probes[probe].position;
            Math::Camera probeCamera;

            for (int face = 0; face < 6; ++face)
            {
                RenderToCubemapFace(context, g_SceneDepthBuffer, probe, face, camera, probePosition, mainViewport, mainScissor);
            }
        }

        cubeMapsRendered = true;
    }

    void SDFGIManager::InitializeCubemapVisualizationShader() {
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

    void SDFGIManager::RenderCubemapViz(GraphicsContext& context, const Math::Camera& camera) {
        ScopedTimer _prof(L"Visualize Cubemap Faces", context);

        context.SetPipelineState(cubemapVisualizationPSO);
        context.SetRootSignature(cubemapVisualizationRootSignature);

        for (int face = 0; face < 6; ++face) {
            context.SetDynamicDescriptor(0, face, intermediateTextures[555][face].GetSRV());
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


    void SDFGIManager::CopyCubemapFaceToIntermediate(GraphicsContext& context, int face)
    {
    }
}
