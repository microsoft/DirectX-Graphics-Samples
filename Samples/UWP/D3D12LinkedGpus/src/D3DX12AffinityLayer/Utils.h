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

/**
 * This is a utility header with:
 * - Configuration macros
 * - Commonly used (STL) headers
 * - Logging/Assertion methods for debugging
 */

#pragma once

#include <stdio.h>
#include <wtypes.h>

////////////////////////////
// CONFIG //////////////////
////////////////////////////

//#define D3DX12_DEBUG_CLEAR_WHITE

// Max number of nodes(devices or lda nodes) useable by the affinity layer.
// Note that it's fine to have less than this. You don't need to
// recompile anything if you pull a GPU out of your system.
#define D3DX12_MAX_ACTIVE_NODES 2

// This mode allows catching bugs in the affinity layer itself
// by running the recorder for n GPU nodes but execute on GPU0.
//#define D3DX12_SIMULATE_LDA_ON_SINGLE_NODE 1

// Insert a fence, wait and a check for device removed after every commandlist
// useful for debugging the source command-list when a TDR occurs.
//#define SERIALIZE_COMMNANDLIST_EXECUTION

//#define SYNC_CROSS_FRAME_RESOURCES

//#define DEBUG_OBJECT_NAME

// On LDA devices, makes all buffers have a single GPUVA by either having a single
// resource in system memory, or having a single reserved buffer mapped to separate
// heaps on each GPU (via unicast page table mappings). Removes any GPUVA translation
// overhead.
#define TILE_MAPPING_GPUVA 1

// Just a fun experiment to see what happens when all buffers are accessed from remote GPUs.
//#define FORCE_REMOTE_TILE_MAPPING_GPUVA 1

//#define ALWAYS_RESET_ALL_COMMAND_LISTS 1

////////////////////////////
// DEBUG CONFIG ////////////
////////////////////////////

// Logging options. (e.g. memory mapped, every ECL)
//#define ENABLE_DEBUG_LOG 1
//#define ENABLE_RELEASE_LOG 1
//#define D3DX_AFFINITY_ENABLE_HEAP_POINTER_VALIDATION

// Instead of using GetWriteWatch, just upload ALL mapped data when unmapped or ECL.
// This is mostly just used to prove how SLOW it is not to use GetWriteWatch.
//#define DO_FULL_MAPPED_MEM_COPY

////////////////////////////
////////////////////////////

#include <windows.h>
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <cstdio>

struct EAffinityMask
{
    enum Mask
    {
        AllNodes = 0,
        Node0 = 0x01,
        Node1 = 0x02,
        Node2 = 0x04,
        Node3 = 0x08,
        Node4 = 0x10,
    };
};

struct EAffinityMode
{
    enum Mask
    {
        LDA,
    };
};

struct EAffinityRenderingMode
{
    enum Mask
    {
        AFR,
    };
};

void WriteHRESULTError(HRESULT const hr);

#define RETURN_IF_FAILED(x) do { HRESULT const _hr_check = (x); if (S_OK != _hr_check) { WriteHRESULTError(_hr_check); return _hr_check; } } while (false);
#define RETURN_IF_FAILED_WITH_ERROR_LOG(x, y, z) do { HRESULT const _hr_check = (x); if (S_OK != _hr_check) { WriteHRESULTError(_hr_check); z->WriteApplicationMessage(D3D12_MESSAGE_SEVERITY_ERROR, (y)); return _hr_check; } } while (false);

#ifdef DEBUG

#include <cassert>

#define METHOD_NOT_YET_IMPLEMENTED(x) _wassert(L"Method not yet implemented: " L ## #x, L"", 0);
#define HRESULT_METHOD_NOT_YET_IMPLEMENTED(x) _wassert(L"Method not yet implemented: " L ## #x, L"", 0); return E_NOTIMPL;
#define DEBUG_ASSERT(x) assert((x));
#define DEBUG_FAIL_MESSAGE(x) _wassert((x), L"", 0);

#else

#define METHOD_NOT_YET_IMPLEMENTED(x)
#define HRESULT_METHOD_NOT_YET_IMPLEMENTED(x) return S_FALSE;
#define DEBUG_ASSERT(x)
#define DEBUG_FAIL_MESSAGE(x)

#endif

// Thread_local is only available in C++11, so if we're compiling under VS 2012 we need
// to define it.
#if _MSC_VER == 1700
#define thread_local __declspec(thread)
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// See http://stackoverflow.com/a/8712996

#if defined(_MSC_VER) && _MSC_VER < 1900

#define snprintf c99_snprintf
#define vsnprintf c99_vsnprintf

inline int c99_vsnprintf(char* outBuf, size_t size, const char* format, va_list ap)
{
    int count = -1;

    if (size != 0)
        count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
    if (count == -1)
        count = _vscprintf(format, ap);

    return count;
}

inline int c99_snprintf(char* outBuf, size_t size, const char* format, ...)
{
    int count;
    va_list ap;

    va_start(ap, format);
    count = c99_vsnprintf(outBuf, size, format, ap);
    va_end(ap);

    return count;
}

#endif

// End code borrowed from StackOverflow
///////////////////////////////////////////////////////////////////////////////////////////////////

inline void DebugLog(wchar_t const* const Format, ...)
{
#ifdef ENABLE_DEBUG_LOG
    static int const BufferSize = 65536;
    static wchar_t* WideBuffer = nullptr;

    if (!WideBuffer)
    {
        WideBuffer = new wchar_t[BufferSize];
    }

    va_list Args;
    va_start(Args, Format);
    _vsnwprintf_s(WideBuffer, BufferSize, BufferSize - 1, Format, Args);
    va_end(Args);

    OutputDebugString(WideBuffer);
#endif
}

inline void ReleaseLog(wchar_t const* const Format, ...)
{
#ifdef ENABLE_RELEASE_LOG
    static int const BufferSize = 65536;
    static wchar_t* WideBuffer = nullptr;

    if (!WideBuffer)
    {
        WideBuffer = new wchar_t[BufferSize];
    }

    va_list Args;
    va_start(Args, Format);
    //_vsnwprintf(WideBuffer, BufferSize, Format, Args);
    _vsnwprintf_s(WideBuffer, BufferSize, BufferSize, Format, Args);
    va_end(Args);

    OutputDebugString(WideBuffer);
#endif
}
