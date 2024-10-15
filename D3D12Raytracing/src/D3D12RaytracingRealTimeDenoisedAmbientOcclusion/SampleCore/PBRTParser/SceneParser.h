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

//
// Based on DuosRenderer [9/2018]
// https://github.com/wallisc/DuosRenderer/tree/DXRRenderer/PBRTParser
//

#pragma once

namespace SceneParser
{

	struct Vector2
	{
		union {
			struct {
				float x, y;
			};
			struct {
				float u, v;
			};
			DirectX::XMFLOAT2 xmFloat2;
		};

		float &operator[](UINT i)
		{
			switch (i)
			{
			case 0:
				return x;
			case 1:
				return y;
			default:
				assert(false);
				return y;
			}
		}

		XMVECTOR ToXMVECTOR() { return XMLoadFloat2(&xmFloat2); }
	};

	struct Vector3
	{
        Vector3(float v) : x(v), y(v), z(v) {}
		Vector3(float nX, float nY, float nZ) : x(nX), y(nY), z(nZ) {}
		Vector3() : Vector3(0, 0, 0) {}

		float &operator[](UINT i)
		{
			switch (i)
			{
			case 0:
				return x;
			case 1:
				return y;
			case 2:
				return z;
			default:
				assert(false);
				return z;
			}
		}
		union {
			struct {
				float x, y, z;
			};
			struct {
				float r, g, b;
			};
			DirectX::XMFLOAT3 xmFloat3;
		};
		XMVECTOR GetXMVECTOR() { return XMLoadFloat3(&xmFloat3); }
	};

	struct Film
	{
		UINT m_ResolutionX;
		UINT m_ResolutionY;
		std::string m_Filename;
	};

	struct Camera
	{
		// In Degrees. The is the narrower of the view frustrums width/height
		float m_FieldOfView;
		float m_NearPlane;
		float m_FarPlane;
		Vector3 m_Position;
		Vector3 m_LookAt;
		Vector3 m_Up;
	};

    // PBR material
    // Ref: https://www.pbrt.org/fileformat-v3.html
	struct Material
	{
		std::string m_MaterialName;
        MaterialType::Type m_Type;

        Vector3 m_Kd = 0;                 // The diffuse reflectivity of the surface.
        Vector3 m_Ks = 0.04f;                 // The specular reflectivity of the surface.
		Vector3 m_Kr = 0;                 // The reflectivity of the surface.
		Vector3 m_Kt = 0;                 // The transmissivity of the surface.
        Vector3 m_Opacity = 1;            // The opacity of the surface. If less than one, "uber" material transmits light without refracting it.
        Vector3 m_Eta = 1;                // The index of refraction of the object. Exterior is assumed to be vacuum with IOR of 1. 
        float   m_Roughness = 0.2f;        // Microfacet roughness alpha [0, 1].
		std::string m_DiffuseTextureFilename;
		std::string m_SpecularTextureFilename;
		std::string m_OpacityTextureFilename;
        std::string m_NormalMapTextureFilename;

        void Initialize(std::string type)
        {
            if (type == "matte")
            {
                m_Type = MaterialType::Matte;
                m_Kd = 1;
                m_Ks = 0;
                m_Roughness = 0.2f;
            }
            else if (type == "glass")
            {
                m_Type = MaterialType::Default;
                m_Eta = 1.5f;
                m_Kd = 0;
                m_Kr = 1;
                m_Kt = 1;
                m_Opacity = 0;
            }
            else if (type == "substrate")
            {
                m_Type = MaterialType::Default;
                m_Kd = 0.5f;
                m_Roughness = 0.1f;
            }
            else if (type == "uber")
            {
                m_Type = MaterialType::Matte;
                m_Kd = 0.25f;
                m_Kr = 0;
                m_Kt = 0;
                m_Roughness = 0.1f;
                m_Eta = 1.5f;
            }
            else if (type == "metal")
            {
                m_Type = MaterialType::Default;
                m_Kd = 1;
                m_Ks = 1;
            }
            else if (type == "mirror")
            {
                m_Type = MaterialType::Mirror;
                m_Kr = 0.9f;
            }
            else if (type == "AnalyticalCheckerboard")
            {
                m_Type = MaterialType::AnalyticalCheckerboardTexture;
            }
            else // Plastic
            {
                m_Type = MaterialType::Default;
                m_Kd = 0.25f;
                m_Roughness = 0.1f;
            }
        }
	};


	struct Vertex
	{
		Vector3 Normal;
		Vector3 Position;
        Vector2 UV;
		Vector3 Tangent;
	};

	typedef UINT Index;

    struct Mesh
    {
        Material *m_pMaterial;
        std::vector<Index> m_IndexBuffer;
        std::vector<Vertex> m_VertexBuffer;
		XMMATRIX m_transform;

        void GenerateTangents()
        {
            // Zero tangents.
            for (auto& vertex : m_VertexBuffer)
            {
                vertex.Tangent.xmFloat3 = XMFLOAT3(0, 0, 0);
            }

            // Add tangents from all triangles a vertex corresponds to.
            for (UINT i = 0; i < m_IndexBuffer.size(); i += 3)
            {
                auto& index0 = m_IndexBuffer[i];
                auto& index1 = m_IndexBuffer[i+1];
                auto& index2 = m_IndexBuffer[i+2];
                XMFLOAT3& v0 = m_VertexBuffer[index0].Position.xmFloat3;
                XMFLOAT3& v1 = m_VertexBuffer[index1].Position.xmFloat3;
                XMFLOAT3& v2 = m_VertexBuffer[index2].Position.xmFloat3;

                XMFLOAT2& uv0 = m_VertexBuffer[index0].UV.xmFloat2;
                XMFLOAT2& uv1 = m_VertexBuffer[index1].UV.xmFloat2;
                XMFLOAT2& uv2 = m_VertexBuffer[index2].UV.xmFloat2;

                Vector3& tangent1 = m_VertexBuffer[index0].Tangent;
                Vector3& tangent2 = m_VertexBuffer[index1].Tangent;
                Vector3& tangent3 = m_VertexBuffer[index2].Tangent;

                XMVECTOR tangent = XMLoadFloat3(&CalculateTangent(v0, v1, v2, uv0, uv1, uv2));

                XMStoreFloat3(&tangent1.xmFloat3, tangent1.GetXMVECTOR() + tangent);
                XMStoreFloat3(&tangent2.xmFloat3, tangent2.GetXMVECTOR() + tangent);
                XMStoreFloat3(&tangent3.xmFloat3, tangent3.GetXMVECTOR() + tangent);
            }

            // Renormalize the tangents.
            for (auto& vertex : m_VertexBuffer)
            {
                XMStoreFloat3(&vertex.Tangent.xmFloat3, XMVector3Normalize(XMLoadFloat3(&vertex.Tangent.xmFloat3)));
            }
        }
    };

    struct AreaLight
    {
        AreaLight(Vector3 LightColor) : m_LightColor(LightColor) {}

        Mesh m_Mesh;
        Vector3 m_LightColor;
    };

    struct EnvironmentMap
    {
        EnvironmentMap() {}
        EnvironmentMap(const std::string &fileName) : m_FileName(fileName) {}
        std::string m_FileName;
    };

    struct Scene
    {
        Camera m_Camera;
        Film m_Film;
        std::unordered_map<std::string, Material> m_Materials;
        std::vector<AreaLight> m_AreaLights;
        std::vector<Mesh> m_Meshes;
        EnvironmentMap m_EnvironmentMap;
		XMMATRIX m_transform;
    };

    class BadFormatException : public std::exception
    {
    public:
        BadFormatException(char const* const errorMessage) : std::exception(errorMessage) {}
    };

    class SceneParserClass
    {
        virtual void Parse(std::string filename, Scene &outputScene, bool bClockwiseWindingORder = true, bool rhCoords = false) = 0;
    };
};
