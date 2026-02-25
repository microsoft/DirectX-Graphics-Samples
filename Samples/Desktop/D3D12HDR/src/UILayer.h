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

#include "D3D12HDR.h"

class UILayer
{
public:
    UILayer(D3D12HDR* pSample);

    void UpdateLabels();
    void Render();
    void ReleaseResources();
    void Resize();

private:
    struct TextBlock
    {
        std::wstring text;
        D2D1_RECT_F layout;
        IDWriteTextFormat* pFormat;
    };

    enum Strings
    {
        Format = 0,
        Signal,
        HDRSupport,
        StandardGradient,
        BrightGradient,
        Rec709,
        Rec2020,
        ChangeFormat,
        ChangeCurve,
        HideUI,
        StringsCount
    };

    D3D12HDR* m_pSample;
    ComPtr<ID3D11DeviceContext> m_d3d11DeviceContext;
    ComPtr<ID3D11On12Device> m_d3d11On12Device;
    ComPtr<IDWriteFactory> m_dwriteFactory;
    ComPtr<ID2D1Factory3> m_d2dFactory;
    ComPtr<ID2D1Device2> m_d2dDevice;
    ComPtr<ID2D1DeviceContext2> m_d2dDeviceContext;
    ComPtr<ID3D11Resource> m_wrappedRenderTarget;
    ComPtr<ID2D1Bitmap1> m_d2dRenderTarget;
    ComPtr<ID2D1SolidColorBrush> m_textBrush;
    ComPtr<IDWriteTextFormat> m_textFormat;
    ComPtr<IDWriteTextFormat> m_smallTextFormat;

    std::vector<std::wstring> m_labels;
    std::vector<TextBlock> m_ui;

    void Initialize();
};

