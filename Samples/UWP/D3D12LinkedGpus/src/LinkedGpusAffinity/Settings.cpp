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

UINT Settings::NodeCount;
UINT Settings::BackBuffersPerNode;
UINT Settings::FrameCount;
UINT Settings::SharedNodeMask;
bool Settings::Tier2Support;
UINT Settings::RtvDescriptorSize;
UINT Settings::CbvSrvDescriptorSize;
UINT Settings::BackBufferCount;

UINT Settings::Width;
UINT Settings::Height;
D3D12_VIEWPORT Settings::Viewport;
D3D12_RECT Settings::ScissorRect;

void Settings::Initialize(CD3DX12AffinityDevice* pDevice, UINT width, UINT height)
{
	NodeCount = pDevice->GetNodeCount();
	_ASSERT(NodeCount <= MaxNodeCount);

	BackBuffersPerNode = (NodeCount > 1) ? 1 : 2;
	FrameCount = (NodeCount > 1) ? 1 : 2;
	_ASSERT(FrameCount * NodeCount <= SceneConstantBufferFrames);

	D3D12_FEATURE_DATA_D3D12_OPTIONS featureOptions;
	ThrowIfFailed(pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &featureOptions, sizeof(featureOptions)));
	Tier2Support = (featureOptions.CrossNodeSharingTier == D3D12_CROSS_NODE_SHARING_TIER_2);

	RtvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CbvSrvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	SharedNodeMask = (1 << NodeCount) - 1;
	BackBufferCount = NodeCount * BackBuffersPerNode;

	// We assume that each node will be assigned the same number of render targets.
	_ASSERT(SceneHistoryCount % NodeCount == 0);

	OnSizeChanged(width, height);
}

void Settings::OnSizeChanged(UINT width, UINT height)
{
	Width = width;
	Height = height;
	Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f);
	ScissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));
}
