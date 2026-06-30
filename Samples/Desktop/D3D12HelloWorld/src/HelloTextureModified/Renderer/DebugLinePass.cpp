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

#include "DebugLinePass.h"
#include "../MyDx12Utils.h"
#include <d3dx12_core.h>
#include <d3dx12_root_signature.h>

namespace Engine
{

void DebugLinePass::Create(
    ID3D12Device* device,
    D3D12_SHADER_BYTECODE vs,
    D3D12_SHADER_BYTECODE ps)
{
    CreateRootSignature(device);
    CreatePipelineState(device, vs, ps);
    CreateVertexBuffer(device);
}

void DebugLinePass::CreateRootSignature(ID3D12Device* device)
{
    CD3DX12_DESCRIPTOR_RANGE1 cbvRange = {};
    cbvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

    CD3DX12_ROOT_PARAMETER1 rootParameters[1] = {};
    rootParameters[0].InitAsConstantBufferView(0);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr,
                               D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error));
    ThrowIfFailed(device->CreateRootSignature(
        0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
}

void DebugLinePass::CreatePipelineState(ID3D12Device* device, D3D12_SHADER_BYTECODE vs, D3D12_SHADER_BYTECODE ps)
{
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = vs;
    psoDesc.PS = ps;
    psoDesc.InputLayout = {inputElementDescs, _countof(inputElementDescs)};
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    psoDesc.NumRenderTargets = 1;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleMask = UINT_MAX;

    // Depth test off: lines are always visible on top of the scene.
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;

    // Default rasterizer: solid fill, back-face cull.
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;

    // Default blend: overwrite.
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

void DebugLinePass::CreateVertexBuffer(ID3D12Device* device)
{
    const UINT bufferSize = sizeof(DebugLineVertex) * kMaxDebugVertices;
    MyDx12Util::CreateUploadBuffer(device, bufferSize, m_vertexBuffer);

    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(DebugLineVertex);
    m_vertexBufferView.SizeInBytes = bufferSize;
}

void DebugLinePass::UpdateLines(
    const std::vector<DebugLineVertex>& vertices,
    ID3D12GraphicsCommandList* commandList)
{
    const UINT count = static_cast<UINT>((std::min)(vertices.size(), static_cast<size_t>(kMaxDebugVertices)));
    m_vertexCount = count;

    if (count == 0)
    {
        return;
    }

    // Map and copy vertex data.
    void* mappedData = nullptr;
    CD3DX12_RANGE readRange(0, 0);
    ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, &mappedData));
    memcpy(mappedData, vertices.data(), sizeof(DebugLineVertex) * count);
    m_vertexBuffer->Unmap(0, nullptr);

    // Issue a barrier if the buffer was previously used as a vertex buffer.
    // With UPLOAD heap and GENERIC_READ state this is normally unnecessary,
    // but the vertex buffer will be used for drawing later in the same command list.
    // No barrier needed for UPLOAD -> VERTEX_AND_CONSTANT_BUFFER since
    // GENERIC_READ is compatible.
    UNREFERENCED_PARAMETER(commandList);
}

void DebugLinePass::RecordDraw(
    ID3D12GraphicsCommandList* commandList,
    D3D12_GPU_VIRTUAL_ADDRESS viewProjCbv) const
{
    if (m_vertexCount == 0)
    {
        return;
    }

    commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    commandList->SetPipelineState(m_pipelineState.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    commandList->SetGraphicsRootConstantBufferView(0, viewProjCbv);
    commandList->DrawInstanced(m_vertexCount, 1, 0, 0);
}

} // namespace Engine
