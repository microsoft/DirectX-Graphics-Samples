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
#include <climits>
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
#include <wrl/client.h>

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

class PipelineRegistry
{
public:
    HRESULT Create(ID3D12Device* device, PipelineKey key, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc)
    {
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
        HRESULT hr = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipelineState));
        if (SUCCEEDED(hr))
        {
            m_pipelines[key] = std::move(pipelineState);
        }
        return hr;
    }

    ID3D12PipelineState* Find(PipelineKey key) const
    {
        if (!key.IsValid())
        {
            return nullptr;
        }

        auto pipeline = m_pipelines.find(key);
        return pipeline != m_pipelines.end() ? pipeline->second.Get() : nullptr;
    }

    bool Contains(PipelineKey key) const
    {
        return Find(key) != nullptr;
    }

    void Bind(ID3D12GraphicsCommandList* commandList, const RenderPass& pass) const
    {
        ID3D12PipelineState* pipelineState = Find(pass.pipeline);
        assert(!pass.pipeline.IsValid() || pipelineState != nullptr);
        if (pipelineState != nullptr)
        {
            commandList->SetPipelineState(pipelineState);
        }
    }

private:
    std::unordered_map<PipelineKey, Microsoft::WRL::ComPtr<ID3D12PipelineState>, KeyHash<PipelineKey>> m_pipelines;
};

class PassConstantsRegistry
{
public:
    using Handler = std::function<void(UINT rootParameterIndex)>;

    void Clear()
    {
        m_handlers.clear();
    }

    PassConstantsKey Register(PassConstantsKey constants, Handler handler)
    {
        auto [registered, inserted] = m_handlers.emplace(constants, std::move(handler));
        assert(inserted && "Pass constants registered more than once.");
        return constants;
    }

    bool Contains(PassConstantsKey constants) const
    {
        return m_handlers.find(constants) != m_handlers.end();
    }

    void Bind(const RenderPass& pass) const
    {
        for (const PassConstantsBinding& binding : pass.constantsBindings)
        {
            auto handler = m_handlers.find(binding.constants);
            assert(handler != m_handlers.end() && "Unsupported pass constants binding.");
            if (handler != m_handlers.end())
            {
                handler->second(binding.rootParameterIndex);
            }
        }
    }

private:
    std::unordered_map<PassConstantsKey, Handler, KeyHash<PassConstantsKey>> m_handlers;
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

struct ResourceTransitionContext
{
    ID3D12GraphicsCommandList* commandList = nullptr;
    std::function<ID3D12Resource*(const std::string& name)> resolveResource;
    std::function<D3D12_RESOURCE_STATES(const std::string& name)> getResourceState;
    std::function<void(const std::string& name, D3D12_RESOURCE_STATES state)> setResourceState;
    std::function<void(const ResourceUsage& usage)> onMissingResource;
};

inline void TransitionResource(const ResourceTransitionContext& context, const ResourceUsage& usage)
{
    assert(context.commandList != nullptr);
    assert(context.resolveResource);
    assert(context.getResourceState);
    assert(context.setResourceState);

    const D3D12_RESOURCE_STATES currentState = context.getResourceState(usage.name);
    if (currentState == usage.state)
    {
        return;
    }

    ID3D12Resource* resource = context.resolveResource(usage.name);
    assert(resource != nullptr && "Cannot transition a null resource.");
    if (resource == nullptr)
    {
        if (context.onMissingResource)
        {
            context.onMissingResource(usage);
        }
        return;
    }

    context.commandList->ResourceBarrier(
        1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, currentState, usage.state));
    context.setResourceState(usage.name, usage.state);
}

inline void TransitionPassResources(const ResourceTransitionContext& context, const RenderPass& pass)
{
    pass.ForEachResourceUsage([&context](const ResourceUsage& usage) { TransitionResource(context, usage); });
}

struct RenderPassExecutionContext
{
    ID3D12GraphicsCommandList* commandList = nullptr;
    const RenderPassBindingResolverRegistry* bindingResolvers = nullptr;
    const PipelineRegistry* pipelineRegistry = nullptr;
    const PassConstantsRegistry* constantsRegistry = nullptr;
    const ResourceTransitionContext* resourceTransitions = nullptr;

    std::function<void(int)> createResourcesForPass;
    std::function<void(const RenderPass&)> executeOperation;
    std::function<void(int)> releaseResourcesAfterPass;
};

inline void ExecuteRenderPassGraph(const RenderPassGraph& graph, const RenderPassExecutionContext& context)
{
    assert(context.commandList != nullptr);
    assert(context.bindingResolvers != nullptr);
    assert(context.pipelineRegistry != nullptr);
    assert(context.constantsRegistry != nullptr);
    assert(context.resourceTransitions != nullptr);

    for (int passIndex = 0; passIndex < static_cast<int>(graph.Size()); ++passIndex)
    {
        if (context.createResourcesForPass)
        {
            context.createResourcesForPass(passIndex);
        }

        const RenderPass& pass = graph[passIndex];
        if (context.resourceTransitions)
        {
            TransitionPassResources(*context.resourceTransitions, pass);
        }

        context.bindingResolvers->BindRenderTargets(context.commandList, pass);
        context.bindingResolvers->BindDescriptors(context.commandList, pass);
        context.pipelineRegistry->Bind(context.commandList, pass);

        if (context.constantsRegistry)
        {
            context.constantsRegistry->Bind(pass);
        }
        if (context.executeOperation)
        {
            context.executeOperation(pass);
        }
        if (context.releaseResourcesAfterPass)
        {
            context.releaseResourcesAfterPass(passIndex);
        }
    }
}

struct RenderPassGraphValidationCallbacks
{
    std::function<bool(PipelineKey)> hasPipeline;
    std::function<bool(PassOperationKey)> hasOperation;
    std::function<bool(DescriptorKey)> canResolveDescriptor;
    std::function<bool(RtvKey)> canResolveRtv;
    std::function<bool(DsvKey)> canResolveDsv;
    std::function<bool(PassConstantsKey)> canBindConstants;
};

template <typename OperationHandlerT> struct RenderPassGraphValidationContext
{
    const PipelineRegistry* pipelineRegistry = nullptr;
    const RenderPassBindingResolverRegistry* bindingResolvers = nullptr;
    const PassOperationRegistry<OperationHandlerT>* operationRegistry = nullptr;
    const PassConstantsRegistry* constantsRegistry = nullptr;
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

template <typename OperationHandlerT>
inline void ValidateRenderPassGraph(const std::vector<RenderPass>& renderPasses,
                                    const RenderPassGraphValidationContext<OperationHandlerT>& context)
{
    assert(context.pipelineRegistry != nullptr);
    assert(context.bindingResolvers != nullptr);
    assert(context.operationRegistry != nullptr);

    ValidateRenderPassGraph(
        renderPasses,
        {context.pipelineRegistry != nullptr
             ? [pipelineRegistry = context.pipelineRegistry](PipelineKey pipeline)
               { return pipelineRegistry->Contains(pipeline); }
             : std::function<bool(PipelineKey)>(),
         context.operationRegistry != nullptr
             ? [operationRegistry = context.operationRegistry](PassOperationKey operation)
               { return operationRegistry->Contains(operation); }
             : std::function<bool(PassOperationKey)>(),
         context.bindingResolvers != nullptr
             ? [bindingResolvers = context.bindingResolvers](DescriptorKey descriptor)
               { return bindingResolvers->ContainsDescriptor(descriptor); }
             : std::function<bool(DescriptorKey)>(),
         context.bindingResolvers != nullptr
             ? [bindingResolvers = context.bindingResolvers](RtvKey rtv) { return bindingResolvers->ContainsRtv(rtv); }
             : std::function<bool(RtvKey)>(),
         context.bindingResolvers != nullptr
             ? [bindingResolvers = context.bindingResolvers](DsvKey dsv) { return bindingResolvers->ContainsDsv(dsv); }
             : std::function<bool(DsvKey)>(),
         context.constantsRegistry != nullptr
             ? [constantsRegistry = context.constantsRegistry](PassConstantsKey constants)
               { return constantsRegistry->Contains(constants); }
             : context.canBindConstants});
}

struct ResourceLifetime
{
    int firstPass = INT_MAX;
    int lastPass = -1;
};

using ResourceLifetimeMap = std::unordered_map<std::string, ResourceLifetime>;
using ResourceStateMap = std::unordered_map<std::string, D3D12_RESOURCE_STATES>;

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

enum class TransientResourceState
{
    Uninitialized,
    Initialized,
    Created,
    PendingRelease1,
    PendingRelease2
};

template <typename TransientResourceT> class ResourceRegistry
{
public:
    using TransientResourceMap = std::unordered_map<std::string, TransientResourceT>;

    ResourceStateMap states;
    ResourceLifetimeMap lifetimes;
    TransientResourceMap transientResources;

    void AnalyzeLifetimes(const std::vector<RenderPass>& renderPasses)
    {
        lifetimes = Engine::AnalyzeResourceLifetimes(renderPasses);
    }

    void ResetStates(std::initializer_list<ResourceUsage> usages)
    {
        states.clear();
        for (const ResourceUsage& usage : usages)
        {
            SetState(usage.name, usage.state);
        }
    }

    void RegisterTransientResource(TransientResourceT resource)
    {
        transientResources[resource.name] = std::move(resource);
    }

    void UnregisterTransientResource(const std::string& name)
    {
        transientResources.erase(name);
    }

    void MarkEndOfLifeResources(int passIndex, const char* backBufferName)
    {
        for (auto& [name, lt] : lifetimes)
        {
            if (lt.lastPass != passIndex || name == backBufferName || !transientResources.contains(name))
            {
                continue;
            }

            auto& resource = transientResources.at(name);
            assert(resource.state != TransientResourceState::Uninitialized &&
                   "Transient resource must be registered before release.");
            if (resource.state == TransientResourceState::Uninitialized || resource.persistent ||
                resource.state != TransientResourceState::Created)
            {
                continue;
            }

            resource.state = TransientResourceState::PendingRelease1;
        }
    }

    void MarkPendingTransientResources(UINT64 fenceValue)
    {
        for (auto& [name, resource] : transientResources)
        {
            if (resource.state != TransientResourceState::PendingRelease1)
            {
                continue;
            }

            resource.retireFenceValue = fenceValue;
            resource.state = TransientResourceState::PendingRelease2;
        }
    }

    std::vector<std::string> CollectGarbageTransientResources(UINT64 completedFenceValue)
    {
        std::vector<std::string> releasedResources;

        for (auto& [name, resource] : transientResources)
        {
            if (resource.state != TransientResourceState::PendingRelease2 ||
                completedFenceValue < resource.retireFenceValue)
            {
                continue;
            }

            resource.retireFenceValue = 0;
            resource.state = TransientResourceState::Initialized;
            releasedResources.push_back(name);
        }

        return releasedResources;
    }

    std::vector<std::string> GetResourcesStartingAtPass(int passIndex, const char* backBufferName) const
    {
        std::vector<std::string> resourceNames;

        for (const auto& [name, lt] : lifetimes)
        {
            if (lt.firstPass != passIndex || name == backBufferName || !transientResources.contains(name))
            {
                continue;
            }

            resourceNames.push_back(name);
        }

        return resourceNames;
    }

    TransientResourceT* PrepareTransientResourceForCreate(const std::string& name)
    {
        auto transientResource = transientResources.find(name);
        if (transientResource == transientResources.end())
        {
            return nullptr;
        }

        auto& resource = transientResource->second;
        assert(resource.state != TransientResourceState::Uninitialized &&
               "Transient resource must be registered before use.");
        if (resource.state == TransientResourceState::Uninitialized || resource.state == TransientResourceState::Created)
        {
            return nullptr;
        }

        if (resource.state == TransientResourceState::PendingRelease1 ||
            resource.state == TransientResourceState::PendingRelease2)
        {
            resource.retireFenceValue = 0;
            resource.state = TransientResourceState::Created;
            return nullptr;
        }

        return resource.state == TransientResourceState::Initialized ? &resource : nullptr;
    }

    void MarkTransientResourceCreated(const std::string& name)
    {
        auto transientResource = transientResources.find(name);
        assert(transientResource != transientResources.end() &&
               "Transient resource must be registered before marking it created.");
        if (transientResource != transientResources.end())
        {
            transientResource->second.state = TransientResourceState::Created;
        }
    }

    D3D12_RESOURCE_STATES GetState(const std::string& name) const
    {
        auto resourceState = states.find(name);
        return resourceState != states.end() ? resourceState->second : D3D12_RESOURCE_STATE_COMMON;
    }

    void SetState(const std::string& name, D3D12_RESOURCE_STATES state)
    {
        states[name] = state;
    }
};

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
