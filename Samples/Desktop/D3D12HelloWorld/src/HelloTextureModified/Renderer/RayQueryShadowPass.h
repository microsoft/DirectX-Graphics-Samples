#pragma once

#include "../DXSampleHelper.h"

namespace Engine
{

struct RayQueryShadowPassDesc
{
    ID3D12RootSignature* rootSignature = nullptr;
    ID3D12PipelineState* pipelineState = nullptr;
    D3D12_GPU_DESCRIPTOR_HANDLE shadowMaskUav = {};
    D3D12_GPU_DESCRIPTOR_HANDLE tlasSrv = {};
    D3D12_GPU_DESCRIPTOR_HANDLE depthSrv = {};
    D3D12_GPU_DESCRIPTOR_HANDLE normalSrv = {};
    D3D12_GPU_DESCRIPTOR_HANDLE cameraCbv = {};
    DirectX::XMFLOAT3 lightDirection = {};
    float normalBias = 0.01f;
    float rayTMin = 0.001f;
    float rayTMax = 10000.0f;
    uint32_t enabled = 1;
    UINT width = 0;
    UINT height = 0;
};

void RecordRayQueryShadowPass(ID3D12GraphicsCommandList* commandList, const RayQueryShadowPassDesc& desc);

} // namespace Engine
