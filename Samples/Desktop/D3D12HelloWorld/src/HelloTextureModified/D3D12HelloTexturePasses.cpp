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

void D3D12HelloTexture::BuildRenderPasses()
{
    m_renderGraphRuntime.Graph().Clear();
    m_renderGraphRuntime.Operations().Clear();

    AddPass(MakeClearPass());
    AddPass(MakeDepthPrePass());
    AddSceneRenderPasses();
    AddPass(MakeToneMapPass());

    if (m_debugViewSettings.requestHdrDump)
    {
        AddPass(MakeDebugDumpPass());
    }

    AddPass(MakeImGuiPass());
}

void D3D12HelloTexture::AddSceneRenderPasses()
{
    if (m_renderingPath == RenderingPath::Forward)
    {
        AddPass(MakeMainPass());
    }
    else
    {
        AddPass(MakeGBufferPass());
        AddDeferredSceneOutputPass();
    }
}

void D3D12HelloTexture::AddDeferredSceneOutputPass()
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

void D3D12HelloTexture::AddPass(RenderPass pass)
{
    m_renderGraphRuntime.Graph().Add(std::move(pass));
}

void D3D12HelloTexture::ValidateRenderPassGraph() const
{
    Engine::ValidateRenderPassGraph(
        m_renderGraphRuntime.Graph().Passes(),
        Engine::RenderPassGraphValidationContext<PassOperationHandler>{
            &m_renderGraphRuntime.Pipelines(),
            &m_renderGraphRuntime.BindingResolvers(),
            &m_renderGraphRuntime.Operations(),
            &m_renderGraphRuntime.Constants()});
}

auto D3D12HelloTexture::MakeResourceUsages(std::initializer_list<ResourceUsage> usages) const -> ResourceUsages
{
    return ResourceUsages(usages);
}

auto D3D12HelloTexture::MakeGBufferReadUsages() const -> ResourceUsages
{
    return MakeResourceUsages(
        {{kGBufferResourceNames[GBuffer::Albedo], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
         {kGBufferResourceNames[GBuffer::Normal], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
         {kGBufferResourceNames[GBuffer::Material], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
         {kGBufferResourceNames[GBuffer::MotionVector], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
         {kGBufferResourceNames[GBuffer::PBRParams], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE},
         {kDepthStencilResourceName, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE}});
}

PipelineKey D3D12HelloTexture::PipelineId(const std::string& name)
{
    return m_renderGraphRuntime.Keys().RegisterPipeline(name, m_renderGraphRuntime.KeyRegistry());
}

DescriptorKey D3D12HelloTexture::DescriptorId(const std::string& name)
{
    return m_renderGraphRuntime.Keys().RegisterDescriptor(name, m_renderGraphRuntime.KeyRegistry());
}

auto D3D12HelloTexture::MakeClearPass() -> RenderPass
{
    return m_renderGraphRuntime.Authoring().CreatePass(L"Clear")
        .Writes({{kBackBufferResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET},
                 {kDepthStencilResourceName, D3D12_RESOURCE_STATE_DEPTH_WRITE}})
        .Rtv(RtvName::BackBuffer)
        .Dsv(DsvName::Depth)
        .ClearColor(m_backBufferClearColor)
        .Operation(Op::Clear, &D3D12HelloTexture::ExecuteClearPass)
        .Build();
}

auto D3D12HelloTexture::MakeDepthPrePass() -> RenderPass
{
    return m_renderGraphRuntime.Authoring().CreatePass(L"Depth PrePass")
        .Pipeline(Pipe::DepthPrePass)
        .Writes({{kDepthStencilResourceName, D3D12_RESOURCE_STATE_DEPTH_WRITE}})
        .Descriptor(RootParam_InstanceSrv, Desc::InstanceBufferSrv)
        .Descriptor(RootParam_ConstantBuffer, Desc::CameraCbv)
        .Dsv(DsvName::Depth)
        .Operation(Op::DepthPrePass, &D3D12HelloTexture::ExecuteDepthPrePass)
        .Build();
}

auto D3D12HelloTexture::MakeGBufferPass() -> RenderPass
{
    return m_renderGraphRuntime.Authoring().CreatePass(L"GBufferPass")
        .Pipeline(Pipe::GBuffer)
        .Reads({{kDepthStencilResourceName, D3D12_RESOURCE_STATE_DEPTH_WRITE}})
        .Writes({{kGBufferResourceNames[GBuffer::Albedo], D3D12_RESOURCE_STATE_RENDER_TARGET},
                 {kGBufferResourceNames[GBuffer::Normal], D3D12_RESOURCE_STATE_RENDER_TARGET},
                 {kGBufferResourceNames[GBuffer::Material], D3D12_RESOURCE_STATE_RENDER_TARGET},
                 {kGBufferResourceNames[GBuffer::MotionVector], D3D12_RESOURCE_STATE_RENDER_TARGET},
                 {kGBufferResourceNames[GBuffer::PBRParams], D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Descriptor(RootParam_TextureTable, Desc::TextureTable)
        .Descriptor(RootParam_InstanceSrv, Desc::InstanceBufferSrv)
        .Descriptor(RootParam_MaterialSrv, Desc::MaterialBufferSrv)
        .Descriptor(RootParam_ConstantBuffer, Desc::CameraCbv)
        .Rtvs({RtvName::GBufferAlbedo, RtvName::GBufferNormal, RtvName::GBufferMaterial,
               RtvName::GBufferMotionVector, RtvName::GBufferPBRParams})
        .Dsv(DsvName::Depth)
        .Operation(Op::GBuffer, &D3D12HelloTexture::ExecuteGBufferPass)
        .Build();
}

auto D3D12HelloTexture::MakeMainPass() -> RenderPass
{
    return m_renderGraphRuntime.Authoring().CreatePass(L"MainPass")
        .Pipeline(Pipe::Main)
        .Reads({{kDepthStencilResourceName, D3D12_RESOURCE_STATE_DEPTH_WRITE}})
        .Writes({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Descriptor(RootParam_TextureTable, Desc::TextureTable)
        .Descriptor(RootParam_InstanceSrv, Desc::InstanceBufferSrv)
        .Descriptor(RootParam_MaterialSrv, Desc::MaterialBufferSrv)
        .Descriptor(RootParam_ConstantBuffer, Desc::CameraCbv)
        .Descriptor(RootParam_LightConstants, Desc::LightCbv)
        .Rtv(RtvName::LightPass)
        .Dsv(DsvName::Depth)
        .ClearColor({0.0f, 0.0f, 0.0f, 1.0f})
        .Operation(Op::Main, &D3D12HelloTexture::ExecuteMainPass)
        .Build();
}

auto D3D12HelloTexture::MakeLightingPass() -> RenderPass
{
    return m_renderGraphRuntime.Authoring().CreatePass(L"LightPass")
        .Pipeline(Pipe::Lighting)
        .Reads(MakeGBufferReadUsages())
        .Writes({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Descriptor(RootParam_GBufferSrvBase, Desc::GBufferAlbedoSrv)
        .Descriptor(RootParam_MaterialSrv, Desc::MaterialBufferSrv)
        .Descriptor(RootParam_ConstantBuffer, Desc::CameraCbv)
        .Descriptor(RootParam_LightConstants, Desc::LightCbv)
        .Rtv(RtvName::LightPass)
        .Operation(Op::Lighting, &D3D12HelloTexture::ExecuteLightingPass)
        .Build();
}

auto D3D12HelloTexture::MakeLightingDebugGradientPass() -> RenderPass
{
    return m_renderGraphRuntime.Authoring().CreatePass(L"LightPassDebugGradient")
        .Pipeline(Pipe::LightingDebugGradient)
        .Reads(MakeGBufferReadUsages())
        .Writes({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Descriptor(RootParam_GBufferSrvBase, Desc::GBufferAlbedoSrv)
        .Descriptor(RootParam_MaterialSrv, Desc::MaterialBufferSrv)
        .Descriptor(RootParam_ConstantBuffer, Desc::CameraCbv)
        .Descriptor(RootParam_LightConstants, Desc::LightCbv)
        .Rtv(RtvName::LightPass)
        .Operation(Op::LightingDebugGradient, &D3D12HelloTexture::ExecuteLightingDebugGradientPass)
        .Constants(RootParam_ToneMapConstants, ConstName::ToneMap)
        .Build();
}

auto D3D12HelloTexture::MakeToneMapPass() -> RenderPass
{
    return m_renderGraphRuntime.Authoring().CreatePass(L"ToneMapPass")
        .Pipeline(Pipe::ToneMap)
        .Reads({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE}})
        .Writes({{kBackBufferResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Descriptor(RootParam_ToneMapSceneColor, Desc::ToneMapSceneColorSrv)
        .Rtv(RtvName::BackBuffer)
        .Operation(Op::ToneMap, &D3D12HelloTexture::ExecuteToneMapPass)
        .Constants(RootParam_ToneMapConstants, ConstName::ToneMap)
        .Build();
}

auto D3D12HelloTexture::MakeDebugDumpPass() -> RenderPass
{
    return m_renderGraphRuntime.Authoring().CreatePass(L"DebugDump")
        .Reads({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_COPY_SOURCE},
                {kBackBufferResourceName, D3D12_RESOURCE_STATE_COPY_SOURCE}})
        .Operation(Op::DebugDump, &D3D12HelloTexture::ExecuteDebugDumpPass)
        .Build();
}

auto D3D12HelloTexture::MakeGBufferDebugPass() -> RenderPass
{
    return m_renderGraphRuntime.Authoring().CreatePass(L"GBufferDebugPass")
        .Pipeline(Pipe::GBufferDebug)
        .Reads(MakeGBufferReadUsages())
        .Writes({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Descriptor(RootParam_GBufferSrvBase, Desc::GBufferAlbedoSrv)
        .Rtv(RtvName::LightPass)
        .Operation(Op::GBufferDebug, &D3D12HelloTexture::ExecuteGBufferDebugPass)
        .Constants(RootParam_GBufferDebugConstants, ConstName::GBufferDebugTarget)
        .Build();
}

auto D3D12HelloTexture::MakeImGuiPass() -> RenderPass
{
    return m_renderGraphRuntime.Authoring().CreatePass(L"ImGui")
        .Writes({{kBackBufferResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
        .Rtv(RtvName::BackBuffer)
        .Operation(Op::ImGui, &D3D12HelloTexture::ExecuteImGuiPass)
        .Build();
}

