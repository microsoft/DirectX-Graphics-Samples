//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#pragma once

#include "Model.h"

//
// Helper class which draws a visualization of the view frustum against which meshlets are culled.
//
class CullDataVisualizer
{
public:
    CullDataVisualizer();

    void CreateDeviceResources(ID3D12Device2* device, DXGI_FORMAT rtFormat, DXGI_FORMAT dsFormat);
    void ReleaseResources();

    void SetConstants(DirectX::FXMMATRIX world, DirectX::CXMMATRIX view, DirectX::CXMMATRIX proj, DirectX::CXMVECTOR color);
    void Draw(ID3D12GraphicsCommandList6* commandList, const Mesh& mesh, uint32_t offset, uint32_t count);

private:
    _declspec(align(256u))
    struct Constants
    {
        DirectX::XMFLOAT4X4 World;
        DirectX::XMFLOAT4X4 ViewProj;
        DirectX::XMFLOAT4 Color;
        DirectX::XMFLOAT4 ViewUp;
        DirectX::XMFLOAT3 ViewForward;
        float Scale;
        uint32_t MeshletOffset;
        uint32_t MeshletCount;
    };

private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_boundingSpherePso;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_normalConePso;
    
    Microsoft::WRL::ComPtr<ID3D12Resource>      m_constantResource;
    void*                                       m_constantData;

    uint32_t                                    m_frameIndex;
};
