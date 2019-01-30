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
    class GPUTimer
    {
    public:
        static const size_t c_maxTimers = 8;

        GPUTimer() :
            m_gpuFreqInv(1.f),
            m_avg{},
            m_timing{},
            m_maxframeCount(0)
        {}

        GPUTimer(ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue, UINT maxFrameCount) :
            m_gpuFreqInv(1.f),
            m_avg{},
            m_timing{}
        {
            Init(pDevice, pCommandQueue, maxFrameCount);
        }

        GPUTimer(const GPUTimer&) = delete;
        GPUTimer& operator=(const GPUTimer&) = delete;

        GPUTimer(GPUTimer&&) = default;
        GPUTimer& operator=(GPUTimer&&) = default;

        ~GPUTimer() { Destroy(); }

        void Init(_In_ ID3D12Device* pDevice, _In_ ID3D12CommandQueue* pCommandQueue, UINT maxFrameCount);
        void Destroy();

        // Indicate beginning & end of frame.
        void BeginFrame(_In_ ID3D12GraphicsCommandList* pCommandList);
        void EndFrame(_In_ ID3D12GraphicsCommandList* pCommandList);

        // Start/stop a particular performance timer (don't start same index more than once in a single frame).
        void Start(_In_ ID3D12GraphicsCommandList* pCommandList, UINT timerid = 0);
        void Stop(_In_ ID3D12GraphicsCommandList* pCommandList, UINT timerid = 0);

        // Reset running average.
        void ResetAverage();

        // Returns delta time in milliseconds.
        double GetElapsedMS(UINT timerid = 0) const;

        // Returns running average in milliseconds.
        float GetAverageMS(UINT timerid = 0) const
        {
            return (timerid < c_maxTimers) ? m_avg[timerid] : 0.f;
        }

    private:
        static const size_t c_timerSlots = c_maxTimers * 2;

        Microsoft::WRL::ComPtr<ID3D12QueryHeap> m_heap;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_buffer;
        double m_gpuFreqInv;
        float m_avg[c_maxTimers];
        UINT64 m_timing[c_timerSlots];
        size_t m_maxframeCount;
    };
}