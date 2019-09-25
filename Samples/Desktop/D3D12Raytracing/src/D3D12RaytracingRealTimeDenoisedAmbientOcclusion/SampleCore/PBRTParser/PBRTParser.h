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
#include "SceneParser.h"

#define PBRTPARSER_STRINGBUFFERSIZE 200

namespace PBRTParser
{
struct AreaLightAttribute
{
    SceneParser::Vector3 m_lightColor;
};

class Attributes
{
public:
    typedef enum _ObjectType
    {
        Normal,
        AreaLight
    } ObjectType;

    Attributes()
    {
        m_Type = Normal;
    }

    Attributes(const Attributes &attributes)
    {
        memcpy(this, &attributes, sizeof(*this));
    }

    Attributes operator=(const Attributes &attributes)
    {
        return Attributes(attributes);
    }

    Attributes(AreaLightAttribute &attribute)
    {
        m_Type = AreaLight;
        m_areaLightAttribute = attribute;
    }

    ObjectType GetType() { return m_Type; }
    AreaLightAttribute &GetAreaLightAttribute() 
    { 
        assert(GetType() == AreaLight);
        return m_areaLightAttribute;
    }

private:
    ObjectType m_Type;
    union
    {
        AreaLightAttribute m_areaLightAttribute;
    };
};

class PBRTParser : public SceneParser::SceneParserClass
{
    public:
        PBRTParser();
        ~PBRTParser();
        virtual void Parse(std::string filename, SceneParser::Scene &outputScene, bool bClockwiseWindingORder = true, bool rhCoords = false);

    private:
        void ParseFilm(std::ifstream &fileStream, SceneParser::Scene &outputScene);
        void ParseLookAt(std::ifstream &fileStream, SceneParser::Scene &outputScene);
		void ParseCamera(std::ifstream &fileStream, SceneParser::Scene &outputScene);
        void ParseWorld(std::ifstream &fileStream, SceneParser::Scene &outputScene);
        void ParseMaterial(std::ifstream &fileStream, SceneParser::Scene &outputScene);
        void ParseMesh(std::ifstream &fileStream, SceneParser::Scene &outputScene);
        void ParseTexture(std::ifstream &fileStream, SceneParser::Scene &outputScene);
        void ParseLightSource(std::ifstream &fileStream, SceneParser::Scene &outputScene);
        void ParseAreaLightSource(std::ifstream &fileStream, SceneParser::Scene &outputScene);
        void ParseTransform();

        void ParseShape(std::ifstream &fileStream, SceneParser::Scene &outputScene, SceneParser::Mesh &mesh);

        void ParseBracketedVector3(std::istream, float &x, float &y, float &z);

		void SetWindingOrder(bool bSetClockwiseOrder, SceneParser::Scene &scene);
		void SwapGeometryCoordinateSystem(SceneParser::Scene &scene);
		void FixZeroVertexNormals(SceneParser::Scene &scene);
        void InitializeDefaults(SceneParser::Scene &outputScene);
        void InitializeCameraDefaults(SceneParser::Camera &camera);

        static std::string CorrectNameString(const char *pString);
        static std::string CorrectNameString(const std::string &str);

        void GetTempCharBuffer(char **ppBuffer, size_t &charBufferSize)
        {
            *ppBuffer = _m_buffer;
            charBufferSize = ARRAYSIZE(_m_buffer);
        };

        char *GetLine()
        {
            char *pTempBuffer;
            size_t bufferSize;
            GetTempCharBuffer(&pTempBuffer, bufferSize);

            m_fileStream.getline(pTempBuffer, bufferSize);

            lastParsedWord = "";
            return pTempBuffer;
        }

        std::stringstream GetLineStream()
        {
            char *pTempBuffer;
            size_t bufferSize;
            GetTempCharBuffer(&pTempBuffer, bufferSize);

            m_fileStream.getline(pTempBuffer, bufferSize);

            return std::stringstream(std::string(pTempBuffer));
        }


        static SceneParser::Vector3 ConvertToVector3(const XMVECTOR &_vec)
        {
			XMFLOAT3 vec;
			XMStoreFloat3(&vec, _vec);
            return SceneParser::Vector3(vec.x, vec.y, vec.z);
        }

        Attributes &GetCurrentAttributes()
        {
            return m_AttributeStack.top();
        }
        
        void SetCurrentAttributes(const Attributes &attritbutes)
        {
            m_AttributeStack.top() = attritbutes;
        }

        float ParseFloat1(std::istream &inStream);
        bool ParseBool(std::istream &inStream);
        std::string ParseString(std::istream &inStream);
        void ParseExpectedWords(std::istream &inStream, _In_reads_(numWords) std::string *pWords, UINT numWords);
        void ParseExpectedWord(std::istream &inStream, const std::string &word);

        std::string GenerateCheckerboardTexture(std::string fileName, float uScale, float vScale, SceneParser::Vector3 color1, SceneParser::Vector3 color2);
        void GenerateBMPFile(std::string fileName, _In_reads_(width * height)SceneParser::Vector3 *pDmageData, UINT width, UINT height);

        std::ifstream m_fileStream;
        std::string m_CurrentMaterial;
        std::stack<Attributes> m_AttributeStack;
        std::unordered_map<std::string, std::string> m_TextureNameToFileName;

        XMMATRIX m_currentTransform;

        // Shouldn't be accessed directly outside of GetTempCharBuffer
        char _m_buffer[500];
        std::string lastParsedWord;
        std::string m_relativeDirectory;
    };
}
