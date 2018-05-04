# D3D12 Raytracing Procedural Geometry sample
![D3D12 Raytracing Procedural Geometry GUI](Screenshot.png)

This sample demonstrates how to implement procedural geometry using intersection shaders. It utilizes multiple intersections shaders to create analytic and volumetric, signed distance and fractal geometry. In addition, it introduces:
* Extended shader table layouts and indexing covering multiple geometries and bottom-level acceleration structures (bottom-level AS, or BLAS for short).
* Use of trace ray recursion and two different ray types: radiance and shadow rays.

The sample assumes familiarity with Dx12 programming and DirectX raytracing concepts introduced in the [D3D12 Raytracing Simple Lighting sample](../D3D12RaytracingProceduralGeometry/readme.md).

### Scene
The scene consists of triangle and procedural/AABB geometry. Since the mixed geometry types are not currently supported within a single bottom-level AS, both triangle and procedural geometry are stored in seprate bottom-level AS. Specificaly, the geometry in the scene is:
* Triangle geometry - consisting of a quad for ground plane.
* Procedural geometry - remaining objects above the ground plane.

#### Procedural geometry
Procedural geometry is defined by an axis-aligned bounding box (AABB) and an intersection shader. The AABB defines geometric confines within a bottom-level AS object space. Once ray hits the AABB, intersection shader for the AABB is called to evaluate if the ray intersects any geometry within it and calls ReportHit() if it does. 

This sample implements three intersection shaders, as well as multiple different primitives per each intersection shader:
  * Analytic - a geometry with multiple spheres and an axis aligned box.
  * Volumetric - a metaballs isosurface (aka "blobs").
  * Signed distance - six different primitives and a fractal pyramid.

The ray/procedural primitive tests used in the intersection shaders assume primitive local space with AABB implicitly being <-1,1>. The sample applies per primitive scale transform to scale up the AABBs. A ray can only be retrieved either in world space (via *WorldRay\*()* intrinsic) or bottom-level AS object space (via *ObjectRay\*()* intrinsic). Therefore the application passes transforms for converting between bottom-level AS object and primitive local space and vice versa to the intersection shader. The shader then transforms an object space ray into local space and calls one of the ray/primitive intersection tests such as *RayAnalyticGeometryIntersectionTest* in *MyIntersectionShader_AnalyticPrimitive* intersection shader: 
```c
// Get ray in AABB's local space.
Ray GetRayInAABBPrimitiveLocalSpace()
{
    PrimitiveInstancePerFrameBuffer attr = g_AABBPrimitiveAttributes[l_aabbCB.instanceIndex];
    
    // Retrieve a ray origin position and direction in bottom level AS space 
    // and transform them into the AABB primitive's local space.
    Ray ray;
    ray.origin = mul(float4(ObjectRayOrigin(), 1), attr.bottomLevelASToLocalSpace).xyz;
    ray.direction = mul(ObjectRayDirection(), (float3x3) attr.bottomLevelASToLocalSpace);
    return ray;
}

[shader("intersection")]
void MyIntersectionShader_AnalyticPrimitive()
{
    Ray localRay = GetRayInAABBPrimitiveLocalSpace();
    AnalyticPrimitive::Enum primitiveType = (AnalyticPrimitive::Enum) l_aabbCB.primitiveType;

    float thit;
    ProceduralPrimitiveAttributes attr;
    if (RayAnalyticGeometryIntersectionTest(localRay, primitiveType, thit, attr))
    ...
```


If the ray intersects the geometry, the test returns true with the time of the hit as well as a corresponding surface normal in the local space. The shader then needs to transform the normal to world space and pass it to *ReportHit()*, which either accepts or rejects the hit.
```c
    ...
    if (RayAnalyticGeometryIntersectionTest(localRay, primitiveType, thit, attr))
    {
        PrimitiveInstancePerFrameBuffer aabbAttribute = g_AABBPrimitiveAttributes[l_aabbCB.instanceIndex];
        attr.normal = mul(attr.normal, (float3x3) aabbAttribute.localSpaceToBottomLevelAS);
        attr.normal = normalize(mul((float3x3) ObjectToWorld(), attr.normal));

        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}
```
To increase efficiency, ray intersection tests in the sample validate each ray hit against the valid global time range as well as the *RayFlags()*, instead of waiting on *ReportHit()* to confirm a hit and redo all the setup math. This also let the sample not require any hit shaders to evaluate the hits after *ReportHit()* is called
```c
// Test if a hit is valid based on specified RayFlags and <RayTMin, RayTCurrent> range.
bool IsAValidHit(in Ray ray, in float thit, in float3 hitSurfaceNormal)
{
    float rayDirectionNormalDot = dot(ray.direction, hitSurfaceNormal);
    return
        // Is thit within <tmin, tmax> range.
        IsInRange(thit, RayTMin(), RayTCurrent())
        &&
        // Is the hit not culled.
        (!(RayFlags() & (RAY_FLAG_CULL_BACK_FACING_TRIANGLES | RAY_FLAG_CULL_FRONT_FACING_TRIANGLES))
            || ((RayFlags() & RAY_FLAG_CULL_BACK_FACING_TRIANGLES) && (rayDirectionNormalDot < 0))
            || ((RayFlags() & RAY_FLAG_CULL_FRONT_FACING_TRIANGLES) && (rayDirectionNormalDot > 0)));
}
```
##### Procedural geometry types
This sample demonstrates intersection tests for ray vs following procedural geometry:

**Analytic geometry** including multiple spheres and an AABB.

**Volumetric geometry** that implements metaballs (aka "blobs"). Metaballs are an isosurface within a potential field that is formed from point sources. Each source has an area of influence and the field potential function is a quintic polynomial that smoothly decreases with distance from the source's center. If there are multiple field sources, their contributing potential values are summed. The isosurface corresponds to a pre-defined threshold value. The intersection test ray marches through the field within the AABB, calculating the total field potential at each iteration, until it hits the threshold or it exits the AABB. See more detailed explanation of the algorithm at [https://www.scratchapixel.com/lessons/advanced-rendering/rendering-distance-fields/blobbies](https://www.scratchapixel.com/lessons/advanced-rendering/rendering-distance-fields/blobbies)

**Signed distance geometry** is geometry based of signed distance functions. Each function returns a closest distance to the geometry considering all directions from a specific position. Since the distance is not necessarily along the ray direction, the intersection test needs to iteratively ray march and calculate signed distances at each step until it gets close enough to the surface. This algorithm is called sphere tracing and described in more detail at [https://www.scratchapixel.com/lessons/advanced-rendering/rendering-distance-fields/basic-sphere-tracer](https://www.scratchapixel.com/lessons/advanced-rendering/rendering-distance-fields/basic-sphere-tracer). A nice property of signed distance functions is that they support different logical operators and transformations allowing to combine simpler primitives into more complex geometry. This is explained in more detail at [http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm](http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm).

##### Updates
 Procedural geometry can be animated or modified without requiring acceleration structure updates as long as the AABBs don't change. The sample animates some of the geometry in the scene this way. It simply updates the transforms passed into shaders with updated rotation transforms every frame. In the metaballs case it also passes application time to animate sphere position updates within the metaballs' AABB.

#### Ray types
The sample utilizes two ray types for raytracing: a radiance and a shadow ray. 

Radiance ray is used for primary/view and secondary/reflected ray calls of *TraceRay()*, where a ray calculates shading for each geometry hit and aggregates color contribution over multiple bounces from the scene. 

Shadow ray is used for visibility/occlusion testing towards a light source and is simpler since all it does is to return a boolean value if it hit any or missed all objects. In practice, shadow ray only requires a miss shader and doesn't require any hit and closest hit shaders defined. It can simply be initialized with ray payload marking a hit and and *RayFlags()* to skip all but a miss shader. Then if the ray doesn't hit any geometry, the miss shader updates the hit boolean value to false:
```c
// Initialize shadow ray payload.
// Set the initial value to true since closest and any hit shaders are skipped. 
// Shadow miss shader, if called, will set it to false.
ShadowRayPayload shadowPayload = { true };
TraceRay(g_scene,
    /* RayFlags */
    RAY_FLAG_CULL_BACK_FACING_TRIANGLES
    | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH
    | RAY_FLAG_FORCE_OPAQUE             // ~skip any hit shaders
    | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, // ~skip closest hit shaders
    ...
```

#### Shader tables & indexing
When a ray hits geometry or needs to call a miss shader, GPU indexes into the application provided shader tables according to the following shader table addressing calculation:
```
Miss shader table index = 
     MissShaderIndex                                     ~ from shader: TraceRay()

Hit group shader table index = 
     RayContributionToHitGroupIndex                      ~ from shader: TraceRay()  
     + MultiplierForGeometryContributionToHitGroupIndex  ~ from shader: TraceRay()
     * GeometryContributionToHitGroupIndex               ~ system generated index 
                                                           of geometry in BLAS
     + InstanceContributionToHitGroupIndex               ~ from BLAS instance desc
```

 
Since each ray type applies different actions in the shaders, each requires a separate shader record in the shader table. The same applies for separate geometries as well. This sample uses triangle geometry type and three procedural geometry types with differing intersection shaders, each requiring a seprate shader record. Furthermore, there are multiple geometry variations per procedural type, each parametrized via attributes that are provided through local root signatures, which in turn again require a separate shader record for each. 

With that said, the sample has following shader table layouts: 

* **Miss shader table layout** - with two shader records.
```
Shader table - MissShaderTable
| [0]: MyMissShader                                   ~ Radiance ray
| [1]: MyMissShader_ShadowRay                         ~ Shadow ray
```
* **Hit group shader table layout** - with multiple geometries and two shader records for each. The full shader table is printed out in the debug output by this sample 
and is useful as a reference when setting up the indexing parameters. Here are first few shader records of a hit group shader table as a reference for an indexing example below:
```
Shader table - HitGroupShaderTable: 
| [0] : MyHitGroup_Triangle                           ~ Triangle geometry in 1st BLAS
| [1] : MyHitGroup_Triangle_ShadowRay
| [2] : MyHitGroup_AABB_AnalyticPrimitive             ~ 1st AABB geometry in 2nd BLAS
| [3] : MyHitGroup_AABB_AnalyticPrimitive_ShadowRay
| [4] : MyHitGroup_AABB_AnalyticPrimitive             ~ 2nd AABB geometry in 2nd BLAS
| [5] : MyHitGroup_AABB_AnalyticPrimitive_ShadowRay
  ...
```

Given the shader table layouts, the shader table indexing is set as follows:
* **MissShaderIndex** is set to 0 for radiance rays, and 1 for shadow rays in TraceRay(). Each geometry requires both so they are stored right after each other.
* **MultiplierForGeometryContributionToHitGroupIndex** is 2, since there are two hit groups (radiance, shadow ray) per geometry ID.
* **GeometryContributionToHitGroupIndex** is a geometry ID that is system 
  generated for each geometry within a BLAS. This directly maps to GeometryDesc array order passed in by the app, i.e. {0, 1, 2,...}.
* **RayContributionToHitGroupIndex** is set to 0 and 1, for radiance and shadow 
  rays respectively, since they're stored subsequently in the shader table.
* **InstanceContributionToHitGroupIndex** is an offset in-between BLAS instances.
  Because triangle geometry BLAS is first, it's set to 0 for triangle BLAS. AABB BLAS
  sets the offset to 2 since the triangle BLAS has two shader records for the plane geometry with radiance and shadow ray hit groups.

**Shader table indexing example**
 * A shader calls a TraceRay() for a shadow ray. Given its a shadow ray it sets *RayContributionToHitGroupIndex*
 to 1 as an offset to index shadow shader records. It sets *MultiplierForGeometryContributionToHitGroupIndex*
 to 2 because there are two shader records per geometry. 
* The shadow ray hits a second 
 AABB geometry, hence *GeometryContributionToHitGroupIndex* being 1, in the second BLAS that contains AABB geometries and second BLAS provides *InstanceContributionToHitGroupIndex* of 2. 
* The shader index into the hit group shader table above will be
 5 and calculated as:
```
  1  // ~ RayContributionToHitGroupIndex                   - from TraceRay()
+ 2  // ~ MultiplierForGeometryContributionToHitGroupIndex - from TraceRay() 
* 1  // ~ GeometryContributionToHitGroupIndex              - from runtime, 2nd geometry => ID:1
+ 2  // ~ InstanceContributionToHitGroupIndex              - from BLAS instance desc
```

## Usage
The sample starts with Raytracing Fallback Layer API implementation being used by default. The Fallback Layer will use raytracing driver if available, otherwise it will default to the compute fallback. This default behavior can be overriden via UI controls or input arguments.

D3D12RaytracingProceduralGeometry.exe [ -FL | -DXR | ...]
* [-FL] - select Fallback Layer API with forced compute fallback path.
* [-DXR] - select DirectX Raytracing API.

Additional arguments:
  * [-forceAdapter \<ID>] - create a D3D12 device on an adapter <ID>. Defaults to adapter 0.

### UI
The title bar of the sample provides runtime information:
* Name of the sample
* Raytracing API being active:
  * FL - Fallback Layer with compute fallback being used
  * FL-DXR - Fallback Layer with raytracing driver being used
  * DXR - DirectX Raytracing being used
* Frames per second
* Million Primary Rays/s: a number of dispatched rays per second calculated based of FPS.

### Controls
* ALT+ENTER - toggles between windowed and fullscreen modes.
* 1 - select Fallback Layer API.
* 2 - select Fallback Layer API with forced compute fallback path.
* 3 - select DirectX Raytracing API.
* L - enable/disable light animation.
* C - enable/disable camera animation.

## Requirements
* Consult the main [D3D12 Raytracing readme](../../readme.md) for requirements.