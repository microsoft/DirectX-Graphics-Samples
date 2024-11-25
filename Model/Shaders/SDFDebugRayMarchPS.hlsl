// -- BINDINGS --

cbuffer CameraCB : register(b0) {
    float4x4 invViewProjection; 
    float3 cameraPosition;    
    float padding; 
    float xmin;
    float xmax;
    float ymin;
    float ymax;
    float zmin;
    float zmax;
    // texture resolution
    float sdfResolution;
};

RWTexture3D<float4> AlbedoTex : register(u0);
RWTexture3D<float> SDFTex : register(u1);

struct VSOutput {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

// -- CONSTANTS --

static int MAX_MARCHING_STEPS = 512;
// matches near and far plane of camera defined in ModelViewer.cpp
static float MIN_DIST = 1.0;
static float MAX_DIST = 10000.0;
static float EPSILON = 0.1;

// -- SHADER START --

/**
 * Signed distance function for a sphere centered at the origin with radius 1.0;
 */
float sphereSDF(float3 samplePoint, float r) {
    return length(samplePoint) - r;
}

float boxSDF(float3 p, float3 b)
{
    float3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

// assuming that the 3D texture covers a box (in world space) that:
//      * is centered at the origin
//      * Xbounds = [-2000, 2000]
//      * Ybounds = [-2000, 2000]
//      * Zbounds = [-2000, 2000]
float3 worldToTex(float3 worldPos) {
    float3 texCoord = float3(0, 0, 0); 

    // world coord to [0, 1] coords
    texCoord.x = (worldPos.x - xmin) / (xmax - xmin);
    texCoord.y = (worldPos.y - ymin) / (ymax - ymin);
    texCoord.z = (worldPos.z - zmin) / (zmax - zmin);

    // assuming a 128 * 128 * 128 texture, but this can be changed
    // u' = u * (tmax - tmin) + tmin
    // where tmax == 127 and tmin == 0
    return texCoord * (sdfResolution - 1);
}

// hm I'm not actually sure if I need texToWorld
float3 texToWorld(uint3 texPos) {
    // normalize texPos
    uint tmin = 0; 
    uint tmax = sdfResolution - 1;
    float3 range = tmax - tmin; 
    float3 uvw = float3(0, 0, 0);
    uvw.x = (texPos.x - tmin) / range; 
    uvw.y = (texPos.y - tmin) / range; 
    uvw.z = (texPos.z - tmin) / range; 

    // uvw space to world space

    float3 worldPos = float3(0, 0, 0); 

    worldPos.x = uvw.x * (xmax - xmin) + xmin; 
    worldPos.y = uvw.y * (ymax - ymin) + ymin; 
    worldPos.z = uvw.z * (zmax - zmin) + zmin; 

    return worldPos;
}

/**
 * Signed distance function describing the scene.
 *
 * Absolute value of the return value indicates the distance to the surface.
 * Sign indicates whether the point is inside or outside the surface,
 * negative indicating inside.
 */
float sceneSDF(float3 samplePoint) {
    return boxSDF(samplePoint, float3(100, 100, 100));
}

float3 rayDirection(float fieldOfView, float2 uv) {
    // Map uv from [0,1] to [-1,1] for x and y
    float2 xy = uv * 2.0 - 1.0;

    // Adjust for the aspect ratio
    float aspectRatio = 512.0 / 512.0; // Replace with actual resolution if not square
    xy.x *= aspectRatio; 

    // Compute z based on the field of view
    float z = 1.0 / tan(radians(fieldOfView) / 2.0);

    return normalize(float3(xy, -z));
}

float3 rayDirectionFromCamera(float2 uv, float4x4 invViewProjection) {
    uv.y = 1. - uv.y; 

    // Transform the UV coordinates to NDC (Normalized Device Coordinates)
    float4 ndc = float4(uv * 2.0 - 1.0, 0.0, 1.0);

    // Transform from NDC to world space using the inverse view-projection matrix
    float4 worldPos = mul(invViewProjection, ndc);
    worldPos /= worldPos.w;

    // Compute the ray direction in world space
    float3 rayDir = normalize(worldPos.xyz - cameraPosition);

    return rayDir;
}

float shortestDistanceToSurface(float3 eye, float3 marchingDirection, float start, float end) {
    float depth = start;
    for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
        float dist = sceneSDF(eye + depth * marchingDirection);
        // float dist = 0; 
        if (dist < EPSILON) {
            return depth;
        }
        depth += dist;
        if (depth >= end) {
            return end;
        }
    }
    return end;
}

int3 shortestDistanceToSurfaceTexSpace(float3 eye, float3 marchingDirection, out float depth) {
    float start = 0; 
    depth = start;
    for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
        int3 hit = (eye + depth * marchingDirection);
        if (any(hit > int3(sdfResolution - 1, sdfResolution - 1, sdfResolution - 1)) || any(hit < int3(0, 0, 0))) {
            return int3(-1, -1, -1);
        }
        hit.y = sdfResolution - 1 - hit.y;
        hit.z = sdfResolution - 1 - hit.z;
        float dist = SDFTex[hit];
        if (dist == 0.f) {
            return hit;
        }
        depth += dist;
    }
    return int3(-1, -1, -1);
}

float4 main(VSOutput input) : SV_TARGET{
    // calculate eye and direction using hard-coded values
    // float3 dir = rayDirection(45.0, input.uv);
    // float3 eye = float3(0.0, 0.0, 5.0);

    // Computing eye and direction using camera matrix
    float3 dir = rayDirectionFromCamera(input.uv, invViewProjection);
    float3 eye = cameraPosition;
    float dist = 0.f; 

    int3 hit = shortestDistanceToSurfaceTexSpace(worldToTex(eye), dir, dist);

    if (hit.x == -1) {
        // Didn't hit anything
        return float4(0.0, 0.0, 0.0, 0.0);
    }

    return AlbedoTex[hit];
}
