# Variable Rate Shading

## About this Document
This document describes Direct3D 12 exposure of graphics hardware functionality. 

*For now, this document is being temporarily distributed alongside the Direct3D samples while the more permanent place for it is being set up. Once the permanent location is set up, this document will be moved there. Please avoid taking dependencies on this document existing at this location.*

## Motivation
Due to performance constraints, graphics renderers cannot always afford to deliver the same quality level on every part of their output image. Variable rate shading, or coarse pixel shading, is a mechanism to enable allocation of rendering performance/power at varying rates across the rendered image.

Visually, there are cases where shading rate can be reduced with little or no reduction in perceptible output quality, leading to “free” performance.

## Today’s model: Multi-Sample Anti-Aliasing and Supersampling
Without variable rate shading, the only means of controlling shading rate is MSAA with sample-based execution, or supersampling. 

For context on what MSAA is— MSAA is a mechanism to reduce geometric aliasing and improve the rendering quality of an image as compared to non-MSAA. The MSAA sample count, which can be 1x, 2x, 4x, 8x, or 16x— governs the number of samples allocated per render target pixel. The MSAA sample count must be known up front when the target is allocated, and can’t be changed thereafter.

Supersampling causes the pixel shader to be invoked once per sample, at a higher quality but also higher performance cost compared to per-pixel execution.

Applications can control shading rate by choosing between per-pixel-based execution or MSAA-with-supersampling. These two choices don’t provide very fine control.  In cases such as such as objects behind HUD elements, transparencies, blurs (depth-of-field, motion, etc.), and optical distortions due to VR optics— a lower shading rate may be desired compared to the rest of the image, but it’s not possible because the shading quality and cost are fixed over the entire image. 

## New model:
The new model extends supersampling-with-MSAA into the opposite, “coarse pixel”, direction, by adding a new concept of coarse shading. This is where shading can be performed at a frequency coarser than a pixel. That is to say, a group of pixels can be shaded as a single unit and the result is then broadcast to all samples in the group.

A coarse shading API allows apps to specify the number of pixels that belong to a shaded group. The coarse pixel size can be varied after the render target is allocated. So, different portions of the screen or different draw passes can have different subsampling rates.

Following is a table describing which MSAA level is supported with which coarse pixel size; some are not supported on any platform, while others are conditionally enabled based on a cap 
(AdditionalShadingRatesSupported).

![coarsePixelSizeSupport](images/CoarsePixelSizeSupport.PNG "Coarse Pixel Size Support")

There exists a 2x4 cap and not 4x2 for 2x MSAA due to currently-known hardware plans across some platforms.

For what is exposed by Tiers 1 and 2, and in the current plan for this feature, there is no coarse-pixel-size-and-sample-count combination where hardware must track more than 16 samples per pixel shader invocation. For illustration purposes those are halftone-shaded above.

## Feature Tiering
A multiple-tier implementation is proposed. There are two user-queryable caps.

Features for each tier are described in greater detail below the table.

![tiers](images/Tiers.PNG "Tiers")

### Tier 1
* Shading rate can only be specified on a per-draw-basis; nothing more granular than that
* Shading rate applies uniformly to what is drawn independently of where it lies within the rendertarget

### Tier 2
* Shading rate can be specified on a per-draw-basis, as in Tier 1. It can also be specified by a combination of per-draw-basis, and of:
  * Semantic from the per-provoking-vertex, and
  * a screenspace image
* Shading rates from the three sources are combined using a set of combiners
* Screen space image tile size is 16x16 or smaller
* Shading rate requested by the app is guaranteed to be delivered exactly (for precision of temporal and other reconstruction filters) 
* SV_ShadingRate PS input is supported
* The per-provoking vertex rate, also referred to here as a per-primitive rate, is valid when one viewport is used and SV_ViewportIndex is not written to.
* The per-provoking vertex rate, also referred to as a per-primitive rate, can be used with more than one viewport if the SupportsPerVertexShadingRateWithMultipleViewports cap is marked *true*. Additionally, in that case, it can be used when SV_ViewportIndex is written to.

### List of Caps
* AdditionalShadingRatesSupported 
  * Boolean type
  * Indicates whether 2X4, 4X2, and 4X4 coarse pixel sizes are supported for single sampled rendering, and whether coarse pixel size 2X4 is supported for 2xMSAA.
* SupportsPerVertexShadingRateWithMultipleViewports
  * Boolean type
  * Indicates whether more than one viewport can be used with the per-vertex (also referred to as ‘per-primitive’) shading rate

## Specifying Shading Rate
For flexibility in applications, there are a variety of mechanisms provided to control the shading rate. Different mechanisms are available depending on the hardware feature tier.

### Pipeline State
This is the simplest mechanism for setting the shading rate, available on Tier 1 and above.

The app can specify a subsampling level in the command buffer. This API takes a single enum argument. The API provides an overall control of the level of quality for rendering— an ability to set the shading rate on a per-draw basis. 

Values for this state are expressed through the enumeration D3D12_SHADING_RATE.

#### Coarse pixel size support
The shading rates 1x1, 1x2, 2x2 and 2x2 are supported on all tiers.

There is a cap, AdditionalShadingRatesSupported, to indicate whether 2x4, 4x2, and 4x4 are supported on the device.

### Screen Space Image (image-based):
On Tier 2 and higher, pixel shading rate can be specified by a screen-space image.

The screen-space image allows the app to create an “LOD mask” image indicating regions of varying quality, such as areas which will be covered by motion blur, depth-of-field blur, transparent objects, or HUD UI elements. The resolution of the image is in macroblocks, not the resolution of the render target. In other words, the subsampling data is specified at a granularity of 8x8 or 16x16 pixel tiles as indicated by the VRS tile size.

#### Tile size
The app can query an API to know the supported VRS tile size for its device. 

Tiles are square, and the size refers to the tile’s width or height in texels.

If the hardware does not support Tier 2 variable rate shading, the capability query for the tile size will yield 0.

If the hardware does support Tier 2 variable rate shading, the tile size is one of
* 8
* 16
* 32

#### Screen space image size
For a render target of size {rtWidth, rtHeight}, using a given tile size called VRSTileSize, the screen space image that will cover it is of dimensions

```
 {ceil((float)rtWidth / VRSTileSize), ceil((float)rtHeight / VRSTileSize)}.
```

The screenspace image’s top left (0, 0) is locked to the render-target’s top left (0, 0).

To look up the (x,y) co-ordinate of a tile that corresponds to a particular location in the render target, divide the window-space co-ordinates of (x, y) by the tile size, ignoring fractional bits.

If the screen space image is larger than it needs to be for a given render target, the extra portions at the right and/or bottom are not used.

If the screen space image is too small for a given render target, any attempted read from the image beyond its actual extents yields a default shading rate of 1x1. Since the screenspace image’s top left (0, 0) is locked to the render-target’s top left (0, 0) and “reading beyond the render-target extents” means reading too-great of values for x and y.

#### Format, layout, resource properties
The format of this surface is a single-channel 8-bit surface (DXGI_FORMAT_R8_UINT).

The resource is dimension TEXTURE2D.

It cannot be arrayed or mipped. It must explicitly have one mip level.

It has sample count 1 and sample quality 0.

It has texture layout UNKNOWN. It implicitly cannot be row-major layout because cross-adapter is not allowed.

The expected way in which the screen-space image data is populated is to either
1.	Write the data using a compute shader; the screen-space image is bound as a UAV
2.	Copy the data to the screen-space image

When creating the screen-space image, the following flags are permitted:
* NONE
* ALLOW_UNORDERED_ACCESS
* DENY_SHADER_RESOURCE

The following flags are not permitted:
* ALLOW_RENDER_TARGET
* ALLOW_DEPTH_STENCIL
* ALLOW_CROSS_ADAPTER
* ALLOW_SIMULTANEOUS_ACCESS
* VIDEO_DECODE_REFERENCE_ONLY

The resource’s heap type cannot be UPLOAD or READBACK.

The resource cannot be SIMULTANEOUS_ACCESS. The resource is not allowed to be cross-adapter.

#### Data
Each byte of the screen space image corresponds to a value of the D3D12_SHADING_RATE enum.

#### Resource state
A resource needs to be transitioned into a read-only state when used as a screen-space image. A read-only state, D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE, is defined for this purpose. 

The image resource is transitioned out of that state to become writable again.

#### Setting the image
The screen-space image for specifying shader rate is set on the command list.

A resource which has been set as a shading rate source cannot be read or written from any shader stage.

A *null* screen-space image can be set for specifying shader rate. This has the effect of 1x1 being consistently used as contribution from the screen-space image. The screen-space image can initially be considered set to *null*.

#### Promotion and Decay
A screen-space image resource does not have any special implications with respect to promotion or decay.

### Per-Primitive Attribute
A per-primitive attribute adds the ability to specify a shading rate term as an attribute from a provoking vertex. This attribute is flat-shaded— that is, propagated to all pixels in the current triangle or line primitive. Use of a per-primitive attribute can enable finer-grained control of image quality compared to the other shading rate specifiers.

The per-primitive attribute is a settable semantic called SV_ShadingRate. SV_ShadingRate exists as part of shader model 6_4.

If a VS or GS sets SV_ShadingRate but VRS is not enabled, the semantic-setting has no effect.
If no value for SV_ShadingRate is specified per-primitive, a shading rate value of 1x1 is assumed as the per-primitive contribution.

### Combining Shading Rate Factors
The various sources of shading rate are applied in sequence using this diagram.

![combiners](images/Combiners.PNG "Combiners")

Each pair of A and B is combined using a combiner.

*When specifying a shader rate by vertex attribute:
* If a geometry shader is used, shading rate can be specified through that. 
* If a geometry shader is not used, the shading rate is specified by the provoking vertex.

#### List of combiners
The following combiners are supported. Using a Combiner (C) and two inputs (A and B):
* Passthrough. C.xy = A.xy
* Override. C.xy = B.xy
* Higher quality. C.xy = min(A.xy, B.xy)
* Lower quality. C.xy = max(A.xy, B.xy)
* Apply cost B relative to A. C.xy = min(maxRate, A.xy + B.xy)

where *maxRate* is the largest permitted dimension of coarse pixel on the device. This would be
* D3D12_AXIS_SHADING_RATE_2X (i.e., a value of 1), if AdditionalShadingRatesSupported is false
* D3D12_AXIS_SHADING_RATE_4X (i.e., a value of 2), if AdditionalShadingRatesSupported is true

The choice of combiner for variable rate shading is set on the command list through RSSetShadingRate.

If no combiners are ever set, they stay at the default which is passthrough.

If the source to a combiner is a D3D12_AXIS_SHADING_RATE which is not allowed in the support table, the input is sanitized to a shading rate that is supported.

If the output of a combiner does not correspond to a shading rate supported on the platform, the result is sanitized to a shading rate that is supported.

### Default State and State Clearing
All shading rate sources, namely
* the pipeline state-specified rate
* the screen space image-specified rate
* the per-primitive attribute

have a default of D3D12_SHADING_RATE_1X1. The default combiners are {PASSTHROUGH, PASSTHROUGH}.

If no screen space image is specified, a shading rate of 1X1 is inferred from that source.

If no per-primitive attribute is specified, a shading rate of 1X1 is inferred from that source.

ID3D12CommandList::ClearState resets the pipeline-state-specified rate to default, and the selection of screen-space image to the default of “no screen space image”.

## Querying Shading Rate— SV_ShadingRate
It’s useful to know what shading rate was selected by the hardware at any given pixel shader invocation. This could enable a variety of optimizations in the PS code. A PS-only system variable, SV_ShadingRate, provides information about the shading rate.

### Type
The type of this semantic is uint.

### Data Interpretation
The data is interpreted as D3D12_SHADING_RATE enum.

### If VRS Is Not Being Used
If coarse pixel shading is not being used, SV_ShadingRate is read back as a value of 1x1, indicating fine pixels.

### Behavior under Sample-Based Execution
A pixel shader fails compilation if it inputs SV_ShadingRate and also uses sample-based execution— for example, by inputting SV_SampleIndex or using the sample interpolation keyword. 

> ### Remarks on Deferred Shading
>
> Deferred shading applications’ lighting passes may need to know shading rate was used for which area of the screen. This is so their lighting pass dispatches can launch at a coarser rate. The SV_ShadingRate variable can be used to accomplish this if it is written out to the gbuffer.

## Depth and Stencil
When coarse pixel shading is used, depth and stencil and coverage are always computed and emitted at the full sample resolution.

## Using the Shading Rate Requested
For all tiers, it is expected that if a shading rate is requested, and it is supported on the device-and-MSAA-level-combination, then that is the shading rate delivered by the hardware.

A requested shading rate means a shading rate computed as an output of the combiners (see section "Combining Shading Rate Factors").

A supported shading rate is 1x1, 1x2, 2x1, or 2x2 in a rendering operation where the sample count is less than or equal to four. If the *AdditionalShadingRatesSupported* cap is true, then 2x4, 4x2, 4x4 are also supported shading rates for some sample counts (see table under section "New model").

## Screen-space Derivatives
Calculations of pixel-to-adjacent-pixel gradients are affected by coarse pixel shading. For example, when 2x2 coarse pixels are used, a gradient will be twice the size as compared to when coarse pixels are not used. Apps may want to adjust shaders to compensate for this— or not, depending on the desired functionality.

Because mips are chosen based on a screen-space derivative, the usage of coarse pixel shading affects mip selection. Usage of coarse pixel shading will cause lesser-detailed mips to be selected compared to when coarse pixels are not used.

## Attribute Interpolation
Inputs to a pixel shader may be interpolated based on their source vertices. Because variable rate shading affects the areas of the target written by each invocation of the pixel shader, it interacts with attribute interpolation. The three types of interpolation are center, centroid and sample.

### Center
The center interpolation location for a coarse pixel is the geometric center of the full coarse pixel area. 
SV_Position is always interpolated at center of the coarse pixel region.

### Centroid
When coarse pixel shading is used with MSAA, for each fine pixel there will still be writes to the full number of samples allocated for the target’s MSAA level. So, the centroid interpolation location will consider all samples for fine pixels within coarse pixels. That being said, the centroid interpolation location is defined as the first covered sample, in increasing order of sample index. The sample’s effective coverage is and-ed with the corresponding bit of the rasterizer state SampleMask. 

> ### Note
>
> When coarse pixel shading is used on Tier 1, SampleMask is always a full mask. If SampleMask is configured to not be a full mask, coarse pixel shading is disabled on Tier 1.

### Sample-based execution

Sample-based execution, or *supersampling*— which is caused by use of the sample interpolation feature— can be used with coarse pixel shading, and causes the pixel shader to be invoked per sample. For targets of sample count N, the pixel shader is invoked N times per fine pixel.

### EvaluateAttributeSnapped
Pull-model intrinsics are not compatible with coarse pixel shading on Tier 1. If there is an attempt to use pull-model intrinsics with coarse pixel shading on Tier 1, then coarse pixel shading is automatically disabled.

The intrinsic EvaluateAttributeSnapped is allowed to be used with coarse pixel shading on Tier 2. Its syntax is the same as it has always been:

```
numeric EvaluateAttributeSnapped(	
    in attrib numeric value, 
    in int2 offset);
```

For context: EvaluateAttributeSnapped has an offset parameter with two fields. When used without coarse pixel shading, just the lower-order four bits out of the full thirty-two are used. These four bits represent the range [-8, 7]. This range spans a 16x16 grid within a pixel. The range is such that the top and left edges of the pixel are included, and the bottom and right edges are not. Offset (-8, -8) is at the top-left corner, and offset (7, 7) is by the bottom-right corner. Offset (0, 0) is the center of the pixel.

When used with coarse pixel shading, EvaluateAttributeSnapped’s offset parameter is capable of specifying a wider range of locations. The offset parameter selects a 16x16 grid for each fine pixel, and there are multiple fine pixels. The expressible range and consequent number of bits used depends on the coarse pixel size. The top and left edges of the coarse pixel are included, and the bottom and right edges are not.

The table below describes the interpretation of EvaluateAttributeSnapped’s offset parameter for each coarse pixel size.

#### EvaluateAttributeSnapped’s Offset Range

|Coarse pixel size  |Indexable range             |Representable range size  |Number of bits needed {x, y}  |Binary mask of usable bits          |    
|------------------:|---------------------------:|-------------------------:|-----------------------------:|-----------------------------------:|    
|1x1 (fine)         |{\[-8, 7\], \[-8, 7\]}      |{16, 16}                  |{4, 4}                        |{000000000000xxxx, 000000000000xxxx}|    
|1x2                |{\[-8, 7\], \[-16, 15\]}    |{16, 32}                  |{4, 5}                        |{000000000000xxxx, 00000000000xxxxx}|    
|2x1                |{\[-16, 15\], \[-8, 7\]}    |{32, 16}                  |{5, 4}                        |{00000000000xxxxx, 000000000000xxxx}|    
|2x2                |{\[-16, 15\], \[-16, 15\]}  |{32, 32}                  |{5, 5}                        |{00000000000xxxxx, 00000000000xxxxx}|    
|2x4                |{\[-16, 15\], \[-32, 31\]}  |{32, 64}                  |{5, 6}                        |{00000000000xxxxx, 0000000000xxxxxx}|    
|4x2                |{\[-32, 31\], \[-16, 15\]}  |{64, 32}                  |{6, 5}                        |{0000000000xxxxxx, 00000000000xxxxx}|    
|4x4                |{\[-32, 31\], \[-32, 31\]}  |{64, 64}                  |{6, 6}                        |{0000000000xxxxxx, 0000000000xxxxxx}|   

The tables below are a guide for conversion to from the fixed-point to decimal and fractional representation. The first usable bit in the binary mask is the sign bit, and the rest of the binary mask comprise the numerical portion.

The number scheme for four-bit values passed into EvaluateAttributeSnapped is not anything new brought about by variable rate shading. It is reiterated here for completeness.  


For four-bit values:

| Binary value | Decimal  | Fractional |
|-------------:|---------:|-----------:|
|         1000 |-0.5f     |-8 / 16     |
|         1001 |-0.4375f  |-7 / 16|    |
|         1010 |-0.375f   |-6 / 16|    |
|         1011 |-0.3125f  |-5 / 16     |
|         1100 |-0.25f    |-4 / 16     |
|         1101 |-0.1875f  |-3 / 16     |
|         1110 |-0.125f   |-2 / 16     |
|         1111 |-0.0625f  |-1 /16      |
|         0000 |0.0f      |0 / 16      |
|         0001 |-0.0625f  |1 / 16      |
|         0010 |-0.125f   |2 / 16      |
|         0011 |-0.1875f  |3 / 16      |
|         0100 |-0.25f    |4 / 16      |
|         0101 |-0.3125f  |5 / 16      |
|         0110 |-0.375f   |6 / 16      |
|         0111 |-0.4375f  |7 / 16      |


For five-bit values:

| Binary value | Decimal  | Fractional |
|-------------:|---------:|-----------:|
|        10000 |-1        |-16 / 16    |
|        10001 |-0.9375   |-15 / 16    |
|        10010 |-0.875    |-14 / 16    |
|        10011 |-0.8125   |-13 / 16    |
|        10100 |-0.75     |-12 / 16    |
|        10101 |-0.6875   |-11 / 16    |
|        10110 |-0.625    |-10 / 16    |
|        10111 |-0.5625   |-9 / 16     |
|        11000 |-0.5      |-8 / 16     |
|        11001 |-0.4375   |-7 / 16     |
|        11010 |-0.375    |-6 / 16     |
|        11011 |-0.3125   |-5 / 16     |
|        11100 |-0.25     |-4 / 16     |
|        11101 |-0.1875   |-3 / 16     |
|        11110 |-0.125    |-2 / 16     |
|        11111 |-0.0625   |-1 / 16     |
|        00000 |0         |0 / 16      |
|        00001 |0.0625    |1 / 16      |
|        00010 |0.125     |2 / 16      |
|        00011 |0.1875    |3 / 16      |
|        00100 |0.25      |4 / 16      |
|        00101 |0.3125    |5 / 16      |
|        00110 |0.375     |6 / 16      |
|        00111 |0.4375    |7 / 16      |
|        01000 |0.5       |8 / 16      |
|        01001 |0.5625    |9 / 16      |
|        01010 |0.625     |10 / 16     |
|        01011 |0.6875    |11 / 16     |
|        01100 |0.75      |12 / 16     |
|        01101 |0.8125    |13 / 16     |
|        01110 |0.875     |14 / 16     |
|        01111 |0.9375    |15 / 16     |


For six-bit values:


| Binary value | Decimal  | Fractional |
|-------------:|---------:|-----------:|
|       100000 |-2        |-32 / 16    |
|       100001 |-1.9375   |-31 / 16    |
|       100010 |-1.875    |-30 / 16    |
|       100011 |-1.8125   |-29 / 16    |
|       100100 |-1.75     |-28 / 16    |
|       100101 |-1.6875   |-27 / 16    |
|       100110 |-1.625    |-26 / 16    |
|       100111 |-1.5625   |-25 / 16    |
|       101000 |-1.5      |-24 / 16    |
|       101001 |-1.4375   |-23 / 16    |
|       101010 |-1.375    |-22 / 16    |
|       101011 |-1.3125   |-21 / 16    |
|       101100 |-1.25     |-20 / 16    |
|       101101 |-1.1875   |-19 / 16    |
|       101110 |-1.125    |-18 / 16    |
|       101111 |-1.0625   |-17 / 16    |
|       110000 |-1        |-16 / 16    |
|       110001 |-0.9375   |-15 / 16    |
|       110010 |-0.875    |-14 / 16    |
|       110011 |-0.8125   |-13 / 16    |
|       110100 |-0.75     |-12 / 16    |
|       110101 |-0.6875   |-11 / 16    |
|       110110 |-0.625    |-10 / 16    |
|       110111 |-0.5625   |-9 / 16     |
|       111000 |-0.5      |-8 / 16     |
|       111001 |-0.4375   |-7 / 16     |
|       111010 |-0.375    |-6 / 16     |
|       111011 |-0.3125   |-5 / 16     |
|       111100 |-0.25     |-4 / 16     |
|       111101 |-0.1875   |-3 / 16     |
|       111110 |-0.125    |-2 / 16     |
|       111111 |-0.0625   |-1 / 16     |
|       000000 |0         |0 / 16      |
|       000001 |0.0625    |1 / 16      |
|       000010 |0.125     |2 / 16      |
|       000011 |0.1875    |3 / 16      |
|       000100 |0.25      |4 / 16      |
|       000101 |0.3125    |5 / 16      |
|       000110 |0.375     |6 / 16      |
|       000111 |0.4375    |7 / 16      |
|       001000 |0.5       |8 / 16      |
|       001001 |0.5625    |9 / 16      |
|       001010 |0.625     |10 / 16     |
|       001011 |0.6875    |11 / 16     |
|       001100 |0.75      |12 / 16     |
|       001101 |0.8125    |13 / 16     |
|       001110 |0.875     |14 / 16     |
|       001111 |0.9375    |15 / 16     |
|       010000 |1         |16 / 16     |
|       010001 |1.0625    |17 / 16     |
|       010010 |1.125     |18 / 16     |
|       010011 |1.1875    |19 / 16     |
|       010100 |1.25      |20 / 16     |
|       010101 |1.3125    |21 / 16     |
|       010110 |1.375     |22 / 16     |
|       010111 |1.4375    |23 / 16     |
|       011000 |1.5       |24 / 16     |
|       011001 |1.5625    |25 / 16     |
|       011010 |1.625     |26 / 16     |
|       011011 |1.6875    |27 / 16     |
|       011100 |1.75      |28 / 16     |
|       011101 |1.8125    |29 / 16     |
|       011110 |1.875     |30 / 16     |
|       011111 |1.9375    |31 / 16     |

In the same manner as with fine pixels, EvaluateAttributeSnapped’s grid of evaluate-able locations is centered at the coarse pixel center when using coarse pixel shading.

## SetSamplePositions
When the API SetSamplePositions is used with coarse shading, the API sets the sample positions for fine pixels.

## SV_Coverage
If SV_Coverage is declared as a shader input or output on Tier 1, coarse pixel shading is disabled.

The SV_Coverage semantic is usable with coarse pixel shading on Tier 2 and reflects which samples of an MSAA target are being written.

When coarse pixel shading is used— allowing multiple source pixels to comprise a tile— the coverage mask represents all samples that come from that tile.

Given coarse pixel shading’s compatibility with MSAA, the number of coverage bits required to be specified can vary. For example, with a 4x MSAA resource using D3D12_SHADING_RATE_2x2, each coarse pixel will write to four fine pixels, and each fine pixel has four samples. This means each coarse pixel will write to a total of 4 * 4 = 16 samples.

The following table indicates how many coverage bits are needed for each combination of coarse pixel size and MSAA level.

### Number of Coverage Bits Needed

![NumberOfCoverageBits](images/NumberOfCoverageBits.png "NumberOfCoverageBits")

As indicated in the above table, it is not possible to use coarse pixels to write to more than 16 samples at a time using the variable rate shading feature exposed through Direct3D12. This restriction is brought about by Direct3D12’s constraints regarding which MSAA levels are allowed with which coarse pixel size (see table under section "New model").

### Ordering and Format of Bits in the Coverage Mask
Bits of the coverage mask adhere to a well-defined order. The mask consists of the of coverages from pixels from left-to-right, then top-to-bottom (column-major) order. Coverage bits are the low-order bits of the coverage semantic and are densely packed together. 

The table below shows the coverage mask format for supported combinations of coarse pixel size and MSAA level.

![Coverage1x](images/Coverage1x.png "Coverage1x")

The following table portrays 2xMSAA pixels, where each pixel has two samples of indices 0 and 1.

The positioning of the labels of samples on the pixels are for illustrative purposes and do not necessarily convey the spatial {X, Y} locations of samples on that pixel; especially given sample positions can be programmatically changed. Samples are referred to by their 0-based index.

![Coverage2x](images/Coverage2x.png "Coverage2x")

The following table shows 4xMSAA pixels, where each pixel has four samples of indices 0, 1, 2, and 3.

![Coverage4x](images/Coverage4x.png "Coverage4x")

## Discard
When the HLSL semantic *discard* is used with coarse pixel shading, coarse pixels are discarded.

## TIR (Target Independent Rasterization)
TIR is not supported when subsampling is used.

## Raster Order Views
ROV interlocks are specified as operating at fine pixel granularity. If shading is performed per sample then interlocks are operating at sample granularity.

## Conservative Rasterization
Conservative rasterization is allowed to be used with variable rate shading. When conservative rasterization is used with coarse pixel shading, fine pixels within coarse pixels are conservatively rasterized by being given full coverage.

### Coverage
When conservative rasterization is used, the coverage semantic contains full masks for fine pixels that are covered and 0 for fine pixels that are not covered.

## Bundles
Variable-rate shading APIs can be called on bundles.

## Render Passes
Variable-rate shading APIs can be called in render passes.

## API
The following section describes the manner in which variable rate shading is accessible to applications through Direct3D12.

### Capability Querying

```
enum D3D12_SHADING_RATE_TIER
{
    D3D12_SHADING_RATE_TIER_NOT_SUPPORTED,
    D3D12_SHADING_RATE_TIER_1,
    D3D12_SHADING_RATE_TIER_2,
};

struct D3D12_FEATURE_DATA_D3D12_OPTIONS6
{
    BOOL AdditionalShadingRatesSupported; 
    BOOL PerPrimitiveShadingRateIndependentOfViewportUse;
    D3D12_SHADING_RATE_TIER ShadingRateTier;
    UINT ShadingRateImageTileSize;
};

enum D3D12_FEATURE
{
    // ...
    D3D12_FEATURE_D3D12_OPTIONS6
};

```

To query for Tier 1 capability, an application can, for example:

```
// Check that Tier 1 is supported
D3D12_FEATURE_DATA_D3D12_OPTIONS6 options;
return 
    SUCCEEDED(m_device->CheckFeatureSupport(
        D3D12_FEATURE_D3D12_OPTIONS6, 
        &options, 
        sizeof(options))) && 
    options.ShadingRateTier == D3D12_SHADING_RATE_TIER_1;

```

### Shading Rates

```
enum D3D12_SHADING_RATE
{
    D3D12_SHADING_RATE_1X1 = 0x0, // No change to shading rate
    D3D12_SHADING_RATE_1X2 = 0x1, // Reduces vertical resolution 2x
    D3D12_SHADING_RATE_2X1 = 0x4, // Reduces horizontal resolution 2x
    D3D12_SHADING_RATE_2X2 = 0x5, // Reduces both axes by 2x
    D3D12_SHADING_RATE_2X4 = 0x6, // Reduce horizontal res 2x, vertical by 4x
    D3D12_SHADING_RATE_4X2 = 0x9, // Reduce horizontal res 4x, vertical by 2x
    D3D12_SHADING_RATE_4X4 = 0xA, // Reduces both axes by 4x
};
```
The values are organized so that shading rates are easily decomposable into two axes, where each axis’s values are compactly represented in logarithmic space:

```
enum D3D12_AXIS_SHADING_RATE
{
    D3D12_AXIS_SHADING_RATE_1X = 0x0,
    D3D12_AXIS_SHADING_RATE_2X = 0x1, 
    D3D12_AXIS_SHADING_RATE_4X = 0x2
};
```

Allowing, for example

```
#define D3D12_MAKE_COARSE_SHADING_RATE(x,y) ((x) << 2 | (y))
D3D12_MAKE_COARSE_SHADING_RATE(
    D3D12_AXIS_SHADING_RATE_2X, 
    D3D12_AXIS_SHADING_RATE_1X)
```

As a convenience, there is also:

```
#define D3D12_GET_COARSE_SHADING_RATE_X_AXIS(x) ((x) >> 2 )
#define D3D12_GET_COARSE_SHADING_RATE_Y_AXIS(y) ((y) & 3 )
```

which can be used to dissect and understand SV_ShaderRate.

> #### Remarks
> 
> This data interpretation is geared toward describing the screenspace mask, which can be manipulated by shaders— discussed more in the sections above. But there’s no reason not to have a consistent definition of the coarse pixel sizes to be used everywhere, including when setting the command-level shading rate.

### Setting command-level shading rate and combiners
The shading rate, and optionally, combiners, are specified through a command list method.

```
enum D3D12_SHADING_RATE_COMBINER
{
    D3D12_SHADING_RATE_COMBINER_PASSTHROUGH,
    D3D12_SHADING_RATE_COMBINER_OVERRIDE,
    D3D12_SHADING_RATE_COMBINER_MIN,
    D3D12_SHADING_RATE_COMBINER_MAX,
    D3D12_SHADING_RATE_COMBINER_SUM,
};

class ID3D12GraphicsCommandList5: public ID3D12GraphicsCommandList4
{
public:
    // ...
    virtual void RSSetShadingRate(
        D3D12_SHADING_RATE baseShadingRate, 
        _In_reads_opt_(2) D3D12_SHADING_RATE_COMBINER* combiners) = 0;
};
```

### Preparing the screen space image
The read-only resource state designating a usable shading rate image is defined as

enum D3D12_RESOURCE_STATES
{
    // ...
    D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE
};

### Setting the screen space image
The screen space image is specified through a command list method.

```
class ID3D12GraphicsCommandList3: public ID3D12GraphicsCommandList2
{
public:
    // ...
    virtual void RSSetShadingRateImage(ID3D12Resource* image) = 0;
};
```

For example,
```
m_commandList->RSSetShadingRateImage(screenSpaceImage);
```

### Querying the tile size
The tile size is queried from a capability.

```
D3D12_FEATURE_DATA_D3D12_OPTIONS6 options;
return 
    SUCCEEDED(m_device->CheckFeatureSupport(
        D3D12_FEATURE_D3D12_OPTIONS6, 
        &options, 
        sizeof(options))) && 
    UINT tileSize = options.ShadingRateImageTileSize;
```
 
One dimension is retrieved, since the horizontal and vertical dimensions are always the same.
If the system’s capability is D3D12_SHADING_RATE_TIER_NOT_SUPPORTED, then the tile size returned is 0. 
