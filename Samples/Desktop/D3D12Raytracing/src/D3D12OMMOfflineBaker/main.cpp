#include <Windows.h>
#include <string>
#include <chrono>
#include <vector>

#include <dxgi.h>
#include "DDS.h"
#include "fast_obj.h"
#include <J:\\Opacity-MicroMap-SDK\\libraries\\omm-lib\\include\\omm.hpp>

#pragma comment(lib, "J:\\Opacity-MicroMap-SDK\\build\\libraries\\omm-lib\\Debug\\omm-lib.lib")

void OMM_ABORT_ON_ERROR(omm::Result res)
{
    if (res != omm::Result::SUCCESS)
    {
        __debugbreak();
        exit(1);
    }
}

static void Log(omm::MessageSeverity severity, const char* message, void* userArg)
{
    printf("%s\n", message);
}

char* LoadDDSFile(const char* filename, omm::Cpu::TextureDesc& texDesc, omm::Cpu::TextureMipDesc mipDescs[16])
{
    // Load the DDS file
    HANDLE fh = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fh == INVALID_HANDLE_VALUE)
    {
        printf("Failed to open file %s\n", filename);
        exit(1);
    }

    DWORD fileSize = GetFileSize(fh, NULL);

    char* fileData = new char[fileSize];
    DWORD bytesRead;
    ReadFile(fh, fileData, fileSize, &bytesRead, NULL);
    CloseHandle(fh);

    FOURCC* fourcc = (FOURCC*)fileData;

    // Check if the file is a DDS file
    if (*fourcc != DirectX::DDS_MAGIC)
    {
        printf("File %s is not a DDS file\n", filename);
        exit(1);
    }

    DirectX::DDS_HEADER* header = (DirectX::DDS_HEADER*)(fileData + sizeof(FOURCC));

    mipDescs[0].width = header->width;
    mipDescs[0].height = header->height;
    mipDescs[0].rowPitch = header->width;
    mipDescs[0].textureData = fileData + sizeof(FOURCC) + header->size;
    
    texDesc.alphaCutoff = 0.5f;
    texDesc.format = omm::Cpu::TextureFormat::UNORM8;
    texDesc.mipCount = 1;
    texDesc.mips = mipDescs;

    return fileData;
}

fastObjMesh* LoadOBJFile(const char* filename, omm::Cpu::BakeInputDesc& input, std::vector<unsigned int>& texCoordIndices)
{
    // Load the OBJ file
    fastObjMesh* obj = fast_obj_read(filename);

    // We only want to build OMMs for the leaves, not the trunk or branches as these are opaque.
    // Treat each object as a 'geometry' in DXR terms
    int numObjects = obj->object_count;
        
    for (int i = 0; i < numObjects; i++)
    {
        fastObjGroup& object = obj->objects[i];

        // Push all the vertex positions, normals and texture coordinates into the input data
        bool isLeaves = strstr(object.name, "leaves");

        if (isLeaves)
        {
            unsigned int numFaces = object.face_count;
            unsigned int faceOffset = object.face_offset;
            unsigned int indexOffset = object.index_offset;

            unsigned int* faceIndices = obj->face_vertices + faceOffset;

            unsigned int numVerticesUsed = 0;

            for (int face = 0; face < numFaces; face++)
            {
                int numVerticesInFace = obj->face_vertices[faceOffset + face];
                int numTrianglesInFace = numVerticesInFace - 2;

                for (int tri = 0; tri < numTrianglesInFace; tri++)
                {
                    texCoordIndices.push_back(obj->indices[indexOffset + numVerticesUsed + 0].t);
                    texCoordIndices.push_back(obj->indices[indexOffset + numVerticesUsed + tri + 1].t);
                    texCoordIndices.push_back(obj->indices[indexOffset + numVerticesUsed + tri + 2].t);
                }

                numVerticesUsed += numVerticesInFace;
            }
        }
    }

    input.indexBuffer = texCoordIndices.data();
    input.indexFormat = omm::IndexFormat::UINT_32;
    input.indexCount = texCoordIndices.size();

    int numTexCoords = obj->texcoord_count;

    input.texCoords = new float[numTexCoords * 2];
    input.texCoordFormat = omm::TexCoordFormat::UV32_FLOAT;
    input.texCoordStrideInBytes = 8;

    uint64_t copySize = numTexCoords * 2 * sizeof(float);
    memcpy_s((void*)input.texCoords, copySize, (void*)obj->texcoords, copySize);

    return obj;
}

int main()
{
    omm::BakerCreationDesc desc;
    desc.type = omm::BakerType::CPU;
    desc.messageInterface.messageCallback = &Log;

    omm::Cpu::BakeInputDesc input;
    omm::Cpu::BakeResult result;
    const omm::Cpu::BakeResultDesc* resultDesc;
    omm::Debug::Stats stats;

    omm::Baker baker;
    OMM_ABORT_ON_ERROR(omm::CreateBaker(desc, &baker));

    // Create a texture
    omm::Cpu::TextureMipDesc mipDescs[16];
    omm::Cpu::TextureDesc texDesc;
    char* textureData = LoadDDSFile("jacaranda_tree_leaves_alpha_4k.dds", texDesc, mipDescs);

    OMM_ABORT_ON_ERROR(omm::Cpu::CreateTexture(baker, texDesc, &input.texture));    

    // Load the model
    std::vector<unsigned int> indices;
    fastObjMesh* obj = LoadOBJFile("tree_merged.obj", input, indices);

    // Configuration
    input.maxSubdivisionLevel = 4;
    input.subdivisionLevels = nullptr;  // Use maxSubdivisionLevel globally
    input.runtimeSamplerDesc.addressingMode = omm::TextureAddressMode::Clamp;
    input.runtimeSamplerDesc.filter = omm::TextureFilterMode::Linear;
    input.runtimeSamplerDesc.borderAlpha = 0.0f;
    input.bakeFlags = omm::Cpu::BakeFlags::EnableInternalThreads | omm::Cpu::BakeFlags::EnableValidation;
    input.format = omm::Format::OC1_2_State;
    input.alphaMode = omm::AlphaMode::Test;

    // Time the bake using std::chrono
    auto start = std::chrono::high_resolution_clock::now();

    omm::Result res = omm::Cpu::Bake(baker, input, &result);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    // Print the time to the console
    printf("Bake time: %f seconds\n", elapsed.count());

    if (res == omm::Result::SUCCESS)
    {
        OMM_ABORT_ON_ERROR(omm::Cpu::GetBakeResultDesc(result, &resultDesc));

        OMM_ABORT_ON_ERROR(omm::Debug::GetStats2(baker, result, &stats));

        // Serialize the OMMs to disk
        {
            HANDLE fh = CreateFileA("treeOMM.bin", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

            UINT data[5] = { resultDesc->arrayDataSize, resultDesc->descArrayCount, resultDesc->descArrayHistogramCount, (UINT)resultDesc->indexFormat, resultDesc->indexCount };
            WriteFile(fh, data, sizeof(data), nullptr, nullptr);

            WriteFile(fh, resultDesc->descArray, sizeof(resultDesc->descArray[0]) * resultDesc->descArrayCount, nullptr, nullptr);
            WriteFile(fh, resultDesc->arrayData, resultDesc->arrayDataSize, nullptr, nullptr);

            for (int i = 0; i < resultDesc->descArrayHistogramCount; i++)
            {
                UINT Count = resultDesc->descArrayHistogram[i].count;
                WriteFile(fh, &Count, sizeof(Count), nullptr, nullptr);

                UINT SubdivisionLevel = resultDesc->descArrayHistogram[i].subdivisionLevel;
                WriteFile(fh, &SubdivisionLevel, sizeof(SubdivisionLevel), nullptr, nullptr);

                UINT Format = resultDesc->descArrayHistogram[i].format;
                WriteFile(fh, &Format, sizeof(Format), nullptr, nullptr);
            }

            //WriteFile(fh, resultDesc->descArrayHistogram, sizeof(resultDesc->descArrayHistogram[0]) * resultDesc->descArrayHistogramCount, nullptr, nullptr);
            WriteFile(fh, resultDesc->indexBuffer, resultDesc->indexCount * (resultDesc->indexFormat == omm::IndexFormat::UINT_32 ? 4 : 2), nullptr, nullptr);

            CloseHandle(fh);
        }

        // Simple custom file format for the model itself
        // The format essentially mirrors an OBJ file - meaning separate indices for positions, normals and texture coordinates.
        // Not something you would use in a real application, but useful for keeping this sample simple.
        {
            HANDLE fh = CreateFileA("treeModel.bin", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

            // Write the core vertex data
            UINT counts[3] = { obj->position_count, obj->normal_count, obj->texcoord_count };
            WriteFile(fh, counts, sizeof(counts), nullptr, nullptr);

            WriteFile(fh, obj->positions, obj->position_count * 3 * sizeof(float), nullptr, nullptr);
            WriteFile(fh, obj->normals, obj->normal_count * 3 * sizeof(float), nullptr, nullptr);
            WriteFile(fh, obj->texcoords, obj->texcoord_count * 2 * sizeof(float), nullptr, nullptr);

            UINT numGeoms = obj->object_count;
            WriteFile(fh, &numGeoms, sizeof(numGeoms), nullptr, nullptr);

            std::vector<unsigned int> positionIndices;
            std::vector<unsigned int> normalIndices;
            std::vector<unsigned int> texCoordIndices;

            // We need to triangulate any faces that are 4+ sided, so a bit of extra work here
            for (int i = 0; i < numGeoms; i++)
            {
                UINT indexCount = 0;

                fastObjGroup& object = obj->objects[i];

                unsigned int numFaces = object.face_count;
                unsigned int faceOffset = object.face_offset;
                unsigned int indexOffset = object.index_offset;

                unsigned int* faceIndices = obj->face_vertices + faceOffset;

                unsigned int numVerticesUsed = 0;

                for (int face = 0; face < numFaces; face++)
                {
                    int numVerticesInFace = obj->face_vertices[faceOffset + face];
                    int numTrianglesInFace = numVerticesInFace - 2;

                    for (int tri = 0; tri < numTrianglesInFace; tri++)
                    {
                        positionIndices.push_back(obj->indices[indexOffset + numVerticesUsed + 0].p);
                        positionIndices.push_back(obj->indices[indexOffset + numVerticesUsed + tri + 1].p);
                        positionIndices.push_back(obj->indices[indexOffset + numVerticesUsed + tri + 2].p);

                        normalIndices.push_back(obj->indices[indexOffset + numVerticesUsed + 0].n);
                        normalIndices.push_back(obj->indices[indexOffset + numVerticesUsed + tri + 1].n);
                        normalIndices.push_back(obj->indices[indexOffset + numVerticesUsed + tri + 2].n);

                        texCoordIndices.push_back(obj->indices[indexOffset + numVerticesUsed + 0].t);
                        texCoordIndices.push_back(obj->indices[indexOffset + numVerticesUsed + tri + 1].t);
                        texCoordIndices.push_back(obj->indices[indexOffset + numVerticesUsed + tri + 2].t);
                    }

                    indexCount += numTrianglesInFace * 3;
                    numVerticesUsed += numVerticesInFace;
                }

                WriteFile(fh, &indexCount, sizeof(indexCount), nullptr, nullptr);
            }

            // Write the indices
            UINT numIndices = positionIndices.size();
            WriteFile(fh, &numIndices, sizeof(numIndices), nullptr, nullptr);

            WriteFile(fh, positionIndices.data(), numIndices * sizeof(unsigned int), nullptr, nullptr);
            WriteFile(fh, normalIndices.data(), numIndices * sizeof(unsigned int), nullptr, nullptr);
            WriteFile(fh, texCoordIndices.data(), numIndices * sizeof(unsigned int), nullptr, nullptr);

            CloseHandle(fh);
        }
    }

    OMM_ABORT_ON_ERROR(omm::Cpu::DestroyTexture(baker, input.texture));
    fast_obj_destroy(obj);
    delete[] textureData;
}