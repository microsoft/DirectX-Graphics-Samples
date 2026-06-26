# feature/shadow-test-scenes

## RayQuery Shadow メモ

このブランチでは RayQuery shadow の検証用シーンを追加する。対象は Ground + Cubes、Animated Shadow Grid、Contact Shadow Test、Occluder Wall Test。

## Light Direction

`lightDirection` は surface-to-light 方向として扱う。

ShadowMask 生成と LightPass の direct lighting は同じ向きに揃える。

- `shaders_RayQueryShadow.hlsl`: `rayDir = normalize(lightDirection)`
- `shaders_LightPass.hlsl`: `lightDir = normalize(lightDirection)`

片方だけ `-lightDirection` を使うと、ShadowMask 単体では正しく見えても、最終描画の光と影の向きがずれる。

## RayQuery Culling

現在の shadow ray では `RAY_FLAG_CULL_BACK_FACING_TRIANGLES` を使わない。

shadow ray は binary occlusion test なので、back face も遮蔽物として有効に扱う。back-face culling を有効にすると、Cube の面ごとに hit する面と抜ける面が分かれ、影が部分的に欠ける。

期待する形:

```hlsl
RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
```

## TLAS Instance Transform

描画 shader は `InstanceData::world` を直接使う。Scene 側ではこの行列を `XMMatrixTranspose(M)` として保存している。

`D3D12_RAYTRACING_INSTANCE_DESC::Transform` に詰めるときも、shader が見る行列規約と揃える。つまり `InstanceData::world` の先頭 3 行をそのまま使う。

壊れていた実装では、一度 untransposed matrix に戻すような詰め方をしていた。平行移動だけの scene では目立ちにくいが、回転する Cube では ShadowMask に「別の Cube が投影された」ような模様が出る。

## Bias Tuning

ShadowMask が構造的におかしい場合、最初に `kNormalBias` を調整しない。

bias は self-intersection acne と peter-panning の調整用。mask の向きがおかしい、別オブジェクトが投影されたように見える、という場合は先に以下を見る。

1. ShadowMask と LightPass の light direction が一致しているか。
2. RayQuery に back-face culling を入れていないか。
3. TLAS instance transform の詰め方が shader 側と一致しているか。
4. animated instance に対して TLAS rebuild が追従しているか。

現在の基準値:

```hlsl
static const float kNormalBias = 0.01;
ray.TMin = 0.001;
```

## Animated Pause

Pause は現在の accumulated animation time を止めるだけにする。回転項に pause 用 speed を掛けてはいけない。

問題のある形:

```cpp
const float speed = context.isPlaying ? 1.0f : 0.0f;
const float rotY = m_accumTime * rotSpeed * speed + phase;
```

正しい形:

```cpp
const float rotY = m_accumTime * rotSpeed + phase;
```

`m_accumTime` は Pause 中に進まない。さらに `speed = 0` を掛けると、現在姿勢ではなく `phase` だけの姿勢に戻ってしまう。

## 期待する結果

- ShadowMask の方向と最終 direct lighting が一致する。
- Cube の shadow face が RayQuery back-face culling によって欠けない。
- Animated cube の回転が TLAS / ShadowMask に反映される。
- Space で Pause しても Cube の姿勢が変化しない。
- すべての shadow validation scene が一貫して見える。
