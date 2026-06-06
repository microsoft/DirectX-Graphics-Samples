#pragma once

#include "../DXSampleHelper.h"

static constexpr DXGI_COLOR_SPACE_TYPE kHdr10ColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
static constexpr DXGI_COLOR_SPACE_TYPE kSdrColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
static constexpr UINT kHdr10TransferFunction = 1; // 0: Linear, 1: ST.2084 PQ, 2: HLG
static constexpr UINT kSdrTransferFunction = 3;   // 0: Linear, 1: ST.2084 PQ, 2: HLG, 3: SDR Rec.709
static constexpr UINT kHdr10MaxMasteringLuminance = 1000;
static constexpr UINT kHdr10MinMasteringLuminance = 10; // 0.001 nits in 0.0001 nit units.
static constexpr UINT16 kHdr10MaxContentLightLevel = 1000;
static constexpr UINT16 kHdr10MaxFrameAverageLightLevel = 400;

struct HdrOutputSettings
{
    DXGI_COLOR_SPACE_TYPE currentSwapChainColorSpace = DXGI_COLOR_SPACE_CUSTOM;
    bool hdr10Enabled = false;

    DXGI_COLOR_SPACE_TYPE TargetColorSpace() const
    {
        return hdr10Enabled ? kHdr10ColorSpace : kSdrColorSpace;
    }
    UINT TransferFunction() const
    {
        return hdr10Enabled ? kHdr10TransferFunction : kSdrTransferFunction;
    }
};

struct HdrOutputPolicy
{
    HdrOutputSettings settings;

    // HDR output is evaluated here because it bridges swap chain state and tone map constants.
    bool CheckSwapChainColorSpaceSupport(IDXGISwapChain3* swapChain, DXGI_COLOR_SPACE_TYPE colorSpace) const;
    bool CheckCurrentOutputHdr10Support(IDXGIFactory4* dxgiFactory, HWND hwnd) const;
    void ApplySwapChainColorSpace(IDXGISwapChain3* swapChain, DXGI_COLOR_SPACE_TYPE colorSpace);
    void ApplyHdr10Metadata(IDXGISwapChain3* swapChain, bool enabled) const;
    void Update(IDXGIFactory4* dxgiFactory, IDXGISwapChain3* swapChain, HWND hwnd);
    void ReapplyColorSpace(IDXGISwapChain3* swapChain);
};
