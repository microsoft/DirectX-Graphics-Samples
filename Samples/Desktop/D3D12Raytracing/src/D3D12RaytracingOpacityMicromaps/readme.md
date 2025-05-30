# D3D12 Raytracing Opacity Micromaps Sample
![D3D12 Raytracing Opacity Micromaps GUI](Screenshot.png)

This sample demonstrates how to use Opacity Micromaps in D3D12. The sample assumes familiarity with D3D12 programming and DirectX Raytracing concepts introduced in the [D3D12 Raytracing Hello World sample](../D3D12RaytracingHelloWorld/readme.md).

### Rendering

* Primary visibility is raytraced and a single shadow ray is cast per pixel.
* The model is made up of 3 Geometries: Trunk, Branches and Leaves.
    * Only the 'Leaves' geometry has OMMs applied to it, the 'Trunk' and 'Branches' are a non-OMM geometry.
* The OMM Array, BLAS and TLAS are built on Frame 0 and then only intermittently on frames where a build is required.

### Keyboard Controls

* 'O'
    * Enable/Disable the use of OMMs.
    * This is achieved by rebuilding the TLAS with `D3D12_RAYTRACING_INSTANCE_FLAG_DISABLE_OMMS` applied to the instance.
* 'Q/W'
    * Changes the maximum subdivision level between 1-12.
        * The OMM data was built 24 times offline (2-state / 4 state) x (Subdiv 1-12)
        * Changing this value will rebuild the OMM Array on the next frame, which necessitates rebuilding the BLAS and TLAS as well. An OMM linkage update could be used instead, but is not currently implemented.
* 'F'
    * Changes between 2-state and 4-state OMMs.
        * As per 'Q/W', changing this option will rebuild the OMM Array, BLAS and TLAS on the next frame.
        * The sample could have shipped only 4-state OMMs and used a `FORCE_2_STATE` flag to emulate, however we chose to ship both 2-bit and 4-bit OMMs since the latter does come at additional memory cost.
* 'R'
    * Enable/Disable the camera orbit around the model.
* 'Z/X'
    * Changes the Field of View (FOV) of the camera to allow for extreme zoom on the geometry.
* 'A'
    * Enable/Disable the use of Any-Hit Shaders (AHS).
    * This is achieved by dynamically adding/removing `D3D12_RAY_FLAG_FORCE_OPAQUE` from the primary and shadow ray flags.
        * Higher subdivision levels can achieve a highly accurate cut-out of the leaves without the need to use Any-Hit shaders at all. 
* 'H'
    * Colours magenta any pixel/primary ray that executed an Any-Hit shader at least once.
* 'B'
    * Enable/Disable the rebuilding of the OMMs, BLAS and TLAS every frame.
        * A useful option should you wish to take a PIX capture of the AS building process, otherwise it is only performed on Frame 0 or on a frame when a required option is toggled.

### Things to note

* The NVIDIA OMM SDK will automatically collapse an OMM down to the lowest subdivision required to represent the required pattern. Offering it the ability to use Subdivision Level 12 doesn't necessarily mean it will.
    * For this reason, Subdivision Levels 9-12 may look visually indistinguishable from level 8.
    * Other content may benefit from higher subdivision levels.
* OMMs have no concept of "mips". At a distance this can lead to aliasing artifacts, especially at higher subdivision levels.
* This model requires ~120 OMMs in the array - this has a cost of just ~3KB.
    * The OMM Index array is 4-bytes per triangle, a cost of around 2.5MB for the entire model. 
    * The NVIDIA OMM SDK currently emits only 4-byte indices. However, 2-byte indices would suffice for a model with less than ~65000 OMMs, saving 50% of the memory cost of the index buffer.
* OMM Linkage Updates are not demonstrated, the sample instead rebuilds the OMM/BLAS/TLAS when necessary.
* The sample adopts the Wavefront OBJ concept of separate buffers (and index buffers) for Positions, Normals and Texture Coordinates.
    * This is not considered a 'best practice' for game content, but done for convenience and simplicity.

### Known issues

* The sample uses ray/uv derivatives to calculate which mip level to sample the textures from. However, the shadow rays always sample Mip 0 which can lead to aliasing.
* The rendering of this model does not faithfully replicate the texture blending, UV scaling/rotation on the trunk from the Blender model original as that was unnecessary for the demonstration of OMMs.



## Usage
D3D12RaytracingOpacityMicromaps.exe

Additional arguments:
  * [-forceAdapter \<ID>] - create a D3D12 device on an adapter \<ID>. Defaults to adapter 0.

### Title bar
The title bar of the sample provides runtime information:
* Name of the sample
* Frames per second
* Million Primary Rays/s: a number of dispatched rays per second calculated based of FPS.
* GPU[ID]: name

### Controls
* ALT+ENTER - toggles between windowed and fullscreen modes.

## Requirements
* Windows 10 with the October 2019 update or higher.
* Consult the main [D3D12 Raytracing readme](../../readme.md) for further requirements.