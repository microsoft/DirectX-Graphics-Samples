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
#include <filesystem>
#include <vector>

// CPU-owned bytecode survives device loss. Reload never changes the build's
// DXIL files; a replacement set is accepted only after pipeline creation succeeds.
struct ShaderBytecodes
{
    std::vector<char> raytracing;
    std::vector<char> animation;
    std::vector<char> physics;
    std::vector<char> markPartitions;
    std::vector<char> compositing;
};

ShaderBytecodes LoadSampleShaders(const std::filesystem::path& executableDirectory);
ShaderBytecodes CompileSampleShaders(const std::filesystem::path& executableDirectory);
