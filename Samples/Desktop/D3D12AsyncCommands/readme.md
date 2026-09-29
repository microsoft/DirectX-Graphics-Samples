---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Direct3D 12 Async Commands sample
urlFragment: d3d12-async-commands-sample-win32
description: Demonstrates the batched asynchronous Copy, Clear, and Fill command list APIs and measures their performance versus the legacy serialized commands.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Direct3D 12 Async Commands sample

This sample demonstrates the **Batched Asynchronous Command List APIs** (a.k.a. *Async Commands*),
part of the retail DirectX 12 **Agility SDK 620** surface on `ID3D12GraphicsCommandList12`.

The legacy `CopyBufferRegion`, `ClearUnorderedAccessView*`, `ResolveSubresource`, and similar
commands execute strictly in series because the old `ResourceBarrier` model has no way to express
a dependency between two operations of the same type (for example `COPY_DEST` -> `COPY_DEST`). As a
result the GPU stalls between every sequential copy or clear even when the operations touch
completely independent memory.

The async commands remove this implicit serialization contract. Independent work within a single
batched call can overlap on the GPU, and the developer opts into explicit synchronization using
[enhanced barriers](https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html) only
where a true data hazard exists.

The new commands, all on the `ID3D12GraphicsCommandList12` interface (obtained by
`QueryInterface` from a graphics command list):

| Async command | Legacy baseline used for comparison in this sample |
|---|---|
| `CopyBufferRegions` | `CopyBufferRegion` (batched, independent copies overlap) |
| `CopyTextureRegions` | `CopyTextureRegion` |
| `CopyResources` | `CopyResource` |
| `CopyTilesAsync` | `CopyTiles` |
| `ResolveSubresourceRegionAsync` | `ResolveSubresourceRegion` |
| `ResolveQueryDataAsync` | `ResolveQueryData` |
| `FillBuffers` | `ClearUnorderedAccessViewUint` / `ClearUnorderedAccessViewFloat` (no descriptors required) |
| `ClearTextureSubresources` | UAV clears / `ClearRenderTargetView` (clears by resource pointer + format; no RTV/UAV; works on block-compressed formats) |
| `ClearBoundRenderTargetViews` | `ClearRenderTargetView` (clears the currently bound RTVs as an in-render-pass raster op) |
| `ClearBoundDepthStencilView` | `ClearDepthStencilView` (clears the currently bound DSV in-render-pass) |

## What the sample does

1. **Real render.** It draws a colored triangle into an offscreen render target and clears the
   render target with the async `ClearBoundRenderTargetViews` — an in-render-pass raster clear — in
   place of the legacy `ClearRenderTargetView`. The result is read back and verified.

2. **Performance comparison.** It benchmarks each async command against a legacy baseline
   using GPU timestamp queries over a set of independent resources, reporting both GPU time and
   CPU command-recording time. **All ten** async methods of `ID3D12GraphicsCommandList12` are exercised:
   - `FillBuffers` vs. N x `ClearUnorderedAccessViewUint`
   - `CopyBufferRegions` vs. N x `CopyBufferRegion`
   - `CopyResources` vs. N x `CopyResource`
   - `CopyTextureRegions` vs. N x `CopyTextureRegion`
   - `CopyTilesAsync` vs. N x `CopyTiles`
   - `ResolveSubresourceRegionAsync` vs. N x `ResolveSubresourceRegion`
   - `ResolveQueryDataAsync` vs. N x `ResolveQueryData`
   - `ClearTextureSubresources` vs. N x `ClearRenderTargetView`
   - `ClearBoundRenderTargetViews` vs. N x `ClearRenderTargetView`
   - `ClearBoundDepthStencilView` vs. N x `ClearDepthStencilView`

   The `Copy*`, `Resolve*Async`, `FillBuffers`, and `ClearTextureSubresources` commands drop the
   implicit serialization contract, so independent work overlaps on the GPU. `ClearBoundRenderTargetViews`
   and `ClearBoundDepthStencilView` are intentionally **raster-ordered** (they serialize with `Draw*`
   like the output merger), so their benefit is ergonomics — in/mid-render-pass clears, batching all
   bound render targets in a single call, and lower CPU-record cost — rather than GPU overlap.

Sample console output (running on the preview WARP software renderer; absolute numbers
depend heavily on the driver and hardware):

```
  Buffer fill         ClearUnorderedAccessViewUint xN -> FillBuffers                  GPU 3.01x
  Buffer region copy  CopyBufferRegion xN             -> CopyBufferRegions            GPU 5.03x
  Whole-resource copy CopyResource xN                 -> CopyResources                GPU 0.99x
  Texture region copy CopyTextureRegion xN            -> CopyTextureRegions           GPU 5.79x
  MSAA resolve        ResolveSubresourceRegion xN     -> ResolveSubresourceRegionAsync GPU 4.70x
  Query resolve       ResolveQueryData xN             -> ResolveQueryDataAsync        GPU 0.88x
  Texture clear       ClearRenderTargetView xN        -> ClearTextureSubresources     GPU 3.11x
  Bound RTV clear     ClearRenderTargetView xN        -> ClearBoundRenderTargetViews  GPU 0.97x (raster-ordered)
  Bound DSV clear     ClearDepthStencilView xN        -> ClearBoundDepthStencilView   GPU 1.01x (raster-ordered)
  Tiled copy          CopyTiles xN                    -> CopyTilesAsync               GPU 4.22x
```

On this software renderer the overlap-oriented commands show large GPU wins (up to ~5.8x), while
`CopyResources`, `ResolveQueryData`, and the raster-ordered `ClearBound*` commands show parity — the
software renderer does not reorder those. Real hardware that implements async commands is expected to
show additional overlap on the independent work.

3. **Optional end-to-end frame benchmark (`-e2e`).** In addition to the microbenchmarks above, the
   sample can measure a mixed workload where independent resource-update commands and rendering work
   are recorded together in one submission. This highlights whether async batching improves
   whole-frame behavior, not just isolated command latency.

## Requirements

* DirectX 12 **Agility SDK 620** (`Microsoft.Direct3D.D3D12`), restored automatically via NuGet.
  Async Commands is part of the retail surface from this version on — no experimental opt-in and no
  Developer Mode requirement.
* A driver that implements async commands. If the hardware driver does not yet support the feature,
  the sample automatically falls back to the preview **WARP** software renderer
  (`Microsoft.Direct3D.WARP` `1.65535.20-preview`), which does support it.

On startup the sample reports which implementation it obtained:

* `NATIVE` — the driver implements async commands, so independent work can genuinely overlap.
* `FALLBACK` — the runtime lowers the async commands onto the legacy serialized path. The benchmark
  still runs, but it is measuring the same work twice, so the comparisons are not meaningful.

## Running the sample

Build `src\D3D12AsyncCommands.slnx` (x64 or ARM64) and run the console app.

| Option | Effect |
|---|---|
| *(none)* | Use the default adapter, falling back to WARP if it has no native support |
| `-list` | List every adapter with its vendor, device ID and driver version, then exit |
| `-adapter <index>` | Use a specific adapter from `-list` |
| `-warp` | Force the WARP software adapter |
| `-fallback` | Force the runtime async-commands fallback (requires **Developer Mode**) |
| `-e2e` | Run an additional end-to-end mixed-frame benchmark (updates + rendering together) |

The sample prints adapter description, vendor and device IDs, adapter LUID, user-mode driver
version, and the implementation tier it obtained, so the output records exactly which driver
produced the numbers.

When an adapter is named explicitly with `-adapter` or `-warp`, the sample **never** silently
substitutes WARP; if that adapter does not support async commands it reports the fact and exits
non-zero. Only the default (no-argument) path falls back to WARP.

`-fallback` opts into `D3D12ExperimentalForceAsyncCommandsFallback`, which makes the runtime lower
the async commands onto their legacy counterparts. This is useful for seeing the behavior a driver
without native support would give — expect roughly 1.0x everywhere, since both sides then perform
the same work.

In fallback mode, the sample attempts each benchmark and reports any unsupported async commands as
skipped for the current adapter/runtime combination.

For `-e2e` in forced fallback mode, the sample runs the fallback-compatible subset
(copy/resolve/render) if `FillBuffers` fallback support is unavailable on that system.
