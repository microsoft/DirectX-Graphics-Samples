#include "stdafx.h"

#include "RayQueryTlasDebugPass.h"

#include <pix3.h>

namespace Engine
{

void RecordRayQueryTlasDebugPass(ID3D12GraphicsCommandList* commandList, const RayQueryTlasDebugPassDesc& desc)
{
    PIXBeginEvent(commandList, 0, L"RayQueryTlasDebugPass");

    commandList->SetComputeRootSignature(desc.rootSignature);
    commandList->SetPipelineState(desc.pipelineState);

    // Root param 0: TlasDebug UAV
    commandList->SetComputeRootDescriptorTable(0, desc.tlasDebugUav);

    // Root param 1: TLAS SRV
    commandList->SetComputeRootDescriptorTable(1, desc.tlasSrv);

    // Root param 2: Depth SRV
    commandList->SetComputeRootDescriptorTable(2, desc.depthSrv);

    // Root param 3: Normal SRV (GBuffer normal for normal-offset bias - reserved for future use)
    commandList->SetComputeRootDescriptorTable(3, desc.normalSrv);

    // Root param 4: Camera CBV (contains invViewProj for world position reconstruction)
    commandList->SetComputeRootDescriptorTable(4, desc.cameraCbv);

    // Root param 5: Light direction (float3) as 4 root constants (3 used + padding)
    commandList->SetComputeRoot32BitConstants(5, 3, &desc.lightDirection, 0);

    constexpr UINT kThreadGroupSize = 8;
    const UINT dispatchX = (desc.width + kThreadGroupSize - 1) / kThreadGroupSize;
    const UINT dispatchY = (desc.height + kThreadGroupSize - 1) / kThreadGroupSize;
    commandList->Dispatch(dispatchX, dispatchY, 1);

    PIXEndEvent(commandList);
}

} // namespace Engine
