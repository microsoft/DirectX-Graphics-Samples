# Review: feature/growable-descriptor-heap @ cc4f63fed3b3

対象コミット:
- `cc4f63fed3b3124c6a56f86b992163e6c04b23b2`
- `Step 2: add StagedDescriptorAllocator prototype with smoke test`

## 結論

方向性は正しいです。

CPU 側の non-shader-visible heap を「正」として持ち、shader-visible heap へ stage する 2-heap staging allocator の方針は、将来 `kMainHeapDescriptorCount` の手動加算を減らす方向として妥当です。

ただし、現状はまだ merge 用の完成実装ではなく、設計検証用の prototype と見るのがよいでしょう。特に grow 後の handle の扱いと、古い shader-visible heap の GPU lifetime 管理は mainline に入れる前に直したいです。

## 良い点

- `StagedDescriptorAllocator` として責任がまとまっており、既存の `SimpleDescriptorHeapAllocator` から次の段階へ進む意図が見えます。
- CPU heap と GPU heap を分ける構成は、descriptor の永続スロット管理と resize を両立しやすいです。
- `PersistentSrvSlotCount` の導入により、固定 descriptor 数の意味が少し明確になっています。
- smoke test があり、allocate / free / grow / stage の最小動作を確認する入口があります。
- Debug x64 build は通っています。

## Findings

### 1. `StagedDescriptorHandle` が絶対 CPU/GPU handle を持っている

重要度: 高

`StagedDescriptorHandle` が `Index` に加えて `Cpu` / `Gpu` descriptor handle を持っています。

しかし `Grow()` で CPU heap / GPU heap が作り直されるため、grow 前に返された `Cpu` / `Gpu` は古い heap を指す stale handle になります。これは growable allocator として一番危ない部分です。

長期的には handle は slot/index だけを持ち、CPU/GPU descriptor handle は allocator から都度解決する形がよいです。

例:
```cpp
struct StagedDescriptorHandle
{
    UINT Index = UINT_MAX;
    bool IsValid() const { return Index != UINT_MAX; }
};

D3D12_CPU_DESCRIPTOR_HANDLE Cpu(StagedDescriptorHandle handle) const;
D3D12_GPU_DESCRIPTOR_HANDLE Gpu(StagedDescriptorHandle handle) const;
```

この形なら grow 後も同じ slot index から新しい heap 上の handle を取得できます。

### 2. grow 後の古い shader-visible heap を即時破棄できない

重要度: 高

`Grow()` で shader-visible heap を差し替える場合、古い GPU heap は直前まで command list / GPU が参照している可能性があります。

そのため、古い heap を即時 release すると GPU 側 lifetime の問題になります。既存の環境マップ周辺で使っている deferred release と同様に、fence 完了後に破棄する仕組みが必要です。

この allocator を main rendering path に入れるなら、古い shader-visible heap は `DeferredGpuReleaseQueue` のような仕組みに渡す設計にしたいです。

### 3. smoke test が app 初期化時に実行されている

重要度: 中

`RunStagedAllocatorTests()` が `InitializeFrameResources()` から呼ばれています。

prototype としては便利ですが、mainline に入れるなら app 起動 path からは外したいです。残す場合でも debug-only か、明示的な developer test path に寄せるほうがよいです。

### 4. テストが stale handle / GPU lifetime を検出できない

重要度: 中

現在の smoke test は allocate / free / grow / stage の表面的な確認としては有効ですが、この allocator の本命リスクである stale handle と GPU lifetime は検出できません。

追加したいテスト:
- grow 前に取得した slot index が grow 後も同じ意味で解決できること。
- grow 後に取得した `Gpu(handle)` が新しい GPU heap 上の handle になること。
- double free を検出すること。
- free 済み handle の再利用時の挙動が明確であること。

### 5. contiguous descriptor table の扱いが未整理

重要度: 中

ShadowMask などでは contiguous SRV/UAV table が必要になります。

単体 slot allocation だけでは、今後 `AllocContiguous(count)` 相当が必要になりそうです。growable allocator に統合するなら、単発 descriptor と contiguous range の両方をどう扱うかを早めに決めたいです。

## merge 前の推奨修正

1. `StagedDescriptorHandle` を slot-only にする。
2. `Cpu(handle)` / `Gpu(handle)` accessor を追加する。
3. grow 時に古い shader-visible heap を deferred release できる形にする。
4. `RunStagedAllocatorTests()` を通常 app 起動 path から外す。
5. stale handle を防ぐテストを追加する。

## 次の段階

この branch は `kMainHeapDescriptorCount` 手動管理の問題を解く方向として価値があります。

次に進めるなら、まず allocator API を「slot を返し、handle は都度解決」に直すのが最優先です。その後、ShadowMask / EnvironmentMap のような descriptor 再確保が多い箇所に小さく適用して、既存の `SimpleDescriptorHeapAllocator` と置き換える範囲を決めるのがよいです。
