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
// Author(s):  Jack Elliott
//

#include "pch.h"
#include "Cube.h"

namespace Graphics
{
    namespace Shapes
    {
        namespace Cube
        {
            ByteAddressBuffer g_CubeVerts;
            uint32_t g_NumVerts = 0;
            uint32_t g_NumIndicies = 0;

            void InitializeCubeBuffers()
            {
                const XMFLOAT3 pos[] =
                {
                    {-0.5f, -0.5f, -0.5f},
                    {-0.5f, -0.5f, +0.5f},
                    {+0.5f, -0.5f, +0.5f},
                    {+0.5f, -0.5f, -0.5f},

                    {-0.5f, +0.5f, -0.5f},
                    {-0.5f, +0.5f, +0.5f},
                    {+0.5f, +0.5f, +0.5f},
                    {+0.5f, +0.5f, -0.5f},
                };

                const XMFLOAT2 uv[] =
                {
                    {0, 0},
                    {1, 0},
                    {0, 1},
                    {1, 1}
                };

                const XMFLOAT3 posX = { +1, +0, +0 };
                const XMFLOAT3 posY = { +0, +1, +0 };
                const XMFLOAT3 posZ = { +0, +0, +1 };
                const XMFLOAT3 negX = { -1, +0, +0 };
                const XMFLOAT3 negY = { +0, -1, +0 };
                const XMFLOAT3 negZ = { +0, +0, -1 };

                CubeVertex verts[] =
                {
                    { pos[4], uv[0], negZ, posX, posY }, //Front
                    { pos[7], uv[1], negZ, posX, posY },
                    { pos[0], uv[2], negZ, posX, posY },
                    { pos[0], uv[2], negZ, posX, posY },
                    { pos[7], uv[1], negZ, posX, posY },
                    { pos[3], uv[3], negZ, posX, posY },

                    { pos[7], uv[0], posX, posZ, posY }, //Right
                    { pos[6], uv[1], posX, posZ, posY },
                    { pos[3], uv[2], posX, posZ, posY },
                    { pos[3], uv[2], posX, posZ, posY },
                    { pos[6], uv[1], posX, posZ, posY },
                    { pos[2], uv[3], posX, posZ, posY },

                    { pos[6], uv[0], posZ, negX, posY }, //Back
                    { pos[5], uv[1], posZ, negX, posY },
                    { pos[2], uv[2], posZ, negX, posY },
                    { pos[2], uv[2], posZ, negX, posY },
                    { pos[5], uv[1], posZ, negX, posY },
                    { pos[1], uv[3], posZ, negX, posY },

                    { pos[5], uv[0], negX, posZ, posY }, //Left
                    { pos[4], uv[1], negX, posZ, posY },
                    { pos[1], uv[2], negX, posZ, posY },
                    { pos[1], uv[2], negX, posZ, posY },
                    { pos[4], uv[1], negX, posZ, posY },
                    { pos[0], uv[3], negX, posZ, posY },

                    { pos[5], uv[0], posY, posX, posZ }, //Top
                    { pos[6], uv[1], posY, posX, posZ },
                    { pos[4], uv[2], posY, posX, posZ },
                    { pos[4], uv[2], posY, posX, posZ },
                    { pos[6], uv[1], posY, posX, posZ },
                    { pos[7], uv[3], posY, posX, posZ },

                    { pos[0], uv[0], negY, posX, posZ }, //Bottom
                    { pos[3], uv[1], negY, posX, posZ },
                    { pos[1], uv[2], negY, posX, posZ },
                    { pos[1], uv[2], negY, posX, posZ },
                    { pos[3], uv[1], negY, posX, posZ },
                    { pos[2], uv[3], negY, posX, posZ },
                };

                g_NumVerts = _countof(verts);

                g_CubeVerts.Create(L"Cube Vertices", _countof(verts), sizeof(verts[0]), verts);
            }

            void DestroyCubeBuffers()
            {
                g_CubeVerts.Destroy();
            }
        }
    }
}