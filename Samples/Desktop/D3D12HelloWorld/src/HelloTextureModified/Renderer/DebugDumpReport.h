#pragma once

#include "DebugDumpCapture.h"
#include "HdrOutput.h"
#include "ToneMap.h"

namespace Engine
{

struct DebugDumpReportDesc
{
    DebugDumpMappedCapture mappedCapture;
    HdrOutputSettings hdrOutputSettings;
    ToneMapSettings toneMapSettings;
};

void PrintDebugDumpReport(const DebugDumpReportDesc& desc);

} // namespace Engine
