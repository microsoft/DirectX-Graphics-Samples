---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Direct3D 12 depth bounds test sample
urlFragment: d3d12-depth-bounds-test-sample-win32
description: Demonstrates the depth bound test in Direct3D 12.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Direct3D 12 depth bounds test sample
![DepthBounds GUI](src/D3D12DepthBoundsTest.png)

This sample demonstrates the depth bound test in Direct3D 12.
Depth-bounds testing allows pixels to be discarded if the currently-stored depth value is outside the range specified by Min and Max, inclusive.

For more detail about depth bounds test, see [**ID3D12GraphicsCommandList1::OMSetDepthBounds**](https://docs.microsoft.com/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist1-omsetdepthbounds).
