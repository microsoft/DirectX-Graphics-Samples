//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// The mesh shader procedurally builds a solid 3D spur gear every frame - there
// is no vertex or index buffer. One thread group generates the whole gear: each
// of DIVISIONS threads produces one angular slice (4 vertices and 8 triangles)
// covering the front face, back face, outer tooth wall, and inner hub-hole wall.
// This showcases parallel, buffer-less geometry amplification, while the three
// blend permutations that draw it come from partial graphics programs.

#define DIVISIONS      24               // angular slices around the gear
#define TEETH          6                // number of gear teeth
#define DIV_PER_TOOTH  (DIVISIONS / TEETH)

struct VertexOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

// Three 32-bit root constants supplied per draw:
//   InstanceIndex - selects which of the three gears (offset + color + depth layer).
//   AspectRatio   - window aspect ratio, keeps the gear from stretching.
//   Rotation      - animation angle (radians); the gear spins about its axis.
cbuffer DrawConstants : register(b0)
{
    uint  InstanceIndex;
    float AspectRatio;
    float Rotation;
};

static const float2 Offsets[3] =
{
    float2(-0.30f, 0.0f), // opaque   (red)   - left, back layer
    float2( 0.00f, 0.0f), // additive (green) - center
    float2( 0.30f, 0.0f), // alpha    (blue)  - right, front layer
};

static const float3 BaseColors[3] =
{
    float3(1.00f, 0.25f, 0.20f), // opaque (red)
    float3(0.25f, 1.00f, 0.35f), // additive (green)
    float3(0.40f, 0.55f, 1.00f), // alpha blend (blue)
};

static const float  Alpha        = 0.8f;
static const float  TipRadius    = 0.34f;  // tooth tip radius (NDC)
static const float  RootRadius   = 0.25f;  // valley radius between teeth
static const float  HoleRadius   = 0.13f;  // central hub hole radius
static const float  HalfDepth    = 0.05f;  // half the gear thickness (along Z)
static const float  Tilt         = 0.42f;  // fixed tilt (radians) so the 3D reads
static const float  Ambient      = 0.25f;
static const float3 LightDir     = float3(0.3234f, 0.4312f, 0.8427f); // normalized, toward viewer
static const float  DepthScale   = 0.35f;  // maps model Z into a shallow NDC depth band
static const float  DepthBiasStep = 0.16f; // separates the three gears in depth

float3 RotateZ(float3 p, float s, float c) { return float3(c * p.x - s * p.y, s * p.x + c * p.y, p.z); }
float3 RotateX(float3 p, float s, float c) { return float3(p.x, c * p.y - s * p.z, s * p.y + c * p.z); }

// Build one transformed, lit vertex. radialSign is +1 for the outer edge and -1
// for the inner (hole) edge; zside is +1 for the front face and -1 for the back.
// The smooth normal blends the radial and axial directions, giving beveled edges.
VertexOut MakeVertex(float radius, float theta, float zside, float radialSign)
{
    const float2 off  = Offsets[InstanceIndex];
    const float3 base = BaseColors[InstanceIndex];

    float3 pos = float3(radius * cos(theta), radius * sin(theta), zside * HalfDepth);
    float3 nrm = normalize(float3(radialSign * cos(theta), radialSign * sin(theta), zside));

    // Spin about the gear's axis (animated), then a fixed tilt about X so the
    // thickness and teeth are visible. Normals get the same rotations.
    float ss = sin(Rotation), cs = cos(Rotation);
    float st = sin(Tilt),     ct = cos(Tilt);
    pos = RotateX(RotateZ(pos, ss, cs), st, ct);
    nrm = RotateX(RotateZ(nrm, ss, cs), st, ct);

    float diff  = saturate(dot(nrm, LightDir));
    float shade = Ambient + (1.0f - Ambient) * diff;

    // Orthographic projection. Larger world Z is nearer the viewer, so negate it
    // for NDC depth (smaller = nearer). A per-instance bias layers the gears.
    float2 xy    = float2(pos.x + off.x, (pos.y + off.y) * AspectRatio);
    float  depth = 0.5f - pos.z * DepthScale + (1.0f - InstanceIndex) * DepthBiasStep;

    VertexOut v;
    v.position = float4(xy, saturate(depth), 1.0f);
    v.color = float4(base * shade, Alpha);
    return v;
}

[NumThreads(DIVISIONS, 1, 1)]
[OutputTopology("triangle")]
void MSMain(
    uint gtid : SV_GroupThreadID,
    out vertices VertexOut verts[DIVISIONS * 4],
    out indices uint3 tris[DIVISIONS * 8])
{
    SetMeshOutputCounts(DIVISIONS * 4, DIVISIONS * 8);

    const uint a = gtid;                       // this slice
    const uint b = (gtid + 1) % DIVISIONS;     // next slice (wraps around)

    // Trapezoidal (flat-topped) tooth profile: two tip samples then two root
    // samples per tooth period give a flat top, sloped flanks, and flat valley.
    const uint  toothPhase = a % DIV_PER_TOOTH;
    const float outerR = (toothPhase == 1 || toothPhase == 2) ? TipRadius : RootRadius;

    const float theta = (6.2831853f * a) / DIVISIONS;

    // Four vertices for this slice: outer/inner x front/back.
    const uint va = 4 * a;
    verts[va + 0] = MakeVertex(outerR,     theta,  1.0f,  1.0f); // OuterFront
    verts[va + 1] = MakeVertex(HoleRadius, theta,  1.0f, -1.0f); // InnerFront
    verts[va + 2] = MakeVertex(outerR,     theta, -1.0f,  1.0f); // OuterBack
    verts[va + 3] = MakeVertex(HoleRadius, theta, -1.0f, -1.0f); // InnerBack

    // Eight triangles bridging this slice (a) to the next (b). Winding is chosen
    // so each outward-facing surface is front-facing under the default back-face
    // culling (matching the sibling sample's convention).
    const uint vb = 4 * b;
    const uint t  = 8 * a;
    tris[t + 0] = uint3(va + 0, va + 1, vb + 1); // front face
    tris[t + 1] = uint3(va + 0, vb + 1, vb + 0);
    tris[t + 2] = uint3(va + 2, vb + 2, vb + 3); // back face
    tris[t + 3] = uint3(va + 2, vb + 3, va + 3);
    tris[t + 4] = uint3(va + 0, vb + 0, vb + 2); // outer (tooth) wall
    tris[t + 5] = uint3(va + 0, vb + 2, va + 2);
    tris[t + 6] = uint3(va + 1, va + 3, vb + 3); // inner (hub hole) wall
    tris[t + 7] = uint3(va + 1, vb + 3, vb + 1);
}

float4 PSMain(VertexOut input) : SV_TARGET
{
    return input.color;
}
