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

class SystemTime
{
public:

    // Query the performance counter frequency
    static void Initialize( void );

    // Query the current value of the performance counter
    static int64_t GetCurrentTick( void );

    static void BusyLoopSleep( float SleepTime );

    static inline double TicksToSeconds( int64_t TickCount )
    {
        return TickCount * sm_CpuTickDelta;
    }

    static inline double TicksToMillisecs( int64_t TickCount )
    {
        return TickCount * sm_CpuTickDelta * 1000.0;
    }

    static inline double TimeBetweenTicks( int64_t tick1, int64_t tick2 )
    {
        return TicksToSeconds(tick2 - tick1);
    }

private:

    // The amount of time that elapses between ticks of the performance counter
    static double sm_CpuTickDelta;
};


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
        return SystemTime::TicksToSeconds(m_ElapsedTicks);
    }

private:

    int64_t m_StartTick;
    int64_t m_ElapsedTicks;
};
