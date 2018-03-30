# D3D12 Raytracing Fallback Layer
The D3D12 Raytracing Fallback Layer is a library that emulates the DirectX Raytracing API on devices without native driver/hardware support.

## Overview
![MiniEngine Screenshot](../../Samples/Desktop/D3D12Raytracing/src/D3D12RaytracingMiniEngineSample/Screenshot.png)
The library is built as a wrapper around the DirectX 12 API and has distinct but similar interfaces from the DirectX Raytracing (DXR) API. The library also redirects to the DXR API when driver support exists, so developers do not need to branch code to support the DXR API simultaneously. 

The goal of the Fallback Layer is to enable developers to hit the ground running with the new DXR API without the need for a GPU with hardware support and a DXR capable variant of Windows. As of the GDC release, the Fallback Layer is in an early prototype stage and is a limited substitute for a DXR driver, particularly in terms of performance. A future goal of this project is to tune the Fallback to be performant enough for real-world scenarios in lieu of a DXR driver for small-scale raytracing techniques.


## Building the Fallback Layer
In order to build and run the Fallback Layer you will need:
* [Visual Studio 2017](https://www.visualstudio.com/) with the [Windows 10 Fall Creators Update SDK](https://developer.microsoft.com/en-US/windows/downloads/windows-10-sdk)
* Windows 10 with the Fall Creators Update
* A graphics card with retail DXIL capabilities
* DXIL.dll should be pulled from the latest Windows SDK to accompany the compiler or enable Developer mode
* Download DirectXRaytracingBinariesV1.0.zip from https://github.com/Microsoft/DirectX-Graphics-Samples/releases and copy all the contents to Samples/Desktop/D3D12Raytracing/tools/x64

## API Samples
After cloning the project, you can open up D3D12Raytracing.sln in Samples/Desktop/D3D12Raytracing/src. There are several projects that demonstrate basic usage of the Fallback Layer/DXR API including a larger demo integrated with the MiniEngine. More details in the [readme](../../Samples/Desktop/D3D12Raytracing/readme.md)

## Developing with the Fallback Layer
For details on developing an app using the Fallback Layer, please consult the [developer guide](developerguide.md)

## Driver requirements
* Retail support for DXIL
* Resource Binding Tier 3

## Contributing
See [contributing](contributing.md)

## Feedback
We would love to hear how you're using the Fallback Layer! We're also interested in any form of feedback you may have:
* How many rays per sec would make the Fallback Layer usable in practice for your scenario? Real-time or offline?
* Are there functional issues blocking adoption of the Fallback Layer? Are the ways the interface could be less obstructive?
* Bugs/Issues using the Fallback Layer?

Please feel free to reach out at [DirectX Raytracing forums](http://forums.directxtech.com/index.php?PHPSESSID=394klvdd3683tt1fjkh2jteav1&board=248.0).
