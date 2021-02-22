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
// Author(s):  James Stanard
//

#include "ParticleEffects.h"
#include "../Core/ParticleEffectManager.h"
#include "TextureManager.h"
#include <fstream>

#pragma warning(push)
#pragma warning(disable : 4100) // unreferenced formal parameter
#include "json.hpp"
#pragma warning(pop)

using namespace std;
using namespace Graphics;
using namespace Math;

namespace
{
    map<wstring, uint32_t> s_TextureArrayLookup;
    vector<TextureRef> s_TextureReferences;

    uint32_t GetTextureIndex(const wstring& name)
    {
        // Look for the texture already being assigned an index.  If it's not found,
        // load the texture and assign it an index.
        auto iter = s_TextureArrayLookup.find(name);

        if (iter != s_TextureArrayLookup.end())
        {
            return iter->second;
        }
        else
        {
            uint32_t index = (uint32_t)s_TextureArrayLookup.size();
            s_TextureArrayLookup[name] = index;

            // Load the texture and register it with the effect manager
            TextureRef texture = TextureManager::LoadDDSFromFile(name, kMagenta2D, true);
            s_TextureReferences.push_back(texture);
            ParticleEffectManager::RegisterTexture(index, *texture.Get());

            return index;
        }
    }

    void InstantiateEffect( ParticleEffectProperties& effectProperties )
    {
        effectProperties.EmitProperties.TextureID = GetTextureIndex(effectProperties.TexturePath);
        ParticleEffectManager::InstantiateEffect(effectProperties);
    }
}

//---------------------------------------------------------------------
//
//	JSON Init File Handling
//
//---------------------------------------------------------------------

#define MAKE_FLOAT(it) (float)it.value()
#define MAKE_UINT(it) (uint32_t)it.value()
#define MAKE_WSTRING(it) Utility::UTF8ToWideString(it.value())
#define MAKE_COLOR(it) Color((float)it.value()[0], (float)it.value()[1], (float)it.value()[2], (float)it.value()[3])
#define MAKE_VECTOR3(it) Vector3((float)it.value()[0], (float)it.value()[1], (float)it.value()[2])
#define MAKE_VECTOR4(it) Vector4((float)it.value()[0], (float)it.value()[1], (float)it.value()[2], (float)it.value()[3])
#define MAKE_XMFLOAT2(it) XMFLOAT2((float)it.value()[0], (float)it.value()[1])
#define MAKE_XMFLOAT3(it) XMFLOAT3((float)it.value()[0], (float)it.value()[1], (float)it.value()[2])

void ParticleEffects::InitFromJSON(const wstring& InitJsonFile)
{
    using json = nlohmann::json;

    json particle_setup;
    ifstream(InitJsonFile) >> particle_setup;

    if (!particle_setup.is_object() || particle_setup.find("ParticleEmitters") == particle_setup.end())
        return;

    for (auto& emitter : particle_setup["ParticleEmitters"])
    {
        ParticleEffectProperties Effect = ParticleEffectProperties();

        for (json::iterator it = emitter.begin(); it != emitter.end(); ++it)
        {
            if (it.key() == "TexturePath")
            {
                Effect.TexturePath = MAKE_WSTRING(it);
            }
            else if (it.key() == "MinStartColor")
            {
                Effect.MinStartColor = MAKE_COLOR(it);
            }
            else if (it.key() == "MaxStartColor")
            {
                Effect.MaxStartColor = MAKE_COLOR(it);
            }
            else if (it.key() == "MinEndColor")
            {
                Effect.MinEndColor = MAKE_COLOR(it);
            }
            else if (it.key() == "MaxEndColor")
            {
                Effect.MaxEndColor = MAKE_COLOR(it);
            }
            else if (it.key() == "MaxEndColor")
            {
                Effect.MaxEndColor = MAKE_COLOR(it);
            }
            else if (it.key() == "TotalActiveLifetime")
            {
                Effect.TotalActiveLifetime = MAKE_FLOAT(it);
                if (Effect.TotalActiveLifetime == 0.0f)
                    Effect.TotalActiveLifetime = FLT_MAX;
            }
            else if (it.key() == "EmitRate")
            {
                Effect.EmitRate = MAKE_FLOAT(it);
            }
            else if (it.key() == "Size")
            {
                Effect.Size = MAKE_VECTOR4(it);
            }
            else if (it.key() == "Velocity")
            {
                Effect.Velocity = MAKE_VECTOR4(it);
            }
            else if (it.key() == "LifeMinMax")
            {
                Effect.LifeMinMax = MAKE_XMFLOAT2(it);
            }
            else if (it.key() == "MassMinMax")
            {
                Effect.MassMinMax = MAKE_XMFLOAT2(it);
            }
            else if (it.key() == "Spread")
            {
                Effect.Spread = MAKE_XMFLOAT3(it);
            }
            else if (it.key() == "EmitProperties")
            {
                for (json::iterator it2 = it.value().begin(); it2 != it.value().end(); ++it2)
                {
                    if (it2.key() == "Gravity")
                    {
                        Effect.EmitProperties.Gravity = MAKE_XMFLOAT3(it2);
                    }
                    else if (it2.key() == "FloorHeight")
                    {
                        Effect.EmitProperties.FloorHeight = MAKE_FLOAT(it2);
                    }
                    else if (it2.key() == "MaxParticles")
                    {
                        Effect.EmitProperties.MaxParticles = MAKE_UINT(it2);
                    }
                    else if (it2.key() == "EmitPosW")
                    {
                        Effect.EmitProperties.EmitPosW = MAKE_XMFLOAT3(it2);
                    }
                    else if (it2.key() == "LastEmitPosW")
                    {
                        Effect.EmitProperties.LastEmitPosW = MAKE_XMFLOAT3(it2);
                    }
                    else
                    {
                        Utility::Printf("Warning:  Emitter property \"%s\" not found \n", it2.key().c_str());
                    }
                }
            }
            else
            {
                Utility::Printf("Warning:  Effect property \"%s\" not found \n", it.key().c_str());
            }

        }

        InstantiateEffect(Effect);
    }
}
