# D3D12 Raytracing Samples
This collection of samples act as an introduction to Direct3D 12 Raytracing. The samples are divided into tutorials and advanced samples. Each tutorial sample introduces a few new concepts. Advanced samples demonstrate more complex techniques and applications of raytracing.

The samples implement both D3D12 Raytracing and D3D12 Raytracing Fallback Layer APIs. This is purely for demonstration purposes to show where the API differences are. Real-world applications will implement only one or the other. The Fallback Layer uses D3D12 Raytracing if a driver and OS supports it. Otherwise, it falls back to compute pipeline to emulate raytracing. Developers aiming for a wider HW support should target the Fallback Layer. You can learn more about programming with the Fallback Layer and its differences from DirectX Raytracing in the library's [documentation](../../../Libraries\D3D12RaytracingFallback\readme.md).


## Feedback and Questions
We would love to hear how you're using DirectX Raytracing and the Fallback Layer. Reach us with any questions and discussions at  [DirectX Raytracing forums](http://forums.directxtech.com/index.php?PHPSESSID=394klvdd3683tt1fjkh2jteav1&board=248.0).


# Tutorial Samples
## 1. [Hello World Sample](src\D3D12RaytracingHelloWorld\readme.md)
This sample demonstrates how to setup a raytracing pipeline and render a triangle in screen space.

![D3D12 Raytracing Hello World GUI](src\D3D12RaytracingHelloWorld\Screenshot_small.png)

## 2. [Simple Lighting Sample](src\D3D12RaytracingSimpleLighting\readme.md)
This sample demonstrates how to do ray generation for a dynamic perspective camera and calculate simple diffuse shading for a cube from a dynamic point light. 

![D3D12 Raytracing Hello World GUI](src\D3D12RaytracingSimpleLighting\Screenshot_small.png)


# Advanced Samples

## [MiniEngine Sample](src\D3D12RaytracingMiniEngineSample\readme.md)
This sample demonstrates integration of the Fallback Layer in the MiniEngine's Model Viewer and several sample uses of raytracing.

![D3D12 Raytracing Mini Engine](src/D3D12RaytracingMiniEngineSample/Screenshot_small.png)

