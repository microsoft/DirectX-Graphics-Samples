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
// Author:  Jack Elliott
//

#pragma once

namespace CommandLineArgs
{
    void Initialize(int argc, wchar_t** argv);
    bool GetInteger(const wchar_t* key, uint32_t& value);
    bool GetFloat(const wchar_t* key, float& value);
    bool GetString(const wchar_t* key, std::wstring& value);
}