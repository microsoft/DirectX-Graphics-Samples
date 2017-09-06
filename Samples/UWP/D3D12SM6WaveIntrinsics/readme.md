# Shader Model 6 Wave Intrinsics Sample
This sample visualizes how Wave Intrinsics work. Wave intrinsics are a new set of intrinsics for use in HLSL Shader Model 6. They enable operations across lanes in the SIMD processor cores, helping the performance of certain algorithms such as culling and packing sparse data sets.

Doc and other resources:
*) Wave Intrinsics Wiki: https://github.com/Microsoft/DirectXShaderCompiler/wiki/Wave-Intrinsics
*) DirectX MiniEngine: This engine takes advantage of the wave intrinsics to improve the performance of forward plus lighting. https://github.com/Microsoft/DirectX-Graphics-Samples/tree/master/MiniEngine

Requirement:
*) OS: Windows 10 Creators Update 15063+.
*) Windows SDK: Install Windows 10 SDK 15063+ to obtain DXC, the new shader compiler that supports SM6. Note that you might need to modify the paths in CompileShader_SM6.bat if you are using a SDK that is newer than 15063. For simplicity we point the dxc.exe path to 15063 SDK folder. 
*) Graphics Driver: See this page for details. https://github.com/Microsoft/DirectXShaderCompiler/wiki/Running-Shaders
