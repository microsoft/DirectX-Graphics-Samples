#include "stdafx.h"

#include "SceneGeometryPass.h"

#include <pix3.h>

namespace Engine
{

void RecordSceneGeometryDraw(ID3D12GraphicsCommandList* commandList, const SceneGeometryDrawDesc& drawDesc)
{
    if (drawDesc.instanceCount == 0)
    {
        return;
    }

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &drawDesc.vertexBufferView);

    if (drawDesc.usesIndexedDraw)
    {
        commandList->IASetIndexBuffer(&drawDesc.indexBufferView);
        commandList->DrawIndexedInstanced(drawDesc.indexCountPerInstance, drawDesc.instanceCount, 0, 0, 0);
    }
    else
    {
        commandList->DrawInstanced(drawDesc.vertexCountPerInstance, drawDesc.instanceCount, 0, 0);
    }
}

void RecordDepthPrePass(ID3D12GraphicsCommandList* commandList, const SceneGeometryDrawDesc& drawDesc)
{
    PIXBeginEvent(commandList, 0, L"DepthPrepass");

    RecordSceneGeometryDraw(commandList, drawDesc);

    PIXEndEvent(commandList);
}

void RecordForwardPass(ID3D12GraphicsCommandList* commandList, const ForwardPassDesc& passDesc)
{
    PIXBeginEvent(commandList, 0, L"ForwardPass");

    if (passDesc.renderTargets.clearColor != nullptr)
    {
        for (D3D12_CPU_DESCRIPTOR_HANDLE rtv : passDesc.renderTargets.rtvs)
        {
            commandList->ClearRenderTargetView(rtv, passDesc.renderTargets.clearColor, 0, nullptr);
        }
    }

    RecordSceneGeometryDraw(commandList, passDesc.geometryDraw);

    PIXEndEvent(commandList);
}

} // namespace Engine
