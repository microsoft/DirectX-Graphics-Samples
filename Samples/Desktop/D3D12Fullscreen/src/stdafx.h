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
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include "d3dx12.h"
#include <DirectXMath.h>
#include <pix.h>

//
// The DXGI tearing feature is available on Windows 10 systems with KB3156421 or the
// Anniversary Update installed. Since the 10586 SDK will not be patched, we define
// the relevant symbols if they are missing.
//
#ifdef DXGI_PRESENT_ALLOW_TEARING
	#include <dxgi1_5.h>
#else
	#define DXGI_PRESENT_ALLOW_TEARING          0x00000200UL
	#define DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING  2048

	typedef
	enum DXGI_FEATURE
	{
		DXGI_FEATURE_PRESENT_ALLOW_TEARING = 0
	} DXGI_FEATURE;

	MIDL_INTERFACE("7632e1f5-ee65-4dca-87fd-84cd75f8838d")
	IDXGIFactory5 : public IDXGIFactory4
	{
	public:
		virtual HRESULT STDMETHODCALLTYPE CheckFeatureSupport(
			DXGI_FEATURE Feature,
			_Inout_updates_bytes_(FeatureSupportDataSize) void *pFeatureSupportData,
			UINT FeatureSupportDataSize) = 0;
	};
#endif

#include <wrl.h>
#include <string>
#include <shellapi.h>
