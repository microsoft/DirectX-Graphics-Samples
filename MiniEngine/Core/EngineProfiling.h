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

#include <string>
#include "TextRenderer.h"

class CommandContext;

class ScopedTimer
{
public:
	ScopedTimer( const std::wstring& name );
	ScopedTimer( const std::wstring& name, CommandContext& Context );
	~ScopedTimer();

#ifndef RELEASE
private:
	CommandContext* m_Context;
#endif
};

#ifdef RELEASE
inline ScopedTimer::ScopedTimer( const std::wstring& name ) {}
inline ScopedTimer::ScopedTimer( const std::wstring&, CommandContext& ) {}
inline ScopedTimer::~ScopedTimer(  ) {}
#endif

namespace EngineProfiling
{
	// Bookend the game update loop minus the call to Present() which can incur a long wait loop
	void BeginFrame( CommandContext& Context );
	void EndFrame( CommandContext& Context );

	void DisplayFrameRate( TextContext& Text );
	void Display( TextContext& Text, float x, float y, float w, float h );
}
