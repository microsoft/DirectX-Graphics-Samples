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
    class BVHTraversalShaderBuilder : public ITraversalShaderBuilder
    {
    public:
        BVHTraversalShaderBuilder(DxilShaderPatcher &dxilShaderPatcher) : m_DxilShaderPatcher(dxilShaderPatcher) {}

        void Compile(_In_ bool IsAnyHitOrIntersectionUsed, _Out_ TraversalShader &traversalShader);

    private:
        DxilShaderPatcher &m_DxilShaderPatcher;
    };
}
