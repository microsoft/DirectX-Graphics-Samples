# D3D12 Raytracing Procedural Geometry sample
![D3D12 Raytracing Procedural Geometry GUI](Screenshot.png)

This sample demonstrates how to implement procedural geometry using intersection shaders. Particularly, it showcases multiple intersection shaders creating analytic and volumetric, signed distance and fractal geometry. In addition, the sample introduces:
* Extended shader table layouts and indexing covering multiple geometries and bottom-level acceleration structures (AS).
* Use of trace ray recursion and two different ray types: color and shadow rays.

The sample assumes familiarity with Dx12 programming and DirectX raytracing concepts introduced in the [D3D12 Raytracing Simple Lighting sample](../D3D12RaytracingProceduralGeometry/readme.md).

##### Scene
The scene consists of triangle and procedural/AABB geometry each stored in a separate bottom-level AS:
* Triangle geometry - ground plane
* AABB geometry:
  * Analytic - multiple sphere and axis aligned box.
  * Volumetric - isosurface of metaballs (aka "blobs").
  * Signed distance - 6 different primitives and a pyramid fractal.

##### Ray types
The sample employs two ray types in the visulization: a color and a shadow ray. Color ray is used for primary/view and secondary/reflected ray TraceRay(). Shadow ray is used for visibility/occlusion testing and is more simpler since it all it does is to return value if it hits any or misses all objects. Given that, shadow rays is initialized with payload marking a hit and and RayFlags to skipp all but a miss shader, which will set payload value to a false (i.e. a light source is not blocked by any object):
```C++
    // Initialize shadow ray payload.
    // Set the initial value to true since closest and any hit shaders are skipped. 
    // Shadow miss shader, if called, will set it to false.
    ShadowRayPayload shadowPayload = { true };
    TraceRay(g_scene,
        /* RayFlags */
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES
        | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH
        | RAY_FLAG_FORCE_OPAQUE             // ~skip any hit shaders
        | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, // ~skip closest hit shaders
..
```

ToDo...

##### AABBs

##### Raytracing setup

T

## Usage
The sample starts with Fallback Layer implementation being used by default. The Fallback Layer will use raytracing driver if available, otherwise it will default to the compute fallback. This default behavior can be overriden via UI controls or input arguments.

D3D12RaytracingProceduralGeometry.exe [ -FL | -DXR | ...]
* [-FL] - select Fallback Layer API with forced compute fallback path.
* [-DXR] - select DirectX Raytracing API.

Additional arguments:
  * [-forceAdapter \<ID>] - create a D3D12 device on an adapter <ID>. Defaults to adapter 0.

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
* L - enable/disable light animation.
* C - enable/disable camera animation.

## Requirements
* Consult the main [D3D12 Raytracing readme](../../readme.md) for requirements.