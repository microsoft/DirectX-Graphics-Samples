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


#pragma once

#include "pch.h"

class RootSignature;

class IndirectParameter
{
    friend class CommandSignature;
public:

    IndirectParameter() 
    {
        m_IndirectParam.Type = (D3D12_INDIRECT_ARGUMENT_TYPE)0xFFFFFFFF;
    }

    void Draw(void)
    {
        m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
    }

    void DrawIndexed(void)
    {
        m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
    }

    void Dispatch(void)
    {
        m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
    }

    void VertexBufferView(UINT Slot)
    {
        m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
        m_IndirectParam.VertexBuffer.Slot = Slot;
    }

    void IndexBufferView(void)
    {
        m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;
    }

    void Constant(UINT RootParameterIndex, UINT DestOffsetIn32BitValues, UINT Num32BitValuesToSet)
    {
        m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
        m_IndirectParam.Constant.RootParameterIndex = RootParameterIndex;
        m_IndirectParam.Constant.DestOffsetIn32BitValues = DestOffsetIn32BitValues;
        m_IndirectParam.Constant.Num32BitValuesToSet = Num32BitValuesToSet;
    }

    void ConstantBufferView(UINT RootParameterIndex)
    {
        m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
        m_IndirectParam.ConstantBufferView.RootParameterIndex = RootParameterIndex;
    }

    void ShaderResourceView(UINT RootParameterIndex)
    {
        m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
        m_IndirectParam.ShaderResourceView.RootParameterIndex = RootParameterIndex;
    }

    void UnorderedAccessView(UINT RootParameterIndex)
    {
        m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW;
        m_IndirectParam.UnorderedAccessView.RootParameterIndex = RootParameterIndex;
    }

    const D3D12_INDIRECT_ARGUMENT_DESC& GetDesc( void ) const { return m_IndirectParam; }

protected:

    D3D12_INDIRECT_ARGUMENT_DESC m_IndirectParam;
};

class CommandSignature
{
public:

    CommandSignature( UINT NumParams = 0 ) : m_Finalized(FALSE), m_NumParameters(NumParams)
    {
        Reset(NumParams);
    }

    void Destroy( void )
    {
        m_Signature = nullptr;
        m_ParamArray = nullptr;
    }

    void Reset( UINT NumParams )
    {
        if (NumParams > 0)
            m_ParamArray.reset(new IndirectParameter[NumParams]);
        else
            m_ParamArray = nullptr;

        m_NumParameters = NumParams;
    }

    IndirectParameter& operator[] ( size_t EntryIndex )
    {
        ASSERT(EntryIndex < m_NumParameters);
        return m_ParamArray.get()[EntryIndex];
    }

    const IndirectParameter& operator[] ( size_t EntryIndex ) const
    {
        ASSERT(EntryIndex < m_NumParameters);
        return m_ParamArray.get()[EntryIndex];
    }

    void Finalize( const RootSignature* RootSignature = nullptr );

    ID3D12CommandSignature* GetSignature() const { return m_Signature.Get(); }

protected:

    BOOL m_Finalized;
    UINT m_NumParameters;
    std::unique_ptr<IndirectParameter[]> m_ParamArray;
    Microsoft::WRL::ComPtr<ID3D12CommandSignature> m_Signature;
};
