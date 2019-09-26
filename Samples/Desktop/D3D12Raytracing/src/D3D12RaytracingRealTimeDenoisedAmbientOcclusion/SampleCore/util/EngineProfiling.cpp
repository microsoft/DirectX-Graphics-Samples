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
#include "GameInput.h"
#include "PerformanceTimers.h"
#include "EngineTuning.h"
#include "GpuTimeManager.h"
#include <vector>
#include <unordered_map>
#include <array>

using namespace std;
using namespace DX;

namespace EngineProfiling
{
    bool Paused = false;
}

namespace
{
    wstring Indent(UINT spaces)
    {
        wstring s;
        return s.append(spaces, L' ');
    }
}

class GpuTimer
{
public:

    GpuTimer::GpuTimer()
    {
        m_TimerIndex = GpuTimeManager::instance().NewTimer();
    }

    void Start(ID3D12GraphicsCommandList4* CommandList)
    {
        GpuTimeManager::instance().Start(CommandList, m_TimerIndex);
    }

    void Stop(ID3D12GraphicsCommandList4* CommandList)
    {
        GpuTimeManager::instance().Stop(CommandList, m_TimerIndex);
    }

    float GpuTimer::GetElapsedMS(void)
    {
        return GpuTimeManager::instance().GetElapsedMS(m_TimerIndex);
    }

    float GetAverageMS() const
    {
        return GpuTimeManager::instance().GetAverageMS(m_TimerIndex);
    }

    UINT GetTimerIndex(void)
    {
        return m_TimerIndex;
    }
private:

    UINT m_TimerIndex;
};

class NestedTimingTree
{
public:
    NestedTimingTree(const wstring& name, NestedTimingTree* parent = nullptr)
        : m_Name(name), m_Parent(parent), m_IsExpanded(false) {}

    NestedTimingTree* GetChild(const wstring& name)
    {
        auto iter = m_LUT.find(name);
        if (iter != m_LUT.end())
            return iter->second;

        NestedTimingTree* node = new NestedTimingTree(name, this);
        m_Children.push_back(node);
        m_LUT[name] = node;
        return node;
    }

    NestedTimingTree* NextScope(void)
    {
        if (m_IsExpanded && m_Children.size() > 0)
            return m_Children[0];

        return m_Parent->NextChild(this);
    }

    NestedTimingTree* PrevScope(void)
    {
        NestedTimingTree* prev = m_Parent->PrevChild(this);
        return prev == m_Parent ? prev : prev->LastChild();
    }

    NestedTimingTree* FirstChild(void)
    {
        return m_Children.size() == 0 ? nullptr : m_Children[0];
    }

    NestedTimingTree* LastChild(void)
    {
        if (!m_IsExpanded || m_Children.size() == 0)
            return this;

        return m_Children.back()->LastChild();
    }

    NestedTimingTree* NextChild(NestedTimingTree* curChild)
    {
        assert(curChild->m_Parent == this);

        for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
        {
            if (*iter == curChild)
            {
                auto nextChild = iter; ++nextChild;
                if (nextChild != m_Children.end())
                    return *nextChild;
            }
        }

        if (m_Parent != nullptr)
            return m_Parent->NextChild(this);
        else
            return &sm_RootScope;
    }

    NestedTimingTree* PrevChild(NestedTimingTree* curChild)
    {
        assert(curChild->m_Parent == this);

        if (*m_Children.begin() == curChild)
        {
            if (this == &sm_RootScope)
                return sm_RootScope.LastChild();
            else
                return this;
        }

        for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
        {
            if (*iter == curChild)
            {
                auto prevChild = iter; --prevChild;
                return *prevChild;
            }
        }

        return nullptr;
    }

    void StartTiming(ID3D12GraphicsCommandList4* CommandList)
    {
        m_CpuTimer.Start();
        if (CommandList == nullptr)
            return;

        m_GpuTimer.Start(CommandList);

        PIXBeginEvent(CommandList, 0, m_Name.c_str());
    }

    void StopTiming(ID3D12GraphicsCommandList4* CommandList)
    {
        m_CpuTimer.Stop();
        m_CpuTimer.Update();
        if (CommandList == nullptr)
            return;

        m_GpuTimer.Stop(CommandList);

        PIXEndEvent(CommandList);
    }

    void GatherTimes(UINT FrameIndex)
    {
        if (EngineProfiling::Paused)
        {
            for (auto node : m_Children)
                node->GatherTimes(FrameIndex);
            return;
        }

        for (auto node : m_Children)
            node->GatherTimes(FrameIndex);

        m_StartTick = 0;
        m_EndTick = 0;
    }

    static void PushProfilingMarker(const wstring& name, ID3D12GraphicsCommandList4* CommandList);
    static void PopProfilingMarker(ID3D12GraphicsCommandList4* CommandList);
    static void Update(void);

    float GetAverageCpuTimeMS(void) const { return m_CpuTimer.GetAverageMS(); }
    float GetAverageGpuTimeMS(void) const { return m_GpuTimer.GetAverageMS(); }


    static void Display(wstringstream& Text, UINT indent, bool expandAllNodes)
    {
        sm_RootScope.DisplayNode(Text, indent, expandAllNodes);
    }

    static const NestedTimingTree& Root() { return sm_RootScope; }

    const vector<NestedTimingTree*>& Children() const { return m_Children; }
    const wstring& Name() const { return m_Name; }
    static CPUTimer& FrameCpuTimer() { return sm_FrameCpuTimer; }
    static CPUTimer& FrameToFrameCpuTimer() { return sm_FrameToFrameCpuTimer; }
    static GpuTimer& FrameGpuTimer() { return sm_FrameGpuTimer; }
private:

    void DisplayNode(wstringstream& Text, UINT indent, bool expandAllNodes);
    void DeleteChildren(void)
    {
        for (auto node : m_Children)
            delete node;
        m_Children.clear();
    }

    wstring m_Name;
    NestedTimingTree* m_Parent;
    vector<NestedTimingTree*> m_Children;
    unordered_map<wstring, NestedTimingTree*> m_LUT;
    int64_t m_StartTick;
    int64_t m_EndTick;
    bool m_IsExpanded;
    CPUTimer m_CpuTimer;
    GpuTimer m_GpuTimer;
    static NestedTimingTree sm_RootScope;
    static NestedTimingTree* sm_CurrentNode;
    static NestedTimingTree* sm_SelectedScope;
    static CPUTimer sm_FrameToFrameCpuTimer;
    static CPUTimer sm_FrameCpuTimer;
    static GpuTimer sm_FrameGpuTimer;
};

NestedTimingTree NestedTimingTree::sm_RootScope(L"");
NestedTimingTree* NestedTimingTree::sm_CurrentNode = &NestedTimingTree::sm_RootScope;
NestedTimingTree* NestedTimingTree::sm_SelectedScope = &NestedTimingTree::sm_RootScope;
CPUTimer NestedTimingTree::sm_FrameToFrameCpuTimer;
CPUTimer NestedTimingTree::sm_FrameCpuTimer;
GpuTimer NestedTimingTree::sm_FrameGpuTimer;

namespace EngineProfiling
{
    BoolVar DrawFrameRate(L"Display Frame Rate", true);
    BoolVar DrawProfiler(L"Display Profiler (on,then backspace)", false);
    BoolVar DrawCpuTime(L"Display CPU Times", false);

    void Update(void)
    {
        if (GameInput::IsFirstPressed(GameInput::kStartButton)
            || GameInput::IsFirstPressed(GameInput::kKey_space))
        {
            Paused = !Paused;
        }
    }
    
    void BeginFrame(ID3D12GraphicsCommandList4* CommandList)
    {
        static bool isFirstFrame = true;
        if (!isFirstFrame)
        {
            NestedTimingTree::FrameToFrameCpuTimer().Stop();
            NestedTimingTree::FrameToFrameCpuTimer().Update();
        }
        isFirstFrame = false;
        NestedTimingTree::FrameToFrameCpuTimer().Start();
        NestedTimingTree::FrameCpuTimer().Start();
        

        GpuTimeManager::instance().BeginFrame(CommandList);

        NestedTimingTree::FrameGpuTimer().Start(CommandList);
    }

    void EndFrame(ID3D12GraphicsCommandList4* CommandList)
    {
        NestedTimingTree::FrameCpuTimer().Stop();
        NestedTimingTree::FrameCpuTimer().Update();
        NestedTimingTree::FrameGpuTimer().Stop(CommandList);
        GpuTimeManager::instance().EndFrame(CommandList);
    }

    void RestoreDevice(ID3D12Device5* Device, ID3D12CommandQueue* CommandQueue, UINT MaxFrameCount, UINT MaxNumTimers)
    {
        GpuTimeManager::instance().RestoreDevice(Device, CommandQueue, MaxFrameCount, MaxNumTimers);
    }

    void ReleaseDevice()
    {
        GpuTimeManager::instance().ReleaseDevice();
    }

    void BeginBlock(const wstring& name, ID3D12GraphicsCommandList4* CommandList)
    {
        NestedTimingTree::PushProfilingMarker(name, CommandList);
    }

    void EndBlock(ID3D12GraphicsCommandList4* CommandList)
    {
        NestedTimingTree::PopProfilingMarker(CommandList);
    }

    bool IsPaused()
    {
        return Paused;
    }

    void DisplayFrameRate(wstringstream& Text, UINT indent)
    {
        if (!DrawFrameRate)
            return;


        float cpuTime = NestedTimingTree::FrameCpuTimer().GetAverageMS();
        float cpuFrameToFrameTime = NestedTimingTree::FrameToFrameCpuTimer().GetAverageMS();
        float gpuTime = NestedTimingTree::FrameGpuTimer().GetAverageMS();
        float frameRate = 1e3f / cpuFrameToFrameTime;

        streamsize prevPrecision = Text.precision(3);
        Text << Indent(indent);
        if (DrawCpuTime)
        {
            Text << L"CPU " << cpuTime << L"ms, ";
        }
        Text << L"GPU " << gpuTime << L"ms, ";

        Text.width(3);
        Text << (UINT)(frameRate + 0.5f) << L" FPS\n";

        Text.precision(prevPrecision);
    }

    void Display(wstringstream& Text, UINT indent, bool expandAllNodes)
    {
        if (DrawProfiler)
        {
            NestedTimingTree::Update();

            Text << Indent(indent) << L"Engine Profiling (use arrow keys) "
                << L"           ";
            if (DrawCpuTime)
            {
                Text << L"CPU[ms]    ";
            }
            Text << L"GPU[ms]\n";

            NestedTimingTree::Display(Text, indent, expandAllNodes);
        }
    }

} // EngineProfiling

void NestedTimingTree::PushProfilingMarker(const wstring& name, ID3D12GraphicsCommandList4* CommandList)
{
    sm_CurrentNode = sm_CurrentNode->GetChild(name);
    sm_CurrentNode->StartTiming(CommandList);
}

void NestedTimingTree::PopProfilingMarker(ID3D12GraphicsCommandList4* CommandList)
{
    sm_CurrentNode->StopTiming(CommandList);
    sm_CurrentNode = sm_CurrentNode->m_Parent;
}

void NestedTimingTree::Update(void)
{
    assert(sm_SelectedScope != nullptr && L"Corrupted profiling data structure");

    if (sm_SelectedScope == &sm_RootScope)
    {
        sm_SelectedScope = sm_RootScope.FirstChild();
        if (sm_SelectedScope == &sm_RootScope)
            return;
    }

    if (GameInput::IsFirstPressed(GameInput::kDPadLeft)
        || GameInput::IsFirstPressed(GameInput::kKey_left))
    {
        sm_SelectedScope->m_IsExpanded = false;
    }
    else if (GameInput::IsFirstPressed(GameInput::kDPadRight)
        || GameInput::IsFirstPressed(GameInput::kKey_right))
    {
        sm_SelectedScope->m_IsExpanded = true;
    }
    else if (GameInput::IsFirstPressed(GameInput::kDPadDown)
        || GameInput::IsFirstPressed(GameInput::kKey_down))
    {
        sm_SelectedScope = sm_SelectedScope ? sm_SelectedScope->NextScope() : nullptr;
    }
    else if (GameInput::IsFirstPressed(GameInput::kDPadUp)
        || GameInput::IsFirstPressed(GameInput::kKey_up))
    {
        sm_SelectedScope = sm_SelectedScope ? sm_SelectedScope->PrevScope() : nullptr;
    }
}

void NestedTimingTree::DisplayNode(wstringstream& Text, UINT indent, bool expandAllNodes)
{

    if (this == &sm_RootScope)
    {
        m_IsExpanded = true;
        //sm_RootScope.FirstChild()->m_IsExpanded = true;
    }
    else
    {
        Text << (sm_SelectedScope == this ? L"[x] " : L"[] ");

        Text << Indent(indent);

        if (m_Children.size() == 0)
            Text << L"   ";
        else if (m_IsExpanded)
            Text << L"-  ";
        else
            Text << L"+  ";

        Text << m_Name.c_str() << L": ";

        streamsize prevPrecision = Text.precision(3);
        streamsize prevWidth = Text.width(6);

        if (EngineProfiling::DrawCpuTime)
        {
            Text << m_CpuTimer.GetAverageMS() << L" ";    
        }
        Text << m_GpuTimer.GetAverageMS() << L"   ";
        Text.width(prevWidth);
        Text.precision(prevPrecision);

        Text << L"\n";

        indent += 2;
    }

    if (!expandAllNodes && !m_IsExpanded)
        return;

    for (auto node : m_Children)
        node->DisplayNode(Text, indent, expandAllNodes);
}

