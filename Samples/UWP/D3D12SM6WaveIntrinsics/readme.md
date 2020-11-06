---
page_type: sample
languages:
- cpp
products:
- windows
- windows-uwp
name: Direct3D 12 shader model 6 wave intrinsics sample
urlFragment: d3d12-shader-model-6-wave-intrinsics-sample-uwp
description: This sample visualizes how wave intrinsics work.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Direct3D 12 shader model 6 wave intrinsics sample
This sample visualizes how wave intrinsics work. Wave intrinsics are a new set of intrinsics for use in HLSL shader model 6. They enable operations across lanes in the SIMD processor cores, helping the performance of certain algorithms such as culling and packing sparse data sets.

Doc and other resources:
*) Wave intrinsics Wiki: https://github.com/Microsoft/DirectXShaderCompiler/wiki/Wave-Intrinsics
*) DirectX MiniEngine: This engine takes advantage of the wave intrinsics to improve the performance of forward plus lighting. https://github.com/Microsoft/DirectX-Graphics-Samples/tree/master/MiniEngine

Requirement:
*) OS: Windows 10 Creators Update 15063+.
*) Windows SDK: Install Windows 10 SDK 15063+ to obtain DXC, the new shader compiler that supports SM6. Note that you might need to modify the paths in CompileShader_SM6.bat if you are using a SDK that is newer than 15063. For simplicity we point the dxc.exe path to 15063 SDK folder. 
*) Graphics Driver: See this page for details. https://github.com/Microsoft/DirectXShaderCompiler/wiki/Running-Shaders
