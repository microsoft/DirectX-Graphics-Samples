#include "stdafx.h"

#include "ShadowMaskDebugPass.h"

#include "FullscreenTriangle.h"

#include <pix3.h>

namespace Engine
{

void RecordShadowMaskDebugPass(ID3D12GraphicsCommandList* commandList)
{
    PIXBeginEvent(commandList, 0, L"ShadowMaskDebugPass");

    DrawFullscreenTriangle(commandList);

    PIXEndEvent(commandList);
}

} // namespace Engine
