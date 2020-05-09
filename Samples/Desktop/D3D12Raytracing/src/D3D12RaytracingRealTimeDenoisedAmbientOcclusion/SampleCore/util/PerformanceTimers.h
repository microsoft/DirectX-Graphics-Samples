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

//
// Helpers for doing CPU & GPU performance timing and statitics
//

#pragma once



namespace DX
{
    //----------------------------------------------------------------------------------
    // CPU performance timer
    class CPUTimer
    {
    public:
        static const size_t c_maxTimers = 8;

        CPUTimer();

        CPUTimer(const CPUTimer&) = delete;
        CPUTimer& operator=(const CPUTimer&) = delete;

        CPUTimer(CPUTimer&&) = default;
        CPUTimer& operator=(CPUTimer&&) = default;

        // Start/stop a particular performance timer (don't start same index more than once in a single frame)
        void Start(UINT timerid = 0);
        void Stop(UINT timerid = 0);

        // Should Update once per frame to compute timer results
        void Update();

        // Reset running average
        void Reset();

        // Returns delta time in milliseconds
		float GetElapsedMS(UINT timerid = 0) const;

        // Returns running average in milliseconds
        float GetAverageMS(UINT timerid = 0) const
        {
            return (timerid < c_maxTimers) ? m_avg[timerid] : 0.f;
        }

    private:
        double          m_cpuFreqInv;
        LARGE_INTEGER   m_start[c_maxTimers];
        LARGE_INTEGER   m_end[c_maxTimers];
        float           m_avg[c_maxTimers];
    };


    //----------------------------------------------------------------------------------
    // DirectX 12 implementation of GPU timer
    class GPUTimer
    {
    public:
        static const size_t c_maxTimers = 8;

        GPUTimer() :
            m_gpuFreqInv(1.f),
            m_avg{},
            m_timing{},
            m_maxframeCount(0),
			m_avgTimestampsTotal(0),
			m_avgPeriodTotal{},
			m_avgRefreshPeriodMs(0)
        {}

        GPUTimer(ID3D12Device5* device, ID3D12CommandQueue* commandQueue, UINT maxFrameCount, float averageRefreshPeriodMs) :
            m_gpuFreqInv(1.f),
            m_avg{},
            m_timing{},
			m_avgTimestampsTotal(0),
			m_avgPeriodTotal{},
			m_avgRefreshPeriodMs(averageRefreshPeriodMs)
        {
            RestoreDevice(device, commandQueue, maxFrameCount);
        }

        GPUTimer(const GPUTimer&) = delete;
        GPUTimer& operator=(const GPUTimer&) = delete;

        GPUTimer(GPUTimer&&) = default;
        GPUTimer& operator=(GPUTimer&&) = default;

        ~GPUTimer() { ReleaseDevice(); }

        // Indicate beginning & end of frame
        void BeginFrame(_In_ ID3D12GraphicsCommandList4* commandList);
        void EndFrame(_In_ ID3D12GraphicsCommandList4* commandList);

        // Start/stop a particular performance timer (don't start same index more than once in a single frame)
        void Start(_In_ ID3D12GraphicsCommandList4* commandList, UINT timerid = 0);
        void Stop(_In_ ID3D12GraphicsCommandList4* commandList, UINT timerid = 0);
		
		void SetAvgRefreshPeriodMS(float avgRefreshPeriodMs) { m_avgRefreshPeriodMs = avgRefreshPeriodMs; }

        // Reset running average
        void Reset();
		
        // Returns delta time in milliseconds
        float GetElapsedMS(UINT timerid = 0) const;

        // Returns running average in milliseconds
        float GetAverageMS(UINT timerid = 0) const
        {
            return (timerid < c_maxTimers) ? m_avg[timerid] : 0.f;
        }

        // Device management
        void ReleaseDevice();

        void RestoreDevice(_In_ ID3D12Device5* device, _In_ ID3D12CommandQueue* commandQueue, UINT maxFrameCount);

    private:
        static const size_t c_timerSlots = c_maxTimers * 2;

        ComPtr<ID3D12QueryHeap> m_heap;
        ComPtr<ID3D12Resource>  m_buffer;
        double                  m_gpuFreqInv;
        float                   m_avg[c_maxTimers];
        UINT64                  m_timing[c_timerSlots];
        size_t                  m_maxframeCount;
		float					m_avgRefreshPeriodMs;
		float					m_avgPeriodTotal[c_maxTimers];
		UINT					m_avgTimestampsTotal;
		CPUTimer				m_avgPeriodTimer;
    };
}