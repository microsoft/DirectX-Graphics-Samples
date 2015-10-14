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

#include "VectorMath.h"

namespace Math
{
	class BoundingSphere
	{
	public:
		BoundingSphere() {}
		BoundingSphere( Vector3 center, Scalar radius );
		explicit BoundingSphere( Vector4 sphere );

		Vector3 GetCenter( void ) const;
		Scalar GetRadius( void ) const;

	private:

		Vector4 m_repr;
	};

	//=======================================================================================================
	// Inline implementations
	//

	inline BoundingSphere::BoundingSphere( Vector3 center, Scalar radius )
	{
		m_repr = Vector4(center);
		m_repr.SetW(radius);
	}

	inline BoundingSphere::BoundingSphere( Vector4 sphere )
		: m_repr(sphere)
	{
	}

	inline Vector3 BoundingSphere::GetCenter( void ) const
	{
		return Vector3(m_repr);
	}

	inline Scalar BoundingSphere::GetRadius( void ) const
	{
		return m_repr.GetW();
	}

} // namespace Math
