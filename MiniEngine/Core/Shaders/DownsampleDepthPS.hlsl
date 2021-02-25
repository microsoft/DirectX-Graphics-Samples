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

#include "CommonRS.hlsli"

Texture2DMS<float> DepthBuffer : register(t0);

[RootSignature(Common_RootSig)]
float main( float4 position : SV_Position ) : SV_Depth
{
    return DepthBuffer.Load((int2)position.xy, 0);
}
