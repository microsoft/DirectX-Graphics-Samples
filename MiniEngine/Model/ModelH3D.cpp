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
// Author:  Alex Nankervis
//

#include "ModelH3D.h"
#include <string.h>
#include <float.h>

#include "Utility.h"
#include "TextureManager.h"
#include "GraphicsCore.h"
#include "DescriptorHeap.h"
#include "CommandContext.h"
#include "UploadBuffer.h"
#include "GraphicsCommon.h"
#include <fstream>

using namespace std;
using namespace Math;

namespace Renderer
{
    extern DescriptorHeap s_TextureHeap;
    extern DescriptorHeap s_SamplerHeap;
}

ModelH3D::ModelH3D()
    : m_pMesh(nullptr)
    , m_pMaterial(nullptr)
    , m_pVertexData(nullptr)
    , m_pIndexData(nullptr)
    , m_pVertexDataDepth(nullptr)
    , m_pIndexDataDepth(nullptr)
{
    Clear();
}

ModelH3D::~ModelH3D()
{
    Clear();
}

void ModelH3D::Clear()
{
    m_GeometryBuffer.Destroy();

    delete [] m_pMesh;
    m_pMesh = nullptr;
    m_Header.meshCount = 0;

    delete [] m_pMaterial;
    m_pMaterial = nullptr;
    m_Header.materialCount = 0;

    delete [] m_pVertexData;
    delete [] m_pIndexData;
    delete [] m_pVertexDataDepth;
    delete [] m_pIndexDataDepth;

    m_pVertexData = nullptr;
    m_Header.vertexDataByteSize = 0;
    m_pIndexData = nullptr;
    m_Header.indexDataByteSize = 0;
    m_pVertexDataDepth = nullptr;
    m_Header.vertexDataByteSizeDepth = 0;
    m_pIndexDataDepth = nullptr;

    m_TextureReferences.clear();

    m_Header.boundingBox = AxisAlignedBox();
}

// assuming at least 3 floats for position
void ModelH3D::ComputeMeshBoundingBox(uint32_t meshIndex, AxisAlignedBox& bbox) const
{
    const Mesh *mesh = m_pMesh + meshIndex;

    bbox = AxisAlignedBox();

    if (mesh->vertexCount > 0)
    {
        uint32_t vertexStride = mesh->vertexStride;

        const float *p = (float*)(m_pVertexData + mesh->vertexDataByteOffset + mesh->attrib[attrib_position].offset);
        const float *pEnd = (float*)(m_pVertexData + mesh->vertexDataByteOffset + mesh->vertexCount * mesh->vertexStride + mesh->attrib[attrib_position].offset);

        while (p < pEnd)
        {
            Vector3 pos(*(p + 0), *(p + 1), *(p + 2));

            bbox.AddPoint(pos);

            (*(uint8_t**)&p) += vertexStride;
        }
    }
}

void ModelH3D::ComputeGlobalBoundingBox(AxisAlignedBox &bbox) const
{
    bbox = AxisAlignedBox();

    if (m_Header.meshCount > 0)
    {
        for (uint32_t meshIndex = 0; meshIndex < m_Header.meshCount; meshIndex++)
        {
            const Mesh *mesh = m_pMesh + meshIndex;
            bbox.AddBoundingBox(mesh->boundingBox);
        }
    }
}

void ModelH3D::ComputeAllBoundingBoxes()
{
    for (uint32_t meshIndex = 0; meshIndex < m_Header.meshCount; meshIndex++)
    {
        Mesh *mesh = m_pMesh + meshIndex;
        ComputeMeshBoundingBox(meshIndex, mesh->boundingBox);
    }
    ComputeGlobalBoundingBox(m_Header.boundingBox);
}

bool ModelH3D::LoadH3D(const wstring& filename)
{
    std::ifstream file(filename, ios::in | ios::binary);

    file.read((char*)&m_Header, sizeof(Header));
    if (!file)
        return false;

    m_pMesh = new Mesh[m_Header.meshCount];
    m_pMaterial = new Material[m_Header.materialCount];

    if (m_Header.meshCount > 0)
    {
        file.read((char*)m_pMesh, sizeof(Mesh) * m_Header.meshCount);
        if (!file)
            return false;
    }
    if (m_Header.materialCount > 0)
    {
        file.read((char*)m_pMaterial, sizeof(Material) * m_Header.materialCount);
        if (!file)
            return false;
    }

    m_VertexStride = m_pMesh[0].vertexStride;
    m_VertexStrideDepth = m_pMesh[0].vertexStrideDepth;
#if _DEBUG
    for (uint32_t meshIndex = 1; meshIndex < m_Header.meshCount; ++meshIndex)
    {
        const Mesh& mesh = m_pMesh[meshIndex];
        ASSERT(mesh.vertexStride == m_VertexStride);
        ASSERT(mesh.vertexStrideDepth == m_VertexStrideDepth);
    }
    for (uint32_t meshIndex = 0; meshIndex < m_Header.meshCount; ++meshIndex)
    {
        const Mesh& mesh = m_pMesh[meshIndex];

        ASSERT( mesh.attribsEnabled ==
            (attrib_mask_position | attrib_mask_texcoord0 | attrib_mask_normal | attrib_mask_tangent | attrib_mask_bitangent) );
        ASSERT(mesh.attrib[0].components == 3 && mesh.attrib[0].format == ModelH3D::attrib_format_float); // position
        ASSERT(mesh.attrib[1].components == 2 && mesh.attrib[1].format == ModelH3D::attrib_format_float); // texcoord0
        ASSERT(mesh.attrib[2].components == 3 && mesh.attrib[2].format == ModelH3D::attrib_format_float); // normal
        ASSERT(mesh.attrib[3].components == 3 && mesh.attrib[3].format == ModelH3D::attrib_format_float); // tangent
        ASSERT(mesh.attrib[4].components == 3 && mesh.attrib[4].format == ModelH3D::attrib_format_float); // bitangent

        ASSERT( mesh.attribsEnabledDepth ==
            (attrib_mask_position) );
        ASSERT(mesh.attrib[0].components == 3 && mesh.attrib[0].format == ModelH3D::attrib_format_float); // position
    }
#endif

    UploadBuffer geomBuffer;

    uint32_t totalBinarySize = m_Header.vertexDataByteSize
        + m_Header.indexDataByteSize
        + m_Header.vertexDataByteSizeDepth
        + m_Header.indexDataByteSize;

    geomBuffer.Create(L"Geometry Upload Buffer", totalBinarySize);
    uint8_t* uploadMem = (uint8_t*)geomBuffer.Map();

    m_pVertexData = uploadMem;
    m_pIndexData = m_pVertexData + m_Header.vertexDataByteSize;
    m_pVertexDataDepth = m_pIndexData + m_Header.indexDataByteSize;
    m_pIndexDataDepth = m_pVertexDataDepth + m_Header.vertexDataByteSizeDepth;

    if (m_Header.vertexDataByteSize > 0 && !file.read((char*)m_pVertexData, m_Header.vertexDataByteSize))
        return false;
    if (m_Header.indexDataByteSize > 0 && !file.read((char*)m_pIndexData, m_Header.indexDataByteSize))
        return false;

    if (m_Header.vertexDataByteSizeDepth > 0 && !file.read((char*)m_pVertexDataDepth, m_Header.vertexDataByteSizeDepth))
        return false;
    if (m_Header.indexDataByteSize > 0 && !file.read((char*)m_pIndexDataDepth, m_Header.indexDataByteSize))
        return false;

    m_GeometryBuffer.Create(L"Geometry Buffer", totalBinarySize, 1, geomBuffer);

    m_VertexBuffer = m_GeometryBuffer.VertexBufferView(0, m_Header.vertexDataByteSize, m_VertexStride);
    m_IndexBuffer = m_GeometryBuffer.IndexBufferView(m_pIndexData - m_pVertexData, m_Header.indexDataByteSize, false);
    m_VertexBufferDepth = m_GeometryBuffer.VertexBufferView(m_pVertexDataDepth - m_pVertexData, m_Header.vertexDataByteSizeDepth, m_VertexStride);
    m_IndexBufferDepth = m_GeometryBuffer.IndexBufferView(m_pIndexDataDepth - m_pVertexData, m_Header.indexDataByteSize, false);

    m_pVertexData = new unsigned char[m_Header.vertexDataByteSize];
    m_pIndexData = new unsigned char[m_Header.indexDataByteSize];
    m_pVertexDataDepth = new unsigned char[m_Header.vertexDataByteSizeDepth];
    m_pIndexDataDepth = new unsigned char[m_Header.indexDataByteSize];

    std::memcpy(m_pVertexData, uploadMem, m_Header.vertexDataByteSize);
    uploadMem += m_Header.vertexDataByteSize;
    std::memcpy(m_pIndexData, uploadMem, m_Header.indexDataByteSize);
    uploadMem += m_Header.indexDataByteSize;
    std::memcpy(m_pVertexDataDepth, uploadMem, m_Header.vertexDataByteSizeDepth);
    uploadMem += m_Header.vertexDataByteSizeDepth;
    std::memcpy(m_pIndexDataDepth, uploadMem, m_Header.indexDataByteSize);

    geomBuffer.Unmap();


    LoadTextures(Utility::GetBasePath(filename));

    return true;
}

void ModelH3D::CreateVertexBufferSRV(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) const
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = (UINT)(m_VertexBuffer.SizeInBytes / sizeof(UINT32));
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
    srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    Graphics::g_Device->CreateShaderResourceView(const_cast<ID3D12Resource*>(m_GeometryBuffer.GetResource()), &srvDesc, cpuHandle);
}

void ModelH3D::CreateIndexBufferSRV(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) const
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = (UINT)(m_VertexBuffer.SizeInBytes / sizeof(UINT32));
    srvDesc.Buffer.NumElements = (UINT)(m_IndexBuffer.SizeInBytes / sizeof(UINT32));
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
    srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    Graphics::g_Device->CreateShaderResourceView(const_cast<ID3D12Resource*>(m_GeometryBuffer.GetResource()), &srvDesc, cpuHandle);
}

bool ModelH3D::SaveH3D(const wstring& filename) const
{
    std::ofstream file(filename, ios::out | ios::binary);
    if (!file)
        return false;

    if (!file.write((char*)&m_Header, sizeof(Header)))
        return false;

    if (m_Header.meshCount > 0 && !file.write((const char*)m_pMesh, sizeof(Mesh) * m_Header.meshCount))
        return false;

    if (m_Header.materialCount > 0 && !file.write((const char*)m_pMaterial, sizeof(Material) * m_Header.materialCount))
        return false;

    if (m_pVertexData != nullptr && m_Header.vertexDataByteSize > 0 &&
        !file.write((char*)m_pVertexData, m_Header.vertexDataByteSize))
        return false;

    if (m_pIndexData != nullptr && m_Header.indexDataByteSize > 0 &&
        !file.write((char*)m_pIndexData, m_Header.indexDataByteSize))
        return false;

    if (m_pVertexDataDepth != nullptr && m_Header.vertexDataByteSizeDepth > 0 &&
        !file.write((char*)m_pVertexDataDepth, m_Header.vertexDataByteSizeDepth))
        return false;

    if (m_pIndexDataDepth != nullptr && m_Header.indexDataByteSize > 0 &&
        !file.write((char*)m_pIndexDataDepth, m_Header.indexDataByteSize))
        return false;

    return true;
}

static inline std::wstring RemoveExt(const char* filename)
{
    return Utility::RemoveExtension(Utility::UTF8ToWideString(std::string(filename)));
}

void ModelH3D::LoadTextures(const std::wstring& basePath)
{
    using namespace Graphics;
    using namespace TextureManager;

    m_TextureReferences.resize(m_Header.materialCount * 3);
    m_SRVs = Renderer::s_TextureHeap.Alloc(m_Header.materialCount * 6);
    m_SRVDescriptorSize = Renderer::s_TextureHeap.GetDescriptorSize();

    DescriptorHandle SRVs = m_SRVs;

    TextureRef* MatTextures = m_TextureReferences.data();

    for (uint32_t materialIdx = 0; materialIdx < m_Header.materialCount; ++materialIdx)
    {
        const Material& pMaterial = m_pMaterial[materialIdx];

        // Load diffuse
        std::wstring diffusePath = basePath + RemoveExt(pMaterial.texDiffusePath);
        MatTextures[0] = LoadDDSFromFile(diffusePath + L".dds", kWhiteOpaque2D, true);

        // Load specular
        std::wstring specularPath = basePath + RemoveExt(pMaterial.texSpecularPath);
        MatTextures[1] = LoadDDSFromFile(specularPath + L".dds", kBlackOpaque2D, true);
        if (!MatTextures[1].IsValid())
            MatTextures[1] = LoadDDSFromFile(diffusePath + L"_specular.dds", kBlackOpaque2D, true);

        // Load normal
        std::wstring normalPath = basePath + RemoveExt(pMaterial.texNormalPath);
        MatTextures[2] = LoadDDSFromFile(normalPath + L".dds", kDefaultNormalMap, false);
        if (!MatTextures[2].IsValid())
            MatTextures[2] = LoadDDSFromFile(diffusePath + L"_normal.dds", kDefaultNormalMap, false);

        uint32_t DestCount = 6;
        uint32_t SourceCounts[] = { 1, 1, 1, 1, 1, 1 };
        D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[6] =
        {
            MatTextures[0].GetSRV(),
            MatTextures[1].GetSRV(),
            GetDefaultTexture(kBlackTransparent2D),
            MatTextures[2].GetSRV(),
            GetDefaultTexture(kBlackTransparent2D),
            GetDefaultTexture(kBlackCubeMap)
        };
        Graphics::g_Device->CopyDescriptors(1, &SRVs, &DestCount,
            DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        SRVs += (m_SRVDescriptorSize * 6);
        MatTextures += 3;
    }
}
