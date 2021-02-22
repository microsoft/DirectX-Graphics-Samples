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
// Author:  James Stanard
//

#include "glTF.h"

#include "../Core/CommandContext.h"
#include "../Core/SamplerManager.h"
#include "../Core/UploadBuffer.h"
#include "../Core/GraphicsCore.h"
#include "../Core/FileUtility.h"

#include <fstream>
#include <iostream>

using namespace glTF;
using namespace Graphics;
using namespace Utility;

void ReadFloats( json& list, float flt_array[] )
{
    uint32_t i = 0;
    for (auto& flt : list)
        flt_array[i++] = flt;
}

void glTF::Asset::ProcessNodes( json& nodes )
{
    m_nodes.resize(nodes.size());

    uint32_t nodeIdx = 0;

    for (json::iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        glTF::Node& node = m_nodes[nodeIdx++];
        json& thisNode = it.value();

        node.flags = 0;
        node.mesh = nullptr;
        node.linearIdx = -1;

        if (thisNode.find("camera") != thisNode.end())
        {
            node.camera = &m_cameras[thisNode.at("camera")];
            node.pointsToCamera = true;
        }
        else if (thisNode.find("mesh") != thisNode.end())
        {
            node.mesh = &m_meshes[thisNode.at("mesh")];
        }

        if (thisNode.find("skin") != thisNode.end())
        {
            ASSERT(node.mesh != nullptr);
            node.mesh->skin = thisNode.at("skin");
        }

        if (thisNode.find("children") != thisNode.end())
        {
            json& children = thisNode["children"];
            node.children.reserve(children.size());
            for (auto& child : children)
                node.children.push_back(&m_nodes[child]);
        }

        if (thisNode.find("matrix") != thisNode.end())
        {
            // TODO:  Should check for negative determinant to reverse triangle winding
            ReadFloats(thisNode["matrix"], node.matrix);
            node.hasMatrix = true;
        }
        else
        {
            // TODO:  Should check scale for 1 or 3 negative values to reverse triangle winding
            json::iterator scale = thisNode.find("scale");
            if (scale != thisNode.end())
            {
                ReadFloats(scale.value(), node.scale);
            }
            else
            {
                node.scale[0] = 1.0f;
                node.scale[1] = 1.0f;
                node.scale[2] = 1.0f;
            }

            json::iterator rotation = thisNode.find("rotation");
            if (rotation != thisNode.end())
            {
                ReadFloats(rotation.value(), node.rotation);
            }
            else
            {
                node.rotation[0] = 0.0f;
                node.rotation[1] = 0.0f;
                node.rotation[2] = 0.0f;
                node.rotation[3] = 1.0f;
            }

            json::iterator translation = thisNode.find("translation");
            if (translation != thisNode.end())
            {
                ReadFloats(translation.value(), node.translation);
            }
            else
            {
                node.translation[0] = 0.0f;
                node.translation[1] = 0.0f;
                node.translation[2] = 0.0f;
            }
        }
    }
}

void glTF::Asset::ProcessScenes( json& scenes )
{
    m_scenes.reserve(scenes.size());

    for (json::iterator it = scenes.begin(); it != scenes.end(); ++it)
    {
        glTF::Scene scene;
        json& thisScene = it.value();

        if (thisScene.find("nodes") != thisScene.end())
        {
            json& nodes = thisScene["nodes"];
            scene.nodes.reserve(nodes.size());
            for (auto& node : nodes)
                scene.nodes.push_back(&m_nodes[node]);
        }

        m_scenes.push_back(scene);
    }
}

void glTF::Asset::ProcessCameras( json& cameras )
{
    m_cameras.reserve(cameras.size());

    for (json::iterator it = cameras.begin(); it != cameras.end(); ++it)
    {
        glTF::Camera camera;
        json& thisCamera = it.value();

        if (thisCamera["type"] == "perspective")
        {
            json& perspective = thisCamera["perspective"];
            camera.type = Camera::kPerspective;
            camera.aspectRatio = 0.0f;
            if (perspective.find("aspectRatio") != perspective.end())
                camera.aspectRatio = perspective.at("aspectRatio");
            camera.yfov = perspective["yfov"];
            camera.znear = perspective["znear"];
            camera.zfar = 0.0f;
            if (perspective.find("zfar") != perspective.end())
                camera.zfar = perspective.at("zfar");
        }
        else
        {
            camera.type = Camera::kOrthographic;
            json& orthographic = thisCamera["orthographic"];
            camera.xmag = orthographic["xmag"];
            camera.ymag = orthographic["ymag"];
            camera.znear = orthographic["znear"];
            camera.zfar = orthographic["zfar"];
            ASSERT(camera.zfar > camera.znear);
        }

        m_cameras.push_back(camera);
    }
}

uint16_t TypeToEnum( const char type[] )
{
    if (strncmp(type, "VEC", 3) == 0)
        return Accessor::kVec2 + type[3] - '2';
    else if (strncmp(type, "MAT", 3) == 0)
        return Accessor::kMat2 + type[3] - '2';
    else
        return Accessor::kScalar;
}

void glTF::Asset::ProcessAccessors( json& accessors )
{
    m_accessors.reserve(accessors.size());

    for (json::iterator it = accessors.begin(); it != accessors.end(); ++it)
    {
        glTF::Accessor accessor;
        json& thisAccessor = it.value();

        glTF::BufferView& bufferView = m_bufferViews[thisAccessor.at("bufferView")];
        accessor.dataPtr = m_buffers[bufferView.buffer]->data() + bufferView.byteOffset;
        accessor.stride = bufferView.byteStride;
        if (thisAccessor.find("byteOffset") != thisAccessor.end())
            accessor.dataPtr += thisAccessor.at("byteOffset");
        accessor.count = thisAccessor.at("count");
        accessor.componentType = thisAccessor.at("componentType").get<uint16_t>() - 5120;

        char type[8];
        strcpy_s(type, thisAccessor.at("type").get<std::string>().c_str());

        accessor.type = TypeToEnum(type);

        m_accessors.push_back(accessor);
    }
}

void glTF::Asset::FindAttribute( Primitive& prim, json& attributes, Primitive::eAttribType type, const string& name )
{
    json::iterator attrib = attributes.find(name);
    if (attrib != attributes.end())
    {
        prim.attribMask |= 1 << type;
        prim.attributes[type] = &m_accessors[attrib.value()];
    }
    else
    {
        prim.attributes[type] = nullptr;
    }
}

void glTF::Asset::ProcessMeshes( json& meshes, json& accessors )
{
    m_meshes.resize(meshes.size());

    uint32_t curMesh = 0;
    for (json::iterator meshIt = meshes.begin(); meshIt != meshes.end(); ++meshIt, ++curMesh)
    {
        json& thisMesh = meshIt.value();
        json& primitives = thisMesh.at("primitives");

        m_meshes[curMesh].primitives.resize(primitives.size());
        m_meshes[curMesh].skin = -1;

        uint32_t curSubMesh = 0;
        for (json::iterator primIt = primitives.begin(); primIt != primitives.end(); ++primIt, ++curSubMesh)
        {
            glTF::Primitive& prim = m_meshes[curMesh].primitives[curSubMesh];
            json& thisPrim = primIt.value();

            prim.attribMask = 0;
            json& attributes = thisPrim.at("attributes");

            FindAttribute(prim, attributes, Primitive::kPosition, "POSITION");
            FindAttribute(prim, attributes, Primitive::kNormal, "NORMAL");
            FindAttribute(prim, attributes, Primitive::kTangent, "TANGENT");
            FindAttribute(prim, attributes, Primitive::kTexcoord0, "TEXCOORD_0");
            FindAttribute(prim, attributes, Primitive::kTexcoord1, "TEXCOORD_1");
            FindAttribute(prim, attributes, Primitive::kColor0, "COLOR_0");
            FindAttribute(prim, attributes, Primitive::kJoints0, "JOINTS_0");
            FindAttribute(prim, attributes, Primitive::kWeights0, "WEIGHTS_0");

            // Read position AABB
            json& positionAccessor = accessors[attributes.at("POSITION").get<uint32_t>()];
            ReadFloats(positionAccessor.at("min"), prim.minPos);
            ReadFloats(positionAccessor.at("max"), prim.maxPos);

            prim.mode = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            prim.indices = nullptr;
            prim.material = nullptr;
            prim.minIndex = 0;
            prim.maxIndex = 0;
            prim.mode = 4;

            if (thisPrim.find("mode") != thisPrim.end())
                prim.mode = thisPrim.at("mode");

            if (thisPrim.find("indices") != thisPrim.end())
            {
                uint32_t accessorIndex = thisPrim.at("indices");
                json& indicesAccessor = accessors[accessorIndex];
                prim.indices = &m_accessors[accessorIndex];
                if (indicesAccessor.find("max") != indicesAccessor.end())
                    prim.maxIndex = indicesAccessor.at("max")[0];
                if (indicesAccessor.find("min") != indicesAccessor.end())
                    prim.minIndex = indicesAccessor.at("min")[0];
            }

            if (thisPrim.find("material") != thisPrim.end())
                prim.material = &m_materials[thisPrim.at("material")];

            // TODO:  Add morph targets
            //if (thisPrim.find("targets") != thisPrim.end())
        }
    }
}

void glTF::Asset::ProcessSkins( json& skins )
{
    uint32_t skinIdx = 0;

    for (json::iterator it = skins.begin(); it != skins.end(); ++it)
    {
        glTF::Skin& skin = m_skins[skinIdx++];

        json& thisSkin = it.value();

        skin.inverseBindMatrices = nullptr;
        skin.skeleton = nullptr;

        if (thisSkin.find("inverseBindMatrices") != thisSkin.end())
            skin.inverseBindMatrices = &m_accessors[thisSkin.at("inverseBindMatrices")];

        if (thisSkin.find("skeleton") != thisSkin.end())
        {
            skin.skeleton = &m_nodes[thisSkin.at("skeleton")];
            skin.skeleton->skeletonRoot = true;
        }

        json& joints = thisSkin.at("joints");
        skin.joints.reserve(joints.size());
        for (auto& joint : joints)
            skin.joints.push_back(&m_nodes[joint]);
    }
}

inline uint32_t floatToHalf( float f )
{
    const float kF32toF16 = (1.0 / (1ull << 56)) * (1.0 / (1ull << 56)); // 2^-112
    union { float f; uint32_t u; } x;
    x.f = Math::Clamp(f, 0.0f, 1.0f) * kF32toF16; 
    return x.u >> 13;
}

uint32_t glTF::Asset::ReadTextureInfo( json& info_json, glTF::Texture* &info )
{
    info = nullptr;

    if (info_json.find("index") != info_json.end())
        info = &m_textures[info_json.at("index")];

    if (info_json.find("texCoord") != info_json.end())
        return info_json.at("texCoord");
    else
        return 0;
}

void glTF::Asset::ProcessMaterials( json& materials )
{
    m_materials.reserve(materials.size());

    uint32_t materialIdx = 0;

    for (json::iterator it = materials.begin(); it != materials.end(); ++it)
    {
        glTF::Material material;
        json& thisMaterial = it.value();

        material.index = materialIdx++;
        material.flags = 0;
        material.alphaCutoff = floatToHalf(0.5f);
        material.normalTextureScale = 1.0f;

        if (thisMaterial.find("alphaMode") != thisMaterial.end())
        {
            string alphaMode = thisMaterial.at("alphaMode");
            if (alphaMode == "BLEND")
                material.alphaBlend = true;
            else if (alphaMode == "MASK")
                material.alphaTest = true;
        }

        if (thisMaterial.find("alphaCutoff") != thisMaterial.end())
        {
            material.alphaCutoff = floatToHalf(thisMaterial.at("alphaCutoff"));
            //material.alphaTest = true;  // Should we alpha test and alpha blend?
        }

        if (thisMaterial.find("pbrMetallicRoughness") != thisMaterial.end())
        {
            json& metallicRoughness = thisMaterial.at("pbrMetallicRoughness");

            material.baseColorFactor[0] = 1.0f;
            material.baseColorFactor[1] = 1.0f;
            material.baseColorFactor[2] = 1.0f;
            material.baseColorFactor[3] = 1.0f;
            material.metallicFactor = 1.0f;
            material.roughnessFactor = 1.0f;
            for (uint32_t i = 0; i < Material::kNumTextures; ++i)
                material.textures[i] = nullptr;

            if (metallicRoughness.find("baseColorFactor") != metallicRoughness.end())
                ReadFloats(metallicRoughness.at("baseColorFactor"), material.baseColorFactor);

            if (metallicRoughness.find("metallicFactor") != metallicRoughness.end())
                material.metallicFactor = metallicRoughness.at("metallicFactor");

            if (metallicRoughness.find("roughnessFactor") != metallicRoughness.end())
                material.roughnessFactor = metallicRoughness.at("roughnessFactor");

            if (metallicRoughness.find("baseColorTexture") != metallicRoughness.end())
                material.baseColorUV = ReadTextureInfo(metallicRoughness.at("baseColorTexture"),
                    material.textures[Material::kBaseColor]);

            if (metallicRoughness.find("metallicRoughnessTexture") != metallicRoughness.end())
                material.metallicRoughnessUV = ReadTextureInfo(metallicRoughness.at("metallicRoughnessTexture"),
                    material.textures[Material::kMetallicRoughness]);
        }

        if (thisMaterial.find("doubleSided") != thisMaterial.end())
            material.twoSided = thisMaterial.at("doubleSided");

        if (thisMaterial.find("normalTextureScale") != thisMaterial.end())
            material.normalTextureScale = thisMaterial.at("normalTextureScale");

        if (thisMaterial.find("emissiveFactor") != thisMaterial.end())
            ReadFloats(thisMaterial.at("emissiveFactor"), material.emissiveFactor);

        if (thisMaterial.find("occlusionTexture") != thisMaterial.end())
            material.occlusionUV = ReadTextureInfo(thisMaterial.at("occlusionTexture"),
                material.textures[Material::kOcclusion]);

        if (thisMaterial.find("emissiveTexture") != thisMaterial.end())
            material.emissiveUV = ReadTextureInfo(thisMaterial.at("emissiveTexture"),
                material.textures[Material::kEmissive]);

        if (thisMaterial.find("normalTexture") != thisMaterial.end())
            material.normalUV = ReadTextureInfo(thisMaterial.at("normalTexture"),
                material.textures[Material::kNormal]);

        m_materials.push_back(material);
    }
}

bool ReadFile(const wstring& fileName, void* Dest, size_t Size)
{
    struct _stat64 fileStat;
    int fileExists = _wstat64(fileName.c_str(), &fileStat);
    if (fileExists == -1)
        return false;

    ifstream file(fileName, ios::in | ios::binary);
    if (!file)
        return false;

    ASSERT(Size == (size_t)fileStat.st_size);
    file.read((char*)Dest, Size);
    file.close();

    return true;
}

void glTF::Asset::ProcessBuffers( json& buffers, ByteArray chunk1bin )
{
    m_buffers.reserve(buffers.size());

    for (json::iterator it = buffers.begin(); it != buffers.end(); ++it)
    {
        json& thisBuffer = it.value();

        if (thisBuffer.find("uri") != thisBuffer.end())
        {
            const string& uri = thisBuffer.at("uri");
            wstring filepath = m_basePath + wstring(uri.begin(), uri.end());

            ByteArray ba = ReadFileSync(filepath);
            ASSERT(ba->size() > 0, "Missing bin file %ws", filepath.c_str());
            m_buffers.push_back(ba);
        }
        else
        {
            ASSERT(it == buffers.begin(), "Only the 1st buffer allowed to be internal");
            ASSERT(chunk1bin->size() > 0, "GLB chunk1 missing data or not a GLB file");
            m_buffers.push_back(chunk1bin);
        }
    }
}

void glTF::Asset::ProcessBufferViews( json& bufferViews )
{
    m_bufferViews.reserve(bufferViews.size());

    for (json::iterator it = bufferViews.begin(); it != bufferViews.end(); ++it)
    {
        glTF::BufferView bufferView;
        json& thisBufferView = it.value();

        bufferView.buffer = thisBufferView.at("buffer");
        bufferView.byteLength = thisBufferView.at("byteLength");
        bufferView.byteOffset = 0;
        bufferView.byteStride = 0;
        bufferView.elementArrayBuffer = false;

        if (thisBufferView.find("byteOffset") != thisBufferView.end())
            bufferView.byteOffset = thisBufferView.at("byteOffset");

        if (thisBufferView.find("byteStride") != thisBufferView.end())
            bufferView.byteStride = thisBufferView.at("byteStride");

        // 34962 = ARRAY_BUFFER;  34963 = ELEMENT_ARRAY_BUFFER
        if (thisBufferView.find("target") != thisBufferView.end() && thisBufferView.at("target") == 34963)
            bufferView.elementArrayBuffer = true;

        m_bufferViews.push_back(bufferView);
    }
}

void glTF::Asset::ProcessImages( json& images )
{
    m_images.resize(images.size());

    uint32_t imageIdx = 0;

    for (json::iterator it = images.begin(); it != images.end(); ++it)
    {
        json& thisImage = it.value();
        if (thisImage.find("uri") != thisImage.end())
        {
            m_images[imageIdx++].path = thisImage.at("uri").get<string>();
        }
        else if (thisImage.find("bufferView") != thisImage.end())
        {
            Utility::Printf("GLB image at buffer view %d with mime type %s\n", thisImage.at("bufferView").get<uint32_t>(), thisImage.at("mimeType").get<string>().c_str());
        }
        else
        {
            ASSERT(0);
        }
    }
}

D3D12_TEXTURE_ADDRESS_MODE GLtoD3DTextureAddressMode( int32_t glWrapMode )
{
    switch (glWrapMode)
    {
    default: ERROR("Unexpected sampler wrap mode");
    case 33071: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    case 33648: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    case 10497: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    }
}

/*
D3D12_FILTER GLtoD3DTextureFilterMode( int32_t magFilter, int32_t minFilter )
{
    bool linearMag = magFilter == 9729;
    switch (minFilter)
    {
    case 9728: //nearest
    case 9984: return linearMag ? D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_POINT;//nearest_mipmap_nearest
    case 9729: //linear
    case 9987: return linearMag ? D3D12_FILTER_MIN_MAG_MIP_LINEAR : D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;//linear_mipmap_linear
    case 9985: return linearMag ? D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT : D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;//linear_mipmap_nearest
    case 9986: break;//nearest_mipmap_linear
    }
}
*/

void glTF::Asset::ProcessSamplers( json& samplers )
{
    m_samplers.resize(samplers.size());

    uint32_t samplerIdx = 0;

    for (json::iterator it = samplers.begin(); it != samplers.end(); ++it)
    {
        json& thisSampler = it.value();

        glTF::Sampler& sampler = m_samplers[samplerIdx++];
        sampler.filter = D3D12_FILTER_ANISOTROPIC;
        sampler.wrapS = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.wrapT = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

        /*
        // Who cares what is provided?  It's about what you can afford, generally
        // speaking about materials.  If you want anisotropic filtering, why let
        // the asset dictate that.  And AF isn't represented in WebGL, so blech.
        int32_t magFilter = 9729;
        int32_t minFilter = 9987;
        if (thisSampler.find("magFilter") != thisSampler.end())
            magFilter = thisSampler.at("magFilter");
        if (thisSampler.find("minFilter") != thisSampler.end())
            minFilter = thisSampler.at("minFilter");
        sampler.filter = GLtoD3DTextureFilterMode(magFilter, minFilter);
        */

        // But these could matter for correctness.  Though, where is border mode?
        if (thisSampler.find("wrapS") != thisSampler.end())
            sampler.wrapS = GLtoD3DTextureAddressMode(thisSampler.at("wrapS"));
        if (thisSampler.find("wrapT") != thisSampler.end())
            sampler.wrapT = GLtoD3DTextureAddressMode(thisSampler.at("wrapT"));
    }
}

void glTF::Asset::ProcessTextures( json& textures )
{
    m_textures.resize(textures.size());

    uint32_t texIdx = 0;

    for (json::iterator it = textures.begin(); it != textures.end(); ++it)
    {
        glTF::Texture& texture = m_textures[texIdx++];
        json& thisTexture = it.value();

        texture.source = nullptr;
        texture.sampler = nullptr;

        if (thisTexture.find("source") != thisTexture.end())
            texture.source = &m_images[thisTexture.at("source")];

        if (thisTexture.find("sampler") != thisTexture.end())
            texture.sampler = &m_samplers[thisTexture.at("sampler")];
    }
}

void glTF::Asset::ProcessAnimations(json& animations)
{
    m_animations.resize(animations.size());
    uint32_t animIdx = 0;

    // Process all animations
    for (json::iterator it = animations.begin(); it != animations.end(); ++it)
    {
        json& thisAnimation = it.value();
        glTF::Animation& animation = m_animations[animIdx++];

        // Process this animation's samplers
        json& samplers = thisAnimation.at("samplers");
        animation.m_samplers.resize(samplers.size());
        uint32_t samplerIdx = 0;

        for (json::iterator it2 = samplers.begin(); it2 != samplers.end(); ++it2)
        {
            json& thisSampler = it2.value();
            glTF::AnimSampler& sampler = animation.m_samplers[samplerIdx++];
            sampler.m_input = &m_accessors[thisSampler.at("input")];
            sampler.m_output = &m_accessors[thisSampler.at("output")];
            sampler.m_interpolation = AnimSampler::kLinear;
            if (thisSampler.find("interpolation") != thisSampler.end())
            {
                const std::string& interpolation = thisSampler.at("interpolation");
                if (interpolation == "LINEAR")
                    sampler.m_interpolation = AnimSampler::kLinear;
                else if (interpolation == "STEP")
                    sampler.m_interpolation = AnimSampler::kStep;
                else if (interpolation == "CATMULLROMSPLINE")
                    sampler.m_interpolation = AnimSampler::kCatmullRomSpline;
                else if (interpolation == "CUBICSPLINE")
                    sampler.m_interpolation = AnimSampler::kCubicSpline;
            }
        }

        // Process this animation's channels
        json& channels = thisAnimation.at("channels");
        animation.m_channels.resize(channels.size());
        uint32_t channelIdx = 0;

        for (json::iterator it2 = channels.begin(); it2 != channels.end(); ++it2)
        {
            json& thisChannel = it2.value();
            glTF::AnimChannel& channel = animation.m_channels[channelIdx++];
            channel.m_sampler = &animation.m_samplers[thisChannel.at("sampler")];
            json& thisTarget = thisChannel.at("target");
            channel.m_target = &m_nodes[thisTarget.at("node")];
            const std::string& path = thisTarget.at("path");
            if (path == "translation")
                channel.m_path = AnimChannel::kTranslation;
            else if (path == "rotation")
                channel.m_path = AnimChannel::kRotation;
            else if (path == "scale")
                channel.m_path = AnimChannel::kScale;
            else if (path == "weights")
                channel.m_path = AnimChannel::kWeights;
        }
    }
}

void glTF::Asset::Parse(const std::wstring& filepath)
{
    // TODO:  add GLB support by extracting JSON section and BIN sections
    //https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/README.md#glb-file-format-specification

    ByteArray gltfFile;
    ByteArray chunk1Bin;

    std::wstring fileExt = Utility::ToLower(Utility::GetFileExtension(filepath));

    if (fileExt == L"glb")
    {
        ifstream glbFile(filepath, ios::in | ios::binary);
        struct GLBHeader
        {
            char magic[4];
            uint32_t version;
            uint32_t length;
        } header;
        glbFile.read((char*)&header, sizeof(GLBHeader));
        if (strncmp(header.magic, "glTF", 4) != 0)
        {
            Utility::Printf("Error:  Invalid glTF binary format\n");
            return;
        }
        if (header.version != 2)
        {
            Utility::Printf("Error:  Only glTF 2.0 is supported\n");
            return;
        }

        uint32_t chunk0Length;
        char chunk0Type[4];
        glbFile.read((char*)&chunk0Length, 4);
        glbFile.read((char*)&chunk0Type, 4);
        if (strncmp(chunk0Type, "JSON", 4) != 0)
        {
            Utility::Printf("Error: Expected chunk0 to contain JSON\n");
            return;
        }
        gltfFile = make_shared<vector<byte>>( chunk0Length + 1 );
        glbFile.read((char*)gltfFile->data(), chunk0Length);
        (*gltfFile)[chunk0Length] = '\0';

        uint32_t chunk1Length;
        char chunk1Type[4];
        glbFile.read((char*)&chunk1Length, 4);
        glbFile.read((char*)&chunk1Type, 4);
        if (strncmp(chunk1Type, "BIN", 3) != 0)
        {
            Utility::Printf("Error: Expected chunk1 to contain BIN\n");
            return;
       }

        chunk1Bin = make_shared<vector<byte>>(chunk1Length);
        glbFile.read((char*)chunk1Bin->data(), chunk1Length);
    }
    else 
    {
        ASSERT(fileExt == L"gltf");

        // Null terminate the string (just in case)
        gltfFile = ReadFileSync(filepath);
        if (gltfFile->size() == 0)
            return;

        gltfFile->push_back('\0');
        chunk1Bin = make_shared<vector<byte>>(0);
    }

    json root = json::parse((const char*)gltfFile->data());
    if (!root.is_object())
    {
        Printf("Invalid glTF file: %s\n", filepath.c_str());
        return;
    }

    // Strip off file name to get root path to other related files
    m_basePath = Utility::GetBasePath(filepath);

    // Parse all state

    if (root.find("buffers") != root.end())
        ProcessBuffers(root.at("buffers"), chunk1Bin);
    if (root.find("bufferViews") != root.end())
        ProcessBufferViews(root.at("bufferViews"));
    if (root.find("accessors") != root.end())
        ProcessAccessors(root.at("accessors"));
    if (root.find("images") != root.end())
        ProcessImages(root.at("images"));
    if (root.find("samplers") != root.end())
        ProcessSamplers(root.at("samplers"));
    if (root.find("textures") != root.end())
        ProcessTextures(root.at("textures"));
    if (root.find("materials") != root.end())
        ProcessMaterials(root.at("materials"));
    if (root.find("meshes") != root.end())
        ProcessMeshes(root.at("meshes"), root.at("accessors"));
    if (root.find("cameras") != root.end())
        ProcessCameras(root.at("cameras"));
    if (root.find("skins") != root.end())
        m_skins.resize(root.at("skins").size());
    if (root.find("nodes") != root.end())
        ProcessNodes(root.at("nodes"));
    if (root.find("skins") != root.end())
        ProcessSkins(root.at("skins"));
    if (root.find("scenes") != root.end())
        ProcessScenes(root.at("scenes"));
    if (root.find("animations") != root.end())
        ProcessAnimations(root.at("animations"));
    if (root.find("scene") != root.end())
        m_scene = &m_scenes[root.at("scene")];
}
