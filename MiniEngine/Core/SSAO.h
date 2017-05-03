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

#pragma once

namespace Math { class Camera;  }

namespace SSAO
{
    void Initialize( void );
    void Shutdown( void );
    void Render(GraphicsContext& Context, const float* ProjMat, float NearClipDist, float FarClipDist );
    void Render(GraphicsContext& Context, const Math::Camera& camera );

    extern BoolVar Enable;
    extern BoolVar DebugDraw;
    extern BoolVar AsyncCompute;
    extern BoolVar ComputeLinearZ;
}
