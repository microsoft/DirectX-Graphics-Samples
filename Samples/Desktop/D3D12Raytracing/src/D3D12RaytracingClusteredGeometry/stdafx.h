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

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define NOMINMAX
#include <windows.h>

// CRT
#include <stdlib.h>
#include <sstream>
#include <iomanip>
#include <list>
#include <string>
#include <wrl.h>
#include <shellapi.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <assert.h>

// DXGI / D3D12 (headers supplied by the Agility SDK NuGet package)
#include <dxgi1_6.h>
#include <d3d12.h>
#include "d3dx12.h"

// WIC for screenshot saving (PNG encoder)
#include <wincodec.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

// DirectXTK12 is supplied by the shared vcpkg manifest and project imports.
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "CommonStates.h"
#include "GraphicsMemory.h"
#include "ResourceUploadBatch.h"

#include "SampleLog.h"
#include "DXSampleHelper.h"
#include "DeviceResources.h"
