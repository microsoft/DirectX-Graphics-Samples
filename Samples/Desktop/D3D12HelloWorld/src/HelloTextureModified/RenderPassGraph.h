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
#include <cstdint>
#include <cstddef>
#include <d3d12.h>
#include <functional>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace Engine
{

inline constexpr uint32_t kInvalidKeyIndex = (std::numeric_limits<uint32_t>::max)();

template <typename Tag> struct Key
{
    uint32_t index = kInvalidKeyIndex;

    bool IsValid() const { return index != kInvalidKeyIndex; }

    friend bool operator==(Key lhs, Key rhs) { return lhs.index == rhs.index; }
    friend bool operator!=(Key lhs, Key rhs) { return !(lhs == rhs); }
};

struct PipelineKeyTag;
struct DescriptorKeyTag;
struct RtvKeyTag;
struct DsvKeyTag;
struct PassOperationKeyTag;
struct PassConstantsKeyTag;

using PipelineKey = Key<PipelineKeyTag>;
using DescriptorKey = Key<DescriptorKeyTag>;
using RtvKey = Key<RtvKeyTag>;
using DsvKey = Key<DsvKeyTag>;
using PassOperationKey = Key<PassOperationKeyTag>;
using PassConstantsKey = Key<PassConstantsKeyTag>;

class RenderPassKeyRegistry
{
  public:
    PipelineKey AddPipeline(std::string name) { return AddKey<PipelineKey>(m_pipelineNames, std::move(name)); }
    DescriptorKey AddDescriptor(std::string name) { return AddKey<DescriptorKey>(m_descriptorNames, std::move(name)); }
    RtvKey AddRtv(std::string name) { return AddKey<RtvKey>(m_rtvNames, std::move(name)); }
    DsvKey AddDsv(std::string name) { return AddKey<DsvKey>(m_dsvNames, std::move(name)); }
    PassOperationKey AddOperation(std::string name) { return AddKey<PassOperationKey>(m_operationNames, std::move(name)); }
    PassConstantsKey AddConstants(std::string name) { return AddKey<PassConstantsKey>(m_constantsNames, std::move(name)); }

  private:
    template <typename KeyT> KeyT AddKey(std::vector<std::string> &names, std::string name)
    {
        const uint32_t index = static_cast<uint32_t>(names.size());
        names.push_back(std::move(name));
        return KeyT{index};
    }

    std::vector<std::string> m_pipelineNames;
    std::vector<std::string> m_descriptorNames;
    std::vector<std::string> m_rtvNames;
    std::vector<std::string> m_dsvNames;
    std::vector<std::string> m_operationNames;
    std::vector<std::string> m_constantsNames;
};

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
    PassOperationKey operation;
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

namespace std
{
template <typename Tag> struct hash<Engine::Key<Tag>>
{
    size_t operator()(Engine::Key<Tag> key) const noexcept { return hash<uint32_t>{}(key.index); }
};
} // namespace std
