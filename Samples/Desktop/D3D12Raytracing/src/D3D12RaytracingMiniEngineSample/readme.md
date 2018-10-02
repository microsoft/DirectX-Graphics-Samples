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
* forward/backward/strafe - left thumbstick or WASD (FPS controls).
* triggers or E/Q - camera up/down .
* right thumbstick or mouse - camera yaw/pitch.
* F - freeze/unfreeze camera. 
* right/left - cycle through pre-set camera positions.
* click left thumbstick or lshift - toggle slow movement.
* back button or backspace - open debug menu. 
* dpad or arrow keys - navigate debug menu.
* button or return - toggle debug menu item.
* dpad left/right or left/right arrow key - adjust debug menu values.

## Limitations:
 * Currently only tested on the Compute-based Fallback Layer
 * Shadow pass is buggy due to incorrect ray generation
 * Mipmap calculation is incorrect and uses too low of an LOD for distant objects
 * The sample has a few visual bugs on Intel cards

## Requirements
* Consult the main [D3D12 Raytracing readme](../../readme.md) for the requirements.
