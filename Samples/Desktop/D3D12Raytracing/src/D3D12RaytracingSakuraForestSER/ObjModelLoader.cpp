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

#include "stdafx.h"
#include "ObjModelLoader.h"
#include <iostream>


void ObjModelLoader::Load(wchar_t const* fileName)
{
	std::ifstream openedFile(fileName);
	std::string line;
	Object* currentObject = nullptr;

	while (std::getline(openedFile, line))
	{
		if (line.empty()) continue;

		if (line._Starts_with("o ")) {
			std::string objName = line.substr(2);
			std::cout << "Loaded object: " << objName << std::endl;
			currentObject = FindOrCreateObject(line.substr(2));
			
			continue;
		}

		if (line._Starts_with("vt ")) {
			XMFLOAT2 texCoord;
			std::stringstream ss(line.substr(3));
			ss >> texCoord.x >> texCoord.y;
			
            if (currentObject != nullptr) {
                currentObject->m_texCoords.push_back(texCoord);
            } else {
                std::cerr << "Error: currentObject is null." << std::endl;
            }
			continue;
		}

		if (line._Starts_with("vn ")) {
			XMFLOAT3 normal;
			std::stringstream ss(line.substr(2));
			ss >> normal.x >> normal.y >> normal.z;
			if (currentObject != nullptr) {
				currentObject->m_normals.push_back(normal);
			}
			else {
				std::cerr << "Error: currentObject is null." << std::endl;
			}
			continue;
		}

		if (line._Starts_with("v ")) {
			XMFLOAT3 vertex;
			std::stringstream ss(line.substr(1));
			ss >> vertex.x >> vertex.y >> vertex.z;
			if (currentObject != nullptr) {
				currentObject->m_vertices.push_back(vertex);
			}
			else {
				std::cerr << "Error: currentObject is null." << std::endl;
			}
			continue;
		}

		if (line._Starts_with("f "))
		{
			std::string tokens[3];
			std::stringstream stringStream(line.substr(2)); 
			stringStream >> tokens[0] >> tokens[1] >> tokens[2];

			Object::FaceElementIndices v{}, n{}, t{};
			bool hasNormals = false, hasTexCoords = false;

			for (int i = 0; i < 3; ++i)
			{
				std::string& token = tokens[i];
				size_t s1 = token.find('/');
				size_t s2 = token.find('/', s1 + 1);


				v.Values[i] = std::stoi(token.substr(0, s1));

				if (s1 != std::string::npos && s2 != std::string::npos)
				{
					std::string texCoordStr = token.substr(s1 + 1, s2 - s1 - 1);
					std::string normalStr = token.substr(s2 + 1);

					if (!texCoordStr.empty())
					{
						t.Values[i] = std::stoi(texCoordStr);
						hasTexCoords = true;
					}

					if (!normalStr.empty())
					{
						n.Values[i] = std::stoi(normalStr);
						hasNormals = true;
					}
				}
				else if (s1 != std::string::npos)
				{
					std::string texCoordStr = token.substr(s1 + 1);
					if (!texCoordStr.empty())
					{
						t.Values[i] = std::stoi(texCoordStr);
						hasTexCoords = true;
					}
				}
			}

			if (hasNormals && hasTexCoords)
			{
				currentObject->AddObjFace(v, n, t);
			}
			else if (hasNormals)
			{
				currentObject->AddObjFace(v, n);
			}
			else
			{
				currentObject->AddObjFace(v);
			}
		}


	}
}

ObjModelLoader::Object* ObjModelLoader::GetObject(std::string const& name)
{
	for (auto& obj : m_objects) 
	{
		if (obj.GetName() == name)
			return &obj;
	}

	return nullptr;
}

ObjModelLoader::Object* ObjModelLoader::FindOrCreateObject(std::string const& name)
{
	if (auto* obj = GetObject(name))
		return obj;
	else
	{
		m_objects.push_back(Object(name));
	}
	return &m_objects.back();
}


void ObjModelLoader::GetObjectVerticesAndIndices(
	std::string const& name,
	float scale,
	std::vector<Vertex>* vertices,
	std::vector<Index>* indices)
{
	size_t baseIndex = vertices->size();
	Object* obj = GetObject(name);
	if (!obj) {
		throw std::runtime_error("Object '" + name + "' not found in OBJ file.");
	}


	for (const auto& face : obj->m_faces) 
	{
		for (int vertexIndex = 0; vertexIndex < 3; ++vertexIndex)
		{
			Vertex v{};

			// Get vertex position
			Index index = face.VertexIndices.Values[vertexIndex] - 1; // 1-indexed
			if (index >= obj->m_vertices.size()) {
				throw std::runtime_error("Vertex index out of bounds.");
			}

			v.position = obj->m_vertices.at(index);
			v.position.x *= scale;
			v.position.y *= scale;
			v.position.z *= scale;

			// Get normal if available
			if (face.UseNormals)
			{
				uint32_t normalIndex = face.NormalIndices.Values[vertexIndex] - 1;
				v.normal = obj->m_normals[normalIndex];
			}

			// Get texture coordinate if available
			if (face.UseTexCoords)
			{
				uint32_t texIndex = face.TexCoordIndices.Values[vertexIndex] - 1;
				XMFLOAT2 texCoord = obj->m_texCoords[texIndex];
				v.uv.x = texCoord.x;
				v.uv.y = texCoord.y;

			}
			else
			{
				// Default UV
				v.uv.x = 0.5f;
				v.uv.y = 0.5f;
			}

			indices->push_back(static_cast<Index>(vertices->size()));
			vertices->push_back(v);
		}
	}
}

