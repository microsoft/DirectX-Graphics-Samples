#pragma once

#include <chrono>
#include <vector>
#include <string>
#include <assert.h>
#include "DXSampleHelper.h"
#include "MyDx12Utils.h"

using Microsoft::WRL::ComPtr;

namespace MyDx12Util {

	class WorkMeter
	{
	public:
		class CheckPoint
		{
		public:
			CheckPoint(const std::string& name, const std::chrono::steady_clock::time_point& timePoint) :
				name(name), timePoint(timePoint) {
			}
			std::string name;
			std::chrono::steady_clock::time_point timePoint;
		};


		void Start() {
			m_timePoints.clear();
			m_timePoints.emplace_back(CheckPoint{ "StartCpu", std::chrono::steady_clock::now() });	
		}

		void End() {
			m_timePoints.emplace_back(CheckPoint{ "EndCpu", std::chrono::steady_clock::now() });
			auto cpuStart = m_timePoints.front().timePoint;
			auto cpuEnd = m_timePoints.back().timePoint;
			assert(m_timePoints.front().name == "StartCpu" && m_timePoints.back().name == "EndCpu");
			m_cpuFrameTime = std::chrono::duration<float, std::milli>(cpuEnd - cpuStart).count();
		}

		float GetCpuFrameTimeMs() const {
			return m_cpuFrameTime;
		}

	private:

		float m_cpuFrameTime;

		std::vector<CheckPoint> m_timePoints;

	};


	class GpuWorkMeter
	{
	public:

		class CheckPoint
		{
		public:
			CheckPoint(const std::string& name) :
				name(name), timeStamp(0.f) {
			}
			std::string name;
			float timeStamp;
		};


		void Init(ID3D12Device* device, UINT maxQueryCount) {
			m_maxQueryCount = maxQueryCount;
			D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
			queryHeapDesc.Count = maxQueryCount;
			queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
			ThrowIfFailed(device->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&m_queryHeap)));
			ThrowIfFailed(device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT64) * maxQueryCount),
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&m_queryReadback)));
		}

		void Term() {
			m_queryHeap.Reset();
			m_queryReadback.Reset();
		}

		void StartGpu(ID3D12GraphicsCommandList* commandList, std::vector<CheckPoint>& checkPoints ) {
			m_queryIndex = 0;
			m_pCheckPoints = &checkPoints;
			m_pCheckPoints->clear();
			query(commandList, m_queryIndex, std::string("StartGpu"));
			m_queryIndex++;
		}

		void SetCheckPoint(ID3D12GraphicsCommandList* commandList, const std::string& name) {
			if (m_queryIndex >= m_maxQueryCount) {
				// Handle error: too many queries
				assert(false && "Exceeded maximum query count");
				return;
			}
			query(commandList, m_queryIndex, name);
			m_queryIndex++;
		}

		void EndGpu(ID3D12GraphicsCommandList* commandList) {

			if (m_queryIndex >= m_maxQueryCount) {
				// Handle error: too many queries
				assert(false && "Exceeded maximum query count");
				return;
			}
			query(commandList, m_queryIndex, std::string("EndGpu"));
			m_queryIndex++;
			//resolve query data to readback buffer
			commandList->ResolveQueryData(m_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, m_queryIndex, m_queryReadback.Get(), 0);
		}

		void ReadbackData(ID3D12CommandQueue* commandQueue) {
			UINT64* queryData = nullptr;
			D3D12_RANGE readRange = { 0, sizeof(UINT64) * m_queryIndex };
			ThrowIfFailed(m_queryReadback->Map(0, &readRange, reinterpret_cast<void**>(&queryData)));

			UINT64 freq = 0;
			commandQueue->GetTimestampFrequency(&freq);

			for (int i = 0; i < m_queryIndex; i++) {
				m_pCheckPoints->at(i).timeStamp = ((queryData[i]-queryData[0]) / static_cast<float>(freq)) * 1000.0f;
				//DBG_PRINT("Gpu CheckPoint: %s, Time: %f ms\n", m_pCheckPoints->at(i).name.c_str(), m_pCheckPoints->at(i).timeStamp);
			}

			m_queryReadback->Unmap(0, nullptr);
		}

	private:
		ComPtr<ID3D12QueryHeap> m_queryHeap;
		ComPtr<ID3D12Resource> m_queryReadback;

		std::vector<CheckPoint> *m_pCheckPoints = nullptr;
		int m_queryIndex = 0;
		int m_maxQueryCount = 0;

		void query(ID3D12GraphicsCommandList* commandList, UINT queryIndex, const std::string &name) {
			commandList->EndQuery(m_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, queryIndex);
			m_pCheckPoints->emplace_back(CheckPoint(name));
		}


	};

}