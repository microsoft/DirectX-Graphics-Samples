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

#include "Utils.h"
#include "CD3DX12AffinityPageable.h"

class __declspec(uuid("BE1D71C8-88FD-4623-ABFA-D0E546D12FAF")) CD3DX12AffinityPipelineState : public CD3DX12AffinityPageable
{
public:
    HRESULT STDMETHODCALLTYPE GetCachedBlob(
        _COM_Outptr_  ID3DBlob** ppBlob,
        UINT AffinityIndex = 0);

    CD3DX12AffinityPipelineState(CD3DX12AffinityDevice* device, ID3D12PipelineState** pipelineStates, UINT Count);

    ID3D12PipelineState* mPipelineStates[D3DX12_MAX_ACTIVE_NODES];
};
