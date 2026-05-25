// GltfLoader.cpp
#include "stdafx.h"

#include "GltfLoader.h"

#include "MyDx12Utils.h"

#include <cstdint>
#include <cstring>
#include <tiny_gltf.h>

static const unsigned char *GetAccessorData(const tinygltf::Model &model, const tinygltf::Accessor &accessor)
{
    const auto &view = model.bufferViews[accessor.bufferView];
    const auto &buffer = model.buffers[view.buffer];

    return buffer.data.data() + view.byteOffset + accessor.byteOffset;
}

bool LoadGltfMesh(const std::string &path, GltfMeshData &outMesh)
{
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;

    std::string warn;
    std::string err;

    bool ok = false;

    const bool isGlb = path.size() >= 4 && path.substr(path.size() - 4) == ".glb";

    if (isGlb)
        ok = loader.LoadBinaryFromFile(&model, &err, &warn, path);
    else
        ok = loader.LoadASCIIFromFile(&model, &err, &warn, path);

    if (!warn.empty())
        OutputDebugStringA(("glTF warning: " + warn + "\n").c_str());

    if (!err.empty())
        OutputDebugStringA(("glTF error: " + err + "\n").c_str());

    if (!ok)
        return false;

    if (model.meshes.empty())
        return false;

    const tinygltf::Mesh &mesh = model.meshes[0];

    if (mesh.primitives.empty())
        return false;

    const tinygltf::Primitive &prim = mesh.primitives[0];

    auto posIt = prim.attributes.find("POSITION");
    auto normalIt = prim.attributes.find("NORMAL");
    auto uvIt = prim.attributes.find("TEXCOORD_0");
    auto tangentIt = prim.attributes.find("TANGENT");

    if (posIt == prim.attributes.end())
        return false;

    const auto &posAccessor = model.accessors[posIt->second];
    const float *positions = reinterpret_cast<const float *>(GetAccessorData(model, posAccessor));

    const float *normals = nullptr;
    if (normalIt != prim.attributes.end())
    {
        const auto &normalAccessor = model.accessors[normalIt->second];
        normals = reinterpret_cast<const float *>(GetAccessorData(model, normalAccessor));
    }

    const float *uvs = nullptr;
    if (uvIt != prim.attributes.end())
    {
        const auto &uvAccessor = model.accessors[uvIt->second];
        uvs = reinterpret_cast<const float *>(GetAccessorData(model, uvAccessor));
    }

    const float *tangents = nullptr;
    if (tangentIt != prim.attributes.end())
    {
        const auto &tangentAccessor = model.accessors[tangentIt->second];
        tangents = reinterpret_cast<const float *>(GetAccessorData(model, tangentAccessor));
        DBG_PRINT("glTF TANGENT attribute found.\n");
    }
    else
    {
        DBG_PRINT("glTF TANGENT attribute not found. Shader fallback tangent frame will be used.\n");
    }

    outMesh.vertices.resize(posAccessor.count);

    for (size_t i = 0; i < posAccessor.count; ++i)
    {
        GltfVertex v = {};

        v.position = {positions[i * 3 + 0], positions[i * 3 + 1], positions[i * 3 + 2]};

        if (normals)
        {
            v.normal = {normals[i * 3 + 0], normals[i * 3 + 1], normals[i * 3 + 2]};
        }
        else
        {
            v.normal = {0.0f, 1.0f, 0.0f};
        }

        if (uvs)
        {
            v.uv = {uvs[i * 2 + 0], uvs[i * 2 + 1]};
        }
        else
        {
            v.uv = {0.0f, 0.0f};
        }

        if (tangents)
        {
            v.tangent = {tangents[i * 4 + 0], tangents[i * 4 + 1], tangents[i * 4 + 2], tangents[i * 4 + 3]};
        }

        outMesh.vertices[i] = v;
    }

    if (prim.indices < 0)
        return false;

    const auto &indexAccessor = model.accessors[prim.indices];
    const auto *indexData = GetAccessorData(model, indexAccessor);

    outMesh.indices.resize(indexAccessor.count);

    if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
    {
        const uint16_t *src = reinterpret_cast<const uint16_t *>(indexData);
        for (size_t i = 0; i < indexAccessor.count; ++i)
            outMesh.indices[i] = src[i];
    }
    else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
    {
        const uint32_t *src = reinterpret_cast<const uint32_t *>(indexData);
        for (size_t i = 0; i < indexAccessor.count; ++i)
            outMesh.indices[i] = src[i];
    }
    else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
    {
        const uint8_t *src = reinterpret_cast<const uint8_t *>(indexData);
        for (size_t i = 0; i < indexAccessor.count; ++i)
            outMesh.indices[i] = src[i];
    }
    else
    {
        return false;
    }

    // load material

    const int matIndex = prim.material;

    outMesh.materialIndex = matIndex;

    DBG_PRINT("Material index: %d\n", matIndex);

    if (matIndex >= 0)
    {
        const auto &mat = model.materials[matIndex];
        const auto &pbr = mat.pbrMetallicRoughness;

        outMesh.materials.resize(model.materials.size());

        auto &dst = outMesh.materials[matIndex];

        dst.albedoTexIndex = pbr.baseColorTexture.index;
        dst.metallicRoughnessTexIndex = pbr.metallicRoughnessTexture.index;
        dst.emissiveTexIndex = mat.emissiveTexture.index;
        dst.occlusionTexIndex = mat.occlusionTexture.index;
        dst.normalTexIndex = mat.normalTexture.index;
        dst.roughnessFactor = static_cast<float>(pbr.roughnessFactor);
        dst.metallicFactor = static_cast<float>(pbr.metallicFactor);
        dst.occlusionStrength = static_cast<float>(mat.occlusionTexture.strength);

        DBG_PRINT("model.materials[%d].name = %s\n", matIndex, mat.name.c_str());
        DBG_PRINT("baseColorTexture.index: %d\n", dst.albedoTexIndex);
        DBG_PRINT("metallicRoughnessTexture.index: %d\n", dst.metallicRoughnessTexIndex);
        DBG_PRINT("emissiveTexture.index: %d\n", dst.emissiveTexIndex);
        DBG_PRINT("occlusionTexture.index: %d\n", dst.occlusionTexIndex);
        DBG_PRINT("normalTexture.index: %d\n", dst.normalTexIndex);

        for (int i = 0; i < 4; ++i)
        {
            dst.baseColorFactor[i] = static_cast<float>(pbr.baseColorFactor[i]);
            DBG_PRINT("baseColorFactor[%d]: %f\n", i, dst.baseColorFactor[i]);
        }
        DBG_PRINT("roughnessFactor: %f\n", dst.roughnessFactor);
        DBG_PRINT("metallicFactor: %f\n", dst.metallicFactor);
        DBG_PRINT("occlusionStrength: %f\n", dst.occlusionStrength);
    }

    for (const auto &tex : model.textures)
    {
        const tinygltf::Image &image = model.images[tex.source];

        GltfTextureData dst;
        DBG_PRINT("image.name = %s\n", image.name.c_str());
        dst.width = image.width;
        dst.height = image.height;
        dst.component = image.component;
        dst.pixels = image.image;

        outMesh.textures.push_back(std::move(dst));
    }

    return true;
}
