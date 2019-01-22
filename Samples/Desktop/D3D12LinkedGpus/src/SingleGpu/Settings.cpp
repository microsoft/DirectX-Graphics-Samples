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
#include "Settings.h"
#include "DXSampleHelper.h"

using Microsoft::WRL::ComPtr;

const float Settings::TriangleHalfWidth = 0.05f;	// The x and y offsets used by the triangle vertices.
const float Settings::TriangleDepth = 1.0f;			// The z offset used by the triangle vertices.
const float Settings::ClearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };

UINT Settings::FrameCount;
UINT Settings::RtvDescriptorSize;
UINT Settings::CbvSrvDescriptorSize;
UINT Settings::BackBufferCount;

UINT Settings::Width;
UINT Settings::Height;
D3D12_VIEWPORT Settings::Viewport;
D3D12_RECT Settings::ScissorRect;

void Settings::Initialize(ID3D12Device* pDevice, UINT width, UINT height)
{
	FrameCount = 2;

	RtvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CbvSrvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	BackBufferCount = 2;

	OnSizeChanged(width, height);
}

void Settings::OnSizeChanged(UINT width, UINT height)
{
	Width = width;
	Height = height;
	Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f);
	ScissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));
}
