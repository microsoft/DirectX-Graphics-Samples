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
#include "CullDataVisualizer.h"

#include "DXSampleHelper.h"

using namespace DirectX;

namespace
{
    const wchar_t* s_boundingSphereMsFilename = L"BoundingSphereMS.cso";
    const wchar_t* s_normalConeMsFilename     = L"NormalConeMS.cso";
    const wchar_t* s_debugDrawPsFilename      = L"DebugDrawPS.cso";

    std::wstring GetAssetFullPath(const wchar_t* path)
    {
        WCHAR assetsPath[512];
        GetAssetsPath(assetsPath, _countof(assetsPath));

        return std::wstring(assetsPath) + path;
    }
}

CullDataVisualizer::CullDataVisualizer()
    : m_constantData(nullptr)
    , m_frameIndex(0)
{ }

void CullDataVisualizer::CreateDeviceResources(ID3D12Device2* device, DXGI_FORMAT rtFormat, DXGI_FORMAT dsFormat)
{
    // Load shader bytecode and extract root signature
    struct
    {
        byte* data;
        uint32_t size;
    } normalConeMs, boundingSphereMs, pixelShader;

    ReadDataFromFile(GetAssetFullPath(s_normalConeMsFilename).c_str(), &normalConeMs.data, &normalConeMs.size);
    ReadDataFromFile(GetAssetFullPath(s_boundingSphereMsFilename).c_str(), &boundingSphereMs.data, &boundingSphereMs.size);
    ReadDataFromFile(GetAssetFullPath(s_debugDrawPsFilename).c_str(), &pixelShader.data, &pixelShader.size);

    ThrowIfFailed(device->CreateRootSignature(0, normalConeMs.data, normalConeMs.size, IID_PPV_ARGS(&m_rootSignature)));

    // Disable culling
    CD3DX12_RASTERIZER_DESC rasterDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    rasterDesc.CullMode = D3D12_CULL_MODE_NONE;

    // Disable depth test & writes
    CD3DX12_DEPTH_STENCIL_DESC dsDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    dsDesc.DepthEnable = false;
    dsDesc.StencilEnable = false;
    dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

    // Populate the Mesh Shader PSO descriptor
    D3DX12_MESH_SHADER_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature        = m_rootSignature.Get();
    psoDesc.PS                    = { pixelShader.data, pixelShader.size };
    psoDesc.NumRenderTargets      = 1;
    psoDesc.RTVFormats[0]         = rtFormat;
    psoDesc.DSVFormat             = dsFormat;
    psoDesc.RasterizerState       = rasterDesc;
    psoDesc.DepthStencilState     = dsDesc;
    psoDesc.SampleMask            = UINT_MAX;
    psoDesc.SampleDesc            = DefaultSampleDesc();

    // Create normal cone pipeline
    {
        // Cone lines are drawn opaquely
        psoDesc.MS = { normalConeMs.data, normalConeMs.size };
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // Opaque

        auto meshStreamDesc = CD3DX12_PIPELINE_MESH_STATE_STREAM(psoDesc);

        // Populate the stream desc with our defined PSO descriptor
        D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};
        streamDesc.SizeInBytes                   = sizeof(meshStreamDesc);
        streamDesc.pPipelineStateSubobjectStream = &meshStreamDesc;

        ThrowIfFailed(device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_normalConePso)));
    }

    // Create bounding sphere pipeline
    {
        // bounding sphere pipeline requires additive blending
        D3D12_BLEND_DESC blendDesc = {};
        blendDesc.RenderTarget[0].BlendEnable           = true;
        blendDesc.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend             = D3D12_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_MAX;
        blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        psoDesc.MS = { boundingSphereMs.data, boundingSphereMs.size };
        psoDesc.BlendState = blendDesc;

        auto meshStreamDesc = CD3DX12_PIPELINE_MESH_STATE_STREAM(psoDesc);

        // Populate the stream desc with our defined PSO descriptor
        D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};
        streamDesc.SizeInBytes                   = sizeof(meshStreamDesc);
        streamDesc.pPipelineStateSubobjectStream = &meshStreamDesc;

        ThrowIfFailed(device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_boundingSpherePso)));
    }

    // Create shared constant buffer
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(sizeof(Constants) * 2),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_constantResource)
    ));

    ThrowIfFailed(m_constantResource->Map(0, nullptr, &m_constantData));
}

void CullDataVisualizer::ReleaseResources()
{
    m_rootSignature.Reset();
    m_boundingSpherePso.Reset();
    m_normalConePso.Reset();
    m_constantResource.Reset();
}

void CullDataVisualizer::SetConstants(FXMMATRIX world, CXMMATRIX view, CXMMATRIX proj, CXMVECTOR color)
{
    m_frameIndex = (m_frameIndex + 1) % 2;

    XMVECTOR scl, rot, pos;
    XMMatrixDecompose(&scl, &rot, &pos, world);

    XMMATRIX viewToWorld = XMMatrixTranspose(view);
    XMVECTOR camUp       = XMVector3TransformNormal(g_XMIdentityR1, viewToWorld);    // Y-axis is up direction
    XMVECTOR camForward  = XMVector3TransformNormal(g_XMNegIdentityR2, viewToWorld); // -Z-axis is forward direction

    auto& constants = *(reinterpret_cast<Constants*>(m_constantData) + m_frameIndex);

    XMStoreFloat4x4(&constants.ViewProj, XMMatrixTranspose(view * proj));
    XMStoreFloat4x4(&constants.World, XMMatrixTranspose(world));
    XMStoreFloat4(&constants.Color, color);
    XMStoreFloat4(&constants.ViewUp, camUp);
    XMStoreFloat3(&constants.ViewForward, camForward);
    constants.Scale = XMVectorGetX(scl);

    constants.Color.w = 0.3f;
}

void CullDataVisualizer::Draw(ID3D12GraphicsCommandList6* commandList, const Mesh& mesh, uint32_t offset, uint32_t count)
{
    // Push constant data to GPU for our shader invocations
    assert(offset + count <= static_cast<uint32_t>(mesh.Meshlets.size()));

    // Shared root signature between two shaders
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    commandList->SetGraphicsRootConstantBufferView(0, m_constantResource->GetGPUVirtualAddress() + sizeof(Constants) * m_frameIndex);
    commandList->SetGraphicsRoot32BitConstant(1, offset, 0);
    commandList->SetGraphicsRoot32BitConstant(1, count, 1);
    commandList->SetGraphicsRootShaderResourceView(2, mesh.CullDataResource->GetGPUVirtualAddress());

    // Dispatch bounding sphere draw
    commandList->SetPipelineState(m_boundingSpherePso.Get());
    commandList->DispatchMesh(count, 1, 1);

    // Dispatch normal cone draw
    commandList->SetPipelineState(m_normalConePso.Get());
    commandList->DispatchMesh(count, 1, 1);
}
