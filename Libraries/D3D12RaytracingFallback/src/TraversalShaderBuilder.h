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

namespace FallbackLayer
{
    class TraversalShader
    {
    public:
        D3D12_DXIL_LIBRARY_DESC m_TraversalShaderDxilLib;
    };

    class ITraversalShaderBuilder
    {
    public:
        virtual void Compile(_In_ bool IsAnyHitOrIntersectionUsed, _Out_ TraversalShader &traversalShader) = 0;
    };
}
