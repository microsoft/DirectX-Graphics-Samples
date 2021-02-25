#include "ModelH3D.h"
#include "ModelLoader.h"
#include "MeshConvert.h"
#include "TextureConvert.h"
#include "glTF.h"
#include <map>
#include <vector>
#include <string>
#include <locale>

using namespace std;
using namespace Renderer;

bool FileExists( const std::wstring& fileName )
{
    struct _stat64 fileStat;
    return _wstat64(fileName.c_str(), &fileStat) == 0;
}

bool ModelH3D::BuildModel(ModelData& model, const std::wstring& basePath) const
{
    model.m_SceneGraph.resize(1);

    GraphNode& node = model.m_SceneGraph[0];
    node.xform = Matrix4(kIdentity);
    node.rotation = Quaternion(kIdentity);
    node.scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
    node.matrixIdx = 0;
    node.hasSibling = 0;

    model.m_MaterialTextures.resize(m_Header.materialCount);
    model.m_MaterialConstants.resize(m_Header.materialCount);
    model.m_TextureNames.clear();
    model.m_TextureOptions.clear();

    std::map<std::string, uint16_t> fileNameMap;

    for (uint32_t i = 0; i < m_Header.materialCount; ++i)
    {
        const Material& material = GetMaterial(i);
        MaterialTextureData& textureData = model.m_MaterialTextures[i];
        textureData.addressModes = 0x55555;

        const string& baseColorPath = material.texDiffusePath;
        const string& metallicRoughnessPath = material.texSpecularPath;
        const string& emissivePath = material.texEmissivePath;
        const string& normalPath = material.texNormalPath;

        const wstring baseColorFullPath = basePath + Utility::UTF8ToWideString(baseColorPath);
        const wstring metallicRoughnessFullPath = basePath + Utility::UTF8ToWideString(metallicRoughnessPath);
        const wstring emissiveFullPath = basePath + Utility::UTF8ToWideString(emissivePath);
        const wstring normalFullPath = basePath + Utility::UTF8ToWideString(normalPath);

        textureData.stringIdx[kBaseColor] = 0xFFFF;
        textureData.stringIdx[kMetallicRoughness] = 0xFFFF;
        textureData.stringIdx[kOcclusion] = 0xFFFF;
        textureData.stringIdx[kEmissive] = 0xFFFF;
        textureData.stringIdx[kNormal] = 0xFFFF;

        bool alphaTest = false;
        const string lowerCaseName = Utility::ToLower(baseColorPath);
        if (lowerCaseName.find("thorn") != string::npos ||
            lowerCaseName.find("plant") != string::npos ||
            lowerCaseName.find("chain") != string::npos)
        {
            alphaTest = true;
        }

        // Handle base color texture
        auto mapLookup = fileNameMap.find(baseColorPath);
        if (mapLookup != fileNameMap.end())
        {
            textureData.stringIdx[kBaseColor] = mapLookup->second;
        }
        else if (baseColorPath.size() > 0 && FileExists(baseColorFullPath))
        {
            fileNameMap[baseColorPath] = (uint16_t)model.m_TextureNames.size();
            textureData.stringIdx[kBaseColor] = (uint16_t)model.m_TextureNames.size();
            model.m_TextureNames.push_back(baseColorPath);
            model.m_TextureOptions.push_back(TextureOptions(true, alphaTest, true));
        }

        // Handle occlusionMetallicRoughness texture
        mapLookup = fileNameMap.find(metallicRoughnessPath);
        if (mapLookup != fileNameMap.end())
        {
            textureData.stringIdx[kMetallicRoughness] = mapLookup->second;
        }
        else if (metallicRoughnessPath.size() > 0 && FileExists(metallicRoughnessFullPath))
        {
            fileNameMap[metallicRoughnessPath] = (uint16_t)model.m_TextureNames.size();
            textureData.stringIdx[kMetallicRoughness] = (uint16_t)model.m_TextureNames.size();
            textureData.stringIdx[kOcclusion] = (uint16_t)model.m_TextureNames.size();
            model.m_TextureNames.push_back(metallicRoughnessPath);
            model.m_TextureOptions.push_back(TextureOptions(false, false, true));
        }

        // Handle emissive texture
        mapLookup = fileNameMap.find(emissivePath);
        if (mapLookup != fileNameMap.end())
        {
            textureData.stringIdx[kEmissive] = mapLookup->second;
        }
        else if (emissivePath.size() > 0 && FileExists(emissiveFullPath))
        {
            fileNameMap[emissivePath] = (uint16_t)model.m_TextureNames.size();
            textureData.stringIdx[kEmissive] = (uint16_t)model.m_TextureNames.size();
            model.m_TextureNames.push_back(emissivePath);
            model.m_TextureOptions.push_back(TextureOptions(true, false, true));
        }

        // Handle normal texture
        mapLookup = fileNameMap.find(normalPath);
        if (mapLookup != fileNameMap.end())
        {
            textureData.stringIdx[kNormal] = mapLookup->second;
        }
        else if (normalPath.size() > 0 && FileExists(normalFullPath))
        {
            fileNameMap[normalPath] = (uint16_t)model.m_TextureNames.size();
            textureData.stringIdx[kNormal] = (uint16_t)model.m_TextureNames.size();
            model.m_TextureNames.push_back(normalPath);
            model.m_TextureOptions.push_back(TextureOptions(false, false, true));
        }

        MaterialConstantData& constantData = model.m_MaterialConstants[i];
        constantData.baseColorFactor[0] = 1.0f;
        constantData.baseColorFactor[1] = 1.0f;
        constantData.baseColorFactor[2] = 1.0f;
        constantData.baseColorFactor[3] = 1.0f;
        constantData.emissiveFactor[0] = 1.0f;
        constantData.emissiveFactor[1] = 1.0f;
        constantData.emissiveFactor[2] = 1.0f;
        constantData.metallicFactor = 1.0f;
        constantData.roughnessFactor = 1.0f;
        constantData.normalTextureScale = 1.0f;
        constantData.flags = 0x3C000000;
        if (alphaTest)
            constantData.flags |= 0x60; // twoSided + alphaTest
    }

    ASSERT(model.m_TextureOptions.size() == model.m_TextureNames.size());
    for (size_t ti = 0; ti < model.m_TextureNames.size(); ++ti)
    {
        std::wstring fullPath = basePath + Utility::UTF8ToWideString(model.m_TextureNames[ti]);
        CompileTextureOnDemand(fullPath, model.m_TextureOptions[ti]);
    }

    model.m_BoundingSphere = BoundingSphere(kZero);
    model.m_BoundingBox = AxisAlignedBox(kZero);

    // We're going to piggy-back off of the work to compile glTF meshes by pretending that's what
    // we have.
    for (uint32_t i = 0; i < m_Header.meshCount; ++i)
    {
        const Mesh& mesh = GetMesh(i);

        glTF::Accessor PosStream;
        PosStream.dataPtr = m_pVertexData + mesh.vertexDataByteOffset;
        PosStream.stride = mesh.vertexStride;
        PosStream.count = mesh.vertexCount;
        PosStream.componentType = glTF::Accessor::kFloat;
        PosStream.type = glTF::Accessor::kVec3;

        glTF::Accessor UVStream;
        UVStream.dataPtr = m_pVertexData + mesh.vertexDataByteOffset + 12;
        UVStream.stride = mesh.vertexStride;
        UVStream.count = mesh.vertexCount;
        UVStream.componentType = glTF::Accessor::kFloat;
        UVStream.type = glTF::Accessor::kVec2;

        glTF::Accessor NormalStream;
        NormalStream.dataPtr = m_pVertexData + mesh.vertexDataByteOffset + 20;
        NormalStream.stride = mesh.vertexStride;
        NormalStream.count = mesh.vertexCount;
        NormalStream.componentType = glTF::Accessor::kFloat;
        NormalStream.type = glTF::Accessor::kVec3;

        glTF::Accessor IndexStream;
        IndexStream.dataPtr = m_pIndexData + mesh.indexDataByteOffset;
        IndexStream.stride = 2;
        IndexStream.count = mesh.indexCount;
        IndexStream.componentType = glTF::Accessor::kUnsignedShort;
        IndexStream.type = glTF::Accessor::kScalar;

        glTF::Material material;
        material.flags = model.m_MaterialConstants[mesh.materialIndex].flags;
        material.index = mesh.materialIndex;
        
        glTF::Mesh gltfMesh;
        gltfMesh.primitives.resize(1);

        glTF::Primitive& prim = gltfMesh.primitives[0];
        prim.attributes[glTF::Primitive::kPosition] = &PosStream;
        prim.attributes[glTF::Primitive::kTexcoord0] = &UVStream;
        prim.attributes[glTF::Primitive::kNormal] = &NormalStream;
        prim.indices = &IndexStream;
        prim.material = &material;
        prim.attribMask = 0xB;
        prim.mode = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        memcpy(prim.minPos, &mesh.boundingBox.GetMin(), 12);
        memcpy(prim.maxPos, &mesh.boundingBox.GetMax(), 12);
        prim.minIndex = 0;
        prim.maxIndex = 0;

        BoundingSphere sphereOS;
        AxisAlignedBox boxOS;
        Renderer::CompileMesh(model.m_Meshes, model.m_GeometryData, gltfMesh, 0, Matrix4(kIdentity), sphereOS, boxOS); 
        model.m_BoundingSphere = model.m_BoundingSphere.Union(sphereOS);
        model.m_BoundingBox.AddBoundingBox(boxOS);
    }

    return true;
}