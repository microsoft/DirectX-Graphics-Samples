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
#include "MemoryMappedFile.h"

MemoryMappedFile::MemoryMappedFile() :
	m_mapFile(INVALID_HANDLE_VALUE),
	m_file(INVALID_HANDLE_VALUE),
	m_mapAddress(nullptr),
	m_currentFileSize(0)
{
}

MemoryMappedFile::~MemoryMappedFile()
{
}

void MemoryMappedFile::Init(std::wstring filename, UINT fileSize)
{
	m_filename = filename;
	WIN32_FIND_DATA findFileData;
	HANDLE handle = FindFirstFileEx(filename.c_str(), FindExInfoBasic, &findFileData, FindExSearchNameMatch, nullptr, 0);
	bool found = handle != INVALID_HANDLE_VALUE;

	if (found)
	{
		FindClose(handle);
	}

	m_file = CreateFile2(
		filename.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		(found) ? OPEN_EXISTING : CREATE_NEW,
		nullptr);

	if (m_file == INVALID_HANDLE_VALUE)
	{
		std::cerr << (L"m_file is invalid. Error %ld.\n", GetLastError());
		std::cerr << (L"Target file is %s\n", filename.c_str());
		return;
	}

	LARGE_INTEGER realFileSize = {};
	BOOL flag = GetFileSizeEx(m_file, &realFileSize);
	if (!flag)
	{
		std::cerr << (L"\nError %ld occurred in GetFileSizeEx!", GetLastError());
		assert(false);
		return;
	}

	assert(realFileSize.HighPart == 0);
	m_currentFileSize = realFileSize.LowPart;
	if (m_currentFileSize == 0)
	{
		// File mapping files with a size of 0 produces an error.
		m_currentFileSize = DefaultFileSize;
	}
	else if(fileSize > m_currentFileSize)
	{
		// Grow to the specified size.
		m_currentFileSize = fileSize;
	}

	m_mapFile = CreateFileMapping(m_file, nullptr, PAGE_READWRITE, 0, m_currentFileSize, nullptr);

	if (m_mapFile == nullptr)
	{
		std::cerr << (L"m_mapFile is NULL: last error: %d\n", GetLastError());
		assert(false);
		return;
	}

	m_mapAddress = MapViewOfFile(m_mapFile, FILE_MAP_ALL_ACCESS, 0, 0, m_currentFileSize);

	if (m_mapAddress == nullptr)
	{
		std::cerr << (L"m_mapAddress is NULL: last error: %d\n", GetLastError());
		assert(false);
		return;
	}
}

void MemoryMappedFile::Destroy(bool deleteFile)
{
	if (m_mapAddress)
	{
		BOOL flag = UnmapViewOfFile(m_mapAddress);
		if (!flag)
		{
			std::cerr << (L"\nError %ld occurred unmapping the view!", GetLastError());
			assert(false);
		}

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
		DeleteFile(m_filename.c_str());
	}
}

void MemoryMappedFile::GrowMapping(UINT size)
{
	// Add space for the extra size at the beginning of the file.
	size += sizeof(UINT);

	// Check the size.
	if (size <= m_currentFileSize)
	{
		// Don't shrink.
		return;
	}

	// Flush.
	BOOL flag = FlushViewOfFile(m_mapAddress, 0);
	if (!flag)
	{
		std::cerr << (L"\nError %ld occurred flushing the mapping object!", GetLastError());
		assert(false);
	}

	// Close the current mapping.
	Destroy(false);

	// Update the size and create a new mapping.
	m_currentFileSize = size;
	Init(m_filename, m_currentFileSize);
}
