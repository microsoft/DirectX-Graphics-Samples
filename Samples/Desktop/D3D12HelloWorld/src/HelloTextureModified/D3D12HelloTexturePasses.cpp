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

#include "D3D12HelloTexture.h"

void HelloTextureEngine::BuildRenderPasses()
{
    m_renderGraphRuntime.Graph().Clear();
    m_renderGraphRuntime.Operations().Clear();

    AddPass(MakeClearPass());

    if (m_sceneResourcesAvailable)
    {
        AddPass(MakeDepthPrePass());
        AddSceneRenderPasses();
        AddPass(MakeToneMapPass());

        if (m_debugViewSettings.requestHdrDump)
        {
            AddPass(MakeDebugDumpPass());
        }
    }

    AddPass(MakeImGuiPass());
}

void HelloTextureEngine::AddSceneRenderPasses()
{
    if (m_renderingPath == RenderingPath::Forward)
    {
        AddPass(MakeForwardPass());
    }
    else
    {
        AddPass(MakeGBufferPass());
        AddDeferredSceneOutputPass();
    }
}

void HelloTextureEngine::AddDeferredSceneOutputPass()
{
    if (m_debugViewSettings.IsGBufferDebugView())
    {
        AddPass(MakeGBufferDebugPass());
    }
    else if (m_lightingPassDebugGradientEnabled)
    {
        AddPass(MakeLightingDebugGradientPass());
    }
    else
    {
        AddPass(MakeLightingPass());
    }
}

void HelloTextureEngine::AddPass(RenderPass pass)
{
    m_renderGraphRuntime.Graph().Add(std::move(pass));
}

void HelloTextureEngine::ValidateRenderPassGraph() const
{
    Engine::ValidateRenderPassGraph(
        m_renderGraphRuntime.Graph().Passes(),
        Engine::RenderPassGraphValidationContext<PassOperationHandler>{&m_renderGraphRuntime.Pipelines(),
                                                                       &m_renderGraphRuntime.Bindings(),
                                                                       &m_renderGraphRuntime.Operations(),
                                                                       &m_renderGraphRuntime.Constants()});
}

auto HelloTextureEngine::MakeResourceUsages(std::initializer_list<ResourceUsage> usages) const -> ResourceUsages
{
    return ResourceUsages(usages);
}

auto HelloTextureEngine::MakeGBufferReadUsages() const -> ResourceUsages
{
    return MakeResourceUsages(
        {{kGBufferResourceNames[Engine::GBuffer::Albedo], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
         {kGBufferResourceNames[Engine::GBuffer::Normal], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
         {kGBufferResourceNames[Engine::GBuffer::Material], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
         {kGBufferResourceNames[Engine::GBuffer::MotionVector], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
         {kGBufferResourceNames[Engine::GBuffer::PBRParams], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
         {kDepthStencilResourceName, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE}});
}

PipelineKey HelloTextureEngine::PipelineId(const std::string& name)
{
    return m_renderGraphRuntime.RegisterPipeline(name);
}

DescriptorKey HelloTextureEngine::DescriptorId(const std::string& name)
{
    return m_renderGraphRuntime.RegisterDescriptor(name);
}

auto HelloTextureEngine::MakeClearPass() -> RenderPass
{
    return m_renderGraphRuntime.Authoring()
        .CreatePass(L"Clear")
        .Writes({{kBackBufferResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET},
                 {kDepthStencilResourceName, D3D12_RESOURCE_STATE_DEPTH_WRITE}})
        .Rtv(RtvName::BackBuffer)
        .Dsv(DsvName::Depth)
        .ClearColor(m_backBufferClearColor)
        .Operation(Op::Clear, &HelloTextureEngine::ExecuteClearPass)
        .Build();
}

auto HelloTextureEngine::MakeDepthPrePass() -> RenderPass
{
    return m_renderGraphRuntime.Authoring()
        .CreatePass(L"Depth PrePass")
        .Pipeline(Pipe::DepthPrePass)
        .Writes({{kDepthStencilResourceName, D3D12_RESOURCE_STATE_DEPTH_WRITE}})
        .Descriptor(RootSignatureLayout::InstanceSrv, Desc::InstanceBufferSrv)
        .Descriptor(RootSignatureLayout::CameraConstants, Desc::CameraCbv)
        .Dsv(DsvName::Depth)
        .Operation(Op::DepthPrePass, &HelloTextureEngine::ExecuteDepthPrePass)
        .Build();
}

auto HelloTextureEngine::MakeGBufferPass() -> RenderPass
{
    return m_renderGraphRuntime.Authoring()
        .CreatePass(L"GBufferPass")
        .Pipeline(Pipe::GBuffer)
        .Reads({{kDepthStencilResourceName, D3D12_RESOURCE_STATE_DEPTH_WRITE}})
        .Writes({{kGBufferResourceNames[Engine::GBuffer::Albedo], D3D12_RESOURCE_STATE_RENDER_TARGET},
                 {kGBufferResourceNames[Engine::GBuffer::Normal], D3D12_RESOURCE_STATE_RENDER_TARGET},
                 {kGBufferResourceNames[Engine::GBuffer::Material], D3D12_RESOURCE_STATE_RENDER_TARGET},
                 {kGBufferResourceNames[Engine::GBuffer::MotionVector], D3D12_RESOURCE_STATE_RENDER_TARGET},
                 {kGBufferResourceNames[Engine::GBuffer::PBRParams], D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Descriptor(RootSignatureLayout::TextureTable, Desc::TextureTable)
        .Descriptor(RootSignatureLayout::InstanceSrv, Desc::InstanceBufferSrv)
        .Descriptor(RootSignatureLayout::MaterialSrv, Desc::MaterialBufferSrv)
        .Descriptor(RootSignatureLayout::CameraConstants, Desc::CameraCbv)
        .Rtvs({RtvName::GBufferAlbedo,
               RtvName::GBufferNormal,
               RtvName::GBufferMaterial,
               RtvName::GBufferMotionVector,
               RtvName::GBufferPBRParams})
        .Dsv(DsvName::Depth)
        .Operation(Op::GBuffer, &HelloTextureEngine::ExecuteGBufferPass)
        .Build();
}

auto HelloTextureEngine::MakeForwardPass() -> RenderPass
{
    return m_renderGraphRuntime.Authoring()
        .CreatePass(L"ForwardPass")
        .Pipeline(Pipe::Forward)
        .Reads({{kDepthStencilResourceName, D3D12_RESOURCE_STATE_DEPTH_WRITE}})
        .Writes({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Descriptor(RootSignatureLayout::TextureTable, Desc::TextureTable)
        .Descriptor(RootSignatureLayout::InstanceSrv, Desc::InstanceBufferSrv)
        .Descriptor(RootSignatureLayout::MaterialSrv, Desc::MaterialBufferSrv)
        .Descriptor(RootSignatureLayout::CameraConstants, Desc::CameraCbv)
        .Descriptor(RootSignatureLayout::LightConstants, Desc::LightCbv)
        .Rtv(RtvName::LightPass)
        .Dsv(DsvName::Depth)
        .ClearColor({0.0f, 0.0f, 0.0f, 1.0f})
        .Operation(Op::Forward, &HelloTextureEngine::ExecuteForwardPass)
        .Build();
}

auto HelloTextureEngine::MakeLightingPass() -> RenderPass
{
    return m_renderGraphRuntime.Authoring()
        .CreatePass(L"LightPass")
        .Pipeline(Pipe::Lighting)
        .Reads(MakeGBufferReadUsages())
        .Writes({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Descriptor(RootSignatureLayout::GBufferSrvBase, Desc::GBufferAlbedoSrv)
        .Descriptor(RootSignatureLayout::MaterialSrv, Desc::MaterialBufferSrv)
        .Descriptor(RootSignatureLayout::EnvironmentMap, Desc::EnvironmentMapSrv)
        .Descriptor(RootSignatureLayout::CameraConstants, Desc::CameraCbv)
        .Descriptor(RootSignatureLayout::LightConstants, Desc::LightCbv)
        .Rtv(RtvName::LightPass)
        .Operation(Op::Lighting, &HelloTextureEngine::ExecuteLightingPass)
        .Build();
}

auto HelloTextureEngine::MakeLightingDebugGradientPass() -> RenderPass
{
    return m_renderGraphRuntime.Authoring()
        .CreatePass(L"LightPassDebugGradient")
        .Pipeline(Pipe::LightingDebugGradient)
        .Reads(MakeGBufferReadUsages())
        .Writes({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Descriptor(RootSignatureLayout::GBufferSrvBase, Desc::GBufferAlbedoSrv)
        .Descriptor(RootSignatureLayout::MaterialSrv, Desc::MaterialBufferSrv)
        .Descriptor(RootSignatureLayout::CameraConstants, Desc::CameraCbv)
        .Descriptor(RootSignatureLayout::LightConstants, Desc::LightCbv)
        .Rtv(RtvName::LightPass)
        .Operation(Op::LightingDebugGradient, &HelloTextureEngine::ExecuteLightingDebugGradientPass)
        .Constants(RootSignatureLayout::ToneMapConstants, ConstName::ToneMap)
        .Build();
}

auto HelloTextureEngine::MakeToneMapPass() -> RenderPass
{
    return m_renderGraphRuntime.Authoring()
        .CreatePass(L"ToneMapPass")
        .Pipeline(Pipe::ToneMap)
        .Reads({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE}})
        .Writes({{kBackBufferResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Descriptor(RootSignatureLayout::ToneMapSceneColor, Desc::ToneMapSceneColorSrv)
        .Rtv(RtvName::BackBuffer)
        .Operation(Op::ToneMap, &HelloTextureEngine::ExecuteToneMapPass)
        .Constants(RootSignatureLayout::ToneMapConstants, ConstName::ToneMap)
        .Build();
}

auto HelloTextureEngine::MakeDebugDumpPass() -> RenderPass
{
    return m_renderGraphRuntime.Authoring()
        .CreatePass(L"DebugDump")
        .Reads({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_COPY_SOURCE},
                {kBackBufferResourceName, D3D12_RESOURCE_STATE_COPY_SOURCE}})
        .Operation(Op::DebugDump, &HelloTextureEngine::ExecuteDebugDumpPass)
        .Build();
}

auto HelloTextureEngine::MakeGBufferDebugPass() -> RenderPass
{
    return m_renderGraphRuntime.Authoring()
        .CreatePass(L"GBufferDebugPass")
        .Pipeline(Pipe::GBufferDebug)
        .Reads(MakeGBufferReadUsages())
        .Writes({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Descriptor(RootSignatureLayout::GBufferSrvBase, Desc::GBufferAlbedoSrv)
        .Rtv(RtvName::LightPass)
        .Operation(Op::GBufferDebug, &HelloTextureEngine::ExecuteGBufferDebugPass)
        .Constants(RootSignatureLayout::GBufferDebugConstants, ConstName::GBufferDebugTarget)
        .Build();
}

auto HelloTextureEngine::MakeImGuiPass() -> RenderPass
{
    return m_renderGraphRuntime.Authoring()
        .CreatePass(L"ImGui")
        .Writes({{kBackBufferResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Rtv(RtvName::BackBuffer)
        .Operation(Op::ImGui, &HelloTextureEngine::ExecuteImGuiPass)
        .Build();
}
