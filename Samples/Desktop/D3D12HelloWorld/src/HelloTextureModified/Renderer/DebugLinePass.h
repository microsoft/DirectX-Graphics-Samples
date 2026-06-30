#pragma once

#include "../DXSampleHelper.h"
#include <d3d12.h>
#include <d3dx12_core.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <vector>

namespace Engine
{

struct DebugLineVertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};

static constexpr UINT kMaxDebugLines = 1024;
static constexpr UINT kMaxDebugVertices = kMaxDebugLines * 2;

class DebugLinePass
{
public:
    DebugLinePass() = default;
    ~DebugLinePass() = default;

    DebugLinePass(const DebugLinePass&) = delete;
    DebugLinePass& operator=(const DebugLinePass&) = delete;

    void Create(
        ID3D12Device* device,
        D3D12_SHADER_BYTECODE vs,
        D3D12_SHADER_BYTECODE ps);

    void UpdateLines(
        const std::vector<DebugLineVertex>& vertices,
        ID3D12GraphicsCommandList* commandList);

    void RecordDraw(
        ID3D12GraphicsCommandList* commandList,
        D3D12_GPU_VIRTUAL_ADDRESS viewProjCbv) const;

    ID3D12RootSignature* RootSignature() const { return m_rootSignature.Get(); }

private:
    void CreateRootSignature(ID3D12Device* device);
    void CreatePipelineState(ID3D12Device* device, D3D12_SHADER_BYTECODE vs, D3D12_SHADER_BYTECODE ps);
    void CreateVertexBuffer(ID3D12Device* device);

    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};
    UINT m_vertexCount = 0;
};

} // namespace Engine
