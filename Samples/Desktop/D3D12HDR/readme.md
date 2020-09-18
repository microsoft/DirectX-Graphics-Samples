---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Direct3D 12 HDR sample
urlFragment: d3d12-hdr-sample-win32
description: Illustrates how to render HDR content, and detect whether the current display supports it.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Direct3D 12 HDR sample
![HDR GUI](src/D3D12HDR.png)

This sample illustrates how to render HDR content, and detect whether the current display supports it.

### Controls
SPACE bar/ALT+ENTER - toggles between windowed and fullscreen modes.
UP/DOWN arrow keys - changes the format of the swap chain. 8-bit, 10-bit, and 16-bit RGBA formats are supported.
ENTER - Reset brightness to paper white and animate the peak brightness.
H - Toggle between sRGB and HDR10 output when the 10-bit swap chain is used.
U - Show/hide the UI.
