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
#include "stdafx.h"
#include "Import.h"

#include "WaveFrontReader.h"

#include <algorithm>
#include <iostream>

using namespace DirectX;

bool ImportFile(const char* filename, const ProcessOptions& options, std::vector<ExportMesh>& meshes)
{
    if (!filename)
    {
        std::cout << "No filename supplied." << std::endl;
        return false;
    }

    if (options.ExportAttributes.empty())
    {
        std::cout << "No export attributes specified - stopping import." << std::endl;
        return false;
    }

    if (std::any_of(options.ExportAttributes.begin(), options.ExportAttributes.end(), [](auto& stream) { return stream.empty(); }))
        return false;

    bool positionFound = false;
    for (uint32_t i = 0; i < options.ExportAttributes.size(); ++i)
    {
        if (options.ExportAttributes[i].empty())
            return false;

        for (uint32_t j = 0; j < options.ExportAttributes[i].size(); ++j)
        {
            if (options.ExportAttributes[i][j] == Attribute::Position)
            {
                positionFound = true;
                break;
            }
        }
    }

    if (!positionFound)
    {
        std::cout << "Failed to find the required position attribute in the output stream." << std::endl;
        return false;
    }

    if (options.LogLevel >= ProcessOptions::Basic)
    {
        std::cout << std::endl;
        std::cout << "Parsing file: " << filename << std::endl;
    }

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring filenameW = converter.from_bytes(filename);

    WaveFrontReader<uint32_t> reader;
    HRESULT hr = reader.Load(filenameW.c_str(), options.Flip);

    // WaveFrontReader::Load() also returns a failing HRESULT if no material file was found - but we only care about mesh data.
    if (FAILED(hr) && reader.vertices.size() == 0)
    {
        std::cout << "Failed to load file: " << filename << std::endl;

        return false;
    }
    
    MeshProcessor processor;
    if (options.LogLevel >= ProcessOptions::Verbose)
    {
        std::cout << std::endl;
        std::cout << "Mesh " << reader.name.c_str() << std::endl;
    }

    meshes.resize(1);
    processor.Process(options, reader, meshes[0]);
    
    return !meshes.empty();
}
