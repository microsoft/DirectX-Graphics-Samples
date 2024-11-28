
//
//  MJP's DX12 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "pch.h"
#include "directxpackedvector.h"

namespace SampleFramework12
{

// Forward declarations
struct Quaternion;
struct Float3x3;
struct Float4x4;

// Extension classes for XMFLOAT* classes

struct Float2
{
    float x, y;

    Float2();
    Float2(float x);
    Float2(float x, float y);
    explicit Float2(const DirectX::XMFLOAT2& xy);
    explicit Float2(DirectX::FXMVECTOR xy);

    Float2& operator+=(const Float2& other);
    Float2 operator+(const Float2& other) const;

    Float2& operator-=(const Float2& other);
    Float2 operator-(const Float2& other) const;

    Float2& operator*=(const Float2& other);
    Float2 operator*(const Float2& other) const;

    Float2& operator*=(float s);
    Float2 operator*(float s) const;

    Float2& operator/=(const Float2& other);
    Float2 operator/(const Float2& other) const;

    Float2& operator/=(float s);
    Float2 operator/(float s) const;

    bool operator==(const Float2& other) const;
    bool operator!=(const Float2& other) const;

    Float2 operator-() const;

    DirectX::XMVECTOR ToSIMD() const;

    static Float2 Clamp(const Float2& val, const Float2& min, const Float2& max);
    static float Length(const Float2& val);
};

struct Float3
{
    float x, y, z;

    Float3();
    Float3(float x);
    Float3(float x, float y, float z);
    Float3(Float2 xy, float z);
    explicit Float3(const DirectX::XMFLOAT3& xyz);
    explicit Float3(DirectX::FXMVECTOR xyz);

    float operator[](unsigned int idx) const;
    Float3& operator+=(const Float3& other);
    Float3 operator+(const Float3& other) const;

    Float3& operator+=(float other);
    Float3 operator+(float other) const;

    Float3& operator-=(const Float3& other);
    Float3 operator-(const Float3& other) const;

    Float3& operator-=(float s);
    Float3 operator-(float s) const;

    Float3& operator*=(const Float3& other);
    Float3 operator*(const Float3& other) const;

    Float3& operator*=(float s);
    Float3 operator*(float s) const;

    Float3& operator/=(const Float3& other);
    Float3 operator/(const Float3& other) const;

    Float3& operator/=(float s);
    Float3 operator/(float s) const;

    bool operator==(const Float3& other) const;
    bool operator!=(const Float3& other) const;

    Float3 operator-() const;

    DirectX::XMVECTOR ToSIMD() const;
    DirectX::XMFLOAT3 ToXMFLOAT3() const;
    Float2 To2D() const;

    float Length() const;

    static float Dot(const Float3& a, const Float3& b);
    static Float3 Cross(const Float3& a, const Float3& b);
    static Float3 Normalize(const Float3& a);
    static Float3 Transform(const Float3& v, const Float3x3& m);
    static Float3 Transform(const Float3& v, const Float4x4& m);
    static Float3 TransformDirection(const Float3&v, const Float4x4& m);
    static Float3 Transform(const Float3& v, const Quaternion& q);
    static Float3 Clamp(const Float3& val, const Float3& min, const Float3& max);
    static Float3 Perpendicular(const Float3& v);
    static float Distance(const Float3& a, const Float3& b);
    static float Length(const Float3& v);
};

// Non-member operators of Float3
Float3 operator*(float a, const Float3& b);

struct Float4
{
    float x, y, z, w;

    Float4();
    Float4(float x);
    Float4(float x, float y, float z, float w);
    explicit Float4(const Float3& xyz, float w = 0.0f);
    explicit Float4(const DirectX::XMFLOAT4& xyzw);
    explicit Float4(DirectX::FXMVECTOR xyzw);

    Float4& operator+=(const Float4& other);
    Float4 operator+(const Float4& other) const;

    Float4& operator-=(const Float4& other);
    Float4 operator-(const Float4& other) const;

    Float4& operator*=(const Float4& other);
    Float4 operator*(const Float4& other) const;

    Float4& operator/=(const Float4& other);
    Float4 operator/(const Float4& other) const;

    bool operator==(const Float4& other) const;
    bool operator!=(const Float4& other) const;

    Float4 operator-() const;

    DirectX::XMVECTOR ToSIMD() const;
    Float3 To3D() const;
    Float2 To2D() const;

    static Float4 Clamp(const Float4& val, const Float4& min, const Float4& max);
    static Float4 Transform(const Float4& v, const Float4x4& m);
};

struct Quaternion
{
    float x, y, z, w;

    Quaternion();
    Quaternion(float x, float y, float z, float w);
    Quaternion(const Float3& axis, float angle);
    explicit Quaternion(const Float3x3& m);
    explicit Quaternion(const DirectX::XMFLOAT4& q);
    explicit Quaternion(DirectX::FXMVECTOR q);

    Quaternion& operator*=(const Quaternion& other);
    Quaternion operator*(const Quaternion& other) const;

    bool operator==(const Quaternion& other) const;
    bool operator!=(const Quaternion& other) const;

    Float3x3 ToFloat3x3() const;
    Float4x4 ToFloat4x4() const;

    static Quaternion Identity();
    static Quaternion Invert(const Quaternion& q);
    static Quaternion FromAxisAngle(const Float3& axis, float angle);
    static Quaternion FromEuler(float x, float y, float z);
    static Quaternion Normalize(const Quaternion& q);
    static Float3x3 ToFloat3x3(const Quaternion& q);
    static Float4x4 ToFloat4x4(const Quaternion& q);

    DirectX::XMVECTOR ToSIMD() const;
    DirectX::XMFLOAT4 ToXMFLOAT4() const;
};

struct Float3x3
{
    float _11, _12, _13;
    float _21, _22, _23;
    float _31, _32, _33;

    Float3x3();
    explicit Float3x3(const DirectX::XMFLOAT3X3& m);
    explicit Float3x3(DirectX::CXMMATRIX m);
    Float3x3(const Float3& r0, const Float3& r1, const Float3& r2);

    Float3x3& operator*=(const Float3x3& other);
    Float3x3 operator*(const Float3x3& other) const;

    Float3 Up() const;
    Float3 Down() const;
    Float3 Left() const;
    Float3 Right() const;
    Float3 Forward() const;
    Float3 Back() const;

    void SetXBasis(const Float3& x);
    void SetYBasis(const Float3& y);
    void SetZBasis(const Float3& z);

    static Float3x3 Transpose(const Float3x3& m);
    static Float3x3 Invert(const Float3x3& m);
    static Float3x3 ScaleMatrix(float s);
    static Float3x3 ScaleMatrix(const Float3& s);
    static Float3x3 RotationAxisAngle(const Float3& axis, float angle);
    static Float3x3 RotationEuler(float x, float y, float z);

    DirectX::XMMATRIX ToSIMD() const;
};

struct Float4x4
{
    float _11, _12, _13, _14;
    float _21, _22, _23, _24;
    float _31, _32, _33, _34;
    float _41, _42, _43, _44;

    Float4x4();
    explicit Float4x4(const DirectX::XMFLOAT4X4& m);
    explicit Float4x4(DirectX::CXMMATRIX m);
    Float4x4(const Float4& r0, const Float4& r1, const Float4& r2, const Float4& r3);

    Float4x4& operator*=(const Float4x4& other);
    Float4x4 operator*(const Float4x4& other) const;

    Float3 Up() const;
    Float3 Down() const;
    Float3 Left() const;
    Float3 Right() const;
    Float3 Forward() const;
    Float3 Back() const;

    Float3 Translation() const;
    void SetTranslation(const Float3& t);

    void SetXBasis(const Float3& x);
    void SetYBasis(const Float3& y);
    void SetZBasis(const Float3& z);

    void Scale(const Float3& scale);

    static Float4x4 Identity();
    static Float4x4 Transpose(const Float4x4& m);
    static Float4x4 Invert(const Float4x4& m);
    static Float4x4 RotationAxisAngle(const Float3& axis, float angle);
    static Float4x4 RotationEuler(float x, float y, float z);
    static Float4x4 ScaleMatrix(float s);
    static Float4x4 ScaleMatrix(const Float3& s);
    static Float4x4 TranslationMatrix(const Float3& t);

    bool operator==(const Float4x4& other) const;
    bool operator!=(const Float4x4& other) const;

    DirectX::XMMATRIX ToSIMD() const;
    Float3x3 To3x3() const;
};

// Unsigned 32-bit integer vector classes
struct Uint2
{
    uint32_t x;
    uint32_t y;

    Uint2();
    Uint2(uint32_t x, uint32_t y);

    bool operator==(Uint2 other) const;
    bool operator!=(Uint2 other) const;
};

struct Uint3
{
    uint32_t x;
    uint32_t y;
    uint32_t z;

    Uint3();
    Uint3(uint32_t x, uint32_t y, uint32_t z);

    bool operator==(const Uint3& other) const;
    bool operator!=(const Uint3& other) const;
};

struct Uint4
{
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint32_t w;

    Uint4();
    Uint4(uint32_t x, uint32_t y, uint32_t z, uint32_t w);

    bool operator==(const Uint4& other) const;
    bool operator!=(const Uint4& other) const;
};

// Signed 32-bit integer vector classes
struct Int2
{
    int32_t x;
    int32_t y;

    Int2();
    Int2(int32_t x, int32_t y);

    bool operator==(Int2 other) const;
    bool operator!=(Int2 other) const;
};

struct Int3
{
    int32_t x;
    int32_t y;
    int32_t z;

    Int3();
    Int3(int32_t x, int32_t y, int32_t z);
};

struct Int4
{
    int32_t x;
    int32_t y;
    int32_t z;
    int32_t w;

    Int4();
    Int4(int32_t x, int32_t y, int32_t z, int32_t w);
};


// Conversion classes for reduced-precision representations
struct Half2
{
    uint16_t x;
    uint16_t y;

    Half2() : x(0), y(0)
    {
    }

    Half2(uint16_t x, uint16_t y) : x(x), y(y)
    {
    }

    Half2(float x, float y)
    {
        DirectX::PackedVector::XMStoreHalf2(reinterpret_cast<DirectX::PackedVector::XMHALF2*>(this), DirectX::XMVectorSet(x, y, 0.0f, 0.0f));
    }

    explicit Half2(const Float2& v)
    {
        DirectX::PackedVector::XMStoreHalf2(reinterpret_cast<DirectX::PackedVector::XMHALF2*>(this), v.ToSIMD());
    }

    DirectX::XMVECTOR ToSIMD() const
    {
        return DirectX::PackedVector::XMLoadHalf2(reinterpret_cast<const DirectX::PackedVector::XMHALF2*>(this));
    }

    Float2 ToFloat2() const
    {
        return Float2(ToSIMD());
    }
};

struct Half4
{
    uint16_t x;
    uint16_t y;
    uint16_t z;
    uint16_t w;

    Half4() : x(0), y(0), z(0), w(0)
    {
    }

    Half4(uint16_t x, uint16_t y, uint16_t z, uint16_t w) : x(x), y(y), z(z), w(w)
    {
    }

    Half4(float x, float y, float z, float w)
    {
        DirectX::PackedVector::XMStoreHalf4(reinterpret_cast<DirectX::PackedVector::XMHALF4*>(this), DirectX::XMVectorSet(x, y, z, w));
    }

    explicit Half4(const Float4& v)
    {
        DirectX::PackedVector::XMStoreHalf4(reinterpret_cast<DirectX::PackedVector::XMHALF4*>(this), v.ToSIMD());
    }

    DirectX::XMVECTOR ToSIMD() const
    {
        return DirectX::PackedVector::XMLoadHalf4(reinterpret_cast<const DirectX::PackedVector::XMHALF4*>(this));
    }

    Float3 ToFloat3() const
    {
        return Float3(ToSIMD());
    }

    Float4 ToFloat4() const
    {
        return Float4(ToSIMD());
    }
};

struct UByte4N
{
    uint32_t Bits;

    UByte4N() : Bits(0)
    {
    }

    explicit UByte4N(uint32_t bits) : Bits(bits)
    {
    }

    UByte4N(uint8_t x, uint8_t y, uint8_t z, uint8_t w)
    {
        Bits = x | (y << 8) | (z << 16) | (w << 14);
    }

    UByte4N(float x, float y, float z, float w)
    {
        DirectX::PackedVector::XMStoreUByteN4(reinterpret_cast<DirectX::PackedVector::XMUBYTEN4*>(this), DirectX::XMVectorSet(x, y, z, w));
    }

    explicit UByte4N(const Float4& v)
    {
        DirectX::PackedVector::XMStoreUByteN4(reinterpret_cast<DirectX::PackedVector::XMUBYTEN4*>(this), v.ToSIMD());
    }

    DirectX::XMVECTOR ToSIMD() const
    {
        return DirectX::PackedVector::XMLoadUByteN4(reinterpret_cast<const DirectX::PackedVector::XMUBYTEN4*>(this));
    }

    Float4 ToFloat4() const
    {
        return Float4(ToSIMD());
    }
};

struct UShort4N
{
    uint64_t Bits;

    UShort4N() : Bits(0)
    {
    }

    explicit UShort4N(uint32_t bits) : Bits(bits)
    {
    }

    UShort4N(uint16_t x, uint16_t y, uint16_t z, uint16_t w)
    {
        Bits = x | (y << 8ull) | (z << 16ull) | (w << 14ull);
    }

    UShort4N(float x, float y, float z, float w)
    {
        DirectX::PackedVector::XMStoreUShortN4(reinterpret_cast<DirectX::PackedVector::XMUSHORTN4*>(this), DirectX::XMVectorSet(x, y, z, w));
    }

    explicit UShort4N(const Float4& v)
    {
        DirectX::PackedVector::XMStoreUShortN4(reinterpret_cast<DirectX::PackedVector::XMUSHORTN4*>(this), v.ToSIMD());
    }

    DirectX::XMVECTOR ToSIMD() const
    {
        return DirectX::PackedVector::XMLoadUShortN4(reinterpret_cast<const DirectX::PackedVector::XMUSHORTN4*>(this));
    }

    Float4 ToFloat4() const
    {
        return Float4(ToSIMD());
    }
};

// Random number generation
class Random
{

public:

    void Roll(uint32_t numRolls);

    uint32_t RandomUint();
    float RandomFloat();
    Float2 RandomFloat2();

private:

    uint32_t x = 123456789;
    uint32_t y = 987654321;
    uint32_t z = 43219876;
    uint32_t c = 6543217;
};

template<typename T> void Swap(T& a, T& b)
{
    T tmp = a;
    a = b;
    b = tmp;
}

template<typename T> void Shuffle(std::vector<T>& values, Random& randomGenerator)
{
    const uint64_t count = values.size();
    for(uint64_t i = 0; i < count; ++i)
    {
        uint64_t other = i + (randomGenerator.RandomUint() % (count - i));
        Swap(values[i], values[other]);
    }
}

template<typename T> void Shuffle(T* values, uint64_t count, Random& randomGenerator)
{
    for(uint64_t i = 0; i < count; ++i)
    {
        uint64_t other = i + (randomGenerator.RandomUint() % (count - i));
        Swap(values[i], values[other]);
    }
}

// Constants
const float Pi = 3.141592654f;
const float Pi2 = 6.283185307f;
const float Pi_2 = 1.570796327f;
const float Pi_4 = 0.7853981635f;
const float InvPi = 0.318309886f;
const float InvPi2 = 0.159154943f;

// Max value that we can store in an fp16 buffer (actually a little less so that we have room for error, real max is 65504)
const float FP16Max = 65000.0f;

// Scale factor used for storing physical light units in fp16 floats (equal to 2^-10).
const float FP16Scale = 0.0009765625f;

const float FloatMax = std::numeric_limits<float>::max();
const float FloatInfinity = std::numeric_limits<float>::infinity();

// General math functions

// Linear interpolation
template<typename T> T Lerp(const T& x, const T& y, float s)
{
    return x + (y - x) * s;
}

template<typename T> T Min(T a, T b)
{
    return a < b ? a : b;
}

template<typename T> T Max(T a, T b)
{
    return a < b ? b : a;
}

// Clamps a value to the specified range
template<typename T> T Clamp(T val, T min, T max)
{
    if(val < min)
        val = min;
    else if(val > max)
        val = max;
    return val;
}

// Clamps a value to [0, 1]
template<typename T> T Saturate(T val)
{
    return Clamp<T>(val, T(0.0f), T(1.0f));
}

inline Float3 Saturate(Float3 val)
{
    Float3 result;
    result.x = Clamp<float>(val.x, 0.0f, 1.0f);
    result.y = Clamp<float>(val.y, 0.0f, 1.0f);
    result.z = Clamp<float>(val.z, 0.0f, 1.0f);
    return result;
}

// Rounds a float
inline float Round(float r)
{
    return (r > 0.0f) ? ::floorf(r + 0.5f) : ::ceilf(r - 0.5f);
}

// Returns x * x
template<typename T> T Square(T x)
{
    return x * x;
}

// Smoothstep cubic interpolation
inline float Smoothstep(float start, float end, float x)
{
    x = Saturate((x - start) / (end - start));
    return x * x * (3.0f - 2.0f * x);
}

inline Float3 Pow(Float3 x, float y)
{
    return Float3(::pow(x.x, y), ::pow(x.y, y), ::pow(x.z, y));
}

// linear -> sRGB conversion
inline Float3 LinearTosRGB(Float3 color)
{
    Float3 x = color * 12.92f;
    Float3 y = 1.055f * Pow(color, 1.0f / 2.4f) - 0.055f;

    Float3 clr = color;
    clr.x = color.x < 0.0031308f ? x.x : y.x;
    clr.y = color.y < 0.0031308f ? x.y : y.y;
    clr.z = color.z < 0.0031308f ? x.z : y.z;

    return clr;
}

inline float DegToRad(float deg)
{
    return deg * (1.0f / 180.0f) * 3.14159265359f;
}

inline float RadToDeg(float rad)
{
    return rad * (1.0f / 3.14159265359f) * 180.0f;
}

// sRGB -> linear conversion
inline Float3 SRGBToLinear(Float3 color)
{
    Float3 x = color / 12.92f;
    Float3 y = Pow((color + 0.055f) / 1.055f, 2.4f);

    Float3 clr = color;
    clr.x = color.x <= 0.04045f ? x.x : y.x;
    clr.y = color.y <= 0.04045f ? x.y : y.y;
    clr.z = color.z <= 0.04045f ? x.z : y.z;

    return clr;
}

inline float ComputeLuminance(Float3 color)
{
    return Float3::Dot(color, Float3(0.299f, 0.587f, 0.114f));
}

// Convert from spherical coordinates to Cartesian coordinates(x, y, z)
// Theta represents how far away from the zenith (north pole/+Y) and phi represents how far
// away from the 'right' axis (+X).
inline void SphericalToCartesianXYZYUP(float r, float theta, float phi, Float3& xyz)
{
    xyz.x = r * ::cosf(phi) * ::sinf(theta);
    xyz.y = r * ::cosf(theta);
    xyz.z = r * ::sinf(theta) * ::sinf(phi);
}

// Convert from spherical coordinates to Cartesian coordinates(x, y, z)
inline Float3 SphericalToCartesian(float azimuth, float elevation)
{
    Float3 xyz;
    xyz.x = ::cos(azimuth) * ::cos(elevation);
    xyz.y = ::sin(elevation);
    xyz.z = ::sin(azimuth) * ::cos(elevation);
    return xyz;
}

inline Float2 CartesianToSpherical(const Float3& xyz)
{
    float elevation = ::asin(xyz.y);

    float azimuth = ::atan2(xyz.z, xyz.x);
    if(azimuth < 0.0f)
        azimuth = 2.0f * Pi + azimuth;

    return Float2(azimuth, elevation);
}

}