#pragma once

#include "../DXSampleHelper.h"

namespace Engine
{

struct ShaderBytecode
{
    const UINT8* data = nullptr;
    UINT size = 0;
};

struct GraphicsPipelineShaderSet
{
    ShaderBytecode vertex;
    ShaderBytecode pixel;
};

struct InputLayoutDefinition
{
    const D3D12_INPUT_ELEMENT_DESC* elements = nullptr;
    UINT count = 0;
};

struct ForwardPipelineDefinition
{
    const char* name = nullptr;
    InputLayoutDefinition inputLayout;
    GraphicsPipelineShaderSet shaders;
    DXGI_FORMAT renderTargetFormat = DXGI_FORMAT_UNKNOWN;
    DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_UNKNOWN;
};

struct GBufferPipelineDefinition
{
    const char* name = nullptr;
    InputLayoutDefinition inputLayout;
    GraphicsPipelineShaderSet shaders;
};

struct DepthPrePassPipelineDefinition
{
    const char* name = nullptr;
    InputLayoutDefinition inputLayout;
    GraphicsPipelineShaderSet shaders;
};

struct FullscreenPipelineDefinition
{
    const char* name = nullptr;
    GraphicsPipelineShaderSet shaders;
    DXGI_FORMAT renderTargetFormat = DXGI_FORMAT_UNKNOWN;
};

D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateForwardPipelineDesc(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                          ID3D12RootSignature* rootSignature,
                                                          const ForwardPipelineDefinition& definition);
D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateGBufferPipelineDesc(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                             const GBufferPipelineDefinition& definition,
                                                             const DXGI_FORMAT* renderTargetFormats,
                                                             UINT renderTargetCount);
D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateDepthPrePassPipelineDesc(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                                  const DepthPrePassPipelineDefinition& definition);
D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateFullscreenPipelineDesc(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
                                                                const FullscreenPipelineDefinition& definition);

} // namespace Engine
