//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author(s):	Alex Nankervis
//

#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Graphics
{

bool Model::LoadAssimp(const char *filename)
{
	Assimp::Importer importer;

	// remove unused data
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, 
		aiComponent_COLORS | aiComponent_LIGHTS | aiComponent_CAMERAS);

	// max triangles and vertices per mesh, splits above this threshold
	importer.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, INT_MAX);
	importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, 0xfffe); // avoid the primitive restart index

	// remove points and lines
	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);

	const aiScene *scene = importer.ReadFile(filename,
		aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices |
		aiProcess_Triangulate |
		aiProcess_RemoveComponent |
		aiProcess_GenSmoothNormals |
		aiProcess_SplitLargeMeshes |
		aiProcess_ValidateDataStructure |
		//aiProcess_ImproveCacheLocality | // handled by optimizePostTransform()
		aiProcess_RemoveRedundantMaterials |
		aiProcess_SortByPType |
		aiProcess_FindInvalidData |
		aiProcess_GenUVCoords |
		aiProcess_TransformUVCoords |
		aiProcess_OptimizeMeshes |
		aiProcess_OptimizeGraph);

	if (scene == nullptr)
		return false;

	if (scene->HasTextures())
	{
		// embedded textures...
	}

	if (scene->HasAnimations())
	{
		// todo
	}

	m_Header.materialCount = scene->mNumMaterials;
	m_pMaterial = new Material [m_Header.materialCount];
	memset(m_pMaterial, 0, sizeof(Material) * m_Header.materialCount);
	for (unsigned int materialIndex = 0; materialIndex < scene->mNumMaterials; materialIndex++)
	{
		const aiMaterial *srcMat = scene->mMaterials[materialIndex];
		Material *dstMat = m_pMaterial + materialIndex;

		aiColor3D diffuse(1.0f, 1.0f, 1.0f);
		aiColor3D specular(1.0f, 1.0f, 1.0f);
		aiColor3D ambient(1.0f, 1.0f, 1.0f);
		aiColor3D emissive(0.0f, 0.0f, 0.0f);
		aiColor3D transparent(1.0f, 1.0f, 1.0f);
		float opacity = 1.0f;
		float shininess = 0.0f;
		float specularStrength = 1.0f;
		aiString texDiffusePath;
		aiString texSpecularPath;
		aiString texEmissivePath;
		aiString texNormalPath;
		aiString texLightmapPath;
		aiString texReflectionPath;
		srcMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
		srcMat->Get(AI_MATKEY_COLOR_SPECULAR, specular);
		srcMat->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
		srcMat->Get(AI_MATKEY_COLOR_EMISSIVE, emissive);
		srcMat->Get(AI_MATKEY_COLOR_TRANSPARENT, transparent);
		srcMat->Get(AI_MATKEY_OPACITY, opacity);
		srcMat->Get(AI_MATKEY_SHININESS, shininess);
		srcMat->Get(AI_MATKEY_SHININESS_STRENGTH, specularStrength);
		srcMat->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texDiffusePath);
		srcMat->Get(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0), texSpecularPath);
		srcMat->Get(AI_MATKEY_TEXTURE(aiTextureType_EMISSIVE, 0), texEmissivePath);
		srcMat->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), texNormalPath);
		srcMat->Get(AI_MATKEY_TEXTURE(aiTextureType_LIGHTMAP, 0), texLightmapPath);
		srcMat->Get(AI_MATKEY_TEXTURE(aiTextureType_REFLECTION, 0), texReflectionPath);

		dstMat->diffuse = Vector3(diffuse.r, diffuse.g, diffuse.b);
		dstMat->specular = Vector3(specular.r, specular.g, specular.b);
		dstMat->ambient = Vector3(ambient.r, ambient.g, ambient.b);
		dstMat->emissive = Vector3(emissive.r, emissive.g, emissive.b);
		dstMat->transparent = Vector3(transparent.r, transparent.g, transparent.b);
		dstMat->opacity = opacity;
		dstMat->shininess = shininess;
		dstMat->specularStrength = specularStrength;

		char *pRem = nullptr;

		strncpy_s(dstMat->texDiffusePath, "models/", Material::maxTexPath - 1);
		strncat_s(dstMat->texDiffusePath, texDiffusePath.C_Str(), Material::maxTexPath - 1);
		pRem = strrchr(dstMat->texDiffusePath, '.');
		while (pRem != nullptr && *pRem != 0) *(pRem++) = 0; // remove extension

		strncpy_s(dstMat->texSpecularPath, "models/", Material::maxTexPath - 1);
		strncat_s(dstMat->texSpecularPath, texSpecularPath.C_Str(), Material::maxTexPath - 1);
		pRem = strrchr(dstMat->texSpecularPath, '.');
		while (pRem != nullptr && *pRem != 0) *(pRem++) = 0; // remove extension

		strncpy_s(dstMat->texEmissivePath, "models/", Material::maxTexPath - 1);
		strncat_s(dstMat->texEmissivePath, texEmissivePath.C_Str(), Material::maxTexPath - 1);
		pRem = strrchr(dstMat->texEmissivePath, '.');
		while (pRem != nullptr && *pRem != 0) *(pRem++) = 0; // remove extension

		strncpy_s(dstMat->texNormalPath, "models/", Material::maxTexPath - 1);
		strncat_s(dstMat->texNormalPath, texNormalPath.C_Str(), Material::maxTexPath - 1);
		pRem = strrchr(dstMat->texNormalPath, '.');
		while (pRem != nullptr && *pRem != 0) *(pRem++) = 0; // remove extension

		strncpy_s(dstMat->texLightmapPath, "models/", Material::maxTexPath - 1);
		strncat_s(dstMat->texLightmapPath, texLightmapPath.C_Str(), Material::maxTexPath - 1);
		pRem = strrchr(dstMat->texLightmapPath, '.');
		while (pRem != nullptr && *pRem != 0) *(pRem++) = 0; // remove extension

		strncpy_s(dstMat->texReflectionPath, "models/", Material::maxTexPath - 1);
		strncat_s(dstMat->texReflectionPath, texReflectionPath.C_Str(), Material::maxTexPath - 1);
		pRem = strrchr(dstMat->texReflectionPath, '.');
		while (pRem != nullptr && *pRem != 0) *(pRem++) = 0; // remove extension

		aiString matName;
		srcMat->Get(AI_MATKEY_NAME, matName);
		strncpy_s(dstMat->name, matName.C_Str(), Material::maxMaterialName - 1);
	}

	m_Header.meshCount = scene->mNumMeshes;
	m_pMesh = new Mesh [m_Header.meshCount];
	memset(m_pMesh, 0, sizeof(Mesh) * m_Header.meshCount);
	// first pass, count everything
	for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
	{
		const aiMesh *srcMesh = scene->mMeshes[meshIndex];
		Mesh *dstMesh = m_pMesh + meshIndex;

		assert(srcMesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE);

		dstMesh->materialIndex = srcMesh->mMaterialIndex;

		// just store everything as float. Can quantize in Model::optimize()
		dstMesh->attribsEnabled |= attrib_mask_position;
		dstMesh->attrib[attrib_position].offset = dstMesh->vertexStride;
		dstMesh->attrib[attrib_position].normalized = 0;
		dstMesh->attrib[attrib_position].components = 3;
		dstMesh->attrib[attrib_position].format = attrib_format_float;
		dstMesh->vertexStride += sizeof(float) * 3;

		dstMesh->attribsEnabled |= attrib_mask_texcoord0;
		dstMesh->attrib[attrib_texcoord0].offset = dstMesh->vertexStride;
		dstMesh->attrib[attrib_texcoord0].normalized = 0;
		dstMesh->attrib[attrib_texcoord0].components = 2;
		dstMesh->attrib[attrib_texcoord0].format = attrib_format_float;
		dstMesh->vertexStride += sizeof(float) * 2;

		dstMesh->attribsEnabled |= attrib_mask_normal;
		dstMesh->attrib[attrib_normal].offset = dstMesh->vertexStride;
		dstMesh->attrib[attrib_normal].normalized = 0;
		dstMesh->attrib[attrib_normal].components = 3;
		dstMesh->attrib[attrib_normal].format = attrib_format_float;
		dstMesh->vertexStride += sizeof(float) * 3;

		dstMesh->attribsEnabled |= attrib_mask_tangent;
		dstMesh->attrib[attrib_tangent].offset = dstMesh->vertexStride;
		dstMesh->attrib[attrib_tangent].normalized = 0;
		dstMesh->attrib[attrib_tangent].components = 3;
		dstMesh->attrib[attrib_tangent].format = attrib_format_float;
		dstMesh->vertexStride += sizeof(float) * 3;

		dstMesh->attribsEnabled |= attrib_mask_bitangent;
		dstMesh->attrib[attrib_bitangent].offset = dstMesh->vertexStride;
		dstMesh->attrib[attrib_bitangent].normalized = 0;
		dstMesh->attrib[attrib_bitangent].components = 3;
		dstMesh->attrib[attrib_bitangent].format = attrib_format_float;
		dstMesh->vertexStride += sizeof(float) * 3;

		// depth-only
		dstMesh->attribsEnabledDepth |= attrib_mask_position;
		dstMesh->attribDepth[attrib_position].offset = dstMesh->vertexStrideDepth;
		dstMesh->attribDepth[attrib_position].normalized = 0;
		dstMesh->attribDepth[attrib_position].components = 3;
		dstMesh->attribDepth[attrib_position].format = attrib_format_float;
		dstMesh->vertexStrideDepth += sizeof(float) * 3;

		// color rendering
		dstMesh->vertexDataByteOffset = m_Header.vertexDataByteSize;
		dstMesh->vertexCount = srcMesh->mNumVertices;

		dstMesh->indexDataByteOffset = m_Header.indexDataByteSize;
		dstMesh->indexCount = srcMesh->mNumFaces * 3;

		m_Header.vertexDataByteSize += dstMesh->vertexStride * dstMesh->vertexCount;
		m_Header.indexDataByteSize += sizeof(uint16_t) * dstMesh->indexCount;

		// depth-only rendering
		dstMesh->vertexDataByteOffsetDepth = m_Header.vertexDataByteSizeDepth;
		dstMesh->vertexCountDepth = srcMesh->mNumVertices;

		m_Header.vertexDataByteSizeDepth += dstMesh->vertexStrideDepth * dstMesh->vertexCountDepth;
	}
	// allocate storage
	m_pVertexData = new unsigned char [m_Header.vertexDataByteSize];
	m_pIndexData = new unsigned char [m_Header.indexDataByteSize];
	m_pVertexDataDepth = new unsigned char [m_Header.vertexDataByteSizeDepth];
	m_pIndexDataDepth = new unsigned char [m_Header.indexDataByteSize];
	// second pass, fill in vertex and index data
	for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
	{
		const aiMesh *srcMesh = scene->mMeshes[meshIndex];
		Mesh *dstMesh = m_pMesh + meshIndex;

		float *dstPos = (float*)(m_pVertexData + dstMesh->vertexDataByteOffset + dstMesh->attrib[attrib_position].offset);
		float *dstTexcoord0 = (float*)(m_pVertexData + dstMesh->vertexDataByteOffset + dstMesh->attrib[attrib_texcoord0].offset);
		float *dstNormal = (float*)(m_pVertexData + dstMesh->vertexDataByteOffset + dstMesh->attrib[attrib_normal].offset);
		float *dstTangent = (float*)(m_pVertexData + dstMesh->vertexDataByteOffset + dstMesh->attrib[attrib_tangent].offset);
		float *dstBitangent = (float*)(m_pVertexData + dstMesh->vertexDataByteOffset + dstMesh->attrib[attrib_bitangent].offset);

		float *dstPosDepth = (float*)(m_pVertexDataDepth + dstMesh->vertexDataByteOffsetDepth + dstMesh->attribDepth[attrib_position].offset);

		for (unsigned int v = 0; v < dstMesh->vertexCount; v++)
		{
			if (srcMesh->mVertices)
			{
				dstPos[0] = srcMesh->mVertices[v].x;
				dstPos[1] = srcMesh->mVertices[v].y;
				dstPos[2] = srcMesh->mVertices[v].z;

				dstPosDepth[0] = srcMesh->mVertices[v].x;
				dstPosDepth[1] = srcMesh->mVertices[v].y;
				dstPosDepth[2] = srcMesh->mVertices[v].z;
			}
			else
			{
				// no position? That's kind of bad.
				assert(0);
			}
			dstPos = (float*)((unsigned char*)dstPos + dstMesh->vertexStride);
			dstPosDepth = (float*)((unsigned char*)dstPosDepth + dstMesh->vertexStrideDepth);

			if (srcMesh->mTextureCoords[0])
			{
				dstTexcoord0[0] = srcMesh->mTextureCoords[0][v].x;
				dstTexcoord0[1] = srcMesh->mTextureCoords[0][v].y;
			}
			else
			{
				dstTexcoord0[0] = 0.0f;
				dstTexcoord0[1] = 0.0f;
			}
			dstTexcoord0 = (float*)((unsigned char*)dstTexcoord0 + dstMesh->vertexStride);

			if (srcMesh->mNormals)
			{
				dstNormal[0] = srcMesh->mNormals[v].x;
				dstNormal[1] = srcMesh->mNormals[v].y;
				dstNormal[2] = srcMesh->mNormals[v].z;
			}
			else
			{
				// Assimp should generate normals if they are missing, according to the postprocessing flag specified on load,
				// so we should never get here.
				assert(0);
			}
			dstNormal = (float*)((unsigned char*)dstNormal + dstMesh->vertexStride);

			if (srcMesh->mTangents)
			{
				dstTangent[0] = srcMesh->mTangents[v].x;
				dstTangent[1] = srcMesh->mTangents[v].y;
				dstTangent[2] = srcMesh->mTangents[v].z;
			}
			else
			{
				// TODO: generate tangents/bitangents if missing
				dstTangent[0] = 1.0f;
				dstTangent[1] = 0.0f;
				dstTangent[2] = 0.0f;
			}
			dstTangent = (float*)((unsigned char*)dstTangent + dstMesh->vertexStride);

			if (srcMesh->mBitangents)
			{
				dstBitangent[0] = srcMesh->mBitangents[v].x;
				dstBitangent[1] = srcMesh->mBitangents[v].y;
				dstBitangent[2] = srcMesh->mBitangents[v].z;
			}
			else
			{
				// TODO: generate tangents/bitangents if missing
				dstBitangent[0] = 0.0f;
				dstBitangent[1] = 1.0f;
				dstBitangent[2] = 0.0f;
			}
			dstBitangent = (float*)((unsigned char*)dstBitangent + dstMesh->vertexStride);
		}

		uint16_t *dstIndex = (uint16_t*)(m_pIndexData + dstMesh->indexDataByteOffset);
		uint16_t *dstIndexDepth = (uint16_t*)(m_pIndexDataDepth + dstMesh->indexDataByteOffset);
		for (unsigned int f = 0; f < srcMesh->mNumFaces; f++)
		{
			assert(srcMesh->mFaces[f].mNumIndices == 3);

			*dstIndex++ = srcMesh->mFaces[f].mIndices[0];
			*dstIndex++ = srcMesh->mFaces[f].mIndices[1];
			*dstIndex++ = srcMesh->mFaces[f].mIndices[2];

			*dstIndexDepth++ = srcMesh->mFaces[f].mIndices[0];
			*dstIndexDepth++ = srcMesh->mFaces[f].mIndices[1];
			*dstIndexDepth++ = srcMesh->mFaces[f].mIndices[2];
		}
	}

	ComputeAllBoundingBoxes();

	return true;
}

} // namespace Graphics
