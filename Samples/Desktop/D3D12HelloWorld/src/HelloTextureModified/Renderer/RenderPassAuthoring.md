# Render Pass Authoring Guide

This note describes how to add or modify a render pass in `D3D12HelloTexture`.
The pass graph is intentionally data-first: a `RenderPass` describes resources, bindings, output targets, GPU pipeline selection, and the operation to record.

## Mental Model

`Pipeline` and `Operation` are separate concepts.

- `Pipeline` answers: which GPU pipeline state should be bound before the pass records commands?
- `Operation` answers: which C++ recording function should emit commands for this pass?

For example, `PipelineId(Pipe::ToneMap)` selects the ToneMap PSO. `RegisterPassOperation(Op::ToneMap, &D3D12HelloTexture::ExecuteToneMapPass)` selects and registers the operation handler, which currently calls `RecordToneMapPass()`.

The execution flow is:

1. `BuildRenderPasses()` builds the ordered pass list.
2. `ValidateRenderPassGraph()` checks that the graph can resolve its pipelines, operation handlers, bindings, and constants.
3. `AnalyzeResourceLifetimes()` derives transient resource lifetime ranges from pass reads and writes.
4. `ExecutePass()` runs each pass:
   - creates transient resources whose lifetime begins at this pass,
   - transitions resources listed in `reads` and `writes`,
   - binds render targets,
   - binds descriptor tables,
   - binds the pass pipeline state,
   - binds root constants,
   - dispatches the pass operation handler,
   - releases transient resources whose lifetime ends at this pass.

## RenderPass Fields

`RenderPass` is declared in `RenderPassGraph.h`. Pass authoring should use `RenderPassBuilder` so the caller does not depend on aggregate field order. The final `RenderPass` still contains these fields internally.

```cpp
struct RenderPass
{
    const wchar_t *name;
    PipelineKey pipeline;
    ResourceUsages reads;
    ResourceUsages writes;
    std::vector<PassDescriptorBinding> descriptorBindings;
    PassRenderTargetBinding renderTargets;
    PassOperationKey operation;
    std::vector<PassConstantsBinding> constantsBindings;
};
```

### `name`

Human-readable pass name used for diagnostics and pass identity.

Use a stable wide string literal such as `L"ToneMapPass"`. This does not create any key and does not control execution.

### `pipeline`

The logical key for the PSO to bind before command recording.

Use `PipelineId(Pipe::SomePass)` when the pass draws or dispatches through a PSO. Use `{}` when the pass does not need a PSO, such as clear-only, copy-only, or ImGui-style passes that manage their own state.

Important: this field only selects a registered pipeline state. The concrete `D3D12_GRAPHICS_PIPELINE_STATE_DESC` is registered separately during asset loading. For ToneMap, that registration is:

```cpp
{Pipe::ToneMap, {{pToneMapVS, toneMapVSSize}, {pToneMapPS, toneMapPSSize}}, m_backBufferFormat}
```

Shader bytecode is represented consistently as `{data, size}` and grouped as `GraphicsPipelineShaders` in pipeline definitions and registration helpers.
Scene pipelines use their own definition structs as well, so pipeline keys, input layouts, shaders, and formats are grouped before materialization.

That definition is in `LoadAssets()` and is materialized by `RegisterFullscreenPipelines()`. It flows through `RegisterFullscreenPipeline(baseDesc, definition)`, which creates the fullscreen PSO desc, calls `PipelineId(definition.name)`, then stores the created PSO in `m_pipelineRegistry.Create(...)`.

At execution time, `BindPassPipeline()` calls `GetPipelineState(pass.pipeline)` and then `m_commandList->SetPipelineState(...)`.

Current root signature note: the command list root signature is bound in `BeginFrame()` with `m_rootSignature`. Current fullscreen and scene PSOs are compatible with that root signature. If a future pass needs a different root signature, the graph needs an explicit root-signature binding step or the pass must be made compatible with the existing root signature.

### `reads`

The resources this pass reads, with the state required before operation recording.

Use logical resource names, not `ID3D12Resource *`. Examples:

```cpp
MakeResourceUsages({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE}})
MakeGBufferReadUsages()
```

`reads` is consumed by lifetime analysis and transition planning. The real resource is resolved later by `ResolveResource(name)`.

### `writes`

The resources this pass writes, with the state required before operation recording.

Examples:

```cpp
MakeResourceUsages({{kBackBufferResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
MakeResourceUsages({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
```

`writes` also participates in lifetime analysis. If a new transient resource is introduced, add its logical name and creation/binding support to the resource registry path before the pass uses it.

### `descriptorBindings`

Root descriptor table bindings to set before the operation records commands.

Each entry maps a root parameter index to a logical descriptor key:

```cpp
{{RootParam_ToneMapSceneColor, DescriptorId(Desc::ToneMapSceneColorSrv)}}
```

At execution time, `BindPassDescriptors()` calls `ResolveDescriptor(binding.descriptor)` and then `SetGraphicsRootDescriptorTable(...)`.

When adding a new descriptor key:

1. Add a name in `PassKeyNames::Descriptor`.
2. Use it through `DescriptorId(Desc::YourDescriptor)`.
3. Add resolution logic in `ResolveDescriptor()`.

### `renderTargets`

The RTV/DSV set bound before operation recording.

The structure is:

```cpp
struct PassRenderTargetBinding
{
    std::vector<RtvKey> rtvs;
    std::optional<DsvKey> dsv;
    std::optional<std::array<float, 4>> clearColor;
};
```

Examples:

```cpp
{{RtvId(RtvName::BackBuffer)}, std::nullopt}

{{RtvId(RtvName::LightPass)},
 DsvId(DsvName::Depth),
 std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}}
```

At execution time, `BindPassRenderTargets()` resolves keys through `ResolveRtv()` and `ResolveDsv()`, then calls `OMSetRenderTargets(...)`.

`clearColor` does not clear automatically. It is data for the operation implementation. For example, `RecordClear()` requires it, and `RecordForwardPass()` uses it when present.

When adding a new target key:

1. Add a name in `PassKeyNames::Rtv` or `PassKeyNames::Dsv`.
2. Use it through `RtvId(RtvName::YourTarget)` or `DsvId(DsvName::YourDepth)`.
3. Add handle resolution in `ResolveRtv()` or `ResolveDsv()`.

### `operation`

The logical key for the C++ command-recording behavior.

Use `RegisterPassOperation(Op::SomePass, &D3D12HelloTexture::ExecuteSomePass)`. This is intentionally independent from `pipeline`; two passes can share an operation with different PSOs, and a pass can have an operation without a PSO.

During pass construction, `RegisterPassOperation(...)` maps the operation key to a member function and returns the key for the `RenderPass`:

```cpp
RegisterPassOperation(Op::ToneMap, &D3D12HelloTexture::ExecuteToneMapPass)
```

At execution time, `ExecutePassOperation()` looks up `pass.operation` in `PassOperationRegistry` and calls the handler. The handler should record only the pass-specific commands; resource transitions, descriptor binding, render target binding, pipeline binding, and constant binding should stay in the shared pass execution path.

### `constantsBindings`

Root 32-bit constant bindings to set before operation recording.

Each entry maps a root parameter index to a logical constants key:

```cpp
{{RootParam_ToneMapConstants, ConstantsId(ConstName::ToneMap)}}
```

At execution time, `BindPassConstants()` resolves the constants key and calls `SetGraphicsRoot32BitConstants(...)`.

When adding a new constants key:

1. Add a name in `PassKeyNames::Constants`.
2. Use it through `ConstantsId(ConstName::YourConstants)`.
3. Add packing and binding logic in `BindPassConstants()`.

If the pass has no constants, do not call `.Constants(...)`; the builder leaves the binding list empty.

## Recommended Fill Order

When authoring a pass, decide the data in this order, then place it into a `RenderPassBuilder` chain. The builder writes the final `RenderPass` fields in the correct internal order.

1. Decide the operation: can an existing `Op::...` record the commands, or do you need a new handler?
2. Decide outputs: which RTV/DSV and resource names are written?
3. Decide inputs: which resources are read, and which descriptors expose them to shaders?
4. Decide the pipeline: which PSO should be bound, and where is it registered?
5. Decide constants: are root constants needed, and who packs them?
6. Add the pass helper, for example `MakeToneMapPass()`.
7. Insert the helper in `BuildRenderPasses()` or a path-specific helper such as `AddDeferredSceneOutputPass()`.

## ToneMap Example

`MakeToneMapPass()` is a compact example of the current pattern:

```cpp
return RenderPassBuilder(L"ToneMapPass")
    .Pipeline(PipelineId(Pipe::ToneMap))
    .Reads({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE}})
    .Writes({{kBackBufferResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
    .Descriptor(RootParam_ToneMapSceneColor, DescriptorId(Desc::ToneMapSceneColorSrv))
    .Rtv(RtvId(RtvName::BackBuffer))
    .Operation(RegisterPassOperation(Op::ToneMap, &D3D12HelloTexture::ExecuteToneMapPass))
    .Constants(RootParam_ToneMapConstants, ConstantsId(ConstName::ToneMap))
    .Build();
```

The matching pieces are:

- Pipeline name: `PassKeyNames::Pipeline::ToneMap`, referenced as `Pipe::ToneMap`.
- Concrete PSO registration: `LoadAssets()` includes a `Pipe::ToneMap` fullscreen pipeline definition.
- PSO materialization: `RegisterFullscreenPipelines()` calls `RegisterFullscreenPipeline(baseDesc, definition)`.
- PSO creation: `RegisterFullscreenPipeline()` builds the PSO desc and calls `m_pipelineRegistry.Create(...)`.
- PSO binding: `BindPassPipeline()` calls `SetPipelineState(...)`.
- Input resource transition: `reads` transitions `LightPass.RenderTarget` to `PIXEL_SHADER_RESOURCE`.
- Output resource transition: `writes` transitions `BackBuffer` to `RENDER_TARGET`.
- Descriptor binding: `Desc::ToneMapSceneColorSrv` resolves to `m_lightPassColorSrv` in `ResolveDescriptor()`.
- Render target binding: `RtvName::BackBuffer` resolves in `ResolveRtv()`.
- Operation dispatch: `Op::ToneMap` maps to `ExecuteToneMapPass()` through `RegisterPassOperation(...)` while building the pass.
- Command recording: `ExecuteToneMapPass()` calls `RecordToneMapPass()`, which draws a fullscreen triangle.
- Constants binding: `ConstName::ToneMap` is handled by `BindPassConstants()`.

## Checklist For A New Draw Pass

1. Add pass key names in `PassKeyNames` only for the categories you need.
2. Add shader loading and PSO registration in `LoadAssets()`.
3. Add or reuse descriptor/RTV/DSV/resource resolution.
4. Add `MakeYourPass()` returning a complete `RenderPass` from `RenderPassBuilder`.
5. Add a pass operation handler if existing operation behavior is not enough, and register it from the pass builder chain.
6. Insert the pass in the graph construction path.
7. Build and verify that `git diff --check HEAD` stays clean.
