#pragma once

#include "../DXSampleHelper.h"
#include "ResolvedRenderTargets.h"

namespace Engine
{

struct SceneGeometryDrawDesc
{
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
    D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
    bool usesIndexedDraw = false;
    UINT vertexCountPerInstance = 0;
    UINT indexCountPerInstance = 0;
    UINT instanceCount = 0;
};

struct ForwardPassDesc
{
    ResolvedRenderTargets renderTargets;
    SceneGeometryDrawDesc geometryDraw = {};
};

void RecordSceneGeometryDraw(ID3D12GraphicsCommandList* commandList, const SceneGeometryDrawDesc& drawDesc);
void RecordDepthPrePass(ID3D12GraphicsCommandList* commandList, const SceneGeometryDrawDesc& drawDesc);
void RecordForwardPass(ID3D12GraphicsCommandList* commandList, const ForwardPassDesc& passDesc);

} // namespace Engine
