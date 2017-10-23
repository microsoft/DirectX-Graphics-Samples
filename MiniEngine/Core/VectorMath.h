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
// This is a system of classes that wrap DirectXMath in a more object-oriented and concise (readable) way.  While these
// classes are not designed to maximize throughput on the instruction level, they are designed to be easily understood.
// I believe that the source of most math library inefficiency is in their being too confusing and not making the preferred
// way of doing things obvious.  This leaves programmers constantly finding usage patterns that "work for them" but are
// ultimately inefficient and don't use the API the "way it was intended".  The goal of this wrapper is to be cogent and
// familiar.
// 
// Note that DirectXMath treats vectors like [1x4] matrices (rows) rather than [4x1] matrices (columns).  Likewise, it
// treats matrices like they are transposed, so that you would multiply a vector and a matrix like so:
//
//    Vector [1x4] = Vector [1x4] * Matrix [4x4]
//
// Applying multiple transforms to a vector involves concatenating on the outside, or right of the previous transform:
//
//    ProjectedPosition = ModelPosition * ModelToWorld * WorldToView * ViewToProj
//
// This is *not* how this API works because it is needlessly contrary to Math textbooks.  It's not "wrong", per se,
// but it's a paradigm I'd like to see changed.  A vector is four floats.  A matrix is four consecutive vectors.  Whether
// you think of them as row or column vectors of a matrix is just a matter of perspective.  In this library, you will see:
//
//    Vector [4x1] = Matrix4 [4x4] * Vector [4x1]
//
// and
//
//    ProjectedPosition = ViewToProj * WorldToView * ModelToWorld * ModelPosition
//
// One very happy result of this is that you can stop transposing every matrix you set in a shader constant buffer.  They
// were always in the right format, you were just multiplying them backwards.  In the shader you should have been calling
// mul( matrix, vector ) rather than mul( vector, matrix ).  Then you wouldn't have needed to transpose the matrix.
//
// It's possible to work in a transposed space:  (B*A*x)ᵀ = xᵀ*Aᵀ*Bᵀ   but why would you want to?
//
// Oh, and we use right-handed coordinate systems because that's what you learned in your Linear Algebra, Calculus, and
// Physics classes.
//
// Peace,
//   James


#pragma once

#include "Math/Scalar.h"
#include "Math/Vector.h"
#include "Math/Quaternion.h"
#include "Math/Matrix3.h"
#include "Math/Transform.h"
#include "Math/Matrix4.h"
#include "Math/Functions.inl"
