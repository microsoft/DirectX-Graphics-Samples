#include "stdafx.h"

#include "RayQueryShadowPass.h"

#include <pix3.h>

namespace Engine
{

void RecordRayQueryShadowPass(ID3D12GraphicsCommandList* commandList, const RayQueryShadowPassDesc& desc)
{
    PIXBeginEvent(commandList, 0, L"RayQueryShadowPass");

    commandList->SetComputeRootSignature(desc.rootSignature);
    commandList->SetPipelineState(desc.pipelineState);
    commandList->SetComputeRootDescriptorTable(0, desc.shadowMaskUav);

    constexpr UINT kThreadGroupSize = 8;
    const UINT dispatchX = (desc.width + kThreadGroupSize - 1) / kThreadGroupSize;
    const UINT dispatchY = (desc.height + kThreadGroupSize - 1) / kThreadGroupSize;
    commandList->Dispatch(dispatchX, dispatchY, 1);

    PIXEndEvent(commandList);
}

} // namespace Engine
