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
namespace PlyParser
{
    class PlyParser
    {
    public:
        void Parse(const std::string &filename, SceneParser::Mesh &mesh);
        void ParseHeader();
        void ParseBody(SceneParser::Mesh &mesh);
    private:
        UINT8 BytesPerIntegerType(std::string type);
        UINT32 ParseVariableInteger(UINT8 bytePerInteger, void *pData);

        std::ifstream m_fileStream;
        std::string lastParsedWord;

        enum ElementType
        {
            POSITION,
            NORMAL,
            TEXTURE
        };

        enum ElementIndex
        {
            U = 0,
            V = 1,
            X = 0,
            Y = 1,
            Z = 2,
        };

        UINT8 m_BytesPerVertexCount;
        UINT8 m_BytesPerIndex;

        typedef std::pair<ElementType, ElementIndex> Element;
        std::vector<Element> m_elementLayout;
        UINT m_numFaces;
        UINT m_numVertices;
    };
}