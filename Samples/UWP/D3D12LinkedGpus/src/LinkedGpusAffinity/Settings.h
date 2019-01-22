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

// Container for common sample state that is initialized and then left constant
// for the lifetime of the sample.
struct Settings
{
	static void Initialize(CD3DX12AffinityDevice* pDevice, UINT width, UINT height);
	static void OnSizeChanged(UINT width, UINT height);

	static const UINT MaxNodeCount = 2;
	static UINT BackBuffersPerNode;
	static UINT FrameCount;								// The maximum number of frames that will be buffered on each node.

	static const UINT SceneHistoryCount = 6;			// The number of frames used in the motion blur effect.
	static const UINT SceneConstantBufferFrames = 10;	// The number of frames that can be buffered in the constant buffer heap.
	static const UINT TriangleCount = 1024;				// The number of triangles drawn per frame.
	static const float TriangleHalfWidth;				// The x and y offsets used by the triangle vertices.
	static const float TriangleDepth;					// The z offset used by the triangle vertices.
	static const float ClearColor[4];

	static UINT NodeCount;								// The number of linked GPUs on the system.
	static UINT SharedNodeMask;							// The mask representing all GPUs on the system.
	static bool Tier2Support;
	static UINT RtvDescriptorSize;
	static UINT CbvSrvDescriptorSize;
	static UINT BackBufferCount;

	static UINT Width;
	static UINT Height;
	static D3D12_VIEWPORT Viewport;
	static D3D12_RECT ScissorRect;
};
