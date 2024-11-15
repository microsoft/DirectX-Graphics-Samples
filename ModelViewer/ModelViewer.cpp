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
// Author:  James Stanard
//

#include "GameCore.h"
#include "CameraController.h"
#include "BufferManager.h"
#include "Camera.h"
#include "CommandContext.h"
#include "TemporalEffects.h"
#include "MotionBlur.h"
#include "DepthOfField.h"
#include "PostEffects.h"
#include "SSAO.h"
#include "FXAA.h"
#include "SystemTime.h"
#include "TextRenderer.h"
#include "ParticleEffectManager.h"
#include "GameInput.h"
#include "SponzaRenderer.h"
#include "glTF.h"
#include "Renderer.h"
#include "Model.h"
#include "ModelLoader.h"
#include "ShadowCamera.h"
#include "Display.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "SDFGI.h"

//#define LEGACY_RENDERER

using namespace GameCore;
using namespace Math;
using namespace Graphics;
using namespace std;

using Renderer::MeshSorter;

namespace GameCore
{
    extern HWND g_hWnd;
}

namespace Graphics
{
    extern ID3D12Device* g_Device;
}

class ModelViewer : public GameCore::IGameApp
{
public:

    ModelViewer( void ) {}

    virtual void Startup( void ) override;
    virtual void Cleanup( void ) override;

    virtual void Update( float deltaT ) override;
    virtual void RenderScene( void ) override;
    

    virtual void RenderUI( class GraphicsContext& ) override;

    void InitializeGUI();

    GlobalConstants UpdateGlobalConstants(const Math::Camera& cam);
    void NonLegacyRenderScene(GraphicsContext& gfxContext, const Math::Camera& cam, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor, bool renderShadows = true);

private:

    Camera m_Camera;
    unique_ptr<CameraController> m_CameraController;

    D3D12_VIEWPORT m_MainViewport;
    D3D12_RECT m_MainScissor;

    ModelInstance m_ModelInst;
    ShadowCamera m_SunShadowCamera;

    SDFGI::SDFGIManager *mp_SDFGIManager;
};

CREATE_APPLICATION( ModelViewer )

ExpVar g_SunLightIntensity("Viewer/Lighting/Sun Light Intensity", 4.0f, 0.0f, 16.0f, 0.1f);
NumVar g_SunOrientation("Viewer/Lighting/Sun Orientation", -0.5f, -100.0f, 100.0f, 0.1f );
NumVar g_SunInclination("Viewer/Lighting/Sun Inclination", 0.75f, 0.0f, 1.0f, 0.01f );

void ChangeIBLSet(EngineVar::ActionType);
void ChangeIBLBias(EngineVar::ActionType);

DynamicEnumVar g_IBLSet("Viewer/Lighting/Environment", ChangeIBLSet);
std::vector<std::pair<TextureRef, TextureRef>> g_IBLTextures;
NumVar g_IBLBias("Viewer/Lighting/Gloss Reduction", 2.0f, 0.0f, 10.0f, 1.0f, ChangeIBLBias);

void ChangeIBLSet(EngineVar::ActionType)
{
    int setIdx = g_IBLSet - 1;
    if (setIdx < 0)
    {
        Renderer::SetIBLTextures(nullptr, nullptr);
    }
    else
    {
        auto texturePair = g_IBLTextures[setIdx];
        Renderer::SetIBLTextures(texturePair.first, texturePair.second);
    }
}

void ChangeIBLBias(EngineVar::ActionType)
{
    Renderer::SetIBLBias(g_IBLBias);
}

#include <direct.h> // for _getcwd() to check data root path

void LoadIBLTextures()
{
    char CWD[256];
    _getcwd(CWD, 256);

    Utility::Printf("Loading IBL environment maps\n");

    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile(L"Textures/*_diffuseIBL.dds", &ffd);

    g_IBLSet.AddEnum(L"None");

    if (hFind != INVALID_HANDLE_VALUE) do
    {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;

       std::wstring diffuseFile = ffd.cFileName;
       std::wstring baseFile = diffuseFile; 
       baseFile.resize(baseFile.rfind(L"_diffuseIBL.dds"));
       std::wstring specularFile = baseFile + L"_specularIBL.dds";

       TextureRef diffuseTex = TextureManager::LoadDDSFromFile(L"Textures/" + diffuseFile);
       if (diffuseTex.IsValid())
       {
           TextureRef specularTex = TextureManager::LoadDDSFromFile(L"Textures/" + specularFile);
           if (specularTex.IsValid())
           {
               g_IBLSet.AddEnum(baseFile);
               g_IBLTextures.push_back(std::make_pair(diffuseTex, specularTex));
           }
       }
    }
    while (FindNextFile(hFind, &ffd) != 0);

    FindClose(hFind);

    Utility::Printf("Found %u IBL environment map sets\n", g_IBLTextures.size());

    if (g_IBLTextures.size() > 0)
        g_IBLSet.Increment();
}

void ModelViewer::Startup( void )
{
    MotionBlur::Enable = false;
    TemporalEffects::EnableTAA = false;
    FXAA::Enable = false;
    PostEffects::EnableHDR = false;
    PostEffects::EnableAdaptation = false;
    SSAO::Enable = false;

    Renderer::Initialize();

    LoadIBLTextures();

    std::wstring gltfFileName;

    float scaleModel = 1.0f;

    bool forceRebuild = false;
    uint32_t rebuildValue;
    if (CommandLineArgs::GetInteger(L"rebuild", rebuildValue))
        forceRebuild = rebuildValue != 0;

    if (CommandLineArgs::GetString(L"model", gltfFileName) == false)
    {
#ifdef LEGACY_RENDERER
        Sponza::Startup(m_Camera);
#else
        scaleModel = 100.0f;
        m_ModelInst = Renderer::LoadModel(L"Sponza/PBR/sponza2.gltf", forceRebuild);
        //m_ModelInst = Renderer::LoadModel(L"Models/BoxAndPlane/BoxAndPlane.gltf", forceRebuild);
        //m_ModelInst = Renderer::LoadModel(L"Models/CornellWithSonic/CornellWithSonic.gltf", forceRebuild);
        m_ModelInst.Resize(scaleModel * m_ModelInst.GetRadius());
        OrientedBox obb = m_ModelInst.GetBoundingBox();
        float modelRadius = Length(obb.GetDimensions()) * 0.5f;
        const Vector3 eye = obb.GetCenter() + Vector3(modelRadius * 0.5f, 0.0f, 0.0f);
        m_Camera.SetEyeAtUp( eye, Vector3(kZero), Vector3(kYUnitVector) );
#endif
    }
    else
    {
        scaleModel = 10.0f;
        m_ModelInst = Renderer::LoadModel(gltfFileName, forceRebuild);
        m_ModelInst.LoopAllAnimations();
        m_ModelInst.Resize(scaleModel* m_ModelInst.GetRadius());

        MotionBlur::Enable = false;
    }

    m_Camera.SetZRange(1.0f, 10000.0f);
    if (gltfFileName.size() == 0)
        m_CameraController.reset(new FlyingFPSCamera(m_Camera, Vector3(kYUnitVector)));
    else
        m_CameraController.reset(new OrbitCamera(m_Camera, m_ModelInst.GetBoundingSphere(), Vector3(kYUnitVector)));
#if UI_ENABLE
    InitializeGUI();
#endif

    #ifdef LEGACY_RENDERER
    const Math::AxisAlignedBox &sceneBounds = Sponza::GetBoundingBox();
    #else
    Math::AxisAlignedBox& test = m_ModelInst.GetAxisAlignedBox();
    Math::AxisAlignedBox& sceneBounds = Math::AxisAlignedBox();
    sceneBounds.AddPoint(100.0f * Vector3(test.GetMin()));
    sceneBounds.AddPoint(100.0f * Vector3(test.GetMax()));
    #endif

    auto renderLambda = [&](GraphicsContext& ctx, const Math::Camera& cam, const D3D12_VIEWPORT& vp, const D3D12_RECT& sc, bool renderShadows) {
        ModelViewer::NonLegacyRenderScene(ctx, cam, vp, sc, renderShadows);
    };

    mp_SDFGIManager = new SDFGI::SDFGIManager(sceneBounds, static_cast<std::function<void(GraphicsContext&, const Math::Camera&, const D3D12_VIEWPORT&, const D3D12_RECT&, bool)>>(renderLambda));
}

void ModelViewer::InitializeGUI() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui_ImplWin32_Init(GameCore::g_hWnd);
    ImGui_ImplDX12_Init(
        Graphics::g_Device,
        // Number of frames in flight.
        3,
        Graphics::g_OverlayBuffer.GetFormat(), 
        // imgui needs SRV descriptors for its font textures.
        Renderer::s_TextureHeap.GetHeapPointer(),
        D3D12_CPU_DESCRIPTOR_HANDLE(Renderer::s_TextureHeap[0]),
        D3D12_GPU_DESCRIPTOR_HANDLE(Renderer::s_TextureHeap[0])
    );
}

void ModelViewer::Cleanup( void )
{
    m_ModelInst = nullptr;

    g_IBLTextures.clear();

    delete mp_SDFGIManager;

#ifdef LEGACY_RENDERER
    Sponza::Cleanup();
#endif

    Renderer::Shutdown();
#if UI_ENABLE
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
#endif
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

#if UI_ENABLE
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {
        // Update camera only if imgui captures the mouse
        m_CameraController->Update(deltaT);
    }
#else 
    m_CameraController->Update(deltaT);
#endif

    GraphicsContext& gfxContext = GraphicsContext::Begin(L"Scene Update");

    m_ModelInst.Update(gfxContext, deltaT);

    gfxContext.Finish();

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

GlobalConstants ModelViewer::UpdateGlobalConstants(const Math::Camera& cam)
{
    GlobalConstants globals;
    globals.ViewProjMatrix = cam.GetViewProjMatrix();
    globals.CameraPos = cam.GetPosition();
    globals.SunIntensity = Vector3(Scalar(g_SunLightIntensity));

    // Handle shadow-related global constants
    {
        // Calculate sun direction and shadow matrix
        float costheta = cosf(g_SunOrientation);
        float sintheta = sinf(g_SunOrientation);
        float cosphi = cosf(g_SunInclination * 3.14159f * 0.5f);
        float sinphi = sinf(g_SunInclination * 3.14159f * 0.5f);

        Vector3 SunDirection = Normalize(Vector3(costheta * cosphi, sinphi, sintheta * cosphi));
        Vector3 ShadowBounds = Vector3(m_ModelInst.GetRadius());
        Vector3 origin = Vector3(0);
        Vector3 ShadowCenter = origin;

        OrientedBox obb = m_ModelInst.GetBoundingBox();
        float x = obb.GetDimensions().GetX();
        float y = obb.GetDimensions().GetY();
        float z = obb.GetDimensions().GetZ();

        m_SunShadowCamera.UpdateMatrix(-SunDirection, Vector3(0, -m_ModelInst.GetCenter().GetY(), 0), obb.GetDimensions() * 1.5f,
            (uint32_t)g_ShadowBuffer.GetWidth(), (uint32_t)g_ShadowBuffer.GetHeight(), 16);


        // Update sun/shadow global constants
        globals.SunShadowMatrix = m_SunShadowCamera.GetShadowMatrix();
        globals.SunDirection = SunDirection;
    }

    return globals;
}

void ModelViewer::NonLegacyRenderScene(GraphicsContext& gfxContext, const Math::Camera& cam, 
    const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor, bool renderShadows)
{
    GlobalConstants globals = UpdateGlobalConstants(cam);

    // Begin rendering depth
    gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
    gfxContext.ClearDepth(g_SceneDepthBuffer);

    MeshSorter mainSorter(MeshSorter::kDefault);
    mainSorter.SetCamera(cam);
    mainSorter.SetViewport(viewport);
    mainSorter.SetScissor(scissor);
    mainSorter.SetDepthStencilTarget(g_SceneDepthBuffer);
    mainSorter.AddRenderTarget(g_SceneColorBuffer);

    m_ModelInst.Render(mainSorter);

    mainSorter.Sort();

#if ENABLE_DEPTH_PREPASS == 1
    {
        ScopedTimer _prof(L"Depth Pre-Pass", gfxContext);
        sorter.RenderMeshes(MeshSorter::kZPass, gfxContext, globals);
    }
#endif

    if (SSAO::Enable) {
        SSAO::Render(gfxContext, cam);
    }

    if (!SSAO::DebugDraw)
    {
        if (renderShadows) {
            ScopedTimer _outerprof(L"Main Render", gfxContext);

            {
                ScopedTimer _prof(L"Sun Shadow Map", gfxContext);

                MeshSorter shadowSorter(MeshSorter::kShadows);
                shadowSorter.SetCamera(m_SunShadowCamera);
                shadowSorter.SetDepthStencilTarget(g_ShadowBuffer);

                m_ModelInst.Render(shadowSorter);

                shadowSorter.Sort();
                shadowSorter.RenderMeshes(MeshSorter::kZPass, gfxContext, globals);
            }
        }

        gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
        gfxContext.ClearColor(g_SceneColorBuffer);

        {
            ScopedTimer _prof(L"Render Color", gfxContext);

            gfxContext.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
            gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV(), g_SceneDepthBuffer.GetDSV_DepthReadOnly());
            gfxContext.SetViewportAndScissor(viewport, scissor);

            mainSorter.RenderMeshes(MeshSorter::kOpaque, gfxContext, globals);
        }

        Renderer::DrawSkybox(gfxContext, cam, viewport, scissor);

        mainSorter.RenderMeshes(MeshSorter::kTransparent, gfxContext, globals);
    }
}

void ModelViewer::RenderScene( void )
{
    GraphicsContext& gfxContext = GraphicsContext::Begin(L"Scene Render");

    uint32_t FrameIndex = TemporalEffects::GetFrameIndexMod2();
    const D3D12_VIEWPORT& viewport = m_MainViewport;
    const D3D12_RECT& scissor = m_MainScissor;

    ParticleEffectManager::Update(gfxContext.GetComputeContext(), Graphics::GetFrameTime());

    if (m_ModelInst.IsNull())
    {
#ifdef LEGACY_RENDERER
        Sponza::RenderScene(gfxContext, m_Camera, viewport, scissor);
#endif
    }
    else
    {
        NonLegacyRenderScene(gfxContext, m_Camera, viewport, scissor);
    }

    // TODO: needs to be done before rendering the scene so that probes can be sampled.
    mp_SDFGIManager->Render(gfxContext, m_Camera, viewport, scissor);

#if MAIN_SUN_SHADOW_BUFFER_VIS == 1  //all main macros in pch.h
    Renderer::DrawShadowBuffer(gfxContext, viewport, scissor);
#endif

    // Commented Out Unnecessary MiniEngine Features
    //   E.g. MotionBlur, Particle Effects, etc.
    /*
    
    {
        // Some systems generate a per-pixel velocity buffer to better track dynamic and skinned meshes.  Everything
        // is static in our scene, so we generate velocity from camera motion and the depth buffer.  A velocity buffer
        // is necessary for all temporal effects (and motion blur).
        MotionBlur::GenerateCameraVelocityBuffer(gfxContext, m_Camera, true);

        TemporalEffects::ResolveImage(gfxContext);

        ParticleEffectManager::Render(gfxContext, m_Camera, g_SceneColorBuffer, g_SceneDepthBuffer,  g_LinearDepth[FrameIndex]);

        // Until I work out how to couple these two, it's "either-or".
        if (DepthOfField::Enable)
            DepthOfField::Render(gfxContext, m_Camera.GetNearClip(), m_Camera.GetFarClip());
        else
            MotionBlur::RenderObjectBlur(gfxContext, g_VelocityBuffer);
    }
    */

    gfxContext.Finish();
}

void ModelViewer::RenderUI( class GraphicsContext& gfxContext ) {
#if UI_ENABLE
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), gfxContext.GetCommandList());
#endif
}