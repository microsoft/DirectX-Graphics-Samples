//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author(s):  James Stanard
//             Alex Nankervis
//

#include "pch.h"
#include "SamplerManager.h"
#include "GraphicsCore.h"
#include "Hash.h"
#include <map>

using namespace std;
using namespace Graphics;

namespace
{
    map< size_t, D3D12_CPU_DESCRIPTOR_HANDLE > s_SamplerCache;
}

D3D12_CPU_DESCRIPTOR_HANDLE SamplerDesc::CreateDescriptor()
{
    size_t hashValue = Utility::HashState(this);
    auto iter = s_SamplerCache.find(hashValue);
    if (iter != s_SamplerCache.end())
    {
        return iter->second;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Handle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    g_Device->CreateSampler(this, Handle);
    return Handle;
}

void SamplerDesc::CreateDescriptor( D3D12_CPU_DESCRIPTOR_HANDLE& Handle )
{
    g_Device->CreateSampler(this, Handle);
}
