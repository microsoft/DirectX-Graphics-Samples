# feature/cornell-box-scene

## SceneFactory

`SceneFactory` is a static factory class that creates `SampleScene` instances. It decouples scene creation from `SampleApp` and provides a clean extension point for adding new procedural scenes.

### Files

| File | Purpose |
|------|---------|
| `Scene/SceneFactory.h` | Factory class declaration |
| `Scene/SceneFactory.cpp` | Factory implementation + internal scene classes |
| `Scene/ProceduralSceneBuilder.h` | Procedural geometry helpers |
| `Scene/ProceduralSceneBuilder.cpp` | Procedural geometry implementation |

### Adding a New Scene

1. Create a new `SampleScene` subclass in `SceneFactory.cpp` (internal linkage)
2. Add a static factory method to `SceneFactory`
3. Call the factory method in `SampleApp::CreateSampleScenes()`

### Architecture

```
SceneFactory
    |
    v
CornellBoxScene : SampleScene
    |
    v
ProceduralSceneBuilder
    |
    v
CreateQuadMesh / CreateSphereMesh / AppendMesh
```

## Cornell Box + Mirror Ball Scene

A classic Cornell Box scene for testing reflections, shadows, and material rendering.

### Geometry

3x3x3 unit box centered at origin. Camera at (0, 0, 5.0) looking at origin.

| Object | Position | Size/Radius | Material |
|--------|----------|-------------|----------|
| Left wall | (-1.5, 0, 0) | 3x3 | Red, rough=0.8 |
| Right wall | (1.5, 0, 0) | 3x3 | Green, rough=0.8 |
| Back wall | (0, 0, -1.5) | 3x3 | White, rough=0.8 |
| Floor | (0, -1.5, 0) | 3x3 | White, rough=0.8 |
| Ceiling | (0, 1.5, 0) | 3x3 | White, rough=0.8 |
| Mirror ball | (-0.4, -0.2, 0.5) | r=0.5 | Metallic=1.0, rough=0.02 |
| Diffuse sphere | (0.6, -0.9, 0.0) | r=0.3 | White, rough=0.9 |
| Emissive panel | (0, 1.48, 0) | 0.4x0.4 | White, emissive=1.0 |

### Materials

| ID | Name | albedo | metallic | roughness | emissive |
|----|------|--------|----------|-----------|----------|
| 0 | Red wall | red | 0.0 | 0.8 | 0.0 |
| 1 | Green wall | green | 0.0 | 0.8 | 0.0 |
| 2 | White diffuse | white | 0.0 | 0.8 | 0.0 |
| 3 | Mirror ball | white | 1.0 | 0.02 | 0.0 |
| 4 | Diffuse sphere | white | 0.0 | 0.9 | 0.0 |
| 5 | Emissive panel | white | 0.0 | 0.8 | 1.0 |

All textures are 1x1 solid colors.

### Winding Order

D3D12 default: `FrontCounterClockwise = FALSE` (CW = front face).

| Wall | Normal | flipWinding | Front face |
|------|--------|-------------|------------|
| Left | (1,0,0) | true | +X |
| Right | (-1,0,0) | false | -X |
| Back | (0,0,1) | true | +Z |
| Floor | (0,1,0) | false | +Y |
| Ceiling | (0,-1,0) | true | -Y |

All walls face inward (towards camera/scene interior).

### Per-Vertex MaterialID

The scene uses per-vertex `materialId` to assign different materials to different objects within a single combined mesh. The shader fallback logic:

```hlsl
result.materialId = vertexMaterialId == 0xffffffff ? inst.materialId : vertexMaterialId;
```

When `vertexMaterialId` is set (not 0xffffffff), it overrides the instance material ID. This allows multiple materials in one draw call.

### Emissive Panel

The emissive quad on the ceiling is a placeholder for future path tracing / GI testing. The current hybrid renderer does not treat emissive surfaces as light sources. Lighting is provided by the existing directional light.

### ProceduralSceneBuilder API

```cpp
// Primitive mesh generation
SceneMesh CreateQuadMesh(v0, v1, v2, v3, normal, flipWinding=false);
SceneMesh CreateCubeMesh(size);
SceneMesh CreateSphereMesh(radius, stackCount, sliceCount);

// Texture helper
int AddSolidColorTexture(mesh, r, g, b, a);

// Mesh combine (per-vertex materialId)
void AppendMesh(dest, src, materialId);
void AppendTransformedMesh(dest, src, transform, materialId);

// High-level scene primitives
void AddQuad(mesh, center, size, normal, materialId, flipWinding=false);
void AddSphere(mesh, position, radius, materialId);
```
