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

#include "RenderPassKeys.h"

#include <array>
#include <cassert>
#include <d3d12.h>
#include <functional>
#include <initializer_list>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

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


} // namespace Engine
