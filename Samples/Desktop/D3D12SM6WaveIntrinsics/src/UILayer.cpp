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

#include "stdafx.h"
#include "UILayer.h"

UILayer::UILayer(D3D12SM6WaveIntrinsics* pSample) :
    m_pSample(pSample),
    m_selectedRenderMode(1)
{
    m_textBlocks.resize(1);
    m_labels.resize(LabelCount);
    
    m_labels[RenderModeSelection] = L"Press 1~9 to switch render mode.\n";
    m_labels[RenderMode1] = L"1. Normal render.\n";
    m_labels[RenderMode2] = L"2. Color pixels by lane indices.\n";
    m_labels[RenderMode3] = L"3. Show first lane (white dot) in each wave.\n";
    m_labels[RenderMode4] = L"4. Show first(white dot) and last(red dot) lanes in each wave.\n";
    m_labels[RenderMode5] = L"5. Color pixels by active lane ratio (white = 100%; black = 0%).\n";
    m_labels[RenderMode6] = L"6. Broadcast the color of the first active lane to the wave.\n";
    m_labels[RenderMode7] = L"7. Average the color in a wave.\n";
    m_labels[RenderMode8] = L"8. Color pixels by prefix sum of distance between current and first lane.\n";
    m_labels[RenderMode9] = L"9. Color pixels by their quad id.\n";

    Initialize();
}

void UILayer::Initialize()
{
    UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};

#if defined(_DEBUG)
    // Enable the D2D debug layer.
    d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
    // Enable the D3D11 debug layer.
    d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // Create an 11 device wrapped around the 12 device and share
    // 12's command queue.
    ComPtr<ID3D11Device> d3d11Device;
    ID3D12CommandQueue* ppCommandQueues[] = { m_pSample->GetCommandQueue() };
    ThrowIfFailed(D3D11On12CreateDevice(
        m_pSample->GetDevice(),
        d3d11DeviceFlags,
        nullptr,
        0,
        reinterpret_cast<IUnknown**>(ppCommandQueues),
        _countof(ppCommandQueues),
        0,
        &d3d11Device,
        &m_d3d11DeviceContext,
        nullptr
    ));

    // Query the 11On12 device from the 11 device.
    ThrowIfFailed(d3d11Device.As(&m_d3d11On12Device));

#if defined(_DEBUG)
    // Filter a debug error coming from the 11on12 layer.
    ComPtr<ID3D12InfoQueue> infoQueue;
    if (SUCCEEDED(m_pSample->GetDevice()->QueryInterface(IID_PPV_ARGS(&infoQueue))))
    {
        // Suppress messages based on their severity level.
        D3D12_MESSAGE_SEVERITY severities[] =
        {
            D3D12_MESSAGE_SEVERITY_INFO,
        };

        // Suppress individual messages by their ID.
        D3D12_MESSAGE_ID denyIds[] =
        {
            // This occurs when there are uninitialized descriptors in a descriptor table, even when a
            // shader does not access the missing descriptors.
            D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,
        };

        D3D12_INFO_QUEUE_FILTER filter = {};
        filter.DenyList.NumSeverities = _countof(severities);
        filter.DenyList.pSeverityList = severities;
        filter.DenyList.NumIDs = _countof(denyIds);
        filter.DenyList.pIDList = denyIds;
        ThrowIfFailed(infoQueue->PushStorageFilter(&filter));
    }
#endif

    // Create D2D/DWrite components.
    {
        ComPtr<IDXGIDevice> dxgiDevice;
        ThrowIfFailed(m_d3d11On12Device.As(&dxgiDevice));
        ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, &m_d2dFactory));
        ThrowIfFailed(m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice));
        ThrowIfFailed(m_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_d2dDeviceContext));
        m_d2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
        ThrowIfFailed(m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_textBrush));
        ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_dWriteFactory));
    }
    UpdateLabels();
    Resize();
}

void UILayer::UpdateLabels(UINT newSelectedRenderMode)
{
    // remove "[x]" from the end of current selected label
    size_t pos = m_labels[m_selectedRenderMode].find_last_of(L"[");
    if (pos != std::wstring::npos)
    {
        m_labels[m_selectedRenderMode].erase(pos, 3);
    }

    // insert "[x]" to the new selected label
    pos = m_labels[newSelectedRenderMode].find_last_of(L'\n');
    if (pos != std::wstring::npos)
    {
        m_labels[newSelectedRenderMode].insert(pos, L"[x]");
    }

    m_selectedRenderMode = newSelectedRenderMode;

    // Update the UI elements.
    float width = static_cast<float>(m_pSample->GetWidth());
    float height = static_cast<float>(m_pSample->GetHeight());
    std::wstring uiText = L"";
    for (auto s : m_labels)
    {
        uiText += s;
    }
    m_textBlocks[0] = { uiText, D2D1::RectF(0.0f, 0.0f, width, height), m_textFormat.Get() };
}

void UILayer::Render()
{
    ID3D11Resource* ppResources[] = { m_wrappedRenderTarget.Get() };

    // Acquire our wrapped render target resource for the current back buffer.
    m_d3d11On12Device->AcquireWrappedResources(ppResources, _countof(ppResources));
    m_d2dDeviceContext->BeginDraw();
    D2D1_COLOR_F clearColor = { 0,0,0,0 };
    m_d2dDeviceContext->Clear(clearColor);
    for (auto textBlock : m_textBlocks)
    {
        m_d2dDeviceContext->DrawText(
            textBlock.text.c_str(),
            static_cast<UINT>(textBlock.text.length()),
            textBlock.pFormat,
            textBlock.layout,
            m_textBrush.Get());
    }

    m_d2dDeviceContext->EndDraw();

    // Release our wrapped render target resource. Releasing
    // transitions the back buffer resource to the state specified
    // as the OutState when the wrapped resource was created.
    m_d3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));

    // Flush to submit the 11 command list to the shared command queue.
    m_d3d11DeviceContext->Flush();
}


// Releases resources that reference the DXGI swap chain so that it can be resized.
void UILayer::ReleaseResources()
{
    m_wrappedRenderTarget.Reset();
    m_d2dRenderTarget.Reset();
    m_d2dDeviceContext->SetTarget(nullptr);
}


void UILayer::Resize()
{
    // Query the desktop's dpi settings, which will be used to create
    // D2D's render targets.
    D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));

    // Create a wrapped 11On12 resource of this back buffer.
    // When ReleaseWrappedResources() is called on the 11On12 device, the resource
    // will be transitioned to the PIXEL_SHADER_RESOURCE state.
    D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
    ThrowIfFailed(m_d3d11On12Device->CreateWrappedResource(
        m_pSample->GetUIRenderTarget(),
        &d3d11Flags,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        IID_PPV_ARGS(&m_wrappedRenderTarget)));

    // Create a render target for D2D to draw directly to this back buffer.
    ComPtr<IDXGISurface> surface;
    ThrowIfFailed(m_wrappedRenderTarget.As(&surface));
    ThrowIfFailed(m_d2dDeviceContext->CreateBitmapFromDxgiSurface(
        surface.Get(),
        &bitmapProperties,
        &m_d2dRenderTarget));

    // Set the render target on the D2D context.
    m_d2dDeviceContext->SetTarget(m_d2dRenderTarget.Get());

    // Create DWrite text format objects.
    float height = static_cast<float>(m_pSample->GetHeight());
    const float fontSize = height / 30.0f;
    const float smallFontSize = height / 40.0f;

    ThrowIfFailed(m_dWriteFactory->CreateTextFormat(
        L"Arial",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        fontSize,
        L"en-us",
        &m_textFormat));

    ThrowIfFailed(m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
    ThrowIfFailed(m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
    ThrowIfFailed(m_dWriteFactory->CreateTextFormat(
        L"Arial",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        smallFontSize,
        L"en-us",
        &m_textFormat));
}