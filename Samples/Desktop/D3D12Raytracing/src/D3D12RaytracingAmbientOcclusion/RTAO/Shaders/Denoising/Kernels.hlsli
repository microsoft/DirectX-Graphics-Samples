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

#define HLSL

// Note: [3/12/2019] DXC fails to compile with both /Od /Zi specified when a global symbol is defined under a namespace. Workaround: remove /Od.

namespace FilterKernel
{
#if defined(BOX_KERNEL_3X3)
    static const unsigned int Radius = 1;
    static const unsigned int Width = 1 + 2 * Radius;
    static const float Kernel[Width][Width] =
    {
        { 1. / 9, 1. / 9, 1. / 9 },
        { 1. / 9, 1. / 9, 1. / 9 },
        { 1. / 9, 1. / 9, 1. / 9 },
    };

#elif defined(BOX_KERNEL_5X5)
    static const unsigned int Radius = 2;
    static const unsigned int Width = 1 + 2 * Radius;
    static const float Kernel[Width][Width] =
    {
        { 1. / 25, 1. / 25, 1. / 25, 1. / 25, 1. / 25  },
        { 1. / 25, 1. / 25, 1. / 25, 1. / 25, 1. / 25  },
        { 1. / 25, 1. / 25, 1. / 25, 1. / 25, 1. / 25  },
        { 1. / 25, 1. / 25, 1. / 25, 1. / 25, 1. / 25  },
        { 1. / 25, 1. / 25, 1. / 25, 1. / 25, 1. / 25  },
    };

#elif defined(BOX_KERNEL_7X7)
    static const unsigned int Radius = 3;
    static const unsigned int Width = 1 + 2 * Radius;

#elif defined(GAUSSIAN_KERNEL_3X3)
    static const unsigned int Radius = 1;
    static const unsigned int Width = 1 + 2 * Radius;
    static const float Kernel1D[Width] = { 0.27901, 0.44198, 0.27901 };
    static const float Kernel[Width][Width] =
    {
        { Kernel1D[0] * Kernel1D[0], Kernel1D[0] * Kernel1D[1], Kernel1D[0] * Kernel1D[2] },
        { Kernel1D[1] * Kernel1D[0], Kernel1D[1] * Kernel1D[1], Kernel1D[1] * Kernel1D[2] },
        { Kernel1D[2] * Kernel1D[0], Kernel1D[2] * Kernel1D[1], Kernel1D[2] * Kernel1D[2] },
    };

#elif defined(GAUSSIAN_KERNEL_5X5)
    static const unsigned int Radius = 2;
    static const unsigned int Width = 1 + 2 * Radius;
    static const float Kernel1D[Width] = { 1. / 16, 1. / 4, 3. / 8, 1. / 4, 1. / 16 };
    static const float Kernel[Width][Width] =
    {
        { Kernel1D[0] * Kernel1D[0], Kernel1D[0] * Kernel1D[1], Kernel1D[0] * Kernel1D[2], Kernel1D[0] * Kernel1D[3], Kernel1D[0] * Kernel1D[4] },
        { Kernel1D[1] * Kernel1D[0], Kernel1D[1] * Kernel1D[1], Kernel1D[1] * Kernel1D[2], Kernel1D[1] * Kernel1D[3], Kernel1D[1] * Kernel1D[4] },
        { Kernel1D[2] * Kernel1D[0], Kernel1D[2] * Kernel1D[1], Kernel1D[2] * Kernel1D[2], Kernel1D[2] * Kernel1D[3], Kernel1D[2] * Kernel1D[4] },
        { Kernel1D[3] * Kernel1D[0], Kernel1D[3] * Kernel1D[1], Kernel1D[3] * Kernel1D[2], Kernel1D[3] * Kernel1D[3], Kernel1D[3] * Kernel1D[4] },
        { Kernel1D[4] * Kernel1D[0], Kernel1D[4] * Kernel1D[1], Kernel1D[4] * Kernel1D[2], Kernel1D[4] * Kernel1D[3], Kernel1D[4] * Kernel1D[4] },
    };

#elif defined(GAUSSIAN_KERNEL_7X7)
    static const unsigned int Radius = 3;
    static const unsigned int Width = 1 + 2 * Radius;
    static const float Kernel1D[Width] = { 0.00598, 0.060626, 0.241843, 0.383103, 0.241843, 0.060626, 0.00598 };

#elif defined(GAUSSIAN_KERNEL_9X9)
    static const unsigned int Radius = 4;
    static const unsigned int Width = 1 + 2 * Radius;
    static const float Kernel1D[Width] = { 0.000229, 0.005977, 0.060598, 0.241732, 0.382928, 0.241732, 0.060598, 0.005977, 0.000229 };
#endif
}