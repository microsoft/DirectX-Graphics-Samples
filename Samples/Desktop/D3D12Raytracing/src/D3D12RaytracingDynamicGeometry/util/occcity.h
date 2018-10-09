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

namespace SampleAssets
{
    LPCWSTR DataFileName = L"occcity.bin";

    const D3D12_INPUT_ELEMENT_DESC StandardVertexDescription[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
    UINT StandardVertexDescriptionNumElements = _countof(StandardVertexDescription);

    const UINT StandardVertexStride = 44;

    const DXGI_FORMAT StandardIndexFormat = DXGI_FORMAT_R32_UINT;

    struct TextureResource
    {
        UINT Width;
        UINT Height;
        UINT16 MipLevels;
        DXGI_FORMAT Format;
        struct DataProperties
        {
            UINT Offset;
            UINT Size;
            UINT Pitch;
        } Data[D3D12_REQ_MIP_LEVELS];
    };

    struct DrawParameters
    {
        INT DiffuseTextureIndex;
        INT NormalTextureIndex;
        INT SpecularTextureIndex;
        UINT IndexStart;
        UINT IndexCount;
        UINT VertexBase;
    };

    const UINT VertexDataOffset = 524288;
    const UINT VertexDataSize = 820248;
    const UINT IndexDataOffset = 1344536;
    const UINT IndexDataSize = 74568;

    TextureResource Textures[] =
    {
        { 1024, 1024, 1, DXGI_FORMAT_BC1_UNORM, { { 0, 524288, 2048 }, } }, // city.dds
    };

    DrawParameters Draws[] =
    {
        { 0, -1, -1, 0, 18642, 0 },
    };
}
