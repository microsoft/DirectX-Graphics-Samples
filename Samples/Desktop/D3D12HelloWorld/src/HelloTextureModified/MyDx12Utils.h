#pragma once

#include <initializer_list>

namespace MyDx12Util {

    inline void CreateUploadBuffer(ComPtr<ID3D12Device>& device, size_t size, ComPtr<ID3D12Resource>& ppResource)
    {
        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(size),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&ppResource)));

    }

    inline void ClearRenderTargetFormats(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc)
    {
        for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        {
            desc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
        }
    }

    inline void SetRenderTargetFormats(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, const DXGI_FORMAT* formats,
        UINT formatCount)
    {
        ClearRenderTargetFormats(desc);

        desc.NumRenderTargets = formatCount;
        for (UINT i = 0; i < formatCount; ++i)
        {
            desc.RTVFormats[i] = formats[i];
        }
    }

    inline void SetRenderTargetFormats(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc,
        std::initializer_list<DXGI_FORMAT> formats)
    {
        SetRenderTargetFormats(desc, formats.begin(), static_cast<UINT>(formats.size()));
    }

    inline D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateGBufferPSODesc(
        const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
        const void* vs,
        UINT vsSize,
        const void* ps,
        UINT psSize,
        const DXGI_FORMAT* rtvFormats,
        UINT rtvFormatCount)
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = baseDesc;

        desc.VS = CD3DX12_SHADER_BYTECODE(vs, vsSize);
        desc.PS = CD3DX12_SHADER_BYTECODE(ps, psSize);
        SetRenderTargetFormats(desc, rtvFormats, rtvFormatCount);

        return desc;
    }

    inline D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateFullscreenPassPSODesc(
        const D3D12_GRAPHICS_PIPELINE_STATE_DESC& baseDesc,
        const void* vs,
        UINT vsSize,
        const void* ps,
        UINT psSize,
        DXGI_FORMAT rtvFormat)
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = baseDesc;

        desc.InputLayout = {};
        desc.VS = CD3DX12_SHADER_BYTECODE(vs, vsSize);
        desc.PS = CD3DX12_SHADER_BYTECODE(ps, psSize);
        desc.DepthStencilState.DepthEnable = FALSE;
        desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
        SetRenderTargetFormats(desc, { rtvFormat });

        return desc;
    }

}


inline void DebugPrint(const char* fmt, ...)
{
    char buf[1024];

    va_list args;
    va_start(args, fmt);
    vsprintf_s(buf, fmt, args);
    va_end(args);

    OutputDebugStringA(buf);
}
#define DBG_PRINT(fmt, ...) \
DebugPrint("[%s:%d] " fmt, __FILE__, __LINE__, __VA_ARGS__)

