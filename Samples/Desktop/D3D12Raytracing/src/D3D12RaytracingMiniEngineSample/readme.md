# D3D12 Raytracing MiniEngine Sample
![MiniEngine Screenshot](Screenshot.png)
## Raytracing Modifications

This is a modified version of MiniEngine that uses the D3D12 Raytracing Fallback Layer for a series of effects.

The keys '1'...'7' can also be used to cycle through different modes (or using Backspace to open up the MiniEngine and going to Application/Raytracing/RaytraceMode): 
* *Off* - [1] Full rasterization.
* *Bary Rays* - [2] Primary rays that return the barycentric of the intersected triangle.
* *Refl Bary* - [3] Secondary reflection rays that return the barycentric of the intersected triangle.
* *Shadow Rays* - [4] Secondary shadow rays are fired and return black/white depending on if a hit is found.
* *Diffuse&ShadowMaps* - [5] Primary rays are fired that calculate diffuse lighting and use a rasterized shadow map.
* *Diffuse&ShadowRays* - [6] Fully-raytraced pass that shoots primary rays for diffuse lights and recursively fires shadow rays.
* *Reflection Rays* - [7] Hybrid pass that renders primary diffuse with rasterization and if the ground plane is detected, fires of reflections rays.

## Controls:
* forward/backward/strafe: left thumbstick or WASD (FPS controls)
* up/down: triggers or E/Q
* yaw/pitch: right thumbstick or mouse
* freeze/unfreeze camera: F
* cycle through pre-set camera positions: right and left arrow keys
* toggle slow movement: click left thumbstick or lshift
* open debug menu: back button or backspace
* navigate debug menu: dpad or arrow keys
* toggle debug menu item: A button or return
* adjust debug menu value: dpad left/right or left/right arrow keys

## Limitations:
 * Currently only tested on the Compute-based Fallback Layer
 * Shadow pass is buggy due to incorrect ray generation
 * Mipmap calculation is incorrect and uses too low of an LOD for distant objects

## Requirements
* Consult the main [D3D12 Raytracing readme](../../readme.md) for the requirements.
