# Render Pass Authoring Guide 日本語版

このメモは、`D3D12HelloTexture` に render pass を追加、または変更するときの作業手順を説明します。
現在の pass graph は data-first の設計です。`RenderPass` は、リソース、バインディング、出力先、GPU pipeline の選択、そして実際に記録する operation をまとめて記述します。

## 基本モデル

`Pipeline` と `Operation` は別の概念です。

- `Pipeline`: pass の command を記録する前に、どの GPU pipeline state を bind するか。
- `Operation`: この pass で、どの C++ の記録関数が command を emit するか。

たとえば、`PipelineId(Pipe::ToneMap)` は ToneMap 用 PSO を選択します。`RegisterPassOperation(Op::ToneMap, &D3D12HelloTexture::ExecuteToneMapPass)` は operation handler を選択、登録し、現在はそこから `RecordToneMapPass()` が呼ばれます。

実行の流れは以下です。

1. `BuildRenderPasses()` が順序付きの pass list を作ります。
2. `AnalyzeResourceLifetimes()` が pass の `reads` / `writes` から transient resource の lifetime 範囲を計算します。
3. `ExecutePass()` が各 pass を実行します。
   - この pass で lifetime が始まる transient resource を作成する。
   - `reads` / `writes` に書かれた resource を必要な state に transition する。
   - render target を bind する。
   - descriptor table を bind する。
   - pass pipeline state を bind する。
   - root constants を bind する。
   - pass operation handler を dispatch する。
   - この pass で lifetime が終わる transient resource を release する。

## RenderPass の要素

`RenderPass` は `RenderPassGraph.h` にあります。pass を書く側では `RenderPassBuilder` を使い、呼び出し側が aggregate のフィールド順に依存しないようにします。最終的な `RenderPass` の内部には、以下のフィールドが入ります。

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

診断や pass 識別用の、人間が読む名前です。

`L"ToneMapPass"` のような安定した wide string literal を使います。これは key を作らず、実行内容も制御しません。

### `pipeline`

command 記録前に bind する PSO の logical key です。

pass が PSO を使って draw / dispatch する場合は `PipelineId(Pipe::SomePass)` を使います。clear-only、copy-only、または ImGui のように自分で state を管理する pass では `{}` を使います。

重要: このフィールドは、登録済みの pipeline state を選択するだけです。具体的な `D3D12_GRAPHICS_PIPELINE_STATE_DESC` は asset loading 時に別途登録します。ToneMap の場合は以下です。

```cpp
RegisterFullscreenPipeline(Pipe::ToneMap, psoDesc, pToneMapVS, toneMapVSSize, pToneMapPS, toneMapPSSize,
                           m_backBufferFormat);
```

この呼び出しは `LoadAssets()` にあります。そこから `RegisterFullscreenPipeline()` に入り、fullscreen 用 PSO desc を作り、`PipelineId(name)` で key を得て、最後に `m_pipelineRegistry.Create(...)` へ保存します。

実行時には、`BindPassPipeline()` が `GetPipelineState(pass.pipeline)` を呼び、その結果を `m_commandList->SetPipelineState(...)` に渡します。

現在の root signature について: command list の root signature は `BeginFrame()` で `m_rootSignature` が bind されます。現在の fullscreen / scene PSO はこの root signature と互換です。将来、別の root signature が必要な pass を追加する場合は、graph に明示的な root-signature bind step を追加するか、既存 root signature と互換にする必要があります。

### `reads`

この pass が読む resource と、operation 記録前に必要な resource state です。

`ID3D12Resource *` ではなく、logical resource name を使います。例:

```cpp
MakeResourceUsages({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE}})
MakeGBufferReadUsages()
```

`reads` は lifetime analysis と transition planning に使われます。実際の resource は後で `ResolveResource(name)` によって解決されます。

### `writes`

この pass が書き込む resource と、operation 記録前に必要な resource state です。

例:

```cpp
MakeResourceUsages({{kBackBufferResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
MakeResourceUsages({{kLightPassRenderTargetResourceName, D3D12_RESOURCE_STATE_RENDER_TARGET}})
```

`writes` も lifetime analysis に使われます。新しい transient resource を導入する場合は、その logical name と、作成/バインドの処理を resource registry 側に追加してから pass で使います。

### `descriptorBindings`

operation が command を記録する前に設定する root descriptor table binding です。

各要素は、root parameter index と logical descriptor key の対応です。

```cpp
{{RootParam_ToneMapSceneColor, DescriptorId(Desc::ToneMapSceneColorSrv)}}
```

実行時には、`BindPassDescriptors()` が `ResolveDescriptor(binding.descriptor)` を呼び、その結果を `SetGraphicsRootDescriptorTable(...)` に渡します。

新しい descriptor key を追加する場合:

1. `PassKeyNames::Descriptor` に名前を追加する。
2. `DescriptorId(Desc::YourDescriptor)` として使う。
3. `ResolveDescriptor()` に解決処理を追加する。

### `renderTargets`

operation 記録前に bind する RTV / DSV のセットです。

構造体は以下です。

```cpp
struct PassRenderTargetBinding
{
    std::vector<RtvKey> rtvs;
    std::optional<DsvKey> dsv;
    std::optional<std::array<float, 4>> clearColor;
};
```

例:

```cpp
{{RtvId(RtvName::BackBuffer)}, std::nullopt}

{{RtvId(RtvName::LightPass)},
 DsvId(DsvName::Depth),
 std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}}
```

実行時には、`BindPassRenderTargets()` が `ResolveRtv()` / `ResolveDsv()` で handle に解決し、`OMSetRenderTargets(...)` を呼びます。

`clearColor` は自動で clear されません。operation implementation に渡すデータです。たとえば `RecordClear()` はこれを必須とし、`RecordMainPass()` は存在する場合に使います。

新しい target key を追加する場合:

1. `PassKeyNames::Rtv` または `PassKeyNames::Dsv` に名前を追加する。
2. `RtvId(RtvName::YourTarget)` または `DsvId(DsvName::YourDepth)` として使う。
3. `ResolveRtv()` または `ResolveDsv()` に handle 解決処理を追加する。

### `operation`

C++ 側の command-recording behavior を表す logical key です。

`RegisterPassOperation(Op::SomePass, &D3D12HelloTexture::ExecuteSomePass)` を使います。これは `pipeline` とは意図的に分離されています。2つの pass が異なる PSO を使いながら同じ operation を共有することもできますし、PSO を持たない pass が operation だけを持つこともできます。

pass 構築時に、`RegisterPassOperation(...)` が operation key と member function を対応づけ、`RenderPass` に入れる key を返します。

```cpp
RegisterPassOperation(Op::ToneMap, &D3D12HelloTexture::ExecuteToneMapPass)
```

実行時には、`ExecutePassOperation()` が `pass.operation` を `m_passOperationHandlers` から探し、対応する handler を呼びます。handler は pass 固有の command だけを記録するのが基本です。resource transition、descriptor binding、render target binding、pipeline binding、constant binding は shared pass execution path に残します。

### `constantsBindings`

operation 記録前に設定する root 32-bit constants binding です。

各要素は、root parameter index と logical constants key の対応です。

```cpp
{{RootParam_ToneMapConstants, ConstantsId(ConstName::ToneMap)}}
```

実行時には、`BindPassConstants()` が constants key を解決し、`SetGraphicsRoot32BitConstants(...)` を呼びます。

新しい constants key を追加する場合:

1. `PassKeyNames::Constants` に名前を追加する。
2. `ConstantsId(ConstName::YourConstants)` として使う。
3. `BindPassConstants()` に packing と binding の処理を追加する。

constants が不要な pass では、`.Constants(...)` を呼びません。builder が binding list を空のままにします。

## 値を決めるおすすめ順序

pass を作るときは、以下の順でデータを決めてから、`RenderPassBuilder` の chain に入れます。builder が最終的な `RenderPass` のフィールドへ正しい内部順序で詰めます。

1. operation を決める。既存の `Op::...` で command を記録できるか、新しい handler が必要か。
2. 出力先を決める。どの RTV / DSV と resource name に書き込むか。
3. 入力を決める。どの resource を読み、shader にはどの descriptor で見せるか。
4. pipeline を決める。どの PSO を bind するか、その PSO はどこで登録されるか。
5. constants を決める。root constants が必要か、誰が pack するか。
6. `MakeToneMapPass()` のような pass helper を追加する。
7. `BuildRenderPasses()` または `AddDeferredSceneOutputPass()` のような path-specific helper に pass helper を挿入する。

## ToneMap の例

`MakeToneMapPass()` は、現在の pattern を見るための小さな例です。

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

対応する実装箇所は以下です。

- Pipeline name: `PassKeyNames::Pipeline::ToneMap`。コード上では `Pipe::ToneMap` として参照する。
- 具体的な PSO 登録: `LoadAssets()` が `RegisterFullscreenPipeline(Pipe::ToneMap, ...)` を呼ぶ。
- PSO 作成: `RegisterFullscreenPipeline()` が PSO desc を作り、`m_pipelineRegistry.Create(...)` を呼ぶ。
- PSO bind: `BindPassPipeline()` が `SetPipelineState(...)` を呼ぶ。
- 入力 resource transition: `reads` が `LightPass.RenderTarget` を `PIXEL_SHADER_RESOURCE` に transition する。
- 出力 resource transition: `writes` が `BackBuffer` を `RENDER_TARGET` に transition する。
- Descriptor binding: `Desc::ToneMapSceneColorSrv` は `ResolveDescriptor()` で `m_lightPassColorSrv` に解決される。
- Render target binding: `RtvName::BackBuffer` は `ResolveRtv()` で解決される。
- Operation dispatch: `Op::ToneMap` は pass 構築時の `RegisterPassOperation(...)` で `ExecuteToneMapPass()` に対応づけられる。
- Command recording: `ExecuteToneMapPass()` が `RecordToneMapPass()` を呼び、fullscreen triangle を描画する。
- Constants binding: `ConstName::ToneMap` は `BindPassConstants()` で処理される。

## 新しい draw pass を追加するチェックリスト

1. 必要な category だけ `PassKeyNames` に key name を追加する。
2. `LoadAssets()` に shader loading と PSO registration を追加する。
3. descriptor / RTV / DSV / resource の resolution を追加、または既存のものを再利用する。
4. `RenderPassBuilder` から完成した `RenderPass` を返す `MakeYourPass()` を追加する。
5. 既存 operation behavior で足りなければ、pass operation handler を追加し、pass builder chain から登録する。
6. graph construction path に pass を挿入する。
7. build し、`git diff --check HEAD` が clean なことを確認する。
