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
#include "stdafx.h"
#include "FrustumVisualizer.h"

#include "DXSampleHelper.h"

using namespace DirectX;

namespace
{
    const wchar_t* s_drawFrustumMsFilename = L"FrustumMS.cso";
    const wchar_t* s_debugDrawPsFilename   = L"DebugDrawPS.cso";

    std::wstring GetAssetFullPath(const wchar_t* path)
    {
        WCHAR assetsPath[512];
        GetAssetsPath(assetsPath, _countof(assetsPath));

        return std::wstring(assetsPath) + path;
    }
}

FrustumVisualizer::FrustumVisualizer()
    : m_constantData(nullptr)
    , m_frameIndex(0)
{ }

void FrustumVisualizer::CreateDeviceResources(ID3D12Device2* device, DXGI_FORMAT rtFormat, DXGI_FORMAT dsFormat)
{
    // Load shader bytecode and extract root signature
    struct
    {
        byte* data;
        uint32_t size;
    } meshShader, pixelShader;

    ReadDataFromFile(GetAssetFullPath(s_drawFrustumMsFilename).c_str(), &meshShader.data, &meshShader.size);
    ReadDataFromFile(GetAssetFullPath(s_debugDrawPsFilename).c_str(), &pixelShader.data, &pixelShader.size);

    // Extract root signature directly from the mesh shader bytecode
    ThrowIfFailed(device->CreateRootSignature(0, meshShader.data, meshShader.size, IID_PPV_ARGS(&m_rootSignature)));

    D3DX12_MESH_SHADER_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature        = m_rootSignature.Get();
    psoDesc.MS                    = { meshShader.data, meshShader.size };
    psoDesc.PS                    = { pixelShader.data, pixelShader.size };
    psoDesc.NumRenderTargets      = 1;
    psoDesc.RTVFormats[0]         = rtFormat;
    psoDesc.DSVFormat             = dsFormat;
    psoDesc.RasterizerState       = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);    // CW front; cull back
    psoDesc.BlendState            = CD3DX12_BLEND_DESC(D3D12_DEFAULT);         // Opaque
    psoDesc.DepthStencilState     = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // Less-equal depth test w/ writes; no stencil
    psoDesc.SampleMask            = UINT_MAX;
    psoDesc.SampleDesc            = DefaultSampleDesc();

    auto meshStreamDesc = CD3DX12_PIPELINE_MESH_STATE_STREAM(psoDesc);

    // Populate the stream desc with our defined PSO descriptor
    D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};
    streamDesc.SizeInBytes                   = sizeof(meshStreamDesc);
    streamDesc.pPipelineStateSubobjectStream = &meshStreamDesc;

    // Create the MS PSO
    ThrowIfFailed(device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_pso)));

    const CD3DX12_HEAP_PROPERTIES constantBufferHeapProps(D3D12_HEAP_TYPE_UPLOAD);
    const CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(Constants) * 2);

    // Create the constant buffer
    ThrowIfFailed(device->CreateCommittedResource(
        &constantBufferHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &constantBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_constantResource)
    ));

    ThrowIfFailed(m_constantResource->Map(0, nullptr, &m_constantData));
}

void FrustumVisualizer::ReleaseResources()
{
    m_rootSignature.Reset();
    m_pso.Reset();
    m_constantResource.Reset();
}

void FrustumVisualizer::Update(FXMMATRIX vp, XMVECTOR (&planes)[6])
{
    m_frameIndex = (m_frameIndex + 1) % 2;

    auto& constants = *(reinterpret_cast<Constants*>(m_constantData) + m_frameIndex);

    XMStoreFloat4(&constants.LineColor, DirectX::Colors::Purple);
    XMStoreFloat4x4(&constants.ViewProj, XMMatrixTranspose(vp));

    for (uint32_t i = 0; i < _countof(planes); ++i)
    {
        XMStoreFloat4(&constants.Planes[i], planes[i]);
    }
}

void FrustumVisualizer::Draw(ID3D12GraphicsCommandList6* cmdList)
{
    // Set root signature, resources, pipeline state and dispatch
    cmdList->SetGraphicsRootSignature(m_rootSignature.Get());
    cmdList->SetGraphicsRootConstantBufferView(0, m_constantResource->GetGPUVirtualAddress() + sizeof(Constants) * m_frameIndex);

    cmdList->SetPipelineState(m_pso.Get());
    cmdList->DispatchMesh(1, 1, 1);
}
