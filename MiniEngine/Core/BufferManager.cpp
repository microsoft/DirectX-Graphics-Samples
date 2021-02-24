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
#include "BufferManager.h"
#include "Display.h"
#include "CommandContext.h"
#include "EsramAllocator.h"
#include "TemporalEffects.h"

namespace Graphics
{
    DepthBuffer g_SceneDepthBuffer;
    ColorBuffer g_SceneColorBuffer;
    ColorBuffer g_SceneNormalBuffer;
    ColorBuffer g_PostEffectsBuffer;
    ColorBuffer g_VelocityBuffer;
    ColorBuffer g_OverlayBuffer;
    ColorBuffer g_HorizontalBuffer;

    ShadowBuffer g_ShadowBuffer;

    ColorBuffer g_SSAOFullScreen(Color(1.0f, 1.0f, 1.0f));
    ColorBuffer g_LinearDepth[2];
    ColorBuffer g_MinMaxDepth8;
    ColorBuffer g_MinMaxDepth16;
    ColorBuffer g_MinMaxDepth32;
    ColorBuffer g_DepthDownsize1;
    ColorBuffer g_DepthDownsize2;
    ColorBuffer g_DepthDownsize3;
    ColorBuffer g_DepthDownsize4;
    ColorBuffer g_DepthTiled1;
    ColorBuffer g_DepthTiled2;
    ColorBuffer g_DepthTiled3;
    ColorBuffer g_DepthTiled4;
    ColorBuffer g_AOMerged1;
    ColorBuffer g_AOMerged2;
    ColorBuffer g_AOMerged3;
    ColorBuffer g_AOMerged4;
    ColorBuffer g_AOSmooth1;
    ColorBuffer g_AOSmooth2;
    ColorBuffer g_AOSmooth3;
    ColorBuffer g_AOHighQuality1;
    ColorBuffer g_AOHighQuality2;
    ColorBuffer g_AOHighQuality3;
    ColorBuffer g_AOHighQuality4;

    ColorBuffer g_DoFTileClass[2];
    ColorBuffer g_DoFPresortBuffer;
    ColorBuffer g_DoFPrefilter;
    ColorBuffer g_DoFBlurColor[2];
    ColorBuffer g_DoFBlurAlpha[2];
    StructuredBuffer g_DoFWorkQueue;
    StructuredBuffer g_DoFFastQueue;
    StructuredBuffer g_DoFFixupQueue;

    ColorBuffer g_MotionPrepBuffer;
    ColorBuffer g_LumaBuffer;
    ColorBuffer g_TemporalColor[2];
    ColorBuffer g_TemporalMinBound;
    ColorBuffer g_TemporalMaxBound;
    ColorBuffer g_aBloomUAV1[2];	// 640x384 (1/3)
    ColorBuffer g_aBloomUAV2[2];	// 320x192 (1/6)  
    ColorBuffer g_aBloomUAV3[2];	// 160x96  (1/12)
    ColorBuffer g_aBloomUAV4[2];	// 80x48   (1/24)
    ColorBuffer g_aBloomUAV5[2];	// 40x24   (1/48)
    ColorBuffer g_LumaLR;
    ByteAddressBuffer g_Histogram;
    ByteAddressBuffer g_FXAAWorkQueue;
    TypedBuffer g_FXAAColorQueue(DXGI_FORMAT_R11G11B10_FLOAT);

    // For testing GenerateMipMaps()
    ColorBuffer g_GenMipsBuffer;

    DXGI_FORMAT DefaultHdrColorFormat = DXGI_FORMAT_R11G11B10_FLOAT;
}

#define T2X_COLOR_FORMAT DXGI_FORMAT_R10G10B10A2_UNORM
#define HDR_MOTION_FORMAT DXGI_FORMAT_R16G16B16A16_FLOAT
#define DSV_FORMAT DXGI_FORMAT_D32_FLOAT

void Graphics::InitializeRenderingBuffers( uint32_t bufferWidth, uint32_t bufferHeight )
{
    GraphicsContext& InitContext = GraphicsContext::Begin();

    const uint32_t bufferWidth1 = (bufferWidth + 1) / 2;
    const uint32_t bufferWidth2 = (bufferWidth + 3) / 4;
    const uint32_t bufferWidth3 = (bufferWidth + 7) / 8;
    const uint32_t bufferWidth4 = (bufferWidth + 15) / 16;
    const uint32_t bufferWidth5 = (bufferWidth + 31) / 32;
    const uint32_t bufferWidth6 = (bufferWidth + 63) / 64;
    const uint32_t bufferHeight1 = (bufferHeight + 1) / 2;
    const uint32_t bufferHeight2 = (bufferHeight + 3) / 4;
    const uint32_t bufferHeight3 = (bufferHeight + 7) / 8;
    const uint32_t bufferHeight4 = (bufferHeight + 15) / 16;
    const uint32_t bufferHeight5 = (bufferHeight + 31) / 32;
    const uint32_t bufferHeight6 = (bufferHeight + 63) / 64;

    EsramAllocator esram;

    esram.PushStack();

        g_SceneColorBuffer.Create( L"Main Color Buffer", bufferWidth, bufferHeight, 1, DefaultHdrColorFormat, esram );
        g_SceneNormalBuffer.Create( L"Normals Buffer", bufferWidth, bufferHeight, 1, DXGI_FORMAT_R16G16B16A16_FLOAT, esram );
        g_VelocityBuffer.Create( L"Motion Vectors", bufferWidth, bufferHeight, 1, DXGI_FORMAT_R32_UINT );
        g_PostEffectsBuffer.Create( L"Post Effects Buffer", bufferWidth, bufferHeight, 1, DXGI_FORMAT_R32_UINT );

        esram.PushStack();	// Render HDR image

            g_LinearDepth[0].Create( L"Linear Depth 0", bufferWidth, bufferHeight, 1, DXGI_FORMAT_R16_UNORM );
            g_LinearDepth[1].Create( L"Linear Depth 1", bufferWidth, bufferHeight, 1, DXGI_FORMAT_R16_UNORM );
            g_MinMaxDepth8.Create(L"MinMaxDepth 8x8", bufferWidth3, bufferHeight3, 1, DXGI_FORMAT_R32_UINT, esram );
            g_MinMaxDepth16.Create(L"MinMaxDepth 16x16", bufferWidth4, bufferHeight4, 1, DXGI_FORMAT_R32_UINT, esram );
            g_MinMaxDepth32.Create(L"MinMaxDepth 32x32", bufferWidth5, bufferHeight5, 1, DXGI_FORMAT_R32_UINT, esram );

            g_SceneDepthBuffer.Create( L"Scene Depth Buffer", bufferWidth, bufferHeight, DSV_FORMAT, esram );

            esram.PushStack(); // Begin opaque geometry

                esram.PushStack();    // Begin Shading

                    g_SSAOFullScreen.Create( L"SSAO Full Res", bufferWidth, bufferHeight, 1, DXGI_FORMAT_R8_UNORM );

                    esram.PushStack();    // Begin generating SSAO
                        g_DepthDownsize1.Create( L"Depth Down-Sized 1", bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R32_FLOAT, esram );
                        g_DepthDownsize2.Create( L"Depth Down-Sized 2", bufferWidth2, bufferHeight2, 1, DXGI_FORMAT_R32_FLOAT, esram );
                        g_DepthDownsize3.Create( L"Depth Down-Sized 3", bufferWidth3, bufferHeight3, 1, DXGI_FORMAT_R32_FLOAT, esram );
                        g_DepthDownsize4.Create( L"Depth Down-Sized 4", bufferWidth4, bufferHeight4, 1, DXGI_FORMAT_R32_FLOAT, esram );
                        g_DepthTiled1.CreateArray( L"Depth De-Interleaved 1", bufferWidth3, bufferHeight3, 16, DXGI_FORMAT_R16_FLOAT, esram );
                        g_DepthTiled2.CreateArray( L"Depth De-Interleaved 2", bufferWidth4, bufferHeight4, 16, DXGI_FORMAT_R16_FLOAT, esram );
                        g_DepthTiled3.CreateArray( L"Depth De-Interleaved 3", bufferWidth5, bufferHeight5, 16, DXGI_FORMAT_R16_FLOAT, esram );
                        g_DepthTiled4.CreateArray( L"Depth De-Interleaved 4", bufferWidth6, bufferHeight6, 16, DXGI_FORMAT_R16_FLOAT, esram );
                        g_AOMerged1.Create( L"AO Re-Interleaved 1", bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R8_UNORM, esram );
                        g_AOMerged2.Create( L"AO Re-Interleaved 2", bufferWidth2, bufferHeight2, 1, DXGI_FORMAT_R8_UNORM, esram );
                        g_AOMerged3.Create( L"AO Re-Interleaved 3", bufferWidth3, bufferHeight3, 1, DXGI_FORMAT_R8_UNORM, esram );
                        g_AOMerged4.Create( L"AO Re-Interleaved 4", bufferWidth4, bufferHeight4, 1, DXGI_FORMAT_R8_UNORM, esram );
                        g_AOSmooth1.Create( L"AO Smoothed 1", bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R8_UNORM, esram );
                        g_AOSmooth2.Create( L"AO Smoothed 2", bufferWidth2, bufferHeight2, 1, DXGI_FORMAT_R8_UNORM, esram );
                        g_AOSmooth3.Create( L"AO Smoothed 3", bufferWidth3, bufferHeight3, 1, DXGI_FORMAT_R8_UNORM, esram );
                        g_AOHighQuality1.Create( L"AO High Quality 1", bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R8_UNORM, esram );
                        g_AOHighQuality2.Create( L"AO High Quality 2", bufferWidth2, bufferHeight2, 1, DXGI_FORMAT_R8_UNORM, esram );
                        g_AOHighQuality3.Create( L"AO High Quality 3", bufferWidth3, bufferHeight3, 1, DXGI_FORMAT_R8_UNORM, esram );
                        g_AOHighQuality4.Create( L"AO High Quality 4", bufferWidth4, bufferHeight4, 1, DXGI_FORMAT_R8_UNORM, esram );
                    esram.PopStack();	// End generating SSAO

                    g_ShadowBuffer.Create( L"Shadow Map", 2048, 2048, esram );

                esram.PopStack();	// End Shading

                esram.PushStack();	// Begin depth of field
                    g_DoFTileClass[0].Create(L"DoF Tile Classification Buffer 0", bufferWidth4, bufferHeight4, 1, DXGI_FORMAT_R11G11B10_FLOAT);
                    g_DoFTileClass[1].Create(L"DoF Tile Classification Buffer 1", bufferWidth4, bufferHeight4, 1, DXGI_FORMAT_R11G11B10_FLOAT);

                    g_DoFPresortBuffer.Create(L"DoF Presort Buffer", bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R11G11B10_FLOAT, esram );
                    g_DoFPrefilter.Create(L"DoF PreFilter Buffer", bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R11G11B10_FLOAT, esram );
                    g_DoFBlurColor[0].Create(L"DoF Blur Color", bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R11G11B10_FLOAT, esram );
                    g_DoFBlurColor[1].Create(L"DoF Blur Color", bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R11G11B10_FLOAT, esram );
                    g_DoFBlurAlpha[0].Create(L"DoF FG Alpha", bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R8_UNORM, esram );
                    g_DoFBlurAlpha[1].Create(L"DoF FG Alpha", bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R8_UNORM, esram );
                    g_DoFWorkQueue.Create(L"DoF Work Queue", bufferWidth4 * bufferHeight4, 4, esram );
                    g_DoFFastQueue.Create(L"DoF Fast Queue", bufferWidth4 * bufferHeight4, 4, esram );
                    g_DoFFixupQueue.Create(L"DoF Fixup Queue", bufferWidth4 * bufferHeight4, 4, esram );
                esram.PopStack();	// End depth of field

                g_TemporalColor[0].Create( L"Temporal Color 0", bufferWidth, bufferHeight, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);
                g_TemporalColor[1].Create( L"Temporal Color 1", bufferWidth, bufferHeight, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);
                TemporalEffects::ClearHistory(InitContext);

                g_TemporalMinBound.Create( L"Temporal Min Color", bufferWidth, bufferHeight, 1, DXGI_FORMAT_R11G11B10_FLOAT);
                g_TemporalMaxBound.Create( L"Temporal Max Color", bufferWidth, bufferHeight, 1, DXGI_FORMAT_R11G11B10_FLOAT);

                esram.PushStack();	// Begin motion blur
                    g_MotionPrepBuffer.Create( L"Motion Blur Prep", bufferWidth1, bufferHeight1, 1, HDR_MOTION_FORMAT, esram );
                esram.PopStack();	// End motion blur

            esram.PopStack();	// End opaque geometry

        esram.PopStack();	// End HDR image

        esram.PushStack();	// Begin post processing

            // This is useful for storing per-pixel weights such as motion strength or pixel luminance
            g_LumaBuffer.Create( L"Luminance", bufferWidth, bufferHeight, 1, DXGI_FORMAT_R8_UNORM, esram );
            g_Histogram.Create( L"Histogram", 256, 4, esram );

            // Divisible by 128 so that after dividing by 16, we still have multiples of 8x8 tiles.  The bloom
            // dimensions must be at least 1/4 native resolution to avoid undersampling.
            //uint32_t kBloomWidth = bufferWidth > 2560 ? Math::AlignUp(bufferWidth / 4, 128) : 640;
            //uint32_t kBloomHeight = bufferHeight > 1440 ? Math::AlignUp(bufferHeight / 4, 128) : 384;
            uint32_t kBloomWidth = bufferWidth > 2560 ? 1280 : 640;
            uint32_t kBloomHeight = bufferHeight > 1440 ? 768 : 384;

            esram.PushStack();	// Begin bloom and tone mapping
                g_LumaLR.Create( L"Luma Buffer", kBloomWidth, kBloomHeight, 1, DXGI_FORMAT_R8_UINT, esram );
                g_aBloomUAV1[0].Create( L"Bloom Buffer 1a", kBloomWidth,    kBloomHeight,    1, DefaultHdrColorFormat, esram );
                g_aBloomUAV1[1].Create( L"Bloom Buffer 1b", kBloomWidth,    kBloomHeight,    1, DefaultHdrColorFormat, esram);
                g_aBloomUAV2[0].Create( L"Bloom Buffer 2a", kBloomWidth/2,  kBloomHeight/2,  1, DefaultHdrColorFormat, esram );
                g_aBloomUAV2[1].Create( L"Bloom Buffer 2b", kBloomWidth/2,  kBloomHeight/2,  1, DefaultHdrColorFormat, esram );
                g_aBloomUAV3[0].Create( L"Bloom Buffer 3a", kBloomWidth/4,  kBloomHeight/4,  1, DefaultHdrColorFormat, esram );
                g_aBloomUAV3[1].Create( L"Bloom Buffer 3b", kBloomWidth/4,  kBloomHeight/4,  1, DefaultHdrColorFormat, esram );
                g_aBloomUAV4[0].Create( L"Bloom Buffer 4a", kBloomWidth/8,  kBloomHeight/8,  1, DefaultHdrColorFormat, esram );
                g_aBloomUAV4[1].Create( L"Bloom Buffer 4b", kBloomWidth/8,  kBloomHeight/8,  1, DefaultHdrColorFormat, esram );
                g_aBloomUAV5[0].Create( L"Bloom Buffer 5a", kBloomWidth/16, kBloomHeight/16, 1, DefaultHdrColorFormat, esram );
                g_aBloomUAV5[1].Create( L"Bloom Buffer 5b", kBloomWidth/16, kBloomHeight/16, 1, DefaultHdrColorFormat, esram );
            esram.PopStack();	// End tone mapping

            esram.PushStack();	// Begin antialiasing
                const uint32_t kFXAAWorkSize = bufferWidth * bufferHeight / 4 + 128;
                g_FXAAWorkQueue.Create( L"FXAA Work Queue", kFXAAWorkSize, sizeof(uint32_t), esram );
                g_FXAAColorQueue.Create( L"FXAA Color Queue", kFXAAWorkSize, sizeof(uint32_t), esram );
            esram.PopStack();	// End antialiasing

        esram.PopStack();	// End post processing

        esram.PushStack(); // GenerateMipMaps() test
            g_GenMipsBuffer.Create(L"GenMips", bufferWidth, bufferHeight, 0, DXGI_FORMAT_R11G11B10_FLOAT, esram );
        esram.PopStack();

        g_OverlayBuffer.Create( L"UI Overlay", g_DisplayWidth, g_DisplayHeight, 1, DXGI_FORMAT_R8G8B8A8_UNORM, esram );
        g_HorizontalBuffer.Create( L"Bicubic Intermediate", g_DisplayWidth, bufferHeight, 1, DefaultHdrColorFormat, esram );

    esram.PopStack(); // End final image

    InitContext.Finish();
}

void Graphics::ResizeDisplayDependentBuffers(uint32_t NativeWidth, uint32_t NativeHeight)
{
    (NativeWidth);
    g_OverlayBuffer.Create( L"UI Overlay", g_DisplayWidth, g_DisplayHeight, 1, DXGI_FORMAT_R8G8B8A8_UNORM );
    g_HorizontalBuffer.Create( L"Bicubic Intermediate", g_DisplayWidth, NativeHeight, 1, DefaultHdrColorFormat );
}

void Graphics::DestroyRenderingBuffers()
{
    g_SceneDepthBuffer.Destroy();
    g_SceneColorBuffer.Destroy();
    g_SceneNormalBuffer.Destroy();
    g_VelocityBuffer.Destroy();
    g_OverlayBuffer.Destroy();
    g_HorizontalBuffer.Destroy();
    g_PostEffectsBuffer.Destroy();

    g_ShadowBuffer.Destroy();

    g_SSAOFullScreen.Destroy();
    g_LinearDepth[0].Destroy();
    g_LinearDepth[1].Destroy();
    g_MinMaxDepth8.Destroy();
    g_MinMaxDepth16.Destroy();
    g_MinMaxDepth32.Destroy();
    g_DepthDownsize1.Destroy();
    g_DepthDownsize2.Destroy();
    g_DepthDownsize3.Destroy();
    g_DepthDownsize4.Destroy();
    g_DepthTiled1.Destroy();
    g_DepthTiled2.Destroy();
    g_DepthTiled3.Destroy();
    g_DepthTiled4.Destroy();
    g_AOMerged1.Destroy();
    g_AOMerged2.Destroy();
    g_AOMerged3.Destroy();
    g_AOMerged4.Destroy();
    g_AOSmooth1.Destroy();
    g_AOSmooth2.Destroy();
    g_AOSmooth3.Destroy();
    g_AOHighQuality1.Destroy();
    g_AOHighQuality2.Destroy();
    g_AOHighQuality3.Destroy();
    g_AOHighQuality4.Destroy();

    g_DoFTileClass[0].Destroy();
    g_DoFTileClass[1].Destroy();
    g_DoFPresortBuffer.Destroy();
    g_DoFPrefilter.Destroy();
    g_DoFBlurColor[0].Destroy();
    g_DoFBlurColor[1].Destroy();
    g_DoFBlurAlpha[0].Destroy();
    g_DoFBlurAlpha[1].Destroy();
    g_DoFWorkQueue.Destroy();
    g_DoFFastQueue.Destroy();
    g_DoFFixupQueue.Destroy();

    g_MotionPrepBuffer.Destroy();
    g_LumaBuffer.Destroy();
    g_TemporalColor[0].Destroy();
    g_TemporalColor[1].Destroy();
    g_TemporalMinBound.Destroy();
    g_TemporalMaxBound.Destroy();
    g_aBloomUAV1[0].Destroy();
    g_aBloomUAV1[1].Destroy();
    g_aBloomUAV2[0].Destroy();
    g_aBloomUAV2[1].Destroy();
    g_aBloomUAV3[0].Destroy();
    g_aBloomUAV3[1].Destroy();
    g_aBloomUAV4[0].Destroy();
    g_aBloomUAV4[1].Destroy();
    g_aBloomUAV5[0].Destroy();
    g_aBloomUAV5[1].Destroy();
    g_LumaLR.Destroy();
    g_Histogram.Destroy();
    g_FXAAWorkQueue.Destroy();
    g_FXAAColorQueue.Destroy();

    g_GenMipsBuffer.Destroy();
}
