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
            VertexBuffers,      // ToDo Rename as it has both IB and VB
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
            MaterialConstantBuffer materialCb;
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
            MaterialConstantBuffer materialCb;
            AABBConstantBuffer aabbCB;
        };
    }
}

namespace LocalRootSignature {
    inline UINT MaxRootArgumentsSize()
    {
        return max(sizeof(Triangle::RootArguments), sizeof(AABB::RootArguments));
    }
}

// Bottom level acceleration structures (BottomLevelASType).
// This sample uses two BottomLevelASType, one for AABB and one for Triangle geometry.
// ToDo desc why the sample uses two - can a BLAS mix geometry types?
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
        AABB,
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

namespace IntersectionShaderType {
    enum Enum {
        AABB = 0,
        Sphere,
        Spheres,
        Count
    };
}
