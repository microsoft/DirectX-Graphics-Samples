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

class MemoryMappedFile
{
protected:
	MemoryMappedFile();
	~MemoryMappedFile();

	void Init(std::wstring filename, UINT filesize = DefaultFileSize);
	void Destroy(bool deleteFile);
	void GrowMapping(UINT size);

	void SetSize(UINT size)
	{
		if(m_mapAddress)
		{
			static_cast<UINT*>(m_mapAddress)[0] = size;
		}
	}

	UINT GetSize() const
	{
		if (m_mapAddress)
		{
			return static_cast<UINT*>(m_mapAddress)[0];
		}
		return 0;
	}

	void* GetData()
	{
		if (m_mapAddress)
		{
			// The actual data comes after the length.
			return &static_cast<UINT*>(m_mapAddress)[1];
		}
		return nullptr;
	}

public:
	bool IsMapped() const { return m_mapAddress != nullptr; }
	
protected:
	static const UINT DefaultFileSize = 64;

	HANDLE m_mapFile;
	HANDLE m_file;
	LPVOID m_mapAddress;
	std::wstring m_filename;

	UINT m_currentFileSize;
};
