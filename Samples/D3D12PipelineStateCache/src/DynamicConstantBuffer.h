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
#include "DXSample.h"

using namespace Microsoft::WRL;
using namespace DirectX;

class DynamicConstantBuffer
{
public:
	DynamicConstantBuffer(UINT constantSize, UINT maxDrawsPerFrame, UINT frameCount);
	~DynamicConstantBuffer();

	void Init(ID3D12Device* pDevice);
	void* GetMappedMemory(UINT drawIndex, UINT frameIndex);
	D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress(UINT drawIndex, UINT frameIndex);

private:
	ComPtr<ID3D12Resource> m_constantBuffer;
	void* m_pMappedConstantBuffer;
	UINT  m_alignedPerDrawConstantBufferSize;
	UINT  m_perFrameConstantBufferSize;

	UINT m_frameCount;
	UINT m_maxDrawsPerFrame;
};
