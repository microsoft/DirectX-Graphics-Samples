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
// Original implementation: https://github.com/wallisc/DuosRenderer/tree/DXRRenderer/PBRTParser
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
	};

	struct Vector3
	{
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
		};
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

	struct Material
	{
		std::string m_MaterialName;
		Vector3 m_Diffuse;
		Vector3 m_Specular;
		float m_URoughness;
		float m_VRoughness;
		std::string m_DiffuseTextureFilename;
		std::string m_SpecularTextureFilename;
	};

	struct Vertex
	{
		Vector3 Normal;
		Vector3 Position;
		Vector3 Tangents;
		Vector2 UV;
	};

	typedef UINT Index;

    struct Mesh
    {
        Material *m_pMaterial;
        std::vector<Index> m_IndexBuffer;
        std::vector<Vertex> m_VertexBuffer;

        bool m_AreTangentsValid;
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
    };

    class BadFormatException : public std::exception
    {
    public:
        BadFormatException(char const* const errorMessage) : std::exception(errorMessage) {}
    };

    class SceneParserClass
    {
        virtual void Parse(std::string filename, Scene &outputScene) = 0;
    };
};
