# D3D12 Raytracing Shader Execution Reordering Sample - Sakura Scene
![D3D12 Raytracing SER](Screenshot.png)

This sample demonstrates the use of **Shader Execution Reordering (SER)** in a stylized **sakura (cherry blossom) forest** scene, composed of multiple geometry types including tree trunks, blossoms, bushes, and cubes for the floor.
 
SER is used to give hints to the GPU for grouping threads for better execution efficiency, based on three modes:
- **Sort by HitObject**
- **Sort by reflectHint**: A custom key derived from texture sampling on the floor, used to identify reflective regions such as dark crevices resembling water and cubes randomly in the space.
- **Sort by Both**: Combines HitObject and reflectHint .

The `reflectHint` is computed by sampling the floor texture at the estimated hit location. If the sampled color is sufficiently dark, the surface is treated as reflective, triggering additional shading logic such as Fresnel-based reflections. This technique mimics subtle water pooling effects in shaded areas.

**Note:**  

The scene applies a heavy workload to the skybox, which makes this element more expensive than in a typical game scenario. This characteristic contributes to the performance differences observed with SER in this sample. In particular, viewpoints where the sky is visible through gaps in the tree canopy exhibit highly variable shading costs. This variability is the kind of situation where SER can provide benefits. 

MaybeReorderThread() on `reflectHint` was introduced to demo another potential property that causes variability (e.g. reflections are much heavier than other workloads such as simple lighting calculations) SER could reorder on. In testing for this particular sample, this property showed minimal benefit on the RTX 5070; however, it had no noticeable impact on higher-end, albeit older architecture, GPUs like the RTX 4080 and 4090.
## Usage 
D3D12RaytracingBasicShaderExecutionReordering.exe 

Additional arguments:
  * [-forceAdapter \<ID>] - create a D3D12 device on an adapter \<ID>. Defaults to adapter 0.

### UI
The title bar of the sample provides runtime information:
* Name of the sample
* Frames per second
* Million Primary Rays/s: a number of dispatched rays per second calculated based of FPS.
* GPU[ID]: name
* SER: ON/OFF - toggles shader execution reordering feature on/off.

### Controls
* ALT+ENTER - toggles between windowed and fullscreen modes.
* P - toggles shader execution reordering on/off.
* H - MaybeReorderThread() based on HitObject
* M - MaybeReorderThread() based on reflectHint
* B - MaybeReorderThread() based on both HitObject and reflectHint
* WASD - Move through the scene (W: forward, A: left, S: backward, D: right)
* Q/E - Shift the camera vertically (Q: down, E: up)
* Arrow Keys - Look around (Up/Down: look up/down, Left/Right: look left/right)

## Requirements
* Windows 10 with the October 2019 update or higher.
* Consult the main [D3D12 Raytracing readme](../../readme.md) for further requirements.