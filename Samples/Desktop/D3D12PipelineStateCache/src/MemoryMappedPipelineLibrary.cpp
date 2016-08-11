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
#include "MemoryMappedPipelineLibrary.h"
#include "DXSampleHelper.h"

using std::wstring;
using Microsoft::WRL::ComPtr;

bool MemoryMappedPipelineLibrary::Init(ID3D12Device* pDevice, std::wstring filename)
{
	// ID3D12PipelineLibrary usage requires OS and driver support.
	//		- Pipeline Libraries require the ID3D12Device1 interface (OS support).
	//		- All WDDM 2.1+ drivers are required to support Pipeline Libraries (driver support).


	// Note: Checking for Pipeline Library support is intended to be temporary during the transition period
	// as customers update to the latest version of Windows 10 and drivers are updated to the latest driver model.
	// All future versions of the OS and drivers will support Pipeline Libraries.
	if (pDevice)
	{
		// Create the Pipeline Library.
		ComPtr<ID3D12Device1> device1;
		if (SUCCEEDED(pDevice->QueryInterface(IID_PPV_ARGS(&device1))))
		{
			// Init the memory mapped file.
			MemoryMappedFile::Init(filename);

			// Create a Pipeline Library from the serialized blob.
			// Note: The provided Library Blob must remain valid for the lifetime of the object returned - for efficiency, the data is not copied.
			const HRESULT hr = device1->CreatePipelineLibrary(GetData(), GetSize(), IID_PPV_ARGS(&m_pipelineLibrary));
			switch (hr)
			{
			case DXGI_ERROR_UNSUPPORTED: // The driver doesn't support Pipeline libraries. WDDM2.1 drivers must support it.
				break;

			case E_INVALIDARG: // The provided Library is corrupted or unrecognized.
			case D3D12_ERROR_ADAPTER_NOT_FOUND: // The provided Library contains data for different hardware (Don't really need to clear the cache, could have a cache per adapter).
			case D3D12_ERROR_DRIVER_VERSION_MISMATCH: // The provided Library contains data from an old driver or runtime. We need to re-create it.
				MemoryMappedFile::Destroy(true);
				MemoryMappedFile::Init(filename);
				ThrowIfFailed(device1->CreatePipelineLibrary(GetData(), GetSize(), IID_PPV_ARGS(&m_pipelineLibrary)));
				break;

			default:
				ThrowIfFailed(hr);
			}

			if (m_pipelineLibrary)
			{
				NAME_D3D12_OBJECT(m_pipelineLibrary);
			}
		}
	}

	return m_pipelineLibrary != nullptr;
}

void MemoryMappedPipelineLibrary::Destroy(bool deleteFile)
{
	// If we're not going to destroy the file, serialize the library to disk.
	if (!deleteFile && m_pipelineLibrary)
	{
		// Important: An ID3D12PipelineLibrary object becomes undefined when the underlying memory, that was used to initalize it, changes.

		assert(m_pipelineLibrary->GetSerializedSize() <= UINT_MAX);	// Code below casts to UINT.
		const UINT librarySize = static_cast<UINT>(m_pipelineLibrary->GetSerializedSize());
		if (librarySize > 0)
		{
			// Grow the file if needed.
			const size_t neededSize = sizeof(UINT) + librarySize;
			if (neededSize > m_currentFileSize)
			{
				// The file mapping is going to change thus it will invalidate the ID3D12PipelineLibrary object.
				// Serialize the library contents to temporary memory first.
				void* pTempData = new BYTE[librarySize];
				if (pTempData)
				{
					ThrowIfFailed(m_pipelineLibrary->Serialize(pTempData, librarySize));

					// Now it's safe to grow the mapping.
					MemoryMappedFile::GrowMapping(librarySize);

					// Save the size of the library and the library itself.
					memcpy(GetData(), pTempData, librarySize);
					MemoryMappedFile::SetSize(librarySize);

					delete[] pTempData;
					pTempData = nullptr;
				}
			}
			else
			{
				// The mapping didn't change, we can serialize directly to the mapped file.
				// Save the size of the library and the library itself.
				assert(neededSize <= m_currentFileSize);
				ThrowIfFailed(m_pipelineLibrary->Serialize(GetData(), librarySize));
				MemoryMappedFile::SetSize(librarySize);
			}

			// m_pipelineLibrary is now undefined because we just wrote to the mapped file, don't use it again.
			// This is ok in this sample because we only write to the mapped file when the sample exits.
		}
	}

	MemoryMappedFile::Destroy(deleteFile);
	m_pipelineLibrary = nullptr;
}
