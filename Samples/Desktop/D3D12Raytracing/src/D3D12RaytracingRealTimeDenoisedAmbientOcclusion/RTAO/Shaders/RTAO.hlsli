//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#ifndef RTAO_HLSLI
#define RTAO_HLSLI

namespace RTAO {
    static const float RayHitDistanceOnMiss = 0;
    static const float InvalidAOCoefficientValue = -1;
    bool HasAORayHitAnyGeometry(in float tHit)
    {
        return tHit != RayHitDistanceOnMiss;
    }
}

#endif // RTAO_HLSLI