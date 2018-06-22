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
#include "DxSampleHelper.h"
#include "UILayer.h"

using namespace std;

UILayer::UILayer(UINT frameCount, ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue) :
    m_width(0.0f),
    m_height(0.0f)
{
    m_wrappedRenderTargets.resize(frameCount);
    m_d2dRenderTargets.resize(frameCount);
    m_textBlocks.resize(1);
    Initialize(pDevice, pCommandQueue);
}

void UILayer::Initialize(ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue)
{
    UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};

#if 0 // defined(_DEBUG) || defined(DBG)
    // Enable the D2D debug layer.
    d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;

    // Enable the D3D11 debug layer.
    d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // Create an 11 device wrapped around the 12 device and share
    // 12's command queue.
    ComPtr<ID3D11Device> d3d11Device;
    ID3D12CommandQueue* ppCommandQueues[] = { pCommandQueue };
    ThrowIfFailed(D3D11On12CreateDevice(
        pDevice,
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

#if defined(_DEBUG) || defined(DBG)
    // Filter a debug error coming from the 11on12 layer.
    ComPtr<ID3D12InfoQueue> infoQueue;
    if (SUCCEEDED(pDevice->QueryInterface(IID_PPV_ARGS(&infoQueue))))
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

        ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_dwriteFactory));
    }
}

void UILayer::UpdateLabels(const wstring& uiText)
{
    // Update the UI elements.
    m_textBlocks[0] = { uiText, D2D1::RectF(0.0f, 0.0f, m_width, m_height), m_textFormat.Get() };
}

void UILayer::Render(UINT frameIndex)
{
    ID3D11Resource* ppResources[] = { m_wrappedRenderTargets[frameIndex].Get() };

    m_d2dDeviceContext->SetTarget(m_d2dRenderTargets[frameIndex].Get());

    // Acquire our wrapped render target resource for the current back buffer.
    m_d3d11On12Device->AcquireWrappedResources(ppResources, _countof(ppResources));
    m_d2dDeviceContext->BeginDraw();
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
    for (UINT i = 0; i < FrameCount(); i++)
    {
        ID3D11Resource* ppResources[] = { m_wrappedRenderTargets[i].Get() };
        m_d3d11On12Device->ReleaseWrappedResources(ppResources, _countof(ppResources));
    }
    m_d2dDeviceContext->SetTarget(nullptr);
    m_d3d11DeviceContext->Flush();
    for (UINT i = 0; i < FrameCount(); i++)
    {
        m_d2dRenderTargets[i].Reset();
        m_wrappedRenderTargets[i].Reset();
    }
    m_textBrush.Reset();
    m_d2dDeviceContext.Reset();
    m_textFormat.Reset();
    m_dwriteFactory.Reset();
    m_d2dDevice.Reset();
    m_d2dFactory.Reset();
    m_d3d11DeviceContext.Reset();
    m_d3d11On12Device.Reset();
}

void UILayer::Resize(ComPtr<ID3D12Resource>* ppRenderTargets, UINT width, UINT height)
{
    m_width = static_cast<float>(width);
    m_height = static_cast<float>(height);

    // Query the desktop's dpi settings, which will be used to create
    // D2D's render targets.
    D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));

    // Create a wrapped 11On12 resource of this back buffer. Since we are 
    // rendering all D3D12 content first and then all D2D content, we specify 
    // the In resource state as RENDER_TARGET - because D3D12 will have last 
    // used it in this state - and the Out resource state as PRESENT. When 
    // ReleaseWrappedResources() is called on the 11On12 device, the resource 
    // will be transitioned to the PRESENT state.
    for (UINT i = 0; i < FrameCount(); i++)
    {
        D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
        ThrowIfFailed(m_d3d11On12Device->CreateWrappedResource(
            ppRenderTargets[i].Get(),
            &d3d11Flags,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT,
            IID_PPV_ARGS(&m_wrappedRenderTargets[i])));

        // Create a render target for D2D to draw directly to this back buffer.
        ComPtr<IDXGISurface> surface;
        ThrowIfFailed(m_wrappedRenderTargets[i].As(&surface));
        ThrowIfFailed(m_d2dDeviceContext->CreateBitmapFromDxgiSurface(
            surface.Get(),
            &bitmapProperties,
            &m_d2dRenderTargets[i]));
    }

    ThrowIfFailed(m_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_d2dDeviceContext));
    m_d2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    ThrowIfFailed(m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_textBrush));

    // Create DWrite text format objects.
    const float fontSize = m_height / 30.0f;
    const float smallFontSize = m_height / 40.0f;

    ThrowIfFailed(m_dwriteFactory->CreateTextFormat(
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
    ThrowIfFailed(m_dwriteFactory->CreateTextFormat(
        L"Arial",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        smallFontSize,
        L"en-us",
        &m_textFormat));
}