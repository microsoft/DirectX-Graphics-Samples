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

#include "DXSampleHelper.h"
#include "ShaderCompiler.h"
#include <windows.h>
#include <unknwn.h>
#include <objidl.h>
#include <oleauto.h>
#include <dxcapi.h>
#include <wrl/client.h>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace
{
struct Shader
{
    const wchar_t* source;
    const wchar_t* target;
    const wchar_t* entry;
    const wchar_t* output;
    std::vector<char> ShaderBytecodes::*bytecode;
    bool preciseMath;
};

constexpr Shader kShaders[] = {
    {L"raytracing.hlsl", L"lib_6_6", nullptr, L"raytracing.dxil", &ShaderBytecodes::raytracing, false},
    {L"animation.hlsl", L"cs_6_6", L"Animate", L"animation.dxil", &ShaderBytecodes::animation, false},
    {L"animation.hlsl", L"cs_6_6", L"Simulate", L"physics.dxil", &ShaderBytecodes::physics, true},
    {L"animation.hlsl", L"cs_6_6", L"MarkPartitions", L"mark_partitions.dxil", &ShaderBytecodes::markPartitions, false},
    {L"compositing.hlsl", L"cs_6_6", L"Composite", L"compositing.dxil", &ShaderBytecodes::compositing, false}};

std::filesystem::path GetShaderSourceDirectory(const std::filesystem::path& binaryDirectory)
{
    // Find project sources from bin/x64/Debug or Release (or another output
    // directory inside this project). Standalone copies use their bundled HLSL.
    for (auto directory = binaryDirectory; directory != directory.root_path(); directory = directory.parent_path())
        if (std::filesystem::exists(directory / "D3D12RaytracingPartitionedTLAS.vcxproj"))
            return directory / "shaders";
    return binaryDirectory / "shaders";
}

} // namespace

ShaderBytecodes LoadSampleShaders(const std::filesystem::path& binaryDirectory)
{
    ShaderBytecodes shaders;
    for (const Shader& shader : kShaders)
    {
        std::ifstream file(binaryDirectory / shader.output, std::ios::binary | std::ios::ate);
        if (!file || file.tellg() <= 0)
            throw std::runtime_error("Could not open compiled shader");
        auto& bytes = shaders.*shader.bytecode;
        bytes.resize(static_cast<size_t>(file.tellg()));
        file.seekg(0);
        if (!file.read(bytes.data(), static_cast<std::streamsize>(bytes.size())))
            throw std::runtime_error("Could not read compiled shader");
    }
    return shaders;
}

ShaderBytecodes CompileSampleShaders(const std::filesystem::path& binaryDirectory)
{
    const auto shaderDirectory = GetShaderSourceDirectory(binaryDirectory);
    if (!std::filesystem::exists(shaderDirectory / "raytracing.hlsl"))
        throw std::runtime_error("Shader sources are missing");

    struct Library
    {
        HMODULE handle{};
        ~Library()
        {
            if (handle)
                FreeLibrary(handle);
        }
    } library{LoadLibraryW((binaryDirectory / "dxcompiler.dll").c_str())};
    if (!library.handle)
        throw std::runtime_error("Could not load dxcompiler.dll");
    const auto createInstance =
        reinterpret_cast<DxcCreateInstanceProc>(GetProcAddress(library.handle, "DxcCreateInstance"));
    if (!createInstance)
        throw std::runtime_error("DxcCreateInstance is missing");

    ComPtr<IDxcUtils> utils;
    ComPtr<IDxcCompiler3> compiler;
    ComPtr<IDxcIncludeHandler> includeHandler;
    ThrowIfFailed(createInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)), "Create DXC utils");
    ThrowIfFailed(createInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)), "Create DXC compiler");
    ThrowIfFailed(utils->CreateDefaultIncludeHandler(&includeHandler), "Create DXC include handler");

    ShaderBytecodes shaders;
    for (const Shader& shader : kShaders)
    {
        const auto sourcePath = shaderDirectory / shader.source;
        ComPtr<IDxcBlobEncoding> source;
        ThrowIfFailed(utils->LoadFile(sourcePath.c_str(), nullptr, &source), "Load HLSL source");
        const DxcBuffer sourceBuffer{source->GetBufferPointer(), source->GetBufferSize(), DXC_CP_UTF8};
        std::vector<LPCWSTR> arguments{L"-T", shader.target, L"-HV", L"2021", L"-I", shaderDirectory.c_str()};
        if (shader.entry)
        {
            arguments.push_back(L"-E");
            arguments.push_back(shader.entry);
        }
        if (shader.preciseMath)
            arguments.push_back(L"-Gis");
        ComPtr<IDxcResult> result;
        ThrowIfFailed(compiler->Compile(&sourceBuffer, arguments.data(), static_cast<UINT32>(arguments.size()),
                                        includeHandler.Get(), IID_PPV_ARGS(&result)),
                      "Compile HLSL");
        HRESULT status{};
        ThrowIfFailed(result->GetStatus(&status), "Get DXC status");
        if (FAILED(status))
        {
            ComPtr<IDxcBlobUtf8> errors;
            result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
            const std::string message = errors && errors->GetStringLength()
                                            ? std::string(errors->GetStringPointer(), errors->GetStringLength())
                                            : "DXC shader compilation failed";
            throw std::runtime_error(message);
        }
        ComPtr<IDxcBlob> compiled;
        ThrowIfFailed(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&compiled), nullptr), "Get compiled shader");
        const auto* begin = static_cast<const char*>(compiled->GetBufferPointer());
        (shaders.*shader.bytecode).assign(begin, begin + compiled->GetBufferSize());
    }
    return shaders;
}
