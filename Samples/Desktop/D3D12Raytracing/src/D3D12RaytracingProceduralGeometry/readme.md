# D3D12 Raytracing Procedural Geometry sample
![D3D12 Raytracing Procedural Geometry GUI](Screenshot.png)

This sample demonstrates how to implement procedural geometry using intersection shaders. Particularly, it showcases multiple intersection shaders creating analytic and volumetric, signed distance and fractal geometry. In addition, the sample introduces:
* Extended shader table layouts and indexing covering multiple geometries and bottom-level acceleration structures (AS).
* Use of trace ray recursion and two different ray types: radiance and shadow rays.

The sample assumes familiarity with Dx12 programming and DirectX raytracing concepts introduced in the [D3D12 Raytracing Simple Lighting sample](../D3D12RaytracingProceduralGeometry/readme.md).

### Scene
The scene consists of triangle and procedural/AABB geometry each stored in a separate bottom-level AS:
* Triangle geometry - ground plane.
* Procedural geometry - remaining objects above the ground plane in the scene.

#### Procedural geometry
Procedural geometry is defined by an AABB and an intersection shader. The AABB defines geometric confines within a bottom-level AS object space. Intersection shader evaluates if the ray that intersects the AABB intersects any geometry within and calls ReportHit(). This sample implements three intersection shaders, as well as multiple different primitives per each intersection shader:
  * Analytic - a geometry with multiple spheres and an axis aligned box.
  * Volumetric - a metaballs isosurface (aka "blobs").
  * Signed distance - six different primitives and a pyramid fractal.

The ray/procedural primitive tests used in the intersection shaders in this work in a primitive local space and their AABB implicitly is <-1,1>. The ray can however only be retrieved either retrieved in world space (via *WorldRay\*()* intrinsic) or bottom-level AS object space (via *ObjectRay\*()* intrinsic). Therefore the application passes transforms for converting between bottom-level AS object and primitive local space and vice versa to the shaders. Intersection shader then uses the transforms to calculate a ray in local space and calls one of the ray/primitive intersection tests such as *RayAnalyticGeometryIntersectionTest* in *MyIntersectionShader_AnalyticPrimitive* intersection shader: 
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


If the ray intersects the geometry, the test calculates the time of the hit as well as a surface normal. This is then passed to ReportHit(), which either accepts or rejects the hit. Before that happens however, the intersection shader needs to convert the normal from primitive local space to world space:
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
For efficiency, ray intersection tests in the sample validate each ray hit against the valid global time range as well as the RayFlags(), instead of depending on ReportHit and redoing all the setup math. 
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

You as a developer have a freedom to implement the ray intersection tests to fit your needs. This sample demonstrates intersection tests for ray vs following geometry:
  * Analytic - sphere and AABB.
  * Volumetric - metaballs isosurface (aka "blobs"). Metaballs are a form of point force fields that have an area of influence. The closer you get the larger the field potential value. The field values are summed from all participating metaballs. This sample visualizes them as an isosurface by testing for a particular threshold field value. Specifically, the intersection test ray marches within the AABB, calculating the total field potential at each iteration, until it hits the threshold or it exits the AABB.
  * Signed distance - signed distance geometry returns a distance to the geometry considering all directions. Since the distance is not given along the ray necessarily, the intersection test iteratively ray marches using the new signed distance until it crosses some application defined threshold. This algorithm is called sphere tracing and described in more detail at [https://www.scratchapixel.com/lessons/advanced-rendering/rendering-distance-fields/basic-sphere-tracer](https://www.scratchapixel.com/lessons/advanced-rendering/rendering-distance-fields/basic-sphere-tracer). The signed distance primitives are formed by signed distance functions or they can be combined along with allowed transforms applied to them to create more complex geometry. This is explained in more detail at [http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm](http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm).

##### Updates
 Procedural geometry can be animated or modified without requiring acceleration structure updates as long as the AABBs don't change. The sample animates some primitives this way and simply updates the transforms passed to shaders with update rotation transforms every frame. In the metaballs case it also passes application time to drive sphere position updates within the metaballs' AABB.

#### Ray types
The sample utilizes two ray types for raytracing: a radiance and a shadow ray. Radiance ray is used for primary/view and secondary/reflected ray calls for TraceRay, where ray calculats shading for each geometry hit and aggregates color contribution over multiple bounces from the scene. Shadow ray is used for visibility/occlusion testing towards a light source and is simpler since all it does is to return a boolean value if it hit any or missed all objects. In practice, shadow ray only requires a miss shader and doesn't require any hit and closest hit shaders defined. It can simply be initialized with ray payload marking a hit and and RayFlags to skip all but a miss shader and then let the miss shader change the hit boolean value to a false (i.e. a light source is not blocked by any object):
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

#### Shader tables


##### Shader table indexing 

When a ray hits geometry or needs to call a miss shader, GPU indexes into the application 
provided shader tables according to the shader table addressing:
```c
Miss shader table index = 
     MissShaderIndex                                     ~ from shader: TraceRay()

Hit group shader table index = 
     RayContributionToHitGroupIndex                      ~ from shader: TraceRay()  
     + MultiplierForGeometryContributionToHitGroupIndex  ~ from shader: TraceRay()
     * GeometryContributionToHitGroupIndex               ~ system generated index 
                                                           of geometry in BLAS
     + InstanceContributionToHitGroupIndex               ~ from BLAS instance desc
```

 
The sample traces radiance and shadow rays. Since the ray types apply different 
actions in the shaders, they require separate shader records for each geometry,
so that GPU executes the right shader when a geometry is hit or or miss shader needs
to be executed.

In addition, this sample uses multiple different geometries. First, there is 
triangle geometry (the ground plane) and then there is AABB/procedural geometry. 
AABB geometry requires intersection shaders to be specifiend in the hit 
groups. This sample demonstrates a use of three different intersection shaders,
and thus further enumeration of different shader records per each:
 - MyIntersectionShader_AnalyticPrimitive
 - MyIntersectionShader_VolumetricPrimitive
 - MyIntersectionShader_SignedDistancePrimitive

Last, the sample defines multiple geometries per each intersection shader, 
all parametrized via attributes provided via local root signatures,
which in turn each require a different shader record. 
//
With that said, the sample has following shader table layouts. 
//
Miss shader table layout
o There are two miss shader records:
  [0] : Miss shader record for a radiance ray
  [1] : Miss shader record for a shadow ray
//
Hit group shader table layout:
 o Triangle geometry shader records - single triangular geometry (ground plane)
 o AABB geometry shader records- multiple AABB geometries ~ IntersectionShaderType::TotalPrimitiveCount()
Both triangle and ABB geometry require two shader records, per ray type:
  [0] : Hit group shader record for geometry ID 0 for a radiance ray
  [1] : Hit group shader record for geometry ID 0 for a shadow ray
//
The actual shader tables are printed out in the debug output by this sample 
and its useful as a reference to indexing parameters. Here are first few
shader records of a hit group for reference in the examples below:
```
| Shader table - HitGroupShaderTable: 
| [0] : MyHitGroup_Triangle                           Triangle geometry in 1st BLAS
| [1] : MyHitGroup_Triangle_ShadowRay
| [2] : MyHitGroup_AABB_AnalyticPrimitive             ~ 1st AABB geometry in 2nd BLAS
| [3] : MyHitGroup_AABB_AnalyticPrimitive_ShadowRay
| [4] : MyHitGroup_AABB_AnalyticPrimitive             ~ 2nd AABB geometry in 2nd BLAS
| [5] : MyHitGroup_AABB_AnalyticPrimitive_ShadowRay
  ...
```
Given the shader table layout, the shader table indexing is set as follows:
o MissShaderIndex is set to 0 for radiance rays, and 1 for shadow rays in TraceRay().
o *GeometryContributionToHitGroupIndex* is a Geometry ID that is system 
  generated for each geometry within a BLAS. This directly maps to GeometryDesc 
  order passed in by the app, i.e. {0, 1, 2,...}
o Given two hit groups (radiance, shadow ray) per geometry ID, 
  *MultiplierForGeometryContributionToHitGroupIndex* is 2. 
o *RayContributionToHitGroupIndex* is set to 0 and 1, for radiance and shadow 
  rays respectively, since they're stored subsequently in a shader table.
* InstanceContributionToHitGroupIndex is an offset in-between instances.
  Since triangle BLAS is first, it's set to 0 for triangle BLAS. AABB BLAS 
  sets it 2 since the BLAS has to skip over the triangle BLAS's shader
  records, which is 2 since the triangle plane has two shader records 
  (one for radiance, and one for shadow ray).
//
Example:
 A shader calls a TraceRay() for a shadow ray and sets *RayContributionToHitGroupIndex*
 to 1 as an offset to shadow shader records and *MultiplierForGeometryContributionToHitGroupIndex*
 to 2 since there are two shader records per geometry. The shadow ray hits a second 
 AABB geometry in the 2nd BLAS that contains AABB geometries. The shader index will be
 5 in the table above and is calculated as:
    1  // ~ RayContributionToHitGroupIndex                   - from TraceRay()                 
  + 2  // ~ MultiplierForGeometryContributionToHitGroupIndex - from TraceRay() 
  * 1  // ~ GeometryContributionToHitGroupIndex              - from runtime, 2nd geometry => ID:1
+ 2  // ~ InstanceContributionToHitGroupIndex              - from BLAS instance desc

##### Raytracing setup


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