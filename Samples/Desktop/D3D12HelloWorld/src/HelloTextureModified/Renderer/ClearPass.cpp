#include "stdafx.h"

#include "ClearPass.h"

#include <pix3.h>

namespace Engine
{

void RecordClearPass(ID3D12GraphicsCommandList* commandList, const ResolvedRenderTargets& renderTargets)
{
    PIXBeginEvent(commandList, 0, L"ClearPrepass");
    assert(!renderTargets.rtvs.empty());
    assert(renderTargets.dsv.has_value());
    assert(renderTargets.clearColor != nullptr);

    for (D3D12_CPU_DESCRIPTOR_HANDLE rtv : renderTargets.rtvs)
    {
        commandList->ClearRenderTargetView(rtv, renderTargets.clearColor, 0, nullptr);
    }

    commandList->ClearDepthStencilView(renderTargets.dsv.value(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    PIXEndEvent(commandList);
}

} // namespace Engine
