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
#ifndef HLSL_COMPAT_H_INCLUDED
#define HLSL_COMPAT_H_INCLUDED

#define OUTPARAM(type, name)    type& name
#define INOUTPARAM(type, name)    type& name

struct float2
{
    float   x, y;
};

struct float3
{
    float   x, y, z;
};

struct float4
{
    float   x, y, z, w;
};

__declspec(align(16))
struct uint4
{
    UINT    x, y, z, w;
};

__declspec(align(16))
struct float4x4
{
    float   mat[16];
};

typedef UINT uint;

typedef UINT uint2[2];

inline
float3 operator + (const float3& a, const float3& b)
{
    return float3{ a.x + b.x, a.y + b.y, a.z + b.z };
}


inline
float3 operator - (const float3& a, const float3& b)
{
    return float3{ a.x - b.x, a.y - b.y, a.z - b.z };
}

inline
float3 operator * (const float3& a, const float3& b)
{
    return float3{ a.x * b.x, a.y * b.y, a.z * b.z };
}

inline
float3 operator * (const float3& a, const float& b)
{
    return float3{ a.x * b, a.y * b, a.z * b };
}

inline
float3 operator / (const float3& a, const float& b)
{
    return float3{ a.x / b, a.y / b, a.z / b };
}

inline
float3 operator / (const float3& a, const float3& b)
{
    return float3{ a.x / b.x, a.y / b.y, a.z / b.z };
}

inline
float3 abs(const float3& a)
{
    return float3{ std::abs(a.x), std::abs(a.y), std::abs(a.z) };
}

inline
float min(float a, float b)
{
    return std::min(a, b);
}

inline
float3 min(const float3& a, const float3& b)
{
    return float3{ std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z) };
}


inline
float max(float a, float b)
{
    return std::max(a, b);
}

inline
float3 max(const float3& a, const float3& b)
{
    return float3{ std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z) };
}

inline
float sign(float v)
{
    if (v < 0)
        return -1;
    return 1;
}

inline
float   dot(float3 a, float3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline
float3 cross(float3 a, float3 b)
{
    return float3
    {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

#endif // HLSL_COMPAT_H_INCLUDED
