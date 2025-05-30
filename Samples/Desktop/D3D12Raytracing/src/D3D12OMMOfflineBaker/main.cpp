#include <Windows.h>
#include <string>
#include <chrono>
#include <vector>

#include <dxgi.h>
#include "DDS.h"
#include "fast_obj.h"

// The NVIDIA OMM SDK needs to be installed / pulled from Github and the
// Include and Library paths for this project must be setup correctly
// in order for the compiler/linker to build this project.
// 
// We're choosing not to include the OMM SDK in this repository as a submodule as
// the code in this project is not intended to demonstrate the use of NVIDIA's OMM SDK or best practices.
// 
// However, the D3D12OMMOfflineBaker project is required to rebuild the OMMs themselves incase we need
// to in the future and we don't want this code to be lost. 
// The OMMs have been built and shipped as part of the D3D12RaytracingOpacityMicromaps
// project and therefore running this project is NOT a prerequisite to run the D3D12RaytracingOpacityMicromaps sample.
#include <omm.hpp>
#pragma comment(lib, "omm-lib.lib")

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

    UINT s = sizeof(DirectX::DDS_HEADER);
    
    texDesc.alphaCutoff = 0.5f;
    texDesc.format = omm::Cpu::TextureFormat::UNORM8;
    texDesc.mipCount = 1;
    texDesc.mips = mipDescs;

    return fileData;
}

struct GeometryData
{
    bool isAlphaTested;
    std::vector<unsigned int> positionIndices;
    std::vector<unsigned int> normalIndices;
    std::vector<unsigned int> texCoordIndices;
};

fastObjMesh* LoadOBJFile(const char* filename, std::vector<GeometryData>& geometryData)
{
    // Load the OBJ file
    fastObjMesh* obj = fast_obj_read(filename);

	// Flip the V coordinate of the texture coordinates
	for (int i = 0; i < obj->texcoord_count; i++)
	{
		obj->texcoords[i * 2 + 1] = 1.0f - obj->texcoords[i * 2 + 1];
	}

    // We only want to build OMMs for the leaves, not the trunk or branches as these are opaque.
    // Treat each object as a 'geometry' in DXR terms
    int numObjects = obj->object_count;

    for (int i = 0; i < obj->material_count; i++)
    {
        GeometryData data;
        data.isAlphaTested = false;
        char* materialName = obj->materials[i].name;

        if (strstr(materialName, "leaves"))
        {
            data.isAlphaTested = true;
        }

        geometryData.push_back(data);
    }

    for (int i = 0; i < numObjects; i++)
    {
        fastObjGroup& object = obj->objects[i];

        unsigned int numFaces = object.face_count;
        unsigned int faceOffset = object.face_offset;
        unsigned int indexOffset = object.index_offset;

        unsigned int* faceIndices = obj->face_vertices + faceOffset;
        unsigned int* faceMaterialIndices = obj->face_materials + faceOffset;

        unsigned int numVerticesUsed = 0;

        for (int face = 0; face < numFaces; face++)
        {
            int numVerticesInFace = obj->face_vertices[faceOffset + face];
            int materialIndex = obj->face_materials[faceOffset + face];

            GeometryData& data = geometryData[materialIndex];
            bool isAlphaTested = data.isAlphaTested;

            int numTrianglesInFace = numVerticesInFace - 2;

            for (int tri = 0; tri < numTrianglesInFace; tri++)
            {
                data.positionIndices.push_back(obj->indices[indexOffset + numVerticesUsed + 0].p);
                data.positionIndices.push_back(obj->indices[indexOffset + numVerticesUsed + tri + 1].p);
                data.positionIndices.push_back(obj->indices[indexOffset + numVerticesUsed + tri + 2].p);

                data.normalIndices.push_back(obj->indices[indexOffset + numVerticesUsed + 0].n);
                data.normalIndices.push_back(obj->indices[indexOffset + numVerticesUsed + tri + 1].n);
                data.normalIndices.push_back(obj->indices[indexOffset + numVerticesUsed + tri + 2].n);

                data.texCoordIndices.push_back(obj->indices[indexOffset + numVerticesUsed + 0].t);
                data.texCoordIndices.push_back(obj->indices[indexOffset + numVerticesUsed + tri + 1].t);
                data.texCoordIndices.push_back(obj->indices[indexOffset + numVerticesUsed + tri + 2].t);
            }

            numVerticesUsed += numVerticesInFace;
        }
    }

    return obj;
}

void CheckForPresenceOfAssets(const char* alphaTextureName, const char* objName)
{
    HANDLE alphaTextureFile = CreateFileA(alphaTextureName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE objFile = CreateFileA(objName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (alphaTextureFile == INVALID_HANDLE_VALUE || objFile == INVALID_HANDLE_VALUE)
    {
        // Open a message box to inform the user that the assets are missing
        MessageBoxA(NULL, "Please ensure that the assets (jacaranda_tree_leaves_alpha_4k.dds and jacaranda_tree_4k_export.obj) are present in the same directory as the .sln/.vcxproj."
            "\n\nThe model and textures can be downloaded from https://polyhaven.com/a/jacaranda_tree - (CC0 licence)"
            "\n\nUse Blender to convert the .blend file to Export an OBJ called jacaranda_tree_4k_export.obj"
            "\n\nConvert jacaranda_tree_leaves_alpha_4k.png to an R8_UNORM DDS file using a tool such as Paint.net and name it jacaranda_tree_leaves_alpha_4k.dds", "Assets Missing", MB_OK | MB_ICONERROR);
        exit(1);
    }

    CloseHandle(alphaTextureFile);
    CloseHandle(objFile);
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

    const char* alphaTextureName = "jacaranda_tree_leaves_alpha_4k.dds";
    const char* objName = "jacaranda_tree_4k_export.obj";

    CheckForPresenceOfAssets(alphaTextureName, objName);

    // Create a texture
    omm::Cpu::TextureMipDesc mipDescs[16];
    omm::Cpu::TextureDesc texDesc;
    char* textureData = LoadDDSFile(alphaTextureName, texDesc, mipDescs);

    OMM_ABORT_ON_ERROR(omm::Cpu::CreateTexture(baker, texDesc, &input.texture));    

    // Load the model
    std::vector<GeometryData> geometryData;
    fastObjMesh* obj = LoadOBJFile(objName, geometryData);

    unsigned int alphaTestedGeometryIndex = -1;

    for (int i = 0; i < geometryData.size(); i++)
    {
        if (geometryData[i].isAlphaTested)
        {
            alphaTestedGeometryIndex = i;
            break;
        }
    }

    input.indexBuffer = geometryData[alphaTestedGeometryIndex].texCoordIndices.data();
    input.indexFormat = omm::IndexFormat::UINT_32;
    input.indexCount = geometryData[alphaTestedGeometryIndex].texCoordIndices.size();

    int numTexCoords = obj->texcoord_count;

    input.texCoords = obj->texcoords;
    input.texCoordFormat = omm::TexCoordFormat::UV32_FLOAT;
    input.texCoordStrideInBytes = 8;

    // Configuration    
    input.subdivisionLevels = nullptr;  // Use maxSubdivisionLevel globally
    input.runtimeSamplerDesc.addressingMode = omm::TextureAddressMode::Clamp;
    input.runtimeSamplerDesc.filter = omm::TextureFilterMode::Linear;
    input.runtimeSamplerDesc.borderAlpha = 0.0f;
    input.bakeFlags = omm::Cpu::BakeFlags::EnableInternalThreads | omm::Cpu::BakeFlags::EnableValidation;
    input.alphaMode = omm::AlphaMode::Test;
    
    input.maxSubdivisionLevel = 2;
    input.format = omm::Format::OC1_4_State;

    for (int subD = 1; subD <= 12; subD++)
    {

        for (int format = 1; format <= 2; format++)
        {
            input.maxSubdivisionLevel = subD;
            input.format = (omm::Format)format;

            // Time the bake using std::chrono
            auto start = std::chrono::high_resolution_clock::now();

            omm::Result res = omm::Cpu::Bake(baker, input, &result);

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = end - start;

            // Print the time to the console
            printf("Bake,%f\n", elapsed.count());

            if (res == omm::Result::SUCCESS)
            {
                OMM_ABORT_ON_ERROR(omm::Cpu::GetBakeResultDesc(result, &resultDesc));

                OMM_ABORT_ON_ERROR(omm::Debug::GetStats2(baker, result, &stats));

                // Serialize the OMMs to disk
                {
                    char ommFilename[256];
                    sprintf_s(ommFilename, "treeOMM_SubD%d_%dState.bin", input.maxSubdivisionLevel, input.format == omm::Format::OC1_4_State ? 4 : 2);

                    HANDLE fh = CreateFileA(ommFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

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

                    WriteFile(fh, resultDesc->indexBuffer, resultDesc->indexCount * (resultDesc->indexFormat == omm::IndexFormat::UINT_32 ? 4 : 2), nullptr, nullptr);

                    CloseHandle(fh);
                }
            }
        }
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

        UINT numGeoms = geometryData.size();
        WriteFile(fh, &numGeoms, sizeof(numGeoms), nullptr, nullptr);

        std::vector<unsigned int> positionIndices;
        std::vector<unsigned int> normalIndices;
        std::vector<unsigned int> texCoordIndices;

        UINT totalIndices = 0;

        for (int i = 0; i < numGeoms; i++)
        {
            GeometryData& geomData = geometryData[i];
            unsigned int geomIndexCount = geomData.positionIndices.size();

            WriteFile(fh, &geomIndexCount, sizeof(geomIndexCount), nullptr, nullptr);

            totalIndices += geomIndexCount;
        }

        // Write the indices
        WriteFile(fh, &totalIndices, sizeof(totalIndices), nullptr, nullptr);

        for (int i = 0; i < numGeoms; i++)
        {
            GeometryData& geomData = geometryData[i];
            WriteFile(fh, geomData.positionIndices.data(), geomData.positionIndices.size() * sizeof(UINT), nullptr, nullptr);
        }

        for (int i = 0; i < numGeoms; i++)
        {
            GeometryData& geomData = geometryData[i];
            WriteFile(fh, geomData.normalIndices.data(), geomData.normalIndices.size() * sizeof(UINT), nullptr, nullptr);
        }

        for (int i = 0; i < numGeoms; i++)
        {
            GeometryData& geomData = geometryData[i];
            WriteFile(fh, geomData.texCoordIndices.data(), geomData.texCoordIndices.size() * sizeof(UINT), nullptr, nullptr);
        }

        CloseHandle(fh);
    }

    OMM_ABORT_ON_ERROR(omm::Cpu::DestroyTexture(baker, input.texture));
    fast_obj_destroy(obj);
    delete[] textureData;
}