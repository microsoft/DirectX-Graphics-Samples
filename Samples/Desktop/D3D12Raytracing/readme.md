# D3D12 Raytracing Samples
This collection of samples act as an introduction to Direct3D 12 Raytracing. Each sample introduces a few new concepts. 

The samples implement both D3D12 Raytracing and D3D12 Raytracing Fallback Layer APIs. This is purely for demonstration purposes to show where the API differences are. Real-world applications will implement only one or the other. The Fallback Layer uses D3D12 Raytracing if a driver and OS supports it. Otherwise, it falls back to compute pipeline to emulate raytracing. Developers aiming for a wider HW support should target the Fallback Layer.

## 1. Hello World Sample
This sample demonstrates how to setup a raytracing pipeline and render a triangle in screenspace.
![D3D12 Raytracing Hello World GUI](src\D3D12RaytracingHelloWorld\Screenshot_small.png)
