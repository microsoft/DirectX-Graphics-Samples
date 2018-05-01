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

#pragma once

// ToDo Rename to Globals?
// ToDo Move Shader, hit group names here?

#include "RayTracingHlslCompat.h"

namespace GlobalRootSignature {
    namespace Slot {
        enum Enum {
            OutputView = 0,
            AccelerationStructure,
            SceneConstant,
            AABBattributeBuffer,
            VertexBuffers,
            Count
        };
    }
}

namespace LocalRootSignature {
    namespace Type {
        enum Enum {
            Triangle = 0,
            AABB,
    #if USE_NON_NULL_LOCAL_ROOT_SIG
            Empty,
    #endif
            Count
        };
    }
}

namespace LocalRootSignature {
    namespace Triangle {
        namespace Slot {
            enum Enum {
                MaterialConstant = 0,
                Count
            };
        }
        struct RootArguments {
            PrimitiveConstantBuffer materialCb;
        };
    }
}

namespace LocalRootSignature {
    namespace AABB {
        namespace Slot {
            enum Enum {
                MaterialConstant = 0,
                GeometryIndex,          // ToDo rename GeometryIndexSlot
                Count
            };
        }
        struct RootArguments {
            PrimitiveConstantBuffer materialCb;
            PrimitiveInstanceConstantBuffer aabbCB;
        };
    }
}

namespace LocalRootSignature {
    inline UINT MaxRootArgumentsSize()
    {
        return max(sizeof(Triangle::RootArguments), sizeof(AABB::RootArguments));
    }
}

// Bottom-level acceleration structures (BottomLevelASType).
// This sample uses two BottomLevelASType, one for AABB and one for Triangle geometry.
// ToDo desc why the sample uses two - can a bottom-level AS mix geometry types?
namespace BottomLevelASType {
    enum Enum {
        Triangle = 0,
        AABB,
        Count
    };
}

// ToDo reuse geometrytype for BottomLevelASType?
namespace GeometryType {
    enum Enum {
        Triangle = 0,
        AABB,       // ToDo rename to procedural
        Count
    };
}

namespace RayType {
    enum Enum {
        Regular = 0,
        Shadow,
        Count
    };
}

// ToDo name all namespaces to * vs *Type.
namespace IntersectionShaderType {
    enum Enum {
        AnalyticPrimitive = 0,
        VolumetricPrimitive,
        SignedDistancePrimitive,
        Count = 3
    };
    inline UINT PerPrimitiveTypeCount(Enum type)
    {
        switch (type)
        {
        case AnalyticPrimitive: return AnalyticPrimitive::Count;
        case VolumetricPrimitive: return VolumetricPrimitive::Count;
        case SignedDistancePrimitive: return SignedDistancePrimitive::Count;
        }
        return 0;
    }
    static const UINT MaxPerPrimitiveTypeCount =
        max(AnalyticPrimitive::Count, max(VolumetricPrimitive::Count, SignedDistancePrimitive::Count));
    static const UINT TotalPrimitiveCount =
        AnalyticPrimitive::Count + VolumetricPrimitive::Count + SignedDistancePrimitive::Count;
}
