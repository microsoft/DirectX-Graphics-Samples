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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <DirectXMath.h>
#include <Windows.h>

#undef max
#undef min

#include <algorithm>
#include <cassert>
#include <codecvt>
#include <iostream>
#include <locale>
#include <memory>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <vector>

    // Helper class for COM exceptions
class com_exception : public std::exception
{
public:
    com_exception(HRESULT hr) : result(hr) {}

    virtual const char* what() const override
    {
        static char s_str[64] = {};
        sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
        return s_str;
    }

private:
    HRESULT result;
};

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
#ifdef _DEBUG
        char str[128] = {};
        sprintf_s(str, "**ERROR** Fatal Error with HRESULT of %08X\n", static_cast<unsigned int>(hr));
        OutputDebugStringA(str);
        __debugbreak();
#endif
        throw com_exception(hr);
    }
}
