# D3D12 Raytracing Samples
This collection of samples act as an introduction to DirectX Raytracing (DXR). The samples are divided into tutorials and advanced samples. Each tutorial sample introduces a few new DXR concepts. Advanced samples demonstrate more complex techniques and applications of raytracing. We will be adding more samples in the coming future, so check back. In addition, you can find more DXR samples tutorials at [Nvidia's DXR samples Github](https://github.com/NVIDIAGameWorks/DxrTutorials).

The samples are implemented using both DXR and D3D12 Raytracing Fallback Layer APIs. This is purely for demonstration purposes to show API differences. Real-world applications will implement only one or the other. The Fallback Layer uses DXR if a driver and OS supports it. Otherwise, it falls back to the compute pipeline to emulate raytracing.

### Getting Started
* DXR spec/documentation is available in the SDK package at [Getting Started with Raytracing](http://forums.directxtech.com/index.php?topic=5860.0) post.
* [Raytracing Fallback Layer documentation](../../../Libraries/D3D12RaytracingFallback/readme.md).

# Tutorial Samples
## 1. [Hello World Sample](src/D3D12RaytracingHelloWorld/readme.md)
This sample demonstrates how to setup a raytracing pipeline and render a triangle in screen space.

![D3D12 Raytracing Hello World GUI](src/D3D12RaytracingHelloWorld/Screenshot_small.png)

## 2. [Simple Lighting Sample](src/D3D12RaytracingSimpleLighting/readme.md)
This sample demonstrates how to do ray generation for a dynamic perspective camera and calculate simple diffuse shading for a cube from a dynamic point light. 

![D3D12 Raytracing Hello World GUI](src/D3D12RaytracingSimpleLighting/Screenshot_small.png)

## 3. [Procedural Geometry Sample](src/D3D12RaytracingProceduralGeometry/readme.md)
This sample demonstrates how to implement procedural geometry using intersection shaders and shows usage of more complex shader table layouts and multiple ray types. 

![D3D12 Raytracing Procedural Geometry GUI](src/D3D12RaytracingProceduralGeometry/Screenshot_small.png)

# Advanced Samples

## [MiniEngine Sample](src/D3D12RaytracingMiniEngineSample/readme.md)
This sample demonstrates integration of the Fallback Layer in the MiniEngine's Model Viewer and several sample uses of raytracing.

![D3D12 Raytracing Mini Engine](src/D3D12RaytracingMiniEngineSample/Screenshot_small.png)

## Requirements
* [Visual Studio 2017](https://www.visualstudio.com/) version 15.8.4 or higher.
* [Windows 10 October 2018 (17763) SDK](https://developer.microsoft.com/en-US/windows/downloads/windows-10-sdk).
* *Fallback Layer* requires:
  * Windows 10 with the Fall Creators Update or higher.
  * Developer Mode enabled in the OS.
  * Dx12 GPU with a driver that has retail DXIL support.
  * DXR Fallback Compiler - download [DirectXRaytracingBinariesV1.5.zip](https://github.com/Microsoft/DirectX-Graphics-Samples/releases/tag/v1.5-dxr) and copy all the contents to Samples/Desktop/D3D12Raytracing/tools/x64.
* *DirectX Raytracing* requires:
  * Windows 10 with the October 2018 update. 
  * Dx12 GPU with a compatible DirectX Raytracing driver.
    * Nvidia: driver version 415 or higher.
    * Other vendors - please consult the vendor you’re working with for HW and driver availability.

## Known limitations
  - NV 397.31+ drivers do not properly support compute Fallback Layer on Nvidia Volta. Samples have artifacts and/or scenes miss altogether. Use the recommended DXR / driver based raytracing mode of samples on this configuration instead.
  - AMD: current/v1.5 revision of the Fallback Layer is not supported on AMD cards and will fail to run. Temporarily, you can use previous v1.2 source code snapshot with v1.1 SDK overlay binary snapshot which work on AMD: 
    - [v1.2 source code zip snapshot from develop-dxr branch](https://github.com/Microsoft/DirectX-Graphics-Samples/releases/tag/v1.2-dxr) 
    - [v1.1 DXR SDK overlay binaries (DirectXRaytracingBinariesv1.1.zip) snapshot](https://github.com/Microsoft/DirectX-Graphics-Samples/releases/tag/v1.1-dxr)
    - Note D3D12RaytracingProceduralGeometry sample doesn't work on AMD.

## Feedback and Questions
We welcome all feedback, questions and discussions about DXR and the Fallback Layer at [DirectX Raytracing forums](http://forums.directxtech.com/index.php?PHPSESSID=394klvdd3683tt1fjkh2jteav1&board=248.0).
