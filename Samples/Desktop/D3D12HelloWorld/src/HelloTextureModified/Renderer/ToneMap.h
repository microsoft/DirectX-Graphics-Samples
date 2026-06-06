#pragma once

#include "../DXSampleHelper.h"
#include "HdrOutput.h"

#include <algorithm>

static constexpr UINT kDefaultToneMapOperator = 0; // 0: None, 1: Reinhard, 2: ACES
static constexpr float kDefaultExposure = 1.0f;
static constexpr float kDefaultPaperWhiteNits = 300.0f;
static constexpr float kDefaultMaxDisplayNits = 1000.0f;

struct ToneMapSettings
{
    struct ShaderConstants
    {
        UINT toneMapOperator;
        UINT transferFunction;
        float exposure;
        float paperWhiteNits;
        float maxDisplayNits;
    };

    int operatorIndex = kDefaultToneMapOperator;
    float exposure = kDefaultExposure;
    float paperWhiteNits = kDefaultPaperWhiteNits;
    float maxDisplayNits = kDefaultMaxDisplayNits;

    void Normalize()
    {
        operatorIndex = std::clamp(operatorIndex, 0, 2);
        exposure = (std::max)(exposure, 0.0f);
        paperWhiteNits = (std::max)(paperWhiteNits, 1.0f);
        maxDisplayNits = (std::max)(maxDisplayNits, paperWhiteNits);
    }

    ShaderConstants MakeShaderConstants(UINT transferFunction) const
    {
        return {static_cast<UINT>(operatorIndex), transferFunction, exposure, paperWhiteNits, maxDisplayNits};
    }
};

struct ToneMapPass
{
    ToneMapSettings settings;

    ToneMapSettings::ShaderConstants MakeShaderConstants(const HdrOutputSettings& hdrOutputSettings) const;
};
