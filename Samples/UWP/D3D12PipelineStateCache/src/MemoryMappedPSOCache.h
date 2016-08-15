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

#include "MemoryMappedFile.h"

// Native, hardware-specific, PSO cache using a Cached Blob.
class MemoryMappedPSOCache : public MemoryMappedFile
{
public:
	void Init(std::wstring filename) { MemoryMappedFile::Init(filename); }
	void Destroy(bool deleteFile) { MemoryMappedFile::Destroy(deleteFile); }
	void Update(ID3DBlob *pBlob);

	size_t GetCachedBlobSize() const { return GetSize(); }
	void* GetCachedBlob() { return GetData(); }
};
