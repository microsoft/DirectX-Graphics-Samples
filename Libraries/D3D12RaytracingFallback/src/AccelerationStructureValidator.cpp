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

#pragma once

namespace FallbackLayer
{
    IAccelerationStructureValidator &GetAccelerationStructureValidator(AccelerationStructureLayoutType type)
    {
        switch (type)
        {
        case BVH2:
            {
                static BvhValidator bvhValidator;
                return bvhValidator;
            }

        default:
            ThrowInternalFailure(E_INVALIDARG);
            return *(IAccelerationStructureValidator*)nullptr;
        }
    }

}
