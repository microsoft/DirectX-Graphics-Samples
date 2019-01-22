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

// Native, hardware-specific, PSO cache using a Pipeline Library.
// Pipeline Libraries allow applications to explicitly group PSOs which are expected to share data.
class MemoryMappedPipelineLibrary : public MemoryMappedFile
{
public:
	bool Init(ID3D12Device* pDevice, std::wstring filename);
	void Destroy(bool deleteFile);
	
	ID3D12PipelineLibrary* GetPipelineLibrary() { return m_pipelineLibrary.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineLibrary> m_pipelineLibrary;
};
