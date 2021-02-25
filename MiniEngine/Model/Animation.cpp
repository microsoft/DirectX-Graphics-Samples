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
// Author:   James Stanard
//

#include "Animation.h"
#include "Model.h"
#include "../Core/Utility.h"
#include "../Core/Math/Common.h"
#include "../Core/Math/Quaternion.h"

using Math::Quaternion;
using Math::Vector4;

static inline float ToFloat(const int8_t x) { return Math::Max(x / 127.0f, -1.0f); }
static inline float ToFloat(const uint8_t x) { return x / 255.0f; }
static inline float ToFloat(const int16_t x) { return Math::Max(x / 32767.0f, -1.0f); }
static inline float ToFloat(const uint16_t x) { return x / 65535.0f; }

static inline void Lerp3(float* Dest, const float* Key1, const float* Key2, float T)
{
    Dest[0] = Math::Lerp(Key1[0], Key2[0], T);
    Dest[1] = Math::Lerp(Key1[1], Key2[1], T);
    Dest[2] = Math::Lerp(Key1[2], Key2[2], T);
}

template <typename T>
static inline Quaternion ToQuat(const T* rot)
{
    return (Quaternion)Vector4(ToFloat(rot[0]), ToFloat(rot[1]), ToFloat(rot[2]), ToFloat(rot[3]));
}

static inline Quaternion ToQuat(const float* rot)
{
    return Quaternion(XMLoadFloat4((const XMFLOAT4*)rot));
}

static inline void Slerp(float* Dest, const void* Key1, const void* Key2, float T, uint32_t Format)
{
    switch (Format)
    {
    case AnimationCurve::kSNorm8:
    {
        const int8_t* key1 = (const int8_t*)Key1;
        const int8_t* key2 = (const int8_t*)Key2;
        XMStoreFloat4((XMFLOAT4*)Dest, (FXMVECTOR)Math::Slerp(ToQuat(key1), ToQuat(key2), T));
        break;
    }
    case AnimationCurve::kUNorm8:
    {
        const uint8_t* key1 = (const uint8_t*)Key1;
        const uint8_t* key2 = (const uint8_t*)Key2;
        XMStoreFloat4((XMFLOAT4*)Dest, (FXMVECTOR)Math::Slerp(ToQuat(key1), ToQuat(key2), T));
        break;
    }
    case AnimationCurve::kSNorm16:
    {
        const int16_t* key1 = (const int16_t*)Key1;
        const int16_t* key2 = (const int16_t*)Key2;
        XMStoreFloat4((XMFLOAT4*)Dest, (FXMVECTOR)Math::Slerp(ToQuat(key1), ToQuat(key2), T));
        break;
    }
    case AnimationCurve::kUNorm16:
    {
        const uint16_t* key1 = (const uint16_t*)Key1;
        const uint16_t* key2 = (const uint16_t*)Key2;
        XMStoreFloat4((XMFLOAT4*)Dest, (FXMVECTOR)Math::Slerp(ToQuat(key1), ToQuat(key2), T));
        break;
    }
    case AnimationCurve::kFloat:
    {
        const float* key1 = (const float*)Key1;
        const float* key2 = (const float*)Key2;
        XMStoreFloat4((XMFLOAT4*)Dest, (FXMVECTOR)Math::Slerp(ToQuat(key1), ToQuat(key2), T));
        break;
    }
    default:
        ASSERT(0, "Unexpected animation key frame data format");
        break;
    }
}
void ModelInstance::UpdateAnimations(float deltaTime)
{
    uint32_t NumAnimations = m_Model->m_NumAnimations;
    GraphNode* animGraph = m_AnimGraph.get();

    for (uint32_t i = 0; i < NumAnimations; ++i)
    {
        AnimationState& anim = m_AnimState[i];
        if (anim.state == AnimationState::kStopped)
            continue;

        anim.time += deltaTime;

        const AnimationSet& animation = m_Model->m_Animations[i];

        if (anim.state == AnimationState::kLooping)
        {
            anim.time = fmodf(anim.time, animation.duration);
        }
        else if (anim.time > animation.duration)
        {
            anim.time = 0.0f;
            anim.state = AnimationState::kStopped;
        }

        const AnimationCurve* firstCurve = m_Model->m_CurveData.get() + animation.firstCurve;

        // Update animation nodes
        for (uint32_t j = 0; j < animation.numCurves; ++j)
        {
            const AnimationCurve& curve = firstCurve[j];
            ASSERT(curve.numSegments > 0);

            const float progress = Math::Clamp((anim.time - curve.startTime) * curve.rangeScale, 0.0f, curve.numSegments);
            const uint32_t segment = (uint32_t)progress;
            const float lerpT = progress - (float)segment;

            const size_t stride = curve.keyFrameStride * 4;
            const byte* key1 = m_Model->m_KeyFrameData.get() + curve.keyFrameOffset + stride * segment;
            const byte* key2 = key1 + stride;
            GraphNode& node = animGraph[curve.targetNode];

            switch (curve.targetPath)
            {
            case AnimationCurve::kTranslation:
                ASSERT(curve.keyFrameFormat == AnimationCurve::kFloat);
                Lerp3((float*)&node.xform + 12, (const float*)key1, (const float*)key2, lerpT);
                break;
            case AnimationCurve::kRotation:
                node.staleMatrix = true;
                Slerp((float*)&node.rotation, key1, key2, lerpT, curve.keyFrameFormat);
                break;
            case AnimationCurve::kScale:
                ASSERT(curve.keyFrameFormat == AnimationCurve::kFloat);
                node.staleMatrix = true;
                Lerp3((float*)&node.scale, (const float*)key1, (const float*)key2, lerpT);
                break;
            default:
            case AnimationCurve::kWeights:
                ASSERT(0, "Unhandled blend shape weights in animation");
                break;
            }
        }
    }
}

void ModelInstance::PlayAnimation(uint32_t animIdx, bool loop)
{
    if (animIdx < m_AnimState.size())
        m_AnimState[animIdx].state = loop ? AnimationState::kLooping : AnimationState::kPlaying;
}

void ModelInstance::PauseAnimation(uint32_t animIdx)
{
    if (animIdx < m_AnimState.size())
        m_AnimState[animIdx].state = AnimationState::kStopped;
}

void ModelInstance::ResetAnimation(uint32_t animIdx)
{
    if (animIdx >= m_AnimState.size())
        m_AnimState[animIdx].time = 0.0f;
}

void ModelInstance::StopAnimation(uint32_t animIdx)
{
    if (animIdx >= m_AnimState.size())
    {
        m_AnimState[animIdx].state = AnimationState::kStopped;
        m_AnimState[animIdx].time = 0.0f;
    }
}

void ModelInstance::LoopAllAnimations(void)
{
    for (auto& anim : m_AnimState)
    {
        anim.state = AnimationState::kLooping;
        anim.time = 0.0f;
    }
}