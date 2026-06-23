# Review: feature/growable-descriptor-heap @ bbaa6c1b2458

対象コミット:

- `bbaa6c1b2458`
- `Update doc: add note about ImGui button for allocator tests`

## 結論

小さい実用 trial に進んでよい状態です。

ただし、最初の適用先は main descriptor heap 全体ではなく、既存 allocator と並行できる小さな descriptor table に限定するのがよいです。allocator 自体は Review-1 から Review-4 の主要リスクをかなり潰せていますが、実描画 path での `Stage()` タイミング、retire fence value の渡し方、Grow 発生時の挙動はまだ実運用で検証されていません。

## Review-4 対応確認

### 対応済み

- `RunContiguousAllocTest()` は `AllocContiguous(1)` のあとに `FreeContiguous(single)` してから 3-block test を開始するよう修正されています。
- `StagedDescriptorRange { Start, Count }` が追加されています。
- `AllocContiguous()` は `StagedDescriptorRange` を返すようになっています。
- `FreeContiguous(StagedDescriptorRange)` overload が追加されています。
- Debug window に `Run Descriptor Allocator Tests` button が追加され、app startup の自動実行ではなく手動実行になっています。

該当箇所:

- `Renderer/StagedDescriptorAllocator.h`
- `Renderer/StagedDescriptorAllocator_Test.cpp`
- `SampleApp.cpp:788`

## 実用 trial の推奨適用先

最初の適用先は **ShadowMask SRV/UAV descriptor table** がよいです。

理由:

- descriptor 数が 2 個で小さい。
- SRV/UAV の table 境界が明確。
- RayQuery shadow work と文脈が近く、今後 descriptor pressure が増える領域です。
- 失敗しても ShadowMask debug view / shadow path に限定しやすい。
- `AllocContiguous()` と `StagedDescriptorRange` の実用検証に向いています。

避けたい初回適用先:

- Texture table 全体
- Material / instance / frame resource descriptor 全体
- EnvironmentMap descriptor table 全体
- ImGui descriptor heap
- `m_descriptorHeapAllocator` 全置換

これらは寿命や参照範囲が広く、初回 trial としては blast radius が大きすぎます。

## Codex への実装指示案

次の作業は、既存 allocator を置き換えるのではなく、**ShadowMask descriptor allocation だけ StagedDescriptorAllocator に逃がす trial** として行うのがよいです。

指示案:

1. `HelloTextureEngine` に `StagedDescriptorAllocator m_stagedDescriptorAllocator` を追加する。
2. 初期 capacity は小さく、例えば `4` から始める。
3. ShadowMask SRV/UAV 用に `StagedDescriptorRange m_shadowMaskDescriptorRange` を持つ。
4. ShadowMask resource 作成時に `AllocContiguous(2, retireFenceValue)` で range を確保する。
5. SRV は `CpuHandle(range.Start)`、UAV は `CpuHandle(range.Start + 1)` に作成する。
6. RenderGraph binding では `GpuHandle(range.Start)` / `GpuHandle(range.Start + 1)` を返す。
7. frame の描画前に `Stage(completedFenceValue)` を 1 回呼ぶ場所を明確にする。
8. scene/resource release では `FreeContiguous(range)` する。
9. Grow を意図的に起こす debug path または小さい capacity で試し、pending GPU heap が壊れないことを確認する。
10. 既存 `m_descriptorHeapAllocator` は他用途ではそのまま残す。

## Findings

### 1. branch doc の API 表記が一部古い

Severity: Low

`growable-descriptor-heap.md` の API snippet が古い戻り値のままです。

該当箇所:

- `doc/branch/feature/growable-descriptor-heap.md:82`
- `doc/branch/feature/growable-descriptor-heap.md:84`
- `doc/branch/feature/growable-descriptor-heap.md:154`
- `doc/branch/feature/growable-descriptor-heap.md:155`

現在の実装では `AllocContiguous()` は `StagedDescriptorRange` を返し、`FreeContiguous(StagedDescriptorRange)` が追加されています。doc は以下のように更新した方がよいです。

```cpp
StagedDescriptorHandle Allocate(UINT64 retireFenceValue);
StagedDescriptorRange AllocContiguous(UINT count, UINT64 retireFenceValue);
void Free(StagedDescriptorHandle handle);
void FreeContiguous(StagedDescriptorRange range);
void Stage(UINT64 completedFenceValue);
```

### 2. `RunStagedAllocatorTests()` の結果表示はまだ `printf`

Severity: Low

Debug UI から手動実行できるようになったのはよいです。ただし結果出力は `printf` のままなので、Visual Studio の Output window で見たい場合は `OutputDebugStringA` に寄せた方が扱いやすいです。

これは実用 trial のブロッカーではありません。

### 3. `Stage()` の呼び出し位置はまだ実コードで検証されていない

Severity: Medium

allocator 単体としては形になっていますが、実用 trial では `Stage(completedFenceValue)` をどの frame timing で呼ぶかが重要です。

おすすめは、descriptor を作成/更新したあと、draw/dispatch command を積む前に 1 回だけ呼ぶ場所を明確にすることです。Grow が起きる場合は、old GPU heap の retire fence value と `Stage(completedFenceValue)` の値が対応していることを確認してください。

## 判定

- 小さい実用 trial: 可能。
- 最初の適用先: ShadowMask SRV/UAV が最適。
- 全体置換: まだ早い。
- trial 前に直すとよいもの: branch doc の古い API 表記。

## 次のステップ

1. `growable-descriptor-heap.md` の API 表記を `StagedDescriptorRange` 対応に更新する。
2. ShadowMask SRV/UAV だけを `StagedDescriptorAllocator` に載せる小さな trial を行う。
3. `Stage()` の呼び出し位置と fence value の対応を実コード上で確認する。
4. 手動 Debug UI test と ShadowMask debug view で動作確認する。