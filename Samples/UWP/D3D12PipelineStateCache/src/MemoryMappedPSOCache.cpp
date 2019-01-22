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
#include "MemoryMappedPSOCache.h"

void MemoryMappedPSOCache::Update(ID3DBlob* pBlob)
{
	if (pBlob)
	{
		assert(pBlob->GetBufferSize() <= UINT_MAX);	// Code below casts to UINT.
		const UINT blobSize = static_cast<UINT>(pBlob->GetBufferSize());
		if (blobSize > 0)
		{
			// Grow the file if needed.
			const size_t neededSize = sizeof(UINT) + blobSize;
			if (neededSize > m_currentFileSize)
			{
				MemoryMappedFile::GrowMapping(blobSize);
			}

			// Save the size of the blob, and then the blob itself.
			assert(neededSize <= m_currentFileSize);
			MemoryMappedFile::SetSize(blobSize);
			memcpy(GetCachedBlob(), pBlob->GetBufferPointer(), pBlob->GetBufferSize());
		}
	}
}