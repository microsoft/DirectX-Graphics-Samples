#pragma once

#include "DXSampleHelper.h"

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

namespace MyDx12Util
{

inline void CreateUploadBuffer(ID3D12Device* device, size_t size, ComPtr<ID3D12Resource>& ppResource)
{
    ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                  D3D12_HEAP_FLAG_NONE,
                                                  &CD3DX12_RESOURCE_DESC::Buffer(size),
                                                  D3D12_RESOURCE_STATE_GENERIC_READ,
                                                  nullptr,
                                                  IID_PPV_ARGS(&ppResource)));
}

} // namespace MyDx12Util

inline void DebugPrint(const char* fmt, ...)
{
    char buf[1024];

    va_list args;
    va_start(args, fmt);
    vsprintf_s(buf, fmt, args);
    va_end(args);

    OutputDebugStringA(buf);
}
#define DBG_PRINT(fmt, ...) DebugPrint("[%s:%d] " fmt, __FILE__, __LINE__, __VA_ARGS__)
