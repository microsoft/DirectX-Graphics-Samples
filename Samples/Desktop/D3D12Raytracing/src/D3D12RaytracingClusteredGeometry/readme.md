# D3D12 Raytracing Clustered Geometry sample

This sample demonstrates **DXR2 clustered geometry** in D3D12: building Cluster
Level Acceleration Structures (CLAS) and Cluster BLAS via the new GPU-driven
indirect acceleration-structure operations API, with both the static path
(`BUILD_CLAS_FROM_TRIANGLES` + selectable `FLOAT32_3` / `COMPRESSED1` vertex
formats) and the animated path (`BUILD_CLUSTER_TEMPLATES_FROM_TRIANGLES` + per-frame
`INSTANTIATE_CLUSTER_TEMPLATES`).

## Build

The sample requires an **experimental D3D12 build** (no public Agility SDK
NuGet supports DXR2 yet). The expected layout is:

```
<root>\install\windows\x64\debug\
    d3d12.lib, D3D12Core.dll, D3D12SDKLayers.dll
    include\d3d12.h, d3dx12.h, ...
<root>\packages\OSS\dxcbin_preview\bin\sdk\amd64\
    dxc.exe, dxcompiler.dll, dxil.dll      (must support lib_6_10)
```

Defaults assume `<root>=d:\experimental`. Override:

```
set DxrExperimentalRoot=d:\my\d3d12
set DxrExperimentalDxcRoot=d:\my\dxc
msbuild D3D12RaytracingClusteredGeometry.sln /p:Configuration=Debug /p:Platform=x64
```

The build copies `D3D12Core.dll` + `D3D12SDKLayers.dll` into
`bin\<cfg>\D3D12\` and `dxcompiler.dll` + `dxil.dll` next to the EXE. The
system `d3d12.dll` loads the experimental `D3D12Core.dll` via the standard
Agility SDK loader (`D3D12SDKVersion` + `D3D12SDKPath` exports in `Main.cpp`).

When a public Agility SDK NuGet shipping DXR2 is available, replace
`ExperimentalD3D12.props` with the standard NuGet-based pattern (see e.g.
`D3D12RaytracingOpacityMicromaps.vcxproj`).

## Run

```
D3D12RaytracingClusteredGeometry.exe [-forceAdapter <ID>] [--screenshot N path.png]
```

`--screenshot N path.png` renders `N` frames, dumps the back buffer to PNG,
and exits. Useful for unattended visual verification.

### Testing on WARP / NVIDIA / etc.

This sample doesn't pick the adapter. Use `d3dconfig` to switch:

```
d3dconfig device force-warp=true     # use WARP (CPU rasteriser, supports DXR2)
d3dconfig device force-warp=false    # use the system's hardware adapter
```

Currently exercised against:
- **Experimental WARP**: FLOAT32_3 and COMPRESSED1 clustered paths.

On the current NVIDIA RTX 4090 setup, the runtime reports clustered geometry as
unsupported and the sample falls back to the traditional BLAS path.

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

### Historical stress-test sweep (RTX 4090, N=10K, ~10M tris)

These measurements came from an earlier experimental runtime/driver combination
that exposed clustered geometry on the RTX 4090. The current setup described
above reports clustered geometry unsupported and therefore cannot reproduce the
cluster row on hardware.

| Mode    | BLAS time / frame | FPS  | What it shows                                                  |
| ------- | ----------------- | ---- | -------------------------------------------------------------- |
| cluster | 0.78 ms (2001 builds) | 43 | DXR2 batched `ExecuteIndirectRTASOperations` — one driver call covering 2K BLAS builds. |
| trad    | 640 ms (2001 builds)  | 1.5 | DXR1 baseline — 2K separate `BuildRaytracingAccelerationStructure` calls. |

→ ~800× faster per-frame BLAS work in the DXR2 path on the same scene
with the same per-clone animated geometry.  The cluster-mode N=10K
result spends most of its frame time on ray traversal, not on AS work
anymore.

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
