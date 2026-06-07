#include "stdafx.h"

#include "DebugDumpReport.h"

#include "../MyDx12Utils.h"

#include <cmath>

namespace Engine
{
namespace
{

struct ExpectedLightDebugGradientSample
{
    float sceneLinear = 0.0f;
    float nits = 0.0f;
};

float SrgbToLinear(float value)
{
    value = (std::clamp)(value, 0.0f, 1.0f);
    return value <= 0.04045f ? value / 12.92f : std::pow((value + 0.055f) / 1.055f, 2.4f);
}

auto ComputeExpectedLightDebugGradientSample(UINT width,
                                             UINT height,
                                             UINT x,
                                             UINT y,
                                             const ToneMapSettings& toneMapSettings) -> ExpectedLightDebugGradientSample
{
    // This expected value mirrors shaders_LightPassDebugGradient.hlsl PSMain:
    // bottom half is SDR[0,1], top half is HDR[0,9], both encoded as a perceptual ramp
    // and returned as scene-linear RGB. Keep this in sync with the shader when its debug
    // gradient changes. The shader's anti-aliased display-max marker is reported separately.
    const float expectedU = width > 0 ? (static_cast<float>(x) + 0.5f) / width : 0.0f;
    const float expectedV = height > 0 ? (static_cast<float>(y) + 0.5f) / height : 0.0f;
    const float expectedRampInput = (std::clamp)(expectedU, 0.0f, 1.0f);
    const float expectedMaxLinear = expectedV < 0.5f ? 1.0f : 9.0f;
    const float expectedPerceptualMax = std::pow(expectedMaxLinear, 1.0f / 2.2f);
    const float expectedPerceptualValue = expectedRampInput * expectedPerceptualMax;
    const float expectedSceneLinear = std::pow(expectedPerceptualValue, 2.2f);

    return {expectedSceneLinear, expectedSceneLinear * toneMapSettings.paperWhiteNits};
}

} // namespace

void PrintDebugDumpReport(const DebugDumpReportDesc& desc)
{
    const UINT lightWidth = desc.mappedCapture.lightPass.width;
    const UINT lightHeight = desc.mappedCapture.lightPass.height;
    const UINT backBufferWidth = desc.mappedCapture.backBuffer.width;
    const UINT backBufferHeight = desc.mappedCapture.backBuffer.height;
    const UINT sampleYs[] = {lightHeight > 0 ? lightHeight / 4 : 0, lightHeight > 0 ? (lightHeight * 3) / 4 : 0};
    const char* bandNames[] = {"SDR[0,1]", "HDR[0,9]"};
    const UINT bandCount = 2;
    const UINT sampleXs[] = {0, lightWidth / 4, lightWidth / 2, lightWidth > 0 ? lightWidth - 1 : 0};
    const char* sampleNames[] = {"left", "25%", "50%", "right"};

    DebugPrint("HDR Gradient Validation: compare expected LightPass gradient with GPU readback.\n");
    DebugPrint("  LightPass=%ux%u BackBuffer=%ux%u hdr10=%d toneMap=%d exposure=%.2f paperWhite=%.0f "
               "maxDisplay=%.0f\n",
               lightWidth,
               lightHeight,
               backBufferWidth,
               backBufferHeight,
               desc.hdrOutputSettings.hdr10Enabled ? 1 : 0,
               desc.toneMapSettings.operatorIndex,
               desc.toneMapSettings.exposure,
               desc.toneMapSettings.paperWhiteNits,
               desc.toneMapSettings.maxDisplayNits);
    const float paperWhiteNits = (std::max)(desc.toneMapSettings.paperWhiteNits, 1.0f);
    const float displayMaxSceneLinear = (std::max)(desc.toneMapSettings.maxDisplayNits, paperWhiteNits) / paperWhiteNits;
    const float displayMaxMarkerX = std::pow((std::clamp)(displayMaxSceneLinear / 9.0f, 0.0f, 1.0f), 1.0f / 2.2f);
    DebugPrint("  HDR[0,9] display-max marker: sceneLinear=%.3f nits=%.0f x=%.3f%s\n",
               displayMaxSceneLinear,
               desc.toneMapSettings.maxDisplayNits,
               displayMaxMarkerX,
               displayMaxSceneLinear >= 9.0f ? " (outside ramp)" : "");
    DebugPrint("\nLightPass expected vs readback\n");
    DebugPrint("  %-8s %-5s %5s %5s %8s %8s %8s %8s %8s\n",
               "band",
               "pos",
               "x",
               "y",
               "expected",
               "readback",
               "diff",
               "diff%",
               "nits");

    for (UINT band = 0; band < bandCount; ++band)
    {
        const UINT sampleY = sampleYs[band];
        const char* bandName = bandNames[band];

        for (UINT i = 0; i < _countof(sampleXs); ++i)
        {
            const UINT lightX = (std::min)(sampleXs[i], lightWidth > 0 ? lightWidth - 1 : 0);
            const ExpectedLightDebugGradientSample expectedSample =
                ComputeExpectedLightDebugGradientSample(lightWidth, lightHeight, lightX, sampleY, desc.toneMapSettings);
            const DebugDumpLightSample lightSample =
                ReadLightPassDebugSample(desc.mappedCapture.lightPass, lightX, sampleY);
            const float diff = lightSample.r - expectedSample.sceneLinear;
            const float diffPercent =
                std::abs(expectedSample.sceneLinear) > 0.0001f ? (diff / expectedSample.sceneLinear) * 100.0f : 0.0f;

            DebugPrint("  %-8s %-5s %5u %5u %8.3f %8.3f %+8.3f %+7.1f%% %8.1f\n",
                       bandName,
                       sampleNames[i],
                       lightX,
                       sampleY,
                       expectedSample.sceneLinear,
                       lightSample.r,
                       diff,
                       diffPercent,
                       expectedSample.nits);
        }
    }

    DebugPrint("\nBackBuffer encoded readback\n");
    if (desc.hdrOutputSettings.hdr10Enabled)
    {
        DebugPrint("  %-8s %-5s %5s %5s %8s %8s %10s\n",
                   "band",
                   "pos",
                   "x",
                   "y",
                   "encoded",
                   "pqNits",
                   "raw");
    }
    else
    {
        DebugPrint("  %-8s %-5s %5s %5s %8s %8s %8s %8s %10s\n",
                   "band",
                   "pos",
                   "x",
                   "y",
                   "encoded",
                   "linear",
                   "nits",
                   "alpha",
                   "raw");
    }

    for (UINT band = 0; band < bandCount; ++band)
    {
        const UINT sampleY = sampleYs[band];
        const char* bandName = bandNames[band];

        for (UINT i = 0; i < _countof(sampleXs); ++i)
        {
            const UINT lightX = (std::min)(sampleXs[i], lightWidth > 0 ? lightWidth - 1 : 0);
            const UINT backBufferX = backBufferWidth > 0 ? (std::min)(lightX, backBufferWidth - 1) : 0;
            const UINT backBufferY = backBufferHeight > 0 ? (std::min)(sampleY, backBufferHeight - 1) : 0;

            const DebugDumpBackBufferSample backBufferSample =
                ReadBackBufferDebugSample(desc.mappedCapture.backBuffer, backBufferX, backBufferY);

            if (desc.hdrOutputSettings.hdr10Enabled)
            {
                DebugPrint("  %-8s %-5s %5u %5u %8.3f %8.1f 0x%08x\n",
                           bandName,
                           sampleNames[i],
                           backBufferX,
                           backBufferY,
                           backBufferSample.r,
                           St2084PqToNits(backBufferSample.r),
                           backBufferSample.raw);
            }
            else
            {
                const float sdrLinearR = SrgbToLinear(backBufferSample.r);
                DebugPrint("  %-8s %-5s %5u %5u %8.3f %8.3f %8.1f %8.3f 0x%08x\n",
                           bandName,
                           sampleNames[i],
                           backBufferX,
                           backBufferY,
                           backBufferSample.r,
                           sdrLinearR,
                           sdrLinearR * desc.toneMapSettings.paperWhiteNits,
                           backBufferSample.a,
                           backBufferSample.raw);
            }
        }
    }
}

} // namespace Engine
