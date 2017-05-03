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

#include <DirectXMath.h>

using namespace DirectX;

class Color
{
public:
    Color( ) : m_value(g_XMOne) {}
    Color( FXMVECTOR vec );
    Color( const XMVECTORF32& vec );
    Color( float r, float g, float b, float a = 1.0f );
    Color( uint16_t r, uint16_t g, uint16_t b, uint16_t a = 255, uint16_t bitDepth = 8 );
    explicit Color( uint32_t rgbaLittleEndian );
        
    float R() const { return XMVectorGetX(m_value); }
    float G() const { return XMVectorGetY(m_value); }
    float B() const { return XMVectorGetZ(m_value); }
    float A() const { return XMVectorGetW(m_value); }

    bool operator==( const Color& rhs ) const { return XMVector4Equal(m_value, rhs.m_value); }
    bool operator!=( const Color& rhs ) const { return !XMVector4Equal(m_value, rhs.m_value); }

    void SetR( float r ) { m_value = XMVectorSetX(m_value, r); }
    void SetG( float g ) { m_value = XMVectorSetY(m_value, g); }
    void SetB( float b ) { m_value = XMVectorSetZ(m_value, b); }
    void SetA( float a ) { m_value = XMVectorSetW(m_value, a); }

    float* GetPtr( void ) { return reinterpret_cast<float*>(this); }
    float& operator[]( int idx ) { return GetPtr()[idx]; }

    void SetRGB( float r, float g, float b ) { m_value = XMVectorSelect( m_value, XMVectorSet(r, g, b, b), g_XMMask3 ); }

    Color ToSRGB() const;
    Color FromSRGB() const;
    Color ToREC709() const;
    Color FromREC709() const;

    // Probably want to convert to sRGB or Rec709 first
    uint32_t R10G10B10A2() const;
    uint32_t R8G8B8A8() const;

    // Pack an HDR color into 32-bits
    uint32_t R11G11B10F(bool RoundToEven=false) const;

    operator XMVECTOR() const { return m_value; }

private:
    XMVECTOR m_value;
};

INLINE Color Max( Color a, Color b ) { return Color(XMVectorMax(a, b)); }
INLINE Color Min( Color a, Color b ) { return Color(XMVectorMin(a, b)); }
INLINE Color Clamp( Color x, Color a, Color b ) { return Color(XMVectorClamp(x, a, b)); }


inline Color::Color( FXMVECTOR vec )
{
    m_value = vec;
}

inline Color::Color( const XMVECTORF32& vec )
{
    m_value = (XMVECTOR)vec;
}

inline Color::Color( float r, float g, float b, float a )
{
    m_value = XMVectorSet(r, g, b, a);
}

inline Color::Color( uint16_t r, uint16_t g, uint16_t b, uint16_t a, uint16_t bitDepth )
{
    m_value = XMVectorScale(XMVectorSet(r, g, b, a), 1.0f / ((1 << bitDepth) - 1));
}

inline Color::Color( uint32_t u32 )
{
    float r = (float)((u32 >>  0) & 0xFF);
    float g = (float)((u32 >>  8) & 0xFF);
    float b = (float)((u32 >> 16) & 0xFF);
    float a = (float)((u32 >> 24) & 0xFF);
    m_value = XMVectorScale( XMVectorSet(r, g, b, a), 1.0f / 255.0f );
}

inline Color Color::ToSRGB( void ) const
{
    XMVECTOR T = XMVectorSaturate(m_value);
    XMVECTOR result = XMVectorSubtract(XMVectorScale(XMVectorPow(T, XMVectorReplicate(1.0f / 2.4f)), 1.055f), XMVectorReplicate(0.055f));
    result = XMVectorSelect(result, XMVectorScale(T, 12.92f), XMVectorLess(T, XMVectorReplicate(0.0031308f)));
    return XMVectorSelect(T, result, g_XMSelect1110);
}

inline Color Color::FromSRGB( void ) const
{
    XMVECTOR T = XMVectorSaturate(m_value);
    XMVECTOR result = XMVectorPow(XMVectorScale(XMVectorAdd(T, XMVectorReplicate(0.055f)), 1.0f / 1.055f), XMVectorReplicate(2.4f));
    result = XMVectorSelect(result, XMVectorScale(T, 1.0f / 12.92f), XMVectorLess(T, XMVectorReplicate(0.0031308f)));
    return XMVectorSelect(T, result, g_XMSelect1110);
}

inline Color Color::ToREC709( void ) const
{
    XMVECTOR T = XMVectorSaturate(m_value);
    XMVECTOR result = XMVectorSubtract(XMVectorScale(XMVectorPow(T, XMVectorReplicate(0.45f)), 1.099f), XMVectorReplicate(0.099f));
    result = XMVectorSelect(result, XMVectorScale(T, 4.5f), XMVectorLess(T, XMVectorReplicate(0.0018f)));
    return XMVectorSelect(T, result, g_XMSelect1110);
}

inline Color Color::FromREC709( void ) const
{
    XMVECTOR T = XMVectorSaturate(m_value);
    XMVECTOR result = XMVectorPow(XMVectorScale(XMVectorAdd(T, XMVectorReplicate(0.099f)), 1.0f / 1.099f), XMVectorReplicate(1.0f / 0.45f));
    result = XMVectorSelect(result, XMVectorScale(T, 1.0f / 4.5f), XMVectorLess(T, XMVectorReplicate(0.0081f)));
    return XMVectorSelect(T, result, g_XMSelect1110);
}

inline uint32_t Color::R10G10B10A2( void ) const
{
    XMVECTOR result = XMVectorRound(XMVectorMultiply(XMVectorSaturate(m_value), XMVectorSet(1023.0f, 1023.0f, 1023.0f, 3.0f)));
    result = _mm_castsi128_ps(_mm_cvttps_epi32(result));
    uint32_t r = XMVectorGetIntX(result);
    uint32_t g = XMVectorGetIntY(result);
    uint32_t b = XMVectorGetIntZ(result);
    uint32_t a = XMVectorGetIntW(result) >> 8;
    return a << 30 | b << 20 | g << 10 | r;
}

inline uint32_t Color::R8G8B8A8( void ) const
{
    XMVECTOR result = XMVectorRound(XMVectorMultiply(XMVectorSaturate(m_value), XMVectorReplicate(255.0f)));
    result = _mm_castsi128_ps(_mm_cvttps_epi32(result));
    uint32_t r = XMVectorGetIntX(result);
    uint32_t g = XMVectorGetIntY(result);
    uint32_t b = XMVectorGetIntZ(result);
    uint32_t a = XMVectorGetIntW(result);
    return a << 24 | b << 16 | g << 8 | r;
}
