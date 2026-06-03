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

#include "stdafx.h"

#include "CubeMesh.h"

GltfMeshData CreateCubeMesh()
{
    constexpr float s = 0.2f;
    constexpr float u = 1.f;

    GltfMeshData mesh;
    mesh.vertices = {
        // front
        {{-s, -s, -s}, {0, u}, {0.0f, 0.0f, -1.0f}},
        {{-s, s, -s}, {0, 0}, {0.0f, 0.0f, -1.0f}},
        {{s, s, -s}, {u, 0}, {0.0f, 0.0f, -1.0f}},
        {{-s, -s, -s}, {0, u}, {0.0f, 0.0f, -1.0f}},
        {{s, s, -s}, {u, 0}, {0.0f, 0.0f, -1.0f}},
        {{s, -s, -s}, {u, u}, {0.0f, 0.0f, -1.0f}},

        // back
        {{-s, -s, s}, {u, u}, {0.0f, 0.0f, 1.0f}},
        {{s, s, s}, {0, 0}, {0.0f, 0.0f, 1.0f}},
        {{-s, s, s}, {u, 0}, {0.0f, 0.0f, 1.0f}},
        {{-s, -s, s}, {u, u}, {0.0f, 0.0f, 1.0f}},
        {{s, -s, s}, {0, u}, {0.0f, 0.0f, 1.0f}},
        {{s, s, s}, {0, 0}, {0.0f, 0.0f, 1.0f}},

        // left
        {{-s, -s, s}, {0, u}, {-1.0f, 0.0f, 0.0f}},
        {{-s, s, s}, {0, 0}, {-1.0f, 0.0f, 0.0f}},
        {{-s, s, -s}, {u, 0}, {-1.0f, 0.0f, 0.0f}},
        {{-s, -s, s}, {0, u}, {-1.0f, 0.0f, 0.0f}},
        {{-s, s, -s}, {u, 0}, {-1.0f, 0.0f, 0.0f}},
        {{-s, -s, -s}, {u, u}, {-1.0f, 0.0f, 0.0f}},

        // right
        {{s, -s, -s}, {0, u}, {1.0f, 0.0f, 0.0f}},
        {{s, s, -s}, {0, 0}, {1.0f, 0.0f, 0.0f}},
        {{s, s, s}, {u, 0}, {1.0f, 0.0f, 0.0f}},
        {{s, -s, -s}, {0, u}, {1.0f, 0.0f, 0.0f}},
        {{s, s, s}, {u, 0}, {1.0f, 0.0f, 0.0f}},
        {{s, -s, s}, {u, u}, {1.0f, 0.0f, 0.0f}},

        // top
        {{-s, s, -s}, {0, u}, {0.0f, 1.0f, 0.0f}},
        {{-s, s, s}, {0, 0}, {0.0f, 1.0f, 0.0f}},
        {{s, s, s}, {u, 0}, {0.0f, 1.0f, 0.0f}},
        {{-s, s, -s}, {0, u}, {0.0f, 1.0f, 0.0f}},
        {{s, s, s}, {u, 0}, {0.0f, 1.0f, 0.0f}},
        {{s, s, -s}, {u, u}, {0.0f, 1.0f, 0.0f}},

        // bottom
        {{-s, -s, s}, {u, 0}, {0.0f, -1.0f, 0.0f}},
        {{-s, -s, -s}, {u, u}, {0.0f, -1.0f, 0.0f}},
        {{s, -s, -s}, {0, u}, {0.0f, -1.0f, 0.0f}},
        {{-s, -s, s}, {u, 0}, {0.0f, -1.0f, 0.0f}},
        {{s, -s, -s}, {0, u}, {0.0f, -1.0f, 0.0f}},
        {{s, -s, s}, {0, 0}, {0.0f, -1.0f, 0.0f}},
    };

    return mesh;
}
