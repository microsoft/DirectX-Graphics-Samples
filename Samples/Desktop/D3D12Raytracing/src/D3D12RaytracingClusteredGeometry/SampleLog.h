//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT License (MIT).
//
//*********************************************************
//
// SampleLog.h - tiny diagnostic logger used by this sample.
//
// Writes to OutputDebugString (visible in DebugView / Visual Studio output) AND
// appends to a log file at %TEMP%\D3D12RaytracingClusteredGeometry.log. The file
// path is overridable via the SAMPLE_LOG env var. The file is truncated on first
// use within a process.
//
// Existing call sites that used OutputDebugStringW continue to work; LogF() is a
// printf-style convenience that does both.
//
#pragma once

#include <cstdio>
#include <cstdarg>
#include <string>
#include <mutex>
#include <windows.h>

namespace SampleLog
{
    inline std::wstring GetPath()
    {
        WCHAR env[MAX_PATH] = {};
        DWORD n = GetEnvironmentVariableW(L"SAMPLE_LOG", env, MAX_PATH);
        if (n > 0 && n < MAX_PATH) return env;
        WCHAR tmp[MAX_PATH] = {};
        GetTempPathW(MAX_PATH, tmp);
        std::wstring p = tmp;
        if (p.empty() || p.back() != L'\\') p += L'\\';
        p += L"D3D12RaytracingClusteredGeometry.log";
        return p;
    }

    inline void Init()
    {
        // Truncate the log file on first call (called from WinMain).
        FILE* f = nullptr;
        if (_wfopen_s(&f, GetPath().c_str(), L"wb") == 0 && f)
        {
            // BOM for UTF-16 LE - most tools handle this fine. We then fall back
            // to wide writes via fputws.
            unsigned char bom[2] = { 0xFF, 0xFE };
            fwrite(bom, 1, 2, f);
            fclose(f);
        }
    }

    inline void Write(const wchar_t* msg)
    {
        OutputDebugStringW(msg);
        static std::mutex m;
        std::lock_guard<std::mutex> lk(m);
        FILE* f = nullptr;
        if (_wfopen_s(&f, GetPath().c_str(), L"ab") == 0 && f)
        {
            fputws(msg, f);
            fclose(f);
        }
    }

    inline void LogF(const wchar_t* fmt, ...)
    {
        wchar_t buf[2048];
        va_list a;
        va_start(a, fmt);
        _vsnwprintf_s(buf, _TRUNCATE, fmt, a);
        va_end(a);
        Write(buf);
    }
}
