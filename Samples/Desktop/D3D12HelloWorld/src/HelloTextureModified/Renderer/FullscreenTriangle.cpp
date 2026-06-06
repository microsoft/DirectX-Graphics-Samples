#include "stdafx.h"

#include "FullscreenTriangle.h"

namespace Engine
{

void DrawFullscreenTriangle(ID3D12GraphicsCommandList* commandList)
{
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);
}

} // namespace Engine
