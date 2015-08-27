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

// RGBM is a good way to pack HDR values into R8G8B8A8_UNORM
uint PackRGBM( float3 rgb, float MaxVal )
{
	rgb = saturate(rgb / MaxVal);
	float maxVal = max(max(1.0 / 255.0, rgb.x), max(rgb.y, rgb.z));
	float divisor = 255.0 / maxVal;
	uint R = (uint)(rgb.x * divisor);
	uint G = (uint)(rgb.y * divisor);
	uint B = (uint)(rgb.z * divisor);
	uint M = (uint)(maxVal * 255.0);
	return R << 24 | G << 16 | B << 8 | M;
}

float3 UnpackRGBM( uint p, float MaxVal )
{
	uint R = p >> 24;
	uint G = (p >> 16) & 0xFF;
	uint B = (p >> 8) & 0xFF;
	uint M = p & 0xFF;
	return float3(R, G, B) * (M * MaxVal) / (255.0f * 255.0f);
}

// The standard 32-bit HDR color format
uint Pack_R11G11B10_FLOAT( float3 rgb )
{
	uint r = (f32tof16(rgb.x) << 17) & 0xFFE00000;
	uint g = (f32tof16(rgb.y) << 6 ) & 0x001FFC00;
	uint b = (f32tof16(rgb.z) >> 5 ) & 0x000003FF;
	return r | g | b;
}

float3 Unpack_R11G11B10_FLOAT( uint rgb )
{
	float r = f16tof32((rgb >> 17) & 0x7FF0);
	float g = f16tof32((rgb >> 6 ) & 0x7FF0);
	float b = f16tof32((rgb << 5 ) & 0x7FE0);
	return float3(r, g, b);
}

// This is even better than R11G11B10_FLOAT for low dynamic range because it gives one more
// bit of precision to the mantissa without sacrificing precision in the blacks.  All
// exponents are non-positive with LDR, so we can simply discard its high bit.
// The easiest way to describe this format is that it's like R11G11B10 except that we limit
// the exponent from exceeding 4 bits.  The saved bit is donated to mantissa.
uint Pack_R11G11B10_LDR( float3 rgb )
{
	// Clamp to [0.0, 2.0).  That magic number is 1.FFFFF x 2^0.  We can't represent hex floats in HLSL.
	rgb = clamp( rgb, 0.0, asfloat(0x3FFFFFFF) );
	uint r = (f32tof16(rgb.x) << 18) & 0xFFE00000;
	uint g = (f32tof16(rgb.y) << 7 ) & 0x001FFC00;
	uint b = (f32tof16(rgb.z) >> 4 ) & 0x000003FF;
	return r | g | b;
}

float3 Unack_R11G11B10_LDR( uint rgb )
{
	float r = f16tof32((rgb >> 18) & 0x3FF8);
	float g = f16tof32((rgb >> 7 ) & 0x3FF8);
	float b = f16tof32((rgb << 4 ) & 0x3FF0);
	return float3(r, g, b);
}

// This is even better still for packing LDR values into 32-bits.  The idea here is that we
// mimic 7e3 float with a biased exponent, but we store an extra mantissa bit for red and green.
// It really doesn't get any better than this for LDR encoding, though 10-bit sRGB would offer
// similar quality with an even more expensive encoding.
// Let's guess at what this might be called:  "DXGI_FORMAT_R11G11_8E3_B10_7E3_FLOAT_BIASED_EXP"?
uint PackCustomLDR( float3 rgb )
{
	// Clamp to [0.0, 2.0).  Divide by 256 to bias the exponent by -8.  This shifts it down to use one
	// fewer bit while still taking advantage of the denormalization hardware.
	rgb = clamp( rgb, 0.0, asfloat(0x3FFFFFFF) ) / 256.0;
	uint r = (f32tof16(rgb.x) << 19) & 0xFFE00000;
	uint g = (f32tof16(rgb.y) << 8 ) & 0x001FFC00;
	uint b = (f32tof16(rgb.z) >> 3 ) & 0x000003FF;
	return r | g | b;
}

float3 UnpackCustomLDR( uint rgb )
{
	float r = f16tof32((rgb >> 19) & 0x1FFC);
	float g = f16tof32((rgb >> 8 ) & 0x1FFC);
	float b = f16tof32((rgb << 3 ) & 0x1FF8);
	return float3(r, g, b) * 256.0;
}
