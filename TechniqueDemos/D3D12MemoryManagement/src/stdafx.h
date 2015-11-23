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

#define _KB(x) (x * 1024)
#define _MB(x) (x * 1024 * 1024)

#define _64KB _KB(64)
#define _1MB _MB(1)
#define _2MB _MB(2)
#define _4MB _MB(4)
#define _8MB _MB(8)
#define _16MB _MB(16)
#define _32MB _MB(32)
#define _64MB _MB(64)
#define _128MB _MB(128)
#define _256MB _MB(256)

#define TILE_SIZE _64KB
#define MAX_HEAP_SIZE _16MB

//
// Specifies the default size of a buffer in a dynamic buffer.
//
#define DYNAMIC_BUFFER_SIZE (_64KB * 4)

//
// Specifies the default number of descriptors in a dynamic descriptor heap.
//
#define DYNAMIC_HEAP_SIZE 2048

//
// A texture can have a maximum dimension of 16384, or 2^14. This means
// a texture can have a maximum of 15 mip levels, ranging from [0, 14].
//
#define MAX_MIP_COUNT_BITS 4
#define MAX_MIP_COUNT (1 << MAX_MIP_COUNT_BITS)
#define UNDEFINED_MIPMAP_INDEX (MAX_MIP_COUNT - 1)
#define MAX_GENERATED_IMAGES 8

#define SWAPCHAIN_BUFFER_COUNT 2
#define STATISTIC_COUNT 60

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <d3d11on12.h>
#include <d2d1_3.h>
#include <dwrite_2.h>
#include <wincodec.h>
#include <stdio.h>
#include <new>
#include <vector>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

struct Frame;
struct PagingFrame;
struct RenderFrame;
struct ResourceMip;
struct ResourceDeviceState;
struct Resource;
struct Buffer;
struct DescriptorHeap;

class DX12Framework;
class Camera;
class Context;
class PagingWorkerThread;
class PagingContext;
class RenderContext;
class Shader;
class TextureShader;
class ColorShader;
class DynamicBuffer;
class DynamicDescriptorHeap;

struct RectF
{
	float Left;
	float Top;
	float Right;
	float Bottom;
};

struct PointF
{
	float X;
	float Y;
};

struct ColorF
{
	float R;
	float G;
	float B;
	float A;
};

#include "Log.h"
#include "List.h"

#include "Camera.h"
#include "Shader.h"
#include "Versioning.h"
#include "Resource.h"
#include "Util.h"
#include "Context.h"
#include "Render.h"
#include "Paging.h"
#include "Framework.h"

template<typename T>
inline void SafeRelease(T *&rpInterface)
{
	T* pInterface = rpInterface;

	if (pInterface)
	{
		pInterface->Release();
		rpInterface = nullptr;
	}
}

template<typename T>
inline void SafeDelete(T *&rpObject)
{
	T* pObject = rpObject;

	if (pObject)
	{
		delete pObject;
		rpObject = nullptr;
	}
}

#include "D3D12MemoryManagement.h"

#ifdef _DEBUG
#include <Initguid.h>
#include <dxgidebug.h>
#endif
