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
// Author(s):	Alex Nankervis
//

#include "ModelAssimp.h"

#include <stdio.h>
#include <iostream>

void PrintHelp()
{
    printf("model_convert\n");

    printf("usage:\n");
    printf("model_convert input_file output_file\n");
}

void AssimpModel::PrintModelStats()
{
    printf("model stats:\n");
    
    Math::AxisAlignedBox bbox = GetBoundingBox();
    printf("bounding box: <%f, %f, %f> <%f, %f, %f>\n",
        (float)bbox.GetMin().GetX(), (float)bbox.GetMin().GetY(), (float)bbox.GetMin().GetZ(),
        (float)bbox.GetMax().GetX(), (float)bbox.GetMax().GetY(), (float)bbox.GetMax().GetZ());

    printf("vertex data size: %u\n", m_Header.vertexDataByteSize);
    printf("index data size: %u\n", m_Header.indexDataByteSize);
    printf("vertex data size depth-only: %u\n", m_Header.vertexDataByteSizeDepth);
    printf("\n");

    printf("mesh count: %u\n", m_Header.meshCount);
    for (uint32_t meshIndex = 0; meshIndex < m_Header.meshCount; meshIndex++)
    {
        const Mesh* mesh = m_pMesh + meshIndex;

        auto printAttribFormat = [](uint32_t format) -> void
        {
            switch (format)
            {
            case attrib_format_ubyte:   printf("ubyte");    break;
            case attrib_format_byte:    printf("byte");     break;
            case attrib_format_ushort:  printf("ushort");   break;
            case attrib_format_short:   printf("short");    break;
            case attrib_format_float:   printf("float");    break;
            }
        };

        printf("mesh %u\n", meshIndex);
        printf("vertices: %u\n", mesh->vertexCount);
        printf("indices: %u\n", mesh->indexCount);
        printf("vertex stride: %u\n", mesh->vertexStride);
        for (int n = 0; n < maxAttribs; n++)
        {
            if (mesh->attrib[n].format == attrib_format_none)
                continue;

            printf("attrib %d: offset %u, normalized %u, components %u, format "
                , n, mesh->attrib[n].offset, mesh->attrib[n].normalized
                , mesh->attrib[n].components);
            printAttribFormat(mesh->attrib[n].format);
            printf("\n");
        }

        printf("vertices depth-only: %u\n", mesh->vertexCountDepth);
        printf("vertex stride depth-only: %u\n", mesh->vertexStrideDepth);
        for (int n = 0; n < maxAttribs; n++)
        {
            if (mesh->attribDepth[n].format == attrib_format_none)
                continue;

            printf("attrib %d: offset %u, normalized %u, components %u, format "
                , n, mesh->attribDepth[n].offset, mesh->attribDepth[n].normalized
                , mesh->attribDepth[n].components);
            printAttribFormat(mesh->attrib[n].format);
            printf("\n");
        }
    }
    printf("\n");

    printf("material count: %u\n", m_Header.materialCount);
    for (uint32_t materialIndex = 0; materialIndex < m_Header.materialCount; materialIndex++)
    {
        //const Material* material = m_pMaterial + materialIndex;
        printf("material %u\n", materialIndex);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        PrintHelp();
        return -1;
    }

    const char *input_file = argv[1];
    const char *output_file = argv[2];

    printf("input file %s\n", input_file);
    printf("output file %s\n", output_file);

	AssimpModel model;

    printf("loading...\n");
    if (!model.Load(input_file))
    {
        printf("failed to load model: %s\n", input_file);
        return -1;
    }

    printf("saving...\n");
    if (!model.Save(output_file))
    {
        printf("failed to save model: %s\n", output_file);
        return -1;
    }

    printf("done\n");

    model.PrintModelStats();

    std::cout << "Press enter...";
    std::string trash;
    std::getline(std::cin, trash);

    return 0;
}
