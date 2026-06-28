#include "stdafx.h"

#include "ProceduralSceneBuilder.h"

#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <DirectXMathVector.inl>
#include <cmath>
#include <cstdint>

using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;
using DirectX::XMFLOAT4X4;
using DirectX::XMLoadFloat3;
using DirectX::XMStoreFloat3;
using DirectX::XMStoreFloat4x4;
using DirectX::XMMATRIX;
using DirectX::XMMatrixIdentity;
using DirectX::XMMatrixInverse;
using DirectX::XMMatrixRotationRollPitchYaw;
using DirectX::XMMatrixScaling;
using DirectX::XMMatrixTranslation;
using DirectX::XMMatrixTranspose;
using DirectX::XMVector3Normalize;
using DirectX::XMVector3Transform;
using DirectX::XMVector3TransformNormal;
using DirectX::XMVectorSet;

namespace Engine
{
namespace Procedural
{

namespace
{

static constexpr float kPI = 3.141592f;

} // namespace

SceneMesh CreateQuadMesh(
    const XMFLOAT3& v0,
    const XMFLOAT3& v1,
    const XMFLOAT3& v2,
    const XMFLOAT3& v3,
    const XMFLOAT3& normal)
{
    SceneMesh mesh = {};

    mesh.vertices.resize(4);
    mesh.vertices[0] = {v0, XMFLOAT2(0.0f, 1.0f), normal};
    mesh.vertices[1] = {v1, XMFLOAT2(1.0f, 1.0f), normal};
    mesh.vertices[2] = {v2, XMFLOAT2(1.0f, 0.0f), normal};
    mesh.vertices[3] = {v3, XMFLOAT2(0.0f, 0.0f), normal};

    mesh.indices = {0, 2, 1, 0, 3, 2};

    return mesh;
}

SceneMesh CreateCubeMesh(float size)
{
    SceneMesh mesh = {};
    const float h = size * 0.5f;

    mesh.vertices.resize(24);
    mesh.indices.resize(36);

    uint32_t vi = 0;
    uint32_t ii = 0;

    const auto addFace = [&](const XMFLOAT3& normal,
                             const XMFLOAT3& v0,
                             const XMFLOAT3& v1,
                             const XMFLOAT3& v2,
                             const XMFLOAT3& v3,
                             bool flipWinding = false)
    {
        const uint32_t base = vi;
        mesh.vertices[vi++] = {v0, {0.0f, 1.0f}, normal};
        mesh.vertices[vi++] = {v1, {1.0f, 1.0f}, normal};
        mesh.vertices[vi++] = {v2, {1.0f, 0.0f}, normal};
        mesh.vertices[vi++] = {v3, {0.0f, 0.0f}, normal};

        if (flipWinding)
        {
            mesh.indices[ii++] = base + 0;
            mesh.indices[ii++] = base + 1;
            mesh.indices[ii++] = base + 2;
            mesh.indices[ii++] = base + 0;
            mesh.indices[ii++] = base + 2;
            mesh.indices[ii++] = base + 3;
        }
        else
        {
            mesh.indices[ii++] = base + 0;
            mesh.indices[ii++] = base + 2;
            mesh.indices[ii++] = base + 1;
            mesh.indices[ii++] = base + 0;
            mesh.indices[ii++] = base + 3;
            mesh.indices[ii++] = base + 2;
        }
    };

    addFace({1.0f, 0.0f, 0.0f}, {h, -h, -h}, {h, -h, h}, {h, h, h}, {h, h, -h});
    addFace({-1.0f, 0.0f, 0.0f}, {-h, -h, h}, {-h, -h, -h}, {-h, h, -h}, {-h, h, h});
    addFace({0.0f, 1.0f, 0.0f}, {-h, h, -h}, {h, h, -h}, {h, h, h}, {-h, h, h});
    addFace({0.0f, -1.0f, 0.0f}, {-h, -h, h}, {h, -h, h}, {h, -h, -h}, {-h, -h, -h});
    addFace({0.0f, 0.0f, 1.0f}, {-h, -h, h}, {h, -h, h}, {h, h, h}, {-h, h, h}, true);
    addFace({0.0f, 0.0f, -1.0f}, {h, -h, -h}, {-h, -h, -h}, {-h, h, -h}, {h, h, -h}, true);

    return mesh;
}

SceneMesh CreateSphereMesh(float radius, int stackCount, int sliceCount)
{
    SceneMesh mesh = {};

    const int vertexCount = (stackCount + 1) * (sliceCount + 1);
    mesh.vertices.resize(vertexCount);

    int vi = 0;
    for (int stack = 0; stack <= stackCount; stack++)
    {
        const float v = static_cast<float>(stack) / static_cast<float>(stackCount);
        const float theta = v * kPI;
        const float sinTheta = std::sin(theta);
        const float cosTheta = std::cos(theta);

        for (int slice = 0; slice <= sliceCount; slice++)
        {
            const float u = static_cast<float>(slice) / static_cast<float>(sliceCount);
            const float phi = u * 2.0f * kPI;
            const float sinPhi = std::sin(phi);
            const float cosPhi = std::cos(phi);

            const XMFLOAT3 normal = {sinTheta * cosPhi, cosTheta, sinTheta * sinPhi};
            const XMFLOAT3 position = {normal.x * radius, normal.y * radius, normal.z * radius};
            mesh.vertices[vi] = {position, XMFLOAT2(u, v), normal};
            vi++;
        }
    }

    const int stride = sliceCount + 1;
    for (int stack = 0; stack < stackCount; stack++)
    {
        for (int slice = 0; slice < sliceCount; slice++)
        {
            const uint32_t a = static_cast<uint32_t>(stack * stride + slice);
            const uint32_t b = static_cast<uint32_t>((stack + 1) * stride + slice);
            const uint32_t c = static_cast<uint32_t>((stack + 1) * stride + slice + 1);
            const uint32_t d = static_cast<uint32_t>(stack * stride + slice + 1);

            mesh.indices.push_back(a);
            mesh.indices.push_back(d);
            mesh.indices.push_back(b);
            mesh.indices.push_back(d);
            mesh.indices.push_back(c);
            mesh.indices.push_back(b);
        }
    }

    return mesh;
}

int AddSolidColorTexture(SceneMesh& mesh, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    SceneTexture texture = {};
    texture.width = 1;
    texture.height = 1;
    texture.component = 4;
    texture.pixels = {r, g, b, a};
    mesh.textures.push_back(std::move(texture));
    return static_cast<int>(mesh.textures.size() - 1);
}

void AppendMesh(SceneMesh& dest, const SceneMesh& src, uint32_t materialId)
{
    const uint32_t baseVertex = static_cast<uint32_t>(dest.vertices.size());
    dest.vertices.reserve(dest.vertices.size() + src.vertices.size());
    for (const auto& v : src.vertices)
    {
        SceneVertex newV = v;
        newV.materialId = materialId;
        dest.vertices.push_back(newV);
    }
    for (uint32_t idx : src.indices)
    {
        dest.indices.push_back(baseVertex + idx);
    }
}

void AppendTransformedMesh(SceneMesh& dest, const SceneMesh& src,
                           DirectX::FXMMATRIX transform, uint32_t materialId)
{
    const uint32_t baseVertex = static_cast<uint32_t>(dest.vertices.size());
    const size_t srcVertexCount = src.vertices.size();
    dest.vertices.reserve(dest.vertices.size() + srcVertexCount);

    XMMATRIX normalMatrix = transform;
    normalMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    XMVECTOR det;
    normalMatrix = XMMatrixInverse(&det, normalMatrix);
    if (XMVectorGetX(det) != 0.0f)
    {
        normalMatrix = XMMatrixTranspose(normalMatrix);
    }
    else
    {
        normalMatrix = XMMatrixIdentity();
    }

    for (size_t i = 0; i < srcVertexCount; i++)
    {
        SceneVertex v = src.vertices[i];
        v.materialId = materialId;

        XMVECTOR pos = XMLoadFloat3(&v.position);
        XMVECTOR nrm = XMLoadFloat3(&v.normal);

        pos = XMVector3Transform(pos, transform);
        nrm = XMVector3TransformNormal(nrm, normalMatrix);
        nrm = XMVector3Normalize(nrm);

        XMStoreFloat3(&v.position, pos);
        XMStoreFloat3(&v.normal, nrm);

        dest.vertices.push_back(v);
    }

    for (uint32_t idx : src.indices)
    {
        dest.indices.push_back(baseVertex + idx);
    }
}

void AddQuad(SceneMesh& mesh,
             const XMFLOAT3& center,
             const XMFLOAT3& size,
             const XMFLOAT3& normal,
             uint32_t materialId)
{
    XMFLOAT3 axis1, axis2;

    // Choose the two axes perpendicular to the normal
    const float nx = std::abs(normal.x);
    const float ny = std::abs(normal.y);
    const float nz = std::abs(normal.z);

    if (nx >= ny && nx >= nz)
    {
        // Normal is primarily X -> quad in YZ plane
        axis1 = {0.0f, 1.0f, 0.0f};
        axis2 = {0.0f, 0.0f, 1.0f};
    }
    else if (ny >= nx && ny >= nz)
    {
        // Normal is primarily Y -> quad in XZ plane
        axis1 = {1.0f, 0.0f, 0.0f};
        axis2 = {0.0f, 0.0f, 1.0f};
    }
    else
    {
        // Normal is primarily Z -> quad in XY plane
        axis1 = {1.0f, 0.0f, 0.0f};
        axis2 = {0.0f, 1.0f, 0.0f};
    }

    const float h1x = axis1.x * size.x * 0.5f;
    const float h1y = axis1.y * size.y * 0.5f;
    const float h1z = axis1.z * size.z * 0.5f;
    const float h2x = axis2.x * size.x * 0.5f;
    const float h2y = axis2.y * size.y * 0.5f;
    const float h2z = axis2.z * size.z * 0.5f;

    const float cx = center.x;
    const float cy = center.y;
    const float cz = center.z;

    const XMFLOAT3 v0 = {cx - h1x - h2x, cy - h1y - h2y, cz - h1z - h2z};
    const XMFLOAT3 v1 = {cx + h1x - h2x, cy + h1y - h2y, cz + h1z - h2z};
    const XMFLOAT3 v2 = {cx + h1x + h2x, cy + h1y + h2y, cz + h1z + h2z};
    const XMFLOAT3 v3 = {cx - h1x + h2x, cy - h1y + h2y, cz - h1z + h2z};

    SceneMesh quad = CreateQuadMesh(v0, v1, v2, v3, normal);
    AppendMesh(mesh, quad, materialId);
}

void AddSphere(SceneMesh& mesh,
               const XMFLOAT3& position,
               float radius,
               uint32_t materialId)
{
    SceneMesh sphere = CreateSphereMesh(radius, 16, 32);

    XMMATRIX t = XMMatrixTranslation(position.x, position.y, position.z);
    AppendTransformedMesh(mesh, sphere, t, materialId);
}

} // namespace Procedural
} // namespace Engine
