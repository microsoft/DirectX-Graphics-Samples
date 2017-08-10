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
// Author:  James Stanard 
//

#include "pch.h"
#include "RootSignature.h"
#include "GraphicsCore.h"
#include "Hash.h"
#include <map>
#include <thread>
#include <mutex>

using namespace Graphics;
using namespace std;
using Microsoft::WRL::ComPtr;

static std::map< size_t, ComPtr<ID3D12RootSignature> > s_RootSignatureHashMap;

void RootSignature::DestroyAll(void)
{
    s_RootSignatureHashMap.clear();
}

void RootSignature::InitStaticSampler(
    UINT Register,
    const D3D12_SAMPLER_DESC& NonStaticSamplerDesc,
    D3D12_SHADER_VISIBILITY Visibility )
{
    ASSERT(m_NumInitializedStaticSamplers < m_NumSamplers);
    D3D12_STATIC_SAMPLER_DESC& StaticSamplerDesc = m_SamplerArray[m_NumInitializedStaticSamplers++];

    StaticSamplerDesc.Filter = NonStaticSamplerDesc.Filter;
    StaticSamplerDesc.AddressU = NonStaticSamplerDesc.AddressU;
    StaticSamplerDesc.AddressV = NonStaticSamplerDesc.AddressV;
    StaticSamplerDesc.AddressW = NonStaticSamplerDesc.AddressW;
    StaticSamplerDesc.MipLODBias = NonStaticSamplerDesc.MipLODBias;
    StaticSamplerDesc.MaxAnisotropy = NonStaticSamplerDesc.MaxAnisotropy;
    StaticSamplerDesc.ComparisonFunc = NonStaticSamplerDesc.ComparisonFunc;
    StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    StaticSamplerDesc.MinLOD = NonStaticSamplerDesc.MinLOD;
    StaticSamplerDesc.MaxLOD = NonStaticSamplerDesc.MaxLOD;
    StaticSamplerDesc.ShaderRegister = Register;
    StaticSamplerDesc.RegisterSpace = 0;
    StaticSamplerDesc.ShaderVisibility = Visibility;

    if (StaticSamplerDesc.AddressU == D3D12_TEXTURE_ADDRESS_MODE_BORDER ||
        StaticSamplerDesc.AddressV == D3D12_TEXTURE_ADDRESS_MODE_BORDER ||
        StaticSamplerDesc.AddressW == D3D12_TEXTURE_ADDRESS_MODE_BORDER)
    {
        WARN_ONCE_IF_NOT(
            // Transparent Black
            NonStaticSamplerDesc.BorderColor[0] == 0.0f &&
            NonStaticSamplerDesc.BorderColor[1] == 0.0f &&
            NonStaticSamplerDesc.BorderColor[2] == 0.0f &&
            NonStaticSamplerDesc.BorderColor[3] == 0.0f ||
            // Opaque Black
            NonStaticSamplerDesc.BorderColor[0] == 0.0f &&
            NonStaticSamplerDesc.BorderColor[1] == 0.0f &&
            NonStaticSamplerDesc.BorderColor[2] == 0.0f &&
            NonStaticSamplerDesc.BorderColor[3] == 1.0f ||
            // Opaque White
            NonStaticSamplerDesc.BorderColor[0] == 1.0f &&
            NonStaticSamplerDesc.BorderColor[1] == 1.0f &&
            NonStaticSamplerDesc.BorderColor[2] == 1.0f &&
            NonStaticSamplerDesc.BorderColor[3] == 1.0f,
            "Sampler border color does not match static sampler limitations");

        if (NonStaticSamplerDesc.BorderColor[3] == 1.0f)
        {
            if (NonStaticSamplerDesc.BorderColor[0] == 1.0f)
                StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
            else
                StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
        }
        else
            StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    }
}

void RootSignature::Finalize(const std::wstring& name, D3D12_ROOT_SIGNATURE_FLAGS Flags)
{
    if (m_Finalized)
        return;

    ASSERT(m_NumInitializedStaticSamplers == m_NumSamplers);

    D3D12_ROOT_SIGNATURE_DESC RootDesc;
    RootDesc.NumParameters = m_NumParameters;
    RootDesc.pParameters = (const D3D12_ROOT_PARAMETER*)m_ParamArray.get();
    RootDesc.NumStaticSamplers = m_NumSamplers;
    RootDesc.pStaticSamplers = (const D3D12_STATIC_SAMPLER_DESC*)m_SamplerArray.get();
    RootDesc.Flags = Flags;

    m_DescriptorTableBitMap = 0;
    m_SamplerTableBitMap = 0;

    size_t HashCode = Utility::HashState(&RootDesc.Flags);
    HashCode = Utility::HashState( RootDesc.pStaticSamplers, m_NumSamplers, HashCode );

    for (UINT Param = 0; Param < m_NumParameters; ++Param)
    {
        const D3D12_ROOT_PARAMETER& RootParam = RootDesc.pParameters[Param];
        m_DescriptorTableSize[Param] = 0;

        if (RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
        {
            ASSERT(RootParam.DescriptorTable.pDescriptorRanges != nullptr);

            HashCode = Utility::HashState( RootParam.DescriptorTable.pDescriptorRanges,
                RootParam.DescriptorTable.NumDescriptorRanges, HashCode );

            // We keep track of sampler descriptor tables separately from CBV_SRV_UAV descriptor tables
            if (RootParam.DescriptorTable.pDescriptorRanges->RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
                m_SamplerTableBitMap |= (1 << Param);
            else
                m_DescriptorTableBitMap |= (1 << Param);

            for (UINT TableRange = 0; TableRange < RootParam.DescriptorTable.NumDescriptorRanges; ++TableRange)
                m_DescriptorTableSize[Param] += RootParam.DescriptorTable.pDescriptorRanges[TableRange].NumDescriptors;
        }
        else
            HashCode = Utility::HashState( &RootParam, 1, HashCode );
    }

    ID3D12RootSignature** RSRef = nullptr;
    bool firstCompile = false;
    {
        static mutex s_HashMapMutex;
        lock_guard<mutex> CS(s_HashMapMutex);
        auto iter = s_RootSignatureHashMap.find(HashCode);

        // Reserve space so the next inquiry will find that someone got here first.
        if (iter == s_RootSignatureHashMap.end())
        {
            RSRef = s_RootSignatureHashMap[HashCode].GetAddressOf();
            firstCompile = true;
        }
        else
            RSRef = iter->second.GetAddressOf();
    }

    if (firstCompile)
    {
        ComPtr<ID3DBlob> pOutBlob, pErrorBlob;

        ASSERT_SUCCEEDED( D3D12SerializeRootSignature(&RootDesc, D3D_ROOT_SIGNATURE_VERSION_1,
            pOutBlob.GetAddressOf(), pErrorBlob.GetAddressOf()));

        ASSERT_SUCCEEDED( g_Device->CreateRootSignature(1, pOutBlob->GetBufferPointer(), pOutBlob->GetBufferSize(),
            MY_IID_PPV_ARGS(&m_Signature)) );

        m_Signature->SetName(name.c_str());

        s_RootSignatureHashMap[HashCode].Attach(m_Signature);
        ASSERT(*RSRef == m_Signature);
    }
    else
    {
        while (*RSRef == nullptr)
            this_thread::yield();
        m_Signature = *RSRef;
    }

    m_Finalized = TRUE;
}
