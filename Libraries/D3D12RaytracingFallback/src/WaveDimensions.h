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

// This ends up being the safest threadgroup size to use across vendors. 
// The largest existing wave size on hardware at the present is 64, so 
// this ensures no empty lanes and hardware with smaller wave sizes are
// multiples of 2 so 64 is guaranteed to divide into smaller waves with
// no remainders
#define     THREAD_GROUP_WIDTH 8
#define     THREAD_GROUP_HEIGHT 8
#define     WAVE_SIZE (THREAD_GROUP_WIDTH * THREAD_GROUP_HEIGHT)
#define     THREAD_GROUP_1D_WIDTH WAVE_SIZE
