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
#include <cstddef>
#include <cstdint>
#include <d3d12.h>
#include <functional>
#include <initializer_list>
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

    bool IsValid() const
    {
        return index != kInvalidKeyIndex;
    }

    friend bool operator==(Key lhs, Key rhs)
    {
        return lhs.index == rhs.index;
    }
    friend bool operator!=(Key lhs, Key rhs)
    {
        return !(lhs == rhs);
    }
};

template <typename KeyT> struct KeyHash
{
    size_t operator()(KeyT key) const noexcept
    {
        return std::hash<uint32_t>{}(key.index);
    }
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
    auto AddPipeline(std::string name) -> PipelineKey
    {
        return AddKey<PipelineKey>(m_pipelineNames, std::move(name));
    }
    auto AddDescriptor(std::string name) -> DescriptorKey
    {
        return AddKey<DescriptorKey>(m_descriptorNames, std::move(name));
    }
    auto AddRtv(std::string name) -> RtvKey
    {
        return AddKey<RtvKey>(m_rtvNames, std::move(name));
    }
    auto AddDsv(std::string name) -> DsvKey
    {
        return AddKey<DsvKey>(m_dsvNames, std::move(name));
    }
    auto AddOperation(std::string name) -> PassOperationKey
    {
        return AddKey<PassOperationKey>(m_operationNames, std::move(name));
    }
    auto AddConstants(std::string name) -> PassConstantsKey
    {
        return AddKey<PassConstantsKey>(m_constantsNames, std::move(name));
    }

private:
    template <typename KeyT> KeyT AddKey(std::vector<std::string>& names, std::string name)
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

    PipelineKey RegisterPipeline(const std::string& name, RenderPassKeyRegistry& registry)
    {
        return Register(pipelines, name,
                        [&registry](const std::string& keyName) { return registry.AddPipeline(keyName); });
    }
    DescriptorKey RegisterDescriptor(const std::string& name, RenderPassKeyRegistry& registry)
    {
        return Register(descriptors, name,
                        [&registry](const std::string& keyName) { return registry.AddDescriptor(keyName); });
    }
    RtvKey RegisterRtv(const std::string& name, RenderPassKeyRegistry& registry)
    {
        return Register(rtvs, name, [&registry](const std::string& keyName) { return registry.AddRtv(keyName); });
    }
    DsvKey RegisterDsv(const std::string& name, RenderPassKeyRegistry& registry)
    {
        return Register(dsvs, name, [&registry](const std::string& keyName) { return registry.AddDsv(keyName); });
    }
    PassOperationKey RegisterOperation(const std::string& name, RenderPassKeyRegistry& registry)
    {
        return Register(operations, name,
                        [&registry](const std::string& keyName) { return registry.AddOperation(keyName); });
    }
    PassConstantsKey RegisterConstants(const std::string& name, RenderPassKeyRegistry& registry)
    {
        return Register(constants, name,
                        [&registry](const std::string& keyName) { return registry.AddConstants(keyName); });
    }

    PipelineKey PipelineId(const std::string& name) const
    {
        return Find(pipelines, name);
    }
    DescriptorKey DescriptorId(const std::string& name) const
    {
        return Find(descriptors, name);
    }
    RtvKey RtvId(const std::string& name) const
    {
        return Find(rtvs, name);
    }
    DsvKey DsvId(const std::string& name) const
    {
        return Find(dsvs, name);
    }
    PassOperationKey OperationId(const std::string& name) const
    {
        return Find(operations, name);
    }
    PassConstantsKey ConstantsId(const std::string& name) const
    {
        return Find(constants, name);
    }

private:
    template <typename KeyT, typename RegisterFunc>
    static KeyT Register(std::unordered_map<std::string, KeyT>& keys, const std::string& name,
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

    template <typename KeyT>
    static KeyT Find(const std::unordered_map<std::string, KeyT>& keys, const std::string& name)
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
    const wchar_t* name;

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
        for (const ResourceUsage& usage : reads)
        {
            func(usage);
        }

        for (const ResourceUsage& usage : writes)
        {
            func(usage);
        }
    }
};

template <typename HandlerT> class PassOperationRegistry
{
public:
    void Clear()
    {
        m_handlers.clear();
    }

    PassOperationKey Register(PassOperationKey operation, HandlerT handler)
    {
        auto [registered, inserted] = m_handlers.emplace(operation, handler);
        assert((inserted || registered->second == handler) && "Pass operation registered with a different handler.");
        return operation;
    }

    bool Contains(PassOperationKey operation) const
    {
        return m_handlers.find(operation) != m_handlers.end();
    }

    const HandlerT* Find(PassOperationKey operation) const
    {
        auto handler = m_handlers.find(operation);
        return handler != m_handlers.end() ? &handler->second : nullptr;
    }

private:
    std::unordered_map<PassOperationKey, HandlerT> m_handlers;
};

template <typename OperationHandlerT> class RenderPassBuilder
{
public:
    RenderPassBuilder(const wchar_t* name, RenderPassKeys& keys, RenderPassKeyRegistry& keyRegistry,
                      PassOperationRegistry<OperationHandlerT>& operationRegistry)
        : m_keys(&keys)
        , m_keyRegistry(&keyRegistry)
        , m_operationRegistry(&operationRegistry)
    {
        m_pass.name = name;
    }

    RenderPassBuilder& Pipeline(PipelineKey pipeline)
    {
        m_pass.pipeline = pipeline;
        return *this;
    }

    RenderPassBuilder& Pipeline(const std::string& pipeline)
    {
        return Pipeline(RegisterPipeline(pipeline));
    }

    RenderPassBuilder& Reads(ResourceUsages reads)
    {
        m_pass.reads = std::move(reads);
        return *this;
    }

    RenderPassBuilder& Reads(std::initializer_list<ResourceUsage> reads)
    {
        m_pass.reads = ResourceUsages(reads);
        return *this;
    }

    RenderPassBuilder& Writes(ResourceUsages writes)
    {
        m_pass.writes = std::move(writes);
        return *this;
    }

    RenderPassBuilder& Writes(std::initializer_list<ResourceUsage> writes)
    {
        m_pass.writes = ResourceUsages(writes);
        return *this;
    }

    RenderPassBuilder& Descriptor(UINT rootParameterIndex, DescriptorKey descriptor)
    {
        m_pass.descriptorBindings.push_back({rootParameterIndex, descriptor});
        return *this;
    }

    RenderPassBuilder& Descriptor(UINT rootParameterIndex, const std::string& descriptor)
    {
        return Descriptor(rootParameterIndex, RegisterDescriptor(descriptor));
    }

    RenderPassBuilder& Descriptors(std::initializer_list<PassDescriptorBinding> descriptors)
    {
        m_pass.descriptorBindings = std::vector<PassDescriptorBinding>(descriptors);
        return *this;
    }

    RenderPassBuilder& Descriptors(std::vector<PassDescriptorBinding> descriptors)
    {
        m_pass.descriptorBindings = std::move(descriptors);
        return *this;
    }

    RenderPassBuilder& RenderTargets(PassRenderTargetBinding renderTargets)
    {
        m_pass.renderTargets = std::move(renderTargets);
        return *this;
    }

    RenderPassBuilder& Rtv(RtvKey rtv)
    {
        m_pass.renderTargets.rtvs.push_back(rtv);
        return *this;
    }

    RenderPassBuilder& Rtv(const std::string& rtv)
    {
        return Rtv(RegisterRtv(rtv));
    }

    RenderPassBuilder& Rtvs(std::initializer_list<RtvKey> rtvs)
    {
        m_pass.renderTargets.rtvs = std::vector<RtvKey>(rtvs);
        return *this;
    }

    RenderPassBuilder& Rtvs(std::initializer_list<const char*> rtvs)
    {
        m_pass.renderTargets.rtvs.clear();
        for (const char* rtv : rtvs)
        {
            m_pass.renderTargets.rtvs.push_back(RegisterRtv(rtv));
        }
        return *this;
    }

    RenderPassBuilder& Dsv(DsvKey dsv)
    {
        m_pass.renderTargets.dsv = dsv;
        return *this;
    }

    RenderPassBuilder& Dsv(const std::string& dsv)
    {
        return Dsv(RegisterDsv(dsv));
    }

    RenderPassBuilder& ClearColor(std::array<float, 4> clearColor)
    {
        m_pass.renderTargets.clearColor = clearColor;
        return *this;
    }

    RenderPassBuilder& Operation(PassOperationKey operation)
    {
        m_pass.operation = operation;
        return *this;
    }

    RenderPassBuilder& Operation(const std::string& operation, OperationHandlerT handler)
    {
        m_pass.operation = m_operationRegistry->Register(RegisterOperation(operation), handler);
        return *this;
    }

    RenderPassBuilder& Constants(UINT rootParameterIndex, PassConstantsKey constants)
    {
        m_pass.constantsBindings.push_back({rootParameterIndex, constants});
        return *this;
    }

    RenderPassBuilder& Constants(UINT rootParameterIndex, const std::string& constants)
    {
        return Constants(rootParameterIndex, RegisterConstants(constants));
    }

    RenderPassBuilder& ConstantsBindings(std::initializer_list<PassConstantsBinding> constantsBindings)
    {
        m_pass.constantsBindings = std::vector<PassConstantsBinding>(constantsBindings);
        return *this;
    }

    RenderPass Build()
    {
        return std::move(m_pass);
    }

private:
    PipelineKey RegisterPipeline(const std::string& name)
    {
        return m_keys->RegisterPipeline(name, *m_keyRegistry);
    }
    DescriptorKey RegisterDescriptor(const std::string& name)
    {
        return m_keys->RegisterDescriptor(name, *m_keyRegistry);
    }
    RtvKey RegisterRtv(const std::string& name)
    {
        return m_keys->RegisterRtv(name, *m_keyRegistry);
    }
    DsvKey RegisterDsv(const std::string& name)
    {
        return m_keys->RegisterDsv(name, *m_keyRegistry);
    }
    PassOperationKey RegisterOperation(const std::string& name)
    {
        return m_keys->RegisterOperation(name, *m_keyRegistry);
    }
    PassConstantsKey RegisterConstants(const std::string& name)
    {
        return m_keys->RegisterConstants(name, *m_keyRegistry);
    }

    RenderPassKeys* m_keys = nullptr;
    RenderPassKeyRegistry* m_keyRegistry = nullptr;
    PassOperationRegistry<OperationHandlerT>* m_operationRegistry = nullptr;
    RenderPass m_pass = {};
};

template <typename OperationHandlerT> class RenderPassAuthoringContext
{
public:
    using Builder = RenderPassBuilder<OperationHandlerT>;

    RenderPassAuthoringContext() = default;

    RenderPassAuthoringContext(RenderPassKeys& keys, RenderPassKeyRegistry& keyRegistry,
                               PassOperationRegistry<OperationHandlerT>& operationRegistry)
    {
        Bind(keys, keyRegistry, operationRegistry);
    }

    void Bind(RenderPassKeys& keys, RenderPassKeyRegistry& keyRegistry,
              PassOperationRegistry<OperationHandlerT>& operationRegistry)
    {
        m_keys = &keys;
        m_keyRegistry = &keyRegistry;
        m_operationRegistry = &operationRegistry;
    }

    Builder CreatePass(const wchar_t* name)
    {
        assert(m_keys != nullptr && m_keyRegistry != nullptr && m_operationRegistry != nullptr);
        return Builder(name, *m_keys, *m_keyRegistry, *m_operationRegistry);
    }

private:
    RenderPassKeys* m_keys = nullptr;
    RenderPassKeyRegistry* m_keyRegistry = nullptr;
    PassOperationRegistry<OperationHandlerT>* m_operationRegistry = nullptr;
};

class RenderPassBindingResolverRegistry
{
public:
    using DescriptorResolver = std::function<D3D12_GPU_DESCRIPTOR_HANDLE()>;
    using RtvResolver = std::function<D3D12_CPU_DESCRIPTOR_HANDLE()>;
    using DsvResolver = std::function<D3D12_CPU_DESCRIPTOR_HANDLE()>;

    void Clear()
    {
        m_descriptorResolvers.clear();
        m_rtvResolvers.clear();
        m_dsvResolvers.clear();
    }

    void RegisterDescriptor(DescriptorKey descriptor, DescriptorResolver resolver)
    {
        m_descriptorResolvers[descriptor] = std::move(resolver);
    }

    void RegisterRtv(RtvKey rtv, RtvResolver resolver)
    {
        m_rtvResolvers[rtv] = std::move(resolver);
    }

    void RegisterDsv(DsvKey dsv, DsvResolver resolver)
    {
        m_dsvResolvers[dsv] = std::move(resolver);
    }

    bool ContainsDescriptor(DescriptorKey descriptor) const
    {
        return m_descriptorResolvers.find(descriptor) != m_descriptorResolvers.end();
    }

    bool ContainsRtv(RtvKey rtv) const
    {
        return m_rtvResolvers.find(rtv) != m_rtvResolvers.end();
    }

    bool ContainsDsv(DsvKey dsv) const
    {
        return m_dsvResolvers.find(dsv) != m_dsvResolvers.end();
    }

    D3D12_GPU_DESCRIPTOR_HANDLE ResolveDescriptor(DescriptorKey descriptor) const
    {
        auto resolver = m_descriptorResolvers.find(descriptor);
        assert(resolver != m_descriptorResolvers.end() && "Missing pass descriptor resolver.");
        return resolver != m_descriptorResolvers.end() ? resolver->second() : D3D12_GPU_DESCRIPTOR_HANDLE{};
    }

    D3D12_CPU_DESCRIPTOR_HANDLE ResolveRtv(RtvKey rtv) const
    {
        auto resolver = m_rtvResolvers.find(rtv);
        assert(resolver != m_rtvResolvers.end() && "Missing pass RTV resolver.");
        return resolver != m_rtvResolvers.end() ? resolver->second() : D3D12_CPU_DESCRIPTOR_HANDLE{};
    }

    D3D12_CPU_DESCRIPTOR_HANDLE ResolveDsv(DsvKey dsv) const
    {
        auto resolver = m_dsvResolvers.find(dsv);
        assert(resolver != m_dsvResolvers.end() && "Missing pass DSV resolver.");
        return resolver != m_dsvResolvers.end() ? resolver->second() : D3D12_CPU_DESCRIPTOR_HANDLE{};
    }

    void BindDescriptors(ID3D12GraphicsCommandList* commandList, const RenderPass& pass) const
    {
        for (const PassDescriptorBinding& binding : pass.descriptorBindings)
        {
            commandList->SetGraphicsRootDescriptorTable(binding.rootParameterIndex,
                                                        ResolveDescriptor(binding.descriptor));
        }
    }

    void BindRenderTargets(ID3D12GraphicsCommandList* commandList, const RenderPass& pass) const
    {
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvs;
        rtvs.reserve(pass.renderTargets.rtvs.size());
        for (RtvKey rtv : pass.renderTargets.rtvs)
        {
            rtvs.push_back(ResolveRtv(rtv));
        }

        std::optional<D3D12_CPU_DESCRIPTOR_HANDLE> dsv;
        if (pass.renderTargets.dsv)
        {
            dsv = ResolveDsv(pass.renderTargets.dsv.value());
        }

        const D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles = rtvs.empty() ? nullptr : rtvs.data();
        const D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle = dsv ? &dsv.value() : nullptr;
        commandList->OMSetRenderTargets(static_cast<UINT>(rtvs.size()), rtvHandles, FALSE, dsvHandle);
    }

private:
    std::unordered_map<DescriptorKey, DescriptorResolver, KeyHash<DescriptorKey>> m_descriptorResolvers;
    std::unordered_map<RtvKey, RtvResolver, KeyHash<RtvKey>> m_rtvResolvers;
    std::unordered_map<DsvKey, DsvResolver, KeyHash<DsvKey>> m_dsvResolvers;
};

struct RenderPassGraph
{
    std::vector<RenderPass> passes;

    void Clear()
    {
        passes.clear();
    }
    void Add(RenderPass pass)
    {
        passes.push_back(std::move(pass));
    }
    const std::vector<RenderPass>& Passes() const
    {
        return passes;
    }
    size_t Size() const
    {
        return passes.size();
    }
    const RenderPass& operator[](size_t index) const
    {
        return passes[index];
    }
};

struct RenderPassGraphValidationCallbacks
{
    std::function<bool(PipelineKey)> hasPipeline;
    std::function<bool(PassOperationKey)> hasOperation;
    std::function<bool(DescriptorKey)> canResolveDescriptor;
    std::function<bool(RtvKey)> canResolveRtv;
    std::function<bool(DsvKey)> canResolveDsv;
    std::function<bool(PassConstantsKey)> canBindConstants;
};

inline void ValidateRenderPassGraph(const std::vector<RenderPass>& renderPasses,
                                    const RenderPassGraphValidationCallbacks& callbacks = {})
{
    for (const RenderPass& pass : renderPasses)
    {
        if (callbacks.hasPipeline && pass.pipeline.IsValid())
        {
            assert(callbacks.hasPipeline(pass.pipeline) && "Render pass references an unregistered pipeline.");
        }

        if (callbacks.hasOperation)
        {
            assert(callbacks.hasOperation(pass.operation) &&
                   "Render pass references an unregistered operation handler.");
        }

        if (callbacks.canResolveDescriptor)
        {
            for (const PassDescriptorBinding& binding : pass.descriptorBindings)
            {
                assert(callbacks.canResolveDescriptor(binding.descriptor) &&
                       "Render pass references an unresolved descriptor.");
            }
        }

        if (callbacks.canResolveRtv)
        {
            for (RtvKey rtv : pass.renderTargets.rtvs)
            {
                assert(callbacks.canResolveRtv(rtv) && "Render pass references an unresolved RTV.");
            }
        }

        if (callbacks.canResolveDsv && pass.renderTargets.dsv)
        {
            assert(callbacks.canResolveDsv(pass.renderTargets.dsv.value()) &&
                   "Render pass references an unresolved DSV.");
        }

        if (callbacks.canBindConstants)
        {
            for (const PassConstantsBinding& binding : pass.constantsBindings)
            {
                assert(callbacks.canBindConstants(binding.constants) &&
                       "Render pass references unsupported constants binding.");
            }
        }
    }
}

struct ResourceLifetime
{
    int firstPass = INT_MAX;
    int lastPass = -1;
};

using ResourceLifetimeMap = std::unordered_map<std::string, ResourceLifetime>;

inline ResourceLifetimeMap AnalyzeResourceLifetimes(const std::vector<RenderPass>& renderPasses)
{
    ResourceLifetimeMap lifetimes;

    for (int passIndex = 0; passIndex < static_cast<int>(renderPasses.size()); ++passIndex)
    {
        const auto& pass = renderPasses[passIndex];

        pass.ForEachResourceUsage(
            [&](const ResourceUsage& usage)
            {
                auto& lifetime = lifetimes[usage.name];
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
    size_t operator()(Engine::Key<Tag> key) const noexcept
    {
        return hash<uint32_t>{}(key.index);
    }
};
} // namespace std
