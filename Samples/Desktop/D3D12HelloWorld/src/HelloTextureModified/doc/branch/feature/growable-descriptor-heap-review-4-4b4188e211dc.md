# Review: feature/growable-descriptor-heap @ 4b4188e211dc

対象コミット:

- `4b4188e211dc`
- `Update branch doc with review-3 changes`

## 結論

実用段階へ「限定的に進む」ことは可能です。

ただし、いきなり既存の main descriptor heap 全体を置き換える段階ではまだありません。まずは実際の描画経路から切り離しやすい、小さな descriptor table か debug / optional resource で試すのがよいです。

理由は、Review-1 から Review-3 で指摘した根本リスクの多くは解消されていますが、まだ「テストの信頼性」と「実運用時の API 形」が完全ではないためです。

## 対応済みとして確認できたこと

- `SetPendingFenceValue()` は削除され、`Allocate(retireFenceValue)` / `AllocContiguous(count, retireFenceValue)` になっています。
- Grow 時の old GPU heap deferred release に fence value が API 経由で必ず渡る形になっています。
- `m_slotState` が導入され、double-free / out-of-range / 未割当 slot の解放を debug assert で検出できます。
- `AllocContiguous(1)` の edge case は `FindContiguousRun()` で処理されています。
- `FreeContiguous(first, count)` が追加されています。
- `CpuHandle()` / `GpuHandle()` に `slot < m_capacity` assert が追加されています。
- `RunStagedAllocatorTests()` は app startup path から外れています。
- `growable-descriptor-heap.md` は Review-1/2/3 の反映内容を追える状態になっています。
- `git diff --check` は問題ありません。

## 実用段階へ進める場合の推奨範囲

最初の適用先は、以下の条件を満たすものがよいです。

- descriptor 数が少ない。
- descriptor table の境界が明確。
- 失敗しても描画全体を壊しにくい。
- scene reload / resource reload で再作成しやすい。

候補:

1. ShadowMask SRV/UAV のような小さい table
2. Debug view 用 descriptor
3. EnvironmentMap 系 descriptor table の一部

逆に、最初から以下を置き換えるのは避けたいです。

- 全 `m_descriptorHeapAllocator`
- texture table 全体
- ImGui descriptor heap 周辺
- frame resource / material / instance buffer など寿命が混ざる領域全部

## Findings

### 1. contiguous test の期待値が壊れている

Severity: Medium

`RunContiguousAllocTest()` で、先に `AllocContiguous(1)` を実行したあと、さらに `AllocContiguous(3)` を実行しています。

該当箇所:

- `Renderer/StagedDescriptorAllocator_Test.cpp:93`
- `Renderer/StagedDescriptorAllocator_Test.cpp:102`
- `Renderer/StagedDescriptorAllocator_Test.cpp:109`

この時点で used slot は `1 + 3 = 4` になるはずですが、test は `alloc.Used() != 3` を失敗条件にしています。

つまり、Review-3 の edge case test を追加したことで、既存の期待値が古くなっています。もしこの test を実行すれば失敗する可能性が高いです。

推奨修正:

```cpp
if (alloc.Used() != 4 || alloc.Capacity() != 8)
```

または `single` を `Free(single)` してから 3-block test を始める形でもよいです。

### 2. `FreeContiguous()` は追加されたが、range 型はまだない

Severity: Low to Medium

`FreeContiguous(first, count)` が追加され、前回より安全になりました。

該当箇所:

- `Renderer/StagedDescriptorAllocator.h:233`

ただし、`AllocContiguous()` の戻り値はまだ start slot の `StagedDescriptorHandle` のみです。利用側は count を別途覚える必要があります。

最初の実用段階では許容できますが、descriptor table を複数箇所で使い始めるなら、次の段階で range 型にする価値があります。

推奨案:

```cpp
struct StagedDescriptorRange
{
    UINT Start = UINT_MAX;
    UINT Count = 0;
};
```

そして `AllocContiguous()` は range を返し、`FreeContiguous(range)` で解放する形にすると、count mismatch を減らせます。

### 3. test file は build には入っているが、実行 path がない

Severity: Low

`RunStagedAllocatorTests()` は app startup から外れました。これは正しい方向です。

一方で、`StagedDescriptorAllocator_Test.cpp` は project に残っており、関数は compile されますが、明示的に呼ぶ test harness はまだありません。

該当箇所:

- `Renderer/StagedDescriptorAllocator_Test.cpp:171`

今後この allocator を実用に近づけるなら、以下のどちらかを決めたいです。

- developer-only command / debug menu / explicit test hook から呼ぶ。
- app project からは外し、別の unit/smoke test target に移す。

現状のままでも build 確認にはなりますが、allocator behavior の回帰検出としては弱いです。

## 実用化の進め方

次は「本格置換」ではなく「小さい実適用」がよいです。

推奨ステップ:

1. `RunContiguousAllocTest()` の期待値を修正する。
2. `StagedDescriptorAllocator` を既存 allocator と並行導入する。
3. まず ShadowMask など小さい descriptor table にだけ使う。
4. 1 frame ごとに `Stage(completedFenceValue)` を呼ぶ場所を明確にする。
5. Grow が起きるケースを意図的に作って、古い GPU heap の deferred release が破綻しないか確認する。
6. 問題がなければ EnvironmentMap descriptor table など少し大きい範囲へ広げる。

## 判定

- Prototype としてはかなり改善済み。
- Small real-use trial へ進むのは可能。
- Main descriptor heap 全体の置換はまだ早い。
- 最低限、contiguous test の期待値だけは実用 trial 前に直したいです。

## 次の作業候補

1. `RunContiguousAllocTest()` の used count 期待値を修正。
2. 小さい実適用先を 1 つ選ぶ。
3. `Stage()` 呼び出し位置と retire fence value の渡し方を実コード上で固める。
4. 必要なら `StagedDescriptorRange` 導入を検討する。