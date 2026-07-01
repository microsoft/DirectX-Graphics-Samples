#include "stdafx.h"

#include "SpecularDebugRayQueryPass.h"

#include <pix3.h>

namespace Engine
{

void RecordSpecularDebugRayQueryPass(ID3D12GraphicsCommandList* commandList, const SpecularDebugRayQueryPassDesc& desc)
{
    PIXBeginEvent(commandList, 0, L"SpecularDebugRayQueryPass");

    commandList->SetComputeRootSignature(desc.rootSignature);
    commandList->SetPipelineState(desc.pipelineState);
    commandList->SetComputeRootUnorderedAccessView(0, desc.resultUav);
    commandList->SetComputeRootDescriptorTable(1, desc.tlasSrv);
    commandList->SetComputeRoot32BitConstants(2, 8, &desc.constants, 0);
    commandList->Dispatch(1, 1, 1);

    PIXEndEvent(commandList);
}

} // namespace Engine
