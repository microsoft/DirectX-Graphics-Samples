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
// Author(s):  James Stanard
//             Chuck Walbourn (ATG)
//
// This code depends on DirectXTex
//

#include "MeshConvert.h"
#include "TextureConvert.h"
#include "glTF.h"
#include "Model.h"
#include "IndexOptimizePostTransform.h"
#include "../Core/VectorMath.h"
#include "DirectXMesh.h"

using namespace DirectX;
using namespace glTF;
using namespace Math;

static DXGI_FORMAT JointIndexFormat(const Accessor& accessor)
{
    switch (accessor.componentType)
    {
    case Accessor::kUnsignedByte:  return DXGI_FORMAT_R8G8B8A8_UINT;
    case Accessor::kUnsignedShort: return DXGI_FORMAT_R16G16B16A16_UINT;
    default:
        ASSERT("Invalid joint index format");
        return DXGI_FORMAT_UNKNOWN;
    }
}

static DXGI_FORMAT AccessorFormat(const Accessor& accessor)
{
    switch (accessor.componentType)
    {
    case Accessor::kUnsignedByte:
        switch (accessor.type)
        {
        case Accessor::kScalar: return DXGI_FORMAT_R8_UNORM;
        case Accessor::kVec2:   return DXGI_FORMAT_R8G8_UNORM;
        default:                return DXGI_FORMAT_R8G8B8A8_UNORM;
        }
    case Accessor::kUnsignedShort:
        switch (accessor.type)
        {
        case Accessor::kScalar: return DXGI_FORMAT_R16_UNORM;
        case Accessor::kVec2:   return DXGI_FORMAT_R16G16_UNORM;
        default:                return DXGI_FORMAT_R16G16B16A16_UNORM;
        }
    case Accessor::kFloat:
        switch (accessor.type)
        {
        case Accessor::kScalar: return DXGI_FORMAT_R32_FLOAT;
        case Accessor::kVec2:   return DXGI_FORMAT_R32G32_FLOAT;
        case Accessor::kVec3:   return DXGI_FORMAT_R32G32B32_FLOAT;
        default:                return DXGI_FORMAT_R32G32B32A32_FLOAT;
        }
    default:
        ASSERT("Invalid accessor format");
        return DXGI_FORMAT_UNKNOWN;
    }
}

void OptimizeMesh( Renderer::Primitive& outPrim, const glTF::Primitive& inPrim, const Math::Matrix4& localToObject )
{
    ASSERT(inPrim.attributes[0] != nullptr, "Must have POSITION");
    uint32_t vertexCount = inPrim.attributes[0]->count;

    void* indices = nullptr;
    uint32_t indexCount;
    bool b32BitIndices;
    uint32_t maxIndex = inPrim.maxIndex;

    if (inPrim.indices == nullptr)
    {
        ASSERT(inPrim.mode == 4, "Impossible primitive topology when lacking indices");

        indexCount = vertexCount * 3;
        maxIndex = indexCount - 1;
        if (indexCount > 0xFFFF)
        {
            b32BitIndices = true;
            outPrim.IB = std::make_shared<std::vector<byte>>(4 * indexCount);
            indices = outPrim.IB->data();
            uint32_t* tmp = (uint32_t*)indices;
            for (uint32_t i = 0; i < indexCount; ++i)
                tmp[i] = i;
        }
        else
        {
            b32BitIndices = false;
            outPrim.IB = std::make_shared<std::vector<byte>>(2 * indexCount);
            indices = outPrim.IB->data();
            uint16_t* tmp = (uint16_t*)indices;
            for (uint16_t i = 0; i < indexCount; ++i)
                tmp[i] = i;
        }
    }
    else
    {
        switch (inPrim.mode)
        {
        default:
        case 0: // POINT LIST
        case 1: // LINE LIST
        case 2: // LINE LOOP
        case 3: // LINE STRIP
            Utility::Printf("Found unsupported primitive topology\n");
            return;
        case 4: // TRIANGLE LIST
            break;
        case 5: // TODO: Convert TRIANGLE STRIP
        case 6: // TODO: Convert TRIANGLE FAN
            Utility::Printf("Found an index buffer that needs to be converted to a triangle list\n");
            return;
        }

        indices = inPrim.indices->dataPtr;
        indexCount = inPrim.indices->count;
        if (maxIndex == 0)
        {
            if (inPrim.indices->componentType == Accessor::kUnsignedInt)
            {
                uint32_t* ib = (uint32_t*)inPrim.indices->dataPtr;
                for (uint32_t k = 0; k < indexCount; ++k)
                    maxIndex = std::max(ib[k], maxIndex);
            }
            else
            {
                uint16_t* ib = (uint16_t*)inPrim.indices->dataPtr;
                for (uint32_t k = 0; k < indexCount; ++k)
                    maxIndex = std::max<uint32_t>(ib[k], maxIndex);
            }
        }
        b32BitIndices = maxIndex > 0xFFFF;
        uint32_t indexSize = b32BitIndices ? 4 : 2;
        outPrim.IB = std::make_shared<std::vector<byte>>(indexSize * indexCount);
        if (b32BitIndices)
        {
            ASSERT(inPrim.indices->componentType == Accessor::kUnsignedInt);
            OptimizeFaces((uint32_t*)inPrim.indices->dataPtr, inPrim.indices->count, (uint32_t*)outPrim.IB->data(), 64);
        }
        else if (inPrim.indices->componentType == Accessor::kUnsignedShort)
        {
            OptimizeFaces((uint16_t*)inPrim.indices->dataPtr, inPrim.indices->count, (uint16_t*)outPrim.IB->data(), 64);
        }
        else
        {
            OptimizeFaces((uint32_t*)inPrim.indices->dataPtr, inPrim.indices->count, (uint16_t*)outPrim.IB->data(), 64);
        }
        indices = outPrim.IB->data();
    }

    ASSERT(maxIndex > 0);

    const bool HasNormals = inPrim.attributes[glTF::Primitive::kNormal] != nullptr;
    const bool HasTangents = inPrim.attributes[glTF::Primitive::kTangent] != nullptr;
    const bool HasUV0 = inPrim.attributes[glTF::Primitive::kTexcoord0] != nullptr;
    const bool HasUV1 = inPrim.attributes[glTF::Primitive::kTexcoord1] != nullptr;
    const bool HasJoints = inPrim.attributes[glTF::Primitive::kJoints0] != nullptr;
    const bool HasWeights = inPrim.attributes[glTF::Primitive::kWeights0] != nullptr;
    const bool HasSkin = HasJoints && HasWeights;
    
    std::vector<D3D12_INPUT_ELEMENT_DESC> InputElements;
    InputElements.push_back({"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, glTF::Primitive::kPosition});
    if (HasNormals)
    {
        InputElements.push_back({"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, glTF::Primitive::kNormal });
    }
    if (HasTangents)
    {
        InputElements.push_back({"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,  glTF::Primitive::kTangent });
    }
    if (HasUV0)
    {
        InputElements.push_back({ "TEXCOORD", 0,
            AccessorFormat(*inPrim.attributes[glTF::Primitive::kTexcoord0]),
            glTF::Primitive::kTexcoord0 });
    }
    if (HasUV1)
    {
        InputElements.push_back({ "TEXCOORD", 1,
            AccessorFormat(*inPrim.attributes[glTF::Primitive::kTexcoord1]),
            glTF::Primitive::kTexcoord1 });
    }
    if (HasSkin)
    {
        InputElements.push_back({ "BLENDINDICES", 0,
            JointIndexFormat(*inPrim.attributes[glTF::Primitive::kJoints0]),
            glTF::Primitive::kJoints0 });
        InputElements.push_back({ "BLENDWEIGHT", 0,
            AccessorFormat(*inPrim.attributes[glTF::Primitive::kWeights0]), 
            glTF::Primitive::kWeights0 });
    }

    VBReader vbr;
    vbr.Initialize({InputElements.data(), (uint32_t)InputElements.size()});

    for (uint32_t i = 0; i < Primitive::kNumAttribs; ++i)
    {
        Accessor* attrib = inPrim.attributes[i];
        if (attrib)
            vbr.AddStream(attrib->dataPtr, vertexCount, i, attrib->stride);
    }

    const glTF::Material& material = *inPrim.material;

    std::unique_ptr<XMFLOAT3[]> position;
    std::unique_ptr<XMFLOAT3[]> normal;
    std::unique_ptr<XMFLOAT4[]> tangent;
    std::unique_ptr<XMFLOAT2[]> texcoord0;
    std::unique_ptr<XMFLOAT2[]> texcoord1;
    std::unique_ptr<XMFLOAT4[]> joints;
    std::unique_ptr<XMFLOAT4[]> weights;
    position.reset(new XMFLOAT3[vertexCount]);
    normal.reset(new XMFLOAT3[vertexCount]);

    ASSERT_SUCCEEDED(vbr.Read(position.get(), "POSITION", 0, vertexCount));
    {
        // Local space bounds
        Vector3 sphereCenterLS = (Vector3(*(XMFLOAT3*)inPrim.minPos) + Vector3(*(XMFLOAT3*)inPrim.maxPos)) * 0.5f;
        Scalar maxRadiusLSSq(kZero);

        // Object space bounds
        // (This would be expressed better with an AffineTransform * Vector3)
        Vector3 sphereCenterOS = Vector3(localToObject * Vector4(sphereCenterLS));
        Scalar maxRadiusOSSq(kZero);

        outPrim.m_BBoxLS = AxisAlignedBox(kZero);
        outPrim.m_BBoxOS = AxisAlignedBox(kZero);

        for (uint32_t v = 0; v < vertexCount/*maxIndex*/; ++v)
        {
            Vector3 positionLS = Vector3(position[v]);
            maxRadiusLSSq = Max(maxRadiusLSSq, LengthSquare(sphereCenterLS - positionLS));

            outPrim.m_BBoxLS.AddPoint(positionLS);

            Vector3 positionOS = Vector3(localToObject * Vector4(positionLS));
            maxRadiusOSSq = Max(maxRadiusOSSq, LengthSquare(sphereCenterOS - positionOS));

            outPrim.m_BBoxOS.AddPoint(positionOS);
        }

        outPrim.m_BoundsLS = Math::BoundingSphere(sphereCenterLS, Sqrt(maxRadiusLSSq));
        outPrim.m_BoundsOS = Math::BoundingSphere(sphereCenterOS, Sqrt(maxRadiusOSSq));
        ASSERT(outPrim.m_BoundsOS.GetRadius() > 0.0f);
    }

    if (HasNormals)
    {
        ASSERT_SUCCEEDED(vbr.Read(normal.get(), "NORMAL", 0, vertexCount));
    }
    else
    {
        const size_t faceCount = indexCount / 3;

        if (b32BitIndices)
            ComputeNormals((const uint32_t*)indices, faceCount, position.get(), vertexCount, CNORM_DEFAULT, normal.get());
        else
            ComputeNormals((const uint16_t*)indices, faceCount, position.get(), vertexCount, CNORM_DEFAULT, normal.get());
    }

    if (HasUV0)
    {
        texcoord0.reset(new XMFLOAT2[vertexCount]);
        ASSERT_SUCCEEDED(vbr.Read(texcoord0.get(), "TEXCOORD", 0, vertexCount));
    }

    if (HasUV1)
    {
        texcoord1.reset(new XMFLOAT2[vertexCount]);
        ASSERT_SUCCEEDED(vbr.Read(texcoord1.get(), "TEXCOORD", 1, vertexCount));
    }

    if (HasTangents)
    {
        tangent.reset(new XMFLOAT4[vertexCount]);
        ASSERT_SUCCEEDED(vbr.Read(tangent.get(), "TANGENT", 0, vertexCount));
    }
    else
    {
        ASSERT(maxIndex < vertexCount);
        ASSERT(indexCount % 3 == 0);

        HRESULT hr = S_OK;

        if (HasUV0 && material.normalUV == 0)
        {
            tangent.reset(new XMFLOAT4[vertexCount]);
            if (b32BitIndices)
            {
                hr = ComputeTangentFrame((uint32_t*)indices, indexCount / 3, position.get(), normal.get(), texcoord0.get(),
                    vertexCount, tangent.get());
            }
            else
            {
                hr = ComputeTangentFrame((uint16_t*)indices, indexCount / 3, position.get(), normal.get(), texcoord0.get(),
                    vertexCount, tangent.get());
            }
        }
        else if (HasUV1 && material.normalUV == 1)
        {
            tangent.reset(new XMFLOAT4[vertexCount]);
            if (b32BitIndices)
            {
                hr = ComputeTangentFrame((uint32_t*)indices, indexCount / 3, position.get(), normal.get(), texcoord1.get(),
                    vertexCount, tangent.get());
            }
            else
            {
                hr = ComputeTangentFrame((uint16_t*)indices, indexCount / 3, position.get(), normal.get(), texcoord1.get(),
                    vertexCount, tangent.get());
            }
        }

        ASSERT_SUCCEEDED(hr, "Error generating a tangent frame");
    }

    if (HasSkin)
    {
        joints.reset(new XMFLOAT4[vertexCount]);
        weights.reset(new XMFLOAT4[vertexCount]);
        ASSERT_SUCCEEDED(vbr.Read(joints.get(), "BLENDINDICES", 0, vertexCount));
        ASSERT_SUCCEEDED(vbr.Read(weights.get(), "BLENDWEIGHT", 0, vertexCount));
    }

    // Use VBWriter to generate a new, interleaved and compressed vertex buffer
    std::vector<D3D12_INPUT_ELEMENT_DESC> OutputElements;

    outPrim.psoFlags = PSOFlags::kHasPosition | PSOFlags::kHasNormal;
    OutputElements.push_back({"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT});
    OutputElements.push_back({"NORMAL", 0, DXGI_FORMAT_R10G10B10A2_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT});
    if (tangent.get())
    {
        OutputElements.push_back({"TANGENT", 0, DXGI_FORMAT_R10G10B10A2_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT});
        outPrim.psoFlags |= PSOFlags::kHasTangent;
    }
    if (texcoord0.get())
    {
        OutputElements.push_back({"TEXCOORD", 0, DXGI_FORMAT_R16G16_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT});
        outPrim.psoFlags |= PSOFlags::kHasUV0;
    }
    if (texcoord1.get())
    {
        OutputElements.push_back({"TEXCOORD", 1, DXGI_FORMAT_R16G16_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT});
        outPrim.psoFlags |= PSOFlags::kHasUV1;
    }
    if (HasSkin)
    {
        OutputElements.push_back({ "BLENDINDICES", 0, DXGI_FORMAT_R16G16B16A16_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT });
        OutputElements.push_back({ "BLENDWEIGHT", 0, DXGI_FORMAT_R16G16B16A16_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT });
        outPrim.psoFlags |= PSOFlags::kHasSkin;
    }
    if (material.alphaBlend)
        outPrim.psoFlags |= PSOFlags::kAlphaBlend;
    if (material.alphaTest)
        outPrim.psoFlags |= PSOFlags::kAlphaTest;
    if (material.twoSided)
        outPrim.psoFlags |= PSOFlags::kTwoSided;

    D3D12_INPUT_LAYOUT_DESC layout = {OutputElements.data(), (uint32_t)OutputElements.size()};

    VBWriter vbw;
    vbw.Initialize(layout);

    uint32_t offsets[10];
    uint32_t strides[D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
    ComputeInputLayout(layout, offsets, strides);
    uint32_t stride = strides[0];

    outPrim.VB = std::make_shared<std::vector<byte>>(stride * vertexCount);
    ASSERT_SUCCEEDED(vbw.AddStream(outPrim.VB->data(), vertexCount, 0, stride));

    vbw.Write( position.get(), "POSITION", 0, vertexCount );
    vbw.Write( normal.get(), "NORMAL", 0, vertexCount, true );
    if (tangent.get())
        vbw.Write( tangent.get(), "TANGENT", 0, vertexCount, true );
    if (texcoord0.get())
        vbw.Write( texcoord0.get(), "TEXCOORD", 0, vertexCount );
    if (texcoord1.get())
        vbw.Write( texcoord1.get(), "TEXCOORD", 1, vertexCount );
    if (HasSkin)
    {
        vbw.Write(joints.get(), "BLENDINDICES", 0, vertexCount);
        vbw.Write(weights.get(), "BLENDWEIGHT", 0, vertexCount);
    }

    // Now write a VB for positions only (or positions and UV when alpha testing)
    uint32_t depthStride = 12;
    std::vector<D3D12_INPUT_ELEMENT_DESC> DepthElements;
    DepthElements.push_back({"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT});
    if (material.alphaTest)
    {
        depthStride += 4;
        DepthElements.push_back({"TEXCOORD", 0, DXGI_FORMAT_R16G16_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT});
    }
    if (HasSkin)
    {
        depthStride += 16;
        DepthElements.push_back({ "BLENDINDICES", 0, DXGI_FORMAT_R16G16B16A16_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT });
        DepthElements.push_back({ "BLENDWEIGHT", 0, DXGI_FORMAT_R16G16B16A16_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT });
    }

    VBWriter dvbw;
    dvbw.Initialize({DepthElements.data(), (uint32_t)DepthElements.size()});

    outPrim.DepthVB = std::make_shared<std::vector<byte>>(depthStride * vertexCount);
    ASSERT_SUCCEEDED(dvbw.AddStream(outPrim.DepthVB->data(), vertexCount, 0, depthStride));

    dvbw.Write( position.get(), "POSITION", 0, vertexCount );
    if (material.alphaTest)
    {
        dvbw.Write(material.baseColorUV ? texcoord1.get() : texcoord0.get(), "TEXCOORD", 0, vertexCount);
    }
    if (HasSkin)
    {
        dvbw.Write(joints.get(), "BLENDINDICES", 0, vertexCount);
        dvbw.Write(weights.get(), "BLENDWEIGHT", 0, vertexCount);
    }

    ASSERT(material.index < 0x8000, "Only 15-bit material indices allowed");

    outPrim.vertexStride = (uint16_t)stride;
    outPrim.index32 = b32BitIndices ? 1 : 0;
    outPrim.materialIdx = material.index;

    outPrim.primCount = indexCount;

    // TODO:  Generate optimized depth-only streams
}

