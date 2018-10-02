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

#include "RayTracingHlslCompat.h"

namespace RNGVisualizerRootSignature {
    namespace Slot {
        enum Enum {
            OutputView = 0,
            SampleBuffers,
            SceneConstant,
            Count
        };
    }
}

namespace GlobalRootSignature {
    namespace Slot {
        enum Enum {
            OutputView = 0,
            AccelerationStructure,
            SceneConstant,
            AABBattributeBuffer,
            SampleBuffers,
            Count
        };
    }
}

namespace LocalRootSignature {
    namespace Type {
        enum Enum {
            Triangle = 0,
            AABB,
            Count
        };
    }
}

namespace LocalRootSignature {
    namespace Triangle {
        namespace Slot {
            enum Enum {
                MaterialConstant = 0,
                VertexBuffers,
                Count
            };
        }
        struct RootArguments {
            PrimitiveConstantBuffer materialCb;
            UINT32 vertexBufferGPUHandle;
            UINT32 padding; // ToDo desc
        };
    }
}

namespace LocalRootSignature {
    namespace AABB {
        namespace Slot {
            enum Enum {
                MaterialConstant = 0,
                GeometryIndex,
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

namespace GeometryType {
    enum Enum {
        Plane = 0,
        Sphere,
        Count
    };
}

namespace GpuTimers {
	enum Enum {
		Raytracing = 0,
		UpdateBLAS,
		UpdateTLAS,
		Count
	};
}

namespace UIParameters {
	enum Enum {
		RaytracingAPI = 0,
		BuildQuality,
		UpdateAlgorithm,
		TesselationQuality,
		NumberOfObjects,
		Count
	};
}

// Bottom-level acceleration structures (BottomLevelASType).
// This sample uses two BottomLevelASType, one for AABB and one for Triangle geometry.
// Mixing of geometry types within a BLAS is not supported.
namespace BottomLevelASType = GeometryType;


namespace IntersectionShaderType {
    enum Enum {
        AnalyticPrimitive = 0,
        VolumetricPrimitive,
        SignedDistancePrimitive,
        Count
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

