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
// Author(s):  Alex Nankervis
//             James Stanard
//

#include "LightManager.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "CommandContext.h"
#include "Camera.h"
#include "BufferManager.h"
#include "TemporalEffects.h"

#include "CompiledShaders/FillLightGridCS_8.h"
#include "CompiledShaders/FillLightGridCS_16.h"
#include "CompiledShaders/FillLightGridCS_24.h"
#include "CompiledShaders/FillLightGridCS_32.h"

using namespace Math;
using namespace Graphics;

// must keep in sync with HLSL
struct LightData
{
    float pos[3];
    float radiusSq;
    float color[3];

    uint32_t type;
    float coneDir[3];
    float coneAngles[2];

    float shadowTextureMatrix[16];
};

enum { kMinLightGridDim = 8 };

namespace Lighting
{
    IntVar LightGridDim("Application/Forward+/Light Grid Dim", 16, kMinLightGridDim, 32, 8 );

    RootSignature m_FillLightRootSig;
    ComputePSO m_FillLightGridCS_8(L"Fill Light Grid 8 CS");
    ComputePSO m_FillLightGridCS_16(L"Fill Light Grid 16 CS");
    ComputePSO m_FillLightGridCS_24(L"Fill Light Grid 24 CS");
    ComputePSO m_FillLightGridCS_32(L"Fill Light Grid 32 CS");

    LightData m_LightData[MaxLights];
    StructuredBuffer m_LightBuffer;
    ByteAddressBuffer m_LightGrid;

    ByteAddressBuffer m_LightGridBitMask;
    uint32_t m_FirstConeLight;
    uint32_t m_FirstConeShadowedLight;

    enum {shadowDim = 512};
    ColorBuffer m_LightShadowArray;
    ShadowBuffer m_LightShadowTempBuffer;
    Matrix4 m_LightShadowMatrix[MaxLights];

    void InitializeResources(void);
    void CreateRandomLights(const Vector3 minBound, const Vector3 maxBound);
    void FillLightGrid(GraphicsContext& gfxContext, const Camera& camera);
    void Shutdown(void);
}

void Lighting::InitializeResources( void )
{
    m_FillLightRootSig.Reset(3, 0);
    m_FillLightRootSig[0].InitAsConstantBuffer(0);
    m_FillLightRootSig[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 2);
    m_FillLightRootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 2);
    m_FillLightRootSig.Finalize(L"FillLightRS");

    m_FillLightGridCS_8.SetRootSignature(m_FillLightRootSig);
    m_FillLightGridCS_8.SetComputeShader(g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8));
    m_FillLightGridCS_8.Finalize();

    m_FillLightGridCS_16.SetRootSignature(m_FillLightRootSig);
    m_FillLightGridCS_16.SetComputeShader(g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16));
    m_FillLightGridCS_16.Finalize();

    m_FillLightGridCS_24.SetRootSignature(m_FillLightRootSig);
    m_FillLightGridCS_24.SetComputeShader(g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24));
    m_FillLightGridCS_24.Finalize();

    m_FillLightGridCS_32.SetRootSignature(m_FillLightRootSig);
    m_FillLightGridCS_32.SetComputeShader(g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32));
    m_FillLightGridCS_32.Finalize();

    // Assumes max resolution of 3840x2160
    uint32_t lightGridCells = Math::DivideByMultiple(3840, kMinLightGridDim) * Math::DivideByMultiple(2160, kMinLightGridDim);
    uint32_t lightGridSizeBytes = lightGridCells * (4 + MaxLights * 4);
    m_LightGrid.Create(L"m_LightGrid", lightGridSizeBytes, 1);

    uint32_t lightGridBitMaskSizeBytes = lightGridCells * 4 * 4;
    m_LightGridBitMask.Create(L"m_LightGridBitMask", lightGridBitMaskSizeBytes, 1);

    m_LightShadowArray.CreateArray(L"m_LightShadowArray", shadowDim, shadowDim, MaxLights, DXGI_FORMAT_R16_UNORM);
    m_LightShadowTempBuffer.Create(L"m_LightShadowTempBuffer", shadowDim, shadowDim);

    m_LightBuffer.Create(L"m_LightBuffer", MaxLights, sizeof(LightData));
}

void Lighting::CreateRandomLights( const Vector3 minBound, const Vector3 maxBound )
{
    Vector3 posScale = maxBound - minBound;
    Vector3 posBias = minBound;

    // todo: replace this with MT
    srand(12645);
    auto randUint = []() -> uint32_t
    {
        return rand(); // [0, RAND_MAX]
    };
    auto randFloat = [randUint]() -> float
    {
        return randUint() * (1.0f / RAND_MAX); // convert [0, RAND_MAX] to [0, 1]
    };
    auto randVecUniform = [randFloat]() -> Vector3
    {
        return Vector3(randFloat(), randFloat(), randFloat());
    };
    auto randGaussian = [randFloat]() -> float
    {
        // polar box-muller
        static bool gaussianPair = true;
        static float y2;

        if (gaussianPair)
        {
            gaussianPair = false;

            float x1, x2, w;
            do
            {
                x1 = 2 * randFloat() - 1;
                x2 = 2 * randFloat() - 1;
                w = x1 * x1 + x2 * x2;
            } while (w >= 1);

            w = sqrtf(-2 * logf(w) / w);
            y2 = x2 * w;
            return x1 * w;
        }
        else
        {
            gaussianPair = true;
            return y2;
        }
    };
    auto randVecGaussian = [randGaussian]() -> Vector3
    {
        return Normalize(Vector3(randGaussian(), randGaussian(), randGaussian()));
    };

    const float pi = 3.14159265359f;
    for (uint32_t n = 0; n < MaxLights; n++)
    {
        Vector3 pos = randVecUniform() * posScale + posBias;
        float lightRadius = randFloat() * 800.0f + 200.0f;

        Vector3 color = randVecUniform();
        float colorScale = randFloat() * .3f + .3f;
        color = color * colorScale;

        uint32_t type;
        // force types to match 32-bit boundaries for the BIT_MASK_SORTED case
        if (n < 32 * 1)
            type = 0;
        else if (n < 32 * 3)
            type = 1;
        else
            type = 2;

        Vector3 coneDir = randVecGaussian();
        float coneInner = (randFloat() * .2f + .025f) * pi;
        float coneOuter = coneInner + randFloat() * .1f * pi;

        if (type == 1 || type == 2)
        {
            // emphasize cone lights
            color = color * 5.0f;
        }

        Math::Camera shadowCamera;
        shadowCamera.SetEyeAtUp(pos, pos + coneDir, Vector3(0, 1, 0));
        shadowCamera.SetPerspectiveMatrix(coneOuter * 2, 1.0f, lightRadius * .05f, lightRadius * 1.0f);
        shadowCamera.Update();
        m_LightShadowMatrix[n] = shadowCamera.GetViewProjMatrix();
        Matrix4 shadowTextureMatrix = Matrix4(AffineTransform(Matrix3::MakeScale( 0.5f, -0.5f, 1.0f ), Vector3(0.5f, 0.5f, 0.0f))) * m_LightShadowMatrix[n];

        m_LightData[n].pos[0] = pos.GetX();
        m_LightData[n].pos[1] = pos.GetY();
        m_LightData[n].pos[2] = pos.GetZ();
        m_LightData[n].radiusSq = lightRadius * lightRadius;
        m_LightData[n].color[0] = color.GetX();
        m_LightData[n].color[1] = color.GetY();
        m_LightData[n].color[2] = color.GetZ();
        m_LightData[n].type = type;
        m_LightData[n].coneDir[0] = coneDir.GetX();
        m_LightData[n].coneDir[1] = coneDir.GetY();
        m_LightData[n].coneDir[2] = coneDir.GetZ();
        m_LightData[n].coneAngles[0] = 1.0f / (cosf(coneInner) - cosf(coneOuter));
        m_LightData[n].coneAngles[1] = cosf(coneOuter);
        std::memcpy(m_LightData[n].shadowTextureMatrix, &shadowTextureMatrix, sizeof(shadowTextureMatrix));
        //*(Matrix4*)(m_LightData[n].shadowTextureMatrix) = shadowTextureMatrix;
    }
    // sort lights by type, needed for efficiency in the BIT_MASK approach
    /*	{
    Matrix4 copyLightShadowMatrix[MaxLights];
    memcpy(copyLightShadowMatrix, m_LightShadowMatrix, sizeof(Matrix4) * MaxLights);
    LightData copyLightData[MaxLights];
    memcpy(copyLightData, m_LightData, sizeof(LightData) * MaxLights);

    uint32_t sortArray[MaxLights];
    for (uint32_t n = 0; n < MaxLights; n++)
    {
    sortArray[n] = n;
    }
    std::sort(sortArray, sortArray + MaxLights,
    [this](const uint32_t &a, const uint32_t &b) -> bool
    {
    return this->m_LightData[a].type < this->m_LightData[b].type;
    });
    for (uint32_t n = 0; n < MaxLights; n++)
    {
    m_LightShadowMatrix[n] = copyLightShadowMatrix[sortArray[n]];
    m_LightData[n] = copyLightData[sortArray[n]];
    }
    }*/
    for (uint32_t n = 0; n < MaxLights; n++)
    {
        if (m_LightData[n].type == 1)
        {
            m_FirstConeLight = n;
            break;
        }
    }
    for (uint32_t n = 0; n < MaxLights; n++)
    {
        if (m_LightData[n].type == 2)
        {
            m_FirstConeShadowedLight = n;
            break;
        }
    }

    CommandContext::InitializeBuffer(m_LightBuffer, m_LightData, MaxLights * sizeof(LightData));
}

void Lighting::Shutdown(void)
{
    m_LightBuffer.Destroy();
    m_LightGrid.Destroy();
    m_LightGridBitMask.Destroy();
    m_LightShadowArray.Destroy();
    m_LightShadowTempBuffer.Destroy();
}

void Lighting::FillLightGrid(GraphicsContext& gfxContext, const Camera& camera)
{
    ScopedTimer _prof(L"FillLightGrid", gfxContext);

    ComputeContext& Context = gfxContext.GetComputeContext();

    Context.SetRootSignature(m_FillLightRootSig);

    switch ((int)LightGridDim)
    {
    case  8: Context.SetPipelineState(m_FillLightGridCS_8 ); break;
    case 16: Context.SetPipelineState(m_FillLightGridCS_16); break;
    case 24: Context.SetPipelineState(m_FillLightGridCS_24); break;
    case 32: Context.SetPipelineState(m_FillLightGridCS_32); break;
    default: ASSERT(false); break;
    }

    ColorBuffer& LinearDepth = g_LinearDepth[ TemporalEffects::GetFrameIndexMod2() ];

    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightGrid, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(m_LightGridBitMask, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    Context.SetDynamicDescriptor(1, 0, m_LightBuffer.GetSRV());
    Context.SetDynamicDescriptor(1, 1, LinearDepth.GetSRV());
    //Context.SetDynamicDescriptor(1, 1, g_SceneDepthBuffer.GetDepthSRV());
    Context.SetDynamicDescriptor(2, 0, m_LightGrid.GetUAV());
    Context.SetDynamicDescriptor(2, 1, m_LightGridBitMask.GetUAV());

    // todo: assumes 1920x1080 resolution
    uint32_t tileCountX = Math::DivideByMultiple(g_SceneColorBuffer.GetWidth(), LightGridDim);
    uint32_t tileCountY = Math::DivideByMultiple(g_SceneColorBuffer.GetHeight(), LightGridDim);

    float FarClipDist = camera.GetFarClip();
    float NearClipDist = camera.GetNearClip();
    const float RcpZMagic = NearClipDist / (FarClipDist - NearClipDist);

    struct CSConstants
    {
        uint32_t ViewportWidth, ViewportHeight;
        float InvTileDim;
        float RcpZMagic;
        uint32_t TileCount;
        Matrix4 ViewProjMatrix;
    } csConstants;
    // todo: assumes 1920x1080 resolution
    csConstants.ViewportWidth = g_SceneColorBuffer.GetWidth();
    csConstants.ViewportHeight = g_SceneColorBuffer.GetHeight();
    csConstants.InvTileDim = 1.0f / LightGridDim;
    csConstants.RcpZMagic = RcpZMagic;
    csConstants.TileCount = tileCountX;
    csConstants.ViewProjMatrix = camera.GetViewProjMatrix();
    Context.SetDynamicConstantBufferView(0, sizeof(CSConstants), &csConstants);

    Context.Dispatch(tileCountX, tileCountY, 1);

    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightGrid, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightGridBitMask, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}
