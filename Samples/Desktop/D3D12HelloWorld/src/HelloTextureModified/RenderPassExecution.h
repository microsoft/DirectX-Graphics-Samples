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

#include "RenderPassGraph.h"

#include <cassert>
#include <d3d12.h>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <wrl/client.h>

namespace Engine
{

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

class ResourceResolverRegistry
{
public:
    using Resolver = std::function<ID3D12Resource*()>;

    void Clear()
    {
        m_resolvers.clear();
        m_fallbackResolver = {};
    }

    void RegisterResource(std::string name, Resolver resolver)
    {
        m_resolvers[std::move(name)] = std::move(resolver);
    }

    void SetFallbackResolver(std::function<ID3D12Resource*(const std::string& name)> resolver)
    {
        m_fallbackResolver = std::move(resolver);
    }

    ID3D12Resource* Resolve(const std::string& name) const
    {
        auto resolver = m_resolvers.find(name);
        if (resolver != m_resolvers.end())
        {
            return resolver->second();
        }

        return m_fallbackResolver ? m_fallbackResolver(name) : nullptr;
    }

private:
    std::unordered_map<std::string, Resolver> m_resolvers;
    std::function<ID3D12Resource*(const std::string& name)> m_fallbackResolver;
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


} // namespace Engine
