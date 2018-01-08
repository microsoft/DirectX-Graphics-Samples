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

class ColorBuffer;
class BoolVar;
class NumVar;
class ComputeContext;

namespace FXAA
{
    extern BoolVar Enable;
    extern NumVar ContrastThreshold;	// Default = 0.20
    extern NumVar SubpixelRemoval;		// Default = 0.75

    void Initialize( void );
    void Shutdown( void );
    void Render( ComputeContext& Context, bool bUsePreComputedLuma );

} // namespace FXAA
