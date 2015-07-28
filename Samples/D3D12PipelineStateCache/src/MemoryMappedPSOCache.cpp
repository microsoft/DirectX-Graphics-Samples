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

MemoryMappedPSOCache::MemoryMappedPSOCache() :
	m_mapFile(INVALID_HANDLE_VALUE),
	m_file(INVALID_HANDLE_VALUE),
	m_mapAddress(nullptr)
{
}

MemoryMappedPSOCache::~MemoryMappedPSOCache()
{
}

void MemoryMappedPSOCache::Init(LPCWSTR filename)
{
	m_filename = filename;
	WIN32_FIND_DATA findFileData;
	HANDLE handle = FindFirstFile(filename, &findFileData);
	bool found = handle != INVALID_HANDLE_VALUE;

	if (found)
	{
		FindClose(handle);
	}

	m_file = CreateFile(
		filename,
		GENERIC_READ | GENERIC_WRITE,
		0,
		nullptr,
		(found) ? OPEN_EXISTING : CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL,
		nullptr);

	if (m_file == INVALID_HANDLE_VALUE)
	{
		std::cerr << (L"m_file is invalid\n");
		std::cerr << (L"Target file is %s\n", filename);
		return;
	}

	m_mapFile = CreateFileMapping(m_file, nullptr, PAGE_READWRITE, 0, m_fileSize, nullptr);

	if (m_mapFile == nullptr)
	{
		std::cerr << (L"m_mapFile is NULL: last error: %d\n", GetLastError());
		assert(false);
		return;
	}

	m_mapAddress = MapViewOfFile(m_mapFile, FILE_MAP_ALL_ACCESS, 0, 0, m_fileSize);

	if (m_mapAddress == nullptr)
	{
		std::cerr << (L"m_mapAddress is NULL: last error: %d\n", GetLastError());
		assert(false);
		return;
	}
}

void MemoryMappedPSOCache::Update(ID3DBlob* pBlob)
{
	static_cast<UINT*>(m_mapAddress)[0] = static_cast<UINT>(pBlob->GetBufferSize());
	memcpy(GetCachedBlob(), pBlob->GetBufferPointer(), pBlob->GetBufferSize());
}

void MemoryMappedPSOCache::Destroy(bool deleteFile)
{
	if (m_mapAddress)
	{
		BOOL flag = UnmapViewOfFile(m_mapAddress);

		m_mapAddress = nullptr;
		flag = CloseHandle(m_mapFile);	// Close the file mapping object.

		if (!flag)
		{
			std::cerr << (L"\nError %ld occurred closing the mapping object!", GetLastError());
			assert(false);
		}

		flag = CloseHandle(m_file);		// Close the file itself.

		if (!flag)
		{
			std::cerr << (L"\nError %ld occurred closing the file!", GetLastError());
			assert(false);
		}
	}

	if (deleteFile)
	{
		DeleteFile(m_filename);
	}
}
