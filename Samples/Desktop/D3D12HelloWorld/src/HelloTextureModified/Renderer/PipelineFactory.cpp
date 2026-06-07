#include "stdafx.h"

#include "PipelineFactory.h"

#include <initializer_list>

namespace Engine
{
namespace
{

void ClearRenderTargetFormats(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc)
{
    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
    {
        desc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
    }
}

void SetRenderTargetFormats(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, const DXGI_FORMAT* formats, UINT formatCount)
{
    ClearRenderTargetFormats(desc);

    desc.NumRenderTargets = formatCount;
    for (UINT i = 0; i < formatCount; ++i)
    {
        desc.RTVFormats[i] = formats[i];
    }
}

void SetRenderTargetFormats(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, std::initializer_list<DXGI_FORMAT> formats)
{
    SetRenderTargetFormats(desc, formats.begin(), static_cast<UINT>(formats.size()));
}

} // namespace

D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateForwardPipelineDesc(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                          ID3D12RootSignature* rootSignature,
                                                          const ForwardPipelineDefinition& definition)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = baseDesc;
    desc.InputLayout = {definition.inputLayout.elements, definition.inputLayout.count};
    desc.pRootSignature = rootSignature;
    desc.VS = CD3DX12_SHADER_BYTECODE(definition.shaders.vertex.data, definition.shaders.vertex.size);
    desc.PS = CD3DX12_SHADER_BYTECODE(definition.shaders.pixel.data, definition.shaders.pixel.size);
    desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    desc.SampleMask = UINT_MAX;
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.NumRenderTargets = 1;
    desc.RTVFormats[0] = definition.renderTargetFormat;
    desc.DSVFormat = definition.depthStencilFormat;
    desc.SampleDesc.Count = 1;
    return desc;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateGBufferPipelineDesc(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                             const GBufferPipelineDefinition& definition,
                                                             const DXGI_FORMAT* renderTargetFormats,
                                                             UINT renderTargetCount)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC gbufferBaseDesc = baseDesc;
    gbufferBaseDesc.InputLayout = {definition.inputLayout.elements, definition.inputLayout.count};

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = gbufferBaseDesc;
    desc.VS = CD3DX12_SHADER_BYTECODE(definition.shaders.vertex.data, definition.shaders.vertex.size);
    desc.PS = CD3DX12_SHADER_BYTECODE(definition.shaders.pixel.data, definition.shaders.pixel.size);
    SetRenderTargetFormats(desc, renderTargetFormats, renderTargetCount);
    return desc;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateDepthPrePassPipelineDesc(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                                  const DepthPrePassPipelineDefinition& definition)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = baseDesc;
    desc.InputLayout = {definition.inputLayout.elements, definition.inputLayout.count};
    desc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
    desc.VS = CD3DX12_SHADER_BYTECODE(definition.shaders.vertex.data, definition.shaders.vertex.size);
    desc.PS = {};
    desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    desc.BlendState.RenderTarget[0].RenderTargetWriteMask = 0;
    desc.NumRenderTargets = 0;
    return desc;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateFullscreenPipelineDesc(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                                const FullscreenPipelineDefinition& definition)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = baseDesc;
    desc.InputLayout = {};
    desc.VS = CD3DX12_SHADER_BYTECODE(definition.shaders.vertex.data, definition.shaders.vertex.size);
    desc.PS = CD3DX12_SHADER_BYTECODE(definition.shaders.pixel.data, definition.shaders.pixel.size);
    desc.DepthStencilState.DepthEnable = FALSE;
    desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
    SetRenderTargetFormats(desc, {definition.renderTargetFormat});
    return desc;
}

} // namespace Engine
