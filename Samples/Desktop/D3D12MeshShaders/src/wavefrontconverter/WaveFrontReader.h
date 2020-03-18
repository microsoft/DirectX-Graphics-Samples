//--------------------------------------------------------------------------------------
// File: WaveFrontReader.h
//
// Code for loading basic mesh data from a WaveFront OBJ file
//
// http://en.wikipedia.org/wiki/Wavefront_.obj_file
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkID=324981
//--------------------------------------------------------------------------------------

#pragma once

#pragma warning(push)
#pragma warning(disable : 4005)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NODRAWTEXT
#define NOGDI
#define NOMCX
#define NOSERVICE
#define NOHELP
#pragma warning(pop)

#include <Windows.h>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

#include <DirectXMath.h>
#include <DirectXCollision.h>

template<class index_t>
class WaveFrontReader
{
public:
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT2 textureCoordinate;
    };

    WaveFrontReader() noexcept : hasNormals(false), hasTexcoords(false) {}

    HRESULT Load(_In_z_ const wchar_t* szFileName, bool ccw = true)
    {
        Clear();

        static const size_t MAX_POLY = 64;

        using namespace DirectX;

        std::wifstream InFile(szFileName);
        if (!InFile)
            return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

        wchar_t fname[_MAX_FNAME] = {};
        _wsplitpath_s(szFileName, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, nullptr, 0);

        name = fname;

        std::vector<XMFLOAT3>   positions;
        std::vector<XMFLOAT3>   normals;
        std::vector<XMFLOAT2>   texCoords;

        VertexCache  vertexCache;

        Material defmat;

        wcscpy_s(defmat.strName, L"default");
        materials.emplace_back(defmat);

        uint32_t curSubset = 0;

        wchar_t strMaterialFilename[MAX_PATH] = {};
        for (;; )
        {
            std::wstring strCommand;
            InFile >> strCommand;
            if (!InFile)
                break;

            if (*strCommand.c_str() == L'#')
            {
                // Comment
            }
            else if (0 == wcscmp(strCommand.c_str(), L"o"))
            {
                // Object name ignored
            }
            else if (0 == wcscmp(strCommand.c_str(), L"g"))
            {
                // Group name ignored
            }
            else if (0 == wcscmp(strCommand.c_str(), L"s"))
            {
                // Smoothing group ignored
            }
            else if (0 == wcscmp(strCommand.c_str(), L"v"))
            {
                // Vertex Position
                float x, y, z;
                InFile >> x >> y >> z;
                positions.emplace_back(XMFLOAT3(x, y, z));
            }
            else if (0 == wcscmp(strCommand.c_str(), L"vt"))
            {
                // Vertex TexCoord
                float u, v;
                InFile >> u >> v;
                texCoords.emplace_back(XMFLOAT2(u, v));

                hasTexcoords = true;
            }
            else if (0 == wcscmp(strCommand.c_str(), L"vn"))
            {
                // Vertex Normal
                float x, y, z;
                InFile >> x >> y >> z;
                normals.emplace_back(XMFLOAT3(x, y, z));

                hasNormals = true;
            }
            else if (0 == wcscmp(strCommand.c_str(), L"f"))
            {
                // Face
                INT iPosition, iTexCoord, iNormal;
                Vertex vertex;

                uint32_t faceIndex[MAX_POLY];
                size_t iFace = 0;
                for (;;)
                {
                    if (iFace >= MAX_POLY)
                    {
                        // Too many polygon verts for the reader
                        return E_FAIL;
                    }

                    memset(&vertex, 0, sizeof(vertex));

                    InFile >> iPosition;

                    uint32_t vertexIndex = 0;
                    if (!iPosition)
                    {
                        // 0 is not allowed for index
                        return E_UNEXPECTED;
                    }
                    else if (iPosition < 0)
                    {
                        // Negative values are relative indices
                        vertexIndex = uint32_t(ptrdiff_t(positions.size()) + iPosition);
                    }
                    else
                    {
                        // OBJ format uses 1-based arrays
                        vertexIndex = uint32_t(iPosition - 1);
                    }

                    if (vertexIndex >= positions.size())
                        return E_FAIL;

                    vertex.position = positions[vertexIndex];

                    if ('/' == InFile.peek())
                    {
                        InFile.ignore();

                        if ('/' != InFile.peek())
                        {
                            // Optional texture coordinate
                            InFile >> iTexCoord;

                            uint32_t coordIndex = 0;
                            if (!iTexCoord)
                            {
                                // 0 is not allowed for index
                                return E_UNEXPECTED;
                            }
                            else if (iTexCoord < 0)
                            {
                                // Negative values are relative indices
                                coordIndex = uint32_t(ptrdiff_t(texCoords.size()) + iTexCoord);
                            }
                            else
                            {
                                // OBJ format uses 1-based arrays
                                coordIndex = uint32_t(iTexCoord - 1);
                            }

                            if (coordIndex >= texCoords.size())
                                return E_FAIL;

                            vertex.textureCoordinate = texCoords[coordIndex];
                        }

                        if ('/' == InFile.peek())
                        {
                            InFile.ignore();

                            // Optional vertex normal
                            InFile >> iNormal;

                            uint32_t normIndex = 0;
                            if (!iNormal)
                            {
                                // 0 is not allowed for index
                                return E_UNEXPECTED;
                            }
                            else if (iNormal < 0)
                            {
                                // Negative values are relative indices
                                normIndex = uint32_t(ptrdiff_t(normals.size()) + iNormal);
                            }
                            else
                            {
                                // OBJ format uses 1-based arrays
                                normIndex = uint32_t(iNormal - 1);
                            }

                            if (normIndex >= normals.size())
                                return E_FAIL;

                            vertex.normal = normals[normIndex];
                        }
                    }

                    // If a duplicate vertex doesn't exist, add this vertex to the Vertices
                    // list. Store the index in the Indices array. The Vertices and Indices
                    // lists will eventually become the Vertex Buffer and Index Buffer for
                    // the mesh.
                    uint32_t index = AddVertex(vertexIndex, &vertex, vertexCache);
                    if (index == uint32_t(-1))
                        return E_OUTOFMEMORY;

#pragma warning( suppress : 4127 )
                    if (sizeof(index_t) == 2 && (index >= 0xFFFF))
                    {
                        // Too many indices for 16-bit IB!
                        return E_FAIL;
                    }
#pragma warning( suppress : 4127 )
                    else if (sizeof(index_t) == 4 && (index >= 0xFFFFFFFF))
                    {
                        // Too many indices for 32-bit IB!
                        return E_FAIL;
                    }

                    faceIndex[iFace] = index;
                    ++iFace;

                    // Check for more face data or end of the face statement
                    bool faceEnd = false;
                    for (;;)
                    {
                        wchar_t p = InFile.peek();

                        if ('\n' == p || !InFile)
                        {
                            faceEnd = true;
                            break;
                        }
                        else if (isdigit(p) || p == '-' || p == '+')
                            break;

                        InFile.ignore();
                    }

                    if (faceEnd)
                        break;
                }

                if (iFace < 3)
                {
                    // Need at least 3 points to form a triangle
                    return E_FAIL;
                }

                // Convert polygons to triangles
                uint32_t i0 = faceIndex[0];
                uint32_t i1 = faceIndex[1];

                for (size_t j = 2; j < iFace; ++j)
                {
                    uint32_t index = faceIndex[j];
                    indices.emplace_back(static_cast<index_t>(i0));
                    if (ccw)
                    {
                        indices.emplace_back(static_cast<index_t>(i1));
                        indices.emplace_back(static_cast<index_t>(index));
                    }
                    else
                    {
                        indices.emplace_back(static_cast<index_t>(index));
                        indices.emplace_back(static_cast<index_t>(i1));
                    }

                    attributes.emplace_back(curSubset);

                    i1 = index;
                }

                assert(attributes.size() * 3 == indices.size());
            }
            else if (0 == wcscmp(strCommand.c_str(), L"mtllib"))
            {
                // Material library
                InFile >> strMaterialFilename;
            }
            else if (0 == wcscmp(strCommand.c_str(), L"usemtl"))
            {
                // Material
                wchar_t strName[MAX_PATH] = {};
                InFile >> strName;

                bool bFound = false;
                uint32_t count = 0;
                for (auto it = materials.cbegin(); it != materials.cend(); ++it, ++count)
                {
                    if (0 == wcscmp(it->strName, strName))
                    {
                        bFound = true;
                        curSubset = count;
                        break;
                    }
                }

                if (!bFound)
                {
                    Material mat;
                    curSubset = static_cast<uint32_t>(materials.size());
                    wcscpy_s(mat.strName, MAX_PATH - 1, strName);
                    materials.emplace_back(mat);
                }
            }
            else
            {
#ifdef _DEBUG
                // Unimplemented or unrecognized command
                OutputDebugStringW(strCommand.c_str());
#endif
            }

            InFile.ignore(1000, L'\n');
        }

        if (positions.empty())
            return E_FAIL;

        // Cleanup
        InFile.close();

        BoundingBox::CreateFromPoints(bounds, positions.size(), positions.data(), sizeof(XMFLOAT3));

        // If an associated material file was found, read that in as well.
        if (*strMaterialFilename)
        {
            wchar_t ext[_MAX_EXT] = {};
            _wsplitpath_s(strMaterialFilename, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

            wchar_t drive[_MAX_DRIVE] = {};
            wchar_t dir[_MAX_DIR] = {};
            _wsplitpath_s(szFileName, drive, _MAX_DRIVE, dir, _MAX_DIR, nullptr, 0, nullptr, 0);

            wchar_t szPath[MAX_PATH] = {};
            _wmakepath_s(szPath, MAX_PATH, drive, dir, fname, ext);
            HRESULT hr = LoadMTL(szPath);
            if (FAILED(hr))
                return hr;
        }

        return S_OK;
    }

    HRESULT LoadMTL(_In_z_ const wchar_t* szFileName)
    {
        using namespace DirectX;

        // Assumes MTL is in CWD along with OBJ
        std::wifstream InFile(szFileName);
        if (!InFile)
            return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

        auto curMaterial = materials.end();

        for (;; )
        {
            std::wstring strCommand;
            InFile >> strCommand;
            if (!InFile)
                break;

            if (0 == wcscmp(strCommand.c_str(), L"newmtl"))
            {
                // Switching active materials
                wchar_t strName[MAX_PATH] = {};
                InFile >> strName;

                curMaterial = materials.end();
                for (auto it = materials.begin(); it != materials.end(); ++it)
                {
                    if (0 == wcscmp(it->strName, strName))
                    {
                        curMaterial = it;
                        break;
                    }
                }
            }

            // The rest of the commands rely on an active material
            if (curMaterial == materials.end())
                continue;

            if (0 == wcscmp(strCommand.c_str(), L"#"))
            {
                // Comment
            }
            else if (0 == wcscmp(strCommand.c_str(), L"Ka"))
            {
                // Ambient color
                float r, g, b;
                InFile >> r >> g >> b;
                curMaterial->vAmbient = XMFLOAT3(r, g, b);
            }
            else if (0 == wcscmp(strCommand.c_str(), L"Kd"))
            {
                // Diffuse color
                float r, g, b;
                InFile >> r >> g >> b;
                curMaterial->vDiffuse = XMFLOAT3(r, g, b);
            }
            else if (0 == wcscmp(strCommand.c_str(), L"Ks"))
            {
                // Specular color
                float r, g, b;
                InFile >> r >> g >> b;
                curMaterial->vSpecular = XMFLOAT3(r, g, b);
            }
            else if (0 == wcscmp(strCommand.c_str(), L"Ke"))
            {
                // Emissive color
                float r, g, b;
                InFile >> r >> g >> b;
                curMaterial->vEmissive = XMFLOAT3(r, g, b);
                if (r > 0.f || g > 0.f || b > 0.f)
                {
                    curMaterial->bEmissive = true;
                }
            }
            else if (0 == wcscmp(strCommand.c_str(), L"d"))
            {
                // Alpha
                float alpha;
                InFile >> alpha;
                curMaterial->fAlpha = std::min(1.f, std::max(0.f, alpha));
            }
            else if (0 == wcscmp(strCommand.c_str(), L"Tr"))
            {
                // Transparency (inverse of alpha)
                float invAlpha;
                InFile >> invAlpha;
                curMaterial->fAlpha = std::min(1.f, std::max(0.f, 1.f - invAlpha));
            }
            else if (0 == wcscmp(strCommand.c_str(), L"Ns"))
            {
                // Shininess
                int nShininess;
                InFile >> nShininess;
                curMaterial->nShininess = uint32_t(nShininess);
            }
            else if (0 == wcscmp(strCommand.c_str(), L"illum"))
            {
                // Specular on/off
                int illumination;
                InFile >> illumination;
                curMaterial->bSpecular = (illumination == 2);
            }
            else if (0 == wcscmp(strCommand.c_str(), L"map_Kd"))
            {
                // Diffuse texture
                LoadTexturePath(InFile, curMaterial->strTexture, MAX_PATH);
            }
            else if (0 == wcscmp(strCommand.c_str(), L"map_Ks"))
            {
                // Specular texture
                LoadTexturePath(InFile, curMaterial->strSpecularTexture, MAX_PATH);
            }
            else if (0 == wcscmp(strCommand.c_str(), L"map_Kn")
                || 0 == wcscmp(strCommand.c_str(), L"norm"))
            {
                // Normal texture
                LoadTexturePath(InFile, curMaterial->strNormalTexture, MAX_PATH);
            }
            else if (0 == wcscmp(strCommand.c_str(), L"map_Ke")
                || 0 == wcscmp(strCommand.c_str(), L"map_emissive"))
            {
                // Emissive texture
                LoadTexturePath(InFile, curMaterial->strEmissiveTexture, MAX_PATH);
                curMaterial->bEmissive = true;
            }
            else if (0 == wcscmp(strCommand.c_str(), L"map_RMA")
                || 0 == wcscmp(strCommand.c_str(), L"map_ORM"))
            {
                // RMA texture
                LoadTexturePath(InFile, curMaterial->strRMATexture, MAX_PATH);
            }
            else
            {
                // Unimplemented or unrecognized command
            }

            InFile.ignore(1000, L'\n');
        }

        InFile.close();

        return S_OK;
    }

    void Clear()
    {
        vertices.clear();
        indices.clear();
        attributes.clear();
        materials.clear();
        name.clear();
        hasNormals = false;
        hasTexcoords = false;

        bounds.Center.x = bounds.Center.y = bounds.Center.z = 0.f;
        bounds.Extents.x = bounds.Extents.y = bounds.Extents.z = 0.f;
    }

    HRESULT LoadVBO(_In_z_ const wchar_t* szFileName)
    {
        using namespace DirectX;

        Clear();

        wchar_t fname[_MAX_FNAME] = {};
        _wsplitpath_s(szFileName, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, nullptr, 0);

        name = fname;

        Material defmat;
        wcscpy_s(defmat.strName, L"default");
        materials.emplace_back(defmat);

        std::ifstream vboFile(szFileName, std::ifstream::in | std::ifstream::binary);
        if (!vboFile.is_open())
            return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

        hasNormals = hasTexcoords = true;

        uint32_t numVertices = 0;
        uint32_t numIndices = 0;

        vboFile.read(reinterpret_cast<char*>(&numVertices), sizeof(uint32_t));
        if (!numVertices)
            return E_FAIL;

        vboFile.read(reinterpret_cast<char*>(&numIndices), sizeof(uint32_t));
        if (!numIndices)
            return E_FAIL;

        vertices.resize(numVertices);
        vboFile.read(reinterpret_cast<char*>(vertices.data()), sizeof(Vertex) * numVertices);

#pragma warning( suppress : 4127 )
        if (sizeof(index_t) == 2)
        {
            indices.resize(numIndices);
            vboFile.read(reinterpret_cast<char*>(indices.data()), sizeof(uint16_t) * numIndices);
        }
        else
        {
            std::vector<uint16_t> tmp;
            tmp.resize(numIndices);
            vboFile.read(reinterpret_cast<char*>(tmp.data()), sizeof(uint16_t) * numIndices);

            indices.reserve(numIndices);
            for (auto it = tmp.cbegin(); it != tmp.cend(); ++it)
            {
                indices.emplace_back(*it);
            }
        }

        BoundingBox::CreateFromPoints(bounds, vertices.size(), reinterpret_cast<const XMFLOAT3*>(vertices.data()), sizeof(Vertex));

        vboFile.close();

        return S_OK;
    }

    struct Material
    {
        DirectX::XMFLOAT3 vAmbient;
        DirectX::XMFLOAT3 vDiffuse;
        DirectX::XMFLOAT3 vSpecular;
        DirectX::XMFLOAT3 vEmissive;
        uint32_t nShininess;
        float fAlpha;

        bool bSpecular;
        bool bEmissive;

        wchar_t strName[MAX_PATH];
        wchar_t strTexture[MAX_PATH];
        wchar_t strNormalTexture[MAX_PATH];
        wchar_t strSpecularTexture[MAX_PATH];
        wchar_t strEmissiveTexture[MAX_PATH];
        wchar_t strRMATexture[MAX_PATH];

        Material() noexcept :
            vAmbient(0.2f, 0.2f, 0.2f),
            vDiffuse(0.8f, 0.8f, 0.8f),
            vSpecular(1.0f, 1.0f, 1.0f),
            vEmissive(0.f, 0.f, 0.f),
            nShininess(0),
            fAlpha(1.f),
            bSpecular(false),
            bEmissive(false),
            strName{},
            strTexture{},
            strNormalTexture{},
            strSpecularTexture{},
            strEmissiveTexture{},
            strRMATexture{}
        {
        }
    };

    std::vector<Vertex>     vertices;
    std::vector<index_t>    indices;
    std::vector<uint32_t>   attributes;
    std::vector<Material>   materials;

    std::wstring            name;
    bool                    hasNormals;
    bool                    hasTexcoords;

    DirectX::BoundingBox    bounds;

private:
    using VertexCache = std::unordered_multimap<uint32_t, uint32_t>;

    uint32_t AddVertex(uint32_t hash, const Vertex* pVertex, VertexCache& cache)
    {
        auto f = cache.equal_range(hash);

        for (auto it = f.first; it != f.second; ++it)
        {
            auto& tv = vertices[it->second];

            if (0 == memcmp(pVertex, &tv, sizeof(Vertex)))
            {
                return it->second;
            }
        }

        auto index = static_cast<uint32_t>(vertices.size());
        vertices.emplace_back(*pVertex);

        VertexCache::value_type entry(hash, index);
        cache.insert(entry);
        return index;
    }

    void LoadTexturePath(std::wifstream& InFile, _Out_writes_(maxChar) wchar_t* texture, size_t maxChar)
    {
        wchar_t buff[1024] = {};
        InFile.getline(buff, 1024, L'\n');
        InFile.putback(L'\n');

        std::wstring path = buff;

        // Ignore any end-of-line comment
        size_t pos = path.find_first_of(L'#');
        if (pos != std::wstring::npos)
        {
            path = path.substr(0, pos);
        }

        // Trim any trailing whitespace
        pos = path.find_last_not_of(L" \t");
        if (pos != std::wstring::npos)
        {
            path = path.substr(0, pos + 1);
        }

        // Texture path should be last element in line
        pos = path.find_last_of(' ');
        if (pos != std::wstring::npos)
        {
            path = path.substr(pos + 1);
        }

        if (!path.empty())
        {
            wcscpy_s(texture, maxChar, path.c_str());
        }
    }
};
