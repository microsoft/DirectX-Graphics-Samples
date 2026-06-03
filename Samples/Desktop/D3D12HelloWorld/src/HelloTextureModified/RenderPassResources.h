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

#include <algorithm>
#include <cassert>
#include <climits>
#include <d3d12.h>
#include <initializer_list>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Engine
{

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
