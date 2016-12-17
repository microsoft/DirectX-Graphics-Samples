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
#include "CrossNodeResources.h"
#include "GpuNode.h"

using Microsoft::WRL::ComPtr;

// Demonstrate how to render a scene on multiple GPUs using alternate frame rendering (AFR).
// Each GPU node available to the application will render a frame in round-robin fashion.
//
// The rendering consists of two passes:
// - The scene pass draws a number of triangles of different colors. The triangles
//   animate across the screen as the frames progress.
// - The post-processing pass takes the render targets of the previous 6 scene passes and
//   blends them together to produce a motion blur effect.
//
// When this sample runs on a system with linked GPUs, each node will take turns
// rendering the scene pass and share that frame's resulting render target with the
// other linked nodes.
class D3D12LinkedGpus : public DXSample
{
public:
	D3D12LinkedGpus(UINT width, UINT height, std::wstring name);

protected:
	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnSizeChanged(UINT width, UINT height, bool minimized);
	virtual void OnKeyDown(UINT8 key);
	virtual void OnDestroy();

private:
	// Data about each of the triangles in the scene.
	std::vector<SceneConstantBuffer> m_sceneData;

	std::shared_ptr<CrossNodeResources> m_crossNodeResources;
	std::shared_ptr<GpuNode> m_nodes[Settings::MaxNodeCount];

	UINT64 m_frameId;
	UINT m_simulatedGpuLoad;
	UINT m_nodeIndex;
	UINT m_syncInterval;
	bool m_windowVisible;
	bool m_windowedMode;

	void LoadSceneData();
	float GetRandomFloat(float min, float max);
	void UpdateWindowTitle();
	void WaitForGpus();
	void MoveToNextFrame();
};
