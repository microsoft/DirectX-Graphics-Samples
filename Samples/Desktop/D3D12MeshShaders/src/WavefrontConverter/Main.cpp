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
#include "Export.h"


namespace
{
    void PrintHelp()
    {
        std::cout << std::endl;
        std::cout << "---------------------------- Meshlet Converter ----------------------------" << std::endl;
        std::cout << "This tool generates a meshlet structure from meshes the OBJ file format." << std::endl;
        std::cout << std::endl;

        std::cout << "Usage:" << std::endl;
        std::cout << "\t<string list> -- Specifies paths to the .obj file to process." << std::endl;
        std::cout << std::endl;

        std::cout << "Switches:" << std::endl;
        std::cout << "\t-h            -- Display this help message." << std::endl;
        std::cout << "\t-a <ids>      -- Specifies the vertex attribute layout of the exported meshes - more info below. Default is vnu" << std::endl;
        std::cout << "\t-v <int>      -- Specifies the maximum vertex count of a meshlet. Must be less than 256. Default is 64" << std::endl;
        std::cout << "\t-p <int>      -- Specifies the maximum primitive count of a meshlet. Must be less than 256. Default is 126" << std::endl;
        std::cout << "\t-s <float>    -- Specifies a global scaling factor for scene geometry. Default is 1.0" << std::endl;
        std::cout << "\t-i            -- Forces vertex indices to be 32 bits, even if only 16 bits are required. Default is false" << std::endl;
        std::cout << "\t-f            -- Flip primitive winding order. Default is false" << std::endl;
        std::cout << "\t-l <int>      -- Sets the log verbosity: 0 - Error, 1 - Basic, 2 - Verbose. Default is Basic" << std::endl;
        std::cout << std::endl;

        std::cout << "Vertex Attribute Layout IDs:" << std::endl;
        std::cout << "A non-spaced list of id characters determine how output vertices are laid out." << std::endl;
        std::cout << "\tp - Position" << std::endl;
        std::cout << "\tn - Normal" << std::endl;
        std::cout << "\tu - TexCoord" << std::endl;
        std::cout << "\tt - Tangent" << std::endl;
        std::cout << "\tb - Bitangent" << std::endl;
        std::cout << "\t: - Used to separate attributes into separate buffers." << std::endl;
        std::cout << std::endl;

        std::cout << "Example:" << std::endl;
        std::cout << "\tConverterApp.exe -a p:nutb -v 128 -p 128 -i Path/To/MyFile1.obj Path/To/MyFile2.obj " << std::endl;
        std::cout << std::endl;
    }

    int ParseCommandLine(uint32_t argc, const char* args[], std::vector<std::string>& files, ProcessOptions& options)
    {
        if (argc < 2)
        {
            PrintHelp();
            return 0;
        }

        for (uint32_t i = 1; i < argc; ++i)
        {
            if (std::strcmp(args[i], "-h") == 0)
            {
                PrintHelp();
                return 0;
            }
            else if (std::strcmp(args[i], "-v") == 0)
            {
                if (i + 1 == argc)
                {
                    std::cout << "Must provide an integral value for meshlet max vertex count if supplying -v switch." << std::endl;
                    return 1;
                }

                uint32_t maxSize = std::strtoul(args[++i], nullptr, 10);
                uint32_t adjSize = std::min(maxSize, 256u);

                if (maxSize != adjSize)
                {
                    std::cout << "Meshlet max vertex count must be less than 256." << std::endl;
                    std::cout << "Specified: " << maxSize << ", Adjusted: " << adjSize << std::endl;

                    maxSize = adjSize;
                }

                options.MeshletMaxVerts = maxSize;
            }
            else if (std::strcmp(args[i], "-p") == 0)
            {
                if (i + 1 == argc)
                {
                    std::cout << "Must provide an integral value for meshlet max primitive count if supplying -p switch." << std::endl;
                    return 1;
                }

                uint32_t maxSize = std::strtoul(args[++i], nullptr, 10);
                uint32_t adjSize = std::min(maxSize, 256u);

                if (maxSize != adjSize)
                {
                    std::cout << "Meshlet max primitive count must be less than 256." << std::endl;
                    std::cout << "Specified: " << maxSize << ", Adjusted: " << adjSize << std::endl;

                    maxSize = adjSize;
                }

                options.MeshletMaxPrims = maxSize;
            }
            else if (std::strcmp(args[i], "-s") == 0)
            {
                if (i + 1 == argc)
                {
                    std::cout << "Must provide a float value for scaling factor if supplying -s switch." << std::endl;
                    return 1;
                }

                float scale = (float)strtod(args[++i], nullptr);
                if (scale == 0.0f)
                    scale = 1.0f;

                options.UnitScale = scale;
            }
            else if (std::strcmp(args[i], "-a") == 0)
            {
                if (i + 1 == argc)
                {
                    std::cout << "Must provide a string specifying attribute layout if supplying -a switch." << std::endl;
                    return 1;
                }

                AttrStream stream;

                const char* curr = args[i + 1];
                while (*curr != '\0')
                {
                    switch (*curr)
                    {
                    case 'p':
                        stream.push_back(Attribute::Position);
                        break;

                    case 'n':
                        stream.push_back(Attribute::Normal);
                        break;

                    case 'u':
                        stream.push_back(Attribute::TexCoord);
                        break;

                    case 't':
                        stream.push_back(Attribute::Tangent);
                        break;

                    case 'b':
                        stream.push_back(Attribute::Bitangent);
                        break;

                    case ':':
                        options.ExportAttributes.emplace_back(std::move(stream));
                        break;
                    }

                    ++curr;
                }
                ++i;

                if (!stream.empty())
                    options.ExportAttributes.emplace_back(std::move(stream));
            }
            else if (std::strcmp(args[i], "-i") == 0)
            {
                std::cout << "Forcing vertex indices to 32 bits." << std::endl;
                options.Force32BitIndices = true;
            }
            else if (std::strcmp(args[i], "-f") == 0)
            {
                std::cout << "Flipping winding order." << std::endl;
                options.Flip = true;
            }
            else if (std::strcmp(args[i], "-l") == 0)
            {
                if (i + 1 == argc)
                {
                    std::cout << "Must provide an integer specify log verbosity if supplying -l switch." << std::endl;
                    return 1;
                }

                uint32_t level = std::strtoul(args[++i], nullptr, 10);
                uint32_t adjSize = std::min(level, 2u);

                options.LogLevel = (ProcessOptions::ELogVerbosity)adjSize;
            }
            else
            {
                files.push_back(args[i]);
            }
        }

        if (options.ExportAttributes.empty())
        {
            AttrStream stream;
            stream.push_back(Attribute::Position);
            stream.push_back(Attribute::Normal);
            stream.push_back(Attribute::TexCoord);
            stream.push_back(Attribute::Tangent);
            stream.push_back(Attribute::Bitangent);

            options.ExportAttributes.emplace_back(std::move(stream));
        }


        static const char* s_verbosityNames[] =
        {
            "None",
            "Basic",
            "Verbose"
        };

        static const char* s_semanticNames[] =
        {
            "Position",
            "Normal",
            "TexCoord",
            "Tangent",
            "Bitangent"
        };

        std::cout << "Log verbosity set to " << s_verbosityNames[static_cast<uint32_t>(options.LogLevel)] << std::endl;
        std::cout << "Using global scale factor - " << options.UnitScale << std::endl;
        std::cout << "Using meshlet size - Vertices: " << options.MeshletMaxVerts << "   Primitives: " << options.MeshletMaxPrims << std::endl;

        if (options.Force32BitIndices)
        {
            std::cout << "Forcing indices to 32 bits" << std::endl;
        }
        
        std::cout << "Exporting vertex buffers as: " << std::endl;
        for (uint32_t i = 0; i < options.ExportAttributes.size(); ++i)
        {
            std::cout << "\tStream " << i << ": ";

            for (auto& attr : options.ExportAttributes[i])
            {
                std::cout << s_semanticNames[attr] << " | ";
            }

            std::cout << std::endl;
        }

        std::cout << "Specified input files: " << std::endl;
        for (auto& f : files)
        {
            std::cout << "\t" << f << std::endl;
        }

        return 0;
    }
}

int _cdecl main(int argc, const char* args[])
{
    std::vector<std::string> files;

    // Parse command line options
    ProcessOptions options;
    int ret = ParseCommandLine(argc, args, files, options);
    if (ret != 0)
    {
        return ret;
    }

    // Try loading all files specified on command line
    bool success = true;
    for (auto& filename : files)
    {
        std::vector<ExportMesh> meshes;

        // Try import
        if (!ImportFile(filename.c_str(), options, meshes))
        {
            success = false;
            continue;
        }

        // Generate output filename
        auto extLoc = filename.find_last_of(".");
        auto path = filename.substr(0, extLoc) + ".bin";

        // Try export
        if (!ExportMeshes(path.c_str(), meshes))
        {
            success = false;
        }
    }

    // Return success if all files were successfully loaded & exported.
    return success ? 0 : 1;
}
