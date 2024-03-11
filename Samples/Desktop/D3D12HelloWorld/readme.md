---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Direct3D 12 Hello, World! samples
urlFragment: d3d12-hello-world-samples-win32
description: This collection of samples act as an introduction to Direct3D 12. Each sample introduces an elementary concept, and lays a foundation for all of the Direct3D 12 samples.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Direct3D 12 Hello, World! samples
![HelloWorlds GUI](src/D3D12HelloWorlds.png)

This collection of samples act as an introduction to Direct3D 12. Each sample introduces an elementary concept, and lays a foundation for all of the Direct3D 12 samples.

## Hello, window! sample

This sample shows you how to create a window, Direct3D device (with debug layers enabled), and present to the window. These are the basic elements that every sample uses.

## Hello, triangle! sample

This sample shows you how to draw a static triangle using a vertex buffer.

## Hello, texture! sample

This sample shows you how to apply a Texture2D to triangle.

## Hello, bundles! sample

This sample shows you how to use Bundles to draw a static triangle more efficiently.

## Hello, constant buffers! sample

This sample shows you how to animate a triangle using a constant buffer.

## Hello, frame buffering! sample

This sample shows you how to use fences and multiple allocators to queue up multiple frames to the GPU.

## Hello, VADecode! sample

This sample shows you how to use Libva for Windows for decoding a basic H264 bitstream and rendering to screen. This sample depends on https://www.nuget.org/packages/Microsoft.Direct3D.VideoAccelerationCompatibilityPack/

## Hello, VAEncode! sample

This sample shows you how to use Libva for Windows to perform video processing effects and encode the D3D12 render targets presented to screen to a basic H264 encoded bitstream. This sample depends on https://www.nuget.org/packages/Microsoft.Direct3D.VideoAccelerationCompatibilityPack/

## Hello, VAResourceInterop! sample

This sample shows you how to use Libva for Windows for inter-operating between D3D12 resources and VA surfaces. This sample depends on https://www.nuget.org/packages/Microsoft.Direct3D.VideoAccelerationCompatibilityPack/

## Hello, Work Graphs! sample

This sample acts as an introduction to work graphs.

See the [D3D12 work graphs blog post](https://devblogs.microsoft.com/directx/d3d12-work-graphs/) for details.

In particular, descriptions of the samples are in that post [here](https://devblogs.microsoft.com/directx/d3d12-work-graphs/#Samples).

## Work Graphs Sandbox sample

This sample acts as a slightly more elaborate version of the above hello work graphs sample.

See the [D3D12 work graphs blog post](https://devblogs.microsoft.com/directx/d3d12-work-graphs/) for details.

In particular, descriptions of the samples are in that post [here](https://devblogs.microsoft.com/directx/d3d12-work-graphs/#Samples).

## Hello, Generic Programs! sample

This sample shows how to use generic programs in state objects as an alternative to previous ways of making pipeline state.  It also demonstrates using AddToStateObject() to make a new program resuing an existing shader along with a new one.

See the [D3D12 work graphs blog post](https://devblogs.microsoft.com/directx/d3d12-work-graphs/) for details.

In particular, descriptions of the samples are in that post [here](https://devblogs.microsoft.com/directx/d3d12-work-graphs/#Samples).

### Optional features
The Texture and Constant Buffer samples have been updated to build against the Windows 10 Anniversary Update SDK. In this SDK a new revision of Root Signatures is available for Direct3D 12 apps to use. Root Signature 1.1 allows for apps to declare when descriptors in a descriptor heap won't change or the data descriptors point to won't change.  This allows the option for drivers to make optimizations that might be possible knowing that something (like a descriptor or the memory it points to) is static for some period of time.