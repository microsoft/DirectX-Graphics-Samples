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

#include "pch.h"
#include "TextRenderer.h"
#include "FileUtility.h"
#include "TextureManager.h"
#include "SystemTime.h"
#include "GraphicsCore.h"
#include "CommandContext.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "BufferManager.h"
#include "CompiledShaders/TextVS.h"
#include "CompiledShaders/TextAntialiasPS.h"
#include "CompiledShaders/TextShadowPS.h"
#include "Fonts/consola24.h"
#include <map>
#include <string>
#include <cstdio>
#include <memory>
#include <malloc.h>

using namespace Graphics;
using namespace Math;
using namespace std;

namespace TextRenderer
{
    class Font
    {
    public:
        Font()
        {
            m_NormalizeXCoord = 0.0f;
            m_NormalizeYCoord = 0.0f;
            m_FontLineSpacing = 0.0f;
            m_AntialiasRange = 0.0f;
            m_FontHeight = 0;
            m_BorderSize = 0;
            m_TextureWidth = 0;
            m_TextureHeight = 0;
        }

        ~Font()
        {
            m_Dictionary.clear();
        }

        void LoadFromBinary( const wchar_t* fontName, const uint8_t* pBinary, const size_t binarySize )
        {
            (fontName);

            // We should at least use this to assert that we have a complete file
            (binarySize);

            struct FontHeader
            {
                char FileDescriptor[8];		// "SDFFONT\0"
                uint8_t  majorVersion;		// '1'
                uint8_t  minorVersion;		// '0'
                uint16_t borderSize;		// Pixel empty space border width
                uint16_t textureWidth;		// Width of texture buffer
                uint16_t textureHeight;		// Height of texture buffer
                uint16_t fontHeight;		// Font height in 12.4
                uint16_t advanceY;			// Line height in 12.4
                uint16_t numGlyphs;			// Glyph count in texture
                uint16_t searchDist;		// Range of search space 12.4
            };

            FontHeader* header = (FontHeader*)pBinary;
            m_NormalizeXCoord = 1.0f / (header->textureWidth * 16);
            m_NormalizeYCoord = 1.0f / (header->textureHeight * 16);
            m_FontHeight = header->fontHeight;
            m_FontLineSpacing = (float)header->advanceY / (float)header->fontHeight;
            m_BorderSize = header->borderSize * 16;
            m_AntialiasRange = (float)header->searchDist / header->fontHeight;
            uint16_t textureWidth = header->textureWidth;
            uint16_t textureHeight = header->textureHeight;
            uint16_t NumGlyphs = header->numGlyphs;

            const wchar_t* wcharList = (wchar_t*)(pBinary + sizeof(FontHeader));
            const Glyph* glyphData = (Glyph*)(wcharList + NumGlyphs);
            const void* texelData = glyphData + NumGlyphs;

            for (uint16_t i = 0; i < NumGlyphs; ++i)
                m_Dictionary[wcharList[i]] = glyphData[i];

            m_Texture.Create( textureWidth, textureHeight, DXGI_FORMAT_R8_SNORM, texelData );

            DEBUGPRINT( "Loaded SDF font:  %ls (ver. %d.%d)", fontName, header->majorVersion, header->minorVersion);
        }

        bool Load( const wstring& fileName )
        {
            Utility::ByteArray ba = Utility::ReadFileSync( fileName );

            if (ba->size() == 0)
            {
                ERROR( "Cannot open file %ls", fileName.c_str() );
                return false;
            }

            LoadFromBinary( fileName.c_str(), ba->data(), ba->size() );

            return true;
        }

        // Each character has an XY start offset, a width, and they all share the same height
        struct Glyph
        {
            uint16_t x, y, w;
            int16_t bearing;
            uint16_t advance;
        };

        const Glyph* GetGlyph( wchar_t ch ) const
        {
            auto it = m_Dictionary.find( ch );
            return it == m_Dictionary.end() ? nullptr : &it->second;
        }

        // Get the texel height of the font in 12.4 fixed point
        uint16_t GetHeight( void ) const { return m_FontHeight; }

        // Get the size of the border in 12.4 fixed point
        uint16_t GetBorderSize( void ) const { return m_BorderSize; }

        // Get the line advance height given a certain font size
        float GetVerticalSpacing( float size ) const { return size * m_FontLineSpacing; }

        // Get the texture object
        const Texture& GetTexture( void ) const { return m_Texture; }

        float GetXNormalizationFactor() const { return m_NormalizeXCoord; }
        float GetYNormalizationFactor() const { return m_NormalizeYCoord; }

        // Get the range in terms of height values centered on the midline that represents a pixel
        // in screen space (according to the specified font size.)
        // The pixel alpha should range from 0 to 1 over the height range 0.5 +/- 0.5 * aaRange.
        float GetAntialiasRange( float size ) const { return Max( 1.0f, size * m_AntialiasRange ); }

    private:
        float m_NormalizeXCoord;
        float m_NormalizeYCoord;
        float m_FontLineSpacing;
        float m_AntialiasRange;
        uint16_t m_FontHeight;
        uint16_t m_BorderSize;
        uint16_t m_TextureWidth;
        uint16_t m_TextureHeight;
        Texture m_Texture;
        map<wchar_t, Glyph> m_Dictionary;
    };

    map< wstring, unique_ptr<Font> > LoadedFonts;

    const Font* GetOrLoadFont(const wstring& filename)
    {
        auto fontIter = LoadedFonts.find( filename );
        if (fontIter != LoadedFonts.end())
            return fontIter->second.get();

        Font* newFont = new Font();
        if (filename == L"default")
            newFont->LoadFromBinary(L"default", g_pconsola24, sizeof(g_pconsola24));
        else
            newFont->Load(L"Fonts/" + filename + L".fnt");
        LoadedFonts[filename].reset(newFont);
        return newFont;
    }

    RootSignature s_RootSignature;
    GraphicsPSO s_TextPSO[2];	// 0: R8G8B8A8_UNORM   1: R11G11B10_FLOAT
    GraphicsPSO s_ShadowPSO[2];	// 0: R8G8B8A8_UNORM   1: R11G11B10_FLOAT


} // namespace TextRenderer

void TextRenderer::Initialize( void )
{
    s_RootSignature.Reset(3, 1);
    s_RootSignature.InitStaticSampler(0, SamplerLinearClampDesc, D3D12_SHADER_VISIBILITY_PIXEL);
    s_RootSignature[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
    s_RootSignature[1].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
    s_RootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    s_RootSignature.Finalize(L"TextRenderer", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    // The glyph vertex description.  One vertex will correspond to a single character.
    D3D12_INPUT_ELEMENT_DESC vertElem[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT     , 0, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
        { "TEXCOORD", 0, DXGI_FORMAT_R16G16B16A16_UINT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }
    };

    s_TextPSO[0].SetRootSignature(s_RootSignature);
    s_TextPSO[0].SetRasterizerState( Graphics::RasterizerTwoSided );
    s_TextPSO[0].SetBlendState( Graphics::BlendPreMultiplied );
    s_TextPSO[0].SetDepthStencilState( Graphics::DepthStateDisabled );
    s_TextPSO[0].SetInputLayout(_countof(vertElem), vertElem);
    s_TextPSO[0].SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    s_TextPSO[0].SetVertexShader( g_pTextVS, sizeof(g_pTextVS) );
    s_TextPSO[0].SetPixelShader( g_pTextAntialiasPS, sizeof(g_pTextAntialiasPS) );
    s_TextPSO[0].SetRenderTargetFormats(1, &g_OverlayBuffer.GetFormat(), DXGI_FORMAT_UNKNOWN);
    s_TextPSO[0].Finalize();

    s_TextPSO[1] = s_TextPSO[0];
    s_TextPSO[1].SetRenderTargetFormats(1, &g_SceneColorBuffer.GetFormat(), DXGI_FORMAT_UNKNOWN);
    s_TextPSO[1].Finalize();

    s_ShadowPSO[0] = s_TextPSO[0];
    s_ShadowPSO[0].SetPixelShader(g_pTextShadowPS, sizeof(g_pTextShadowPS) );
    s_ShadowPSO[0].Finalize();

    s_ShadowPSO[1] = s_ShadowPSO[0];
    s_ShadowPSO[1].SetRenderTargetFormats(1, &g_SceneColorBuffer.GetFormat(), DXGI_FORMAT_UNKNOWN);
    s_ShadowPSO[1].Finalize();
}

void TextRenderer::Shutdown( void )
{
    LoadedFonts.clear();
}

TextContext::TextContext( GraphicsContext& CmdContext, float ViewWidth, float ViewHeight )
    : m_Context(CmdContext)
{
    m_HDR = FALSE;
    m_CurrentFont = nullptr;
    m_ViewWidth = ViewWidth;
    m_ViewHeight = ViewHeight;

    // Transform from text view space to clip space.
    const float vpX = 0.0f;
    const float vpY = 0.0f;
    const float twoDivW = 2.0f / ViewWidth;
    const float twoDivH = 2.0f / ViewHeight;
    m_VSParams.ViewportTransform = Vector4(twoDivW, -twoDivH, -vpX * twoDivW - 1.0f, vpY * twoDivH + 1.0f);

    // The font texture dimensions are still unknown
    m_VSParams.NormalizeX = 1.0f;
    m_VSParams.NormalizeY = 1.0f;

    ResetSettings();
}

void TextContext::ResetSettings( void )
{
    m_EnableShadow = true;
    ResetCursor(0.0f, 0.0f);
    m_ShadowOffsetX = 0.05f;
    m_ShadowOffsetY = 0.05f;
    m_PSParams.ShadowHardness = 0.5f;
    m_PSParams.ShadowOpacity = 1.0f;
    m_PSParams.TextColor = Color(1.0f, 1.0f, 1.0f, 1.0f);

    m_VSConstantBufferIsStale = true;
    m_PSConstantBufferIsStale = true;
    m_TextureIsStale = true;

    SetFont( L"default", 24.0f );
}

void  TextContext::SetLeftMargin( float x ) { m_LeftMargin = x; }
void  TextContext::SetCursorX( float x ) { m_TextPosX = x; }
void  TextContext::SetCursorY( float y ) { m_TextPosY = y; }
void  TextContext::NewLine( void ) { m_TextPosX = m_LeftMargin; m_TextPosY += m_LineHeight; }
float TextContext::GetLeftMargin( void ) { return m_LeftMargin; }
float TextContext::GetCursorX( void ) { return m_TextPosX; }
float TextContext::GetCursorY( void ) { return m_TextPosY; }


void TextContext::ResetCursor(float x, float y)
{
    m_LeftMargin = x;
    m_TextPosX = x;
    m_TextPosY = y;
}

void TextContext::EnableDropShadow(bool enable)
{
    if (m_EnableShadow == enable)
        return;

    m_EnableShadow = enable;

    m_Context.SetPipelineState( m_EnableShadow ? TextRenderer::s_ShadowPSO[m_HDR] : TextRenderer::s_TextPSO[m_HDR] );
}

void TextContext::SetShadowOffset(float xPercent, float yPercent)
{
    m_ShadowOffsetX = xPercent;
    m_ShadowOffsetY = yPercent;
    m_PSParams.ShadowOffsetX = m_CurrentFont->GetHeight() * m_ShadowOffsetX * m_VSParams.NormalizeX;
    m_PSParams.ShadowOffsetY = m_CurrentFont->GetHeight() * m_ShadowOffsetY * m_VSParams.NormalizeY;
    m_PSConstantBufferIsStale = true;
}

void TextContext::SetShadowParams(float opacity, float width)
{
    m_PSParams.ShadowHardness = 1.0f / width;
    m_PSParams.ShadowOpacity = opacity;
    m_PSConstantBufferIsStale = true;
}

void TextContext::SetColor( Color c )
{
    m_PSParams.TextColor = c;
    m_PSConstantBufferIsStale = true;
}

float TextContext::GetVerticalSpacing( void )
{
    return m_LineHeight;
}

void TextContext::Begin( bool EnableHDR )
{
    ResetSettings();

    m_HDR = (BOOL)EnableHDR;

    m_Context.SetRootSignature(TextRenderer::s_RootSignature);
    m_Context.SetPipelineState(TextRenderer::s_ShadowPSO[m_HDR]);
    m_Context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void TextContext::SetFont( const wstring& fontName, float size )
{
    // If that font is already set or doesn't exist, return.
    const TextRenderer::Font* NextFont = TextRenderer::GetOrLoadFont( fontName );
    if (NextFont == m_CurrentFont || NextFont == nullptr)
    {
        if (size > 0.0f)
            SetTextSize(size);

        return;
    }

    m_CurrentFont = NextFont;

    // Check to see if a new size was specified
    if (size > 0.0f)
        m_VSParams.TextSize = size;

    // Update constants directly tied to the font or the font size
    m_LineHeight = NextFont->GetVerticalSpacing( m_VSParams.TextSize );
    m_VSParams.NormalizeX = m_CurrentFont->GetXNormalizationFactor();
    m_VSParams.NormalizeY = m_CurrentFont->GetYNormalizationFactor();
    m_VSParams.Scale = m_VSParams.TextSize / m_CurrentFont->GetHeight();
    m_VSParams.DstBorder = m_CurrentFont->GetBorderSize() * m_VSParams.Scale;
    m_VSParams.SrcBorder = m_CurrentFont->GetBorderSize();
    m_PSParams.ShadowOffsetX = m_CurrentFont->GetHeight() * m_ShadowOffsetX * m_VSParams.NormalizeX;
    m_PSParams.ShadowOffsetY = m_CurrentFont->GetHeight() * m_ShadowOffsetY * m_VSParams.NormalizeY;
    m_PSParams.HeightRange = m_CurrentFont->GetAntialiasRange( m_VSParams.TextSize );
    m_VSConstantBufferIsStale = true;
    m_PSConstantBufferIsStale = true;
    m_TextureIsStale = true;
}

void TextContext::SetTextSize( float size )
{
    if (m_VSParams.TextSize == size)
        return;

    m_VSParams.TextSize = size;
    m_VSConstantBufferIsStale = true;

    if (m_CurrentFont != nullptr)
    {
        m_PSParams.HeightRange = m_CurrentFont->GetAntialiasRange( m_VSParams.TextSize );
        m_VSParams.Scale = m_VSParams.TextSize / m_CurrentFont->GetHeight();
        m_VSParams.DstBorder = m_CurrentFont->GetBorderSize() * m_VSParams.Scale;
        m_PSConstantBufferIsStale = true;
        m_LineHeight = m_CurrentFont->GetVerticalSpacing( size );
    }
    else
        m_LineHeight = 0.0f;
}

void TextContext::SetViewSize( float ViewWidth, float ViewHeight )
{
    m_ViewWidth = ViewWidth;
    m_ViewHeight = ViewHeight;

    const float vpX = 0.0f;
    const float vpY = 0.0f;
    const float twoDivW = 2.0f / ViewWidth;
    const float twoDivH = 2.0f / ViewHeight;

    // Essentially transform from screen coordinates to to clip space with W = 1.
    m_VSParams.ViewportTransform = Vector4(twoDivW, -twoDivH, -vpX * twoDivW - 1.0f, vpY * twoDivH + 1.0f);
    m_VSConstantBufferIsStale = true;
}

void TextContext::End( void )
{
    m_VSConstantBufferIsStale = true;
    m_PSConstantBufferIsStale = true;
    m_TextureIsStale = true;
}

void TextContext::SetRenderState( void )
{
    WARN_ONCE_IF(nullptr == m_CurrentFont, "Attempted to draw text without a font");

    if (m_VSConstantBufferIsStale)
    {
        m_Context.SetDynamicConstantBufferView(0, sizeof(m_VSParams), &m_VSParams);
        m_VSConstantBufferIsStale = false;
    }

    if (m_PSConstantBufferIsStale)
    {
        m_Context.SetDynamicConstantBufferView(1, sizeof(m_PSParams), &m_PSParams);
        m_PSConstantBufferIsStale = false;
    }

    if (m_TextureIsStale)
    {
        m_Context.SetDynamicDescriptors(2, 0, 1, &m_CurrentFont->GetTexture().GetSRV());
        m_TextureIsStale = false;
    }
}

// These are made with templates to handle char and wchar_t simultaneously.
UINT TextContext::FillVertexBuffer( TextVert volatile* verts, const char* str, size_t stride, size_t slen )
{
    UINT charsDrawn = 0;

    const float UVtoPixel = m_VSParams.Scale;

    float curX = m_TextPosX;
    float curY = m_TextPosY;

    const uint16_t texelHeight = m_CurrentFont->GetHeight();

    const char* iter = str;
    for (size_t i = 0; i < slen; ++i)
    {
        wchar_t wc = (stride == 2 ? *(wchar_t*)iter : *iter);
        iter += stride;

        // Terminate on null character (this really shouldn't happen with string or wstring)
        if (wc == L'\0')
            break;

        // Handle newlines by inserting a carriage return and line feed
        if (wc == L'\n')
        {
            curX = m_LeftMargin;
            curY += m_LineHeight;
            continue;
        }

        const TextRenderer::Font::Glyph* gi = m_CurrentFont->GetGlyph(wc);

        // Ignore missing characters
        if (nullptr == gi)
            continue;

        verts->X = curX + (float)gi->bearing * UVtoPixel;
        verts->Y = curY;
        verts->U = gi->x;
        verts->V = gi->y;
        verts->W = gi->w;
        verts->H = texelHeight;
        ++verts;

        // Advance the cursor position
        curX += (float)gi->advance * UVtoPixel;
        ++charsDrawn;
    }

    m_TextPosX = curX;
    m_TextPosY = curY;

    return charsDrawn;
}

void TextContext::DrawString( const std::wstring& str )
{
    SetRenderState();

    void* stackMem = _malloca((str.size() + 1) * 16);
    TextVert* vbPtr = Math::AlignUp((TextVert*)stackMem, 16);
    UINT primCount = FillVertexBuffer(vbPtr, (char*)str.c_str(), 2, str.size());

    if (primCount > 0)
    {
        m_Context.SetDynamicVB(0, primCount, sizeof(TextVert), vbPtr);
        m_Context.DrawInstanced( 4, primCount );
    }

    _freea(stackMem);
}

void TextContext::DrawString( const std::string& str )
{
    SetRenderState();

    void* stackMem = _malloca((str.size() + 1) * 16);
    TextVert* vbPtr = Math::AlignUp((TextVert*)stackMem, 16);
    UINT primCount = FillVertexBuffer(vbPtr, (char*)str.c_str(), 1, str.size());

    if (primCount > 0)
    {
        m_Context.SetDynamicVB(0, primCount, sizeof(TextVert), vbPtr);
        m_Context.DrawInstanced( 4, primCount );
    }

    _freea(stackMem);
}

void TextContext::DrawFormattedString( const wchar_t* format, ... )
{
    wchar_t buffer[256];
    va_list ap;
    va_start(ap, format);
    vswprintf( buffer, 256, format, ap );
    DrawString( wstring(buffer) );
}

void TextContext::DrawFormattedString( const char* format, ... )
{
    char buffer[256];
    va_list ap;
    va_start(ap, format);
    vsprintf_s( buffer, 256, format, ap );
    DrawString( string(buffer) );
}
