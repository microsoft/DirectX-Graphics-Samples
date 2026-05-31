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
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <d3d12.h>
#include <functional>
#include <limits>
#include <optional>
#include <string>
#include <unordered_map>
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

struct RenderPassKeys
{
    std::unordered_map<std::string, PipelineKey> pipelines;
    std::unordered_map<std::string, DescriptorKey> descriptors;
    std::unordered_map<std::string, RtvKey> rtvs;
    std::unordered_map<std::string, DsvKey> dsvs;
    std::unordered_map<std::string, PassOperationKey> operations;
    std::unordered_map<std::string, PassConstantsKey> constants;

    PipelineKey RegisterPipeline(const std::string &name, RenderPassKeyRegistry &registry)
    {
        return Register(pipelines, name, [&registry](const std::string &keyName) { return registry.AddPipeline(keyName); });
    }
    DescriptorKey RegisterDescriptor(const std::string &name, RenderPassKeyRegistry &registry)
    {
        return Register(descriptors, name,
                        [&registry](const std::string &keyName) { return registry.AddDescriptor(keyName); });
    }
    RtvKey RegisterRtv(const std::string &name, RenderPassKeyRegistry &registry)
    {
        return Register(rtvs, name, [&registry](const std::string &keyName) { return registry.AddRtv(keyName); });
    }
    DsvKey RegisterDsv(const std::string &name, RenderPassKeyRegistry &registry)
    {
        return Register(dsvs, name, [&registry](const std::string &keyName) { return registry.AddDsv(keyName); });
    }
    PassOperationKey RegisterOperation(const std::string &name, RenderPassKeyRegistry &registry)
    {
        return Register(operations, name,
                        [&registry](const std::string &keyName) { return registry.AddOperation(keyName); });
    }
    PassConstantsKey RegisterConstants(const std::string &name, RenderPassKeyRegistry &registry)
    {
        return Register(constants, name, [&registry](const std::string &keyName) { return registry.AddConstants(keyName); });
    }

    PipelineKey PipelineId(const std::string &name) const { return Find(pipelines, name); }
    DescriptorKey DescriptorId(const std::string &name) const { return Find(descriptors, name); }
    RtvKey RtvId(const std::string &name) const { return Find(rtvs, name); }
    DsvKey DsvId(const std::string &name) const { return Find(dsvs, name); }
    PassOperationKey OperationId(const std::string &name) const { return Find(operations, name); }
    PassConstantsKey ConstantsId(const std::string &name) const { return Find(constants, name); }

  private:
    template <typename KeyT, typename RegisterFunc>
    static KeyT Register(std::unordered_map<std::string, KeyT> &keys, const std::string &name,
                         RegisterFunc registerFunc)
    {
        auto key = keys.find(name);
        if (key != keys.end())
        {
            return key->second;
        }

        const KeyT registeredKey = registerFunc(name);
        keys[name] = registeredKey;
        return registeredKey;
    }

    template <typename KeyT> static KeyT Find(const std::unordered_map<std::string, KeyT> &keys,
                                              const std::string &name)
    {
        auto key = keys.find(name);
        assert(key != keys.end() && "Missing render pass key.");
        return key != keys.end() ? key->second : KeyT{};
    }
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

    // Pipeline selects the GPU state to bind for the pass. It is intentionally separate from operation:
    // multiple passes may record similar commands while using different PSOs, root signatures, or shaders.
    PipelineKey pipeline;
    ResourceUsages reads;
    ResourceUsages writes;
    std::vector<PassDescriptorBinding> descriptorBindings;
    PassRenderTargetBinding renderTargets;

    // Operation selects the command recording behavior for the pass. It answers "what commands are recorded",
    // while pipeline answers "which GPU state is bound before recording them".
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

struct ResourceLifetime
{
    int firstPass = INT_MAX;
    int lastPass = -1;
};

using ResourceLifetimeMap = std::unordered_map<std::string, ResourceLifetime>;

inline ResourceLifetimeMap AnalyzeResourceLifetimes(const std::vector<RenderPass> &renderPasses)
{
    ResourceLifetimeMap lifetimes;

    for (int passIndex = 0; passIndex < static_cast<int>(renderPasses.size()); ++passIndex)
    {
        const auto &pass = renderPasses[passIndex];

        pass.ForEachResourceUsage(
            [&](const ResourceUsage &usage)
            {
                auto &lifetime = lifetimes[usage.name];
                lifetime.firstPass = (std::min)(lifetime.firstPass, passIndex);
                lifetime.lastPass = (std::max)(lifetime.lastPass, passIndex);
            });
    }

    return lifetimes;
}

} // namespace Engine

namespace std
{
template <typename Tag> struct hash<Engine::Key<Tag>>
{
    size_t operator()(Engine::Key<Tag> key) const noexcept { return hash<uint32_t>{}(key.index); }
};
} // namespace std
