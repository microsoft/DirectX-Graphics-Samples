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

#pragma once

#include "Math/Common.h"
#include <map>

namespace Utility
{
	inline size_t HashIterate( size_t Next, size_t CurrentHash = 2166136261U)
	{
		return 16777619U * CurrentHash ^ Next;
	}

	template <typename T> inline size_t HashRange(const T* Begin, const T* End, size_t InitialVal = 2166136261U)
	{
		size_t Val = InitialVal;

		while (Begin < End)
			Val = HashIterate((size_t)*Begin++, Val);

		return Val;
	}

	template <typename T> inline size_t HashStateArray( const T* StateDesc, size_t Count, size_t InitialVal = 2166136261U )
	{
		static_assert((sizeof(T) & 3) == 0, "State object is not word-aligned");
		return HashRange((UINT*)StateDesc, (UINT*)(StateDesc + Count), InitialVal);
	}

	template <typename T> inline size_t HashState( const T* StateDesc, size_t InitialVal = 2166136261U )
	{
		static_assert((sizeof(T) & 3) == 0, "State object is not word-aligned");
		return HashRange((UINT*)StateDesc, (UINT*)(StateDesc + 1), InitialVal);
	}

} // namespace Utility