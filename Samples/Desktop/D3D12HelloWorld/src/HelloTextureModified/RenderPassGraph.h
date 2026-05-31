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

#pragma once

#include <array>
#include <cstddef>
#include <d3d12.h>
#include <optional>
#include <string>
#include <utility>
#include <vector>

enum class PipelineKey
{
    None,
    Main,
    DepthPrePass,
    GBuffer,
    Lighting,
    LightingDebugGradient,
    ToneMap,
    GBufferDebug,
};

enum class DescriptorKey
{
    TextureTable,
    InstanceBufferSrv,
    MaterialBufferSrv,
    CameraCbv,
    LightCbv,
    GBufferAlbedoSrv,
    ToneMapSceneColorSrv,
};

enum class RtvKey
{
    BackBuffer,
    GBufferAlbedo,
    GBufferNormal,
    GBufferMaterial,
    GBufferMotionVector,
    GBufferPBRParams,
    LightPass,
};

enum class DsvKey
{
    Depth,
};

enum class PassOperation
{
    Clear,
    DepthPrePass,
    GBuffer,
    Main,
    Lighting,
    LightingDebugGradient,
    ToneMap,
    DebugDump,
    GBufferDebug,
    ImGui,
};

enum class PassConstantsKey
{
    ToneMap,
    GBufferDebugTarget,
};

namespace Engine
{

struct ResourceUsage
{
    std::string name;
    D3D12_RESOURCE_STATES state;
};

using ResourceUsages = std::vector<ResourceUsage>;

struct PassDescriptorBinding
{
    UINT rootParameterIndex;
    DescriptorKey descriptor;
};

struct PassRenderTargetBinding
{
    std::vector<RtvKey> rtvs;
    std::optional<DsvKey> dsv;
    std::optional<std::array<float, 4>> clearColor;
};

struct PassConstantsBinding
{
    UINT rootParameterIndex;
    PassConstantsKey constants;
};

struct RenderPass
{
    const wchar_t *name;
    PipelineKey pipeline;
    ResourceUsages reads;
    ResourceUsages writes;
    std::vector<PassDescriptorBinding> descriptorBindings;
    PassRenderTargetBinding renderTargets;
    PassOperation operation;
    std::vector<PassConstantsBinding> constantsBindings;

    template <typename Func> void ForEachResourceUsage(Func func) const
    {
        for (const ResourceUsage &usage : reads)
        {
            func(usage);
        }

        for (const ResourceUsage &usage : writes)
        {
            func(usage);
        }
    }
};

struct RenderPassGraph
{
    std::vector<RenderPass> passes;

    void Clear() { passes.clear(); }
    void Add(RenderPass pass) { passes.push_back(std::move(pass)); }
    const std::vector<RenderPass> &Passes() const { return passes; }
    size_t Size() const { return passes.size(); }
    const RenderPass &operator[](size_t index) const { return passes[index]; }
};

} // namespace Engine
