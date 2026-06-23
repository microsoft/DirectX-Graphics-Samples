# Review: feature/growable-descriptor-heap @ 9b314b2414a8

対象コミット:

- `9b314b2414a8ee331628426539f734df76c70f22`
- `Step 3 fix: frame-buffered staged reservation で STATIC 制約違反を修正`

## 結論

次の判断は **B. 現状このままで `dx12-playground` へマージ** を推奨します。

この branch は、growable descriptor heap へ向かうための最初の実用 trial として十分にまとまっています。
ここからさらに staged descriptor への引っ越しを続けるより、一度 `dx12-playground` に統合して、
次の移行作業は別 branch で段階的に進めるのがよいです。

## 現在の到達点

今回の branch では、以下が確認済みです。

- `StagedDescriptorAllocator` の prototype を導入した。
- allocator 単体の smoke test を Debug UI から手動実行できるようにした。
- ShadowMask SRV/UAV を小規模 trial として staged allocator に移行した。
- Review-6 で問題になった同一 `CBV/SRV/UAV` shader-visible heap の2本同時 bind を解消した。
- Step 3 で staged descriptors を main shader-visible heap の予約領域へ統合した。
- Step 3 fix で frame-buffered staged reservation を導入し、`STATIC_DESCRIPTOR_INVALID_DESCRIPTOR_CHANGE` を解消した。
- 起動・動作確認済み。Error log は出ていない。

## B を推す理由

### 1. 小さく実証済みの単位になっている

ShadowMask SRV/UAV は descriptor 数が少なく、失敗時の影響範囲も比較的限定できます。
この小さな対象で staged allocator の設計、main heap 統合、frame-buffered reservation まで通せたので、
branch としては「実用 trial 成功」のよい切れ目です。

### 2. これ以上の移行は blast radius が広がる

ここからさらに staged descriptor への引っ越しを進めると、対象は次のように広がります。

- GBuffer / depth / light pass SRV
- texture table
- material / instance / constant buffer descriptors
- environment map descriptor table
- transient resources と RenderGraph lifetime

これらは寿命、参照箇所、root signature range flag、resize/recreate path が絡みます。
今の branch に追加で載せると、review と検証の焦点がぼやけやすいです。

### 3. `dx12-playground` に入れる価値がすでにある

現状でも以下の価値があります。

- `kMainHeapDescriptorCount` 固定管理からの脱却に向けた実装基盤が入る。
- staged allocator の slot-only handle / contiguous range / CPU authoritative heap の形が残る。
- main heap 予約領域方式と frame-buffered descriptor copy の基準実装が得られる。
- 今後の descriptor 移行時に ShadowMask を参照実装として使える。

この価値は、今マージしても十分に意味があります。

## A を進める場合のリスク

**A. Staged descriptor への引っ越しを進める** こと自体は次の方向として正しいです。
ただし、この branch で続けるより、別 branch に切り出す方が安全です。

主なリスク:

- root signature の `DATA_STATIC` / `DATA_VOLATILE` / `DESCRIPTORS_VOLATILE` の整理が必要になる。
- frame-buffered staged reservation の per-frame capacity を再設計する必要が出る。
- descriptor 更新タイミングと GPU in-flight lifetime の検証範囲が広がる。
- resize / resource recreate / transient resource release path の確認が必要になる。
- `SimpleDescriptorHeapAllocator` と `StagedDescriptorAllocator` の責務境界を再調整する可能性がある。

## 推奨する次の進め方

1. この branch は `dx12-playground` へマージする。
2. マージ後に新しい branch を切り、staged descriptor 移行の第2段を進める。
3. 次の移行対象は texture/material 全体ではなく、RenderGraph に近い小さな descriptor table から選ぶ。
4. 候補は GBuffer / depth / LightPass SRV など、ShadowMask と同じく範囲が見えやすいものにする。
5. 各移行ごとに、起動確認、対象 debug view、D3D12 debug layer error なしを確認する。

## 判定

- 現 branch の追加実装: ここで止める。
- `dx12-playground` へのマージ: 推奨。
- staged descriptor 移行の継続: 次 branch で段階的に実施。
- 現時点の総評: 小さく始めて、実描画で通し、D3D12 の制約も踏まえて修正済み。マージ可能な状態。
