#pragma once

#include "Scene.h"

#include <DirectXMath.h>
#include <cstdint>

namespace Engine
{
namespace Procedural
{

SceneMesh CreateQuadMesh(
    const DirectX::XMFLOAT3& v0,
    const DirectX::XMFLOAT3& v1,
    const DirectX::XMFLOAT3& v2,
    const DirectX::XMFLOAT3& v3,
    const DirectX::XMFLOAT3& normal);

SceneMesh CreateCubeMesh(float size);

SceneMesh CreateSphereMesh(float radius, int stackCount, int sliceCount);

int AddSolidColorTexture(SceneMesh& mesh, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

void AppendMesh(SceneMesh& dest, const SceneMesh& src, uint32_t materialId);

void AppendTransformedMesh(SceneMesh& dest, const SceneMesh& src,
                           DirectX::FXMMATRIX transform, uint32_t materialId);

void AddQuad(SceneMesh& mesh,
             const DirectX::XMFLOAT3& center,
             const DirectX::XMFLOAT3& size,
             const DirectX::XMFLOAT3& normal,
             uint32_t materialId);

void AddSphere(SceneMesh& mesh,
               const DirectX::XMFLOAT3& position,
               float radius,
               uint32_t materialId);

} // namespace Procedural
} // namespace Engine
