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

// A family of BRDF, BSDF and BTDF functions.
// BRDF, BSDF, BTDF - bidirectional reflectance, scattering, transmission distribution function.
// Ref: Ray Tracing from the Ground Up (RTG), Suffern
// Ref: Real-Time Rendering (RTR), Fourth Edition
// Ref: Real Shading in Unreal Engine 4 (Karis_UE4), Karis2013
// Ref: PBR Diffuse Lighting for GGX+Smith Microsurfaces, Hammon2017

// BRDF terms generally include 1 / PI factor, but this is removed in the implementations below as it cancels out
// with the omitted PI factor in the reflectance equation. Ref: eq 9.14, RTR

// Parameters:
// iorIn - ior of media ray is coming from
// iorOut - ior of media ray is going to
// eta - relative index of refraction, namely iorIn / iorOut
// G - shadowing/masking function.
// Fo - specular reflectance at normal incidence (AKA specular color).
// Albedo - material color
// Roughness - material roughness
// N - surface normal
// V - direction to viewer
// L - incoming "to-light" direction
// T - transmission scale factor (aka transmission color)
// thetai - incident angle

#ifndef BXDF_HLSL
#define BXDF_HLSL

namespace BxDF {

    // This namespace implements BTDF for a perfect transmitter that uses a single index of refraction (ior)
    // and iorOut represent air, i.e. 1.
    bool IsBlack(float3 color)
    {
        return !any(color);
    }

    namespace Diffuse {

        namespace Lambert {

            float3 F(in float3 albedo)
            {
                return albedo;
            }
        }

        namespace Hammon {

            // Compute the value of BRDF
            // Ref: Hammon2017
            float3 F(in float3 Albedo, in float Roughness, in float3 N, in float3 V, in float3 L, in float3 Fo)
            {
                float3 diffuse = 0;

                float3 H = normalize(V + L);
                float NoH = dot(N, H);
                if (NoH > 0)
                {
                    float a = Roughness * Roughness;

                    float NoV = saturate(dot(N, V));
                    float NoL = saturate(dot(N, L));
                    float LoV = saturate(dot(L, V));

                    float facing = 0.5 + 0.5 * LoV;
                    float rough = facing * (0.9 - 0.4 * facing) * ((0.5 + NoH) / NoH);
                    float3 smooth = 1.05 * (1 - pow(1 - NoL, 5)) * (1 - pow(1 - NoV, 5));

                    // Extract 1 / PI from the single equation since it's ommited in the reflectance function.
                    float3 single = lerp(smooth, rough, a);
                    float multi = 0.3641 * a; // 0.3641 = PI * 0.1159

                    diffuse = Albedo * (single + Albedo * multi);
                }
                return diffuse;
            }
        }
    }

    // Fresnel reflectance - schlick approximation.
    float3 Fresnel(in float3 F0, in float cos_thetai)
    {
        return F0 + (1 - F0)*pow(1 - cos_thetai, 5);
    }

    namespace Specular {

        // Perfect/Specular reflection.
        namespace Reflection {
        
            // Calculates L and returns BRDF value for that direction.
            // Assumptions: V and N are in the same hemisphere.
            // Note: to avoid unnecessary precision issues and for the sake of performance the function doesn't divide by the cos term
            // so as to nullify the cos term in the rendering equation. Therefore the caller should skip the cos term in the rendering equation.
            float3 Sample_Fr(in float3 V, out float3 L, in float3 N, in float3 Fo)
            {
                L = reflect(-V, N);
                float cos_thetai = dot(N, L);
                return Fresnel(Fo, cos_thetai);
            }
            
            // Calculate whether a total reflection occurs at a given V and a normal
            // Ref: eq 27.5, Ray Tracing from the Ground Up
            BOOL IsTotalInternalReflection(
                in float3 V,
                in float3 normal)
            {
                float ior = 1; 
                float eta = ior;
                float cos_thetai = dot(normal, V); // Incident angle

                return 1 - 1 * (1 - cos_thetai * cos_thetai) / (eta * eta) < 0;
            }

        }

        // Perfect/Specular trasmission.
        namespace Transmission {

            // Calculates transmitted ray wt and return BRDF value for that direction.
            // Assumptions: V and N are in the same hemisphere.
            // Note: to avoid unnecessary precision issues and for the sake of performance the function doesn't divide by the cos term
            // so as to nullify the cos term in the rendering equation. Therefore the caller should skip the cos term in the rendering equation.
            float3 Sample_Ft(in float3 V, out float3 wt, in float3 N, in float3 Fo)
            {
                float ior = 1;
                wt = -V; // TODO: refract(-V, N, ior);
                float cos_thetai = dot(V, N);
                float3 Kr = Fresnel(Fo, cos_thetai);

                return (1 - Kr);
            }
        }

        // Ref: Chapter 9.8, RTR
        namespace GGX {

            // Compute the value of BRDF
            float3 F(in float Roughness, in float3 N, in float3 V, in float3 L, in float3 Fo)
            {
                float3 H = V + L;
                float NoL = dot(N, L);
                float NoV = dot(N, V);
                float3 specular = 0;

                if (NoL > 0 && NoV > 0 && all(H))
                {
                    H = normalize(H);
                    float a = Roughness;        // The roughness has already been remapped to alpha.
                    float3 M = H;               // microfacet normal, equals h, since BRDF is 0 for all m =/= h. Ref: 9.34, RTR
                    float NoM = saturate(dot(N, M));
                    float HoL = saturate(dot(H, L));

                    // D
                    // Ref: eq 9.41, RTR
                    float denom = 1 + NoM * NoM * (a * a - 1);
                    float D = a * a / (denom * denom);  // Karis

                    // F
                    // Fresnel reflectance - Schlick approximation for F(h,l)
                    // Ref: 9.16, RTR
                    float3 F = Fresnel(Fo , HoL);

                    // G
                    // Visibility due to shadowing/masking of a microfacet.
                    // G coupled with BRDF's {1 / 4 DotNL * DotNV} factor.
                    // Ref: eq 9.45, RTR
                    float G =  0.5 / lerp(2 * NoL * NoV, NoL + NoV, a);

                    // Specular BRDF term
                    // Ref: eq 9.34, RTR
                    specular = F * G * D;
                }

                return specular;
            }
        }
    }

    namespace DirectLighting
    {
        // Calculate a color of the shaded surface due to direct lighting.
        // Returns a shaded color.
        float3 Shade(
            in MaterialType::Type materialType,
            in float3 Albedo,
            in float3 Fo,
            in float3 Radiance,
            in bool inShadow,
            in float Roughness,
            in float3 N,
            in float3 V,
            in float3 L)
        {
            float3 directLighting = 0;
            
            float NoL = dot(N, L);
            if (!inShadow && NoL > 0)
            {
                float3 directDiffuse = 0;
                if (!IsBlack(Albedo))
                {
                    if (materialType == MaterialType::Default)
                    {
                        directDiffuse = BxDF::Diffuse::Hammon::F(Albedo, Roughness, N, V, L, Fo);
                    }
                    else
                    {
                        directDiffuse = BxDF::Diffuse::Lambert::F(Albedo);
                    }
                }

                float3 directSpecular = 0;
                if (materialType == MaterialType::Default)
                {
                    directSpecular = BxDF::Specular::GGX::F(Roughness, N, V, L, Fo);
                }

                directLighting = NoL *  Radiance * (directDiffuse + directSpecular);
            }
            
            return directLighting;
        }
    }

    // Calculate a color of the shaded surface.
    float3 Shade(
        in MaterialType::Type materialType,
        in float3 Albedo,
        in float3 Fo,
        in float3 Radiance,
        in bool isInShadow,
        in float AmbientCoef,
        in float Roughness,
        in float3 N,
        in float3 V,
        in float3 L)
    {
        float NoL = dot(N, L);
        Roughness = max(0.1, Roughness);
        float3 directLighting = 0;

        if (!isInShadow && NoL > 0)
        {
            // Diffuse.
            float3 diffuse;
            if (materialType == MaterialType::Default)
            {
                diffuse = BxDF::Diffuse::Hammon::F(Albedo, Roughness, N, V, L, Fo);
            }
            else
            {
                diffuse = BxDF::Diffuse::Lambert::F(Albedo);
            }

            // Specular.
            float3 directDiffuse = diffuse;
            float3 directSpecular = 0;

            if (materialType == MaterialType::Default)
            {
                directSpecular = BxDF::Specular::GGX::F(Roughness, N, V, L, Fo);
            }

            directLighting = NoL * Radiance * (directDiffuse + directSpecular);
        }

        float3 indirectDiffuse = AmbientCoef * Albedo;
        float3 indirectLighting = indirectDiffuse;

        return directLighting + indirectLighting;
    }
}


#endif // BXDF_HLSL