# Review: feature/growable-descriptor-heap @ a6cbc0a383ea

対象コミット:

- `a6cbc0a383ea`
- `Update branch doc with review-driven changes`

## 結論

前回レビューへの対応はかなり進んでいます。

特に、`StagedDescriptorHandle` を slot-only にした点、`CpuHandle()` / `GpuHandle()` で都度解決する形にした点、古い shader-visible heap を pending list に逃がす設計、`AllocContiguous()` の追加は方向性として正しいです。

ただし、まだ mainline に入れる前に直したい点が残っています。主に、deferred release API が呼び出し側に壊れやすい契約を要求している点、`Free()` の二重解放検出がない点、smoke test が実質的に検証できていない点です。

## 良い点

- `StagedDescriptorHandle` が `Index` のみになり、Grow 後の stale CPU/GPU handle 問題は解消方向に進んでいます。
- `CpuHandle(slot)` / `GpuHandle(slot)` で現在の heap から handle を都度解決する設計はよいです。
- `Grow()` 時に古い GPU heap を `m_pendingGpuHeaps` に保持するようになり、即時破棄問題に対する設計が入っています。
- `AllocContiguous(count)` が追加され、descriptor table 用の連続 slot 確保に進める土台ができています。
- branch doc は前回レビューの論点を反映しており、意図が読みやすくなっています。

## Findings

### 1. Grow の deferred release が `SetPendingFenceValue()` 呼び忘れに弱い

Severity: High

`Grow()` は古い GPU heap を `m_pendingGpuHeaps` に退避していますが、退避時に使う fence value は `m_pendingFenceValue` です。

該当箇所:

- `Renderer/StagedDescriptorAllocator.h:82`
- `Renderer/StagedDescriptorAllocator.h:95`
- `Renderer/StagedDescriptorAllocator.h:260`

`Allocate()` / `AllocContiguous()` 内部で grow が発生するため、呼び出し側が事前に `SetPendingFenceValue()` を呼び忘れると、既定値 `0` のまま old GPU heap が pending list に入ります。その後 `Stage(completedFenceValue)` で `completedFenceValue >= 0` ならすぐ回収されます。

つまり、API の使い方を少し間違えるだけで「deferred release したつもりが即時 release」に戻ります。

推奨:

- `Grow()` が必要になる可能性のある API に retire fence value を渡す。
- あるいは allocator が command queue / fence provider から安全な fence value を取得する。
- 少なくとも `m_pendingFenceValue == 0` のまま grow する場合は debug assert する。

例:

```cpp
StagedDescriptorHandle Allocate(UINT64 retireFenceValue);
StagedDescriptorHandle AllocContiguous(UINT count, UINT64 retireFenceValue);
```

### 2. `Free()` が二重解放や範囲外 slot を検出できない

Severity: High

`Free()` は `handle.Index` をそのまま free list に戻しています。

該当箇所:

- `Renderer/StagedDescriptorAllocator.h:189`
- `Renderer/StagedDescriptorAllocator.h:196`

二重に `Free()` された場合、同じ slot が free list に複数回入ります。その後、同じ descriptor slot が複数の caller に割り当てられる可能性があります。

また `handle.Index >= m_capacity` の範囲外 handle も検出していません。

推奨:

- debug build だけでも `m_isFree` / `m_allocated` のような slot 状態配列を持つ。
- `Free()` 時に範囲外、二重 free を assert する。
- `Allocate()` / `AllocContiguous()` 時に状態を allocated に更新する。

これは growable allocator の信頼性に直結するので、mainline 前に入れたいです。

### 3. `Stage()` が free slot を含む `[0, m_maxUsedIndex)` 全体をコピーする

Severity: Medium

`Stage()` は `m_maxUsedIndex` まで一括コピーします。

該当箇所:

- `Renderer/StagedDescriptorAllocator.h:118`
- `Renderer/StagedDescriptorAllocator.h:133`

この設計自体は単純でよいですが、free slot に古い descriptor が残ることを前提にしています。GPU 側で free slot を参照しない限り問題ありません。

ただし、二重 free や stale handle があると、古い descriptor が見え続けるため症状が分かりづらくなります。Finding 2 の slot 状態管理と合わせて安全にしたいです。

### 4. contiguous allocation test が実質的に検証できていない

Severity: Medium

`RunContiguousAllocTest()` の連続性チェックは、descriptor increment ではなく `alloc.Capacity()` を使っています。

該当箇所:

- `Renderer/StagedDescriptorAllocator_Test.cpp:108`
- `Renderer/StagedDescriptorAllocator_Test.cpp:113`

さらに、条件に入っても何も失敗させず `skip for now` になっています。つまり contiguous allocation の重要な性質を確認できていません。

推奨:

- `DescriptorIncrement()` の accessor を追加するか、CPU handle 差分が一定であることを別の形で検証する。
- `block.Index + i` が期待通り連続 slot であることをテストする。
- `AllocContiguous()` 後に該当 slot が再度 `Allocate()` で返らないことを確認する。

### 5. smoke test が app startup path に残っている

Severity: Low to Medium

`RunStagedAllocatorTests()` は `_DEBUG` guard されていますが、まだ `InitializeFrameResources()` から呼ばれています。

該当箇所:

- `D3D12HelloTexture.cpp:138`
- `D3D12HelloTexture.cpp:139`

前回よりは改善されていますが、通常の debug 起動で毎回 allocator test が走ります。prototype としては許容できますが、branch を mainline に入れるなら、明示的な developer test path に寄せるか、少なくとも compile-time flag をもう一段分けたいです。

### 6. `runStart` が未使用

Severity: Low

`FindContiguousRun()` 内の `runStart` は代入されていますが使われていません。

該当箇所:

- `Renderer/StagedDescriptorAllocator.h:281` 付近

実害は小さいですが、警告や読み手の混乱を避けるため削除してよいです。

## 前回レビューへの対応状況

- slot-only handle: 対応済み。
- CPU/GPU accessor: 対応済み。
- 古い shader-visible heap の deferred release: 設計は入ったが、API 契約がまだ危うい。
- smoke test の通常 app path からの除去: `_DEBUG` guard までは対応。ただし debug 起動 path には残っている。
- stale handle / grow 後 behavior test: まだ弱い。特に double free と contiguous allocation の検証が足りない。

## 推奨次アクション

1. `Free()` の double-free / out-of-range 検出を入れる。
2. grow 時の retire fence value を API 上で必ず渡る形にする。
3. contiguous allocation test を実際に失敗検出できるテストに直す。
4. `RunStagedAllocatorTests()` を debug app startup から外すか、明示 flag 化する。
5. `runStart` など未使用コードを削除する。

## 総評

方向性は正しいです。

ただし、この allocator は将来的に descriptor heap の基盤になるため、少しの不整合が描画破綻や descriptor reuse bug につながります。今の状態は prototype から一歩進んだ段階ですが、mainline に入れるなら slot 状態管理と fence API の堅牢化を先に入れるのがよいです。