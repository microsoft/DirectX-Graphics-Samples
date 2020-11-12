---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Direct3D 12 mesh shader samples
urlFragment: d3d12-mesh-shader-samples-win32
description: This collection of projects act as an introduction to meshlets, and rendering with DirectX mesh shaders.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Direct3D 12 mesh shader samples
This collection of projects act as an introduction to meshlets, and rendering with DirectX mesh shaders.

### Requirements
* GPU and driver with support for [DirectX 12 Ultimate](http://aka.ms/DirectX12UltimateDev)

  <img src="../../../Assets/DirectX12Ultimate.png" alt="DirectX 12 Ultimate" height="100">

### Getting Started
* DirectX Mesh Shader spec/documentation is available at [DirectX Mesh Shader Specs](https://microsoft.github.io/DirectX-Specs/d3d/MeshShader.html) site.

# Projects
## 1. Wavefront Converter Command Line Tool
This project acts as an example of how to incorporate [DirectXMesh](https://github.com/microsoft/DirectXMesh) meshlet generation functionality into a complete mesh conversion application. The application is structured as a basic command line tool for loading and processing obj files. The processed mesh is exported using a simple binary runtime file format targeted at fast loading and rendering with DirectX apps.

## 2. Meshlet Viewer
This project demonstrates the basics of how to render a meshletized model using DirectX 12. This application loads the binary model files exported by the Wavefront Converter command line tool.

![D3D12 Meshlet Render Preview](src/MeshletRender/D3D12MeshletRender.png)

## 3. [Meshlet Instancing](src/MeshletInstancing/readme.md)
In the Mesh Shader Pipeline API there's no concept of instancing such as in the legacy pipeline. This leaves the logic of instancing meshes entirely up to application code. An inefficient implementation can waste precious threads within threadgroups. This sample demonstrates an implementation which aims to optimize instancing of meshletized meshes by packing the final, unfilled meshlets of multiple instances into a single threadgroup.

![D3D12 Meshlet Instancing Preview](src/MeshletInstancing/D3D12MeshletInstancing.png)

## 4. [Meshlet Culling](src/MeshletCull/readme.md)
The generic functionality of amplification shaders make them a useful tool for an innumerable number of tasks. This sample demonstrates the basics of amplification shaders by showcasing how to cull meshlests before ever dispatching a mesh shader threadgroup into the pipeline.

![D3D12 Meshlet Culling Preview](src/MeshletCull/D3D12MeshletCull.png)

## 5. [Instancing Culling & Dynamic LOD Selection](src/DynamicLOD/readme.md)
This sample presents an advanced shader technique using amplification shaders to do per-instance frustum culling and level-of-detail (LOD) selection entirely on the GPU for an arbitrary number of mesh instances.

![D3D12 Dynamic LOD Preview](src/DynamicLOD/D3D12DynamicLOD.png)

## Further resources
* [DirectX Mesh Shader Spec](https://microsoft.github.io/DirectX-Specs/d3d/MeshShader.html)
* [DirectXMesh Repository](https://github.com/microsoft/DirectXMesh)
* [NVIDIA Mesh Shader Blog](https://devblogs.nvidia.com/introduction-turing-mesh-shaders/)

## Feedback and Questions
We welcome all feedback, questions and discussions about the mesh shader pipeline on our [discord server](http://discord.gg/directx).
