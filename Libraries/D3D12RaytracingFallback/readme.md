# D3D12 Raytracing Fallback Layer
The D3D12 Raytracing Fallback Layer is a library that emulates the DirectX Raytracing (DXR) API on devices without native driver/hardware support.

**[Windows October 2018 Update]** The Fallback Layer emulation runtime works with the final DXR API in Windows October 2018 update. The emulation feature proved useful during the experimental phase of DXR design. But as of the first shipping release of DXR, the plan is to stop maintaining this codebase.  The cost/benefit is not justified and further, over time as more native DXR support comes online, the value of emulation will diminish further. That said, the code functionality that was implemented is left in place for any further external contributions or if a strong justification crops up to resurrect the feature. We welcome external pull requests on extending or improving the Fallback Layer codebase.

## Overview
![MiniEngine Screenshot](../../Samples/Desktop/D3D12Raytracing/src/D3D12RaytracingMiniEngineSample/Screenshot.png)
The library is built as a wrapper around the DirectX 12 API and has distinct but similar interfaces from the DirectX Raytracing (DXR) API. The library also redirects to the DXR API when driver support exists, so developers do not need to branch code to support the DXR API simultaneously. 

The goal of the Fallback Layer is to enable developers to hit the ground running with the new DXR API without the need for a GPU with hardware support and a DXR capable variant of Windows. As of the GDC release, the Fallback Layer is in an early prototype stage and is a limited substitute for a DXR driver, particularly in terms of performance. A future goal of this project is to tune the Fallback to be performant enough for real-world scenarios in lieu of a DXR driver for small-scale raytracing techniques.


## Building the Fallback Layer
In order to build and run the Fallback Layer you will need:
* Windows 10 with the Fall Creators Update or higher.
* [Visual Studio 2017](https://www.visualstudio.com/) version 15.8.4 or higher.
* [Windows 10 Fall Creators Update SDK](https://developer.microsoft.com/en-US/windows/downloads/windows-10-sdk) or higher.
* A graphics card with retail DXIL capabilities.
* DXIL.dll should be pulled from the latest Windows SDK to accompany the compiler or enable Developer mode.
* DXR Fallback Compiler - download [DirectXRaytracingBinariesV1.5.zip](https://github.com/Microsoft/DirectX-Graphics-Samples/releases/tag/v1.5-dxr).

## Developing with the Fallback Layer
For details on developing an app using the Fallback Layer, please consult the [developer guide](developerguide.md)

## Driver requirements
* Retail support for DXIL
* Resource Binding Tier 3

## Contributing
See [contributing](contributing.md)
