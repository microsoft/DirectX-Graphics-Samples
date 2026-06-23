# Review: feature/growable-descriptor-heap @ 1826bb741af0

対象コミット:

- `1826bb741af0d49034e25eb79acb461fdf510ab0`
- `Update doc: record runtime fixes for ShadowMask trial`

直近2コミット:

- `1826bb741af0`
  - `Update doc: record runtime fixes for ShadowMask trial`
  - `doc/branch/feature/growable-descriptor-heap.md` に ShadowMask trial の runtime error 対応メモを追加。
- `6223ff54d3fe`
  - `Fix ShadowMask staged allocator runtime errors:`
  - `Stage()` を `RenderFrame()` 先頭へ移動。
  - `StagedDescriptorAllocator::GetGpuHeap()` を追加。
  - `BeginFrame()` で main heap と staged GPU heap を同時に `SetDescriptorHeaps()` するよう変更。

## 結論

`Stage()` を `PopulateCommandList()` より前へ移動した方向は妥当です。
`CopyDescriptorsSimple()` が command list に bind 済みの GPU heap を更新してしまう問題を避けるため、
描画/dispatch command を記録する前に staging を完了する、という整理は正しいです。

ただし、`BeginFrame()` で main heap と staged GPU heap を同時に bind する修正は D3D12 の制約に合っていません。
今回の最新状態は、ShadowMask trial の実装としてまだブロッカーを残しています。

ビルドと実行確認はユーザー側で実施済み。
この review では、主に D3D12 の descriptor heap 制約と test coverage の観点で確認しました。

## Findings

### 1. 同じ `CBV/SRV/UAV` shader-visible descriptor heap を2本同時に bind している

Severity: High

該当箇所:

- `D3D12HelloTexture.cpp:2304`

```cpp
ID3D12DescriptorHeap* ppHeaps[] = {m_heap.Get(), m_stageAllocator.GetGpuHeap()};
m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
```

`m_heap` は main の `D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV` shader-visible heap です。
`m_stageAllocator` も `D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV` として初期化されています。

D3D12 では、command list に同時 bind できる shader-visible heap は heap type ごとに1つです。
つまり `CBV/SRV/UAV` heap を2つ同時に渡す構成は成立しません。

このため、ShadowMask staged descriptor を使う場合は次のどちらかの設計に寄せる必要があります。

- staged allocator の CPU 側 descriptor を、現在 bind している main GPU heap にコピーして使う。
- command list 中で heap を切り替え、その区間では main heap 上の descriptor table を参照しないようにする。

後者は既存 pass graph 全体への影響が大きいので、今回の trial では前者の方が安全です。

### 2. branch doc が無効な heap bind 対策を「修正」として記録している

Severity: Medium

該当箇所:

- `doc/branch/feature/growable-descriptor-heap.md:196`

doc では runtime error 対策として、
`BeginFrame()` で main heap と staged GPU heap を両方 `SetDescriptorHeaps()` すると記録されています。

しかし、これは上記の D3D12 制約に反するため、設計メモとして残すと次回以降の修正も同じ方向に戻りやすいです。
doc 側も「同型 shader-visible heap は同時 bind 不可。staged descriptor は bind 中の heap に統合する必要がある」と更新した方がよいです。

## Stage Allocator Test 評価

`Renderer/StagedDescriptorAllocator_Test.cpp` の導入は、allocator 単体の smoke test として有効です。

良い点:

- `Allocate()` / `Free()` / 再利用 / grow / `Stage()` の基本動作を押さえている。
- `AllocContiguous(1)` の edge case を明示的に検証している。
- `AllocContiguous(3)` で連続 slot が実際に descriptor increment 間隔で並ぶことを確認している。
- `FreeContiguous(StagedDescriptorRange)` 後に同じ連続 range を再確保できることを確認している。
- app startup 自動実行ではなく、Debug UI の手動ボタンから実行する形に分離されている。

制限:

- `Stage()` は「クラッシュしない」確認に近く、実 descriptor を CPU heap に書いて GPU heap 側へコピーされたことまでは見ていない。
- pending GPU heap の fence release は、`UINT64_MAX` でまとめて通しているため、未完了 fence では残り、完了 fence で解放されることまでは検証していない。
- `printf` 出力なので、Debug UI からの実行結果が画面上に残らない。
- command list / root descriptor table / heap bind まで含む統合バグは捕まえられない。

判定としては、**入れてよい test** です。
ただし位置づけは allocator bookkeeping の smoke test であり、D3D12 統合確認の代替ではありません。

## 推奨修正

1. `BeginFrame()` の `SetDescriptorHeaps()` を、同型 `CBV/SRV/UAV` heap 2本同時 bind しない形へ戻す。
2. ShadowMask descriptor の staged 内容を main heap 側に統合する設計を検討する。
3. `growable-descriptor-heap.md` の runtime fix 記録を、D3D12 heap 制約に合わせて修正する。
4. `RunStagedAllocatorTests()` は `bool` を返すか、Debug UI に last result を表示する。
5. 可能なら test に null SRV/UAV descriptor 作成と `Stage()` 後の copy 経路確認を追加する。
6. pending GPU heap の fence release を観測できる debug/test hook を検討する。

## 判定

- `Stage()` の呼び出し位置修正: 妥当。
- staged GPU heap を main heap と同時 bind する修正: 不可。
- Stage allocator test 導入: 有効。ただし smoke test の範囲。
- ShadowMask trial: 現状のままでは descriptor heap bind 設計を直す必要あり。
