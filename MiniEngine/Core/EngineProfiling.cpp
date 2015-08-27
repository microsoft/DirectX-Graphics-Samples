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

#include "pch.h"
#include "SystemTime.h"
#include "GraphicsCore.h"
#include "TextRenderer.h"
#include "GameInput.h"
#include "GpuTimeManager.h"
#include "CommandContext.h"
#include <vector>
#include <unordered_map>

using namespace Graphics;
using namespace Math;
using namespace std;

class GpuTimer
{
public:

	GpuTimer::GpuTimer()
	{
		m_TimerIndex = GpuTimeManager::NewTimer();
	}

	void Start(CommandContext& Context)
	{
		GpuTimeManager::StartTimer(Context, m_TimerIndex);
	}

	void Stop(CommandContext& Context)
	{
		GpuTimeManager::StopTimer(Context, m_TimerIndex);
	}

	float GpuTimer::GetTime(void)
	{
		return GpuTimeManager::GetTime(m_TimerIndex);
	}

private:

	uint32_t m_TimerIndex;
};

class NestedTimingTree
{
public:
	NestedTimingTree( const wstring& name, NestedTimingTree* parent = nullptr )
		: m_Name(name), m_Parent(parent), m_IsExpanded(false) {}

	NestedTimingTree* GetChild( const wstring& name )
	{
		auto iter = m_LUT.find(name);
		if (iter != m_LUT.end())
			return iter->second;

		NestedTimingTree* node = new NestedTimingTree(name, this);
		m_Children.push_back(node);
		m_LUT[name] = node;
		return node;
	}

	NestedTimingTree* NextScope( void )
	{
		if (m_IsExpanded && m_Children.size() > 0)
			return m_Children[0];

		return m_Parent->NextChild(this);
	}

	NestedTimingTree* PrevScope( void )
	{
		NestedTimingTree* prev = m_Parent->PrevChild(this);
		return prev == m_Parent ? prev : prev->LastChild();
	}

	NestedTimingTree* FirstChild( void )
	{
		return m_Children.size() == 0 ? nullptr : m_Children[0];
	}

	NestedTimingTree* LastChild( void )
	{
		if (!m_IsExpanded || m_Children.size() == 0)
			return this;

		return m_Children.back()->LastChild();
	}

	NestedTimingTree* NextChild( NestedTimingTree* curChild )
	{
		ASSERT(curChild->m_Parent == this);

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

	NestedTimingTree* PrevChild( NestedTimingTree* curChild )
	{
		ASSERT(curChild->m_Parent == this);

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

		ERROR("All attempts to find a previous timing sample failed");
		return nullptr;
	}

	void StartTiming( CommandContext* Context )
	{
		m_StartTick = SystemTime::GetCurrentTick();
		if (Context == nullptr)
			return;

		m_GpuTimer.Start(*Context);

		Context->PIXBeginEvent(m_Name.c_str());
	}

	void StopTiming( CommandContext* Context )
	{
		m_EndTick = SystemTime::GetCurrentTick();
		if (Context == nullptr)
			return;

		m_GpuTimer.Stop(*Context);

		Context->PIXEndEvent();
	}

	void GatherTimes( void )
	{
		m_InclusiveCpuTime = 1000.0f * (float)((m_EndTick - m_StartTick) * SystemTime::CpuTickDelta);
		m_ExclusiveCpuTime = m_InclusiveCpuTime;
		m_InclusiveGpuTime = 1000.0f * m_GpuTimer.GetTime();
		m_ExclusiveGpuTime = m_InclusiveGpuTime;

		for_each(m_Children.begin(), m_Children.end(), [this](NestedTimingTree* node)
		{
			node->GatherTimes();
			m_ExclusiveCpuTime -= node->m_InclusiveCpuTime;
			m_ExclusiveGpuTime -= node->m_InclusiveGpuTime;
		});

		m_ExclusiveCpuTime = Max(m_ExclusiveCpuTime, 0.0f);
		m_ExclusiveGpuTime = Max(m_ExclusiveGpuTime, 0.0f);

		m_StartTick = 0;
		m_EndTick = 0;
	}

	static void PushProfilingMarker( const wstring& name, CommandContext* Context );
	static void PopProfilingMarker( CommandContext* Context );
	static void Update( void );
	static void UpdateTimes( void )
	{
		GpuTimeManager::BeginReadBack();
		sm_RootScope.GatherTimes();
		GpuTimeManager::EndReadBack();
	}
	static void Display( TextContext& Text, float x )
	{
		float curX = Text.GetCursorX();
		Text.DrawString("       ");
		float indent = Text.GetCursorX() - curX;
		Text.SetCursorX(curX);
		sm_RootScope.DisplayNode( Text, x - indent, indent );
	}

	static void GetFrameTime( float& cpuTime, float& gpuTime )
	{
		NestedTimingTree* frameProfile = sm_RootScope.FirstChild();
		if (frameProfile == nullptr)
		{
			cpuTime = 0.0f;
			gpuTime = 0.0f;
		}
		else
		{
			cpuTime = frameProfile->m_InclusiveCpuTime;
			gpuTime = frameProfile->m_InclusiveGpuTime;
		}
	}

private:

	void DisplayNode( TextContext& Text, float x, float indent );
	void DeleteChildren( void )
	{
		for_each( m_Children.begin(), m_Children.end(), [=]( NestedTimingTree* node ) { delete node; });
		m_Children.clear();
	}

	wstring m_Name;
	NestedTimingTree* m_Parent;
	vector<NestedTimingTree*> m_Children;
	unordered_map<wstring, NestedTimingTree*> m_LUT;
	int64_t m_StartTick;
	int64_t m_EndTick;
	float m_InclusiveCpuTime;
	float m_ExclusiveCpuTime;
	float m_InclusiveGpuTime;
	float m_ExclusiveGpuTime;
	bool m_IsExpanded;
	GpuTimer m_GpuTimer;

	static NestedTimingTree sm_RootScope;
	static NestedTimingTree* sm_CurrentNode;
	static NestedTimingTree* sm_SelectedScope;

};

NestedTimingTree NestedTimingTree::sm_RootScope(L"");
NestedTimingTree* NestedTimingTree::sm_CurrentNode = &NestedTimingTree::sm_RootScope;
NestedTimingTree* NestedTimingTree::sm_SelectedScope = &NestedTimingTree::sm_RootScope;


void NestedTimingTree::PushProfilingMarker( const wstring& name, CommandContext* Context )
{
	sm_CurrentNode = sm_CurrentNode->GetChild(name);
	sm_CurrentNode->StartTiming(Context);
}

void NestedTimingTree::PopProfilingMarker( CommandContext* Context )
{
	sm_CurrentNode->StopTiming(Context);
	sm_CurrentNode = sm_CurrentNode->m_Parent;
}

#ifndef RELEASE

ScopedTimer::ScopedTimer( const wstring& name ) : m_Context(nullptr)
{
	NestedTimingTree::PushProfilingMarker(name, nullptr);
}

ScopedTimer::ScopedTimer( const wstring& name, CommandContext& Context ) : m_Context(&Context)
{
	NestedTimingTree::PushProfilingMarker(name, m_Context);
}

ScopedTimer::~ScopedTimer( void )
{
	NestedTimingTree::PopProfilingMarker(m_Context);
}
#endif

void NestedTimingTree::Update( void )
{
	ASSERT(sm_SelectedScope != nullptr, "Corrupted profiling data structure");

	if (sm_SelectedScope == &sm_RootScope)
	{
		sm_SelectedScope = sm_RootScope.FirstChild();
		if (sm_SelectedScope == &sm_RootScope)
			return;
	}

	if (GameInput::IsFirstPressed( GameInput::kDPadLeft )
		|| GameInput::IsFirstPressed( GameInput::kKey_left ))
	{
		sm_SelectedScope->m_IsExpanded = false;
	}
	else if (GameInput::IsFirstPressed( GameInput::kDPadRight )
		|| GameInput::IsFirstPressed( GameInput::kKey_right ))
	{
		sm_SelectedScope->m_IsExpanded = true;
	}
	else if (GameInput::IsFirstPressed( GameInput::kDPadDown )
		|| GameInput::IsFirstPressed( GameInput::kKey_down ))
	{
		sm_SelectedScope = sm_SelectedScope ? sm_SelectedScope->NextScope() : nullptr;
	}
	else if (GameInput::IsFirstPressed( GameInput::kDPadUp )
		|| GameInput::IsFirstPressed( GameInput::kKey_up ))
	{
		sm_SelectedScope = sm_SelectedScope ? sm_SelectedScope->PrevScope() : nullptr;
	}
}

void NestedTimingTree::DisplayNode( TextContext& Text, float leftMargin, float indent )
{
	if (this == &sm_RootScope)
	{
		m_IsExpanded = true;
		sm_RootScope.FirstChild()->m_IsExpanded = true;
	}
	else
	{
		if (sm_SelectedScope == this)
			Text.SetColor( Color(1.0f, 1.0f, 0.5f) );
		else
			Text.SetColor( Color(1.0f, 1.0f, 1.0f) );

		Text.SetLeftMargin(leftMargin);
		Text.SetCursorX(leftMargin);

		if (m_Children.size() == 0)
			Text.DrawString("  ");
		else if (m_IsExpanded)
			Text.DrawString("- ");
		else
			Text.DrawString("+ ");

		Text.DrawString(m_Name.c_str());
		Text.SetCursorX(leftMargin + 300.0f);
		if (m_Children.size() == 0)
			Text.DrawFormattedString("%6.3f        %6.3f\n", m_InclusiveCpuTime, m_InclusiveGpuTime);
		else
			Text.DrawFormattedString("%6.3f %6.3f %6.3f %6.3f\n", m_InclusiveCpuTime, m_ExclusiveCpuTime, m_InclusiveGpuTime, m_ExclusiveGpuTime);
	}

	if (!m_IsExpanded)
		return;

	for_each( m_Children.begin(), m_Children.end(), [=, &Text]( NestedTimingTree* node )
	{
		node->DisplayNode( Text, leftMargin + indent, indent );
	});
}

namespace EngineProfiling
{
	BoolVar DrawFrameRate("Display Frame Rate", true);
	BoolVar DrawProfiler("Display Profiler", false);
	
	void BeginFrame( CommandContext& Context )
	{
		NestedTimingTree::UpdateTimes();
		NestedTimingTree::PushProfilingMarker(L"Update Frame", &Context);
	}

	void EndFrame( CommandContext& Context )
	{
		NestedTimingTree::PopProfilingMarker(&Context);
	}

	void DisplayFrameRate( TextContext& Text )
	{
		if (!DrawFrameRate)
			return;

		float cpuTime, gpuTime;
		NestedTimingTree::GetFrameTime(cpuTime, gpuTime);
		Text.DrawFormattedString( "CPU %7.3f ms, GPU %7.3f ms, %3u Hz\n",
			cpuTime, gpuTime, (unsigned)(SystemTime::GetFrameRate() + 0.5f));
	}

	void Display( TextContext& Text, float x, float y, float w, float h )
	{
		Text.ResetCursor(x, y);

		if (DrawProfiler)
		{
			Text.GetCommandContext().SetScissor((uint32_t)Floor(x), (uint32_t)Floor(y), (uint32_t)Ceiling(w), (uint32_t)Ceiling(h));

			NestedTimingTree::Update();

			Text.SetColor( Color(0.5f, 1.0f, 1.0f) );
			Text.DrawString("Engine Profiling\n");
			Text.SetTextSize(20.0f);
			Text.SetColor( Color(1.0f, 1.0f, 1.0f) );

			NestedTimingTree::Display( Text, x );
		}

		Text.GetCommandContext().SetScissor(0, 0, 1920, 1080);
	}


} // EngineProfiling
