#pragma once

#include "../DXSampleHelper.h"

namespace Engine
{

struct DebugDumpReadback
{
    ComPtr<ID3D12Resource> resource;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout = {};

    bool IsValid() const;
    void Reset();
};

struct DebugDumpCapture
{
    DebugDumpReadback lightPass;
    DebugDumpReadback backBuffer;

    bool IsReady() const;
    void Reset();
};

struct DebugDumpMappedReadback
{
    UINT8* data = nullptr;
    UINT width = 0;
    UINT height = 0;
    UINT64 offset = 0;
    UINT rowPitch = 0;
};

struct DebugDumpMappedCapture
{
    DebugDumpMappedReadback lightPass;
    DebugDumpMappedReadback backBuffer;
};

struct DebugDumpLightSample
{
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 0.0f;
};

struct DebugDumpBackBufferSample
{
    UINT raw = 0;
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 0.0f;
};

void CreateDebugDumpReadback(ID3D12Device* device, ID3D12Resource* source, DebugDumpReadback& readback);
void RecordDebugDumpCapture(ID3D12GraphicsCommandList* commandList,
                            ID3D12Device* device,
                            ID3D12Resource* lightPassSource,
                            ID3D12Resource* backBufferSource,
                            DebugDumpCapture& capture);
void MapDebugDumpCapture(DebugDumpCapture& capture, DebugDumpMappedCapture& mappedCapture);
void UnmapDebugDumpCapture(DebugDumpCapture& capture);
DebugDumpLightSample ReadLightPassDebugSample(const DebugDumpMappedReadback& readback, UINT x, UINT y);
DebugDumpBackBufferSample ReadBackBufferDebugSample(const DebugDumpMappedReadback& readback, UINT x, UINT y);

} // namespace Engine
