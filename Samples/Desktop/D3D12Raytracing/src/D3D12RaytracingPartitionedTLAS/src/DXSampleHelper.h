//*********************************************************
//
// Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include <windows.h>
#include <cstdio>
#include <stdexcept>

// Retain the HRESULT so the application can distinguish device loss from
// programming errors. Never turn an unrelated API failure into a retry.
class HrException : public std::runtime_error
{
public:
    HrException(HRESULT result, const char* message) : std::runtime_error(message), m_result(result) {}
    HRESULT Error() const { return m_result; }

private:
    HRESULT m_result;
};

// Include the failing API and HRESULT in diagnostics, including Release builds.
inline void ThrowIfFailed(HRESULT result, const char* operation)
{
    if (FAILED(result))
    {
        char message[256];
        sprintf_s(message, "%s failed (HRESULT 0x%08lX)", operation, static_cast<unsigned long>(result));
        throw HrException(result, message);
    }
}
