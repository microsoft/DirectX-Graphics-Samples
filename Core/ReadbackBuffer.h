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

#include "GpuBuffer.h"

class ReadbackBuffer : public GpuBuffer
{
public:
    virtual ~ReadbackBuffer() { Destroy(); }

    void Create( const std::wstring& name, uint32_t NumElements, uint32_t ElementSize );

    void* Map(void);
    void Unmap(void);

protected:

    void CreateDerivedViews(void) {}

};
