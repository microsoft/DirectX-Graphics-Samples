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

#include "pch.h"
#include "BoundingSphere.h"

using namespace Math;

BoundingSphere BoundingSphere::Union( const BoundingSphere& rhs )
{
    float radA = GetRadius();
    if (radA == 0.0f)
        return rhs;

    float radB = rhs.GetRadius();
    if (radB == 0.0f)
        return *this;

    Vector3 diff = GetCenter() - rhs.GetCenter();
    float dist = Length(diff);

    // Safe normalize vector between sphere centers
    diff = dist < 1e-6f ? Vector3(kXUnitVector) : diff * Recip(dist);

    Vector3 extremeA = GetCenter() + diff * Max(radA, radB - dist);
    Vector3 extremeB = rhs.GetCenter() - diff * Max(radB, radA - dist);

    return BoundingSphere((extremeA + extremeB) * 0.5f, Length(extremeA - extremeB) * 0.5f);
}
