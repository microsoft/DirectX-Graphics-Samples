# feature/shadow-test-scenes

## RayQuery Shadow Notes

This branch adds validation scenes for RayQuery shadows: Ground + Cubes, Animated Shadow Grid, Contact Shadow Test, and Occluder Wall Test.

## Light Direction

`lightDirection` is treated as the surface-to-light direction.

Keep ShadowMask generation and LightPass direct lighting consistent:

- `shaders_RayQueryShadow.hlsl`: `rayDir = normalize(lightDirection)`
- `shaders_LightPass.hlsl`: `lightDir = normalize(lightDirection)`

If one side uses `-lightDirection`, the ShadowMask can look correct by itself while the final lighting looks wrong.

## RayQuery Culling

Do not use `RAY_FLAG_CULL_BACK_FACING_TRIANGLES` for the current shadow ray.

A shadow ray is a binary occlusion test. Back faces are still valid blockers. Enabling back-face culling made cube shadows partially correct and partially missing because different faces were accepted or rejected by winding.

Expected form:

```hlsl
RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
```

## TLAS Instance Transform

Rendering uses `InstanceData::world` directly in shaders. Scene code stores it as `XMMatrixTranspose(M)`.

When filling `D3D12_RAYTRACING_INSTANCE_DESC::Transform`, use the first three rows of `InstanceData::world` directly so TLAS sees the same object-to-world transform convention as the shaders.

The broken version tried to reconstruct the untransposed matrix before filling the DXR 3x4 transform. Translation-only scenes could look acceptable, but animated rotating cubes produced ShadowMask patterns that looked like another cube was projected onto the surface.

## Bias Tuning

Do not tune `kNormalBias` first when the mask is structurally wrong.

Bias is for self-intersection acne and peter-panning. If the mask looks like the wrong orientation or another object projection, check light direction, RayQuery culling flags, TLAS transform packing, and TLAS rebuild timing first.

Current baseline:

```hlsl
static const float kNormalBias = 0.01;
ray.TMin = 0.001;
```

## Animated Pause

Pausing should freeze the current accumulated animation time. Do not multiply the rotation term by a pause speed value.

Broken pattern:

```cpp
const float speed = context.isPlaying ? 1.0f : 0.0f;
const float rotY = m_accumTime * rotSpeed * speed + phase;
```

Correct pattern:

```cpp
const float rotY = m_accumTime * rotSpeed + phase;
```

`m_accumTime` already stops advancing while paused. Multiplying by zero changes the current orientation back to `phase`.

## Expected Result

- ShadowMask direction matches final direct lighting.
- Cubes do not lose shadow faces due to RayQuery back-face culling.
- Animated cube rotation is reflected in the TLAS / ShadowMask.
- Pressing Space to pause does not change cube orientation.
- All shadow validation scenes look consistent.
