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

#include "pch.h"

class EsramAllocator
{
public:
    EsramAllocator() {}

    void PushStack() {}
    void PopStack() {}

    D3D12_GPU_VIRTUAL_ADDRESS Alloc( size_t size, size_t align, const std::wstring& bufferName )
    {
        (size); (align); (bufferName);
        return D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }

    intptr_t SizeOfFreeSpace( void ) const
    {
        return 0;
    }

};
