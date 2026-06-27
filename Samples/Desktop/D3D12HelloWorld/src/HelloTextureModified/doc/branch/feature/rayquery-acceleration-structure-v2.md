# feature/rayquery-acceleration-structure-v2

## 目的

D3D12 RayQuery (DXR Inline Ray Tracing) を用いてシャドウマスクを生成する基盤を構築する。
GBuffer ベースの deferred rendering パイプラインに RayQuery パスを追加し、
ライト方向に対する shadow ray を各ピクセルからトレースする。

## 背景と問題

既存パイプラインは Forward rendering / deferred lighting に対応済みだが、
シャドウは未実装。DXR RayQuery は Compute Shader から直接レイトレーシングを利用でき、
専用の Raytracing Pipeline (RTPipeline) よりオーバーヘッドが少ない。

## 実施内容

### Commit 3bd805a5: Implement RayQuery shadow pass wiring and fix TLAS transform

**新規ファイル:**
- `Renderer/AccelerationStructureResources.h` / `.cpp`: BLAS/TLAS の Build 管理
  - `CreateTriangleGeometryDesc()`: 頂点バッファから DXR 用三角形ジオメトリを設定
  - `FillTlasInstanceDescs()`: InstanceData の world 行列を TLAS の 3x4 row-major 変換行列に展開
  - `Build()`: BLAS → UAV barrier → TLAS の一連の Build を実行
  - `CreateTlasSrv()`: TLAS 用 Shader Resource View を作成
- `Renderer/RayQueryShadowPass.h`: `RayQueryShadowPassDesc` — `tlasSrv`, `depthSrv`, `normalSrv`, `cameraCbv`, `lightDirection`
- `Renderer/RayQueryShadowPass.cpp`: `RecordRayQueryShadowPass()` — 6 つの root parameter をバインド後 Dispatch

**既存ファイル変更:**
- `shaders_RayQueryShadow.hlsl`:
  - ShadowMask 出力 UAV (`u0`), TLAS SRV (`t0`), Depth SRV (`t1`), Normal SRV (`t2`) を宣言
  - CameraCB (`b0`): `viewProj` / `prevViewProj` / `invViewProj` / `cameraPosition`
  - ShadowConstants (`b1`): `lightDirection`
  - `ReconstructWorldPosition()`: depth buffer からワールド座標を復元
  - `CSMain`: 各ピクセルからカメラレイをトレース (TLAS 検証用)
    - 背景ピクセルも含め全ピクセルでレイトレースを実行
    - Hit → `saturate(t / 50.0)` 濃淡表示。Miss → 0.0 (黒)
- `D3D12HelloTexture.cpp` / `.h`:
  - `PrepareSceneInstanceData()`: シーンインスタンスデータの事前準備
  - `CreateInstanceBuffers()`: インスタンスバッファ (StructuredBuffer) の作成
  - `BuildAccelerationStructures()`: `AccelerationStructureResources::Build()` を初期化時に呼び出し
  - `kMaxInstanceCount = 1000` 定義
  - `CreateRayQueryShadowRootSignature()`: 6 つの root parameter を設定 (ShadowMask UAV, TLAS SRV, Depth SRV, Normal SRV, CameraCB, lightDirection)
  - `ExecuteRayQueryShadowPass()`: passDesc に各ハンドルを設定して Record を呼び出し
- `D3D12HelloTexturePasses.cpp`:
  - `MakeRayQueryShadowPass()`: Depth と GBuffer.Normal を `NON_PIXEL_SHADER_RESOURCE` read として登録
- `D3D12HelloTextureModified.vcxproj` / `.filters`: 新規ファイル追加
- `Renderer/AccelerationStructureResources.cpp`:
  - `FillTlasInstanceDescs()` の Transform 展開バグ修正:
    - `InstanceData::world` は `XMMatrixTranspose(M)` で格納されている
    - 平行移動成分は `src[12..14]` ではなく `src[3][7][11]` (M._41/_42/_43 = tx/ty/tz) が正しい
    - 参照 (`AccelerationStructureResources.cpp:126-138`)

### Commit 0d4d832a: Fix rebase regression — restore missing declarations

Rebase dropped header declarations from the original commit. Restored in `D3D12HelloTexture.h`:
- `#include "AccelerationStructureResources.h"`
- `AccelerationStructureSrv` in `Desc::Srv`
- `kTlasDescriptorCount` and updated `kMainHeapDescriptorCount`
- `m_accelerationStructures` member variable
- `BuildAccelerationStructures()` method declaration

### Commit 446a0714: Fix rebase regression — restore vcxproj and BuildAccelerationStructures()

Rebase dropped the `.cpp` implementation and project entries. Restored:
- `D3D12HelloTexture.cpp`: `BuildAccelerationStructures()` implementation, `CreateInstanceBuffers()` call in `LoadAssets()`
- `D3D12HelloTextureModified.vcxproj` / `.filters`: `<ClCompile>` / `<ClInclude>` entries for `AccelerationStructureResources.cpp/.h` and `RayQueryShadowPass.cpp/.h`

### Commit e5b54e8b: Split RayQuery TLAS debug from shadow mask

Combined shader `shaders_RayQueryShadow.hlsl` was split into two dedicated compute shaders:

**新規ファイル:**
- `Renderer/RayQueryTlasDebugPass.h`: `RayQueryTlasDebugPassDesc` — mirror of `RayQueryShadowPassDesc` but writes to `TlasDebugUAV`
- `Renderer/RayQueryTlasDebugPass.cpp`: `RecordRayQueryTlasDebugPass()` — camera-ray TLAS/BLAS visibility debug
- `shaders_RayQueryTlasDebug.hlsl`:
  - カメラレイを全ピクセルからトレース (TLAS 可視性検証)
  - Hit → `saturate(t / 50.0)` 濃淡表示。Miss → 0.0 (黒)
  - `CameraRayDirection()`: invViewProj を用いてカメラレイ方向を計算

**既存ファイル変更:**
- `shaders_RayQueryShadow.hlsl`: camera-ray TLAS 検証コードを削除し、ShadowMask 生成のみに (lightDirection ベースの shadow ray)
- `D3D12HelloTexture.cpp` / `.h`:
  - `CreateRayQueryTlasDebugRootSignature()` 追加
  - `ExecuteRayQueryTlasDebugPass()` 追加
  - `m_rayQueryTlasDebugRootSignature`, `m_rayQueryTlasDebugPipeline` 追加
  - Pipeline / Operation key `RayQueryTlasDebug` 追加
- `D3D12HelloTexturePasses.cpp`:
  - `MakeRayQueryTlasDebugPass()` 追加
  - `AddSceneRenderPasses()`: TLAS Debug Mode 時のみ RayQueryTlasDebugPass を追加
- Pipeline states: `rayQueryTlasDebug` PSO 作成

### Commit 4abf0268: Implement RayQuery shadow mask shader

`shaders_RayQueryShadow.hlsl` を本物の directional-light シャドウマスク生成に書き換え:

- `CSMain`: 各ピクセルから `lightDirection` 方向に shadow ray をトレース
  - 深度 > 1.0 の背景ピクセルは 1.0 (完全照明)
  - `ReconstructWorldPosition()` でワールド座標を復元後、法線方向に `kNormalBias = 0.01` オフセット
  - `TraceRayInline()` で TLAS に shadow ray を発射
  - Hit → 0.0 (影)、Miss → 1.0 (照明)
- `g_shadowMask` UAV に 0.0 or 1.0 を書き込み
- CBV リソースは既存の CameraCB / ShadowConstants を流用

### Commit 26eec75b: Apply RayQuery shadow mask to direct lighting

- `shaders_LightPass.hlsl`:
  - `g_shadowMask` SRV (`t0, space4`) を追加
  - `rayTracingSupported` フラグで条件付きシャドウマスクサンプリング
  - `shadowMask` を直接光の `ndotl` 項に乗算
- `D3D12HelloTexturePasses.cpp`:
  - `MakeLightingPass()`: `rayTracingSupported` 時は ShadowMask を `PIXEL_SHADER_RESOURCE` read として登録
  - `RootSignatureLayout::ToneMapSceneColor` スロットに ShadowMask SRV をバインド
- `D3D12HelloTexture.cpp`:
  - `LightingConstants` に `rayTracingSupported` フラグ追加

### Commit b5eb2098: Move TLAS instance buffer to frame resources

- `D3D12HelloTexture.h` / `.cpp`:
  - `FrameResource` に `tlasInstanceBuffer` 追加 (フレーム毎の TLAS インスタンスバッファ)
  - TLAS インスタンスバッファをフレームリソース化することで、更新中の読み取り競合を防止
  - `BuildAccelerationStructures()`: 初期インスタンスバッファを frame 0 から取得
- `AccelerationStructureResources::Build()`: 外部から `tlasInstanceBuffer` を受け取る形に変更

### Commit f9fa2fb1: Rebuild TLAS for current frame instances

- `AccelerationStructureResources::RebuildTlas()` 追加:
  - 既存の TLAS / TLAS Scratch を再利用 (再確保不要)
  - フレーム毎にインスタンスデータを更新し、TLAS のみ再 Build
  - BLAS は安定 (不変) のため再 Build 不要
  - TLAS SRV も再作成不要 (tlas リソースは同一)
- `D3D12HelloTexture.cpp`:
  - `RebuildAccelerationStructures()`: 毎フレーム呼び出し
  - フレームリソースの `tlasInstanceBuffer` に現在のインスタンスデータを書き込み後、`RebuildTlas()` を実行
  - TLAS 更新により、インスタンス移動 (アニメーション) にシャドウが追従可能に

## 現在の完成状態

- **RayQuery Shadow Mask**: directional-light シャドウマスク生成 (正常動作)
- **Shadow Mask Debug View**: Shadow Mask テクスチャを表示するデバッグビュー (`RenderViewMode::ShadowMask`)
- **RayQuery TLAS Debug**: カメラレイによる TLAS/BLAS 可視性デバッグパス (`RenderViewMode::TlasDebug`)
- **TLAS per-frame rebuild**: 毎フレームのインスタンスデータから TLAS を再 Build (BLAS は安定)

## 既知の制限 / 将来の対応

- **Bias 調整**: シャドウマスクの `kNormalBias = 0.01` は固定値。シーンやジオメトリのスケールによって調整が必要な可能性がある。
- **シャドウ検証シーン未実装**: 専用のシャドウ検証用シーン (Ground + Cubes / Animated Shadow Test など) は未実装。デフォルトの Helmet シーンのみで確認済み。

## 今後の展望 (deferred)

- **Dedicated shadow test scene**: Ground + Cubes または Animated Shadow Test のシーンを追加し、影の品質とパフォーマンスを検証する。
- **PCF 等のソフトシャドウ**: 現状はハードシャドウ (binary mask)。複数レイやフィルタリングによる軟らかい影への拡張が可能。
- **Spot / Point Light 対応**: 現在は単一 directional light のみ。他の光源タイプへの拡張。
