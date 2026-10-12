# D3D12 Raytracing Clustered Geometry sample

This sample demonstrates **DXR2 clustered geometry** in D3D12: building Cluster
Level Acceleration Structures (CLAS) and Cluster BLAS via the new GPU-driven
indirect acceleration-structure operations API, with both the static path
(`BUILD_CLAS_FROM_TRIANGLES` + selectable `FLOAT32_3` / `COMPRESSED1` vertex
formats) and the animated path (`BUILD_CLUSTER_TEMPLATES_FROM_TRIANGLES` + per-frame
`INSTANTIATE_CLUSTER_TEMPLATES`).

## Build

Use Visual Studio 2022 with the Desktop development with C++ workload, a
Windows SDK, and the **vcpkg package manager** component
(`Microsoft.VisualStudio.Component.Vcpkg`). Alternatively, set `VCPKG_ROOT`
to your vcpkg installation. Enable Windows Developer Mode for the experimental
SM 6.10 shaders.
The sample restores these native NuGet dependencies into the repository's
`Packages` directory:

- `Microsoft.Direct3D.D3D12` **1.722.0-160058127-preview** (DXR2 preview).
- `Microsoft.Direct3D.DXC` **1.10.2605.37-preview** (SM 6.10 compiler).

DirectXTK12 (text overlay) is built from source through the repository's shared
`vcpkg.json` manifest and `vcpkg\DirectX.vcpkg.props` / `.targets` integration,
matching the other raytracing samples. MSBuild automatically installs the
manifest dependencies for `x64-windows-static-md`, using the appropriate Debug
or Release libraries. No prebuilt DirectXTK NuGet package is required.

Until the Agility package is published, add the folder containing its `.nupkg`
as a **local package source** in Visual Studio's NuGet Package Manager settings,
alongside nuget.org. Alternatively, configure it in your user-level
`NuGet.Config`. The feed location is machine-specific and is not encoded in the
project; `packages.config` and the `.vcxproj` encode only package versions.
When the public Agility version is available, update its version in those two
files and restore normally from nuget.org.

Open `D3D12RaytracingClusteredGeometry.sln` and restore/build **Debug|x64** or
**Release|x64**. From a VS 2022 Developer PowerShell:

```powershell
msbuild D3D12RaytracingClusteredGeometry.sln /restore /p:RestorePackagesConfig=true /p:Configuration=Debug /p:Platform=x64
```

NuGet copies `D3D12Core.dll` and `D3D12SDKLayers.dll` into
`bin\x64\<Configuration>\D3D12\`, and the DXC DLLs next to the EXE.
`Main.cpp` exports `D3D12_PREVIEW_SDK_VERSION` and `D3D12SDKPath` so the system
loader uses the matching preview runtime. No private D3D12 build is required.
The sample enables `D3D12RaytracingPreview` before device creation and queries
`D3D12_FEATURE_D3D12_OPTIONS_RAYTRACING2` to check clustered-geometry support.

## Run

```
D3D12RaytracingClusteredGeometry.exe [-forceAdapter <ID>] [--screenshot N path.png]
```

`--screenshot N path.png` renders `N` frames, dumps the back buffer to PNG,
and exits. Useful for unattended visual verification.

### Testing on hardware / WARP

The sample selects a hardware adapter by default; `-forceAdapter <ID>` overrides
its DXGI adapter index. Use `d3dconfig` to force software rendering:

```
d3dconfig device force-warp=true     # requires a DXR2-capable preview WARP
d3dconfig device force-warp=false    # use the system's hardware adapter
```

The Agility package does not include WARP. Software DXR2 testing requires a
compatible preview `d3d10warp.dll` next to the EXE; the OS-provided WARP may not
support clustered geometry. Ensure that the executable's directory is in
`d3dconfig`'s application paths before changing its settings, and restore
`force-warp=false` after software testing.

On a supported preview driver, the sample uses clustered geometry. If the
adapter reports no clustered-geometry support, the sample logs the unsupported
capability, visibly labels the fallback, and uses traditional BLAS instead.
Diagnostic output goes to `%TEMP%\D3D12RaytracingClusteredGeometry.log`, or the
path specified by the `SAMPLE_LOG` environment variable.

### Runtime controls

On-screen overlay (right column) lists the active value for each toggle.

| Key      | Action                                                                                  |
| -------- | --------------------------------------------------------------------------------------- |
| `[T]`    | Geometry path: **clustered** ↔ **traditional** (DXR1 BLAS). Triggers static AS rebuild. |
| `[A]`    | Cluster mode: cycle CLAS alloc strategy (`implicit-dest` → `get-sizes` → `compact`).    |
|          | Trad mode: cycle BLAS alloc strategy (`implicit-dest` ↔ `compact`).                     |
| `[V]`    | Vertex format: `FLOAT32_3` ↔ `COMPRESSED1` (shared-exponent quantized).                 |
| `[F]`    | Trad mode: animated BLAS update strategy (`rebuild` ↔ `refit`).                         |
| `[R]`    | Cluster mode: per-frame static-AS rebuild (`off` → `BLAS only` → `CLAS+BLAS`).          |
| `[B]`    | Build preference: `none` → `fast-build` → `fast-trace`.                               |
| `Space`  | Pause / resume camera motion.                                                           |
| `[M]`    | Pause / resume animation.                                                               |
| `[N]`    | Workload scaling — cycle extra clones (`0` → `100` → `1,000` → `10,000` → `0`).         |
|          | Each clone gets its OWN CLAS array + BLAS (1:1 instance:BLAS), so toggling progressively |
|          | stresses the CLAS/BLAS/template paths.  Clones spiral out from the floor in a           |
|          | sunflower pattern; distance-based LOD swaps the source mesh for a lower-tessellation    |
|          | variant as radius grows, so the back rings stay cheap.                                  |
| `,` `.`  | Bounce slider — reflection + refraction depth (held +2 apart, range 0..5).              |
| `[` `]`  | Vertex precision slider — fewer / more position bits (`FLOAT32_3` truncate-bits or      |
|          | `COMPRESSED1` bits/component). Rebuilds static CLAS and animated templates together.    |

### Headless / scripted runs

For a timed run that writes a JSON snapshot and exits:

```
D3D12RaytracingClusteredGeometry.exe --geometry-mode clusters --vertex-format compressed --bench-seconds 5 --bench-out result.json
```

Use `--exit-after-frames N` for a frame-count smoke test. The command
`--screenshot-at S path.png` jumps the camera/animation time to `S`, warms up
the swap chain, saves a PNG, and exits.

### Scheduled actions

`--at <frame>:<action>` queues an action for a zero-based rendered-frame index.
Combine multiple `--at` arguments for scripted state sweeps. Actions:

| Action               | Equivalent hotkey                          |
| -------------------- | ------------------------------------------ |
| `geom-clusters`      | `[T]` → clustered                          |
| `geom-traditional`   | `[T]` → traditional                        |
| `alloc-implicit`     | `[A]` in cluster mode → implicit-dest      |
| `alloc-getsizes`     | `[A]` in cluster mode → get-sizes          |
| `alloc-compact`      | `[A]` in cluster mode → compact            |
| `trad-implicit`      | `[A]` in trad mode → implicit-dest         |
| `trad-compact`       | `[A]` in trad mode → compact               |
| `anim-rebuild`       | `[F]` → rebuild                            |
| `anim-refit`         | `[F]` → refit                              |
| `rebuild-none`       | `[R]` → off                                |
| `rebuild-blas`       | `[R]` → BLAS only                          |
| `rebuild-clas-blas`  | `[R]` → CLAS+BLAS                          |
| `extra-none`         | `[N]` → 0                                  |
| `extra-100`          | `[N]` → 100                                |
| `extra-1k`           | `[N]` → 1,000                              |
| `extra-10k`          | `[N]` → 10,000                             |
| `flags-none`         | `[B]` → no build preference                |
| `flags-fast-build`   | `[B]` → prefer fast build                  |
| `flags-fast-trace`   | `[B]` → prefer fast trace                  |
| `log`                | Write a timing snapshot to the sample log  |
| `exit`               | Exit the sample                            |

### Window behavior

On HW adapters the window launches maximized; on WARP / Basic Render it stays
at the default 1280×720 since maximizing a CPU rasterizer to 4K would make
each frame take seconds. Headless runs (any of the above CLI flags) also stay
at 1280×720 so screenshot resolution is deterministic.

## Source organization

The sample is split across a few logical files so the bits worth copy-pasting
into your own renderer are concentrated in one place.

- **`D3D12RaytracingClusteredGeometry.cpp`** – the **main file**: CORE
  acceleration-structure management + rendering.  This is where the DXR2 work
  lives that you'd actually want to crib for your own engine:
    * `BuildClasIndirect` / `BuildClasImplicit` / `BuildClasGetSizes` /
      `BuildClasCompact` — the three CLAS alloc-mode variants the `[A]` toggle
      cycles through.
    * `BuildBlasFromClasIndirect` — pooled per-object BLAS-from-CLAS for the
      static scene.
    * `BuildAnimatedObjectSetup` / `UpdateAnimatedObjectPerFrame` — template
      build (once) + per-frame `INSTANTIATE_CLUSTER_TEMPLATES` +
      `BUILD_BLAS_FROM_CLAS` for the deforming ball.
    * `BuildAnimatedClonesSetup` — phase-2 extension that fans the per-frame
      BLAS-from-CLAS op out to N additional dest GVAs so each `[N]` anim clone
      gets its own per-frame BLAS in a shared pool.
    * `BuildTraditionalStaticAS` / `BuildAnimatedTraditionalAS` /
      `UpdateAnimatedTradPerFrame` — DXR1 baseline path the `[T]` toggle
      compares against.
    * `BuildTlasClassic` — TLAS instance assembly + per-instance material
      overrides + per-mode BLAS GVA selection.
    * `CreateRaytracingPipelineAndShaderTables` + the `CreateFill*Pipeline`
      helpers — state object + 8-record shader table + compute PSOs for the
      GPU-side arg-fill passes.
    * `OnInit` / `OnRender` / `DoRender` — the device + per-frame render loop.
- **`OverlayUI.cpp`** – on-screen stats overlay (`RenderUI`, `CreateUIFont`,
  delta-color snapshotting).  Pure presentation; safe to ignore if you're
  reading for AS knowledge.
- **`SceneSetup.cpp`** – `BuildScene` + `BuildMaterials`: this particular
  demo's collection of spheres/torus/cube/klein + their PBR-ish materials.
  Your engine has its own scene system; this file shows what it should hand
  the AS pipeline.
- **`ScenePopulation.cpp`** – `[N]` workload-scaling clone generator
  (`RegenerateWorkloadCloneInstances` + `EnsureCloneSourceLodMeshes`).
  Demo-only — engineers cribbing the AS pipeline can ignore this file
  entirely.
- **`InputHandling.cpp`** – `ParseCommandLineArgs` + `OnKeyDown` keyboard
  routing.  CLI + UI hookups, not part of the AS pipeline.

## Things to note

- The static clustered scene and animated templates use the format selected
  by `[V]` or `--vertex-format`. Animated source vertices remain `FLOAT32_3`;
  template instantiation converts them to the selected stored format. In
  **COMPRESSED1** mode, each template locks the active bits/component and a
  shared exponent sized for the full animation envelope, so `[ ]` visibly and
  consistently changes both static CLAS and template-instantiated CLAS.
- Cluster size statistics are read back via the
  `D3D12_RTAS_OPERATION_MODE_GET_SIZES` mode and via `ResultSizeArray` on
  implicit-destination builds.
- Per-frame cluster-template instantiate args are populated by the
  `FillInstantiateArgs.hlsl` compute shader, illustrating the spec's GPU-driven
  intent even though the demo's argument count is small enough that CPU upload
  would also work.
