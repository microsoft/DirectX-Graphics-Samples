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

//
// A render frame has additional information used to track objects and data used
// during rendering operations. This is limited mostly to buffers and heaps which
// can grow dynamically when they run out of space. For example, if the vertex
// buffer hits its capacity while generating geometry, a new vertex buffer can be
// generating in a process called 'renaming,' allowing us to track the old buffer,
// while using the new one. Once the rendering frame is retired, the renamed buffers
// are then released back to the system to prevent memory leaks.
//
struct RenderFrame : Frame
{
	LIST_ENTRY PendingBufferListHead;
	LIST_ENTRY PendingHeapListHead;

	DynamicBuffer VertexBuffer;
	DynamicBuffer ConstantBuffer;
	DynamicDescriptorHeap SrvCbvHeap;

	D3D12_GPU_VIRTUAL_ADDRESS CameraAddress;
};

//
// The render context is a specialization of the Context which creates a unique
// command queue for 3D rendering workloads, which can run asynchronously from
// paging operations.
//
class RenderContext : public Context
{
protected:
	virtual void RetireFrame(Frame* pFrame);

public:
	RenderContext(DX12Framework* pFramework);
	~RenderContext();

	HRESULT CreateDeviceDependentState(UINT MaxFrameCount);
	void DestroyDeviceDependentState();

	void CloseBuffer(Buffer* pBuffer);
	void CloseHeap(DescriptorHeap* pHeap);

	inline RenderFrame* GetCurrentFrame() const
	{
		return static_cast<RenderFrame*>(m_pCurrentFrame);
	}
};
