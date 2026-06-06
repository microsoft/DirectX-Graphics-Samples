#include "stdafx.h"

#include "LightingPass.h"

#include "FullscreenTriangle.h"

#include <pix3.h>

namespace Engine
{

void RecordLightingPass(ID3D12GraphicsCommandList* commandList)
{
    PIXBeginEvent(commandList, 0, L"LightPass");

    DrawFullscreenTriangle(commandList);

    PIXEndEvent(commandList);
}

void RecordLightingDebugGradientPass(ID3D12GraphicsCommandList* commandList)
{
    PIXBeginEvent(commandList, 0, L"RecordLightPassDebugGradient");

    DrawFullscreenTriangle(commandList);

    PIXEndEvent(commandList);
}

} // namespace Engine
