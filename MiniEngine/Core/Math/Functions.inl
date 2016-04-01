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

namespace Math
{
	// To allow floats to implicitly construct Scalars, we need to clarify these operators and suppress
	// upconversion.
	INLINE bool operator<  ( const Scalar &lhs, float rhs ) { return (float)lhs <  rhs; }
	INLINE bool operator<= ( const Scalar &lhs, float rhs ) { return (float)lhs <= rhs; }
	INLINE bool operator>  ( const Scalar &lhs, float rhs ) { return (float)lhs >  rhs; }
	INLINE bool operator>= ( const Scalar &lhs, float rhs ) { return (float)lhs >= rhs; }
	INLINE bool operator== ( const Scalar &lhs, float rhs ) { return (float)lhs == rhs; }
	INLINE bool operator<  ( float lhs, const Scalar &rhs ) { return lhs <  (float)rhs; }
	INLINE bool operator<= ( float lhs, const Scalar &rhs ) { return lhs <= (float)rhs; }
	INLINE bool operator>  ( float lhs, const Scalar &rhs ) { return lhs >  (float)rhs; }
	INLINE bool operator>= ( float lhs, const Scalar &rhs ) { return lhs >= (float)rhs; }
	INLINE bool operator== ( float lhs, const Scalar &rhs ) { return lhs == (float)rhs; }

#define CREATE_SIMD_FUNCTIONS( TYPE ) \
	INLINE TYPE Sqrt( const TYPE &s ) { return TYPE(XMVectorSqrt(s)); } \
	INLINE TYPE Recip( const TYPE &s ) { return TYPE(XMVectorReciprocal(s)); } \
	INLINE TYPE RecipSqrt( const TYPE &s ) { return TYPE(XMVectorReciprocalSqrt(s)); } \
	INLINE TYPE Floor( const TYPE &s ) { return TYPE(XMVectorFloor(s)); } \
	INLINE TYPE Ceiling( const TYPE &s ) { return TYPE(XMVectorCeiling(s)); } \
	INLINE TYPE Round( const TYPE &s ) { return TYPE(XMVectorRound(s)); } \
	INLINE TYPE Abs( const TYPE &s ) { return TYPE(XMVectorAbs(s)); } \
	INLINE TYPE Exp( const TYPE &s ) { return TYPE(XMVectorExp(s)); } \
	INLINE TYPE Pow( const TYPE &b, const TYPE &e ) { return TYPE(XMVectorPow(b, e)); } \
	INLINE TYPE Log( const TYPE &s ) { return TYPE(XMVectorLog(s)); } \
	INLINE TYPE Sin( const TYPE &s ) { return TYPE(XMVectorSin(s)); } \
	INLINE TYPE Cos( const TYPE &s ) { return TYPE(XMVectorCos(s)); } \
	INLINE TYPE Tan( const TYPE &s ) { return TYPE(XMVectorTan(s)); } \
	INLINE TYPE ASin( const TYPE &s ) { return TYPE(XMVectorASin(s)); } \
	INLINE TYPE ACos( const TYPE &s ) { return TYPE(XMVectorACos(s)); } \
	INLINE TYPE ATan( const TYPE &s ) { return TYPE(XMVectorATan(s)); } \
	INLINE TYPE ATan2( const TYPE &y, const TYPE &x ) { return TYPE(XMVectorATan2(y, x)); } \
	INLINE TYPE Lerp( const TYPE &a, const TYPE &b, const TYPE &t ) { return TYPE(XMVectorLerpV(a, b, t)); } \
	INLINE TYPE Max( const TYPE &a, const TYPE &b ) { return TYPE(XMVectorMax(a, b)); } \
	INLINE TYPE Min( const TYPE &a, const TYPE &b ) { return TYPE(XMVectorMin(a, b)); } \
	INLINE TYPE Clamp( const TYPE &v, const TYPE &a, const TYPE &b ) { return Min(Max(v, a), b); } \
	INLINE BoolVector operator<  ( const TYPE &lhs, const TYPE &rhs ) { return XMVectorLess(lhs, rhs); } \
	INLINE BoolVector operator<= ( const TYPE &lhs, const TYPE &rhs ) { return XMVectorLessOrEqual(lhs, rhs); } \
	INLINE BoolVector operator>  ( const TYPE &lhs, const TYPE &rhs ) { return XMVectorGreater(lhs, rhs); } \
	INLINE BoolVector operator>= ( const TYPE &lhs, const TYPE &rhs ) { return XMVectorGreaterOrEqual(lhs, rhs); } \
	INLINE BoolVector operator== ( const TYPE &lhs, const TYPE &rhs ) { return XMVectorEqual(lhs, rhs); } \
	INLINE TYPE Select( const TYPE &lhs, const TYPE &rhs, const BoolVector &mask ) { return TYPE(XMVectorSelect(lhs, rhs, mask)); }


	CREATE_SIMD_FUNCTIONS(Scalar)
	CREATE_SIMD_FUNCTIONS(Vector3)
	CREATE_SIMD_FUNCTIONS(Vector4)

#undef CREATE_SIMD_FUNCTIONS

	INLINE float Sqrt( float s ) { return Sqrt(Scalar(s)); }
	INLINE float Recip( float s ) { return Recip(Scalar(s)); }
	INLINE float RecipSqrt( float s ) { return RecipSqrt(Scalar(s)); }
	INLINE float Floor( float s ) { return Floor(Scalar(s)); }
	INLINE float Ceiling( float s ) { return Ceiling(Scalar(s)); }
	INLINE float Round( float s ) { return Round(Scalar(s)); }
	INLINE float Abs( float s ) { return Abs(Scalar(s)); }
	INLINE float Exp( float s ) { return Exp(Scalar(s)); }
	INLINE float Pow( float b, float e ) { return Pow(Scalar(b), Scalar(e)); }
	INLINE float Log( float s ) { return Log(Scalar(s)); }
	INLINE float Sin( float s ) { return Sin(Scalar(s)); }
	INLINE float Cos( float s ) { return Cos(Scalar(s)); }
	INLINE float Tan( float s ) { return Tan(Scalar(s)); }
	INLINE float ASin( float s ) { return ASin(Scalar(s)); }
	INLINE float ACos( float s ) { return ACos(Scalar(s)); }
	INLINE float ATan( float s ) { return ATan(Scalar(s)); }
	INLINE float ATan2( float y, float x ) { return ATan2(Scalar(y), Scalar(x)); }
	INLINE float Lerp( float a, float b, float t ) { return Lerp(Scalar(a), Scalar(b), Scalar(t)); }
	INLINE float Max( float a, float b ) { return Max(Scalar(a), Scalar(b)); }
	INLINE float Min( float a, float b ) { return Min(Scalar(a), Scalar(b)); }
	INLINE float Clamp( float v, float a, float b ) { return Clamp(Scalar(v), Scalar(a), Scalar(b)); }

	INLINE Scalar Length( const Vector3 &v ) 
	{ 
		return Scalar(XMVector3Length(v)); 
	}
	INLINE Scalar LengthSquare( const Vector3 &v ) { return Scalar(XMVector3LengthSq(v)); }
	INLINE Scalar LengthRecip( const Vector3 &v ) { return Scalar(XMVector3ReciprocalLength(v)); }
	INLINE Scalar Dot( const Vector3 &v1, const Vector3 &v2 ) { return Scalar(XMVector3Dot(v1, v2)); }
	INLINE Scalar Dot( const Vector4 &v1, const Vector4 &v2 ) { return Scalar(XMVector4Dot(v1, v2)); }
	INLINE Vector3 Cross( const Vector3 &v1, const Vector3 &v2 ) { return Vector3(XMVector3Cross(v1, v2)); }
	INLINE Vector3 Normalize( const Vector3 &v ) { return Vector3(XMVector3Normalize(v)); }
	INLINE Vector4 Normalize( const Vector4 &v ) { return Vector4(XMVector4Normalize(v)); }
	INLINE Quaternion Normalize( const Quaternion &q ) { return Quaternion(XMQuaternionNormalize(q)); }

	INLINE Matrix3 Transpose( const Matrix3& mat ) { return Matrix3(XMMatrixTranspose(mat)); }

	// inline Matrix3 Inverse( const Matrix3& mat ) { TBD }
	// inline Transform Inverse( const Transform& mat ) { TBD }

	// This specialized matrix invert assumes that the 3x3 matrix is orthogonal (and normalized).
	INLINE AffineTransform OrthoInvert( const AffineTransform& xform )
	{
		Matrix3 basis = Transpose(xform.GetBasis());
		return AffineTransform( basis, basis * -xform.GetTranslation() );
	}

	INLINE OrthogonalTransform Invert( const OrthogonalTransform& xform )	 { return ~xform; }

	INLINE Matrix4 Transpose( const Matrix4& mat ) { return Matrix4(XMMatrixTranspose(mat)); }
	INLINE Matrix4 Invert( const Matrix4& mat ) { return Matrix4(XMMatrixInverse(nullptr, mat)); }

	INLINE Matrix4 OrthoInvert( const Matrix4& xform )
	{
		Matrix3 basis = Transpose(xform.Get3x3());
		Vector3 translate = basis * -Vector3(xform.GetW());
		return Matrix4( basis, translate );
	}

}