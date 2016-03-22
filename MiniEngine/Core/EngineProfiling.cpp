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
#include "GraphRenderer.h"
#include "GameInput.h"
#include "GpuTimeManager.h"
#include "CommandContext.h"
#include <vector>
#include <unordered_map>
#include <array>

using namespace Graphics;
using namespace GraphRenderer;
using namespace Math;
using namespace std;

#define PERF_GRAPH_ERROR uint32_t(0xFFFFFFFF)
namespace EngineProfiling
{
	bool Paused = false;
}

class StatHistory
{
public:
	StatHistory()
	{
		for (uint32_t i = 0; i < kHistorySize; ++i)
			m_RecentHistory[i] = 0.0f;
		for (uint32_t i = 0; i < kExtendedHistorySize; ++i)
			m_ExtendedHistory[i] = 0.0f;
		m_Average = 0.0f;
		m_Minimum = 0.0f;
		m_Maximum = 0.0f;
	}

	void RecordStat( uint32_t FrameIndex, float Value )
	{
		m_RecentHistory[FrameIndex % kHistorySize] = Value;
		m_ExtendedHistory[FrameIndex % kExtendedHistorySize] = Value;
		m_Recent = Value;

		uint32_t ValidCount = 0;
		m_Minimum = FLT_MAX;
		m_Maximum = 0.0f;
		m_Average = 0.0f;

		for (float val : m_RecentHistory)
		{
			if (val > 0.0f)
			{
				++ValidCount;
				m_Average += val;
				m_Minimum = min(val, m_Minimum);
				m_Maximum = max(val, m_Maximum);
			}
		}

		if (ValidCount > 0)
			m_Average /= (float)ValidCount;
		else
			m_Minimum = 0.0f;
	}

	float GetLast(void) const { return m_Recent; }
	float GetMax(void) const { return m_Maximum; }
	float GetMin(void) const { return m_Minimum; }
	float GetAvg(void) const { return m_Average; }

	const float* GetHistory(void) const { return m_ExtendedHistory; }
	uint32_t GetHistoryLength(void) const { return kExtendedHistorySize; }

private:
	static const uint32_t kHistorySize = 64;
	static const uint32_t kExtendedHistorySize = 256;
	float m_RecentHistory[kHistorySize];
	float m_ExtendedHistory[kExtendedHistorySize];
	float m_Recent;
	float m_Average;
	float m_Minimum;
	float m_Maximum;
};

class StatPlot
{
public:
	StatPlot(StatHistory& Data, Color Col = Color(1.0f, 1.0f, 1.0f))
		: m_StatData(Data), m_PlotColor(Col)
	{
	}

	void SetColor( Color Col )
	{
		m_PlotColor = Col;
	}

private:
	StatHistory& m_StatData;
	Color m_PlotColor;
};

class StatGraph
{
public:
	StatGraph(const wstring& Label, D3D12_RECT Window)
		: m_Label(Label), m_Window(Window), m_BGColor(0.0f, 0.0f, 0.0f, 0.2f)
	{
	}

	void SetLabel(const wstring& Label)
	{
		m_Label = Label;
	}

	void SetWindow(D3D12_RECT Window)
	{
		m_Window = Window;
	}

	uint32_t AddPlot( const StatPlot& P )
	{
		uint32_t Idx = (uint32_t)m_Stats.size();
		m_Stats.push_back(P);
		return Idx;
	}

	StatPlot& GetPlot( uint32_t Handle );

	void Draw( GraphicsContext& Context );

private:
	wstring m_Label;
	D3D12_RECT m_Window;
	vector<StatPlot> m_Stats;
	Color m_BGColor;
	float m_PeakValue;
};

class GraphManager
{
public:

private:
	vector<StatGraph> m_Graphs;
};

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

	uint32_t GetTimerIndex(void)
	{
		return m_TimerIndex;
	}
private:

	uint32_t m_TimerIndex;
};

class NestedTimingTree
{
public:
	NestedTimingTree( const wstring& name, NestedTimingTree* parent = nullptr )
		: m_Name(name), m_Parent(parent), m_IsExpanded(false), m_IsGraphed(false), m_GraphHandle(PERF_GRAPH_ERROR) {}

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

	void GatherTimes(uint32_t FrameIndex)
	{
		if (sm_SelectedScope == this)
		{
			GraphRenderer::SetSelectedIndex(m_GpuTimer.GetTimerIndex());
		}
		if (EngineProfiling::Paused)
		{
			for (auto node : m_Children)
				node->GatherTimes(FrameIndex);
			return;
		}
		m_CpuTime.RecordStat(FrameIndex, 1000.0f * (float)SystemTime::TimeBetweenTicks(m_StartTick, m_EndTick));
		m_GpuTime.RecordStat(FrameIndex, 1000.0f * m_GpuTimer.GetTime());

		for (auto node : m_Children)
			node->GatherTimes(FrameIndex);

		m_StartTick = 0;
		m_EndTick = 0;
	}

	void SumInclusiveTimes(float& cpuTime, float& gpuTime)
	{
		cpuTime = 0.0f;
		gpuTime = 0.0f;
		for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
		{
			cpuTime += (*iter)->m_CpuTime.GetLast();
			gpuTime += (*iter)->m_GpuTime.GetLast();
		}
	}

	static void PushProfilingMarker( const wstring& name, CommandContext* Context );
	static void PopProfilingMarker( CommandContext* Context );
	static void Update( void );
	static void UpdateTimes( void )
	{
		uint32_t FrameIndex = (uint32_t)Graphics::GetFrameCount();

		GpuTimeManager::BeginReadBack();
		sm_RootScope.GatherTimes(FrameIndex);
		s_FrameDelta.RecordStat(FrameIndex, GpuTimeManager::GetTime(0));
		GpuTimeManager::EndReadBack();

		float TotalCpuTime, TotalGpuTime;
		sm_RootScope.SumInclusiveTimes(TotalCpuTime, TotalGpuTime);
		s_TotalCpuTime.RecordStat(FrameIndex, TotalCpuTime);
		s_TotalGpuTime.RecordStat(FrameIndex, TotalGpuTime);

		GraphRenderer::Update(XMFLOAT2(TotalCpuTime, TotalGpuTime), 0, true, GraphType::Global);
	}

	static float GetTotalCpuTime(void) { return s_TotalCpuTime.GetAvg(); }
	static float GetTotalGpuTime(void) { return s_TotalGpuTime.GetAvg(); }
	static float GetFrameDelta(void) { return s_FrameDelta.GetAvg(); }

	static void Display( TextContext& Text, float x )
	{
		float curX = Text.GetCursorX();
		Text.DrawString("  ");
		float originalCursorX = Text.GetCursorX();
		float originalCursorY = Text.GetCursorY();
		float indent = originalCursorX - curX;
		Text.SetCursorX(curX);
		sm_RootScope.DisplayNode( Text, x - indent, indent );
		sm_RootScope.StoreToGraph();
	}

	void Toggle()
	{ 
		//if (m_GraphHandle == PERF_GRAPH_ERROR)
		//	m_GraphHandle = GraphRenderer::InitGraph(GraphType::Profile);
		//m_IsGraphed = GraphRenderer::ManageGraphs(m_GraphHandle, GraphType::Profile);
	}
	bool IsGraphed(){ return m_IsGraphed;}

private:

	void DisplayNode( TextContext& Text, float x, float indent );
	void StoreToGraph(void);
	void DeleteChildren( void )
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
	StatHistory m_CpuTime;
	StatHistory m_GpuTime;
	bool m_IsExpanded;
	GpuTimer m_GpuTimer;
	bool m_IsGraphed;
	GraphHandle m_GraphHandle;
	static StatHistory s_TotalCpuTime;
	static StatHistory s_TotalGpuTime;
	static StatHistory s_FrameDelta;
	static NestedTimingTree sm_RootScope;
	static NestedTimingTree* sm_CurrentNode;
	static NestedTimingTree* sm_SelectedScope;

	static bool sm_CursorOnGraph;

};

StatHistory NestedTimingTree::s_TotalCpuTime;
StatHistory NestedTimingTree::s_TotalGpuTime;
StatHistory NestedTimingTree::s_FrameDelta;
NestedTimingTree NestedTimingTree::sm_RootScope(L"");
NestedTimingTree* NestedTimingTree::sm_CurrentNode = &NestedTimingTree::sm_RootScope;
NestedTimingTree* NestedTimingTree::sm_SelectedScope = &NestedTimingTree::sm_RootScope;
bool NestedTimingTree::sm_CursorOnGraph = false;
namespace EngineProfiling
{
	BoolVar DrawFrameRate("Display Frame Rate", true);
	BoolVar DrawProfiler("Display Profiler", false);
	//BoolVar DrawPerfGraph("Display Performance Graph", false);
	const bool DrawPerfGraph = false;
	
	void Update( void )
	{
		if (GameInput::IsFirstPressed( GameInput::kStartButton ) 
			|| GameInput::IsFirstPressed( GameInput::kKey_space ))
		{
			Paused = !Paused;
		}
		NestedTimingTree::UpdateTimes();
	}

	void BeginBlock(const wstring& name, CommandContext* Context)
	{
		NestedTimingTree::PushProfilingMarker(name, Context);
	}

	void EndBlock(CommandContext* Context)
	{
		NestedTimingTree::PopProfilingMarker(Context);
	}

	bool IsPaused()
	{
		return Paused;
	}

	void DisplayFrameRate( TextContext& Text )
	{
		if (!DrawFrameRate)
			return;
		
		float cpuTime = NestedTimingTree::GetTotalCpuTime();
		float gpuTime = NestedTimingTree::GetTotalGpuTime();
		float frameRate = 1.0f / NestedTimingTree::GetFrameDelta();

		Text.DrawFormattedString( "CPU %7.3f ms, GPU %7.3f ms, %3u Hz\n",
			cpuTime, gpuTime, (uint32_t)(frameRate + 0.5f));
	}

	void DisplayPerfGraph( GraphicsContext& Context )
	{
		if (DrawPerfGraph)
			GraphRenderer::RenderGraphs(Context, GraphType::Global );
	}

	void Display( TextContext& Text, float x, float y, float w, float h )
	{
		Text.ResetCursor(x, y);

		if (DrawProfiler)
		{
			//Text.GetCommandContext().SetScissor((uint32_t)Floor(x), (uint32_t)Floor(y), (uint32_t)Ceiling(w), (uint32_t)Ceiling(h));

			NestedTimingTree::Update();

			Text.SetColor( Color(0.5f, 1.0f, 1.0f) );
			Text.DrawString("Engine Profiling");
			Text.SetColor(Color(0.8f, 0.8f, 0.8f));
			Text.SetTextSize(20.0f);
			Text.DrawString("           CPU    GPU");
			Text.SetTextSize(24.0f);
			Text.NewLine();
			Text.SetTextSize(20.0f);
			Text.SetColor( Color(1.0f, 1.0f, 1.0f) );

			NestedTimingTree::Display( Text, x );
		}

		Text.GetCommandContext().SetScissor(0, 0, 1920, 1080);
	}

} // EngineProfiling

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
		//if still on graphs go back to text
		if (sm_CursorOnGraph)
			sm_CursorOnGraph = !sm_CursorOnGraph;
		else
			sm_SelectedScope->m_IsExpanded = false;
	}
	else if (GameInput::IsFirstPressed( GameInput::kDPadRight )
		|| GameInput::IsFirstPressed( GameInput::kKey_right ))
	{
		if (sm_SelectedScope->m_IsExpanded == true && !sm_CursorOnGraph)
			sm_CursorOnGraph = true;
		else
			sm_SelectedScope->m_IsExpanded = true;
		//if already expanded go over to graphs

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
	else if (GameInput::IsFirstPressed( GameInput::kAButton ) 
		|| GameInput::IsFirstPressed( GameInput::kKey_return ))
	{
		sm_SelectedScope->Toggle();
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
		if (sm_SelectedScope == this && !sm_CursorOnGraph)
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
		Text.DrawFormattedString("%6.3f %6.3f   ", m_CpuTime.GetAvg(), m_GpuTime.GetAvg());

		if (IsGraphed())
		{
			Text.SetColor(GraphRenderer::GetGraphColor(m_GraphHandle, GraphType::Profile));
			Text.DrawString("  []\n");
		}
		else
			Text.DrawString("\n");
	}

	if (!m_IsExpanded)
		return;

	for (auto node : m_Children)
		node->DisplayNode(Text, leftMargin + indent, indent);
}

void NestedTimingTree::StoreToGraph(void)
{
	if (m_GraphHandle != PERF_GRAPH_ERROR)
		GraphRenderer::Update( XMFLOAT2(m_CpuTime.GetLast(), m_GpuTime.GetLast()), m_GraphHandle, m_IsGraphed, GraphType::Profile);

	for (auto node : m_Children)
		node->StoreToGraph();
}
