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
#include "CommandContext.h"
#include <thread>

using namespace Graphics;

namespace SystemTime
{
	BoolVar		EnableVSync("Timing/VSync", true);
	BoolVar		LimitTo30Hz("Timing/Limit To 30Hz", false);
	BoolVar		DropFrames("Timing/Drop Random Frames", false);

	double CpuTickDelta = 0.0;
	float FrameTime = 0.0f;
	uint64_t FrameIndex = 0;
	int64_t StartTick = -1ll;
	float PresentLatency = 0.0f;

	// Query the performance counter frequency
	void Initialize( void )
	{
		LARGE_INTEGER frequency;
		ASSERT(TRUE == QueryPerformanceFrequency(&frequency), "Unable to query performance counter frequency");
		CpuTickDelta = 1.0 / static_cast<double>(frequency.QuadPart);
	}

	// Query the current value of the performance counter
	int64_t GetCurrentTick( void )
	{
		LARGE_INTEGER currentTick;
		ASSERT(TRUE == QueryPerformanceCounter(&currentTick), "Unable to query performance counter value");
		return static_cast<int64_t>(currentTick.QuadPart);
	}

	void BusyLoopSleep( float SleepTime )
	{
		int64_t finalTick = (int64_t)((double)SleepTime / CpuTickDelta) + GetCurrentTick();
		while (GetCurrentTick() < finalTick);
	}

	float TimeBetweenTicks( int64_t tick1, int64_t tick2 )
	{
		return (float)((tick2 - tick1) * CpuTickDelta);
	}

	void Update( void )
	{
		// Test robustness to handle spikes in CPU time
		//if (DropFrames)
		//{
		//	if (std::rand() % 25 == 0)
		//		BusyLoopSleep(0.010);
		//}

		int64_t currentTick = GetCurrentTick();

		if (EnableVSync)
		{
			// With VSync enabled, the time step between frames becomes a multiple of 16.666 ms.  We need
			// to add logic to vary between 1 and 2 (or 3 fields).  This delta time also determines how
			// long the previous frame should be displayed (i.e. the present interval.)
			FrameTime = (LimitTo30Hz ? 2.0f : 1.0f) / 60.0f;
			if (DropFrames)
			{
				if (std::rand() % 50 == 0)
					FrameTime += (1.0f / 60.0f);
			}
		}
		else
		{
			// When running free, keep the most recent total frame time as the time step for
			// the next frame simulation.  This is not super-accurate, but assuming a frame
			// time varies smoothly, it should be close enough.
			FrameTime = static_cast<float>(CpuTickDelta * (currentTick - StartTick));
		}

		StartTick = currentTick;

		++FrameIndex;

		CommandContext& Context = CommandContext::Begin();
		EngineProfiling::BeginFrame(Context);
		Context.CloseAndExecute();
	}

}
