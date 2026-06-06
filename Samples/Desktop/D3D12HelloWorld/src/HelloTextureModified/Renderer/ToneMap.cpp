#include "stdafx.h"

#include "ToneMap.h"

auto ToneMapPass::MakeShaderConstants(const HdrOutputSettings& hdrOutputSettings) const
    -> ToneMapSettings::ShaderConstants
{
    return settings.MakeShaderConstants(hdrOutputSettings.TransferFunction());
}
