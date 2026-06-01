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
