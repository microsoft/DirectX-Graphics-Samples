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

This sample demonstrates the **Batched Asynchronous Command List APIs** (a.k.a. *Async Commands*)
introduced in the DirectX 12 **Agility SDK 720 preview**.

The legacy `CopyBufferRegion`, `ClearUnorderedAccessView*`, `ResolveSubresource`, and similar
commands execute strictly in series because the old `ResourceBarrier` model has no way to express
a dependency between two operations of the same type (for example `COPY_DEST` -> `COPY_DEST`). As a
result the GPU stalls between every sequential copy or clear even when the operations touch
completely independent memory.

The async commands remove this implicit serialization contract. Independent work within a single
batched call can overlap on the GPU, and the developer opts into explicit synchronization using
[enhanced barriers](https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html) only
where a true data hazard exists.

The new commands, all on the `ID3D12CommandListAsyncCommands` interface (obtained by
`QueryInterface` from a graphics command list):

| Async command | Legacy counterpart it replaces |
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

2. **Performance comparison.** It benchmarks each async command against its legacy counterpart
   using GPU timestamp queries over a set of independent resources, reporting both GPU time and
   CPU command-recording time. **All ten** methods of `ID3D12CommandListAsyncCommands` are exercised:
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

Sample console output (running on the Agility SDK 720 preview WARP software renderer; absolute numbers
depend heavily on the driver and hardware):

```
  Buffer fill         ClearUnorderedAccessViewUint xN -> FillBuffers                  GPU 3.16x
  Buffer region copy  CopyBufferRegion xN             -> CopyBufferRegions            GPU 4.65x
  Whole-resource copy CopyResource xN                 -> CopyResources                GPU 1.00x
  Texture region copy CopyTextureRegion xN            -> CopyTextureRegions           GPU 6.20x
  MSAA resolve        ResolveSubresourceRegion xN     -> ResolveSubresourceRegionAsync GPU 5.07x
  Query resolve       ResolveQueryData xN             -> ResolveQueryDataAsync        GPU 0.99x
  Texture clear       ClearRenderTargetView xN        -> ClearTextureSubresources     GPU 3.02x
  Bound RTV clear     ClearRenderTargetView xN        -> ClearBoundRenderTargetViews  GPU 0.97x (raster-ordered)
  Bound DSV clear     ClearDepthStencilView xN        -> ClearBoundDepthStencilView   GPU 1.01x (raster-ordered)
  Tiled copy          CopyTiles xN                    -> CopyTilesAsync               GPU 3.98x
```

On this software renderer the overlap-oriented commands show large GPU wins (up to ~6x), while
`CopyResources`, `ResolveQueryData`, and the raster-ordered `ClearBound*` commands show parity — the
software renderer does not reorder those. Real hardware that implements async commands is expected to
show additional overlap on the independent work.

## Requirements

* DirectX 12 **Agility SDK 720 preview** (`Microsoft.Direct3D.D3D12` `1.720.0-preview`), restored
  automatically via NuGet.
* Async Commands is an **experimental** preview feature. The sample enables it with
  `D3D12EnableExperimentalFeatures(D3D12AsyncCommandsExperiment)`, which requires **Developer Mode**
  to be enabled on the machine.
* A driver that implements async commands. If the hardware driver does not yet support the feature,
  the sample automatically falls back to the preview **WARP** software renderer
  (`Microsoft.Direct3D.WARP` `1.65535.20-preview`), which does support it.

## Running the sample

Build `src\D3D12AsyncCommands.slnx` (x64 or ARM64) and run the console app. Pass `-warp` on the
command line to force the WARP software adapter.
