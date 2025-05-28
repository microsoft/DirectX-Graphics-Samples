# D3D12 Raytracing Opacity Micromaps Sample
![D3D12 Raytracing Opacity Micromaps GUI](Screenshot.png)

This sample demonstrates how to do use Opacity Micromaps in D3D12. The sample assumes familiarity with D3D12 programming and DirectX Raytracing concepts introduced in the [D3D12 Raytracing Hello World sample](../D3D12RaytracingHelloWorld/readme.md).

##### Rendering
TODO

## Usage
D3D12RaytracingOpacityMicromaps.exe

Additional arguments:
  * [-forceAdapter \<ID>] - create a D3D12 device on an adapter \<ID>. Defaults to adapter 0.

### UI
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