#pragma once

#include "ResolvedRenderTargets.h"

namespace Engine
{

void RecordClearPass(ID3D12GraphicsCommandList* commandList, const ResolvedRenderTargets& renderTargets);

} // namespace Engine
