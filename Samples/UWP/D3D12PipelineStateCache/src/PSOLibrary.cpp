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

#include "stdafx.h"
#include "PSOLibrary.h"

using namespace Microsoft::WRL::Wrappers;

PSOLibrary::PSOLibrary(UINT frameCount, UINT cbvRootSignatureIndex) :
	m_cbvRootSignatureIndex(cbvRootSignatureIndex),
	m_maxDrawsPerFrame(256),
	m_dynamicCB(sizeof(UberShaderConstantBuffer), m_maxDrawsPerFrame, frameCount),
	m_flagsMutex(),
	m_useUberShaders(true),
	m_useDiskLibraries(true),
	m_psoCachingMechanism(PSOCachingMechanism::PipelineLibraries),
	m_drawIndex(0),
	m_compiledPSOFlags{},
	m_inflightPSOFlags{},
	m_workerThreads{}
{
	m_cachePath = Windows::Storage::ApplicationData::Current->LocalCacheFolder->Path->Data();
	m_cachePath += L"\\";

	m_flagsMutex = CreateMutex(nullptr, FALSE, nullptr);
}

PSOLibrary::~PSOLibrary()
{
	WaitForThreads();

	for (UINT i = 0; i < EffectPipelineTypeCount; i++)
	{
		m_diskCaches[i].Destroy(false);
	}

	// The Pipeline Library is saved to disk on exit.
	m_pipelineLibrary.Destroy(false);
}

void PSOLibrary::WaitForThreads()
{
	for (auto& thread : m_workerThreads)
	{
		if (thread.threadHandle)
		{
			WaitForSingleObject(thread.threadHandle, INFINITE);
			CloseHandle(thread.threadHandle);
		}
		thread.threadHandle = nullptr;
	}
}

void PSOLibrary::Build(ID3D12Device* pDevice, ID3D12RootSignature* pRootSignature)
{
	// Initialize all cache file mappings (file may be empty).
	m_pipelineLibrariesSupported = m_pipelineLibrary.Init(pDevice, m_cachePath + g_cPipelineLibraryFileName);
	for (UINT i = 0; i < EffectPipelineTypeCount; i++)
	{
		m_diskCaches[i].Init(m_cachePath + g_cCacheFileNames[i]);
	}

	// Use Pipeline Libraries for PSO Caching, if available.
	if (!m_pipelineLibrariesSupported)
	{
		// Fallback to Cached Blobs.
		m_psoCachingMechanism = PSOCachingMechanism::CachedBlobs;
	}

	// Always compile the 3D shader and the Ubershader.
	for (UINT i = 0; i < BaseEffectCount; i++)
	{
		m_workerThreads[i].pDevice = pDevice;
		m_workerThreads[i].pRootSignature = pRootSignature;
		m_workerThreads[i].type = EffectPipelineType(i);
		m_workerThreads[i].pLibrary = this;
		CompilePSO(&m_workerThreads[i]);
	}

	m_dynamicCB.Init(pDevice);
}


void PSOLibrary::SetPipelineState(
	ID3D12Device* pDevice,
	ID3D12RootSignature* pRootSignature,
	ID3D12GraphicsCommandList* pCommandList,
	_In_range_(0, EffectPipelineTypeCount-1) EffectPipelineType type,
	UINT frameIndex)
{
	assert(m_drawIndex < m_maxDrawsPerFrame);

	bool isBuilt = false;
	bool isInFlight = false;

	{
		// Take the lock to figure out if we need to build this thing or use an Uber shader.
		auto lock = Mutex::Lock(m_flagsMutex);

		isBuilt = m_compiledPSOFlags[type];
		isInFlight = m_inflightPSOFlags[type];
	}

	if (type > BaseUberShader)
	{
		// If an effect hasn't been built yet.
		if (!isBuilt && m_useUberShaders)
		{
			// Let the uber shader know what effect it should configure for.
			UberShaderConstantBuffer* constantData = (UberShaderConstantBuffer*)m_dynamicCB.GetMappedMemory(m_drawIndex, frameIndex);
			constantData->effectIndex = type;
			pCommandList->SetGraphicsRootConstantBufferView(m_cbvRootSignatureIndex, m_dynamicCB.GetGpuVirtualAddress(m_drawIndex, frameIndex));

			// We don't want to double compile.
			if (!isInFlight)
			{
				m_workerThreads[type].pDevice = pDevice;
				m_workerThreads[type].pRootSignature = pRootSignature;
				m_workerThreads[type].type = type;
				m_workerThreads[type].pLibrary = this;

				// Compile the PSO on a background thread.
				m_workerThreads[type].threadHandle = CreateThread(
					nullptr,
					0,
					reinterpret_cast<LPTHREAD_START_ROUTINE>(CompilePSO),
					reinterpret_cast<void*>(&m_workerThreads[type]),
					CREATE_SUSPENDED,
					nullptr);

				if (!m_workerThreads[type].threadHandle)
				{
					ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
				}

				ResumeThread(m_workerThreads[type].threadHandle);

				{
					auto lock = Mutex::Lock(m_flagsMutex);

					m_inflightPSOFlags[type] = true;
				}
			}

			type = BaseUberShader;
		}
		else if (!isBuilt && !m_useUberShaders)
		{
			// When not using ubershaders this will take a long time and cause a hitch as the 
			// CPU is stalled!
			m_workerThreads[type].pDevice = pDevice;
			m_workerThreads[type].pRootSignature = pRootSignature;
			m_workerThreads[type].type = type;
			m_workerThreads[type].pLibrary = this;

			CompilePSO(&m_workerThreads[type]);
		}
	}
	else
	{
		// We should always have the base shaders around.
		assert(isBuilt);
	}

	pCommandList->SetPipelineState(m_pipelineStates[type].Get());

	m_drawIndex++;
}

void PSOLibrary::CompilePSO(CompilePSOThreadData* pDataPackage)
{
	PSOLibrary* pLibrary = pDataPackage->pLibrary;
	ID3D12Device* pDevice = pDataPackage->pDevice;
	ID3D12RootSignature* pRootSignature = pDataPackage->pRootSignature;
	EffectPipelineType type = pDataPackage->type;
	bool useCache = false;
	bool sleepToEmulateComplexCreatePSO = false;

	{
		auto lock = Mutex::Lock(pLibrary->m_flagsMutex);

		// When using the disk cache compilation should be extremely quick so don't sleep.
		useCache = pLibrary->m_useDiskLibraries;
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc = {};
	baseDesc.pRootSignature = pRootSignature;
	baseDesc.SampleMask = UINT_MAX;
	baseDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	baseDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	baseDesc.NumRenderTargets = 1;
	baseDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	baseDesc.SampleDesc.Count = 1;
	baseDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	baseDesc.DepthStencilState.DepthEnable = FALSE;
	baseDesc.DepthStencilState.StencilEnable = FALSE;

	baseDesc.InputLayout = g_cEffectShaderData[type].inputLayout;
	baseDesc.VS = g_cEffectShaderData[type].VS;
	baseDesc.PS = g_cEffectShaderData[type].PS;
	baseDesc.DS = g_cEffectShaderData[type].DS;
	baseDesc.HS = g_cEffectShaderData[type].HS;
	baseDesc.GS = g_cEffectShaderData[type].GS;

	if (useCache && 
		(pLibrary->m_psoCachingMechanism == PSOCachingMechanism::PipelineLibraries))
	{
		assert(pLibrary->m_pipelineLibrary.IsMapped());
		ID3D12PipelineLibrary* pPipelineLibrary = pLibrary->m_pipelineLibrary.GetPipelineLibrary();

		// Note: Load*Pipeline() will auto-name PSOs for you based on the provided name. However, this sample overrides those names.
		HRESULT hr = pPipelineLibrary->LoadGraphicsPipeline(g_cEffectNames[type], &baseDesc, IID_PPV_ARGS(&pLibrary->m_pipelineStates[type]));
		if (E_INVALIDARG == hr)
		{
			// A PSO with the specified name doesn’t exist, or the input desc doesn’t match the data in the library.
			// Create the PSO and then store it in the library for next time.
			ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pLibrary->m_pipelineStates[type])));

			// Note: You don't need to pass StorePipeline() a name if the object is already named. If the name parameter is null, it will use the object's name.
			hr = pPipelineLibrary->StorePipeline(g_cEffectNames[type], pLibrary->m_pipelineStates[type].Get());
			if (E_INVALIDARG == hr)
			{
				// A PSO with the specified name already exists in the library.
				// This shouldn't happen in this sample, but depending on how you name the PSOs collisions are possible.
			}
			else
			{
				ThrowIfFailed(hr);
			}

			sleepToEmulateComplexCreatePSO = true;
		}
		else
		{
			ThrowIfFailed(hr);
		}
	}
	else if (useCache && 
		(pLibrary->m_psoCachingMechanism == PSOCachingMechanism::CachedBlobs))
	{
		// Read how long the cached shader blob is.
		assert(pLibrary->m_diskCaches[type].IsMapped());
		size_t size = pLibrary->m_diskCaches[type].GetCachedBlobSize();

		// If the size if 0 then this disk cache needs to be refreshed.
		if (size == 0)
		{
			ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pLibrary->m_pipelineStates[type])));

			ComPtr<ID3DBlob> blob;
			pLibrary->m_pipelineStates[type]->GetCachedBlob(&blob);
			pLibrary->m_diskCaches[type].Update(blob.Get());

			sleepToEmulateComplexCreatePSO = true;
		}
		else
		{
			// Read in the blob data from disk to avoid compiling it.
			baseDesc.CachedPSO.pCachedBlob = pLibrary->m_diskCaches[type].GetCachedBlob();
			baseDesc.CachedPSO.CachedBlobSizeInBytes = pLibrary->m_diskCaches[type].GetCachedBlobSize();

			HRESULT hr = pDevice->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pLibrary->m_pipelineStates[type]));

			// If compilation fails the cache is probably stale. (old drivers etc.)
			if (FAILED(hr))
			{
				baseDesc.CachedPSO = {};
				ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pLibrary->m_pipelineStates[type])));

				ComPtr<ID3DBlob> blob;
				pLibrary->m_pipelineStates[type]->GetCachedBlob(&blob);
				pLibrary->m_diskCaches[type].Update(blob.Get());

				sleepToEmulateComplexCreatePSO = true;
			}
		}
	}
	else
	{
		ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pLibrary->m_pipelineStates[type])));

		sleepToEmulateComplexCreatePSO = true;
	}

	// The effects are very simple and should compile quickly so we'll sleep to emulate something more complex.
	if (sleepToEmulateComplexCreatePSO && type > BaseUberShader)
	{
		Sleep(500);
	}

	WCHAR name[50];
	if (swprintf_s(name, L"m_pipelineStates[%s]", g_cEffectNames[type]) > 0)
	{
		SetName(pLibrary->m_pipelineStates[type].Get(), name);
	}

	{
		auto lock = Mutex::Lock(pLibrary->m_flagsMutex);

		pLibrary->m_compiledPSOFlags[type] = true;
		pLibrary->m_inflightPSOFlags[type] = false;
	}
}

void PSOLibrary::EndFrame()
{
	m_drawIndex = 0;
}

void PSOLibrary::ClearPSOCache()
{
	WaitForThreads();

	for (size_t i = PostBlit; i < EffectPipelineTypeCount; i++)
	{
		if (m_pipelineStates[i])
		{
			m_pipelineStates[i] = nullptr;
			m_compiledPSOFlags[i] = false;
			m_inflightPSOFlags[i] = false;
		}
	}

	// Clear the disk caches.
	for (size_t i = 0; i < EffectPipelineTypeCount; i++)
	{
		m_diskCaches[i].Destroy(true);
	}

	m_pipelineLibrary.Destroy(true);
}

void PSOLibrary::ToggleUberShader()
{
	m_useUberShaders = !m_useUberShaders;
}

void PSOLibrary::ToggleDiskLibrary()
{
	{
		auto lock = Mutex::Lock(m_flagsMutex);

		m_useDiskLibraries = !m_useDiskLibraries;
	}

	WaitForThreads();
}

void PSOLibrary::SwitchPSOCachingMechanism()
{
	{
		auto lock = Mutex::Lock(m_flagsMutex);

		UINT newMechanism = static_cast<UINT>(m_psoCachingMechanism) + 1;
		newMechanism = newMechanism % PSOCachingMechanism::PSOCachingMechanismCount;

		// Don't allow Pipeline Libraries if they're not available.
		if (!m_pipelineLibrariesSupported && (newMechanism == PSOCachingMechanism::PipelineLibraries))
		{
			newMechanism++;
			newMechanism = newMechanism % PSOCachingMechanism::PSOCachingMechanismCount;
		}
		
		m_psoCachingMechanism = static_cast<PSOCachingMechanism>(newMechanism);
	}

	WaitForThreads();
}

void PSOLibrary::DestroyShader(EffectPipelineType type)
{
	WaitForThreads();

	if (m_pipelineStates[type])
	{
		m_pipelineStates[type] = nullptr;
		m_compiledPSOFlags[type] = false;
		m_inflightPSOFlags[type] = false;
	}
}
