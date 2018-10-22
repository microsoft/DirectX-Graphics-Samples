//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author(s):  Alex Nankervis
//             James Stanard
//

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

// To enable wave intrinsics, uncomment this macro and #define DXIL in Core/GraphcisCore.cpp.
// Run CompileSM6Test.bat to compile the relevant shaders with DXC.
//#define _WAVE_OP

#include "CompiledShaders/DepthViewerVS.h"
#include "CompiledShaders/DepthViewerPS.h"
#include "CompiledShaders/ModelViewerVS.h"
#include "CompiledShaders/ModelViewerPS.h"
#ifdef _WAVE_OP
#include "CompiledShaders/DepthViewerVS_SM6.h"
#include "CompiledShaders/ModelViewerVS_SM6.h"
#include "CompiledShaders/ModelViewerPS_SM6.h"
#endif
#include "CompiledShaders/WaveTileCountPS.h"

using namespace GameCore;
using namespace Math;
using namespace Graphics;

class ModelViewer : public GameCore::IGameApp
{
public:

    ModelViewer( void ) {}

    virtual void Startup( void ) override;
    virtual void Cleanup( void ) override;

    virtual void Update( float deltaT ) override;
    virtual void RenderScene( void ) override;

private:

    void RenderLightShadows(GraphicsContext& gfxContext);

    enum eObjectFilter { kOpaque = 0x1, kCutout = 0x2, kTransparent = 0x4, kAll = 0xF, kNone = 0x0 };
    void RenderObjects( GraphicsContext& Context, const Matrix4& ViewProjMat, eObjectFilter Filter = kAll );
    void CreateParticleEffects();
    Camera m_Camera;
    std::auto_ptr<CameraController> m_CameraController;
    Matrix4 m_ViewProjMatrix;
    D3D12_VIEWPORT m_MainViewport;
    D3D12_RECT m_MainScissor;

    RootSignature m_RootSig;
    GraphicsPSO m_DepthPSO;
    GraphicsPSO m_CutoutDepthPSO;
    GraphicsPSO m_ModelPSO;
#ifdef _WAVE_OP
    GraphicsPSO m_DepthWaveOpsPSO;
    GraphicsPSO m_ModelWaveOpsPSO;
#endif
    GraphicsPSO m_CutoutModelPSO;
    GraphicsPSO m_ShadowPSO;
    GraphicsPSO m_CutoutShadowPSO;
    GraphicsPSO m_WaveTileCountPSO;

    D3D12_CPU_DESCRIPTOR_HANDLE m_DefaultSampler;
    D3D12_CPU_DESCRIPTOR_HANDLE m_ShadowSampler;
    D3D12_CPU_DESCRIPTOR_HANDLE m_BiasedDefaultSampler;

    D3D12_CPU_DESCRIPTOR_HANDLE m_ExtraTextures[6];
    Model m_Model;
    std::vector<bool> m_pMaterialIsCutout;

    Vector3 m_SunDirection;
    ShadowCamera m_SunShadow;
};

CREATE_APPLICATION( ModelViewer )

ExpVar m_SunLightIntensity("Application/Lighting/Sun Light Intensity", 4.0f, 0.0f, 16.0f, 0.1f);
ExpVar m_AmbientIntensity("Application/Lighting/Ambient Intensity", 0.1f, -16.0f, 16.0f, 0.1f);
NumVar m_SunOrientation("Application/Lighting/Sun Orientation", -0.5f, -100.0f, 100.0f, 0.1f );
NumVar m_SunInclination("Application/Lighting/Sun Inclination", 0.75f, 0.0f, 1.0f, 0.01f );
NumVar ShadowDimX("Application/Lighting/Shadow Dim X", 5000, 1000, 10000, 100 );
NumVar ShadowDimY("Application/Lighting/Shadow Dim Y", 3000, 1000, 10000, 100 );
NumVar ShadowDimZ("Application/Lighting/Shadow Dim Z", 3000, 1000, 10000, 100 );

BoolVar ShowWaveTileCounts("Application/Forward+/Show Wave Tile Counts", false);
#ifdef _WAVE_OP
BoolVar EnableWaveOps("Application/Forward+/Enable Wave Ops", true);
#endif

void ModelViewer::Startup( void )
{
    SamplerDesc DefaultSamplerDesc;
    DefaultSamplerDesc.MaxAnisotropy = 8;

    m_RootSig.Reset(5, 2);
    m_RootSig.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig.InitStaticSampler(1, SamplerShadowDesc, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
    m_RootSig[1].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 6, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 64, 6, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig[4].InitAsConstants(1, 2, D3D12_SHADER_VISIBILITY_VERTEX);
    m_RootSig.Finalize(L"ModelViewer", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    DXGI_FORMAT ColorFormat = g_SceneColorBuffer.GetFormat();
    DXGI_FORMAT DepthFormat = g_SceneDepthBuffer.GetFormat();

    D3D12_INPUT_ELEMENT_DESC vertElem[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // Depth-only (2x rate)
    m_DepthPSO.SetRootSignature(m_RootSig);
    m_DepthPSO.SetRasterizerState(RasterizerDefault);
    m_DepthPSO.SetBlendState(BlendNoColorWrite);
    m_DepthPSO.SetDepthStencilState(DepthStateReadWrite);
    m_DepthPSO.SetInputLayout(_countof(vertElem), vertElem);
    m_DepthPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    m_DepthPSO.SetRenderTargetFormats(0, nullptr, DepthFormat);
    m_DepthPSO.SetVertexShader(g_pDepthViewerVS, sizeof(g_pDepthViewerVS));
    m_DepthPSO.Finalize();

    // Depth-only shading but with alpha testing
    m_CutoutDepthPSO = m_DepthPSO;
    m_CutoutDepthPSO.SetPixelShader(g_pDepthViewerPS, sizeof(g_pDepthViewerPS));
    m_CutoutDepthPSO.SetRasterizerState(RasterizerTwoSided);
    m_CutoutDepthPSO.Finalize();

    // Depth-only but with a depth bias and/or render only backfaces
    m_ShadowPSO = m_DepthPSO;
    m_ShadowPSO.SetRasterizerState(RasterizerShadow);
    m_ShadowPSO.SetRenderTargetFormats(0, nullptr, g_ShadowBuffer.GetFormat());
    m_ShadowPSO.Finalize();

    // Shadows with alpha testing
    m_CutoutShadowPSO = m_ShadowPSO;
    m_CutoutShadowPSO.SetPixelShader(g_pDepthViewerPS, sizeof(g_pDepthViewerPS));
    m_CutoutShadowPSO.SetRasterizerState(RasterizerShadowTwoSided);
    m_CutoutShadowPSO.Finalize();

    // Full color pass
    m_ModelPSO = m_DepthPSO;
    m_ModelPSO.SetBlendState(BlendDisable);
    m_ModelPSO.SetDepthStencilState(DepthStateTestEqual);
    m_ModelPSO.SetRenderTargetFormats(1, &ColorFormat, DepthFormat);
    m_ModelPSO.SetVertexShader( g_pModelViewerVS, sizeof(g_pModelViewerVS) );
    m_ModelPSO.SetPixelShader( g_pModelViewerPS, sizeof(g_pModelViewerPS) );
    m_ModelPSO.Finalize();

#ifdef _WAVE_OP
    m_DepthWaveOpsPSO = m_DepthPSO;
    m_DepthWaveOpsPSO.SetVertexShader( g_pDepthViewerVS_SM6, sizeof(g_pDepthViewerVS_SM6) );
    m_DepthWaveOpsPSO.Finalize();

    m_ModelWaveOpsPSO = m_ModelPSO;
    m_ModelWaveOpsPSO.SetVertexShader( g_pModelViewerVS_SM6, sizeof(g_pModelViewerVS_SM6) );
    m_ModelWaveOpsPSO.SetPixelShader( g_pModelViewerPS_SM6, sizeof(g_pModelViewerPS_SM6) );
    m_ModelWaveOpsPSO.Finalize();
#endif

    m_CutoutModelPSO = m_ModelPSO;
    m_CutoutModelPSO.SetRasterizerState(RasterizerTwoSided);
    m_CutoutModelPSO.Finalize();

    // A debug shader for counting lights in a tile
    m_WaveTileCountPSO = m_ModelPSO;
    m_WaveTileCountPSO.SetPixelShader(g_pWaveTileCountPS, sizeof(g_pWaveTileCountPS));
    m_WaveTileCountPSO.Finalize();

    Lighting::InitializeResources();

    m_ExtraTextures[0] = g_SSAOFullScreen.GetSRV();
    m_ExtraTextures[1] = g_ShadowBuffer.GetSRV();

    TextureManager::Initialize(L"Textures/");
    ASSERT(m_Model.Load("Models/sponza.h3d"), "Failed to load model");
    ASSERT(m_Model.m_Header.meshCount > 0, "Model contains no meshes");

    // The caller of this function can override which materials are considered cutouts
    m_pMaterialIsCutout.resize(m_Model.m_Header.materialCount);
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
    }

    CreateParticleEffects();

    float modelRadius = Length(m_Model.m_Header.boundingBox.max - m_Model.m_Header.boundingBox.min) * .5f;
    const Vector3 eye = (m_Model.m_Header.boundingBox.min + m_Model.m_Header.boundingBox.max) * .5f + Vector3(modelRadius * .5f, 0.0f, 0.0f);
    m_Camera.SetEyeAtUp( eye, Vector3(kZero), Vector3(kYUnitVector) );
    m_Camera.SetZRange( 1.0f, 10000.0f );
    m_CameraController.reset(new CameraController(m_Camera, Vector3(kYUnitVector)));

    MotionBlur::Enable = true;
    TemporalEffects::EnableTAA = true;
    FXAA::Enable = false;
    PostEffects::EnableHDR = true;
    PostEffects::EnableAdaptation = true;
    SSAO::Enable = true;

    Lighting::CreateRandomLights(m_Model.GetBoundingBox().min, m_Model.GetBoundingBox().max);

    m_ExtraTextures[2] = Lighting::m_LightBuffer.GetSRV();
    m_ExtraTextures[3] = Lighting::m_LightShadowArray.GetSRV();
    m_ExtraTextures[4] = Lighting::m_LightGrid.GetSRV();
    m_ExtraTextures[5] = Lighting::m_LightGridBitMask.GetSRV();
}

void ModelViewer::Cleanup( void )
{
    m_Model.Clear();
    Lighting::Shutdown();
}

namespace Graphics
{
    extern EnumVar DebugZoom;
}

void ModelViewer::Update( float deltaT )
{
    ScopedTimer _prof(L"Update State");

    if (GameInput::IsFirstPressed(GameInput::kLShoulder))
        DebugZoom.Decrement();
    else if (GameInput::IsFirstPressed(GameInput::kRShoulder))
        DebugZoom.Increment();

    m_CameraController->Update(deltaT);
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

void ModelViewer::RenderObjects( GraphicsContext& gfxContext, const Matrix4& ViewProjMat, eObjectFilter Filter )
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

        gfxContext.SetConstants(4, baseVertex, materialIdx);

        gfxContext.DrawIndexed(indexCount, startIndex, baseVertex);
    }
}

void ModelViewer::RenderLightShadows(GraphicsContext& gfxContext)
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

void ModelViewer::RenderScene( void )
{
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
    auto pfnSetupGraphicsState = [&](void)
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
            ScopedTimer _prof1(L"Opaque", gfxContext);
            gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
            gfxContext.ClearDepth(g_SceneDepthBuffer);

#ifdef _WAVE_OP
            gfxContext.SetPipelineState(EnableWaveOps ? m_DepthWaveOpsPSO : m_DepthPSO );
#else
            gfxContext.SetPipelineState(m_DepthPSO);
#endif
            gfxContext.SetDepthStencilTarget(g_SceneDepthBuffer.GetDSV());
            gfxContext.SetViewportAndScissor(m_MainViewport, m_MainScissor);
            RenderObjects(gfxContext, m_ViewProjMatrix, kOpaque );
        }

        {
            ScopedTimer _prof2(L"Cutout", gfxContext);
            gfxContext.SetPipelineState(m_CutoutDepthPSO);
            RenderObjects(gfxContext, m_ViewProjMatrix, kCutout );
        }
    }

    SSAO::Render(gfxContext, m_Camera);

    Lighting::FillLightGrid(gfxContext, m_Camera);

    if (!SSAO::DebugDraw)
    {
        ScopedTimer _prof(L"Main Render", gfxContext);

        gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
        gfxContext.ClearColor(g_SceneColorBuffer);

        pfnSetupGraphicsState();

        {
            ScopedTimer _prof3(L"Render Shadow Map", gfxContext);

            m_SunShadow.UpdateMatrix(-m_SunDirection, Vector3(0, -500.0f, 0), Vector3(ShadowDimX, ShadowDimY, ShadowDimZ),
                (uint32_t)g_ShadowBuffer.GetWidth(), (uint32_t)g_ShadowBuffer.GetHeight(), 16);

            g_ShadowBuffer.BeginRendering(gfxContext);
            gfxContext.SetPipelineState(m_ShadowPSO);
            RenderObjects(gfxContext, m_SunShadow.GetViewProjMatrix(), kOpaque);
            gfxContext.SetPipelineState(m_CutoutShadowPSO);
            RenderObjects(gfxContext, m_SunShadow.GetViewProjMatrix(), kCutout);
            g_ShadowBuffer.EndRendering(gfxContext);
        }

        if (SSAO::AsyncCompute)
        {
            gfxContext.Flush();
            pfnSetupGraphicsState();

            // Make the 3D queue wait for the Compute queue to finish SSAO
            g_CommandManager.GetGraphicsQueue().StallForProducer(g_CommandManager.GetComputeQueue());
        }

        {
            ScopedTimer _prof4(L"Render Color", gfxContext);

            gfxContext.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            gfxContext.SetDynamicDescriptors(3, 0, _countof(m_ExtraTextures), m_ExtraTextures);
            gfxContext.SetDynamicConstantBufferView(1, sizeof(psConstants), &psConstants);
#ifdef _WAVE_OP
            gfxContext.SetPipelineState(EnableWaveOps ? m_ModelWaveOpsPSO : m_ModelPSO );
#else
            gfxContext.SetPipelineState(ShowWaveTileCounts ? m_WaveTileCountPSO : m_ModelPSO);
#endif
            gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
            gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV(), g_SceneDepthBuffer.GetDSV_DepthReadOnly());
            gfxContext.SetViewportAndScissor(m_MainViewport, m_MainScissor);

            RenderObjects( gfxContext, m_ViewProjMatrix, kOpaque );

            if (!ShowWaveTileCounts)
            {
                gfxContext.SetPipelineState(m_CutoutModelPSO);
                RenderObjects( gfxContext, m_ViewProjMatrix, kCutout );
            }
        }

    }

    // Some systems generate a per-pixel velocity buffer to better track dynamic and skinned meshes.  Everything
    // is static in our scene, so we generate velocity from camera motion and the depth buffer.  A velocity buffer
    // is necessary for all temporal effects (and motion blur).
    MotionBlur::GenerateCameraVelocityBuffer(gfxContext, m_Camera, true);

    TemporalEffects::ResolveImage(gfxContext);

    ParticleEffects::Render(gfxContext, m_Camera, g_SceneColorBuffer, g_SceneDepthBuffer,  g_LinearDepth[FrameIndex]);

    // Until I work out how to couple these two, it's "either-or".
    if (DepthOfField::Enable)
        DepthOfField::Render(gfxContext, m_Camera.GetNearClip(), m_Camera.GetFarClip());
    else
        MotionBlur::RenderObjectBlur(gfxContext, g_VelocityBuffer);

    gfxContext.Finish();
}

void ModelViewer::CreateParticleEffects()
{
    ParticleEffectProperties Effect = ParticleEffectProperties();
    Effect.MinStartColor = Effect.MaxStartColor = Effect.MinEndColor = Effect.MaxEndColor = Color(1.0f, 1.0f, 1.0f, 0.0f);
    Effect.TexturePath = L"sparkTex.dds";

    Effect.TotalActiveLifetime = FLT_MAX;
    Effect.Size = Vector4(4.0f, 8.0f, 4.0f, 8.0f);
    Effect.Velocity = Vector4(20.0f, 200.0f, 50.0f, 180.0f);
    Effect.LifeMinMax = XMFLOAT2(1.0f, 3.0f);
    Effect.MassMinMax = XMFLOAT2(4.5f, 15.0f);
    Effect.EmitProperties.Gravity = XMFLOAT3(0.0f, -100.0f, 0.0f);
    Effect.EmitProperties.FloorHeight = -0.5f;
    Effect.EmitProperties.EmitPosW = Effect.EmitProperties.LastEmitPosW = XMFLOAT3(-1200.0f, 185.0f, -445.0f);
    Effect.EmitProperties.MaxParticles = 800;
    Effect.EmitRate = 64.0f;
    Effect.Spread.x = 20.0f;
    Effect.Spread.y = 50.0f;
    ParticleEffects::InstantiateEffect( Effect );

    ParticleEffectProperties Smoke = ParticleEffectProperties();
    Smoke.TexturePath = L"smoke.dds";

    Smoke.TotalActiveLifetime = FLT_MAX;
    Smoke.EmitProperties.MaxParticles = 25;
    Smoke.EmitProperties.EmitPosW = Smoke.EmitProperties.LastEmitPosW = XMFLOAT3(1120.0f, 185.0f, -445.0f);
    Smoke.EmitRate = 64.0f;
    Smoke.LifeMinMax = XMFLOAT2(2.5f, 4.0f);
    Smoke.Size = Vector4(60.0f, 108.0f, 30.0f, 208.0f);
    Smoke.Velocity = Vector4(30.0f, 30.0f, 10.0f, 40.0f);
    Smoke.MassMinMax = XMFLOAT2(1.0, 3.5);
    Smoke.Spread.x = 60.0f;
    Smoke.Spread.y = 70.0f;
    Smoke.Spread.z = 20.0f;
    ParticleEffects::InstantiateEffect( Smoke );

    ParticleEffectProperties Fire = ParticleEffectProperties();
    Fire.MinStartColor = Fire.MaxStartColor = Fire.MinEndColor = Fire.MaxEndColor = Color(8.0f, 8.0f, 8.0f, 0.0f);
    Fire.TexturePath = L"fire.dds";

    Fire.TotalActiveLifetime = FLT_MAX;
    Fire.Size = Vector4(54.0f, 68.0f, 0.1f, 0.3f);
    Fire.Velocity = Vector4 (10.0f, 30.0f, 50.0f, 50.0f);
    Fire.LifeMinMax = XMFLOAT2(1.0f, 3.0f);
    Fire.MassMinMax = XMFLOAT2(10.5f, 14.0f);
    Fire.EmitProperties.Gravity = XMFLOAT3(0.0f, 1.0f, 0.0f);
    Fire.EmitProperties.EmitPosW = Fire.EmitProperties.LastEmitPosW = XMFLOAT3(1120.0f, 125.0f, 405.0f);
    Fire.EmitProperties.MaxParticles = 25;
    Fire.EmitRate = 64.0f;
    Fire.Spread.x = 1.0f;
    Fire.Spread.y = 60.0f;
    ParticleEffects::InstantiateEffect( Fire );
}
