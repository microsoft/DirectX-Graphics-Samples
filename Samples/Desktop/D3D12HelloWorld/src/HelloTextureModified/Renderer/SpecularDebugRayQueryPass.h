#pragma once

#include "../DXSampleHelper.h"

namespace Engine
{

struct SpecularDebugRayQueryConstants
{
    DirectX::XMFLOAT3 rayOrigin = {};
    float rayTMin = 0.001f;
    DirectX::XMFLOAT3 rayDirection = {};
    float rayTMax = 10000.0f;
};

struct SpecularDebugRayQueryPassDesc
{
    ID3D12RootSignature* rootSignature = nullptr;
    ID3D12PipelineState* pipelineState = nullptr;
    D3D12_GPU_VIRTUAL_ADDRESS resultUav = 0;
    D3D12_GPU_DESCRIPTOR_HANDLE tlasSrv = {};
    SpecularDebugRayQueryConstants constants = {};
};

void RecordSpecularDebugRayQueryPass(ID3D12GraphicsCommandList* commandList, const SpecularDebugRayQueryPassDesc& desc);

} // namespace Engine
