# Copyright, license, and source acknowledgments

## NVIDIA sample code

The NVIDIA-owned portions of this sample, including the DirectX adaptation,
are provided under the [MIT License](LICENSES/MIT.txt), consistent with
Microsoft's DirectX-Graphics-Samples repository. NVIDIA copyright notices and
original author acknowledgments are retained. Third-party portions retain the
notices identified below.

This sample is based on NVIDIA's public
[vk_partitioned_tlas](https://github.com/nvpro-samples/vk_partitioned_tlas)
Vulkan sample by [Pascal Gautron](https://github.com/pgautron) and the
nvpro-samples contributors. The reviewed reference revision is
[`07acd56c13673ae131ae26d85d3faae5ebc90e06`](https://github.com/nvpro-samples/vk_partitioned_tlas/tree/07acd56c13673ae131ae26d85d3faae5ebc90e06).
Retained adapted material includes:

- `src/scene.hpp` and scene-generation portions of
  `src/D3D12RaytracingPartitionedTLAS.cpp`: Hilbert paths, spline sampling,
  static scenery, domino orientation, and initial velocities from
  `src/partitioned_tlas_scene.cpp`.
- `shaders/physics_core.hlsli` and `shaders/animation.hlsl`: domino physics,
  PCG/WangHash helpers, animation, and partition policy from
  `shaders/animation_physics.comp.glsl`, `animation_init.comp.glsl`, and
  `animation_update_instances.comp.glsl`.
- `Motion` in `shaders/RaytracingHlslCompat.h`: the physics-state record from
  `AnimationState` in `shaders/shaderio.h`.
- `shaders/raytracing.hlsl`: rendering, material/refraction behavior, and
  compositing inputs from the original ray-generation, closest-hit, and miss
  shaders and `shaders/raytrace_common.h`.
- `shaders/compositing.hlsl`: the original compositing stages from
  `shaders/compositing.comp.glsl`.

Selected implementations also adapt NVIDIA's public
[nvpro_core2](https://github.com/nvpro-samples/nvpro_core2), reviewed at
[`85bd9e6db9abd9a57e440c9c8c7f678c7fe6301f`](https://github.com/nvpro-samples/nvpro_core2/tree/85bd9e6db9abd9a57e440c9c8c7f678c7fe6301f):

- `src/SceneMeshes.h`: cube, plane, tetrahedron, and icosahedron implementations
  from `nvutils/primitives.cpp`.
- `shaders/raytracing.hlsl`: PCG/xxHash helpers from
  `nvshaders/random.h.slang`, basis construction from
  `nvshaders/functions.h.slang`, and simple-sky evaluation/defaults from
  `nvshaders/sky_functions.h.slang` and `nvshaders/sky_io.h.slang`.

Algorithm references retained from those helpers include
[PCG](https://www.pcg-random.org/index.html),
[xxHash](https://github.com/Cyan4973/xxHash) and its
[shader reference](https://www.shadertoy.com/view/XlGcRh), and Nelson Max's
[Improved accuracy when building an orthonormal basis](https://jcgt.org/published/0006/01/02/).

## Microsoft contributions

Amar Patel (Microsoft) contributed SDK-layout and build-configuration fixes,
adapter-capability selection changes, and feature-probe improvements. Microsoft
copyright notices are included alongside NVIDIA's in the modified C++ files.

## GLM Catmull-Rom helper

`scene::CatmullRom` in `src/SceneMath.h` follows GLM's `glm/gtx/spline.inl`
implementation, replacing the original Vulkan sample's `glm::catmullRom` call.
The reviewed GLM revision is
[`6f14f4792a0cde5d0cf2c910506724d61cb95834`](https://github.com/g-truc/glm/tree/6f14f4792a0cde5d0cf2c910506724d61cb95834).
GLM offers an MIT license option; its copyright and permission notice are
reproduced in [LICENSES/GLM.txt](LICENSES/GLM.txt).

Copyright (c) 2005 - G-Truc Creation

## Font asset

`assets/SegoeUI_18.spritefont` is the same font asset supplied with Microsoft's
public DirectX ray-tracing samples, including SakuraForestSER. It is provided
under the [MIT license](assets/LICENSE.txt), Copyright (c) 2015 Microsoft.

## Build dependencies

DirectXTK12 is built from source through the repository's shared vcpkg manifest.
Agility SDK, DXC, and WinPixEventRuntime are restored from the packages listed in
`packages.config`. The DirectXTK12 MIT license is included in
[LICENSES/DirectXTK12.txt](LICENSES/DirectXTK12.txt); WinPixEventRuntime's package
contains its license and third-party notices. These notices remain applicable
to their respective dependencies.

The Windows SDK and the Agility SDK, DXC, and WinPixEventRuntime packages retain
their respective Microsoft terms. Their binaries are not included in this source
contribution.
