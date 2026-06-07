#pragma once

#include "../DXSampleHelper.h"

namespace Engine
{
namespace RootSignatureLayout
{

enum RootParameterIndex
{
    // Scene texture SRVs used by material shading.
    TextureTable = 0,

    // Scene geometry data shared by GBuffer, depth, and forward/main passes.
    InstanceSrv,
    MaterialSrv,
    CameraConstants,

    // GBuffer debug and lighting pass inputs.
    GBufferSrvBase,

    // Lighting pass constants.
    LightConstants,

    // Debug pass controls.
    GBufferDebugConstants,

    // Tone mapping pass inputs and controls.
    ToneMapSceneColor,
    ToneMapConstants,

    Count
};

static constexpr UINT kBaseRegister = 0;

// SRV descriptor tables.
static constexpr UINT kTextureSrvSpace = 0;
static constexpr UINT kInstanceSrvSpace = 1;
static constexpr UINT kMaterialSrvSpace = 2;
static constexpr UINT kGBufferSrvSpace = 3;
static constexpr UINT kToneMapSceneColorSrvSpace = 4;

// Per-frame and per-pass CBVs.
static constexpr UINT kCameraCbvRegister = 0;
static constexpr UINT kCameraCbvSpace = 0;
static constexpr UINT kLightCbvRegister = 2;
static constexpr UINT kLightCbvSpace = 0;

// Root constants.
static constexpr UINT kGBufferDebugConstantsRegister = 1;
static constexpr UINT kGBufferDebugConstantsSpace = 0;
static constexpr UINT kGBufferDebugConstantsCount = 1;

static constexpr UINT kToneMapConstantsRegister = 3;
static constexpr UINT kToneMapConstantsSpace = 0;
static constexpr UINT kToneMapConstantsCount = 5;

// Static texture sampler.
static constexpr UINT kStaticSamplerRegister = 0;
static constexpr UINT kStaticSamplerSpace = 0;

} // namespace RootSignatureLayout
} // namespace Engine
