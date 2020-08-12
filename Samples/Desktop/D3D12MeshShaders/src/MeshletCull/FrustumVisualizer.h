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

//
// Helper class which draws a visualization of the view frustum against which meshlets are culled.
//
class FrustumVisualizer
{
public:
    FrustumVisualizer();

    void CreateDeviceResources(ID3D12Device2* device, DXGI_FORMAT rtFormat, DXGI_FORMAT dsFormat);
    void ReleaseResources();

    void Update(DirectX::FXMMATRIX viewProj, DirectX::XMVECTOR(&planes)[6]);
    void Draw(ID3D12GraphicsCommandList6* cmdList);

    _declspec(align(256u))
    struct Constants
    {
        DirectX::XMFLOAT4X4 ViewProj;
        DirectX::XMFLOAT4   Planes[6];
        DirectX::XMFLOAT4   LineColor;
    };

private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;

    Microsoft::WRL::ComPtr<ID3D12Resource>      m_constantResource;
    void*                                       m_constantData;

    uint32_t                                    m_frameIndex;
};

