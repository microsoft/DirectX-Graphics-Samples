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
// The "fast path" Lanczos filter can be used when the source tile dimensions
// plus the border pixels (3) fit inside 16x16.  This reduces bandwidth and
// uses more efficient Math.  To determine this case, use the following logic:
// 
// bool EnableFastPath = all(srcDim.xy / dstDim.xy <= 13.0 / 16.0);
//

#define TILE_DIM_X 16
#define TILE_DIM_Y 16
#define ENABLE_FAST_PATH 1
#include "LanczosCS.hlsl"
