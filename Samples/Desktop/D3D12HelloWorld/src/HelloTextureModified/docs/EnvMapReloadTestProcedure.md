# Environment Map Reload - Manual Test Procedure

## Setup
- Build Debug x64
- Run with DebugView (or VS Output window) to see `[Timer]` lines
- Ensure `kUseGpuProceduralEnvMap = true` in `Renderer/EnvironmentMap.h`

## Test Sequence

Execute in order, one action per bullet, observe each step.

### 1. Initial Load
- Launch the app
- **Check:** Skybox is visible (not black)
- **Check:** `[Timer] ReloadEnvironmentResources total` appears in OutputDebugString
- **Check:** `WaitForGpu (before reload)` is present (first load after startup)
- **Check:** No assert failures

### 2. AssetHdr -> ProceduralSun
- Source combo -> `Procedural Sun`
- **Check:** Skybox switches from HDR to procedural sun look
- **Check:** `[Timer]` lines show: CreateEnvironmentMapResourcesGpu -> subsections -> WaitForGpu (GPU path)
- **Check:** `WaitForGpu (before reload)` is **absent** (no prior GPU work to drain)
- **Check:** Descriptor indices in OutputDebugString show env=N, diffuse=N+1, specular=N+2, brdf=N+3 contiguous

### 3. ProceduralSun -> ProceduralStudio
- Source combo -> `Procedural Studio`
- **Check:** Skybox changes to studio look
- **Check:** No assert
- **Check:** Descriptors remain contiguous (indices may shift if prior ones were freed)

### 4. ProceduralStudio -> AssetHdr
- Source combo -> `Asset HDR`
- **Check:** Falls back to CPU code path (DDS/HDR file)
- **Check:** `[Timer]` shows CPU path section, not CreateEnvironmentMapResourcesGpu
- **Check:** No assert

### 5. AssetHdr -> ProceduralColorPanels
- Source combo -> `Procedural Color Panels`
- **Check:** GPU path used again
- **Check:** Skybox switches to color panels
- **Check:** No `WaitForGpu (before reload)`

### 6. Tweak parameters with Update ON
- Move Sky Color, Ground Color, Light Intensity sliders
- **Check:** Skybox updates in real time (reload triggered on slider release)
- **Check:** No assert

### 7. Multiple rapid switches
- Rapidly click through Source combo: Sun -> Studio -> Horizon -> Color Panels -> Sun -> ...
- **Check:** No crash or assert
- **Check:** Skybox eventually matches the last selection
- **Check:** `[Timer]` times are stable (no growing descriptor count or leaks)

### 8. Close & reopen scene (if implemented)
- Click `Close Scene`, reload a scene
- **Check:** Env map reloads cleanly on new scene
- **Check:** No stale descriptor errors

## Pass Criteria

| Item | Expected |
|------|----------|
| Assert | None during entire sequence |
| `WaitForGpu (before reload)` | Absent on procedural->procedural switch |
| Descriptor depletion | `FreeIndices` never empty before Alloc |
| Visual switching | Each source produces distinct skybox |
| GPU path | `CreateEnvironmentMapResourcesGpu` timer present for procedural sources |
| CPU path | `CreateEnvironmentMapResources` timer present for AssetHdr |
