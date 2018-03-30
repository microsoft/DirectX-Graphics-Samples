# D3D12 Raytracing Hello World sample
![D3D12 Raytracing Hello World GUI](Screenshot.png)

This sample demonstrates how to setup a raytracing pipeline and render a triangle in screenspace via raytracing. The sample implements three shaders: *ray generation*, *closest hit* and *miss* shader. The ray generation shader is executed for the whole render target via DispatchRays(). If a ray index corresponding to a pixel is inside a stencil window, it casts a ray into the scene. For ray indices outside the stencil window, the shader outputs color based on the ray's xy dispatch coordinates from top-left. Casted rays that hit the triangle render barycentric coordinates of the ray's hit position within the triangle. Missed rays render black.


### Usage
The sample starts with Fallback Layer implementation being used by default. The Fallback Layer will use raytracing driver if available, otherwise it will default to the compute fallback. This default behavior can be overriden via UI controls or input arguments.

D3D12RaytracingHelloWorld.exe [ -FL | -DXR ]
* [-FL] - select Fallback Layer API with forced compute fallback path.
* [-DXR] - select DirectX Raytracing API.

### UI
The title bar of the sample provides runtime information:
* Name of the sample
* Raytracing API being active:
  * FL - Fallback Layer with compute fallback being used
  * FL-DXR - Fallback Layer with raytracing driver being used
  * DXR - DirectX Raytracing being used
* Frames per second
* Million Primary Rays/s: a number of dispatched rays per second calculated based of FPS.

### Controls
* ALT+ENTER - toggles between windowed and fullscreen modes.
* 1 - select Fallback Layer API.
* 2 - select Fallback Layer API with forced compute fallback path.
* 3 - select DirectX Raytracing API.

## Requirements
* Consult the main [D3D12 Raytracing readme](../../readme.md) for the requirements.