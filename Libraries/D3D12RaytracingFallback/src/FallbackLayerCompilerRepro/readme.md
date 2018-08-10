# D3D12 Raytracing Fallback Compiler Repro Tool
This is a tool that can be used to reproduce calls to `DxilShaderPatcher::LinkCollection` in [UberShaderRayTracingProgram.cpp](https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/Libraries/D3D12RaytracingFallback/src/UberShaderRayTracingProgram.cpp).

## How to use
1. Build the FallbackLayerCompilerRepro project. The .exe file will be generated in a folder called 'Build' in the project directory.
2. #define DUMP_UBERSHADER, either by putting the definition in UberShaderRayTracingProgram.h, or right clicking the FallbackLayer project, Properties > C/C++ > Command Line, and in 'Additional Options,' write `\D"DUMP_UBERSHADER"`
3. Run your sample on top of the FallbackLayer as you would normally do.
4. Find the generated files in the sample's project directory. For example, the D3D12RaytracingMiniEngineSample will produce its files in DirectX-Graphics-Samples\Samples\Desktop\D3D12Raytracing\src\D3D12RaytracingMiniEngineSample.
5. Move the produced .cmd file and corresponding lib@[pointer].bin files to the Build folder at DirectX-Graphics-Samples\Libraries\D3D12RaytracingFallback\src\FallbackLayerCompilerRepro\Build, and run the .cmd file.
* Note: If you would like to run the repro tool in debug mode (to set breakpoints, etc), move the .bin files to the source folder (Libraries\D3D12RaytracingFallback\src\FallbackLayerCompilerRepro), and copy the command line arguments into Properties > Debugging, next to "Command Arguments".