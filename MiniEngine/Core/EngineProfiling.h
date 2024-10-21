//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#pragma once

#include <string>
#include "TextRenderer.h"

//===============================================================================
// desc: This is the heart of profiling in the engine, utilising WinPix. We want to feed this to ImGui through the use of accessors
// modified: Aliyaan Zulfiqar
//===============================================================================
#include "AZB_Utils.h"

/*
   Change Log:

   [AZB] 21/10/24: Implemented accessors to pass performance metrics from app to ImGui 
*/


class CommandContext;

namespace EngineProfiling
{
    void Update();

    void BeginBlock(const std::wstring& name, CommandContext* Context = nullptr);
    void EndBlock(CommandContext* Context = nullptr);

    void DisplayFrameRate(TextContext& Text);
    void DisplayPerfGraph(GraphicsContext& Text);
    void Display(TextContext& Text, float x, float y, float w, float h);
    bool IsPaused();

// [AZB]: Acessors to performance metrics
#if AZB_MOD
    const float GetCPUTime();
    const float GetGPUTime();
    const float GetFrameRate();
#endif
}

#ifdef RELEASE
class ScopedTimer
{
public:
    ScopedTimer(const std::wstring&) {}
    ScopedTimer(const std::wstring&, CommandContext&) {}
};
#else
class ScopedTimer
{
public:
    ScopedTimer( const std::wstring& name ) : m_Context(nullptr)
    {
        EngineProfiling::BeginBlock(name);
    }
    ScopedTimer( const std::wstring& name, CommandContext& Context ) : m_Context(&Context)
    {
        EngineProfiling::BeginBlock(name, m_Context);
    }
    ~ScopedTimer()
    {
        EngineProfiling::EndBlock(m_Context);
    }

private:
    CommandContext* m_Context;
};
#endif
