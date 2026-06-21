#pragma once

#include "../DXSampleHelper.h"

namespace Engine
{

struct RayQueryShadowPassDesc
{
    ID3D12RootSignature* rootSignature = nullptr;
    ID3D12PipelineState* pipelineState = nullptr;
    D3D12_GPU_DESCRIPTOR_HANDLE shadowMaskUav = {};
    UINT width = 0;
    UINT height = 0;
};

void RecordRayQueryShadowPass(ID3D12GraphicsCommandList* commandList, const RayQueryShadowPassDesc& desc);

} // namespace Engine
