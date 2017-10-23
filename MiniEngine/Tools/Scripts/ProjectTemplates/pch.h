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

#pragma once

#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable:4328) // nonstandard extension used : class rvalue used as lvalue
#pragma warning(disable:4324) // structure was padded due to __declspec(align())

#include <winapifamily.h>		// for WINAPI_FAMILY
#include <wrl.h>

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
	#define NOMINMAX
#endif
#include <windows.h>

#if _XBOX_ONE
#ifndef _DEBUG
    #define D3DCOMPILE_NO_DEBUG 1
#endif
    #include <d3d12_x.h>
    #include <d3dx12_x.h>
    #include <xdk.h>
    #include <pix.h>

    #pragma comment(lib, "d3d12_x.lib")
    #pragma comment(lib, "xg_x.lib")
    #pragma comment(lib, "pixevt.lib")

    #define MY_IID_PPV_ARGS(ppType)  __uuidof(**(ppType)), (void**)(ppType)
#else
    #include <d3d12.h>

    #pragma comment(lib, "d3d12.lib")
    #pragma comment(lib, "dxgi.lib")

    #define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
    #define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)
    #define MY_IID_PPV_ARGS IID_PPV_ARGS

#if _MSC_VER >= 1800
    #include <d3d11_2.h>
    #include <pix3.h>
#endif

    #include "d3dx12.h"

#endif

#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <vector>
#include <memory>
#include <string>
#include <exception>

#include <wrl.h>
#include <ppltasks.h>

#include "Utility.h"
#include "VectorMath.h"
#include "EngineTuning.h"
#include "EngineProfiling.h"
