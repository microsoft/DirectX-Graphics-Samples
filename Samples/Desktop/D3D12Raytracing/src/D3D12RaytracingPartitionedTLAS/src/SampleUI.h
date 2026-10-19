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

#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <Keyboard.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <GraphicsMemory.h>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

// DirectXTK text overlay. The small keyboard menu
// keeps the scene controls separate from the PTLAS teaching code.
class SampleUI
{
public:
    void CreateDeviceResources(ID3D12Device* device, ID3D12CommandQueue* queue, const std::filesystem::path& font);
    void ReleaseDeviceResources();
    void BeginFrame(const DirectX::Keyboard::KeyboardStateTracker& keys, bool shift);
    void Text(const std::wstring& text);
    bool Button(const wchar_t* label);
    void Toggle(const wchar_t* label, bool& value);
    void Integer(const wchar_t* label, int& value, int minimum, int maximum, int step = 1);
    void Float(const wchar_t* label, float& value, float minimum, float maximum, float step);
    void Render(ID3D12GraphicsCommandList* commands, UINT width, UINT height);
    void Commit(ID3D12CommandQueue* queue);
    bool IsEditing() const { return m_editing; }
    bool ShowHelp() const { return m_showHelp; }
    int GetPage() const { return m_page; }

private:
    bool Item(const std::wstring& label);
    struct Line
    {
        std::wstring text;
        bool selected;
    };
    std::vector<Line> m_lines;
    int m_page{};
    int m_selection{};
    int m_itemCount{};
    int m_previousItemCount{};
    int m_adjust{};
    int m_multiplier{1};
    bool m_activate{};
    bool m_editing{};
    bool m_showHelp{};
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_background;
    D3D12_GPU_DESCRIPTOR_HANDLE m_backgroundView{};
    std::unique_ptr<DirectX::GraphicsMemory> m_graphicsMemory;
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
    std::unique_ptr<DirectX::SpriteFont> m_font;
};
