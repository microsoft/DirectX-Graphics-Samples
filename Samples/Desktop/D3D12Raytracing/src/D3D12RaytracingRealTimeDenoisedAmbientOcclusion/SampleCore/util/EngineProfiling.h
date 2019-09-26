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

#include <string>
#include "EngineTuning.h"

class CommandContext;

namespace EngineProfiling
{
    extern BoolVar DrawFrameRate;
    extern BoolVar DrawProfiler;
    extern BoolVar DrawCpuTime;

    void RestoreDevice(ID3D12Device5* Device, ID3D12CommandQueue* CommandQueue, UINT MaxFrameCount, UINT MaxNumTimers = 100);
    void ReleaseDevice();

    void Update();
    void BeginFrame(ID3D12GraphicsCommandList4* CommandList);
    void EndFrame(ID3D12GraphicsCommandList4* CommandList);

    void BeginBlock(const std::wstring& name, ID3D12GraphicsCommandList4* CommandList = nullptr);
    void EndBlock(ID3D12GraphicsCommandList4* CommandList = nullptr);

    void DisplayFrameRate(std::wstringstream& Text, UINT indent);
    void Display(std::wstringstream& text, UINT indent, bool expandAllNodes = false);
    bool IsPaused();
}


class ScopedTimer
{
public:
    ScopedTimer(const std::wstring& name, ID3D12GraphicsCommandList4* CommandList = nullptr) : m_commandList(CommandList)
    {
        EngineProfiling::BeginBlock(name, m_commandList);
    }
    ScopedTimer(const std::wstring& name, UINT indexSuffix, ID3D12GraphicsCommandList4* CommandList = nullptr) : m_commandList(CommandList)
    {
        std::wstringstream wstr;
        wstr << name << L" " << indexSuffix;
        EngineProfiling::BeginBlock(wstr.str(), m_commandList);
    }

    ~ScopedTimer()
    {
        EngineProfiling::EndBlock(m_commandList);
    }



private:
    ID3D12GraphicsCommandList4* m_commandList;
};
