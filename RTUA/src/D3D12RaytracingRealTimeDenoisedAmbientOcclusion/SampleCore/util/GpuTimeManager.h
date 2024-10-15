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

namespace DX
{
    class GpuTimeManager
    {
    public:
        GpuTimeManager() { ThrowIfFalse(++s_numInstances == 1, L"There can be only one GpuTimeManager instance."); }
        ~GpuTimeManager() { ReleaseDevice(); }

        void RestoreDevice(ID3D12Device5* device, ID3D12CommandQueue* commandQueue, UINT maxFrameCount, UINT MaxNumTimers = 100);
        void ReleaseDevice();
        void Shutdown();

        // Reserve a unique timer index
        UINT NewTimer(void);

        // Indicate beginning & end of frame
        void BeginFrame(ID3D12GraphicsCommandList4* commandList);
        void EndFrame(ID3D12GraphicsCommandList4* commandList);

        // Write start and stop time stamps on the GPU timeline
        void Start(ID3D12GraphicsCommandList4* commandList, UINT timerid);
        void Stop(ID3D12GraphicsCommandList4* commandList, UINT timerid);
        
        void SetAvgRefreshPeriodMS(float avgRefreshPeriodMs) { m_avgRefreshPeriodMs = avgRefreshPeriodMs; }
        float AvgRefreshPeriodMS() const { return m_avgRefreshPeriodMs; }

        // Reset running average.
        void Reset();

        // Returns delta time in milliseconds.
        float GetElapsedMS(UINT timerid) const;

        // Returns running average in milliseconds.
        float GetAverageMS(UINT timerid) const
        {
            return m_avg[timerid];
        }

        static GpuTimeManager& instance();


    private:
        ComPtr<ID3D12QueryHeap> m_QueryHeap;
        ComPtr<ID3D12Resource> m_ReadBackBuffer;
        uint64_t* m_TimeStampBuffer = nullptr;
        uint64_t m_Fence = 0;
        UINT m_NumAllotedTimers = 0;
        UINT m_MaxNumTimers = 0;
        UINT m_MaxNumTimerSlots = 0;
        uint64_t m_ValidTimeStart = 0;
        uint64_t m_ValidTimeEnd = 0;
        double m_GpuTickDelta = 0.0;

        std::vector<float>      m_avg;
        std::vector<UINT64>     m_timing;
        size_t                  m_maxframeCount = 0;
        float					m_avgRefreshPeriodMs = 1e3f;
        std::vector<float>      m_avgPeriodTotal;
        UINT					m_avgTimestampsTotal = 0;
        CPUTimer				m_avgPeriodTimer;
        static UINT             s_numInstances;
    };
}