#include "stdafx.h"

#include "HdrOutput.h"

#include <algorithm>

using Microsoft::WRL::ComPtr;

namespace
{
UINT16 Hdr10Chromaticity(float value)
{
    return static_cast<UINT16>(value * 50000.0f + 0.5f);
}

int ComputeIntersectionArea(const RECT& a, const RECT& b)
{
    return static_cast<int>((std::max)(0L, (std::min)(a.right, b.right) - (std::max)(a.left, b.left)) *
                            (std::max)(0L, (std::min)(a.bottom, b.bottom) - (std::max)(a.top, b.top)));
}
} // namespace

void HdrOutputPolicy::Update(IDXGIFactory4* dxgiFactory, IDXGISwapChain3* swapChain, HWND hwnd)
{
    const bool hdr10Enabled = CheckCurrentOutputHdr10Support(dxgiFactory, hwnd) &&
                              CheckSwapChainColorSpaceSupport(swapChain, kHdr10ColorSpace);
    const bool hdr10StateChanged = settings.hdr10Enabled != hdr10Enabled;
    settings.hdr10Enabled = hdr10Enabled;

    ApplySwapChainColorSpace(swapChain, settings.TargetColorSpace());
    if (hdr10StateChanged)
    {
        ApplyHdr10Metadata(swapChain, hdr10Enabled);
    }
}

void HdrOutputPolicy::ReapplyColorSpace(IDXGISwapChain3* swapChain)
{
    ApplySwapChainColorSpace(swapChain, settings.TargetColorSpace());
}

bool HdrOutputPolicy::CheckSwapChainColorSpaceSupport(IDXGISwapChain3* swapChain,
                                                      DXGI_COLOR_SPACE_TYPE colorSpace) const
{
    UINT colorSpaceSupport = 0;
    return SUCCEEDED(swapChain->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport)) &&
           (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT);
}

bool HdrOutputPolicy::CheckCurrentOutputHdr10Support(IDXGIFactory4* dxgiFactory, HWND hwnd) const
{
    RECT windowRect = {};
    GetWindowRect(hwnd, &windowRect);

    ComPtr<IDXGIAdapter1> adapter;
    ThrowIfFailed(dxgiFactory->EnumAdapters1(0, &adapter));

    ComPtr<IDXGIOutput> bestOutput;
    ComPtr<IDXGIOutput> output;
    int bestIntersectArea = -1;

    for (UINT outputIndex = 0; adapter->EnumOutputs(outputIndex, &output) != DXGI_ERROR_NOT_FOUND; ++outputIndex)
    {
        DXGI_OUTPUT_DESC outputDesc = {};
        ThrowIfFailed(output->GetDesc(&outputDesc));

        const int intersectArea = ComputeIntersectionArea(windowRect, outputDesc.DesktopCoordinates);
        if (intersectArea > bestIntersectArea)
        {
            bestOutput = output;
            bestIntersectArea = intersectArea;
        }

        output.Reset();
    }

    if (!bestOutput)
    {
        return false;
    }

    ComPtr<IDXGIOutput6> output6;
    if (FAILED(bestOutput.As(&output6)))
    {
        return false;
    }

    DXGI_OUTPUT_DESC1 outputDesc = {};
    ThrowIfFailed(output6->GetDesc1(&outputDesc));
    return outputDesc.ColorSpace == kHdr10ColorSpace;
}

void HdrOutputPolicy::ApplySwapChainColorSpace(IDXGISwapChain3* swapChain, DXGI_COLOR_SPACE_TYPE colorSpace)
{
    if (settings.currentSwapChainColorSpace == colorSpace)
    {
        return;
    }

    if (CheckSwapChainColorSpaceSupport(swapChain, colorSpace))
    {
        ThrowIfFailed(swapChain->SetColorSpace1(colorSpace));
        settings.currentSwapChainColorSpace = colorSpace;
    }
}

void HdrOutputPolicy::ApplyHdr10Metadata(IDXGISwapChain3* swapChain, bool enabled) const
{
    ComPtr<IDXGISwapChain4> swapChain4;
    if (FAILED(swapChain->QueryInterface(IID_PPV_ARGS(&swapChain4))))
    {
        return;
    }

    if (!enabled)
    {
        ThrowIfFailed(swapChain4->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_NONE, 0, nullptr));
        return;
    }

    DXGI_HDR_METADATA_HDR10 metadata = {};
    metadata.RedPrimary[0] = Hdr10Chromaticity(0.708f);
    metadata.RedPrimary[1] = Hdr10Chromaticity(0.292f);
    metadata.GreenPrimary[0] = Hdr10Chromaticity(0.170f);
    metadata.GreenPrimary[1] = Hdr10Chromaticity(0.797f);
    metadata.BluePrimary[0] = Hdr10Chromaticity(0.131f);
    metadata.BluePrimary[1] = Hdr10Chromaticity(0.046f);
    metadata.WhitePoint[0] = Hdr10Chromaticity(0.3127f);
    metadata.WhitePoint[1] = Hdr10Chromaticity(0.3290f);
    metadata.MaxMasteringLuminance = kHdr10MaxMasteringLuminance;
    metadata.MinMasteringLuminance = kHdr10MinMasteringLuminance;
    metadata.MaxContentLightLevel = kHdr10MaxContentLightLevel;
    metadata.MaxFrameAverageLightLevel = kHdr10MaxFrameAverageLightLevel;

    ThrowIfFailed(swapChain4->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(metadata), &metadata));
}
