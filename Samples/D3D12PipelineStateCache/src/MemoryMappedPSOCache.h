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

class MemoryMappedPSOCache
{
public:
	MemoryMappedPSOCache();
	~MemoryMappedPSOCache();

	void Init(LPCWSTR filename);
	void Update(ID3DBlob *pBlob);
	void Destroy(bool deleteFile);

	void* GetPointerToStartOfFile() { return m_mapAddress; }

	UINT GetCachedBlobSize()
	{
		if (m_mapAddress)
		{
			return static_cast<UINT*>(m_mapAddress)[0];
		}
		return 0;
	}

	void* GetCachedBlob()
	{
		if (m_mapAddress)
		{
			// The actual data comes after the length.
			return &static_cast<UINT*>(m_mapAddress)[1];
		}
		return nullptr;
	}

private:
	static const UINT m_fileSize = 2 * (1024 * 1024);

	HANDLE m_mapFile;
	HANDLE m_file;
	LPVOID m_mapAddress;
	LPCWSTR m_filename;
};
