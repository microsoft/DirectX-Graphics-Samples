//*********************************************************
//
// Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "SampleUI.h"
#include "DXSampleHelper.h"
#include <ResourceUploadBatch.h>
#include <RenderTargetState.h>
#include <DirectXColors.h>
#include <d3dx12.h>
#include <algorithm>
#include <cwchar>

using namespace DirectX;

void SampleUI::CreateDeviceResources(ID3D12Device* device, ID3D12CommandQueue* queue, const std::filesystem::path& font)
{
    D3D12_DESCRIPTOR_HEAP_DESC description{};
    description.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    description.NumDescriptors = 2;
    description.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&description, IID_PPV_ARGS(&m_heap)), "CreateDescriptorHeap(UI)");
    m_graphicsMemory = std::make_unique<GraphicsMemory>(device);
    ResourceUploadBatch upload(device);
    upload.Begin();
    SpriteBatchPipelineStateDescription pipeline(RenderTargetState(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN));
    m_spriteBatch = std::make_unique<SpriteBatch>(device, upload, pipeline);
    m_font = std::make_unique<SpriteFont>(device, upload, font.c_str(), m_heap->GetCPUDescriptorHandleForHeapStart(),
                                          m_heap->GetGPUDescriptorHandleForHeapStart());
    m_font->SetDefaultCharacter(L'?');
    // One white texel lets SpriteBatch draw a translucent backing for text.
    const auto texture = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, 1);
    const CD3DX12_HEAP_PROPERTIES heap(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &texture, D3D12_RESOURCE_STATE_COPY_DEST,
                                                  nullptr, IID_PPV_ARGS(&m_background)),
                  "CreateResource(UI backing)");
    const uint32_t white = 0xffffffff;
    const D3D12_SUBRESOURCE_DATA data{&white, sizeof(white), sizeof(white)};
    upload.Upload(m_background.Get(), 0, &data, 1);
    upload.Transition(m_background.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    const UINT stride = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    auto cpu = m_heap->GetCPUDescriptorHandleForHeapStart();
    cpu.ptr += stride;
    m_backgroundView = m_heap->GetGPUDescriptorHandleForHeapStart();
    m_backgroundView.ptr += stride;
    device->CreateShaderResourceView(m_background.Get(), nullptr, cpu);
    upload.End(queue).get();
}

void SampleUI::ReleaseDeviceResources()
{
    m_font.reset();
    m_spriteBatch.reset();
    m_graphicsMemory.reset();
    m_background.Reset();
    m_heap.Reset();
}

void SampleUI::BeginFrame(const Keyboard::KeyboardStateTracker& keys, bool shift)
{
    if (keys.IsKeyPressed(Keyboard::F1))
        m_showHelp = !m_showHelp;
    if (keys.IsKeyPressed(Keyboard::F2))
        m_editing = !m_editing;
    if (m_editing && keys.IsKeyPressed(Keyboard::Tab))
    {
        m_page = (m_page + (shift ? 3 : 1)) % 4;
        m_selection = 0;
        m_previousItemCount = 0;
    }
    if (m_editing && keys.IsKeyPressed(Keyboard::Down))
        ++m_selection;
    if (m_editing && keys.IsKeyPressed(Keyboard::Up))
        --m_selection;
    m_selection = std::clamp(m_selection, 0, std::max(0, m_previousItemCount - 1));
    m_adjust = m_editing ? int(keys.IsKeyPressed(Keyboard::Right)) - int(keys.IsKeyPressed(Keyboard::Left)) : 0;
    m_activate = m_editing && keys.IsKeyPressed(Keyboard::Enter);
    m_multiplier = shift ? 10 : 1;
    m_lines.clear();
    m_itemCount = 0;
}

void SampleUI::Text(const std::wstring& text)
{
    m_lines.push_back({text, false});
}

bool SampleUI::Item(const std::wstring& label)
{
    const bool selected = m_editing && m_itemCount++ == m_selection;
    m_lines.push_back({(selected ? L"> " : L"  ") + label, selected});
    return selected;
}

bool SampleUI::Button(const wchar_t* label)
{
    return Item(label) && m_activate;
}

void SampleUI::Toggle(const wchar_t* label, bool& value)
{
    if (Item(std::wstring(label) + (value ? L": on" : L": off")) && (m_activate || m_adjust))
        value = !value;
}

void SampleUI::Integer(const wchar_t* label, int& value, int minimum, int maximum, int step)
{
    if (Item(std::wstring(label) + L": " + std::to_wstring(value)))
        value = std::clamp(value + m_adjust * step * m_multiplier, minimum, maximum);
}

void SampleUI::Float(const wchar_t* label, float& value, float minimum, float maximum, float step)
{
    wchar_t text[160];
    swprintf_s(text, L"%s: %.2f", label, value);
    if (Item(text))
        value = std::clamp(value + m_adjust * step * m_multiplier, minimum, maximum);
}

void SampleUI::Render(ID3D12GraphicsCommandList* commands, UINT width, UINT height)
{
    m_previousItemCount = m_itemCount;
    ID3D12DescriptorHeap* heaps[] = {m_heap.Get()};
    commands->SetDescriptorHeaps(1, heaps);
    const D3D12_VIEWPORT viewport{0, 0, float(width), float(height), 0, 1};
    const D3D12_RECT scissor{0, 0, LONG(width), LONG(height)};
    commands->RSSetViewports(1, &viewport);
    commands->RSSetScissorRects(1, &scissor);
    m_spriteBatch->SetViewport(viewport);
    m_spriteBatch->Begin(commands);
    const float scale =
        std::min({1.0f, float(width) / 1000.0f,
                  float(height - 24) / std::max(1.0f, float(m_lines.size()) * m_font->GetLineSpacing())});
    float textWidth = 0;
    for (const auto& line : m_lines)
        textWidth = std::max(textWidth, XMVectorGetX(m_font->MeasureString(line.text.c_str())));
    const RECT panel{4, 4, LONG(std::min(float(width), textWidth * scale + 24)),
                     LONG(float(m_lines.size()) * m_font->GetLineSpacing() * scale + 20)};
    m_spriteBatch->Draw(m_backgroundView, XMUINT2(1, 1), panel, XMVectorSet(0, 0, 0, 0.7f));
    XMFLOAT2 position(12, 12);
    for (const auto& line : m_lines)
    {
        m_font->DrawString(m_spriteBatch.get(), line.text.c_str(), XMFLOAT2(position.x + 1, position.y + 1),
                           Colors::Black, 0, XMFLOAT2(0, 0), scale);
        m_font->DrawString(m_spriteBatch.get(), line.text.c_str(), position,
                           line.selected ? Colors::Yellow : Colors::White, 0, XMFLOAT2(0, 0), scale);
        position.y += m_font->GetLineSpacing() * scale;
    }
    m_spriteBatch->End();
}

void SampleUI::Commit(ID3D12CommandQueue* queue)
{
    m_graphicsMemory->Commit(queue);
}
