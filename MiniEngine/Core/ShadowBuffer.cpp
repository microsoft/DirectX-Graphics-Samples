//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#include "pch.h"
#include "ShadowBuffer.h"
#include "EsramAllocator.h"
#include "CommandContext.h"

void ShadowBuffer::Create( const std::wstring& name, size_t width, size_t height, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr )
{
	DepthBuffer::Create( name, width, height, DXGI_FORMAT_D16_UNORM, VidMemPtr );

	m_Viewport.TopLeftX = 0.0f;
    m_Viewport.TopLeftY = 0.0f;
    m_Viewport.Width = (float)width;
    m_Viewport.Height = (float)height;
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;

	// Prevent drawing to the boundary pixels so that we don't have to worry about shadows stretching
	m_Scissor.left = 1;
	m_Scissor.top = 1;
	m_Scissor.right = (LONG)width - 2;
	m_Scissor.bottom = (LONG)height - 2;
}

void ShadowBuffer::Create( const std::wstring& name, size_t width, size_t height, EsramAllocator& esramAllocator )
{
	DepthBuffer::Create( name, width, height, DXGI_FORMAT_D16_UNORM, esramAllocator );

	m_Viewport.TopLeftX = 0.0f;
    m_Viewport.TopLeftY = 0.0f;
    m_Viewport.Width = (float)width;
    m_Viewport.Height = (float)height;
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;

	// Prevent drawing to the boundary pixels so that we don't have to worry about shadows stretching
	m_Scissor.left = 1;
	m_Scissor.top = 1;
	m_Scissor.right = (LONG)width - 2;
	m_Scissor.bottom = (LONG)height - 2;
}

void ShadowBuffer::BeginRendering( GraphicsContext& Context )
{
	Context.ClearDepth(*this);
	Context.SetDepthStencilTarget(*this);
	Context.SetViewportAndScissor(m_Viewport, m_Scissor);
}

void ShadowBuffer::EndRendering( GraphicsContext& Context )
{
	Context.TransitionResource(*this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}
