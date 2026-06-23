# feature/growable-descriptor-heap

## 目的

現在の `kMainHeapDescriptorCount` 手動計算方式を脱却し、
必要に応じて自動で増加する growable descriptor heap を導入する。

## 背景と問題

現状:
- 単一の shader-visible descriptor heap を `kMainHeapDescriptorCount` で固定確保
- 新しい descriptor を追加するたびにカウントを手動で更新する必要がある
- カウントミスは実行時 assert またはクラッシュにつながる

## 理想形

```
CPU heap (D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 非表示)
  └── アプリケーションが自由に Alloc/Free
  └── 格納されている descriptor は常に最新

GPU heap (D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shader-visible)
  └── 毎フレーム、CPU heap の内容を CopyDescriptors で一括コピー
  └── コピー元の範囲は [0, 最大使用スロット数)
```

### なぜ複雑でないか

- `CopyDescriptors` は1回のAPI呼び出しで GPU heap を更新できる
- 「全部コピー」する場合、dirty 追跡は不要（全部コピーするだけ）
- ディスクリプタ数が数千でもコピーコストは数十μs 程度

### 本当に難しい部分

1. **Transient descriptor のライフタイム管理**
   - 中間レンダーパス用の descriptor はフレームごとに確保・解放される
   - GPU 実行完了まで解放できない
   - 「いつ再利用可能か」の追跡には Fence ベースの管理が必要
   - リングバッファ方式は最大フレーム数を超えると破綻する

2. **設計判断**
   - 全コピー vs dirty 追跡
   - Permanent と Transient の heap 分離 vs 統一
   - RAII ハンドルの型設計

## 進め方

1. 列挙型による slot カウント自動化（中間段階）← 実施済み
2. CPU 非表示 heap + 毎フレーム staging の試作
3. Transient descriptor のライフタイム管理の設計と実装

## 実施内容

### Step 1: `PersistentSrvSlot` 列挙型の導入

`D3D12HelloTexture.h` に `PersistentSrvSlot` 列挙型を追加。

```cpp
enum PersistentSrvSlot : UINT
{
    DepthStencilSrvSlot,
    LightPassColorSrvSlot,

    PersistentSrvSlotCount,
};
```

`kMainHeapDescriptorCount` の末尾の `+ 2` を `+ PersistentSrvSlotCount` に変更。
新しい固定 SRV スロットを追加する場合は、列挙子を1行追加するだけでカウントが自動反映される。
デスクリプタの確保順が enum の定義順と一致することを前提としている（現在の実装と変わらない）。

### Step 2: `StagedDescriptorAllocator` プロトタイプ

`Renderer/StagedDescriptorAllocator.h` に実装。

#### 設計

2つのヒープを管理:
- **CPU heap** (`D3D12_DESCRIPTOR_HEAP_FLAG_NONE`): 常に最新の記述子を保持。全 Alloc/Free/Write はここに。
- **GPU heap** (`D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE`): 毎フレーム `CopyDescriptorsSimple` で CPU から全コピー。

```cpp
StagedDescriptorHandle Allocate(UINT64 retireFenceValue);         // 1 slot 確保
StagedDescriptorRange AllocContiguous(UINT count, UINT64 retireFenceValue); // count 連続 slot 確保
void Stage(UINT64 completedFenceValue);                          // CPU → GPU コピー + 古い GPU heap 解放
void Free(StagedDescriptorHandle handle);                        // slot を解放
void FreeContiguous(StagedDescriptorRange range);                // 連続 slot を解放
UINT DescriptorIncrement() const;                                // descriptor handle increment size
```

#### Grow 動作

空きがない場合、自動的に Grow する:
1. 現在の容量 + N の新しい CPU heap / GPU heap を作成
2. 既存の記述子を古い CPU heap から新しい CPU heap へコピー
3. 新しい GPU heap にも同様にコピー
4. 新しい空きスロットを FreeIndices に追加
5. 古い GPU heap を pending list に移行（即時解放しない）
6. `Stage(completedFenceValue)` で fence 完了後に pending list から解放

#### Review 対応

Review ([review-1](./growable-descriptor-heap-review-1-cc4f63fed3b3.md) / [review-2](./growable-descriptor-heap-review-2-a6cbc0a383ea.md) / [review-3](./growable-descriptor-heap-review-3-ce3e8899c6dc.md) / [review-5](./growable-descriptor-heap-review-5-bbaa6c1b2458.md)) 指摘に基づく修正:

##### 1. `StagedDescriptorHandle` を slot-only に変更

従来は handle が CPU/GPU 絶対アドレスをキャッシュしていたが、Grow 後に stale になるため廃止。
Handle は `Index` のみ保持し、CPU/GPU handle は `CpuHandle(slot)` / `GpuHandle(slot)` accessor で都度解決する。

```cpp
StagedDescriptorHandle h = alloc.Allocate();
D3D12_CPU_DESCRIPTOR_HANDLE cpu = alloc.CpuHandle(h.Index);
D3D12_GPU_DESCRIPTOR_HANDLE gpu = alloc.GpuHandle(h.Index);
```

##### 2. 古い GPU heap の deferred release (review-2 で修正)

`Grow()` で旧 GPU heap を `m_pendingGpuHeaps` に fence value 付きで退避。
`Stage(completedFenceValue)` 呼び出し時に fence 完了済みの pending heap を解放。

v1 では `SetPendingFenceValue()` で事前登録する方式だったが、
呼び出し忘れで `m_pendingFenceValue == 0` のまま即時解放される危険があった。
v2 で `Allocate(retireFenceValue)` / `AllocContiguous(count, retireFenceValue)` に
変更し、呼び出し側が常に fence value を渡す API に改めた。

##### 3. Smoke test を app startup path から完全除去 (review-2 で修正)

v1 では `#if defined(_DEBUG)` ガードを入れていたが、
review-2 で `InitializeFrameResources()` からの呼び出し自体を削除。
テストは `StagedDescriptorAllocator_Test.cpp` に残し、明示的な test harness から
呼び出す形にした。

##### 4. `AllocContiguous(count)` の追加

連続 `count` 個の descriptor slot を1回の呼び出しで確保（descriptor table 用）。
内部で free list をソートして連続領域を探索。見つからなければ Grow してから再試行。

##### 5. Slot state による二重解放検出 (review-2 で追加)

`m_slotState` (`std::vector<SlotState>`) で各 slot の Free/Allocated 状態を管理。
`Free()` / `Allocate()` / `AllocContiguous()` で assert により
- 二重解放
- 未割当 slot の解放
- 範囲外 index

を debug build で検出する。

##### 6. `AllocContiguous(1)` のエッジケース修正 (review-3 で追加)

`FindContiguousRun()` が `count == 1` の場合に loop 内でのみ判定していたため失敗。
sort 直後に `if (count == 1) return sorted[0]` の early return を追加。

##### 7. `FreeContiguous()` の追加 (review-3 で追加)

`AllocContiguous(count)` で確保した連続ブロックを1回の呼び出しで解放する
`FreeContiguous(StagedDescriptorHandle first, UINT count)` を追加。
利用側が count を覚えておく必要はあるが、部分解放や解放漏れを防ぐ。

##### 8. `CpuHandle()` / `GpuHandle()` に範囲 assert 追加 (review-3 で追加)

両方に `assert(slot < m_capacity)` を追加し、out-of-range slot の利用を早期検出。

#### Smoke test

`Renderer/StagedDescriptorAllocator_Test.cpp` に単体テストを実装。
明示的な test harness からの呼び出しを前提とする。
テスト内容:
- 確保・解放・再利用・Grow・Stage の基本動作
- `AllocContiguous()` の連続確保 (`DescriptorIncrement()` で実際の handle 間隔を検証)
- 解放後の連続再確保

Debug ウィンドウの "Debug" セクション末尾に **"Run Descriptor Allocator Tests"** ボタンを追加。
アプリ起動時には自動実行されず、ボタン押下で手動実行される。

### Review-5 Trial: ShadowMask SRV/UAV への適用

Review-5 推奨の小規模 trial として ShadowMask descriptor table を `StagedDescriptorAllocator` に移行。

変更内容:
- `m_shadowMaskSrv` / `m_shadowMaskUav` (`DescriptorAllocation`×2) を `m_shadowMaskRange` (`StagedDescriptorRange`) に置き換え
- `StagedDescriptorAllocator m_stageAllocator` をエンジンに追加、capacity=4 で初期化
- `AllocContiguous(2, UINT64_MAX)` で SRV+UAV を連続確保
- `CpuHandle(range.Start)` / `CpuHandle(range.Start+1)` で view 作成
- `GpuHandle(range.Start)` / `GpuHandle(range.Start+1)` で RenderGraph binding と直接描画コマンドで使用
- `PopulateCommandList()` 内で `m_stageAllocator.Stage(m_graphicsDevice.CompletedFenceValue())` を毎フレーム呼び出し
- `kMainHeapDescriptorCount` から `kShadowMaskDescriptorCount` (=2) を削除

既存の `SimpleDescriptorHeapAllocator` は他用途で共存継続。

#### 追加修正: ランタイムエラー対応

実行時に以下の D3D12 Validation エラーが発生し、修正:

1. **SET_DESCRIPTOR_TABLE_INVALID** (heap mismatch)
   - ShadowMask UAV GPU handle が staged GPU heap から来るが、command list には main heap しか bind されていなかった
   - 対策: `StagedDescriptorAllocator::GetGpuHeap()` を追加し、`BeginFrame()` で main heap と staged GPU heap を両方 `SetDescriptorHeaps()` する

2. **STATIC_DESCRIPTOR_INVALID_DESCRIPTOR_CHANGE** (CopyDescriptorsSimple 実行時に GPU heap が bind されたまま)
   - `Stage()` が `PopulateCommandList()` 内の `BeginFrame()` 後で呼ばれていたため、GPU heap が既に command list に bind された後に `CopyDescriptorsSimple` が走っていた
    - 対策: `Stage()` を `RenderFrame()` 先頭（`PopulateCommandList()` 前）に移動し、`m_graphicsDevice.CompletedFenceValue()` を使用。前フレームの GPU 作業完了後に GPU  heap を更新する設計に変更。

### Step 3: Staged と Main を単一 GPU ヒープに統合 (本作業)

`StagedDescriptorAllocator` が独自の GPU ヒープを持つのをやめ、`SimpleDescriptorHeapAllocator` と同じメイン GPU ヒープの予約領域にステージングするよう変更。

#### 変更理由

- `BeginFrame()` で 2 つの GPU ヒープ (`m_heap` + `GetGpuHeap()`) を bind していたが、ヒープ数が増えるとドライバのスケジューリングコストが増加する
- 将来のフレームワーク統一に向けた布石

#### 変更内容

**StagedDescriptorAllocator.h:**
- `Init()` のシグネチャ変更:
  ```cpp
  void Init(
      ID3D12Device* device,
      D3D12_DESCRIPTOR_HEAP_TYPE type,
      UINT initialCapacity,
      D3D12_CPU_DESCRIPTOR_HANDLE mainCpuStart,  // 外部 GPU ヒープの CPU ハンドル
      D3D12_GPU_DESCRIPTOR_HANDLE mainGpuStart,   // 外部 GPU ヒープの GPU ハンドル
      UINT stageOffset,                            // 予約領域開始オフセット
      UINT reservedCount);                         // 予約最大スロット数
  ```
- プライベートな `m_cpuHeap` (非表示) は引き続き所有。全 Alloc/Free/Write はここに。
- `m_gpuHeap` / `m_gpuCpuStart` / `m_pendingGpuHeaps` / `CollectCompletedGpuHeaps()` を削除。
- `Stage()`: コピー先を `m_mainCpuStart + stageOffset` に変更。`CopyDescriptorsSimple` は変更なし。
- `GpuHandle(slot)`: `m_mainGpuStart + (stageOffset + slot)` を返す。
- `Grow()`: `assert(newCapacity <= reservedCount)` を追加。reservedCount を超える Grow はバグとみなす。
- `Allocate()` / `AllocContiguous()`: `retireFenceValue` パラメータ削除（GPU ヒープを所有しないため不要）。
- `GetGpuHeap()` を削除。

**SimpleDescriptorHeapAllocator.h:**
- `Init(device, heap)` に `capacity` パラメータ追加（既定値 `UINT_MAX`）。
- メインヒープ拡大時に従来領域を超えないよう制限。

**D3D12HelloTexture.h:**
- `kStagedDescriptorReservedCount = 64` を追加。

**D3D12HelloTexture.cpp:**
- メインヒープを `kMainHeapDescriptorCount + kStagedDescriptorReservedCount` で作成。
- `m_descriptorHeapAllocator.Init(..., kMainHeapDescriptorCount)` で通常領域に制限。
- `m_stageAllocator.Init()` に外部ヒープのハンドル・オフセット・予約数を渡す。
- `BeginFrame()`: `m_heap` のみ bind（`GetGpuHeap()` 削除）。
- `RenderFrame()`: `Stage()` をフェンス無しで呼ぶ。
- `CreateShadowMaskDescriptors()`: `AllocContiguous(2)` をフェンス無しで呼ぶ。

**StagedDescriptorAllocator_Test.cpp:**
- テスト用の外部 GPU ヒープ (`CreateTestExternalHeap()`) を作成して `Init()` に渡す。
- `UINT64_MAX` パラメータを全て削除。

#### 確認

- ビルド成功 (Debug x64)
- リンク成功、実行ファイル生成

#### 実行時エラー

```
D3D12 ERROR: ID3D12CommandQueue1::ExecuteCommandLists:
Descriptor (at CPU Handle 0x...) is bound as STATIC (not-DESCRIPTORS_VOLATILE)
on Command List 0x.... It was most recently changed by CopyDescriptorsSimple call,
but it is invalid to change it until the command list has finished executing
for the last time.
[ EXECUTION ERROR #1001: STATIC_DESCRIPTOR_INVALID_DESCRIPTOR_CHANGE]
```

#### 原因

`Stage()` が `CopyDescriptorsSimple` でメイン GPU ヒープの予約領域を書き換える。
旧設計では別々の GPU ヒープだったため問題にならなかったが、
統合後は前フレームのコマンドリストが `m_heap` を STATIC バインドしたまま実行中に、
次フレームの `Stage()` が同じヒープの内容を変更していると D3D12 が検出する。

#### 対処: frame-buffered staged reservation (Review-7 指摘により採用)

`D3D12_DESCRIPTOR_HEAP_FLAG_DESCRIPTORS_VOLATILE` という heap flag は存在しない
（`DESCRIPTORS_VOLATILE` は root signature descriptor range のフラグである）。
そのため volatile 化ではなく、予約領域をフレーム数分に分割する frame-buffered 方式を採用した。

**方式:**

`kStagedDescriptorReservedCount` を per-frame capacity とし、
メインヒープに `kStagedDescriptorReservedCount * kFrameCount` の予約領域を確保する。

```
stagedBase = kMainHeapDescriptorCount
perFrameCapacity = kStagedDescriptorReservedCount

Frame 0: [stagedBase + 0 * perFrameCapacity, stagedBase + 1 * perFrameCapacity)
Frame 1: [stagedBase + 1 * perFrameCapacity, stagedBase + 2 * perFrameCapacity)
```

`StagedDescriptorAllocator` に `SetFrameIndex(frameIndex)` を追加。
`Stage()` と `GpuHandle()` は同じ `CurrentStageOffset()` (= `stageOffset + frameIndex * reservedCount`) を使用する。

`RenderFrame()` では次の順序で呼び出す:

```cpp
m_stageAllocator.SetFrameIndex(m_currentFrameIndex);
m_stageAllocator.Stage();
```

これにより前フレームが参照している descriptor slot を次フレームが上書きしなくなるため、
STATIC descriptor のままでも `STATIC_DESCRIPTOR_INVALID_DESCRIPTOR_CHANGE` が発生しない。

**`reservedCount` 超過の hard failure 化:**

Review-7 の指摘により、`Grow()` での `reservedCount` 超過を `assert` のみから
`ThrowIfFailed(E_OUTOFMEMORY)` に変更し、Release build でも止まるようにした。
