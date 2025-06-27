# D3D12 Raytracing Basic Shader Execution Reordering Sample
![D3D12 Raytracing Simple Lighting GUI](Screenshot.png)

This sample demonstrates how to use the Shader Execution Reordering (SER) feature in a basic raytracing scene composed of multiple materials. It builds on foundational concepts introduced in the [D3D12 Raytracing Hello World sample](../D3D12RaytracingHelloWorld/readme.md).
The scene includes two geometry instances, each assigned a different materialID (0 and 1) in the shader table. These instances are layered and intertwined on top of each other in space. Instances with materialID=1 use a procedural texture to simulate a heavier shading cost. Shader execution reordering is then based on the HitObject and a sort key being materialID, derived from the shader table. This sample is built on the D3D12RaytracingSimpleLighting sample.

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
* S - toggles shader execution reordering on/off.

## Requirements
* Windows 10 with the October 2018 update or higher.
* Consult the main [D3D12 Raytracing readme](../../readme.md) for further requirements.