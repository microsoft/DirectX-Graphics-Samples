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

#include "..\..\stdafx.h"
#include "PBRTParser.h"
#include "PlyParser.h"

using namespace SceneParser;
using namespace std;

#define TEAPOT_HACK 1 // ToDo remove

namespace PBRTParser
{
    void ThrowIfTrue(bool expression, std::string errorMessage = "")
    {
        if (expression)
        {
            throw new BadFormatException(errorMessage.c_str());
        }
    }

    PBRTParser::PBRTParser()
    {
        m_AttributeStack.push(Attributes());
    }

    PBRTParser::~PBRTParser()
    {
        m_AttributeStack.pop();
        assert(m_AttributeStack.size() == 0);
    }

    void PBRTParser::Parse(std::string filename, SceneParser::Scene &outputScene)
    {
        m_fileStream = ifstream(filename);
     
        {
            UINT relativeDirEnd = static_cast<UINT>(filename.find_last_of('\\'));
            m_relativeDirectory = filename.substr(0, relativeDirEnd + 1);
        }

        m_currentTransform = XMMatrixIdentity();

        if (!m_fileStream.good())
        {
            assert(false); // file not found
        }

        InitializeDefaults(outputScene);

        while (m_fileStream.good())
        {
            if (!lastParsedWord.compare("Film"))
            {
                ParseFilm(m_fileStream, outputScene);
            }
            else if (!lastParsedWord.compare("Camera"))
            {
                ParseCamera(m_fileStream, outputScene);
            }
            else if (!lastParsedWord.compare("Transform"))
            {
                ParseTransform();
            }
            else if (!lastParsedWord.compare("WorldBegin"))
            {
                m_currentTransform = XMMatrixIdentity();
                ParseWorld(m_fileStream, outputScene);
            }
            else
            {
                m_fileStream >> lastParsedWord;
            }
        }
    }

    void PBRTParser::ParseWorld(std::ifstream &fileStream, SceneParser::Scene &outputScene)
    {
        while (fileStream.good())
        {
            if (!lastParsedWord.compare("MakeNamedMaterial"))
            {
                ParseMaterial(fileStream, outputScene);
            }
            else if (!lastParsedWord.compare("NamedMaterial"))
            {
                fileStream >> m_CurrentMaterial;
                fileStream >> lastParsedWord;
            }
            else if (!lastParsedWord.compare("Shape"))
            {
                ParseMesh(fileStream, outputScene);
            }
            else if (!lastParsedWord.compare("Texture"))
            {
                ParseTexture(fileStream, outputScene);
            }
            else if (!lastParsedWord.compare("LightSource"))
            {
                ParseLightSource(fileStream, outputScene);
            }
            else if (!lastParsedWord.compare("AreaLightSource"))
            {
                ParseAreaLightSource(fileStream, outputScene);
            }
            else if (!lastParsedWord.compare("AttributeBegin"))
            {
                m_AttributeStack.push(Attributes());
                fileStream >> lastParsedWord;
            }
            else if (!lastParsedWord.compare("AttributeEnd"))
            {
                m_AttributeStack.pop();
                fileStream >> lastParsedWord;
            }
            else if (!lastParsedWord.compare("WorldEnd"))
            {
                break;
            }
            else
            {
                fileStream >> lastParsedWord;
            }
        }
    }

    void PBRTParser::ParseCamera(std::ifstream &fileStream, SceneParser::Scene &outputScene)
    {
        char *pTempBuffer = GetLine();

        UINT argCount = sscanf_s(pTempBuffer, " \"perspective\" \"float fov\" \[ %f \]",
            &outputScene.m_Camera.m_FieldOfView);

        ThrowIfTrue(argCount != 1, "Camera arguments not formatted correctly");

        auto pfnHomogenize = [](const XMVECTOR &vec4) { return vec4 / XMVectorGetW(vec4); };

        outputScene.m_Camera.m_LookAt =   ConvertToVector3(pfnHomogenize(XMVector3Transform(m_lookAt, m_currentTransform)));
        outputScene.m_Camera.m_Position = ConvertToVector3(pfnHomogenize(XMVector3Transform(m_camPos, m_currentTransform)));

#ifndef TEAPOT_HACK
        XMVECTOR normal = XMVector3Transform(m_camUp, m_currentTransform);
        outputScene.m_Camera.m_Up = ConvertToVector3(XMVector3Normalize(normal));
#endif
    }

    void PBRTParser::ParseFilm(std::ifstream &fileStream, SceneParser::Scene &outputScene)
    {
        char *pTempBuffer = GetLine();

        char fileName[PBRTPARSER_STRINGBUFFERSIZE];
        UINT argCount = sscanf_s(pTempBuffer, " \"image\" \"integer xresolution\" \[ %u \] \"integer yresolution\" \[ %u \] \"string filename\" \[ \"%s\" \]",
            &outputScene.m_Film.m_ResolutionX,
            &outputScene.m_Film.m_ResolutionY,
            fileName, ARRAYSIZE(fileName));

        ThrowIfTrue(argCount != 3, "Film arguments not formatted correctly");

        // Sometimes scanf pulls more than it needs to, make sure to clean up 
        // any extra characters on the file name
        string correctedFileName(fileName);
        if (correctedFileName[correctedFileName.size() - 1] == '"')
        {
            correctedFileName = correctedFileName.substr(0, correctedFileName.size() - 1);
        }

        outputScene.m_Film.m_Filename = correctedFileName;
    }

    void PBRTParser::ParseBracketedVector3(std::istream is, float &x, float &y, float &z)
    {
        is >> lastParsedWord;
        ThrowIfTrue(lastParsedWord.compare("["), "Expect '[' at beginning of vector");

        is >> x;
        is >> y;
        is >> z;

        ThrowIfTrue(!is.good());

        is >> lastParsedWord;
        ThrowIfTrue(lastParsedWord.compare("]"), "Expect '[' at beginning of vector");
    }

    float PBRTParser::ParseFloat1(std::istream &inStream)
    {
        ParseExpectedWord(inStream, "[");
        float num;
        inStream >> num;
        ParseExpectedWord(inStream, "]");
        return num;
    }

    std::string PBRTParser::ParseString(std::istream &inStream)
    {
        ParseExpectedWord(inStream, "[");
        std::string word;
        inStream >> word;
        ParseExpectedWord(inStream, "]");
        return word;
    }

    void PBRTParser::ParseMaterial(std::ifstream &fileStream, SceneParser::Scene &outputScene)
    {
        Material material;
        char materialName[PBRTPARSER_STRINGBUFFERSIZE];
        std::string materialType;

        auto lineStream = GetLineStream();

        lineStream >> lastParsedWord;
        material.m_MaterialName = CorrectNameString(lastParsedWord);

        auto pfnParseMaterialColor = [=](std::istream &inStream, Vector3 &color, std::string &textureFileName)
        {
            inStream >> lastParsedWord;
            ThrowIfTrue(lastParsedWord.compare("["));

            inStream >> color.r;
            if (inStream.good())
            {
                inStream >> color.g;
                inStream >> color.b;
            }
            else
            {
                inStream.clear();

                std::string textureName;
                inStream >> textureName;
                textureName = CorrectNameString(textureName);
                textureFileName = m_TextureNameToFileName[textureName];
                ThrowIfTrue(textureFileName.size() == 0);
            }

            inStream >> lastParsedWord;
            ThrowIfTrue(lastParsedWord.compare("]"));
        };

        while (lineStream.good())
        {
            if (!lastParsedWord.compare("\"string"))
            {
                lineStream >> lastParsedWord;
                if (!lastParsedWord.compare("type\""))
                {
                    lineStream >> lastParsedWord;
                    ThrowIfTrue(lastParsedWord.compare("["));

                    lineStream >> materialType;
                    materialType = CorrectNameString(materialType);

                    lineStream >> lastParsedWord;
                    ThrowIfTrue(lastParsedWord.compare("]"));
                }
                else
                {
                    ThrowIfTrue(false, "string not followed up with recognized token");
                }
            }
            else if (!lastParsedWord.compare("\"rgb") || !lastParsedWord.compare("\"texture"))
            {
                lineStream >> lastParsedWord;
                if (!lastParsedWord.compare("Kd\""))
                {
                    pfnParseMaterialColor(lineStream, material.m_Diffuse, material.m_DiffuseTextureFilename);
                }
                else if (!lastParsedWord.compare("Ks\""))
                {
                    pfnParseMaterialColor(lineStream, material.m_Specular, material.m_SpecularTextureFilename);
                }
            }
            else if (!lastParsedWord.compare("\"float"))
            {
                lineStream >> lastParsedWord;
                if (!lastParsedWord.compare("uroughness\""))
                {
                    material.m_URoughness = ParseFloat1(lineStream);
                }
                else if (!lastParsedWord.compare("vroughness\""))
                {
                    material.m_VRoughness = ParseFloat1(lineStream);
                }
            }
            else
            {
                lineStream >> lastParsedWord;
            }
        }
        outputScene.m_Materials[material.m_MaterialName] = material;
    }

    void PBRTParser::ParseLightSource(std::ifstream &fileStream, SceneParser::Scene &outputScene)
    {
        auto &lineStream = GetLineStream();
        lineStream >> lastParsedWord;
        if (lastParsedWord.compare("infinite"))
        {
            std::string expectedWords[] = { "\"string", "mapname\"" };
            ParseExpectedWords(lineStream, expectedWords, ARRAYSIZE(expectedWords));

            ThrowIfTrue(
                outputScene.m_EnvironmentMap.m_FileName.size() > 0,
                "Multiple environment maps defined");
            outputScene.m_EnvironmentMap.m_FileName = m_relativeDirectory + CorrectNameString(ParseString(lineStream));
        }
    }

    void PBRTParser::ParseAreaLightSource(std::ifstream &fileStream, SceneParser::Scene &outputScene)
    {
        auto &lineStream = GetLineStream();

        lineStream >> lastParsedWord;
        ThrowIfTrue(lastParsedWord.compare("\"diffuse\""));

        lineStream >> lastParsedWord;
        ThrowIfTrue(lastParsedWord.compare("\"rgb"));

        lineStream >> lastParsedWord;
        ThrowIfTrue(lastParsedWord.compare("L\""));

        lineStream >> lastParsedWord;
        ThrowIfTrue(lastParsedWord.compare("["));

        AreaLightAttribute attribute;
        char materialName[PBRTPARSER_STRINGBUFFERSIZE];
        lineStream >> attribute.m_lightColor.r;
        lineStream >> attribute.m_lightColor.g;
        lineStream >> attribute.m_lightColor.b;

        lineStream >> lastParsedWord;
        ThrowIfTrue(lastParsedWord.compare("]"));

        SetCurrentAttributes(Attributes(attribute));
    }
    
    void PBRTParser::ParseExpectedWords(std::istream &inStream, _In_reads_(numWords) std::string *pWords, UINT numWords)
    {
        for (UINT i = 0; i < numWords; i++)
        {
            ParseExpectedWord(inStream, pWords[i]);
        }
    }

    void PBRTParser::ParseExpectedWord(std::istream &inStream, const std::string &word)
    {
        inStream >> lastParsedWord;
        ThrowIfTrue(lastParsedWord.compare(word));
    }

    void PBRTParser::ParseTexture(std::ifstream &fileStream, SceneParser::Scene &outputScene)
    {
        // "float uscale"[20.000000] "float vscale"[20.000000] "rgb tex1"[0.325000 0.310000 0.250000] "rgb tex2"[0.725000 0.710000 0.680000]
        auto &lineStream = GetLineStream();

        std::string textureName;
        lineStream >> textureName;
        textureName = CorrectNameString(textureName);

        lineStream >> lastParsedWord;
        ThrowIfTrue(lastParsedWord.compare("\"spectrum\""));

        lineStream >> lastParsedWord;
        if (!lastParsedWord.compare("\"checkerboard\""))
        {
            float uscale;
            {
                std::string expectedWords[] = { "\"float", "uscale\"", "[" };
                ParseExpectedWords(lineStream, expectedWords, ARRAYSIZE(expectedWords));
                lineStream >> uscale;
                ParseExpectedWord(lineStream, "]");
            }

            float vscale;
            {
                std::string expectedWords[] = { "\"float", "vscale\"", "[" };
                ParseExpectedWords(lineStream, expectedWords, ARRAYSIZE(expectedWords));
                lineStream >> vscale;
                ParseExpectedWord(lineStream, "]");
            }

            Vector3 col1;
            {
                std::string expectedWords[] = { "\"rgb", "tex1\"", "[" };
                ParseExpectedWords(lineStream, expectedWords, ARRAYSIZE(expectedWords));
                lineStream >> col1.r;
                lineStream >> col1.g;
                lineStream >> col1.b;
                ParseExpectedWord(lineStream, "]");
            }

            Vector3 col2;
            {
                std::string expectedWords[] = { "\"rgb", "tex2\"", "[" };
                ParseExpectedWords(lineStream, expectedWords, ARRAYSIZE(expectedWords));
                lineStream >> col2.r;
                lineStream >> col2.g;
                lineStream >> col2.b;
                ParseExpectedWord(lineStream, "]");
            }

            std::string fileName = GenerateCheckerboardTexture(
                textureName,
                uscale,
                vscale,
                col1,
                col2);

            m_TextureNameToFileName[textureName] = fileName;
        }
        else
        {
            ThrowIfTrue(true);
        }
    }

    std::string PBRTParser::GenerateCheckerboardTexture(std::string fileName, float uScaleFloat, float vScaleFloat, Vector3 color1, Vector3 color2)
    {
        UINT uScale = (UINT)uScaleFloat;
        UINT vScale = (UINT)vScaleFloat;

        UINT textureWidth = uScale * 2;
        UINT textureHeight = vScale * 2;
        UINT CheckerBlockSize = 2;
        
        std::vector<Vector3> imageData;
        imageData.resize(textureHeight * textureWidth);
        for (UINT y = 0; y < textureHeight; y++)
        {
            for (UINT x = 0; x < textureWidth; x++)
            {
                bool EvenX = (x / CheckerBlockSize) % 2;
                bool EvenY = (y / CheckerBlockSize) % 2;
                imageData[x + y * textureWidth] = (EvenX == EvenY) ? color2 : color1;
            }
        }

        std::string filenameWithExtension = fileName + ".bmp";
        GenerateBMPFile(filenameWithExtension, imageData.data(), textureWidth, textureHeight);
        return filenameWithExtension;
    }

    void PBRTParser::GenerateBMPFile(std::string fileName, _In_reads_(width * height)Vector3 *pImageData, UINT width, UINT height)
    {
        std::ofstream bmpFile(fileName, std::ofstream::out | std::ofstream::binary | std::ofstream::app);
        ThrowIfTrue(!bmpFile.is_open() || bmpFile.fail());

        unsigned char *img = NULL;
        int filesize = 54 + 3 * width*height;  //w is your image width, h is image height, both int
        if (img)
            free(img);
        img = (unsigned char *)malloc(3 * width*height);
        memset(img, 0, sizeof(img));

        for (int i = 0; i<width; i++)
        {
            for (int j = 0; j<height; j++)
            {

                auto pixel = pImageData[i + j * width];

                int x = i; 
                int y = (height - 1) - j;
                int r = pixel.r * 255;
                int g = pixel.g * 255;
                int b = pixel.b * 255;
                if (r > 255) r = 255;
                if (g > 255) g = 255;
                if (b > 255) b = 255;
                img[(x + y*width) * 3 + 2] = (unsigned char)(r);
                img[(x + y*width) * 3 + 1] = (unsigned char)(g);
                img[(x + y*width) * 3 + 0] = (unsigned char)(b);
            }
        }

        unsigned char bmpfileheader[14] = { 'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0 };
        unsigned char bmpinfoheader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0 };
        unsigned char bmppad[3] = { 0,0,0 };

        bmpfileheader[2] = (unsigned char)(filesize);
        bmpfileheader[3] = (unsigned char)(filesize >> 8);
        bmpfileheader[4] = (unsigned char)(filesize >> 16);
        bmpfileheader[5] = (unsigned char)(filesize >> 24);

        bmpinfoheader[4] = (unsigned char)(width);
        bmpinfoheader[5] = (unsigned char)(width >> 8);
        bmpinfoheader[6] = (unsigned char)(width >> 16);
        bmpinfoheader[7] = (unsigned char)(width >> 24);
        bmpinfoheader[8] = (unsigned char)(height);
        bmpinfoheader[9] = (unsigned char)(height >> 8);
        bmpinfoheader[10] = (unsigned char)(height >> 16);
        bmpinfoheader[11] = (unsigned char)(height >> 24);

        for (auto bit : bmpfileheader) bmpFile << bit;
        for (auto bit : bmpinfoheader) bmpFile << bit;
        for (int i = 0; i<height; i++)
        {
            for (int j = 0; j < width * 3; j++)
            {
                bmpFile << *(img + (width*(height - i - 1) * 3) + j);
            }
            for (UINT j = 0; j < (4 - (width * 3) % 4) % 4; j++)
            {
                bmpFile << 0;
            }
        }
        
        bmpFile.close();
        ThrowIfTrue(bmpFile.fail());
    }


    void PBRTParser::ParseMesh(std::ifstream &fileStream, SceneParser::Scene &outputScene)
    {
        Mesh *pMesh;
        if (GetCurrentAttributes().GetType() == Attributes::AreaLight)
        {
            outputScene.m_AreaLights.push_back(
                AreaLight(GetCurrentAttributes().GetAreaLightAttribute().m_lightColor));
            pMesh = &outputScene.m_AreaLights.back().m_Mesh;
        }
        else
        {
            outputScene.m_Meshes.push_back(Mesh());
            pMesh = &outputScene.m_Meshes[outputScene.m_Meshes.size() - 1];
        }
        string correctedMaterialName = CorrectNameString(m_CurrentMaterial);
        pMesh->m_pMaterial = &outputScene.m_Materials[correctedMaterialName];
        ThrowIfTrue(pMesh->m_pMaterial == nullptr, "Material name not found");

        ParseShape(fileStream, outputScene, *pMesh);
    }

    void PBRTParser::ParseShape(std::ifstream &fileStream, SceneParser::Scene &outputScene, SceneParser::Mesh &mesh)
    {
        fileStream >> lastParsedWord;
        
        if (!lastParsedWord.compare("\"plymesh\""))
        {
            std::string ExpectedWords[] = { "\"string", "filename\"" };
            ParseExpectedWords(fileStream, ExpectedWords, ARRAYSIZE(ExpectedWords));

            std::string correctedFileName = CorrectNameString(ParseString(fileStream));
            PlyParser::PlyParser().Parse(m_relativeDirectory + correctedFileName, mesh);
        }
        else if (!lastParsedWord.compare("\"trianglemesh\""))
        {
            fileStream >> lastParsedWord;
            if (!lastParsedWord.compare("\"integer"))
            {
                fileStream >> lastParsedWord;
                ThrowIfTrue(lastParsedWord.compare("indices\""), "\"integer\" expected to be followed up with \"integer\"");

                fileStream >> lastParsedWord;
                ThrowIfTrue(lastParsedWord.compare("["), "\"indices\" expected to be followed up with \"[\"");

                while (fileStream.good())
                {
                    int index;
                    fileStream >> index;
                    if (fileStream.good())
                    {
                        mesh.m_IndexBuffer.push_back(index);
                    }
                    else
                    {
                        fileStream.clear(std::ios::goodbit);
                        fileStream >> lastParsedWord;
                        ThrowIfTrue(lastParsedWord.compare("]"), "Expected closing ']' after indices");
                        break;
                    }
                }

                fileStream >> lastParsedWord;
            }

            if (!lastParsedWord.compare("\"point"))
            {
                fileStream >> lastParsedWord;
                ThrowIfTrue(lastParsedWord.compare("P\""), "Expecting \"point\" syntax to be followed by \"P\"");

                fileStream >> lastParsedWord;
                ThrowIfTrue(lastParsedWord.compare("["), "'P' expected to be followed up with \"[\"");

                while (fileStream.good())
                {
                    SceneParser::Vertex vertex = {};
                    fileStream >> vertex.Position.x;
                    fileStream >> vertex.Position.y;
                    fileStream >> vertex.Position.z;

                    if (fileStream.good())
                    {
                        mesh.m_VertexBuffer.push_back(vertex);
                    }
                    else
                    {
                        fileStream.clear(std::ios::goodbit);
                        fileStream >> lastParsedWord;
                        ThrowIfTrue(lastParsedWord.compare("]"), "Expected closing ']' after positions");
                        break;
                    }
                }

                fileStream >> lastParsedWord;
            }

            if (!lastParsedWord.compare("\"normal"))
            {
                fileStream >> lastParsedWord;
                ThrowIfTrue(lastParsedWord.compare("N\""), "Expecting \"normal\" syntax to be followed by an \"N\"");

                fileStream >> lastParsedWord;
                ThrowIfTrue(lastParsedWord.compare("["), "'N' expected to be followed up with \"[\"");

                UINT vertexIndex = 0;
                while (fileStream.good())
                {
                    float x, y, z;
                    fileStream >> x;
                    fileStream >> y;
                    fileStream >> z;
                    if (fileStream.good())
                    {
                        ThrowIfTrue(vertexIndex >= mesh.m_VertexBuffer.size(), "More position values specified than normals");
                        SceneParser::Vertex &vertex = mesh.m_VertexBuffer[vertexIndex];
                        vertexIndex++;

                        vertex.Normal.x = x;
                        vertex.Normal.y = y;
                        vertex.Normal.z = z;
                    }
                    else
                    {
                        fileStream.clear(std::ios::goodbit);
                        fileStream >> lastParsedWord;
                        ThrowIfTrue(lastParsedWord.compare("]"), "Expected closing ']' after positions");
                        break;
                    }
                }

                fileStream >> lastParsedWord;
            }

            if (!lastParsedWord.compare("\"float"))
            {
                fileStream >> lastParsedWord;
                ThrowIfTrue(lastParsedWord.compare("uv\""), "Expecting \"float\" syntax to be followed by \"uv\"");

                fileStream >> lastParsedWord;
                ThrowIfTrue(lastParsedWord.compare("["), "'UV' expected to be followed up with \"[\"");

                UINT vertexIndex = 0;
                while (fileStream.good())
                {
                    float u, v;
                    fileStream >> u;
                    fileStream >> v;
                    if (fileStream.good())
                    {
                        ThrowIfTrue(vertexIndex >= mesh.m_VertexBuffer.size(), "More UV values specified than normals");
                        SceneParser::Vertex &vertex = mesh.m_VertexBuffer[vertexIndex];
                        vertexIndex++;

                        vertex.UV.u = u;
                        vertex.UV.v = v;
                    }
                    else
                    {
                        fileStream.clear(std::ios::goodbit);
                        fileStream >> lastParsedWord;
                        ThrowIfTrue(lastParsedWord.compare("]"), "Expected closing ']' after positions");
                        break;
                    }
                }

                fileStream >> lastParsedWord;
            }

            mesh.m_AreTangentsValid = false;
        }
    }

    string PBRTParser::CorrectNameString(const string &str)
    {
        return CorrectNameString(str.c_str());
    }

    string PBRTParser::CorrectNameString(const char *pString)
    {
        string correctedString(pString);
        UINT startIndex = 0;
        UINT endIndex = correctedString.size();
        if (correctedString.size())
        {
            // sscanf often pulls extra quotations, cut these out
            if (correctedString[0] == '"')
            {
                startIndex = 1;
            }
            
            if (correctedString[correctedString.size() - 1] == '"')
            {
                endIndex = endIndex - 1;
            }
        }
        return correctedString.substr(startIndex, endIndex - startIndex);
    }

    void PBRTParser::InitializeDefaults(SceneParser::Scene &outputScene)
    {
        InitializeCameraDefaults(outputScene.m_Camera);
    }

    void PBRTParser::InitializeCameraDefaults(Camera &camera)
    {
#if TEAPOT_HACK
        m_lookAt = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
        m_camPos = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
#else
        m_lookAt = XMVectorSet(0.0f, 2.0f, 1.0f, 1.0f);
        m_camPos = XMVectorSet(0.0f, 2.0f, 0.0f, 1.0f);
#endif

        m_camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        camera.m_FieldOfView = 90;
        camera.m_LookAt = ConvertToVector3(m_lookAt);
        camera.m_Position = ConvertToVector3(m_camPos);
        camera.m_Up = Vector3(0.0f, 1.0f, 0.0f);
        camera.m_NearPlane = 0.001f;
        camera.m_FarPlane = 999999.0f;
    }

    void PBRTParser::ParseTransform()
    {
        char *pTempBuffer = GetLine();
        float mat[4][4];

        UINT argCount = sscanf_s(pTempBuffer, " \[ %f %f %f %f  %f %f %f %f  %f %f %f %f  %f %f %f %f \] ",
            &mat[0][0],
            &mat[0][1],
            &mat[0][2],
            &mat[0][3],

            &mat[1][0],
            &mat[1][1],
            &mat[1][2],
            &mat[1][3],

            &mat[2][0],
            &mat[2][1],
            &mat[2][2],
            &mat[2][3],

            &mat[3][0],
            &mat[3][1],
            &mat[3][2],
            &mat[3][3]);

        ThrowIfTrue(argCount != 16, "Transform arguments not formatted correctly");

        m_currentTransform = XMMATRIX(
            mat[0][0],
            mat[1][0],
            mat[2][0],
            mat[3][0],

            mat[0][1],
            mat[1][1],
            mat[2][1],
            mat[3][1],

            mat[0][2],
            mat[1][2],
            mat[2][2],
            mat[3][2],

            mat[0][3],
            mat[1][3],
            mat[2][3],
            mat[3][3]);
    }
}

