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

// Helper class for animation and simulation timing.
class StepTimer
{
public:
    StepTimer() :
        m_elapsedTicks(0),
        m_totalTicks(0),
        m_leftOverTicks(0),
        m_frameCount(0),
        m_framesPerSecond(0),
        m_framesThisSecond(0),
        m_qpcSecondCounter(0),
        m_isFixedTimeStep(false),
        m_targetElapsedTicks(TicksPerSecond / 60)
    {
        QueryPerformanceFrequency(&m_qpcFrequency);
        QueryPerformanceCounter(&m_qpcLastTime);

        // Initialize max delta to 1/10 of a second.
        m_qpcMaxDelta = m_qpcFrequency.QuadPart / 10;
    }

    // Get elapsed time since the previous Update call.
    UINT64 GetElapsedTicks() const                        { return m_elapsedTicks; }
    double GetElapsedSeconds() const                    { return TicksToSeconds(m_elapsedTicks); }

    // Get total time since the start of the program.
    UINT64 GetTotalTicks() const                        { return m_totalTicks; }
    double GetTotalSeconds() const                        { return TicksToSeconds(m_totalTicks); }

    // Get total number of updates since start of the program.
    UINT32 GetFrameCount() const                        { return m_frameCount; }

    // Get the current framerate.
    UINT32 GetFramesPerSecond() const                    { return m_framesPerSecond; }

    // Set whether to use fixed or variable timestep mode.
    void SetFixedTimeStep(bool isFixedTimestep)            { m_isFixedTimeStep = isFixedTimestep; }

    // Set how often to call Update when in fixed timestep mode.
    void SetTargetElapsedTicks(UINT64 targetElapsed)    { m_targetElapsedTicks = targetElapsed; }
    void SetTargetElapsedSeconds(double targetElapsed)    { m_targetElapsedTicks = SecondsToTicks(targetElapsed); }

    // Integer format represents time using 10,000,000 ticks per second.
    static const UINT64 TicksPerSecond = 10000000;

    static double TicksToSeconds(UINT64 ticks)            { return static_cast<double>(ticks) / TicksPerSecond; }
    static UINT64 SecondsToTicks(double seconds)        { return static_cast<UINT64>(seconds * TicksPerSecond); }

    // After an intentional timing discontinuity (for instance a blocking IO operation)
    // call this to avoid having the fixed timestep logic attempt a set of catch-up 
    // Update calls.

    void ResetElapsedTime()
    {
        QueryPerformanceCounter(&m_qpcLastTime);

        m_leftOverTicks = 0;
        m_framesPerSecond = 0;
        m_framesThisSecond = 0;
        m_qpcSecondCounter = 0;
    }

    typedef void(*LPUPDATEFUNC) (void);

    // Update timer state, calling the specified Update function the appropriate number of times.
    void Tick(LPUPDATEFUNC update = nullptr)
    {
        // Query the current time.
        LARGE_INTEGER currentTime;

        QueryPerformanceCounter(&currentTime);

        UINT64 timeDelta = currentTime.QuadPart - m_qpcLastTime.QuadPart;

        m_qpcLastTime = currentTime;
        m_qpcSecondCounter += timeDelta;

        // Clamp excessively large time deltas (e.g. after paused in the debugger).
        if (timeDelta > m_qpcMaxDelta)
        {
            timeDelta = m_qpcMaxDelta;
        }

        // Convert QPC units into a canonical tick format. This cannot overflow due to the previous clamp.
        timeDelta *= TicksPerSecond;
        timeDelta /= m_qpcFrequency.QuadPart;

        UINT32 lastFrameCount = m_frameCount;

        if (m_isFixedTimeStep)
        {
            // Fixed timestep update logic

            // If the app is running very close to the target elapsed time (within 1/4 of a millisecond) just clamp
            // the clock to exactly match the target value. This prevents tiny and irrelevant errors
            // from accumulating over time. Without this clamping, a game that requested a 60 fps
            // fixed update, running with vsync enabled on a 59.94 NTSC display, would eventually
            // accumulate enough tiny errors that it would drop a frame. It is better to just round 
            // small deviations down to zero to leave things running smoothly.

            if (abs(static_cast<int>(timeDelta - m_targetElapsedTicks)) < TicksPerSecond / 4000)
            {
                timeDelta = m_targetElapsedTicks;
            }

            m_leftOverTicks += timeDelta;

            while (m_leftOverTicks >= m_targetElapsedTicks)
            {
                m_elapsedTicks = m_targetElapsedTicks;
                m_totalTicks += m_targetElapsedTicks;
                m_leftOverTicks -= m_targetElapsedTicks;
                m_frameCount++;

                if (update)
                {
                    update();
                }
            }
        }
        else
        {
            // Variable timestep update logic.
            m_elapsedTicks = timeDelta;
            m_totalTicks += timeDelta;
            m_leftOverTicks = 0;
            m_frameCount++;

            if (update)
            {
                update();
            }
        }

        // Track the current framerate.
        if (m_frameCount != lastFrameCount)
        {
            m_framesThisSecond++;
        }

        if (m_qpcSecondCounter >= static_cast<UINT64>(m_qpcFrequency.QuadPart))
        {
            m_framesPerSecond = m_framesThisSecond;
            m_framesThisSecond = 0;
            m_qpcSecondCounter %= m_qpcFrequency.QuadPart;
        }
    }

private:
    // Source timing data uses QPC units.
    LARGE_INTEGER m_qpcFrequency;
    LARGE_INTEGER m_qpcLastTime;
    UINT64 m_qpcMaxDelta;

    // Derived timing data uses a canonical tick format.
    UINT64 m_elapsedTicks;
    UINT64 m_totalTicks;
    UINT64 m_leftOverTicks;

    // Members for tracking the framerate.
    UINT32 m_frameCount;
    UINT32 m_framesPerSecond;
    UINT32 m_framesThisSecond;
    UINT64 m_qpcSecondCounter;

    // Members for configuring fixed timestep mode.
    bool m_isFixedTimeStep;
    UINT64 m_targetElapsedTicks;
};
