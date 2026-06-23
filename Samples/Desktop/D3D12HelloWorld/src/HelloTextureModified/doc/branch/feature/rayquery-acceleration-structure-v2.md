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

### Commit 269e86d4: Add ray query acceleration structure foundation

**新規ファイル:**
- `Renderer/AccelerationStructureResources.h` / `.cpp`: BLAS/TLAS の Build 管理
  - `CreateTriangleGeometryDesc()`: 頂点バッファから DXR 用三角形ジオメトリを設定
  - `FillTlasInstanceDescs()`: InstanceData の world 行列を TLAS の 3x4 row-major 変換行列に展開
  - `Build()`: BLAS → UAV barrier → TLAS の一連の Build を実行
  - `CreateTlasSrv()`: TLAS 用 Shader Resource View を作成

**既存ファイル変更:**
- `D3D12HelloTexture.cpp` / `.h`:
  - `PrepareSceneInstanceData()`: シーンインスタンスデータの事前準備
  - `CreateInstanceBuffers()`: インスタンスバッファ (StructuredBuffer) の作成
  - `BuildAccelerationStructures()`: `AccelerationStructureResources::Build()` を初期化時に呼び出し
  - `kMaxInstanceCount = 1000` 定義
- `D3D12HelloTextureModified.vcxproj` / `.filters`: 新規ファイル追加

### Uncommitted: RayQuery shadow pass 配線

**`shaders_RayQueryShadow.hlsl`:**
- ShadowMask 出力 UAV (`u0`), TLAS SRV (`t0`), Depth SRV (`t1`), Normal SRV (`t2`) を宣言
- CameraCB (`b0`): `viewProj` / `prevViewProj` / `invViewProj` / `cameraPosition`
- ShadowConstants (`b1`): `lightDirection`
- `ReconstructWorldPosition()`: depth buffer からワールド座標を復元
- `CSMain`: 各ピクセルからカメラレイをトレース (TLAS 検証用)
  - 背景ピクセルも含め全ピクセルでレイトレースを実行
  - Hit → `saturate(t / 50.0)` 濃淡表示。Miss → 0.0 (黒)

**`D3D12HelloTexture.cpp`:**
- `CreateRayQueryShadowRootSignature()`: 6 つの root parameter を設定
  - param 0: ShadowMask UAV (u0)
  - param 1: TLAS SRV (t0)
  - param 2: Depth SRV (t1)
  - param 3: Normal SRV (t2)
  - param 4: CameraCB (b0)
  - param 5: ShadowConstants lightDirection (b1, 3 floats)
- `ExecuteRayQueryShadowPass()`: passDesc に TLAS/Depth/Normal/CameraCB/Light のハンドルを設定して Record を呼び出し

**`D3D12HelloTexturePasses.cpp`:**
- `MakeRayQueryShadowPass()`: Depth と GBuffer.Normal を `NON_PIXEL_SHADER_RESOURCE` read として登録

**`Renderer/RayQueryShadowPass.h`:**
- `RayQueryShadowPassDesc` に以下を追加:
  - `tlasSrv`, `depthSrv`, `normalSrv`, `cameraCbv` (GPU descriptor handles)
  - `lightDirection` (XMFLOAT3)

**`Renderer/RayQueryShadowPass.cpp`:**
- `RecordRayQueryShadowPass()`: 6 つの root parameter をバインド後 Dispatch

**`Renderer/AccelerationStructureResources.cpp`:**
- `FillTlasInstanceDescs()` の Transform 展開バグ修正:
  - `InstanceData::world` は `XMMatrixTranspose(M)` で格納されている
  - 平行移動成分は `src[12..14]` (M._14/_24/_34 = 0) ではなく `src[3][7][11]` (M._41/_42/_43 = tx/ty/tz) が正しい
  - 参照 (`AccelerationStructureResources.cpp:126-138`)

### TLAS 更新問題 (未対応)

現在の TLAS は初期化時に一度だけ Build され、インスタンスが移動しても更新されない。
ShadowMask のデバッグ完了後、`UpdateTLAS()` 追加が必要。

## 既知の問題 / 未解決

- [ ] TLAS が毎フレーム更新されない (インスタンス移動に追従しない)
- [ ] シャドウマスクは現在カメラレイの可視性テスト出力中 (shadow ray 未実装)
