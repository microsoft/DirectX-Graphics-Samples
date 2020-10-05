//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//

#pragma once

#include "EngineTuning.h"

namespace SceneParameters
{
    class GrassGeometryCommon
    {
    public:
        inline static const std::wstring RootPath = L"Scene/Grass/";

        NumVar PatchWidth;
        NumVar PatchHeight;
        NumVar WindMapSpeedU;
        NumVar WindMapSpeedV;
        BoolVar ForceLOD0;
        XMFLOAT3 PatchOffset;

        GrassGeometryCommon()
        {
            PatchWidth.Initialize(RootPath + L"Patch/Width", 14.f, 1.f, 100.f, 1.f);
            PatchHeight.Initialize(RootPath + L"Patch/Height", 14.f, 1.f, 100.f, 1.f);
            WindMapSpeedU.Initialize(RootPath + L"Wind Texture speed U", 1.f, -1.f, 1.f, 0.05f);
            WindMapSpeedV.Initialize(RootPath + L"Wind Texture speed V", 1.f, -1.f, 1.f, 0.05f);
            ForceLOD0.Initialize(RootPath + L"Force LOD 0", false);
            PatchOffset = XMFLOAT3(-20, -1.16f, -20);
        }
    };

    class GrassGeometryLOD
    {
    public:
        inline static const std::wstring RootPath = GrassGeometryCommon::RootPath;
        IntVar NumberStrawsX;
        IntVar NumberStrawsZ;
        NumVar StrawHeight;
        NumVar StrawScale;
        NumVar StrawThickness;
        NumVar BendStrengthSideways;
        NumVar WindStrength;
        NumVar WindFrequency;
        NumVar RandomPositionJitterStrength;
        NumVar MaxLODdistance;

        GrassGeometryLOD() {}

        void Initialize(UINT LOD)
        {
            std::wstring RootPath = GrassGeometryCommon::RootPath + L"LOD " + std::to_wstring(LOD) + L"/";

            NumberStrawsX.Initialize(RootPath + L"Patch/Num Straws X", 100, 1, MAX_GRASS_STRAWS_1D, 10);
            NumberStrawsZ.Initialize(RootPath + L"Patch/Num Straws Z", 100, 1, MAX_GRASS_STRAWS_1D, 10);
            StrawHeight.Initialize(RootPath + L"Straw Height", 0.4f, 0.1f, 10.f, 0.05f);
            StrawScale.Initialize(RootPath + L"Straw Scale", 0.6f, 0.1f, 10.f, 0.05f);
            StrawThickness.Initialize(RootPath + L"Straw Thickness", 1.f, 0.1f, 10.f, 0.05f);
            BendStrengthSideways.Initialize(RootPath + L"Bend strength sideways", 0.f, 0.f, 1.f, 0.02f);
            WindStrength.Initialize(RootPath + L"Wind strength", 0.4f, 0.f, 1.f, 0.005f);
            WindFrequency.Initialize(RootPath + L"Wind frequency", 0.04f, 0.f, 1.f, 0.005f);
            RandomPositionJitterStrength.Initialize(RootPath + L"Position jitter strength", 1.0f, 0.f, 2.f, 0.05f);
            MaxLODdistance.Initialize(RootPath + L"Max LOD distance from the camera", 50.f + 50.f * LOD, 0.f, 1000.f, 1.f);
        }
    };
}


class UIParameters
{
public:
    SceneParameters::GrassGeometryCommon GrassCommon;

    static const UINT NumGrassGeometryLODs = 5;
    SceneParameters::GrassGeometryLOD GrassGeometryLOD[NumGrassGeometryLODs];

    UIParameters()
    {
        for (UINT i = 0; i < NumGrassGeometryLODs; i++)
        {
            GrassGeometryLOD[i].Initialize(i);
        }

        GrassGeometryLOD[0].MaxLODdistance = 34;
        GrassGeometryLOD[1].MaxLODdistance = 64;
        GrassGeometryLOD[2].MaxLODdistance = 120;
        GrassGeometryLOD[3].MaxLODdistance = 200;

        GrassGeometryLOD[1].WindStrength = 0.31f;
        GrassGeometryLOD[1].WindStrength = 0.11f;
        GrassGeometryLOD[2].WindStrength = 0.05f;
        GrassGeometryLOD[3].WindStrength = 0.01f;
        GrassGeometryLOD[4].WindStrength = 0.0f;

    }
};

extern UIParameters g_UIparameters;