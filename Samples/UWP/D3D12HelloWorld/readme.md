---
page_type: sample
languages:
- cpp
products:
- windows
- windows-uwp
name: Direct3D 12 Hello, World! samples
urlFragment: d3d12-hello-world-samples-uwp
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

### Optional features
The Texture and Constant Buffer samples have been updated to build against the Windows 10 Anniversary Update SDK. In this SDK a new revision of Root Signatures is available for Direct3D 12 apps to use. Root Signature 1.1 allows for apps to declare when descriptors in a descriptor heap won't change or the data descriptors point to won't change.  This allows the option for drivers to make optimizations that might be possible knowing that something (like a descriptor or the memory it points to) is static for some period of time.