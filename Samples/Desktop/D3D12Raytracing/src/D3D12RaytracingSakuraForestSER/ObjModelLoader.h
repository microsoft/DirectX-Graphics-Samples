//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED AS IS WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once
#include "RaytracingHlslCompat.h"
#include <optional>


class ObjModelLoader
{
public:
	void Load(wchar_t const* fileName);

	void GetObjectVerticesAndIndices(
		std::string const& name,
		float scale,
		std::vector<Vertex>* vertices,
		std::vector<Index>* indices);

private:
	class Object
	{
	public:
		Object(std::string const& name)
			: m_name(name)
		{
		}

		std::string const& GetName() const { return m_name; }

		struct FaceElementIndices
		{
			unsigned int Values[3];
		};

		struct Face
		{
			FaceElementIndices VertexIndices;

			bool UseNormals;
			bool UseTexCoords;
			FaceElementIndices NormalIndices;
			FaceElementIndices TexCoordIndices;
		};

		void AddObjFace(const FaceElementIndices& vertexIndices,
			const std::optional<FaceElementIndices>& normalIndices = std::nullopt,
			const std::optional<FaceElementIndices>& texCoordIndices = std::nullopt)
		{
			Face face;
			face.VertexIndices = vertexIndices;

			if (normalIndices) {
				face.UseNormals = true;
				face.NormalIndices = *normalIndices;
			}
			else {
				face.UseNormals = false;
			}

			if (texCoordIndices) {
				face.UseTexCoords = true;
				face.TexCoordIndices = *texCoordIndices;
			}
			else {
				face.UseTexCoords = false;
			}

			m_faces.push_back(face);
		}


		std::vector<XMFLOAT3> m_vertices;
		std::vector<XMFLOAT3> m_normals;
		std::vector<XMFLOAT2> m_texCoords;
		std::vector<Face> m_faces;

	private:
		std::string m_name;
	};

	std::vector<Object> m_objects;
	Object* GetObject(std::string const& name);
	Object* FindOrCreateObject(std::string const& name);
};