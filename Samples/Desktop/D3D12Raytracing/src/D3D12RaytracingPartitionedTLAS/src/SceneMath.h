//*********************************************************
//
// Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include <SimpleMath.h>
#include <cmath>

namespace scene
{
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

// Scalar helpers for repeatable scene generation and initial velocities.
// Small changes in these calculations can alter the later collision chains.
inline float Length(const Vector2& v)
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}
inline float Length(const Vector3& v)
{
    return std::sqrt((v.x * v.x + v.y * v.y) + v.z * v.z);
}
inline Vector2 Normalize(const Vector2& v)
{
    return v * (1.0f / Length(v));
}
inline Vector3 Normalize(const Vector3& v)
{
    return v * (1.0f / Length(v));
}
inline Vector3 Cross(const Vector3& a, const Vector3& b)
{
    return {a.y * b.z - b.y * a.z, a.z * b.x - b.z * a.x, a.x * b.y - b.x * a.y};
}

// Catmull-Rom interpolation follows GLM's glm/gtx/spline.inl implementation.
// Copyright (c) 2005 - G-Truc Creation. Used under GLM's MIT license option.
// See LICENSES/GLM.txt for its copyright and permission notice.
inline Vector3 CatmullRom(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d, float t)
{
    const float t2 = t * t;
    const float t3 = t * t * t;
    const float f1 = -t3 + 2.0f * t2 - t;
    const float f2 = 3.0f * t3 - 5.0f * t2 + 2.0f;
    const float f3 = -3.0f * t3 + 4.0f * t2 + t;
    const float f4 = t3 - t2;
    return (f1 * a + f2 * b + f3 * c + f4 * d) * 0.5f;
}

// DirectXMath uses row vectors. DXR's 3x4 instance transform is its transpose.
inline Matrix InstanceTransform(const Vector3& position, const Vector3& scale, float sine, float cosine)
{
    return {cosine * scale.x,
            0,
            sine == 0.0f ? 0.0f : -sine * scale.x,
            0,
            0,
            scale.y,
            0,
            0,
            sine * scale.z,
            0,
            cosine * scale.z,
            0,
            position.x,
            position.y,
            position.z,
            1};
}
} // namespace scene
