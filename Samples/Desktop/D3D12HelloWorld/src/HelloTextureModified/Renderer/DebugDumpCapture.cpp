#include "stdafx.h"

#include "DebugDumpCapture.h"

#include <DirectXPackedVector.h>
#include <pix3.h>

namespace Engine
{

bool DebugDumpReadback::IsValid() const
{
    return resource != nullptr;
}

void DebugDumpReadback::Reset()
{
    resource.Reset();
    layout = {};
}

bool DebugDumpCapture::IsReady() const
{
    return lightPass.IsValid() && backBuffer.IsValid();
}

void DebugDumpCapture::Reset()
{
    lightPass.Reset();
    backBuffer.Reset();
}

void CreateDebugDumpReadback(ID3D12Device* device, ID3D12Resource* source, DebugDumpReadback& readback)
{
    D3D12_RESOURCE_DESC desc = source->GetDesc();
    UINT numRows = 0;
    UINT64 rowSizeInBytes = 0;
    UINT64 totalBytes = 0;
    device->GetCopyableFootprints(&desc, 0, 1, 0, &readback.layout, &numRows, &rowSizeInBytes, &totalBytes);

    ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
                                                  D3D12_HEAP_FLAG_NONE,
                                                  &CD3DX12_RESOURCE_DESC::Buffer(totalBytes),
                                                  D3D12_RESOURCE_STATE_COPY_DEST,
                                                  nullptr,
                                                  IID_PPV_ARGS(&readback.resource)));
}

void RecordDebugDumpCapture(ID3D12GraphicsCommandList* commandList,
                            ID3D12Device* device,
                            ID3D12Resource* lightPassSource,
                            ID3D12Resource* backBufferSource,
                            DebugDumpCapture& capture)
{
    PIXBeginEvent(commandList, 0, L"DebugDump");

    CreateDebugDumpReadback(device, lightPassSource, capture.lightPass);
    CreateDebugDumpReadback(device, backBufferSource, capture.backBuffer);

    CD3DX12_TEXTURE_COPY_LOCATION lightDst(capture.lightPass.resource.Get(), capture.lightPass.layout);
    CD3DX12_TEXTURE_COPY_LOCATION lightSrc(lightPassSource, 0);
    commandList->CopyTextureRegion(&lightDst, 0, 0, 0, &lightSrc, nullptr);

    CD3DX12_TEXTURE_COPY_LOCATION backBufferDst(capture.backBuffer.resource.Get(), capture.backBuffer.layout);
    CD3DX12_TEXTURE_COPY_LOCATION backBufferSrc(backBufferSource, 0);
    commandList->CopyTextureRegion(&backBufferDst, 0, 0, 0, &backBufferSrc, nullptr);

    PIXEndEvent(commandList);
}

namespace
{

void MapDebugDumpReadback(DebugDumpReadback& readback, DebugDumpMappedReadback& mappedReadback)
{
    const D3D12_RANGE readRange = {0, static_cast<SIZE_T>(readback.resource->GetDesc().Width)};
    ThrowIfFailed(readback.resource->Map(0, &readRange, reinterpret_cast<void**>(&mappedReadback.data)));

    mappedReadback.width = static_cast<UINT>(readback.layout.Footprint.Width);
    mappedReadback.height = readback.layout.Footprint.Height;
    mappedReadback.offset = readback.layout.Offset;
    mappedReadback.rowPitch = readback.layout.Footprint.RowPitch;
}

void UnmapDebugDumpReadback(DebugDumpReadback& readback)
{
    const D3D12_RANGE writtenRange = {0, 0};
    readback.resource->Unmap(0, &writtenRange);
}

} // namespace

void MapDebugDumpCapture(DebugDumpCapture& capture, DebugDumpMappedCapture& mappedCapture)
{
    assert(capture.IsReady());

    MapDebugDumpReadback(capture.lightPass, mappedCapture.lightPass);
    MapDebugDumpReadback(capture.backBuffer, mappedCapture.backBuffer);
}

void UnmapDebugDumpCapture(DebugDumpCapture& capture)
{
    assert(capture.IsReady());

    UnmapDebugDumpReadback(capture.lightPass);
    UnmapDebugDumpReadback(capture.backBuffer);
}

DebugDumpLightSample ReadLightPassDebugSample(const DebugDumpMappedReadback& readback, UINT x, UINT y)
{
    const UINT8* row = readback.data + readback.offset + static_cast<size_t>(y) * readback.rowPitch;
    const UINT16* half = reinterpret_cast<const UINT16*>(row + static_cast<size_t>(x) * 8);

    return {DirectX::PackedVector::XMConvertHalfToFloat(half[0]),
            DirectX::PackedVector::XMConvertHalfToFloat(half[1]),
            DirectX::PackedVector::XMConvertHalfToFloat(half[2]),
            DirectX::PackedVector::XMConvertHalfToFloat(half[3])};
}

DebugDumpBackBufferSample ReadBackBufferDebugSample(const DebugDumpMappedReadback& readback, UINT x, UINT y)
{
    const UINT8* row = readback.data + readback.offset + static_cast<size_t>(y) * readback.rowPitch;
    const UINT raw = *reinterpret_cast<const UINT*>(row + static_cast<size_t>(x) * 4);

    return {raw,
            static_cast<float>(raw & 0x3ff) / 1023.0f,
            static_cast<float>((raw >> 10) & 0x3ff) / 1023.0f,
            static_cast<float>((raw >> 20) & 0x3ff) / 1023.0f,
            static_cast<float>((raw >> 30) & 0x3) / 3.0f};
}

} // namespace Engine
