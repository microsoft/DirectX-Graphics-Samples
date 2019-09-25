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


// Use the C++ standard templated min/max
#define NOMINMAX

#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>

#include <list>
#include <string>
#include <wrl.h>
#include <shellapi.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <atlbase.h>
#include <assert.h>
#include <array>
#include <unordered_map>
#include <functional>
#include <random>
#include <numeric>
#include <iterator>
#include <sal.h>
#include <stack>

#include <stdint.h>
#include <float.h>
#include <map>
#include <set>

#include <dxgi1_6.h>
#include <d3d12.h>
#include <atlbase.h>
#include "d3dx12.h"	
#include <pix3.h>

#include <GeometricPrimitive.h>
#include <DirectXMath.h>
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include "ResourceUploadBatch.h"
#include <Keyboard.h>
#include <DirectXCollision.h>

// UI 
#include <d2d1_3.h>
#include <dwrite.h>
#include <d3d11on12.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include "RaytracingHlslCompat.h"
#include "RaytracingSceneDefines.h"
#include "DXSampleHelper.h"
#include "DeviceResources.h"

