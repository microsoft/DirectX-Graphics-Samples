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

#include "PostEffectsRS.hlsli"
#include "PixelPacking.hlsli"

RWTexture2D<float3> SceneColor : register( u0 );
Texture2D<uint> PostBuffer : register( t0 );

[RootSignature(PostEffects_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 DTid : SV_DispatchThreadID )
{
    SceneColor[DTid.xy] = Unpack_R11G11B10_FLOAT(PostBuffer[DTid.xy]);
}
