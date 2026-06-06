#include "stdafx.h"

#include "ToneMap.h"

#include "FullscreenTriangle.h"

#include <pix3.h>

namespace Engine
{

auto ToneMapPass::MakeShaderConstants(const HdrOutputSettings& hdrOutputSettings) const
    -> ToneMapSettings::ShaderConstants
{
    return settings.MakeShaderConstants(hdrOutputSettings.TransferFunction());
}

void RecordToneMapPass(ID3D12GraphicsCommandList* commandList)
{
    PIXBeginEvent(commandList, 0, L"ToneMapPass");

    DrawFullscreenTriangle(commandList);

    PIXEndEvent(commandList);
}

} // namespace Engine
