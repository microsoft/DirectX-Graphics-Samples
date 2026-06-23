# Review: feature/growable-descriptor-heap @ 10bc56be51ec

対象コミット:

- `10bc56be51ec0e2adda2951fcd778cc49eb7c54e`
- `Step 3: StagedDescriptorAllocator を単一 GPU ヒープに統合`

## 結論

Step 3 の「StagedDescriptorAllocator を main shader-visible heap の予約領域に統合する」方向は正しいです。

Review-6 で問題だった同一 `CBV/SRV/UAV` heap type の2本同時 bind は解消されており、D3D12 の基本制約に沿った形になりました。
一方で、現在の実装は同じ予約領域を毎フレーム `CopyDescriptorsSimple()` で書き換えるため、`STATIC_DESCRIPTOR_INVALID_DESCRIPTOR_CHANGE` が発生しています。

この問題は heap flag ではなく、descriptor table の lifetime と root signature range flag の問題です。
次の方針は、**案 B の frame-buffered staged reservation を採用**するのがよいです。

## 評価

### 良い点

- `BeginFrame()` が `m_heap` だけを bind する形になり、同型 shader-visible heap 2本 bind の問題が消えています。
- `SimpleDescriptorHeapAllocator` に capacity を渡し、通常 allocator が staged 予約領域を消費しないようにした点はよいです。
- `StagedDescriptorAllocator` が GPU heap を所有しない形になり、old GPU heap の deferred release 管理が不要になりました。
- `GpuHandle(slot)` が main heap の予約領域を指すため、RenderGraph binding と直接 compute pass の両方が同じ bind 済み heap から参照できます。
- Step 3 の位置づけとして、完全な growable heap ではなく bounded staged region に一度寄せた判断は、trial として扱いやすいです。

### 注意点

- 現在の `StagedDescriptorAllocator` は growable というより、`reservedCount` まで grow できる bounded staged allocator です。
- `kStagedDescriptorReservedCount = 64` は main heap 末尾の固定予約であり、予約数を超えたら main heap 自体を再作成しない限り拡張できません。
- ShadowMask SRV/UAV だけなら十分ですが、future transient descriptors まで載せるなら per-frame 容量設計が必要です。

## Findings

### 1. `D3D12_DESCRIPTOR_HEAP_FLAG_DESCRIPTORS_VOLATILE` は存在しない

Severity: High

該当箇所:

- `doc/branch/feature/growable-descriptor-heap.md:277`

doc の案 A は heap flag として `D3D12_DESCRIPTOR_HEAP_FLAG_DESCRIPTORS_VOLATILE` を追加する、としていますが、この flag は存在しません。

`DESCRIPTORS_VOLATILE` は `D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE` として root signature の descriptor range に付けるものです。
したがって案 A はそのままではコンパイルできず、方針としても修正が必要です。

もし volatile で解く場合は、ShadowMask が使う root signature range を `D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE` にする必要があります。
例えば RayQuery shadow の compute root signature は現在 `uavRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0)` の default flag なので、ここは明示的に range flag を設計する必要があります。

ただし、今回は volatile 化を主案にしない方がよいです。
descriptor の変更可能性を root signature 側へ広げるより、書き換える heap 領域をフレームごとに分ける方が allocator の lifetime 管理として自然です。

### 2. 単一予約領域を毎フレーム上書きしているため STATIC 制約に当たる

Severity: High

該当箇所:

- `Renderer/StagedDescriptorAllocator.h:139`
- `D3D12HelloTexture.cpp:1913`

`Stage()` は main heap の `kMainHeapDescriptorCount` 以降の固定予約領域へコピーします。
しかし前フレームの command list がその descriptor table を STATIC として参照している間に、次フレームの `Stage()` が同じ CPU descriptor handle を書き換えると、D3D12 debug layer は正しくエラーを出します。

これは `Stage()` の呼び出し位置だけでは解けません。
`PopulateCommandList()` 前に移しても、前フレームの GPU execution が完了していない限り、同じ descriptor slot を書き換える危険は残ります。

### 3. `reservedCount` 超過が `assert` だけで Release build では守れない

Severity: Medium

該当箇所:

- `Renderer/StagedDescriptorAllocator.h:319`

`Grow()` は `newCapacity <= m_reservedCount` を `assert` で確認しています。
しかし Release build では `assert` が消えるため、予約領域を超えて CPU heap だけ grow し、`Stage()` が main heap の予約外へ `CopyDescriptorsSimple()` する可能性があります。

ここは実行時にも止める必要があります。
少なくとも `if (newCapacity > m_reservedCount) { ThrowIfFailed(E_OUTOFMEMORY); }` 相当の hard failure にするか、allocator API を失敗可能にするべきです。

### 4. test は新 API に追従したが、単一 heap 統合の lifetime 問題は検出できない

Severity: Low

該当箇所:

- `Renderer/StagedDescriptorAllocator_Test.cpp`

テスト用外部 heap を作って `Init()` に渡す形に変わったのは良いです。
ただし現状の test は command list submission や root signature の static/volatile 制約を通らないため、今回の runtime error は検出できません。

この test は allocator bookkeeping の smoke test として維持し、heap lifetime は実描画 path または小さな GPU integration test で確認するのがよいです。

## 方針

### 採用: 案 B の frame-buffered staged reservation

次は予約領域をフレーム数分に分割する方針で進めるのがよいです。

```text
stagedBase = kMainHeapDescriptorCount
perFrameCapacity = kStagedDescriptorReservedCount
totalReserved = perFrameCapacity * kFrameCount

Frame 0: [stagedBase + 0 * perFrameCapacity, stagedBase + 1 * perFrameCapacity)
Frame 1: [stagedBase + 1 * perFrameCapacity, stagedBase + 2 * perFrameCapacity)
Frame 2: [stagedBase + 2 * perFrameCapacity, stagedBase + 3 * perFrameCapacity)
```

この場合、main heap は次のように確保します。

```cpp
heapDesc.NumDescriptors =
    kMainHeapDescriptorCount + kStagedDescriptorReservedCount * kFrameCount;
```

`StagedDescriptorAllocator` は logical slot を維持しつつ、現在 frame の copy destination と GPU handle base だけを切り替えます。

```cpp
void SetFrameIndex(UINT frameIndex);
UINT CurrentStageOffset() const
{
    return m_stageOffset + m_frameIndex * m_reservedCount;
}
```

`Stage()` と `GpuHandle()` は必ず同じ `CurrentStageOffset()` を使います。

```cpp
dstStart.ptr = m_mainCpuStart.ptr + (CurrentStageOffset() * m_increment);
gpu.ptr = m_mainGpuStart.ptr + ((CurrentStageOffset() + slot) * m_increment);
```

`RenderFrame()` では `Stage()` 前、かつ `BuildRenderPasses()` より前に frame index を同期します。

```cpp
m_stageAllocator.SetFrameIndex(m_currentFrameIndex);
m_stageAllocator.Stage();
```

この構成なら、前フレームが参照している descriptor slot を次フレームが上書きしないため、STATIC descriptor のままでも成立しやすくなります。

### 非推奨: heap flag で volatile にする案

heap flag としての `DESCRIPTORS_VOLATILE` は存在しないため不可です。

root signature range に `D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE` を付ける案はありますが、今回の主解決にはしない方がよいです。
理由は以下です。

- ShadowMask は graphics pass の SRV と compute pass の UAV の両方で使われるため、どの root signature range を volatile にするかの監査が必要。
- static descriptor の最適化を捨てる範囲が広がりやすい。
- allocator の本質的な問題は「同じ slot を in-flight 中に上書きしている」ことなので、slot lifetime を分離する方が素直。

volatile は、どうしても同じ slot を短い周期で書き換える設計に寄せる場合の補助策として検討する程度でよいです。

## 実装時のチェックリスト

1. `kStagedDescriptorReservedCount` を per-frame capacity として扱う。
2. main heap の総数を `kMainHeapDescriptorCount + kStagedDescriptorReservedCount * kFrameCount` にする。
3. `SimpleDescriptorHeapAllocator.Init(..., kMainHeapDescriptorCount)` は維持し、通常 allocator が staged 領域を使わないようにする。
4. `StagedDescriptorAllocator` に `SetFrameIndex()` を追加する。
5. `Stage()` と `GpuHandle()` が同じ frame offset を使うことを assert しやすい構造にする。
6. `reservedCount` 超過は Release build でも hard failure にする。
7. RayQuery shadow の root signature range flag は default のままで通るか確認する。volatile を足す場合は heap flag ではなく range flag にする。
8. resize / recreate 時に `m_shadowMaskRange` を再利用する現在の挙動で、descriptor 内容だけ更新されることを確認する。
9. Debug layer で `STATIC_DESCRIPTOR_INVALID_DESCRIPTOR_CHANGE` が消えることを確認する。
10. Debug UI の staged allocator test は、frame offset 付き API に追従させる。

## 判定

- Step 3 の単一 GPU heap 統合: 採用。
- 現在の単一予約領域を毎フレーム上書きする実装: 修正必要。
- 案 A の heap flag volatile: 不可。
- 案 B の frame-buffered staged reservation: 推奨。
- `reservedCount` 超過の `assert` のみ検出: Release でも止まる形に修正。
