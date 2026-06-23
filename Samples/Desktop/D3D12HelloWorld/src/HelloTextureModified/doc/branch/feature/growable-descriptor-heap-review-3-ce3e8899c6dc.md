# Review: feature/growable-descriptor-heap @ ce3e8899c6dc

対象コミット:

- `ce3e8899c6dc`
- `Update branch doc with review-2 changes`

## 結論

Review-2 の指摘にはかなり素直に対応されています。

`SetPendingFenceValue()` を廃止し、`Allocate(retireFenceValue)` / `AllocContiguous(count, retireFenceValue)` にしたことで、Grow 時に fence value が必ず API 上で渡る形になりました。また、`m_slotState` による double-free / out-of-range 検出、contiguous test の改善、startup path からの smoke test 呼び出し削除も確認できました。

残りは `AllocContiguous(1)` の edge case と、contiguous range の解放 API を整えることが主です。

## 確認済み

- `SetPendingFenceValue()` は削除済み。
- `Allocate(UINT64 retireFenceValue)` に変更済み。
- `AllocContiguous(UINT count, UINT64 retireFenceValue)` に変更済み。
- `m_slotState` が追加され、`Allocate()` / `AllocContiguous()` / `Free()` で slot 状態を assert しています。
- `DescriptorIncrement()` が追加され、contiguous test は実 descriptor increment を使うようになっています。
- `InitializeFrameResources()` から `RunStagedAllocatorTests()` 呼び出しは削除済み。
- `runStart` 未使用変数は削除済み。
- `growable-descriptor-heap.md` と review file naming の更新を確認しました。
- 主要ファイルの EOL は CRLF です。
- `git diff --check` は問題ありませんでした。

## Findings

### 1. `AllocContiguous(1, retireFenceValue)` が失敗する

Severity: Medium

`FindContiguousRun()` は `count == 1` を処理できていません。

該当箇所:

- `Renderer/StagedDescriptorAllocator.h:293`
- `Renderer/StagedDescriptorAllocator.h:303`

現在の実装は `consecutive = 1` から始めて、loop 内で `consecutive >= count` を判定します。しかし `count == 1` の場合、最初の free slot で即成功すべきところ、loop に入るまで判定されません。

そのため、API としては `count > 0` を許しているにもかかわらず、`AllocContiguous(1, ...)` が失敗します。

推奨修正:

```cpp
if (count == 1)
{
    return sorted[0];
}
```

`FindContiguousRun()` の sort 後、loop 前に入れるのが自然です。

### 2. contiguous block の解放 API がない

Severity: Medium

`AllocContiguous(count, ...)` は start slot だけを返します。

該当箇所:

- `Renderer/StagedDescriptorAllocator.h:143`
- `Renderer/StagedDescriptorAllocator.h:177`

利用側は `count` を別途覚えて、`Free()` を count 回呼ぶ必要があります。これは descriptor table 用 API としては少し危険です。部分解放、解放漏れ、間違った count での解放が起きやすくなります。

推奨案:

- `StagedDescriptorRange { UINT Start; UINT Count; }` を導入する。
- `AllocContiguous()` は range を返す。
- `FreeContiguous(range)` を用意する。

最小修正なら以下でもよいです。

```cpp
void FreeContiguous(StagedDescriptorHandle first, UINT count);
```

### 3. `CpuHandle()` / `GpuHandle()` に範囲 assert がない

Severity: Low to Medium

slot-only handle にしたことで、handle 解決の入口は `CpuHandle()` / `GpuHandle()` になりました。

該当箇所:

- `Renderer/StagedDescriptorAllocator.h:183`
- `Renderer/StagedDescriptorAllocator.h:191`

ここで `slot < m_capacity` を assert しておくと、out-of-range slot の利用を早く検出できます。

推奨修正:

```cpp
assert(slot < m_capacity);
```

必要なら `m_slotState[slot] == SlotState::Allocated` も debug assert できます。ただし staging copy や descriptor overwrite の用途によっては free slot の CPU handle を取りたいケースもあり得るため、まずは範囲 assert だけでも十分です。

## 総評

Review-2 の High severity 指摘はかなり解消されています。

残っているのは、主に contiguous descriptor range を本格的に使う前の API 仕上げです。`AllocContiguous(1)` の edge case と range 解放 API を整えれば、この prototype は次の検証段階に進めやすくなります。