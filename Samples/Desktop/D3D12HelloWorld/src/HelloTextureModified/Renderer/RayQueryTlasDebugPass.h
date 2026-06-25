#pragma once

#include "../DXSampleHelper.h"

namespace Engine
{

struct RayQueryTlasDebugPassDesc
{
    ID3D12RootSignature* rootSignature = nullptr;
    ID3D12PipelineState* pipelineState = nullptr;
    D3D12_GPU_DESCRIPTOR_HANDLE tlasDebugUav = {};
    D3D12_GPU_DESCRIPTOR_HANDLE tlasSrv = {};
    D3D12_GPU_DESCRIPTOR_HANDLE depthSrv = {};
    D3D12_GPU_DESCRIPTOR_HANDLE normalSrv = {};
    D3D12_GPU_DESCRIPTOR_HANDLE cameraCbv = {};
    DirectX::XMFLOAT3 lightDirection = {};
    UINT width = 0;
    UINT height = 0;
};

void RecordRayQueryTlasDebugPass(ID3D12GraphicsCommandList* commandList, const RayQueryTlasDebugPassDesc& desc);

} // namespace Engine
