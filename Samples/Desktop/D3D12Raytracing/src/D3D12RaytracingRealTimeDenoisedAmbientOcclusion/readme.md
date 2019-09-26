# D3D12 Raytracing Real-Time Denoised Ambient Occlusion sample
![D3D12 Raytracing Real-Time Denoised Ambient Occlusion GUI](Screenshot.png)
**Figure** *Rendering of Raytraced Ambient Occlusion via our method. The AO is ray traced at 1 ray per pixel rate (left), and then spatio-temporally denoised with an edge-aware filter (right). The reconstruction successfully maintains a lot of detail while smoothing out much of the noise even at the low sampling rate.*

This sample presents a combination of established denoising techniques to bring raytraced Ambient Occlusion (AO) to within acceptable frame budget of AAA games on current and previous generation of DirectX Raytracing capable GPUs. With 60 frames-per-second being the standard for modern games, a rendering budget for effects such as AO can be as low as 1.5 ms per frame in modern game engines. The sample employs spatio-temporal accumulation and denoising of ray traced AO rays that are cast at 1 (or 0.5) sample rays per pixel (spp).

It can significantly improve the realism of rendered 3D scenes. AO is a cheap approximation for global illumination that is more appropriate for real-time graphics on a budget. It approximates the amount of indirect lighting that bounces around the scene and reaches a point on a surface. The premise of AO is that any occluders in a hemisphere around a surface point lower the ambient coefficient, making it appear darker. A surface lit with such dynamic indirect lighting, provides us with more cues about it's shape and placement among to other objects and, therefore, resulting in a visually more convincing look. While indirect lighting phenomena can be physically modeled via Global Illumination (GI) models, a full fledged GI solution can be very expensive and impractical for games on today's commodity graphics processing unit hardware (GPU). Rendering games at 60 Hz or more has become the standard for appealing gaming experience leaving 16.6ms or less for a game engine to render a frame. Unlike in movie production with much higher frame GPU budget, game engines generally for simplified GI models to meet the frame budget and trading for lower fidelity or approximations. 

Adding the cheaper AO to the games on top has become popular way of complementing the simpler GI models and achieving a more plausible look. Game engines have done this mostly either by prebaking it into textures, and thus, limiting it to static object layouts or via screen-space approximations, which can suffer because of limited geometry data available in screen space. Some limitations, include creating false dark halos, lack of occlusion due to geometry not being visible on screen, unstable occlusion around screen borders and generally limitation to only occlusions from objects close to the target. Raytraced AO, in contrast can avoid all these issues by simulating AO in a more natural way to solve the problem.  

The physically-based approach to estimating AO is via Monte Carlo sampling of the hemisphere around a normal of a point on a surface and testing for visibility by tracing the casted rays. Rays that hit any objects increase the occlusion value, inversely lowering the ambient lighting term making surfaces appear darker. Generating AO in this way can be very noisy requiring large number of rays per pixel to be cast to reach visually pleasing results. This number could be in hundreds of rays per pixel.

This sample implements a real-time denoiser of 1 sample ray per pixel (spp) raytraced Ambient Occlusion targetted at 60+ FPS apps. Denoising is a critical part to reach real-time raytraced effects on current-gen hardware. The implementation introduced here allows developers to have a starting point that they can experiment with and expand upon for their integrations. See the 2nd part of this readme for more details on the denoiser implementation. 

In addition, this DXR sample shows an implementation of:
* A specular physically-based pathtracer.
* Support for dynamic geometries in an acceleration structure. 
 
The sample assumes familiarity with Dx12 programming and DirectX Raytracing concepts introduced in the [D3D12 Raytracing Procedural Geometry sample](../D3D12RaytracingProceduralGeometry/readme.md). 

## Usage
D3D12RaytracingRealTimeDenoisedAmbientOcclusion.exe [...]
  * [-forceAdapter \<ID>] - create a D3D12 device on an adapter <ID>. Defaults to adapter 0.
  * [-vsync] - renders with VSync enabled.

The sample defaults to 1080p window size and 1080p RTAO. In practice, AO is done at quarter resolution as the 4x performance overhead generally doesn't justify the quality increase, especially on higher resolutions/dpis. Therefore, if you switch to higher window resolutions, such as 4K, also switch to quarter res RTAO via QuarterRes UI option to improve the performance.

### UI
The title bar of the sample provides runtime information:
* Name of the sample
* Frames per second
* GPU[ID]: name

### Controls
* ALT+ENTER - toggles between windowed and fullscreen modes.
* W,S - moves camera forward and back.
* A,D - moves camera to the side.
* Hold left mouse key and drag - rotate camera's focus at position.
* Hold right mouse key and drag - rotate scene.
* L - enable/disable light animation.
* C - enable/disable camera animation.
* T - toggles scene animation.
* Shift - toggles camera movement amplitude.
* 0 - Toggles Ground Truth spp vs 1 spp and switches to raw RTAO visualization.
* 1 - Raw/single frame RTAO visualization.
* 2 - Denoised RTAO visualization.
* 3 - Specular PBR Pathtracer + RTAO visualization.
* 4 - Toggles RTAO ray lengths - short | long.
* F9 - does a profiling pass. Renders 1000 frames, rotates camera 360 degrees and outputs GPU times to Profile.csv.
* space - pauses/resumes rendering.
* U/Y - moves car by the house back and forth.
* J/M - moves spaceship up and down.
* H/K - rotates spaceship around scene's center.
* ESC - terminate the application.

## Requirements
* "*AnyToAnyWaveReadLaneAt*" shaders require ReadLaneAt() with any to any wave read lane support. Tested on (Pascal & Turing). If your HW doesn't support it, you will need to replace those wave intrinsics.
* Requires DXR capable HW and SW. Consult the main [D3D12 Raytracing readme](../../readme.md) for requirements.

## Known Issues\Limitations
* Textures don't work/are correctly read on Debug config - the textured roof renders incorrectly. 
* UI "Sample set distribution across NxN pixels* set to true is only compatible with 1 spp (default). The distribution UI value will get forced to 1 if you select 2+ spp.

## Acknowledgements
* The grass straw generation is based on Emerald Engine implementation https://github.com/lragnarsson/Emerald-Engine
* The PBRT parser is based on Duos renderer implementation https://github.com/wallisc/DuosRenderer/tree/DXRRenderer/PBRTParser
* Assets used in this sample:
  * A pbrt-v3 version of 'Victoryan Style House' by MrChimp2313, downloaded from https://benedikt-bitterli.me/resources/, released under a [CC0 license](https://creativecommons.org/publicdomain/zero/1.0/).
  * A pbrt-v3 version of 'Pontiac GTO 67' by thecali, downloaded from https://benedikt-bitterli.me/resources/, released under a [CC0 license](https://creativecommons.org/publicdomain/zero/1.0/).
  * A pbrt-v3 version of 'Dragon' by Delatronic, downloaded from https://benedikt-bitterli.me/resources/, released under a [CC-BY license](https://creativecommons.org/licenses/by/3.0/).
  * A pbrt-v3 version of '4060.b Spaceship' by thecali, downloaded from https://benedikt-bitterli.me/resources/, released under a [CC0 license](https://creativecommons.org/publicdomain/zero/1.0/).
  * An environment map 'Flower Road', downloaded from https://hdrihaven.com/hdri/?c=outdoor&h=flower_road, released under a [CCO license](https://creativecommons.org/publicdomain/zero/1.0/).

See the sideloaded License.txt next to each asset for further license information.


# Sample implementation details
## AO raytracing
The sample supports two modes: standard 2D Dispatch Rays for 1spp and 1D Dispatch Rays for ray-sorted AO rays.

### Sampling
The random samples for sampling of a hemisphere with AO rays are generated with a cosine weighted multi-jittered sampler (See Section 5.3.4 in book Ray Tracing from the Ground Up). Given low spp requirements of the sample, the sampling quality of a hemisphere can be poor. Since the denoiser accumulates samples from local neighborhoods by blurring the image, it thus improves per pixel spp. The sample improves this further by having the sampler generate sample sets for each for NxN pixels (i.e. 8x8). Then each set of pixels randomply pick a unique sample from this set at raytrace time. This way the hemisphere sampling is improved across local neighborhoods making the local denoising even more effective at sampling the hemisphere. As long as the local neighborhoods (8x8 pixels) are spatially close, have same surface normal and denoiser blurs among them, the distribution is improved. 

## Ray sorting
AO rays have generally divergent ray directions at low spp. This severely affects the ray tracing performance, especially on Pascal architecture. We found the neighboring AO rays can be efficiently ray sorted by their ray direction to provide a boost of 10-40% of original runtime raytracing cost. The sample implements a compute based pre-sort pass for each 64x128 pixels and then dispatches the sorted rays with 1D DispatchRays. On Turing, the ray sorted RTAO is generally as fast as non-ray sorted AO due to the ray sorting overhead and because of the fact, the raytracing performance on Turing is better. The ray sorting is based on Costa2014, Ray Reordering Techniques for GPU Ray-Cast Ambient Occlusion. In contrast to the approach in the paper, we found ray sorting by 8bit encoded octahedral ray direction working well and no impact for sorting by ray origin depth. But surely there can be cases where the ray origin depth would make a difference, such as in cases of cluttered geometry (looking at a dense grass field from a sharp angle).

## Checkerboard/0.5 spp
The sample supports checkerboard pattern generated rays that rotates every frame. Currently, only the ray-sorted RTAO path takes advantage of this. The standard 2D DispatchRays just traces for all pixels. This is be trivial to extend.

## Approximate interreflections
AO has tendency to overdarken the GI effect because it assumes the occluders don't emit/reflect any light. An option to solve this would be to do what a GI solution does and check for amount of light reflected by occluders. This naturally would make the AO rays more expensive to trace. The sample, instead, implements and approximation to intereflections as per Ch 11.3.3 Accounting for Interreflections, Real-Time Rendering (4th edition). The approximation modulates ambient coefficient based on an albedo of the surface. Lighter surfaces will have higher ambient coefficient and darker surfaces will have lower ambient coefficient. This approximation builds on a fact that lighter surfaces reflect more light and local surfaces tend to have similar colors.

## AO Real-Time Denoiser
The sample implements a spatio-temporal denoiser based on an implementation described in Schied 2017, SpatiotemporalVariance-GuidedFiltering: Real-Time ReconstructionforPath-TracedGlobalIllumination. It uses normal, depth and variance buffers to drive an edge-stopping filtering kernel. In contrast to the paper, the sample implements a single pass 3x3 filter per frame to lower the performance overhead. The filtering quality is improved by using an adaptive kernel size based on temporally accumulated ray hit distance. Furthermore, the kernel size is varied between min kernel width and target kernel width every frame for a better spatial coverage. This allows for a fairly good blur result even though the sample uses a small 3x3 kernel, once per frame. However, much of the denoisers filtering capabilities come from temporal supersampling/accumulation which immensely increase temporal spp and decrease the noise. 

The temporal part of the denoiser suffers from two main drawbacks. 1) Using large temporal frame windows results in ghosting (invalid AO result being kept around for longer than it should). This is addressed via clamping of the cached values to a window around local values from a current frame. However, due to the nature of low spp per frame data the input to clamping is very noisy and the window is large. Therefore while the clamping technique helps, it doesn't completely get rid of ghosting, especially for regions with little AO value change from a frame to frame. There's more work and experimentation to be done to improve this. 2) Second drawback is that the cached AO values need to be discarded if the surface gets disoccluded. When disocclusion happens such pixels have very noisy AO values for a first few frames. To address that, the denoiser applies a stronger blur with weaker bilateral constraints for such pixels in three passes. Note that this filter is only applied for pixels with low temporal spp and the filter strength is decreased as number of temporal spps increase over subsequent frames. This generally works well and has an appearance of motion blur making the result visually pleasing.

## Dynamic geometry
The sample supports dynamic updates both to BLAS world transforms and geometry vertices. TLAS is rebuilt every frame picking up active BLAS instances with updated world transforms. BLASes that have had their vertex geometry changed are rebuilt.

### Dynamic Vertex Geometry
The grass patches are the only objects with dynamic vertex geometry in the scene. They are built as patches of grass blades, i.e. 100x100 grass blades, with each blade consisting of 5 triangles generated via a Compute Shader ([GenerateGrassStrawsCS.hlsl](util/GenerateGrassStrawsCS.hlsl)) based on input parameters and a wind map texture. The blade generation is based on [Emerald's engine implementation](https://github.com/lragnarsson/Emerald-Engine). The grass geometry adds high-frequency detail to the scene making it a good stress test for AO Raytracing and Denoising. Rendering of this type of geometry is very prone to aliasing. To lower the alias, the grass CS generator generates multiple grass patches, one for each LOD. Each LOD has its own vertex buffer. The LODs differ primarily in wind strength so that the patches further away move less. Each LOD is built as a separate BLAS and there can be multiple BLAS instances associated with each LOD BLAS. Every frame, the app maps a grass patch BLAS instance to a BLAS/LOD based on its distance to a camera. Therefore, a BLAS instance's LOD can change from a frame to frame.

Temporal reprojection needs to be able to find vertices of a triangle that was hit in the current frame in the previous frame. Since the LOD can change, the previous frame vertex buffers that need to be sampled when calculating motion vectors change. This scenario is handled by creating shader records for all cases of LOD transitions for all LODs. Then, on geometry/instance updates, a BLAS instance updates its InstanceContributionToHitGroupIndex to point to the corresponding shader records for that LOD and the LOD transition. 

                // Dynamic geometry with multiple LODs is handled by creating shader records
                // for all cases. Then, on geometry/instance updates, a BLAS instance updates
                // its InstanceContributionToHitGroupIndex to point to the corresponding 
                // shader records for that LOD. 
                // 
                // The LOD selection can change from a frame to frame depending on distance
                // to the camera. For simplicity, we assume the LOD difference from frame to frame 
                // is no greater than 1. This can be false if camera moves fast, but in that case 
                // temporal reprojection would fail for the most part anyway yielding diminishing returns.
                // Consistency checks will prevent blending in from false geometry.
                //
                // Given multiple LODs and LOD delta being 1 at most, we create the records as follows:
                // 2 * 3 Shader Records per LOD
                //  2 - ping-pong frame to frame
                //  3 - transition types
                //      Transition from lower LOD in previous frame
                //      Same LOD as previous frame
                //      Transition from higher LOD in previous frame

				
### Potential improvements
There are multiple opportunities to improve the denoiser further both quality and performance wise
* RTAO
  * Variable rate sampling. For example sampling could be adjusted depending on temporally accumulated spp (tspp) of a pixel. Temporal reprojection can be run before current's frame AO raytracing and thus provide the per pixel tspp information. In fact, the current denoiser already splits temporal reprojection and blending step and supports the dual stage denoising.
  * Use a better sampler:
     * Take a look at addressing sample clumping that's visible at raw 1 spp AO visualization. For example [Correlated Multi-Jittered Sampling](http://graphics.pixar.com/library/MultiJitteredSampling/paper.pdf) has become popular way to lower clumping.
     * There's a potential to detect undersampled parts of the hemisphere given the accumulated samples over time and improve coverage of the tspp. Take a look at using a progressive sampling technique that for better aggregated sample coverage over time.
  * Ray sorting is a win on Pascal with Ray Tracing time speedup being greater than the overhead of ray sorting. On Turing it breaks around even. Currently AO ray gen and ray sort are two CS passes. Combining them should lower the pre-sorting overhead.  
* Denoiser
  * Consider increasing depth bit allocation to improve depth testing when filtering and upsampling. The sample uses 16b depth and 16b encoded normals to lower bandwidth requirements. However, given the high exponent (64) used for normal testing, it might be worthwhile to trade some of the bits from normals to a depth data as the depth test is more strict. For example, the allocation could be 24b for depth and 8b for encoded normal. This would be only for denoising. 8b normal encoding would likely adversely impact AO ray sampling quality if used in AO ray generation.
  * Upsampling could be improved to find better candidates from low res inputs. Either by increasing the 2x2 sampling quad and/or improving the depth test to be more strict by testing against expected depth at the source low-res sample offset instead of the current test target depth +/- threshold.
  * Disocclusion blur. Current implementation uses a 3x3 separable filter and runs three times at varying kernel steps. It might be possible to get a better quality/perf tradeoff with larger kernel, larger steps and fewer iterations.

There are also few areas in the sample which you should consider to improve in your implementation if porting the sample code over:
* Acceleration Structure 
  * Use a separate scratch resource for each BLAS build to allow driver to overlap the builds.
  * Use compaction to lower the size of static BLAS resources (by ~55%).
