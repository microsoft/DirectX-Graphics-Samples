#pragma once

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

