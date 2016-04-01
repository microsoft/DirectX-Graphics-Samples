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

#include <unordered_set>

// Set implementation using std::unordered_set for use by the residency library.
// Used by D3DX12Residency::ResidencyManager.
template <typename TObject>
class Set
{
public:
	bool Insert(TObject& Object)
	{
		// The second value in the tuple will be true if this got inserted
		return m_set.insert(Object).second;
	}

	// combine this set with another
	void Insert(Set<TObject>& Other)
	{
		m_set.insert(Other.m_set.begin(), Other.m_set.end());
	}

	// remove all items from the set
	void Clear()
	{
		m_set.clear();
	}

	SIZE_T Size()
	{
		return m_set.size();
	}

	// Iterator over all of the items in the set and call the Run function on that item
	template<typename FunctionWrapper>
	void ForEach(FunctionWrapper& Function)
	{
		for (auto& item : m_set)
		{
			Function.Run(item);
		}
	}

private:
	std::unordered_set<TObject> m_set;
};
