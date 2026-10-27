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
| `CopyBufferRegions` | `CopyBufferRegion` (single batched call; independent copies may overlap) |
| `CopyTextureRegions` | `CopyTextureRegion` |
| `CopyResources` | `CopyResource` |
| `CopyTilesAsync` | `CopyTiles` |
| `ResolveSubresourceRegionAsync` | `ResolveSubresourceRegion` |
| `ResolveQueryDataAsync` | `ResolveQueryData` |
| `FillBuffers` | `ClearUnorderedAccessViewUint` / `ClearUnorderedAccessViewFloat` (no descriptors required) |
| `ClearTextureSubresources` | staged `CopyTextureRegion`, `ClearRenderTargetView`, and `ClearUnorderedAccessViewFloat` |
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
   - `ClearTextureSubresources` vs. staged `CopyTextureRegion` for a partial copy-route clear
   - Optional (`-clearalternatives`): `ClearTextureSubresources` vs. transitioned
     `ClearRenderTargetView` for a full RTV-route clear
   - Optional (`-clearalternatives`): `ClearTextureSubresources` vs. transitioned
     `ClearUnorderedAccessViewFloat` as a UAV alternative
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
  Texture clear/copy  CopyTextureRegion xN           -> ClearTextureSubresources     GPU 1.02x
  Bound RTV clear     ClearRenderTargetView xN        -> ClearBoundRenderTargetViews  GPU 0.97x (raster-ordered)
  Bound DSV clear     ClearDepthStencilView xN        -> ClearBoundDepthStencilView   GPU 1.01x (raster-ordered)
  Tiled copy          CopyTiles xN                    -> CopyTilesAsync               GPU 4.22x
```

With `-clearalternatives`, the output also includes the transitioned RTV and UAV comparisons.

On this software renderer the overlap-oriented commands show large GPU wins (up to ~5.8x), while
`CopyResources`, `ResolveQueryData`, and the raster-ordered `ClearBound*` commands show parity — the
software renderer does not reorder those. Real hardware that implements async commands is expected to
show additional overlap on the independent work.

3. **Optional end-to-end frame benchmark (`-e2e`).** In addition to the microbenchmarks above, the
   sample can measure a mixed workload where independent resource-update commands and rendering work
   are recorded together in one submission. This highlights whether async batching improves
   whole-frame behavior, not just isolated command latency.

## Requirements

* DirectX 12 **Agility SDK 620.1** (`Microsoft.Direct3D.D3D12`), restored automatically via NuGet.
* A driver that implements async commands, or `-fallback` to permit the runtime fallback on an
  adapter without native support.

On startup the sample reports which implementation it obtained:

* `NATIVE` — the driver implements async commands, so independent work can genuinely overlap.
* `FALLBACK` — the runtime emulates async commands on legacy command paths. The benchmark still
  runs, but it is measuring fallback behavior rather than native async-command overlap.

## Running the sample

Build `src\D3D12AsyncCommands.slnx` (x64 or ARM64) and run the console app.

| Option | Effect |
|---|---|
| *(none)* | Use the default adapter, falling back to WARP if it has no native support |
| `-list` | List every adapter with its vendor, device ID and driver version, then exit |
| `-adapter <index>` | Use a specific adapter from `-list` |
| `-warp` | Force the WARP software adapter |
| `-fallback` | Enable the runtime fallback on adapters without native support |
| `-e2e` | Run an additional end-to-end mixed-frame benchmark (updates + rendering together) |
| `-clearalternatives` | Also compare `ClearTextureSubresources` with RTV/UAV clear techniques |
| `-debuglayer` | Enable the D3D12 debug layer explicitly (fails fast if debug layer is unavailable) |

The sample prints adapter description, vendor and device IDs, adapter LUID, user-mode driver
version, and the implementation tier it obtained, so the output records exactly which driver
produced the numbers.

When an adapter is named explicitly with `-adapter` or `-warp`, the sample **never** silently
substitutes WARP; if that adapter does not support async commands it reports the fact and exits
non-zero. Only the default (no-argument) path falls back to WARP.

`-fallback` uses `ID3D12AsyncCommandsFallback` before device creation. It permits the runtime to
lower async commands onto legacy paths when the selected adapter has no native implementation.
Native driver support still takes precedence, so `-fallback` does not force a native-capable
adapter onto the fallback path.

The default texture-clear benchmark uses staged `CopyTextureRegion`, matching the fallback's
copy route. RTV/UAV replacement comparisons are exploratory and opt-in with `-clearalternatives`;
applications expecting fallback should retain the corresponding legacy view clear where its
performance is important.

Use `-debuglayer` when validating debug-layer cleanliness in test runs (for example on Release
binaries copied to another machine). If `-debuglayer` is specified and the debug layer cannot be
enabled, the sample exits with a clear error instead of silently continuing.
