#pragma once

namespace D3DX12Residency
{
#if 0
#define RESIDENCY_CHECK(x) \
	if((x) == false) { DebugBreak(); }

#define RESIDENCY_CHECK_RESULT(x) \
	if((x) != S_OK) { DebugBreak(); }
#else
#define RESIDENCY_CHECK(x)
#define RESIDENCY_CHECK_RESULT(x) x
#endif

#define RESIDENCY_SINGLE_THREADED 0

#define RESIDENCY_MIN(x,y) ((x) < (y) ? (x) : (y))
#define RESIDENCY_MAX(x,y) ((x) > (y) ? (x) : (y))

	// Space for 512 concurrent command list records @ 32bits per item.
	// This size can be tuned to your app in order to save space
#define RESIDENCY_NUM_BITMASKS 16

	namespace Internal
	{
		class CriticalSection
		{
			friend class ScopedLock;
		public:
			CriticalSection()
			{
				InitializeCriticalSectionAndSpinCount(&CS, 8);
			}

			~CriticalSection()
			{
				DeleteCriticalSection(&CS);
			}

		private:
			CRITICAL_SECTION CS;
		};

		class ScopedLock
		{
		public:

			ScopedLock() : pCS(nullptr) {};
			ScopedLock(CriticalSection* pCSIn) : pCS(pCSIn)
			{
				if (pCS)
				{
					EnterCriticalSection(&pCS->CS);
				}
			};

			~ScopedLock()
			{
				if (pCS)
				{
					LeaveCriticalSection(&pCS->CS);
				}
			}

		private:

			CriticalSection* pCS;
		};

		// One per Residency Manager
		class SyncManager
		{
		public:
			SyncManager()
			{
				for (UINT32 i = 0; i < ARRAYSIZE(CommandListMask); i++)
				{
					CommandListMask[i] = sUnsetValue;
				}
			}

			Internal::CriticalSection MaskCriticalSection;

			static const UINT32 sUnsetValue = UINT32(-1);
			// Represents which command lists are currently open for recording
			LONG CommandListMask[RESIDENCY_NUM_BITMASKS];
		};

		//Forward Declaration
		class ResidencyManagerInternal;
	}

	// Used to track meta data for each object the app potentially wants
	// to make resident or evict.
	class ManagedObject
	{
	public:
		enum class RESIDENCY_STATUS
		{
			RESIDENT,
			EVICTED
		};

		ManagedObject() :
			pUnderlying(nullptr),
			Size(0),
			ResidencyStatus(RESIDENCY_STATUS::RESIDENT),
			LastGPUSyncPoint(0),
			LastUsedTimestamp(0)
		{
			for (UINT32 i = 0; i < ARRAYSIZE(UsageMask); i++)
			{
				InterlockedExchange(&UsageMask[i], 0);
			}
		}

		void Initialize(ID3D12Pageable* pUnderlyingIn, UINT64 ObjectSize)
		{
			RESIDENCY_CHECK(pUnderlying == nullptr);
			pUnderlying = pUnderlyingIn;
			Size = ObjectSize;
		}

		inline bool IsInitialized() { return pUnderlying != nullptr; }

		// Wether the object is resident or not
		RESIDENCY_STATUS ResidencyStatus;

		// The underlying D3D Object being tracked
		ID3D12Pageable* pUnderlying;
		// The size of the D3D Object in bytes
		UINT64 Size;

		UINT64 LastGPUSyncPoint;
		UINT64 LastUsedTimestamp;

		// This bit mask is used to track which open command lists this resource is currently used on.
		volatile LONG UsageMask[RESIDENCY_NUM_BITMASKS];

		// Linked list entry
		LIST_ENTRY ListEntry;
	};

	// This represents a set of objects which are referenced by a command list i.e. every time a resource
	// is bound for rendering, clearing, copy etc. the set must be updated to ensure the it is resident 
	// for execution.
	class ResidencySet
	{
		friend class ResidencyManager;
		friend class Internal::ResidencyManagerInternal;
	public:

		static const long InvalidMask = -1;

		ResidencySet() :
			CommandListMinorMask(InvalidMask),
			CommandListMajorIndex(InvalidMask),
			MaxResidencySetSize(0),
			CurrentSetSize(0),
			ppSet(nullptr),
			IsOpen(false),
			OutOfMemory(false),
			pSyncManager(nullptr)
		{
		};

		~ResidencySet()
		{
			delete[](ppSet);
		}

		// Returns true if the object was inserted, false otherwise
		inline bool Insert(ManagedObject* pObject)
		{
			RESIDENCY_CHECK(IsOpen);
			RESIDENCY_CHECK(CommandListMinorMask != InvalidMask);
			RESIDENCY_CHECK(CommandListMajorIndex != InvalidMask);

			// If we haven't seen this object on this command list mark it
			if ((InterlockedOr(&pObject->UsageMask[CommandListMajorIndex], CommandListMinorMask) & CommandListMinorMask) == 0)
			{
				if (ppSet == nullptr || CurrentSetSize > MaxResidencySetSize)
				{
					Realloc();
				}
				if (ppSet == nullptr)
				{
					OutOfMemory = true;
					return false;
				}

				ppSet[CurrentSetSize++] = pObject;

				return true;
			}
			else
			{
				return false;
			}
		}

		HRESULT Open()
		{
			Internal::ScopedLock Lock(&pSyncManager->MaskCriticalSection);

			// It's invalid to open a set that is already open
			if (IsOpen)
			{
				return E_INVALIDARG;
			}

			RESIDENCY_CHECK(CommandListMinorMask == InvalidMask);
			RESIDENCY_CHECK(CommandListMajorIndex == InvalidMask);

			UCHAR BitSet = 0;
			// Find the first available command list by bitscanning
			for (UINT32 i = 0; i < ARRAYSIZE(pSyncManager->CommandListMask); i++)
			{
				DWORD BitIndex = 0;
				BitSet = BitScanReverse(&BitIndex, pSyncManager->CommandListMask[i]);

				if (BitSet > 0)
				{
					// Convert to a mask
					CommandListMinorMask = (1UL << UINT32(BitIndex));
					RESIDENCY_CHECK(CommandListMinorMask);

					// Unset this bit because we are using it
					pSyncManager->CommandListMask[i] &= ~CommandListMinorMask;
					CommandListMajorIndex = i;

					break;
				}
			}

			if (BitSet == 0)
			{
				RESIDENCY_CHECK(false);
				return E_OUTOFMEMORY;
			}

			CurrentSetSize = 0;

			IsOpen = true;
			OutOfMemory = false;
			return S_OK;
		}

		HRESULT Close()
		{
			if (IsOpen == false)
			{
				return E_INVALIDARG;
			}

			if (OutOfMemory == true)
			{
				return E_OUTOFMEMORY;
			}

			for (INT32 i = 0; i < CurrentSetSize; i++)
			{
				Remove(ppSet[i]);
			}

			ReturnCommandListReservation();

			IsOpen = false;

			return S_OK;
		}

	private:

		inline void Remove(ManagedObject* pObject)
		{
			InterlockedAnd(&pObject->UsageMask[CommandListMajorIndex], ~CommandListMinorMask);
		}

		inline void ReturnCommandListReservation()
		{
			Internal::ScopedLock Lock(&pSyncManager->MaskCriticalSection);

			pSyncManager->CommandListMask[CommandListMajorIndex] |= CommandListMinorMask;

			CommandListMajorIndex = ResidencySet::InvalidMask;
			CommandListMinorMask = ResidencySet::InvalidMask;
			IsOpen = false;
		}

		void Initialize(Internal::SyncManager* pSyncManagerIn)
		{
			pSyncManager = pSyncManagerIn;
		}

		bool Initialize(Internal::SyncManager* pSyncManagerIn, UINT32 MaxSize)
		{
			pSyncManager = pSyncManagerIn;
			MaxResidencySetSize = MaxSize;

			ppSet = new ManagedObject*[MaxResidencySetSize];

			return ppSet != nullptr;
		}

		inline void Realloc()
		{
			MaxResidencySetSize = (MaxResidencySetSize == 0) ? 4096 : INT32(MaxResidencySetSize + (MaxResidencySetSize / 2.0f));
			ManagedObject** ppNewAlloc = new ManagedObject*[MaxResidencySetSize];

			if (ppSet && ppNewAlloc)
			{
				memcpy(ppNewAlloc, ppSet, CurrentSetSize * sizeof(ManagedObject*));
				delete[](ppSet);
			}

			ppSet = ppNewAlloc;
		}

		LONG CommandListMinorMask;
		LONG CommandListMajorIndex;

		ManagedObject** ppSet;
		INT32 MaxResidencySetSize;
		INT32 CurrentSetSize;

		bool IsOpen;
		bool OutOfMemory;

		Internal::SyncManager* pSyncManager;
	};

	namespace Internal
	{
		/* List Helpers */
		inline void InitializeListHead(LIST_ENTRY* pHead)
		{
			pHead->Flink = pHead->Blink = pHead;
		}

		inline void InsertHeadList(LIST_ENTRY* pHead, LIST_ENTRY* pEntry)
		{
			pEntry->Blink = pHead;
			pEntry->Flink = pHead->Flink;

			pHead->Flink->Blink = pEntry;
			pHead->Flink = pEntry;
		}

		inline void InsertTailList(LIST_ENTRY* pHead, LIST_ENTRY* pEntry)
		{
			pEntry->Flink = pHead;
			pEntry->Blink = pHead->Blink;

			pHead->Blink->Flink = pEntry;
			pHead->Blink = pEntry;
		}

		inline void RemoveEntryList(LIST_ENTRY* pEntry)
		{
			pEntry->Blink->Flink = pEntry->Flink;
			pEntry->Flink->Blink = pEntry->Blink;
		}

		inline LIST_ENTRY* RemoveHeadList(LIST_ENTRY* pHead)
		{
			LIST_ENTRY* pEntry = pHead->Flink;
			RemoveEntryList(pEntry);
			return pEntry;
		}

		inline LIST_ENTRY* RemoveTailList(LIST_ENTRY* pHead)
		{
			LIST_ENTRY* pEntry = pHead->Blink;
			RemoveEntryList(pEntry);
			return pEntry;
		}

		inline bool IsListEmpty(LIST_ENTRY* pEntry)
		{
			return pEntry->Flink == pEntry;
		}

		struct Fence
		{
			Fence(UINT64 StartingValue) : pFence(nullptr), FenceValue(StartingValue)
			{
				InitializeListHead(&ListEntry);
			};

			HRESULT Initialize(ID3D12Device* pDevice)
			{
				HRESULT hr = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence));
				RESIDENCY_CHECK_RESULT(hr);

				return hr;
			}

			void Destroy()
			{
				if (pFence)
				{
					pFence->Release();
					pFence = nullptr;
				}
			}

			HRESULT GPUWait(ID3D12CommandQueue* pQueue)
			{
				HRESULT hr = pQueue->Wait(pFence, FenceValue);
				RESIDENCY_CHECK_RESULT(hr);
				return hr;
			}

			HRESULT GPUSignal(ID3D12CommandQueue* pQueue)
			{
				HRESULT hr = pQueue->Signal(pFence, FenceValue);
				RESIDENCY_CHECK_RESULT(hr);
				return hr;
			}

			inline void Increment()
			{
				FenceValue++;
			}

			ID3D12Fence* pFence;
			UINT64 FenceValue;
			LIST_ENTRY ListEntry;
		};

		// Represents a time on a particular queue that a resource was used
		struct QueueSyncPoint
		{
			QueueSyncPoint() : pFence(nullptr), LastUsedValue(0) {};

			inline bool IsCompleted() { return LastUsedValue <= pFence->pFence->GetCompletedValue(); }

			inline void WaitForCompletion(HANDLE Event)
			{
				RESIDENCY_CHECK_RESULT(pFence->pFence->SetEventOnCompletion(LastUsedValue, Event));
				RESIDENCY_CHECK_RESULT(WaitForSingleObject(Event, INFINITE));
			}

			Fence* pFence;
			UINT64 LastUsedValue;
		};

		struct DeviceWideSyncPoint
		{
			DeviceWideSyncPoint(UINT32 NumQueues, UINT64 Generation) :
				GenerationID(Generation), NumQueueSyncPoints(NumQueues) {};

			// Create the whole structure in one allocation for locality
			static DeviceWideSyncPoint* CreateSyncPoint(UINT32 NumQueues, UINT64 Generation)
			{
				DeviceWideSyncPoint* pSyncPoint = nullptr;
				const SIZE_T Size = sizeof(DeviceWideSyncPoint) + (sizeof(QueueSyncPoint) * (NumQueues - 1));

				BYTE* pAlloc = new BYTE[Size];
				if (pAlloc && Size >= sizeof(DeviceWideSyncPoint))
				{
					pSyncPoint = new (pAlloc) DeviceWideSyncPoint(NumQueues, Generation);
				}

				return pSyncPoint;
			}

			// A device wide fence is completed if all of the queues that were active at that point are completed
			inline bool IsCompleted()
			{
				for (UINT32 i = 0; i < NumQueueSyncPoints; i++)
				{
					if (pQueueSyncPoints[i].IsCompleted() == false)
					{
						return false;
					}
				}
				return true;
			}

			inline void WaitForCompletion(HANDLE Event)
			{
				for (UINT32 i = 0; i < NumQueueSyncPoints; i++)
				{
					if (pQueueSyncPoints[i].IsCompleted() == false)
					{
						pQueueSyncPoints[i].WaitForCompletion(Event);
					}
				}
			}

			const UINT64 GenerationID;
			const UINT32 NumQueueSyncPoints;
			LIST_ENTRY ListEntry;
			// NumQueueSyncPoints QueueSyncPoints will be placed below here
			QueueSyncPoint pQueueSyncPoints[1];
		};

		// A Least Recently Used Cache. Tracks all of the objects requested by the app so that objects
		// that aren't used freqently can get evicted to help the app stay under buget.
		class LRUCache
		{
		public:
			LRUCache() :
				NumResidentObjects(0),
				NumEvictedObjects(0),
				ResidentSize(0)
			{
				InitializeListHead(&ResidentObjectListHead);
				InitializeListHead(&EvictedObjectListHead);
			};

			void Insert(ManagedObject* pObject)
			{
				if (pObject->ResidencyStatus == ManagedObject::RESIDENCY_STATUS::RESIDENT)
				{
					InsertHeadList(&ResidentObjectListHead, &pObject->ListEntry);
					NumResidentObjects++;
					ResidentSize += pObject->Size;
				}
				else
				{
					InsertHeadList(&EvictedObjectListHead, &pObject->ListEntry);
					NumEvictedObjects++;
				}
			}

			void Remove(ManagedObject* pObject)
			{
				RemoveEntryList(&pObject->ListEntry);
				if (pObject->ResidencyStatus == ManagedObject::RESIDENCY_STATUS::RESIDENT)
				{
					NumResidentObjects--;
					ResidentSize -= pObject->Size;
				}
				else
				{
					NumEvictedObjects--;
				}
			}

			// When an object is used by the GPU we move it to the end of the list.
			// This way things closer to the head of the list are the objects which
			// are stale and better candidates for eviction
			void ObjectReferenced(ManagedObject* pObject)
			{
				RESIDENCY_CHECK(pObject->ResidencyStatus == ManagedObject::RESIDENCY_STATUS::RESIDENT);

				RemoveEntryList(&pObject->ListEntry);
				InsertTailList(&ResidentObjectListHead, &pObject->ListEntry);
			}

			void MakeResident(ManagedObject* pObject)
			{
				RESIDENCY_CHECK(pObject->ResidencyStatus == ManagedObject::RESIDENCY_STATUS::EVICTED);

				pObject->ResidencyStatus = ManagedObject::RESIDENCY_STATUS::RESIDENT;
				RemoveEntryList(&pObject->ListEntry);
				InsertTailList(&ResidentObjectListHead, &pObject->ListEntry);

				NumEvictedObjects--;
				NumResidentObjects++;
				ResidentSize += pObject->Size;
			}

			void Evict(ManagedObject* pObject)
			{
				RESIDENCY_CHECK(pObject->ResidencyStatus == ManagedObject::RESIDENCY_STATUS::RESIDENT);

				pObject->ResidencyStatus = ManagedObject::RESIDENCY_STATUS::EVICTED;
				RemoveEntryList(&pObject->ListEntry);
				InsertTailList(&EvictedObjectListHead, &pObject->ListEntry);

				NumResidentObjects--;
				ResidentSize -= pObject->Size;
				NumEvictedObjects++;
			}

			// Evict all of the resident objects used in sync points up to the specficied one (inclusive)
			void TrimToSyncPointInclusive(INT64 CurrentUsage, INT64 CurrentBudget, ID3D12Pageable** EvictionList, UINT32& NumObjectsToEvict, UINT64 SyncPoint)
			{
				NumObjectsToEvict = 0;

				LIST_ENTRY* pResourceEntry = ResidentObjectListHead.Flink;
				while (pResourceEntry != &ResidentObjectListHead)
				{
					ManagedObject* pObject = CONTAINING_RECORD(pResourceEntry, ManagedObject, ListEntry);

					if (pObject->LastGPUSyncPoint > SyncPoint || CurrentUsage < CurrentBudget)
					{
						break;
					}

					RESIDENCY_CHECK(pObject->ResidencyStatus == ManagedObject::RESIDENCY_STATUS::RESIDENT);

					EvictionList[NumObjectsToEvict++] = pObject->pUnderlying;
					Evict(pObject);

					CurrentUsage -= pObject->Size;

					pResourceEntry = ResidentObjectListHead.Flink;
				}
			}

			// Trim all objects which are older than the specified time
			void TrimAgedAllocations(DeviceWideSyncPoint* MaxSyncPoint, ID3D12Pageable** EvictionList, UINT32& NumObjectsToEvict, UINT64 CurrentTimeStamp, UINT64 MinDelta)
			{
				LIST_ENTRY* pResourceEntry = ResidentObjectListHead.Flink;
				while (pResourceEntry != &ResidentObjectListHead)
				{
					ManagedObject* pObject = CONTAINING_RECORD(pResourceEntry, ManagedObject, ListEntry);

					if ((MaxSyncPoint && pObject->LastGPUSyncPoint >= MaxSyncPoint->GenerationID) || // Only trim allocations done on the GPU
						CurrentTimeStamp - pObject->LastUsedTimestamp <= MinDelta) // Don't evict things which have been used recently
					{
						break;
					}

					RESIDENCY_CHECK(pObject->ResidencyStatus == ManagedObject::RESIDENCY_STATUS::RESIDENT);
					EvictionList[NumObjectsToEvict++] = pObject->pUnderlying;
					Evict(pObject);

					pResourceEntry = ResidentObjectListHead.Flink;
				}
			}

			ManagedObject* GetResidentListHead()
			{
				if (IsListEmpty(&ResidentObjectListHead))
				{
					return nullptr;
				}
				return CONTAINING_RECORD(ResidentObjectListHead.Flink, ManagedObject, ListEntry);
			}

			LIST_ENTRY ResidentObjectListHead;
			LIST_ENTRY EvictedObjectListHead;

			UINT32 NumResidentObjects;
			UINT32 NumEvictedObjects;

			UINT64 ResidentSize;
		};

		class ResidencyManagerInternal
		{
		public:
			ResidencyManagerInternal(SyncManager* pSyncManagerIn) :
				Device(nullptr),
				AsyncThreadFence(1),
				CompletionEvent(INVALID_HANDLE_VALUE),
				AsyncThreadWorkCompletionEvent(INVALID_HANDLE_VALUE),
				Adapter(nullptr),
				AsyncWorkEvent(INVALID_HANDLE_VALUE),
				AsyncWorkThread(INVALID_HANDLE_VALUE),
				FinishAsyncWork(false),
				cStartEvicted(false),
				CurrentSyncPointGeneration(0),
				NumQueuesSeen(0),
				NodeMask(0),
				CurrentAsyncWorkloadHead(0),
				CurrentAsyncWorkloadTail(0),
				cMinEvictionGracePeriod(2.0f),
				cMaxEvictionGracePeriod(60.0f),
				AsyncWorkQueue(nullptr),
				MaxSoftwareQueueLatency(6),
				AsyncWorkQueueSize(7),
				pSyncManager(pSyncManagerIn)
			{
				Internal::InitializeListHead(&QueueFencesListHead);
				Internal::InitializeListHead(&InFlightSyncPointsHead);
			};

			HRESULT Initialize(ID3D12Device* ParentDevice, UINT DeviceNodeMask, IDXGIAdapter3* ParentAdapter, UINT32 MaxLatency)
			{
				Device = ParentDevice;
				NodeMask = DeviceNodeMask;
				Adapter = ParentAdapter;
				MaxSoftwareQueueLatency = MaxLatency;

				AsyncWorkQueueSize = MaxLatency + 1;
				AsyncWorkQueue = new AsyncWorkload[AsyncWorkQueueSize];

				if (AsyncWorkQueue == nullptr)
				{
					return E_OUTOFMEMORY;
				}

				LARGE_INTEGER Frequency;
				QueryPerformanceFrequency(&Frequency);

				// Calculate how many QPC ticks are equivalent to the given time in seconds
				MinEvictionGracePeriodTicks = UINT64(Frequency.QuadPart * cMinEvictionGracePeriod);
				MaxEvictionGracePeriodTicks = UINT64(Frequency.QuadPart * cMaxEvictionGracePeriod);

				HRESULT hr = S_OK;
				hr = AsyncThreadFence.Initialize(Device);

				if (SUCCEEDED(hr))
				{
					CompletionEvent = CreateEvent(nullptr, false, false, nullptr);
					if (CompletionEvent == INVALID_HANDLE_VALUE)
					{
						hr = HRESULT_FROM_WIN32(GetLastError());
					}
				}

				if (SUCCEEDED(hr))
				{
					AsyncThreadWorkCompletionEvent = CreateEvent(nullptr, false, false, nullptr);
					if (AsyncThreadWorkCompletionEvent == INVALID_HANDLE_VALUE)
					{
						hr = HRESULT_FROM_WIN32(GetLastError());
					}
				}

				if (SUCCEEDED(hr))
				{
					AsyncWorkEvent = CreateEvent(nullptr, true, false, nullptr);
					if (AsyncWorkEvent == INVALID_HANDLE_VALUE)
					{
						hr = HRESULT_FROM_WIN32(GetLastError());
					}
				}

#if !RESIDENCY_SINGLE_THREADED
				if (SUCCEEDED(hr))
				{
					AsyncWorkThread = CreateThread(NULL, 0, AsyncThreadStart, (void*) this, 0, nullptr);

					if (AsyncWorkThread == INVALID_HANDLE_VALUE)
					{
						hr = HRESULT_FROM_WIN32(GetLastError());
					}
				}
#endif

				return hr;
			}

			void Destroy()
			{
				AsyncThreadFence.Destroy();

				if (CompletionEvent != INVALID_HANDLE_VALUE)
				{
					CloseHandle(CompletionEvent);
					CompletionEvent = INVALID_HANDLE_VALUE;
				}

#if !RESIDENCY_SINGLE_THREADED
				AsyncWorkload* pWork = DequeueAsyncWork();

				while (pWork)
				{
					pWork = DequeueAsyncWork();
				}

				FinishAsyncWork = true;
				if (SetEvent(AsyncWorkEvent) == false)
				{
					RESIDENCY_CHECK_RESULT(HRESULT_FROM_WIN32(GetLastError()));
				}

				if (AsyncWorkThread != INVALID_HANDLE_VALUE)
				{
					CloseHandle(AsyncWorkThread);
					AsyncWorkThread = INVALID_HANDLE_VALUE;
				}

				if (AsyncWorkEvent != INVALID_HANDLE_VALUE)
				{
					CloseHandle(AsyncWorkEvent);
					AsyncWorkEvent = INVALID_HANDLE_VALUE;
				}
#endif

				if (AsyncThreadWorkCompletionEvent != INVALID_HANDLE_VALUE)
				{
					CloseHandle(AsyncThreadWorkCompletionEvent);
					AsyncThreadWorkCompletionEvent = INVALID_HANDLE_VALUE;
				}

				while (Internal::IsListEmpty(&QueueFencesListHead) == false)
				{
					Internal::Fence* pObject =
						CONTAINING_RECORD(QueueFencesListHead.Flink, Internal::Fence, ListEntry);

					pObject->Destroy();
					Internal::RemoveHeadList(&QueueFencesListHead);
					delete(pObject);
				}
			}

			void BeginTrackingObject(ManagedObject* pObject)
			{
				Internal::ScopedLock Lock(&Mutex);

				if (pObject)
				{
					RESIDENCY_CHECK(pObject->pUnderlying != nullptr);
					if (cStartEvicted)
					{
						pObject->ResidencyStatus = ManagedObject::RESIDENCY_STATUS::EVICTED;
						RESIDENCY_CHECK_RESULT(Device->Evict(1, &pObject->pUnderlying));
					}

					LRU.Insert(pObject);
				}
			}

			void EndTrackingObject(ManagedObject* pObject)
			{
				Internal::ScopedLock Lock(&Mutex);

				LRU.Remove(pObject);
			}

			// One residency set per command-list
			HRESULT ExecuteCommandLists(ID3D12CommandQueue* Queue, ID3D12CommandList** CommandLists, ResidencySet** ResidencySets, UINT32 Count)
			{
				return ExecuteSubset(Queue, CommandLists, ResidencySets, Count);
			}

		private:

			HRESULT ExecuteSubset(ID3D12CommandQueue* Queue, ID3D12CommandList** CommandLists, ResidencySet** ResidencySets, UINT32 Count)
			{
				HRESULT hr = S_OK;

				DXGI_QUERY_VIDEO_MEMORY_INFO LocalMemory;
				ZeroMemory(&LocalMemory, sizeof(LocalMemory));
				GetCurrentBudget(&LocalMemory, DXGI_MEMORY_SEGMENT_GROUP_LOCAL);

				DXGI_QUERY_VIDEO_MEMORY_INFO NonLocalMemory;
				ZeroMemory(&NonLocalMemory, sizeof(NonLocalMemory));
				GetCurrentBudget(&NonLocalMemory, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL);

				UINT64 TotalSizeNeeded = 0;

				UINT32 MaxObjectsReferenced = 0;
				for (UINT32 i = 0; i < Count; i++)
				{
					if (ResidencySets[i])
					{
						if (ResidencySets[i]->IsOpen)
						{
							// Residency Sets must be closed before execution just like Command Lists
							return E_INVALIDARG;
						}
						MaxObjectsReferenced += ResidencySets[i]->CurrentSetSize;
					}
				}

				// Create a set to gather up all unique resources required by this call
				ResidencySet* pMasterSet = new ResidencySet();
				if (pMasterSet == nullptr || pMasterSet->Initialize(pSyncManager, MaxObjectsReferenced) == false)
				{
					return E_OUTOFMEMORY;
				}

				hr = pMasterSet->Open();
				if (FAILED(hr))
				{
					return hr;
				}

				// For each residency set
				for (UINT32 i = 0; i < Count; i++)
				{
					if (ResidencySets[i])
					{
						// For each object in this set
						for (INT32 x = 0; x < ResidencySets[i]->CurrentSetSize; x++)
						{
							if (pMasterSet->Insert(ResidencySets[i]->ppSet[x]))
							{
								TotalSizeNeeded += ResidencySets[i]->ppSet[x]->Size;
							}
						}
					}
				}
				// Close this set to free it's slot up for the app
				hr = pMasterSet->Close();
				if (FAILED(hr))
				{
					return hr;
				}

				// This set of commandlists can't possibly fit within the budget, they need to be split up. If the number of command lists is 1 there is
				// nothing we can do
				if (Count > 1 && TotalSizeNeeded > LocalMemory.Budget + NonLocalMemory.Budget)
				{
					delete(pMasterSet);

					// Recursively try to find a small enough set to fit in memory
					const UINT32 Half = Count / 2;
					const HRESULT LowerHR = ExecuteSubset(Queue, CommandLists, ResidencySets, Half);
					const HRESULT UpperHR = ExecuteSubset(Queue, &CommandLists[Half], &ResidencySets[Half], Count - Half);

					return (LowerHR == S_OK && UpperHR == S_OK) ? S_OK : E_FAIL;
				}

				// We have to track each object on each queue so we know when it is safe to evict them. Therefore, for every queue that we
				// see, associate a fence with it
				const GUID FenceGuid = { 0xf0, 0, 0xd,{ 0, 0, 0, 0, 0, 0, 0, 0 } };

				// Generate a GUID based on this queue
				memcpy((void*)FenceGuid.Data4, Queue, sizeof(ID3D12CommandQueue*));

				Internal::Fence* QueueFence = nullptr;
				// Find or create the fence for this queue
				{
					UINT32 Size = sizeof(Internal::Fence*);
					hr = Queue->GetPrivateData(FenceGuid, &Size, &QueueFence);
					if (FAILED(hr))
					{
						QueueFence = new Internal::Fence(1);
						hr = QueueFence->Initialize(Device);
						Internal::InsertTailList(&QueueFencesListHead, &QueueFence->ListEntry);

						InterlockedIncrement(&NumQueuesSeen);

						if (SUCCEEDED(hr))
						{
							hr = Queue->SetPrivateData(FenceGuid, UINT32(sizeof(Internal::Fence*)), &QueueFence);
							RESIDENCY_CHECK_RESULT(hr);
						}
					}
					RESIDENCY_CHECK(QueueFence != nullptr);
				}

				// The following code must be atomic so that things get ordered correctly
				{
					Internal::ScopedLock Lock(&ExecutionCS);
					// Evict or make resident all of the objects we identified above.
					// This will run on an async thread, allowing the current to continue while still blocking the GPU if required
					hr = EnqueueAsyncWork(pMasterSet, AsyncThreadFence.FenceValue, CurrentSyncPointGeneration);
#if RESIDENCY_SINGLE_THREADED
					AsyncWorkload* pWorkload = DequeueAsyncWork();
					ProcessPagingWork(pWorkload);
#endif

					// If there are some things that need to be made resident we need to make sure that the GPU
					// doesn't execute until the async thread signals that the MakeResident call has returned.
					if (SUCCEEDED(hr))
					{
						hr = AsyncThreadFence.GPUWait(Queue);
						AsyncThreadFence.Increment();
					}

					Queue->ExecuteCommandLists(Count, CommandLists);


					if (SUCCEEDED(hr))
					{
						// When this fence is passed it is safe to evict the resources used in the list just submitted
						hr = QueueFence->GPUSignal(Queue);
						QueueFence->Increment();
					}

					if (SUCCEEDED(hr))
					{
						hr = EnqueueSyncPoint();
						RESIDENCY_CHECK_RESULT(hr);
					}

					CurrentSyncPointGeneration++;
				}
				return hr;
			}

			struct AsyncWorkload
			{
				AsyncWorkload() :
					pMasterSet(nullptr),
					FenceValueToSignal(0),
					SyncPointGeneration(0)
				{}

				UINT64 SyncPointGeneration;

				// List of objects to make resident
				ResidencySet* pMasterSet;

				// The GPU will wait on this value so that it doesn't execute until the objects are made resident
				UINT64 FenceValueToSignal;
			};

			SIZE_T AsyncWorkQueueSize;
			AsyncWorkload* AsyncWorkQueue;

			HANDLE AsyncWorkEvent;
			HANDLE AsyncWorkThread;
			Internal::CriticalSection AsyncWorkMutex;
			volatile bool FinishAsyncWork;
			volatile SIZE_T CurrentAsyncWorkloadHead;
			volatile SIZE_T CurrentAsyncWorkloadTail;

			static unsigned long WINAPI AsyncThreadStart(void* pData)
			{
				ResidencyManagerInternal* pManager = (ResidencyManagerInternal*)pData;

				while (1)
				{
					AsyncWorkload* pWork = pManager->DequeueAsyncWork();

					while (pWork)
					{
						// Submit the work
						pManager->ProcessPagingWork(pWork);
						if (SetEvent(pManager->AsyncThreadWorkCompletionEvent) == false)
						{
							RESIDENCY_CHECK_RESULT(HRESULT_FROM_WIN32(GetLastError()));
						}

						// Get more work
						pWork = pManager->DequeueAsyncWork();
					}

					//Wait until there is more work do be done
					WaitForSingleObject(pManager->AsyncWorkEvent, INFINITE);
					if (ResetEvent(pManager->AsyncWorkEvent) == false)
					{
						RESIDENCY_CHECK_RESULT(HRESULT_FROM_WIN32(GetLastError()));
					}

					if (pManager->FinishAsyncWork)
					{
						return 0;
					}
				}

				return 0;
			}

			// This will be run from a worker thread and will emulate a software queue for making gpu resources resident or evicted.
			// The GPU will be synchronized by this queue to ensure that it never executes using an evicted resource.
			void ProcessPagingWork(AsyncWorkload* pWork)
			{
				Internal::DeviceWideSyncPoint* FirstUncompletedSyncPoint = DequeueCompletedSyncPoints();

				// Use a union so that we only need 1 allocation
				union ResidentScratchSpace
				{
					ManagedObject* pManagedObject;
					ID3D12Pageable* pUnderlying;
				};

				ResidentScratchSpace* pMakeResidentList = nullptr;
				UINT32 NumObjectsToMakeResident = 0;

				ID3D12Pageable** pEvictionList = nullptr;
				UINT32 NumObjectsToEvict = 0;

				// the size of all the objects which will need to be made resident in order to execute this set.
				UINT64 SizeToMakeResident = 0;

				LARGE_INTEGER CurrentTime;
				QueryPerformanceCounter(&CurrentTime);

				{
					// A lock must be taken here as the state of the objects will be altered
					Internal::ScopedLock Lock(&Mutex);

					pMakeResidentList = new ResidentScratchSpace[pWork->pMasterSet->CurrentSetSize];
					pEvictionList = new ID3D12Pageable*[LRU.NumResidentObjects];

					// Mark the objects used by this command list to be made resident
					for (INT32 i = 0; i < pWork->pMasterSet->CurrentSetSize; i++)
					{
						ManagedObject*& pObject = pWork->pMasterSet->ppSet[i];
						// If it's evicted we need to make it resident again
						if (pObject->ResidencyStatus == ManagedObject::RESIDENCY_STATUS::EVICTED)
						{
							pMakeResidentList[NumObjectsToMakeResident++].pManagedObject = pObject;
							LRU.MakeResident(pObject);

							SizeToMakeResident += pObject->Size;
						}

						// Update the last sync point that this was used on
						pObject->LastGPUSyncPoint = pWork->SyncPointGeneration;

						pObject->LastUsedTimestamp = CurrentTime.QuadPart;
						LRU.ObjectReferenced(pObject);
					}

					DXGI_QUERY_VIDEO_MEMORY_INFO LocalMemory;
					ZeroMemory(&LocalMemory, sizeof(LocalMemory));
					GetCurrentBudget(&LocalMemory, DXGI_MEMORY_SEGMENT_GROUP_LOCAL);

					UINT64 EvictionGracePeriod = GetCurrentEvictionGracePeriod(&LocalMemory);
					LRU.TrimAgedAllocations(FirstUncompletedSyncPoint, pEvictionList, NumObjectsToEvict, CurrentTime.QuadPart, EvictionGracePeriod);

					if (NumObjectsToEvict)
					{
						RESIDENCY_CHECK_RESULT(Device->Evict(NumObjectsToEvict, pEvictionList));
						NumObjectsToEvict = 0;
					}

					if (NumObjectsToMakeResident)
					{
						UINT32 ObjectsMadeResident = 0;
						UINT32 MakeResidentIndex = 0;
						while (true)
						{
							ZeroMemory(&LocalMemory, sizeof(LocalMemory));

							GetCurrentBudget(&LocalMemory, DXGI_MEMORY_SEGMENT_GROUP_LOCAL);
							DXGI_QUERY_VIDEO_MEMORY_INFO NonLocalMemory;
							ZeroMemory(&NonLocalMemory, sizeof(NonLocalMemory));
							GetCurrentBudget(&NonLocalMemory, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL);

							INT64 TotalUsage = LocalMemory.CurrentUsage + NonLocalMemory.CurrentUsage;
							INT64 TotalBudget = LocalMemory.Budget + NonLocalMemory.Budget;

							INT64 AvailableSpace = TotalBudget - TotalUsage;

							UINT64 BatchSize = 0;
							UINT32 NumObjectsInBatch = 0;
							UINT32 BatchStart = MakeResidentIndex;

							HRESULT hr = S_OK;
							if (AvailableSpace > 0)
							{
								for (UINT32 i = MakeResidentIndex; i < NumObjectsToMakeResident; i++)
								{
									// If we try to make this object resident, will we go over budget?
									if (BatchSize + pMakeResidentList[i].pManagedObject->Size > UINT64(AvailableSpace))
									{
										// Next time we will start here
										MakeResidentIndex = i;
										break;
									}
									else
									{
										BatchSize += pMakeResidentList[i].pManagedObject->Size;
										NumObjectsInBatch++;
										ObjectsMadeResident++;

										pMakeResidentList[i].pUnderlying = pMakeResidentList[i].pManagedObject->pUnderlying;
									}
								}

								hr = Device->MakeResident(NumObjectsInBatch, &pMakeResidentList[BatchStart].pUnderlying);
								if (SUCCEEDED(hr))
								{
									SizeToMakeResident -= BatchSize;
								}
							}

							if (FAILED(hr) || ObjectsMadeResident != NumObjectsToMakeResident)
							{
								ManagedObject* pResidentHead = LRU.GetResidentListHead();

								// Get the next sync point to wait for
								FirstUncompletedSyncPoint = DequeueCompletedSyncPoints();

								// If there is nothing to trim OR the only objects 'Resident' are the ones about to be used by this execute.
								if (pResidentHead == nullptr ||
									pResidentHead->LastGPUSyncPoint >= pWork->SyncPointGeneration ||
									FirstUncompletedSyncPoint == nullptr)
								{
									// Make resident the rest of the objects as there is nothing left to trim
									UINT32 NumObjects = NumObjectsToMakeResident - ObjectsMadeResident;

									// Gather up the remaining underlying objects
									for (UINT32 i = MakeResidentIndex; i < NumObjectsToMakeResident; i++)
									{
										pMakeResidentList[i].pUnderlying = pMakeResidentList[i].pManagedObject->pUnderlying;
									}

									hr = Device->MakeResident(NumObjects, &pMakeResidentList[MakeResidentIndex].pUnderlying);
									if (FAILED(hr))
									{
										// TODO: What should we do if this fails? This is a catastrophic failure in which the app is trying to use more memory
										//       in 1 command list than can possibly be made resident by the system.
										RESIDENCY_CHECK_RESULT(hr);
									}
									break;
								}

								UINT64 GenerationToWaitFor = FirstUncompletedSyncPoint->GenerationID;

								// We can't wait for the sync-point that this work is intended for
								if (GenerationToWaitFor == pWork->SyncPointGeneration)
								{
									RESIDENCY_CHECK(GenerationToWaitFor >= 0);
									GenerationToWaitFor -= 1;
								}
								// Wait until the GPU is done
								WaitForSyncPoint(GenerationToWaitFor);

								LRU.TrimToSyncPointInclusive(TotalUsage + INT64(SizeToMakeResident), TotalBudget, pEvictionList, NumObjectsToEvict, GenerationToWaitFor);

								RESIDENCY_CHECK_RESULT(Device->Evict(NumObjectsToEvict, pEvictionList));
							}
							else
							{
								// We made everything resident, mission accomplished
								break;
							}
						}
					}

					delete[](pMakeResidentList);
					delete[](pEvictionList);
				}

				// Tell the GPU that it's safe to execute since we made things resident
				RESIDENCY_CHECK_RESULT(AsyncThreadFence.pFence->Signal(pWork->FenceValueToSignal));

				delete(pWork->pMasterSet);
				pWork->pMasterSet = nullptr;
			}
			// The Enqueue and Dequeue Async Work functions are threadsafe as there is only 1 producer and 1 consumer, if that changes
			// Synchronisation will be required
			HRESULT EnqueueAsyncWork(ResidencySet* pMasterSet, UINT64 FenceValueToSignal, UINT64 SyncPointGeneration)
			{
				// We can't get too far ahead of the worker thread otherwise huge hitches occur
				while ((CurrentAsyncWorkloadTail - CurrentAsyncWorkloadHead) >= MaxSoftwareQueueLatency)
				{
					WaitForSingleObject(AsyncThreadWorkCompletionEvent, INFINITE);
				}

				RESIDENCY_CHECK(CurrentAsyncWorkloadTail >= CurrentAsyncWorkloadHead);

				const SIZE_T currentIndex = CurrentAsyncWorkloadTail % AsyncWorkQueueSize;
				AsyncWorkQueue[currentIndex].pMasterSet = pMasterSet;
				AsyncWorkQueue[currentIndex].FenceValueToSignal = FenceValueToSignal;
				AsyncWorkQueue[currentIndex].SyncPointGeneration = SyncPointGeneration;

				CurrentAsyncWorkloadTail++;
				if (SetEvent(AsyncWorkEvent) == false)
				{
					return HRESULT_FROM_WIN32(GetLastError());
				}

				return S_OK;
			}

			AsyncWorkload* DequeueAsyncWork()
			{
				if (CurrentAsyncWorkloadHead == CurrentAsyncWorkloadTail)
				{
					return nullptr;
				}

				const SIZE_T currentHead = CurrentAsyncWorkloadHead % AsyncWorkQueueSize;
				AsyncWorkload* pWork = &AsyncWorkQueue[currentHead];

				CurrentAsyncWorkloadHead++;
				return pWork;
			}

			void GetCurrentBudget(DXGI_QUERY_VIDEO_MEMORY_INFO* InfoOut, DXGI_MEMORY_SEGMENT_GROUP Segment)
			{
				RESIDENCY_CHECK_RESULT(Adapter->QueryVideoMemoryInfo(NodeMask, Segment, InfoOut));
			}

			HRESULT EnqueueSyncPoint()
			{
				Internal::ScopedLock Lock(&AsyncWorkMutex);

				Internal::DeviceWideSyncPoint* pPoint = Internal::DeviceWideSyncPoint::CreateSyncPoint(NumQueuesSeen, CurrentSyncPointGeneration);
				if (pPoint == nullptr)
				{
					return E_OUTOFMEMORY;
				}

				UINT32 i = 0;
				LIST_ENTRY* pFenceEntry = QueueFencesListHead.Flink;
				// Record the current state of each queue we track into this sync point
				while (pFenceEntry != &QueueFencesListHead)
				{
					Internal::Fence* pFence = CONTAINING_RECORD(pFenceEntry, Internal::Fence, ListEntry);
					pFenceEntry = pFenceEntry->Flink;

					pPoint->pQueueSyncPoints[i].pFence = pFence;
					pPoint->pQueueSyncPoints[i].LastUsedValue = pFence->FenceValue - 1;//Minus one as we want the last submitted

					i++;
				}

				Internal::InsertTailList(&InFlightSyncPointsHead, &pPoint->ListEntry);

				return S_OK;
			}

			// Returns a pointer to the first synch point which is not completed
			Internal::DeviceWideSyncPoint* DequeueCompletedSyncPoints()
			{
				Internal::ScopedLock Lock(&AsyncWorkMutex);

				while (Internal::IsListEmpty(&InFlightSyncPointsHead) == false)
				{
					Internal::DeviceWideSyncPoint* pPoint =
						CONTAINING_RECORD(InFlightSyncPointsHead.Flink, Internal::DeviceWideSyncPoint, ListEntry);

					if (pPoint->IsCompleted())
					{
						Internal::RemoveHeadList(&InFlightSyncPointsHead);
						delete pPoint;
					}
					else
					{
						return pPoint;
					}
				}

				return nullptr;
			}

			void WaitForSyncPoint(UINT64 SyncPointID)
			{
				Internal::ScopedLock Lock(&AsyncWorkMutex);

				LIST_ENTRY* pPointEntry = InFlightSyncPointsHead.Flink;
				while (pPointEntry != &InFlightSyncPointsHead)
				{
					Internal::DeviceWideSyncPoint* pPoint =
						CONTAINING_RECORD(InFlightSyncPointsHead.Flink, Internal::DeviceWideSyncPoint, ListEntry);

					if (pPoint->GenerationID > SyncPointID)
					{
						// this point is already done
						return;
					}
					else if (pPoint->GenerationID < SyncPointID)
					{
						// Keep popping off until we find the one to wait on
						Internal::RemoveHeadList(&InFlightSyncPointsHead);
						delete(pPoint);
					}
					else
					{
						pPoint->WaitForCompletion(CompletionEvent);
						Internal::RemoveHeadList(&InFlightSyncPointsHead);
						delete(pPoint);
						return;
					}
				}
			}

			// Generate a result between the minimum period and the maximum period based on the current
			// local memory pressure. I.e. when memory pressure is low, objects will persist longer before
			// being evicted.
			UINT64 GetCurrentEvictionGracePeriod(DXGI_QUERY_VIDEO_MEMORY_INFO* LocalMemoryState)
			{
				double Pressure = (double(LocalMemoryState->CurrentUsage) / double(LocalMemoryState->Budget));
				Pressure = RESIDENCY_MIN(Pressure, 1.0);
				UINT64 Period = UINT64(MaxEvictionGracePeriodTicks * (1.0 - Pressure));

				return RESIDENCY_MAX(RESIDENCY_MIN(Period, MaxEvictionGracePeriodTicks), MinEvictionGracePeriodTicks);
			}

			LIST_ENTRY QueueFencesListHead;
			UINT32 NumQueuesSeen;
			Internal::Fence AsyncThreadFence;

			LIST_ENTRY InFlightSyncPointsHead;
			UINT64 CurrentSyncPointGeneration;

			HANDLE CompletionEvent;
			HANDLE AsyncThreadWorkCompletionEvent;

			ID3D12Device* Device;
			UINT NodeMask;
			IDXGIAdapter3* Adapter;
			Internal::LRUCache LRU;

			Internal::CriticalSection Mutex;

			Internal::CriticalSection ExecutionCS;

			const bool cStartEvicted;

			const float cMinEvictionGracePeriod;
			UINT64 MinEvictionGracePeriodTicks;
			const float cMaxEvictionGracePeriod;
			UINT64 MaxEvictionGracePeriodTicks;

			UINT32 MaxSoftwareQueueLatency;

			SyncManager* pSyncManager;
		};
	}

	class ResidencyManager
	{
	public:
		ResidencyManager() :
			Manager(&SyncManager)
		{
		}

		FORCEINLINE HRESULT Initialize(ID3D12Device* ParentDevice, UINT DeviceNodeMask, IDXGIAdapter3* ParentAdapter, UINT32 MaxLatency)
		{
			return Manager.Initialize(ParentDevice, DeviceNodeMask, ParentAdapter, MaxLatency);
		}

		FORCEINLINE void Destroy()
		{
			Manager.Destroy();
		}

		FORCEINLINE void BeginTrackingObject(ManagedObject* pObject)
		{
			Manager.BeginTrackingObject(pObject);
		}

		FORCEINLINE void EndTrackingObject(ManagedObject* pObject)
		{
			Manager.EndTrackingObject(pObject);
		}

		// One residency set per command-list
		FORCEINLINE HRESULT ExecuteCommandLists(ID3D12CommandQueue* Queue, ID3D12CommandList** CommandLists, ResidencySet** ResidencySets, UINT32 Count)
		{
			return Manager.ExecuteCommandLists(Queue, CommandLists, ResidencySets, Count);
		}

		FORCEINLINE ResidencySet* CreateResidencySet()
		{
			ResidencySet* pSet = new ResidencySet();

			if (pSet)
			{
				pSet->Initialize(&SyncManager);
			}
			return pSet;
		}

		FORCEINLINE void DestroyResidencySet(ResidencySet* pSet)
		{
			delete(pSet);
		}

	private:
		Internal::ResidencyManagerInternal Manager;
		Internal::SyncManager SyncManager;
	};
};
