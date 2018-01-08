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

#include "Color.h"
#include "Math/Vector.h"
#include <string>

class Color;
class GraphicsContext;

namespace TextRenderer
{
    // Initialize the text renderer's resources and designate the dimensions of the drawable
    // view space.  These dimensions do not have to match the actual pixel width and height of
    // the viewport.  Instead they create a coordinate space for placing text within the
    // viewport.  For instance, if you specify a ViewWidth of 2.0f, then CursorX = 1.0f marks
    // the middle of the viewport.
    void Initialize( void );
    void Shutdown( void );

    class Font;
}

class TextContext
{
public:
    TextContext( GraphicsContext& CmdContext, float CanvasWidth = 1920.0f, float CanvasHeight = 1080.0f );

    GraphicsContext& GetCommandContext() const { return m_Context; }

    // Put settings back to the defaults.
    void ResetSettings( void );

    //
    // Control various text properties
    //

    // Choose a font from the Fonts folder.  Previously loaded fonts are cached in memory.
    void SetFont( const std::wstring& fontName, float TextSize = 0.0f );

    // Resize the view space.  This determines the coordinate space of the cursor position and font size.  You can always
    // set the view size to the same dimensions regardless of actual display resolution.  It is assumed, however, that the
    // aspect ratio of this virtual coordinate system matches the actual display aspect ratio.
    void SetViewSize( float ViewWidth, float ViewHeight );

    // Set the size of the text relative to the ViewHeight.  The aspect of the text is preserved from
    // the TTF as long as the aspect ratio of the view space is the same as the actual viewport.
    void SetTextSize( float PixelHeight );

    // Move the cursor position--the upper-left anchor for the text.
    void ResetCursor( float x, float y );
    void SetLeftMargin( float x );
    void SetCursorX( float x );
    void SetCursorY( float y );
    void NewLine( void );
    float GetLeftMargin( void );
    float GetCursorX( void );
    float GetCursorY( void );

    // Turn on or off drop shadow.
    void EnableDropShadow( bool enable );

    // Adjust shadow parameters.
    void SetShadowOffset( float xPercent, float yPercent );
    void SetShadowParams( float opacity, float width );

    // Set the color and transparency of text.
    void SetColor( Color color );

    // Get the amount to advance the Y position to begin a new line
    float GetVerticalSpacing( void );

    //
    // Rendering commands
    //

    // Begin and end drawing commands
    void Begin( bool EnableHDR = false );
    void End( void );

    // Draw a string
    void DrawString( const std::wstring& str );
    void DrawString( const std::string& str );

    // A more powerful function which formats text like printf().  Very slow by comparison, so use it
    // only if you're going to format text anyway.
    void DrawFormattedString( const wchar_t* format, ... );
    void DrawFormattedString( const char* format, ... );

private:

    __declspec(align(16)) struct VertexShaderParams
    {
        Math::Vector4 ViewportTransform;
        float NormalizeX, NormalizeY, TextSize;
        float Scale, DstBorder;
        uint32_t SrcBorder;
    };

    __declspec(align(16)) struct PixelShaderParams
    {
        Color TextColor;
        float ShadowOffsetX, ShadowOffsetY;
        float ShadowHardness;		// More than 1 will cause aliasing
        float ShadowOpacity;		// Should make less opaque when making softer
        float HeightRange;
    };

    void SetRenderState(void);

    // 16 Byte structure to represent an entire glyph in the text vertex buffer
    __declspec(align(16)) struct TextVert
    {
        float X, Y;				// Upper-left glyph position in screen space
        uint16_t U, V, W, H;	// Upper-left glyph UV and the width in texture space
    };

    UINT FillVertexBuffer( TextVert volatile* verts, const char* str, size_t stride, size_t slen );
    void DrawStringInternal( const std::string& str );
    void DrawStringInternal( const std::wstring& str );

    GraphicsContext& m_Context;
    const TextRenderer::Font* m_CurrentFont;
    VertexShaderParams m_VSParams;
    PixelShaderParams m_PSParams;
    bool m_VSConstantBufferIsStale;	// Tracks when the CB needs updating
    bool m_PSConstantBufferIsStale;	// Tracks when the CB needs updating
    bool m_TextureIsStale;
    bool m_EnableShadow;
    float m_LeftMargin;
    float m_TextPosX;
    float m_TextPosY;
    float m_LineHeight;
    float m_ViewWidth;				// Width of the drawable area
    float m_ViewHeight;				// Height of the drawable area
    float m_ShadowOffsetX;			// Percentage of the font's TextSize should the shadow be offset
    float m_ShadowOffsetY;			// Percentage of the font's TextSize should the shadow be offset
    BOOL m_HDR;
};
