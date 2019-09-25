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

#include "../../stdafx.h"
#include <memory>
#include <algorithm>
#include "SceneParser.h"
#include "PlyParser.h"

using namespace SceneParser;
using namespace std;

namespace PlyParser
{
void ThrowIfTrue(bool expression, string errorMessage = "")
{
    if (expression)
    {
        throw new BadFormatException(errorMessage.c_str());
    }
}

UINT8 PlyParser::BytesPerIntegerType(string type)
{
    if (!type.compare("uint8") || !type.compare("uchar"))
    {
        return 1;
    }
    else if (!type.compare("uint") || !type.compare("int"))
    {
        return 4;
    }
    else
    {
        ThrowIfTrue(true, "Integer type not implemented");
        return 0;
    }
}

void PlyParser::ParseHeader()
{
    m_fileStream >> lastParsedWord;
    ThrowIfTrue(lastParsedWord.compare("ply"), "First word in ply file expect to be \'Ply\'");
    
    while (m_fileStream.good())
    {
        if (!lastParsedWord.compare("end_header"))
        {
            char tempBuffer[50];
            m_fileStream.getline(tempBuffer, ARRAYSIZE(tempBuffer)); // Consume the last newline character;
            break;
        }
        else if (!lastParsedWord.compare("property"))
        {
            m_fileStream >> lastParsedWord;
            if (!lastParsedWord.compare("list"))
            {
                m_fileStream >> lastParsedWord;
                m_BytesPerVertexCount = BytesPerIntegerType(lastParsedWord);
                m_fileStream >> lastParsedWord;
                m_BytesPerIndex = BytesPerIntegerType(lastParsedWord);
            }
            else if (!lastParsedWord.compare("float"))
            {
                m_fileStream >> lastParsedWord;
                if (!lastParsedWord.compare("x"))
                {
                    m_elementLayout.push_back(Element(POSITION, X));
                }
                else if (!lastParsedWord.compare("y"))
                {
                    m_elementLayout.push_back(Element(POSITION, Y));
                }
                else if (!lastParsedWord.compare("z"))
                {
                    m_elementLayout.push_back(Element(POSITION, Z));
                }
                else if (!lastParsedWord.compare("nx"))
                {
                    m_elementLayout.push_back(Element(NORMAL, X));
                }
                else if (!lastParsedWord.compare("ny"))
                {
                    m_elementLayout.push_back(Element(NORMAL, Y));
                }
                else if (!lastParsedWord.compare("nz"))
                {
                    m_elementLayout.push_back(Element(NORMAL, Z));
                }
                else if (!lastParsedWord.compare("u"))
                {
                    m_elementLayout.push_back(Element(TEXTURE, U));
                }
                else if (!lastParsedWord.compare("v"))
                {
                    m_elementLayout.push_back(Element(TEXTURE, V));
                }
            }
        }
        else if (!lastParsedWord.compare("element"))
        {
            m_fileStream >> lastParsedWord;
            if(!lastParsedWord.compare("face"))
            {
                m_fileStream >> m_numFaces;
            }
            else if (!lastParsedWord.compare("vertex"))
            {
                m_fileStream >> m_numVertices;
            }
        }
        else
        {
            m_fileStream >> lastParsedWord;
        }
    }
}

UINT32 PlyParser::ParseVariableInteger(UINT8 bytePerInteger, void *pData)
{
    switch (bytePerInteger)
    {
    case 1:
    {
        return (UINT32)(*(UINT8*)pData);
    }
    case 4:
    {
        return (*(UINT32*)pData);
    }
    default:
        ThrowIfTrue(true, "Unimplemented integer type");
        return 0;
    }
}

void PlyParser::ParseBody(SceneParser::Mesh &mesh)
{
    ThrowIfTrue(m_fileStream.fail());
    ThrowIfTrue(m_fileStream.eof());
    mesh.m_VertexBuffer.resize(m_numVertices);
	mesh.m_IndexBuffer.reserve(3 * m_numFaces);

    const UINT vertexSize = static_cast<UINT>(m_elementLayout.size() * sizeof(float));
    const UINT verticesPerBuffer = 150;
    const UINT bufferSize = vertexSize * verticesPerBuffer;
    unique_ptr<char[]> pLineBuffer = unique_ptr<char[]>(new char[bufferSize]);
    char *pIterator = pLineBuffer.get();
    
    for (UINT vertex = 0; vertex < m_numVertices; vertex++)
    {
        if (vertex % verticesPerBuffer == 0)
        {
            UINT verticesLeft = m_numVertices - vertex;
            UINT readSize = min(verticesLeft * vertexSize, bufferSize);

            m_fileStream.read(pLineBuffer.get(), readSize);
            ThrowIfTrue(m_fileStream.eof());
            ThrowIfTrue(m_fileStream.fail(), "Failed to open a file");

            pIterator = pLineBuffer.get();
        }

        SceneParser::Vertex &v = mesh.m_VertexBuffer[vertex];

        for (auto element : m_elementLayout)
        {
            float* pValue = nullptr;
            switch (element.first)
            {
            case POSITION:
                pValue = &v.Position[element.second];
                break;
            case NORMAL:
                pValue = &v.Normal[element.second];
                break;
            case TEXTURE:
                pValue = &v.UV[element.second];
                break;
            }

            *pValue = *(float *)pIterator;
            pIterator += sizeof(float);
        }
    }

    const UINT faceSize = m_BytesPerVertexCount + 3 * m_BytesPerIndex;
    const UINT facesPerBuffer = bufferSize / faceSize;
    const UINT faceBufferSize = faceSize * facesPerBuffer;
    for (UINT face = 0; face < m_numFaces; face++)
    {
        if (face % facesPerBuffer == 0)
        {
            UINT facesLeft = m_numFaces - face;
            UINT readSize = min(facesLeft * faceSize, faceBufferSize);
            m_fileStream.read(pLineBuffer.get(), readSize);
            ThrowIfTrue(m_fileStream.fail(), "Failed to open a files");

            pIterator = pLineBuffer.get();
        }

        auto numIndicesPerFace = ParseVariableInteger(m_BytesPerVertexCount, pIterator);
        ThrowIfTrue(numIndicesPerFace != 3, "Not supporting non-triangle faces");
        pIterator += m_BytesPerVertexCount;

        for (UINT faceIndex = 0; faceIndex < numIndicesPerFace; faceIndex++)
        {
            mesh.m_IndexBuffer.push_back((int)ParseVariableInteger(m_BytesPerIndex, pIterator));
            pIterator += m_BytesPerIndex;
        }
    }

    mesh.GenerateTangents();
}

void PlyParser::Parse(const string &filename, SceneParser::Mesh &mesh)
{
    m_fileStream = ifstream(filename, ios::in | ios::binary);
    ThrowIfTrue(!m_fileStream.good(), "Failure opening file");

    ParseHeader();
    ParseBody(mesh);
}

}