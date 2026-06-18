RWTexture2DArray<float4> g_output : register(u0);

struct ProceduralEnvironmentSettings
{
    int source;
    float3 pad0;
    float4 skyColor;
    float4 groundColor;
    float4 lightColor;
    float4 lightDirection;
    float backgroundIntensity;
    float lightIntensity;
    float lightSize;
    float fillIntensity;
    float colorPanelIntensity;
    float horizonSharpness;
    int outputSize;
    int generationMode;
    float roughness;
    int pad2;
};

ConstantBuffer<ProceduralEnvironmentSettings> g_settings : register(b0);

static const int kEnvironmentMapFaceCount = 6;
static const int kEnvironmentStudio = 1;
static const int kEnvironmentSun = 2;
static const int kEnvironmentColorPanels = 3;
static const int kEnvironmentHorizon = 4;
static const int kGenerateEnvironment = 0;
static const int kGenerateDiffuseIrradiance = 1;
static const int kGenerateSpecularPrefilter = 2;
static const uint kSampleCount = 64;
static const float kPi = 3.1415926535;

float3 NormalizeFloat3(float3 v)
{
    return normalize(v);
}

float Clamp01(float v)
{
    return saturate(v);
}

float SmoothStep(float edge0, float edge1, float x)
{
    float t = saturate((x - edge0) / (edge1 - edge0));
    return t * t * (3.0 - 2.0 * t);
}

float3 DirectionForCubeFace(int face, float u, float v)
{
    if (face == 0) return float3(1.0, v, -u);
    if (face == 1) return float3(-1.0, v, u);
    if (face == 2) return float3(u, 1.0, -v);
    if (face == 3) return float3(u, -1.0, v);
    if (face == 4) return float3(u, v, 1.0);
    return float3(-u, v, -1.0);
}

float RadicalInverseVdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

float2 Hammersley(uint i, uint n)
{
    return float2(float(i) / float(n), RadicalInverseVdC(i));
}

void BuildTangentFrame(float3 normal, out float3 tangent, out float3 bitangent)
{
    float3 up = abs(normal.y) < 0.999 ? float3(0.0, 1.0, 0.0) : float3(1.0, 0.0, 0.0);
    tangent = normalize(cross(up, normal));
    bitangent = cross(normal, tangent);
}

float3 ToWorld(float3 tangent, float3 bitangent, float3 normal, float3 local)
{
    return tangent * local.x + bitangent * local.y + normal * local.z;
}

float3 SampleProceduralEnvironment(ProceduralEnvironmentSettings settings, float3 direction)
{
    direction = normalize(direction);
    float3 lightDir = normalize(settings.lightDirection.xyz);
    float skyBlend = Clamp01(direction.y * 0.5 + 0.5);
    float horizonWidth = max(0.01, settings.horizonSharpness);
    float horizonBlend = SmoothStep(-horizonWidth, horizonWidth, direction.y);
    float3 baseHorizon = lerp(settings.groundColor.xyz, settings.skyColor.xyz, horizonBlend);
    float3 baseSky = lerp(settings.groundColor.xyz, settings.skyColor.xyz, skyBlend);
    float lightDot = Clamp01(dot(direction, lightDir));
    float lightSize = max(0.01, settings.lightSize);
    float lightSpot = pow(lightDot, 1.0 / lightSize);

    if (settings.source == kEnvironmentSun)
    {
        float3 background = baseSky * settings.backgroundIntensity;
        float3 sun = settings.lightColor.xyz * lightSpot * settings.lightIntensity;
        return background + sun;
    }
    else if (settings.source == kEnvironmentColorPanels)
    {
        float3 panelColor = 0;
        float ax = abs(direction.x);
        float ay = abs(direction.y);
        float az = abs(direction.z);
        if (ax >= ay && ax >= az)
        {
            panelColor = direction.x > 0.0 ? float3(1.0, 0.12, 0.08) : float3(0.05, 0.55, 1.0);
        }
        else if (ay >= ax && ay >= az)
        {
            panelColor = direction.y > 0.0 ? float3(1.0, 1.0, 1.0) : float3(0.18, 0.15, 0.12);
        }
        else
        {
            panelColor = direction.z > 0.0 ? float3(0.10, 1.0, 0.22) : float3(1.0, 0.82, 0.08);
        }
        float3 background = baseHorizon * settings.fillIntensity;
        return background + panelColor * settings.colorPanelIntensity;
    }
    else if (settings.source == kEnvironmentHorizon)
    {
        float horizonLine = 1.0 - SmoothStep(0.0, horizonWidth, abs(direction.y));
        float3 horizonLight = settings.lightColor.xyz * horizonLine * settings.lightIntensity * 0.25;
        return baseHorizon * settings.backgroundIntensity + horizonLight;
    }
    else
    {
        float3 background = baseHorizon * settings.backgroundIntensity;
        float3 softbox = settings.lightColor.xyz * lightSpot * settings.lightIntensity;
        float3 fill = settings.skyColor.xyz * settings.fillIntensity * skyBlend;
        return background + softbox + fill;
    }
}

float3 ComputeDiffuseIrradiance(ProceduralEnvironmentSettings settings, float3 normal)
{
    float3 tangent;
    float3 bitangent;
    BuildTangentFrame(normal, tangent, bitangent);

    float3 irradiance = 0.0;
    for (uint i = 0; i < kSampleCount; ++i)
    {
        float2 xi = Hammersley(i, kSampleCount);
        float phi = 2.0 * kPi * xi.x;
        float cosTheta = sqrt(1.0 - xi.y);
        float sinTheta = sqrt(xi.y);
        float3 local = float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
        irradiance += SampleProceduralEnvironment(settings, ToWorld(tangent, bitangent, normal, local));
    }

    return irradiance * (kPi / float(kSampleCount));
}

float3 ImportanceSampleGGX(float2 xi, float roughness, float3 tangent, float3 bitangent, float3 normal)
{
    float a = roughness * roughness;
    float phi = 2.0 * kPi * xi.x;
    float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));
    float sinTheta = sqrt(max(0.0, 1.0 - cosTheta * cosTheta));
    float3 local = float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    return normalize(ToWorld(tangent, bitangent, normal, local));
}

float3 ComputeSpecularPrefilter(ProceduralEnvironmentSettings settings, float3 reflectionDir)
{
    float roughness = settings.roughness;
    if (roughness <= 0.001)
    {
        return SampleProceduralEnvironment(settings, reflectionDir);
    }

    float3 tangent;
    float3 bitangent;
    BuildTangentFrame(reflectionDir, tangent, bitangent);

    float3 viewDir = reflectionDir;
    float3 prefiltered = 0.0;
    float totalWeight = 0.0;
    for (uint i = 0; i < kSampleCount; ++i)
    {
        float2 xi = Hammersley(i, kSampleCount);
        float3 halfVector = ImportanceSampleGGX(xi, roughness, tangent, bitangent, reflectionDir);
        float3 lightDir = normalize(reflect(-viewDir, halfVector));
        float ndotl = Clamp01(dot(reflectionDir, lightDir));
        if (ndotl > 0.0)
        {
            prefiltered += SampleProceduralEnvironment(settings, lightDir) * ndotl;
            totalWeight += ndotl;
        }
    }

    return totalWeight > 0.0 ? prefiltered / totalWeight : SampleProceduralEnvironment(settings, reflectionDir);
}

[numthreads(8, 8, 1)]
void CSMain(uint3 dtid : SV_DispatchThreadID)
{
    int face = dtid.z;
    int x = dtid.x;
    int y = dtid.y;
    if (x >= g_settings.outputSize || y >= g_settings.outputSize || face >= kEnvironmentMapFaceCount)
    {
        return;
    }

    float u = (float(x) + 0.5) / float(g_settings.outputSize) * 2.0 - 1.0;
    float v = 1.0 - (float(y) + 0.5) / float(g_settings.outputSize) * 2.0;
    float3 direction = normalize(DirectionForCubeFace(face, u, v));
    float3 color = SampleProceduralEnvironment(g_settings, direction);
    if (g_settings.generationMode == kGenerateDiffuseIrradiance)
    {
        color = ComputeDiffuseIrradiance(g_settings, direction);
    }
    else if (g_settings.generationMode == kGenerateSpecularPrefilter)
    {
        color = ComputeSpecularPrefilter(g_settings, direction);
    }

    g_output[uint3(x, y, face)] = float4(color, 1.0);
}
