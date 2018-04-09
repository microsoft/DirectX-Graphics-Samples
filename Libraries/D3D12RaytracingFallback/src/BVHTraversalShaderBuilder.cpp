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
#include "pch.h"
#include "CompiledShaders\TraverseShaderLib.h"
#include "CompiledShaders\MinimalTraverseShaderLib.h"

#if PROFILE_STATE_MACHINE
#include "CompiledShaders\NullTraverseLib.h"
#endif

namespace FallbackLayer
{
        void BVHTraversalShaderBuilder::Compile(
            _In_ bool IsAnyHitOrIntersectionUsed,
            _Out_ TraversalShader &traversalShader)
    {
            traversalShader = {};
            traversalShader.m_TraversalShaderDxilLib.DXILLibrary.BytecodeLength = IsAnyHitOrIntersectionUsed ? sizeof(g_pTraverseShaderLib) : sizeof(g_pMinimalTraverseShaderLib);
            traversalShader.m_TraversalShaderDxilLib.DXILLibrary.pShaderBytecode = IsAnyHitOrIntersectionUsed ? g_pTraverseShaderLib : g_pMinimalTraverseShaderLib;
#if PROFILE_STATE_MACHINE
            // By providing a null traversal shader, the cost of just running the other states can be measured
            traversalShader.m_TraversalShaderDxilLib.DXILLibrary.BytecodeLength = sizeof(g_pNullTraverseLib);
            traversalShader.m_TraversalShaderDxilLib.DXILLibrary.pShaderBytecode = g_pNullTraverseLib;
#endif
    }

}
