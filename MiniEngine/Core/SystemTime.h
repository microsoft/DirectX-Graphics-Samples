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
// Contains classes needed to time executing code.
//

#pragma once

#include "GameCore.h"
#include "EngineTuning.h"

namespace SystemTime
{
	// Query the performance counter frequency
	void Initialize( void );

	void Update();

	// Query the current value of the performance counter
	int64_t GetCurrentTick( void );

	// The amount of time that elapses between ticks of the performance counter
	extern double CpuTickDelta;

	// The amount of CPU time elapsed during the last completed frame
	extern float FrameTime;

	// The sequential index of the frame that the CPU is currently working on
	extern uint64_t FrameIndex;

	extern int64_t StartTick;
	extern float PresentLatency;

	inline float GetFrameRate( void )
	{
		return FrameTime == 0.0f ? 0.0f : 1.0f / FrameTime;
	}

	extern BoolVar EnableVSync;
}

class CpuTimer
{
public:

	CpuTimer()
	{
		m_StartTick = 0ll;
		m_ElapsedTicks = 0ll;
	}

	void Start()
	{
		if (m_StartTick == 0ll)
			m_StartTick = SystemTime::GetCurrentTick();
	}

	void Stop()
	{
		if (m_StartTick != 0ll)
		{
			m_ElapsedTicks += SystemTime::GetCurrentTick() - m_StartTick;
			m_StartTick = 0ll;
		}
	}

	void Reset()
	{
		m_ElapsedTicks = 0ll;
		m_StartTick = 0ll;
	}

	double GetTime() const
	{
		return SystemTime::CpuTickDelta * m_ElapsedTicks;
	}

private:

	int64_t m_StartTick;
	int64_t m_ElapsedTicks;
};
