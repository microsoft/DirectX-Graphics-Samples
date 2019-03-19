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

#pragma once

// SquidRoom

namespace SampleAssets
{
    const wchar_t DataFileName[] = L"SquidRoom.bin";

    const D3D12_INPUT_ELEMENT_DESC StandardVertexDescription[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    const UINT StandardVertexStride = 44;

    const DXGI_FORMAT StandardIndexFormat = DXGI_FORMAT_R32_UINT;

    struct TextureResource
    {
        UINT Width;
        UINT Height;
        UINT MipLevels;
        DXGI_FORMAT Format;
        struct DataProperties
        {
            UINT Offset;
            UINT Size;
            UINT Pitch;
        } Data[D3D12_REQ_MIP_LEVELS];
    };

    struct DrawParameters
    {
        INT DiffuseTextureIndex;
        INT NormalTextureIndex;
        INT SpecularTextureIndex;
        UINT IndexStart;
        UINT IndexCount;
        UINT VertexBase;
    };

    const UINT VertexDataOffset = 30277640;
    const UINT VertexDataSize = 9685808;
    const UINT IndexDataOffset = 39963448;
    const UINT IndexDataSize = 3056844;

    const TextureResource Textures[] =
    {
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 0, 131072, 1024 }, } }, // squard room platform_3_diff_512.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 131072, 131072, 1024 }, } }, // squard room platform_3_norm_512.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 262144, 524288, 2048 }, } }, // squard room platform_2_diff_1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 786432, 524288, 2048 }, } }, // squard room platform_2_norm_1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 1310720, 524288, 2048 }, } }, // squard room platform_1_diff_1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 1835008, 524288, 2048 }, } }, // squard room platform_1_norm_1024.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 2359296, 131072, 1024 }, } }, // shelves2_diff1_512.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 2490368, 131072, 1024 }, } }, // shelves2_nm1_512.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 2621440, 524288, 2048 }, } }, // Misc_Boss_2 1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 3145728, 524288, 2048 }, } }, // Misc_Boss_2_normal1024.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 3670016, 131072, 1024 }, } }, // Hanging_bundle_512.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 3801088, 131072, 1024 }, } }, // Catwalk_03_Normal_512.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 3932160, 524288, 2048 }, } }, // Stack_ Boxes_Diff02_1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 4456448, 524288, 2048 }, } }, // Stack_ Boxes_Nm02_1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 4980736, 524288, 2048 }, } }, // Stack_ Boxes_Diff03_1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 5505024, 524288, 2048 }, } }, // Stack_ Boxes_Nm03_1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 6029312, 524288, 2048 }, } }, // Stack_ Boxes_Diff01_1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 6553600, 524288, 2048 }, } }, // Stack_ Boxes_Nm01_1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 7077888, 524288, 2048 }, } }, // Back_Alley_box_1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 7602176, 524288, 2048 }, } }, // Back_Alley_box _norm_1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 8126464, 524288, 2048 }, } }, // gameCrates_01_Diff_1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 8650752, 524288, 2048 }, } }, // gameCrates_01_Nor_1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 9175040, 524288, 2048 }, } }, // RaceCar_Strorage_Diff512.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 9699328, 524288, 2048 }, } }, // RaceCar_Strorage_Norm512.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 10223616, 131072, 1024 }, } }, // hats_02_diff_512.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 10354688, 131072, 1024 }, } }, // hats_02_norm_512.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 10485760, 131072, 1024 }, } }, // hats_01_diff_512.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 10616832, 131072, 1024 }, } }, // hats_01_norm_512.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 10747904, 524288, 2048 }, } }, // Misc_Boss_1_1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 11272192, 524288, 2048 }, } }, // Misc_Boss_1_normal_1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 11796480, 524288, 2048 }, } }, // gameCrates_03_Diff_1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 12320768, 524288, 2048 }, } }, // gameCrates_03_Nor_1024.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 12845056, 131072, 1024 }, } }, // gameCrates_02_Diff_512.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 12976128, 524288, 2048 }, } }, // Back_Alley_Drum.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 13500416, 131072, 1024 }, } }, // Back_Alley_Drum _norm_512.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 13631488, 131072, 1024 }, } }, // shelves2_diff_512.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 13762560, 131072, 1024 }, } }, // shelves2_nor_512.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 13893632, 131072, 1024 }, } }, // shelves2_diff2_512.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 14024704, 131072, 1024 }, } }, // shelves2_nm2_512.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 14155776, 524288, 2048 }, } }, // marbel drum texture_1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 14680064, 524288, 2048 }, } }, // marbel drum texture _Nrml_1024.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 15204352, 131072, 1024 }, } }, // Catwalk_02_Diffuse512.dds
        {     1,     1,   1,  DXGI_FORMAT_R8G8B8A8_UNORM, { { 15335424, 4, 4 }, } }, // default-normalmap.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 15335428, 131072, 1024 }, } }, // Catwalk_03_Diffuse_512.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 15466500, 131072, 1024 }, } }, // shelves3_diff_512.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 15597572, 131072, 1024 }, } }, // shelves3_nor_512.dds
        {  2048,  2048,   1,       DXGI_FORMAT_BC1_UNORM, { { 15728644, 2097152, 4096 }, } }, // Misc_Boss_3_2048.dds
        {  2048,  2048,   1,       DXGI_FORMAT_BC1_UNORM, { { 17825796, 2097152, 4096 }, } }, // Misc_Boss_3_normal2048R.dds
        {     1,     1,   1,  DXGI_FORMAT_R8G8B8A8_UNORM, { { 19922948, 4, 4 }, } }, // default.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 19922952, 131072, 1024 }, } }, // Hanghing Light_512.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 20054024, 131072, 1024 }, } }, // Hanghing Light_normal_512.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 20185096, 131072, 1024 }, } }, // Hanging_bundle_normal_512.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 20316168, 131072, 1024 }, } }, // Hanging_bundle_marble_512.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 20447240, 131072, 1024 }, } }, // Hanging_bundle_marble_normal_512.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 20578312, 131072, 1024 }, } }, // window_Diff512.dds
        {   512,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 20709384, 262144, 1024 }, } }, // Sliding Steel Door_Diff_512.dds
        {   512,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 20971528, 262144, 1024 }, } }, // Sliding Steel Door_Norm_512.dds
        {   512,   512,   1,       DXGI_FORMAT_BC1_UNORM, { { 21233672, 131072, 1024 }, } }, // window_Norm512.dds
        {   512,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 21364744, 262144, 1024 }, } }, // Door_Diff_512.dds
        {   512,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 21626888, 262144, 1024 }, } }, // Door_Norm_512.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 21889032, 524288, 2048 }, } }, // floor_Diff_1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 22413320, 524288, 2048 }, } }, // floor_Normal_1024.dds
        {  2048,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 22937608, 1048576, 4096 }, } }, // wall03_Diff_2048.dds
        {  2048,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 23986184, 1048576, 4096 }, } }, // wall03_Normal_2048.dds
        {  2048,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 25034760, 1048576, 4096 }, } }, // wall01_Diff_2048.dds
        {  2048,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 26083336, 1048576, 4096 }, } }, // wall01_Normal_2048.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 27131912, 524288, 2048 }, } }, // Roof_Diff1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 27656200, 524288, 2048 }, } }, // Roof_Normal1024.dds
        {   512,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 28180488, 262144, 1024 }, } }, // pillar_Diff_512.dds
        {   512,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 28442632, 262144, 1024 }, } }, // pillar_Norm_512.dds
        {   512,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 28704776, 262144, 1024 }, } }, // Broken_Pillar_Diff_512.dds
        {   512,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 28966920, 262144, 1024 }, } }, // Broken_Pillar_Norm_512.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 29229064, 524288, 2048 }, } }, // Golfclub_dm_1024.dds
        {  1024,  1024,   1,       DXGI_FORMAT_BC1_UNORM, { { 29753352, 524288, 2048 }, } }, // Golfclub_nm_1024.dds
    };

    const DrawParameters Draws[] =
    {
        {   0,   1,  -1,        0,    15198,        0 }, // subset0_squard_room_platform_3_dif1
        {   2,   3,  -1,    15198,      438,     6051 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    15636,      300,     6164 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    15936,      300,     6225 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    16236,      438,     6286 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    16674,      300,     6399 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    16974,      438,     6460 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    17412,      300,     6573 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    17712,      450,     6634 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    18162,      300,     6756 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    18462,      438,     6817 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    18900,      300,     6930 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    19200,      300,     6991 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    19500,      438,     7052 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    19938,      300,     7165 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    20238,      438,     7226 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    20676,      300,     7339 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    20976,      300,     7400 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    21276,      438,     7461 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    21714,      300,     7574 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    22014,      438,     7635 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    22452,      300,     7748 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    22752,      450,     7809 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    23202,      300,     7931 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    23502,      438,     7992 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    23940,      300,     8105 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    24240,      300,     8166 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    24540,      438,     8227 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    24978,      300,     8340 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    25278,      438,     8401 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    25716,      300,     8514 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    26016,      300,     8575 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    26316,      438,     8636 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    26754,      300,     8749 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    27054,      438,     8810 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    27492,      300,     8923 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    27792,      300,     8984 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    28092,      438,     9045 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    28530,      300,     9158 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    28830,      438,     9219 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    29268,      300,     9332 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    29568,      450,     9393 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    30018,      300,     9515 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    30318,      438,     9576 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    30756,      300,     9689 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    31056,      300,     9750 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    31356,      438,     9811 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    31794,      300,     9924 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    32094,      438,     9985 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    32532,      300,    10098 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    32832,      450,    10159 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    33282,      300,    10281 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    33582,      438,    10342 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    34020,      300,    10455 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    34320,      300,    10516 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    34620,      438,    10577 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    35058,      300,    10690 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    35358,      900,    10751 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    36258,      282,    11016 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    36540,      282,    11142 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    36822,      222,    11228 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    37044,      282,    11304 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    37326,      282,    11396 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    37608,      282,    11485 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    37890,      243,    11577 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    38133,      222,    11665 }, // subset0_squard_room_platform_2_dif
        {   2,   3,  -1,    38355,      282,    11743 }, // subset0_squard_room_platform_2_dif
        {   4,   5,  -1,    38637,     2700,    11834 }, // subset0_squard_room_platform_1_diff
        {   6,   7,  -1,    41337,     1788,    12560 }, // subset0_shelves1_diff
        {   8,   9,  -1,    43125,      180,    13101 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    43305,      180,    13138 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    43485,       96,    13175 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    43581,       96,    13202 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    43677,       96,    13229 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    43773,       96,    13256 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    43869,       96,    13283 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    43965,       30,    13310 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    43995,       24,    13322 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44019,       24,    13332 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44043,       24,    13342 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44067,       24,    13351 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44091,       54,    13360 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44145,       12,    13380 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44157,       72,    13386 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44229,       84,    13424 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44313,       30,    13448 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44343,        6,    13460 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44349,       36,    13464 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44385,       60,    13478 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44445,       72,    13496 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44517,       48,    13534 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44565,       24,    13549 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44589,       24,    13559 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44613,       24,    13569 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44637,       42,    13579 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44679,       12,    13595 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44691,       48,    13603 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44739,        6,    13625 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44745,       48,    13629 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44793,        6,    13644 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44799,       60,    13648 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44859,        6,    13666 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44865,       60,    13670 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44925,       72,    13688 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    44997,       48,    13709 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45045,       36,    13727 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45081,       72,    13741 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45153,       12,    13779 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45165,       12,    13785 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45177,       30,    13791 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45207,       36,    13803 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45243,        6,    13817 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45249,        6,    13821 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45255,       24,    13825 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45279,       24,    13835 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45303,       24,    13845 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45327,       96,    13855 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45423,       30,    13882 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45453,       96,    13894 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45549,       96,    13921 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45645,       72,    13948 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45717,       72,    13969 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45789,       42,    13990 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45831,       42,    14006 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45873,       54,    14022 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45927,       54,    14042 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    45981,       60,    14062 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46041,       60,    14080 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46101,       36,    14098 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46137,       12,    14112 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46149,       12,    14118 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46161,       12,    14124 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46173,       24,    14130 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46197,       24,    14140 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46221,       24,    14150 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46245,       24,    14160 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46269,       24,    14170 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46293,       48,    14180 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46341,       48,    14195 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46389,       48,    14210 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46437,       48,    14225 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46485,       48,    14240 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46533,       36,    14255 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46569,       36,    14269 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46605,       36,    14283 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46641,       36,    14297 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46677,       36,    14311 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46713,       72,    14325 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46785,       72,    14363 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46857,        6,    14401 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46863,        6,    14405 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46869,        6,    14409 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46875,       24,    14413 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46899,       24,    14422 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46923,       48,    14431 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    46971,       48,    14449 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47019,       12,    14464 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47031,       12,    14470 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47043,       12,    14476 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47055,       12,    14482 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47067,       12,    14488 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47079,       12,    14494 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47091,       12,    14500 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47103,       12,    14506 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47115,       24,    14512 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47139,       24,    14521 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47163,       60,    14530 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47223,       60,    14548 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47283,       60,    14566 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47343,       60,    14584 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47403,       60,    14602 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47463,       48,    14620 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47511,       48,    14642 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47559,       36,    14664 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47595,       36,    14678 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47631,       24,    14692 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47655,       12,    14701 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47667,       12,    14707 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47679,       12,    14713 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47691,       12,    14719 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47703,       12,    14725 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47715,       12,    14731 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47727,       12,    14737 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47739,        6,    14743 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47745,        6,    14747 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47751,        6,    14751 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47757,        6,    14755 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47763,       12,    14759 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47775,       12,    14765 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47787,       12,    14771 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47799,       12,    14777 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    47811,       60,    14783 }, // subset0_blinnMisc_Boss_2
        {  10,  11,  -1,    47871,      762,    14801 }, // subset0_lambert1
        {   8,   9,  -1,    48633,       66,    14801 }, // subset1_blinnMisc_Boss_2_0
        {   8,   9,  -1,    48699,      144,    15136 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    48843,      144,    15171 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    48987,      144,    15206 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    49131,      144,    15241 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    49275,      180,    15276 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    49455,      144,    15318 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    49599,      144,    15353 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    49743,      144,    15388 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    49887,      180,    15423 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    50067,      180,    15465 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    50247,      144,    15507 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    50391,      180,    15542 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    50571,      144,    15584 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    50715,      108,    15619 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    50823,     4416,    15647 }, // subset0_blinnMisc_Boss_2
        {   8,   9,  -1,    55239,      273,    16657 }, // subset0_blinnMisc_Boss_2
        {  12,  13,  -1,    55512,     4143,    16754 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    59655,      360,    19212 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    60015,      300,    19293 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    60315,      360,    19359 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    60675,      240,    19436 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    60915,     1566,    19491 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    62481,       48,    19889 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    62529,       48,    19907 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    62577,       48,    19925 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    62625,       48,    19943 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    62673,       48,    19961 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    62721,       48,    19979 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    62769,       96,    19997 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    62865,       36,    20024 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    62901,       36,    20038 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    62937,       36,    20052 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    62973,       36,    20066 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    63009,       36,    20080 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    63045,       36,    20094 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    63081,       36,    20108 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    63117,       36,    20122 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    63153,       36,    20136 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    63189,       36,    20150 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    63225,       36,    20164 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    63261,      108,    20178 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    63369,       36,    20206 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    63405,       36,    20220 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    63441,       36,    20234 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    63477,       36,    20248 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    63513,       36,    20262 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    63549,       36,    20276 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    63585,       36,    20290 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    63621,       72,    20304 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    63693,       72,    20325 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    63765,      144,    20346 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    63909,       72,    20381 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    63981,       36,    20402 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    64017,       36,    20416 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    64053,      144,    20430 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    64197,      144,    20465 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    64341,       36,    20500 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    64377,       36,    20514 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    64413,       36,    20528 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    64449,       36,    20542 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    64485,       36,    20556 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    64521,       36,    20570 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    64557,       36,    20584 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    64593,       36,    20598 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    64629,       36,    20612 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    64665,       36,    20626 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    64701,      252,    20640 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    64953,      276,    20816 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    65229,      276,    21014 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    65505,      276,    21200 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    65781,      294,    21394 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    66075,      279,    21582 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    66354,      264,    21732 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    66618,      288,    21898 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    66906,      468,    22065 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    67374,     1296,    22222 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    68670,       18,    22988 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    68688,       36,    22996 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    68724,       42,    23022 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    68766,       36,    23054 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    68802,       42,    23076 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    68844,      336,    23104 }, // subset0_Stack__Boxes_Diff02
        {  10,  11,  -1,    69180,      144,    23104 }, // subset1_lambert1_0
        {  12,  13,  -1,    69324,      240,    23250 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    69564,      240,    23334 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    69804,      246,    23427 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    70050,      120,    23543 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    70170,      630,    23600 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    70800,     1080,    23743 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    71880,      720,    23991 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    72600,      204,    24154 }, // subset0_Stack__Boxes_Diff02
        {  12,  13,  -1,    72804,      336,    24198 }, // subset0_Stack__Boxes_Diff02
        {  10,  11,  -1,    73140,      144,    24198 }, // subset1_lambert1_0
        {  12,  13,  -1,    73284,      225,    24344 }, // subset0_Stack__Boxes_Diff02
        {  10,  11,  -1,    73509,       15,    24344 }, // subset1_lambert1_0
        {  12,  13,  -1,    73524,      225,    24458 }, // subset0_Stack__Boxes_Diff02
        {  10,  11,  -1,    73749,       15,    24458 }, // subset1_lambert1_0
        {  12,  13,  -1,    73764,      225,    24572 }, // subset0_Stack__Boxes_Diff02
        {  10,  11,  -1,    73989,       15,    24572 }, // subset1_lambert1_0
        {  12,  13,  -1,    74004,      225,    24686 }, // subset0_Stack__Boxes_Diff02
        {  10,  11,  -1,    74229,       15,    24686 }, // subset1_lambert1_0
        {  12,  13,  -1,    74244,      225,    24800 }, // subset0_Stack__Boxes_Diff02
        {  10,  11,  -1,    74469,       15,    24800 }, // subset1_lambert1_0
        {  12,  13,  -1,    74484,      225,    24914 }, // subset0_Stack__Boxes_Diff02
        {  10,  11,  -1,    74709,       15,    24914 }, // subset1_lambert1_0
        {  14,  15,  -1,    74724,     2208,    25028 }, // subset0_Stack__Boxes_Diff03
        {  16,  17,  -1,    76932,     9381,    25553 }, // subset0_Stack__Boxes_Diff01
        {  18,  19,  -1,    86313,      948,    29643 }, // subset0_Back_Alley_box
        {  20,  21,  -1,    87261,     1488,    29946 }, // subset0_gameCrates_01_Diff
        {  20,  21,  -1,    88749,     1998,    30265 }, // subset0_gameCrates_01_Diff
        {  20,  21,  -1,    90747,     1872,    30649 }, // subset0_gameCrates_01_Diff
        {  20,  21,  -1,    92619,     1692,    31034 }, // subset0_gameCrates_01_Diff
        {  20,  21,  -1,    94311,     1638,    31387 }, // subset0_gameCrates_01_Diff
        {  20,  21,  -1,    95949,     1872,    31702 }, // subset0_gameCrates_01_Diff
        {  20,  21,  -1,    97821,     1662,    32056 }, // subset0_gameCrates_01_Diff
        {  20,  21,  -1,    99483,     1902,    32409 }, // subset0_gameCrates_01_Diff
        {  20,  21,  -1,   101385,      720,    32807 }, // subset0_gameCrates_01_Diff
        {  20,  21,  -1,   102105,     1296,    32957 }, // subset0_gameCrates_01_Diff
        {  20,  21,  -1,   103401,      432,    33232 }, // subset0_gameCrates_01_Diff
        {  20,  21,  -1,   103833,     1152,    33332 }, // subset0_gameCrates_01_Diff
        {  20,  21,  -1,   104985,     1152,    33557 }, // subset0_gameCrates_01_Diff
        {  20,  21,  -1,   106137,     1380,    33782 }, // subset0_gameCrates_01_Diff
        {  20,  21,  -1,   107517,     1470,    34080 }, // subset0_gameCrates_01_Diff
        {  20,  21,  -1,   108987,     1308,    34364 }, // subset0_gameCrates_01_Diff
        {  20,  21,  -1,   110295,     1662,    34622 }, // subset0_gameCrates_01_Diff
        {  22,  23,  -1,   111957,      636,    34941 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   112593,     1728,    35137 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   114321,      336,    35500 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   114657,     2304,    35577 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   116961,      246,    36138 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   117207,     1260,    36270 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   118467,     1866,    36512 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   120333,     2070,    36976 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   122403,     1050,    37421 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   123453,      450,    37624 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   123903,      450,    37760 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   124353,      390,    37896 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   124743,      390,    38021 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   125133,      390,    38148 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   125523,      330,    38273 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   125853,      390,    38387 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   126243,      330,    38512 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   126573,      420,    38626 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   126993,      390,    38763 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   127383,      300,    38888 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   127683,      300,    38961 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   127983,      300,    39045 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   128283,      300,    39131 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   128583,      300,    39215 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   128883,      300,    39286 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   129183,      300,    39410 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   129483,      300,    39483 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   129783,      300,    39567 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   130083,      300,    39653 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   130383,      300,    39737 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   130683,      300,    39808 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   130983,      300,    39968 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   131283,      300,    40041 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   131583,      300,    40125 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   131883,      300,    40211 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   132183,      300,    40295 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   132483,      300,    40366 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   132783,      300,    40490 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   133083,      300,    40561 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   133383,      300,    40721 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   133683,      300,    40794 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   133983,      300,    40878 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   134283,      300,    40964 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   134583,      300,    41048 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   134883,      300,    41119 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   135183,      300,    41243 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   135483,      300,    41314 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   135783,      300,    41474 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   136083,      300,    41547 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   136383,      300,    41631 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   136683,      300,    41717 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   136983,      300,    41801 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   137283,      300,    41872 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   137583,      300,    41996 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   137883,      300,    42069 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   138183,      300,    42153 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   138483,      300,    42239 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   138783,      300,    42323 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   139083,      300,    42394 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   139383,      300,    42554 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   139683,      300,    42627 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   139983,      300,    42711 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   140283,      300,    42797 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   140583,      300,    42881 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   140883,      300,    42952 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   141183,      300,    43076 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   141483,      300,    43149 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   141783,      300,    43349 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   142083,      300,    43551 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   142383,      300,    43751 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   142683,      300,    43951 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   142983,      300,    44151 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   143283,      300,    44351 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   143583,      300,    44551 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   143883,      300,    44751 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   144183,      300,    44951 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   144483,      300,    45153 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   144783,      300,    45353 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   145083,      300,    45553 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   145383,      300,    45753 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   145683,      300,    45953 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   145983,      300,    46153 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   146283,      300,    46353 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   146583,      300,    46553 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   146883,      300,    46755 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   147183,      300,    46955 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   147483,      300,    47155 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   147783,      300,    47355 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   148083,      300,    47555 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   148383,      300,    47755 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   148683,      300,    47955 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   148983,      300,    48155 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   149283,      300,    48355 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   149583,      300,    48557 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   149883,      300,    48757 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   150183,      300,    48957 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   150483,      300,    49157 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   150783,      216,    49357 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   150999,      216,    49455 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   151215,      216,    49553 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   151431,     1014,    49651 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   152445,      216,    49929 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   152661,     1014,    50029 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   153675,     1014,    50324 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   154689,     1014,    50614 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   155703,      588,    50894 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   156291,     5574,    51104 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   161865,      588,    52700 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   162453,      588,    52910 }, // subset0_RaceCar_Strorage_Diff
        {  22,  23,  -1,   163041,      588,    53120 }, // subset0_RaceCar_Strorage_Diff
        {  24,  25,  -1,   163629,       48,    53330 }, // subset0_hats_2
        {  24,  25,  -1,   163677,      501,    53362 }, // subset0_hats_2
        {  24,  25,  -1,   164178,      252,    53471 }, // subset0_hats_2
        {  24,  25,  -1,   164430,       48,    53520 }, // subset0_hats_2
        {  24,  25,  -1,   164478,     1620,    53552 }, // subset0_hats_2
        {  24,  25,  -1,   166098,     3978,    53850 }, // subset0_hats_2
        {  24,  25,  -1,   170076,      765,    54754 }, // subset0_hats_2
        {  24,  25,  -1,   170841,      900,    54906 }, // subset0_hats_2
        {  24,  25,  -1,   171741,     1260,    55154 }, // subset0_hats_2
        {  24,  25,  -1,   173001,      408,    55431 }, // subset0_hats_2
        {  24,  25,  -1,   173409,      765,    55546 }, // subset0_hats_2
        {  24,  25,  -1,   174174,      720,    55694 }, // subset0_hats_2
        {  24,  25,  -1,   174894,      252,    55894 }, // subset0_hats_2
        {  24,  25,  -1,   175146,      252,    55943 }, // subset0_hats_2
        {  24,  25,  -1,   175398,      252,    55992 }, // subset0_hats_2
        {  24,  25,  -1,   175650,      252,    56041 }, // subset0_hats_2
        {  24,  25,  -1,   175902,      252,    56090 }, // subset0_hats_2
        {  24,  25,  -1,   176154,      252,    56139 }, // subset0_hats_2
        {  24,  25,  -1,   176406,      252,    56188 }, // subset0_hats_2
        {  24,  25,  -1,   176658,      252,    56237 }, // subset0_hats_2
        {  26,  27,  -1,   176910,       48,    56286 }, // subset0_Hats_1
        {  24,  25,  -1,   176958,     6792,    56318 }, // subset0_hats_2
        {  24,  25,  -1,   183750,     2124,    57790 }, // subset0_hats_2
        {  24,  25,  -1,   185874,     2268,    58361 }, // subset0_hats_2
        {  24,  25,  -1,   188142,       66,    58938 }, // subset0_hats_2
        {  24,  25,  -1,   188208,      270,    58973 }, // subset0_hats_2
        {  24,  25,  -1,   188478,       48,    59080 }, // subset0_hats_2
        {  24,  25,  -1,   188526,       66,    59111 }, // subset0_hats_2
        {  24,  25,  -1,   188592,       60,    59148 }, // subset0_hats_2
        {  24,  25,  -1,   188652,       66,    59178 }, // subset0_hats_2
        {  24,  25,  -1,   188718,      168,    59215 }, // subset0_hats_2
        {  24,  25,  -1,   188886,      168,    59275 }, // subset0_hats_2
        {  24,  25,  -1,   189054,      174,    59333 }, // subset0_hats_2
        {  24,  25,  -1,   189228,     1650,    59418 }, // subset0_hats_2
        {  24,  25,  -1,   190878,      918,    59867 }, // subset0_hats_2
        {  24,  25,  -1,   191796,      426,    60089 }, // subset0_hats_2
        {  26,  27,  -1,   192222,     1209,    60211 }, // subset0_Hats_1
        {  24,  25,  -1,   193431,      180,    60506 }, // subset0_hats_2
        {  26,  27,  -1,   193611,      714,    60571 }, // subset0_Hats_1
        {  26,  27,  -1,   194325,      426,    60759 }, // subset0_Hats_1
        {  26,  27,  -1,   194751,      426,    60888 }, // subset0_Hats_1
        {  26,  27,  -1,   195177,      432,    61010 }, // subset0_Hats_1
        {  26,  27,  -1,   195609,      432,    61147 }, // subset0_Hats_1
        {  26,  27,  -1,   196041,     1608,    61268 }, // subset0_Hats_1
        {  28,  29,  -1,   197649,    18219,    61783 }, // subset0_Misc_Boss_1
        {  16,  17,  -1,   215868,     2484,    69708 }, // subset0_Stack__Boxes_Diff01
        {  16,  17,  -1,   218352,     2409,    71274 }, // subset0_Stack__Boxes_Diff01
        {  16,  17,  -1,   220761,     2409,    72652 }, // subset0_Stack__Boxes_Diff01
        {  16,  17,  -1,   223170,     2409,    74026 }, // subset0_Stack__Boxes_Diff01
        {  16,  17,  -1,   225579,     1833,    75404 }, // subset0_Stack__Boxes_Diff01
        {  30,  31,  -1,   227412,     3867,    76541 }, // subset0_gameCrates_03_Diff
        {  30,  31,  -1,   231279,     3750,    78113 }, // subset0_gameCrates_03_Diff
        {  16,  17,  -1,   235029,     2409,    79856 }, // subset0_Stack__Boxes_Diff01
        {  16,  17,  -1,   237438,     2409,    81222 }, // subset0_Stack__Boxes_Diff01
        {  30,  31,  -1,   239847,    12468,    82594 }, // subset0_gameCrates_03_Diff
        {  30,  31,  -1,   252315,     3441,    88547 }, // subset0_gameCrates_03_Diff
        {  32,  32,  -1,   255756,     1932,    89479 }, // subset0_gameCrates_02_Diff
        {  14,  15,  -1,   257688,      984,    89905 }, // subset0_Stack__Boxes_Diff03
        {  14,  15,  -1,   258672,      741,    90403 }, // subset0_Stack__Boxes_Diff03
        {  14,  15,  -1,   259413,      426,    90727 }, // subset0_Stack__Boxes_Diff03
        {  32,  32,  -1,   259839,     1419,    90932 }, // subset0_gameCrates_02_Diff
        {  18,  19,  -1,   261258,     3090,    91251 }, // subset0_Back_Alley_box
        {  14,  15,  -1,   264348,      573,    92286 }, // subset0_Stack__Boxes_Diff03
        {  32,  32,  -1,   264921,     1176,    92524 }, // subset0_gameCrates_02_Diff
        {  32,  32,  -1,   266097,      792,    92809 }, // subset0_gameCrates_02_Diff
        {  14,  15,  -1,   266889,      573,    93047 }, // subset0_Stack__Boxes_Diff03
        {  14,  15,  -1,   267462,      741,    93285 }, // subset0_Stack__Boxes_Diff03
        {  18,  19,  -1,   268203,     1620,    93609 }, // subset0_Back_Alley_box
        {  14,  15,  -1,   269823,      396,    94126 }, // subset0_Stack__Boxes_Diff03
        {  14,  15,  -1,   270219,      396,    94281 }, // subset0_Stack__Boxes_Diff03
        {  14,  15,  -1,   270615,      396,    94436 }, // subset0_Stack__Boxes_Diff03
        {  32,  32,  -1,   271011,     1008,    94591 }, // subset0_gameCrates_02_Diff
        {  32,  32,  -1,   272019,     2040,    94830 }, // subset0_gameCrates_02_Diff
        {  14,  15,  -1,   274059,      468,    95269 }, // subset0_Stack__Boxes_Diff03
        {  32,  32,  -1,   274527,     1236,    95437 }, // subset0_gameCrates_02_Diff
        {  32,  32,  -1,   275763,      903,    95726 }, // subset0_gameCrates_02_Diff
        {  32,  32,  -1,   276666,     1884,    95942 }, // subset0_gameCrates_02_Diff
        {  32,  32,  -1,   278550,     2088,    96353 }, // subset0_gameCrates_02_Diff
        {  32,  32,  -1,   280638,     1008,    96827 }, // subset0_gameCrates_02_Diff
        {  33,  34,  -1,   281646,     1920,    97063 }, // subset0_Back_Alley_Drum
        {  35,  36,  -1,   283566,     7809,    97728 }, // subset0_shelves2_diff
        {  35,  36,  -1,   291375,     8172,    99390 }, // subset0_shelves2_diff
        {  37,  38,  -1,   299547,       12,   101144 }, // subset0_shelves2_diff1
        {  37,  38,  -1,   299559,      132,   101150 }, // subset0_shelves2_diff1
        {  37,  38,  -1,   299691,      132,   101218 }, // subset0_shelves2_diff1
        {  37,  38,  -1,   299823,      228,   101286 }, // subset0_shelves2_diff1
        {  37,  38,  -1,   300051,      228,   101382 }, // subset0_shelves2_diff1
        {  37,  38,  -1,   300279,      228,   101485 }, // subset0_shelves2_diff1
        {  37,  38,  -1,   300507,      228,   101581 }, // subset0_shelves2_diff1
        {  37,  38,  -1,   300735,      228,   101689 }, // subset0_shelves2_diff1
        {  37,  38,  -1,   300963,      228,   101785 }, // subset0_shelves2_diff1
        {  37,  38,  -1,   301191,      852,   101881 }, // subset0_shelves2_diff1
        {  37,  38,  -1,   302043,      702,   102217 }, // subset0_shelves2_diff1
        {  37,  38,  -1,   302745,      372,   102461 }, // subset0_shelves2_diff1
        {  37,  38,  -1,   303117,      564,   102637 }, // subset0_shelves2_diff1
        {  37,  38,  -1,   303681,      324,   102857 }, // subset0_shelves2_diff1
        {  37,  38,  -1,   304005,      324,   102977 }, // subset0_shelves2_diff1
        {  37,  38,  -1,   304329,      588,   103097 }, // subset0_shelves2_diff1
        {   6,   7,  -1,   304917,     1122,   103337 }, // subset0_shelves1_diff2
        {   6,   7,  -1,   306039,      888,   103622 }, // subset0_shelves1_diff2
        {   6,   7,  -1,   306927,       96,   103854 }, // subset0_shelves1_diff2
        {   6,   7,  -1,   307023,       96,   103902 }, // subset0_shelves1_diff2
        {   6,   7,  -1,   307119,      102,   103947 }, // subset0_shelves1_diff2
        {   6,   7,  -1,   307221,       96,   103987 }, // subset0_shelves1_diff2
        {   6,   7,  -1,   307317,       96,   104032 }, // subset0_shelves1_diff2
        {   6,   7,  -1,   307413,      576,   104078 }, // subset0_shelves1_diff2
        {   6,   7,  -1,   307989,       78,   104208 }, // subset0_shelves1_diff2
        {   6,   7,  -1,   308067,      102,   104240 }, // subset0_shelves1_diff2
        {   6,   7,  -1,   308169,       72,   104284 }, // subset0_shelves1_diff2
        {   6,   7,  -1,   308241,      336,   104315 }, // subset0_shelves1_diff2
        {   6,   7,  -1,   308577,      144,   104450 }, // subset0_shelves1_diff2
        {   6,   7,  -1,   308721,      120,   104490 }, // subset0_shelves1_diff2
        {   6,   7,  -1,   308841,      144,   104518 }, // subset0_shelves1_diff2
        {   6,   7,  -1,   308985,      120,   104558 }, // subset0_shelves1_diff2
        {   6,   7,  -1,   309105,      126,   104586 }, // subset0_shelves1_diff2
        {   6,   7,  -1,   309231,      120,   104624 }, // subset0_shelves1_diff2
        {   6,   7,  -1,   309351,      144,   104652 }, // subset0_shelves1_diff2
        {   6,   7,  -1,   309495,      117,   104692 }, // subset0_shelves1_diff2
        {   6,   7,  -1,   309612,     1572,   104720 }, // subset0_shelves1_diff2
        {  39,  40,  -1,   311184,      540,   105150 }, // subset0_marbel_drum_blinn
        {  39,  40,  -1,   311724,     2136,   105282 }, // subset0_marbel_drum_blinn
        {  39,  40,  -1,   313860,       24,   105899 }, // subset0_marbel_drum_blinn
        {  39,  40,  -1,   313884,       24,   105911 }, // subset0_marbel_drum_blinn
        {  39,  40,  -1,   313908,       27,   105923 }, // subset0_marbel_drum_blinn
        {  39,  40,  -1,   313935,     2964,   105936 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   316899,     2964,   106458 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   319863,      702,   106980 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   320565,      702,   107117 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   321267,      702,   107249 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   321969,      702,   107379 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   322671,     2964,   107511 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   325635,        6,   108033 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   325641,      432,   108037 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   326073,      429,   108130 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   326502,     2964,   108216 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   329466,     2106,   108738 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   331572,     2106,   109129 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   333678,      429,   109515 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   334107,     1794,   109614 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   335901,     2262,   109948 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   338163,     2964,   110365 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   341127,     2964,   110880 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   344091,     2964,   111402 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   347055,     2730,   111919 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   349785,     2964,   112407 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   352749,     2964,   112929 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   355713,     2964,   113444 }, // subset0_marbel_drum_phong
        {  39,  40,  -1,   358677,     3276,   113961 }, // subset0_marbel_drum_blinn
        {  39,  40,  -1,   361953,      780,   114605 }, // subset0_marbel_drum_blinn
        {  39,  40,  -1,   362733,      936,   114773 }, // subset0_marbel_drum_blinn
        {  39,  40,  -1,   363669,      936,   114969 }, // subset0_marbel_drum_blinn
        {  41,  42,  -1,   364605,      108,   115165 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   364713,       81,   115185 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   364794,       81,   115225 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   364875,       81,   115255 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   364956,       81,   115287 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   365037,      108,   115328 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   365145,       81,   115348 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   365226,       81,   115388 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   365307,       81,   115418 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   365388,      108,   115450 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   365496,       81,   115470 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   365577,      108,   115510 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   365685,       81,   115530 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   365766,       81,   115570 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   365847,       81,   115602 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   365928,      108,   115641 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   366036,       81,   115669 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   366117,       81,   115709 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   366198,       81,   115741 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   366279,      108,   115780 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   366387,       81,   115808 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   366468,       81,   115847 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   366549,       81,   115879 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   366630,       81,   115909 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   366711,      108,   115948 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   366819,       81,   115976 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   366900,       81,   116006 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   366981,      108,   116038 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   367089,       81,   116066 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   367170,       81,   116105 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   367251,      108,   116137 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   367359,       81,   116165 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   367440,       81,   116204 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   367521,      237,   116234 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   367758,      216,   116293 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   367974,      216,   116348 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   368190,      240,   116398 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   368430,      216,   116453 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   368646,      216,   116503 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   368862,      324,   116558 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   369186,      108,   116650 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   369294,      324,   116688 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   369618,      108,   116780 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   369726,      324,   116818 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   370050,      108,   116910 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   370158,      324,   116948 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   370482,      108,   117040 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   370590,      324,   117078 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   370914,      108,   117170 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   371022,      324,   117208 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   371346,      108,   117300 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   371454,       54,   117338 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   371508,       54,   117357 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   371562,       54,   117376 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   371616,       54,   117395 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   371670,       54,   117414 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   371724,       54,   117433 }, // subset0_blinnCatwalk
        {  43,  11,  -1,   371778,      120,   117452 }, // subset0_Catwalk_03_Diffuse
        {  43,  11,  -1,   371898,     3078,   117532 }, // subset0_Catwalk_03_Diffuse
        {  43,  11,  -1,   374976,     4356,   118172 }, // subset0_Catwalk_03_Diffuse
        {  43,  11,  -1,   379332,     4356,   119043 }, // subset0_Catwalk_03_Diffuse
        {  43,  11,  -1,   383688,     4356,   119896 }, // subset0_Catwalk_03_Diffuse
        {  43,  11,  -1,   388044,     3636,   120749 }, // subset0_Catwalk_03_Diffuse
        {  41,  42,  -1,   391680,       72,   121481 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   391752,      162,   121513 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   391914,      324,   121553 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   392238,      144,   121630 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   392382,      144,   121665 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   392526,      162,   121705 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   392688,      324,   121745 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   393012,      144,   121822 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   393156,      144,   121857 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   393300,      162,   121897 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   393462,      360,   121937 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   393822,      144,   122014 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   393966,      144,   122049 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   394110,      162,   122084 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   394272,      324,   122124 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   394596,      153,   122194 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   394749,      144,   122234 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   394893,      162,   122269 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   395055,      324,   122309 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   395379,      144,   122379 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   395523,      144,   122419 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   395667,      162,   122459 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   395829,      324,   122503 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   396153,      150,   122573 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   396303,      144,   122621 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   396447,      162,   122661 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   396609,      324,   122701 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   396933,      144,   122778 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   397077,      144,   122818 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   397221,      162,   122853 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   397383,      324,   122897 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   397707,      141,   122967 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   397848,      144,   123006 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   397992,      162,   123041 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   398154,    19506,   123085 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   417660,    19506,   129867 }, // subset0_blinnCatwalk
        {  43,  11,  -1,   437166,      120,   136640 }, // subset0_Catwalk_03_Diffuse
        {  41,  42,  -1,   437286,       72,   136720 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437358,       24,   136752 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437382,       24,   136768 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437406,       24,   136784 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437430,       24,   136800 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437454,       24,   136816 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437478,       24,   136832 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437502,       24,   136848 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437526,       24,   136864 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437550,       24,   136880 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437574,       24,   136896 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437598,       24,   136912 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437622,       24,   136928 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437646,       24,   136944 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437670,       24,   136960 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437694,       24,   136976 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437718,       24,   136992 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437742,       24,   137008 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437766,       24,   137024 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437790,       24,   137040 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437814,       24,   137056 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437838,       24,   137072 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437862,       24,   137088 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437886,       24,   137104 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437910,       24,   137120 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437934,       24,   137136 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437958,       24,   137152 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   437982,       24,   137168 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438006,       24,   137184 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438030,       24,   137200 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438054,       24,   137216 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438078,       24,   137232 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438102,       24,   137248 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438126,       24,   137264 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438150,       24,   137280 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438174,       24,   137296 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438198,       24,   137312 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438222,       24,   137328 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438246,       24,   137344 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438270,       24,   137360 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438294,       24,   137376 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438318,       24,   137392 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438342,       24,   137408 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438366,       24,   137424 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438390,       24,   137440 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438414,       24,   137456 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438438,       24,   137472 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438462,       24,   137488 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438486,       24,   137504 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438510,       24,   137520 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438534,       24,   137536 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438558,       24,   137552 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438582,       24,   137568 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438606,       24,   137584 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438630,       24,   137600 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438654,       24,   137616 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438678,       24,   137632 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438702,       24,   137648 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438726,       24,   137664 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438750,       24,   137680 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438774,       24,   137696 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438798,       24,   137712 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438822,       24,   137728 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438846,       24,   137744 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438870,       24,   137760 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438894,       24,   137776 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438918,       24,   137792 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438942,       24,   137808 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438966,       24,   137824 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   438990,       24,   137840 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439014,       24,   137856 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439038,       24,   137872 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439062,       24,   137888 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439086,       24,   137904 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439110,       24,   137920 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439134,       24,   137936 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439158,       24,   137952 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439182,       24,   137968 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439206,       24,   137984 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439230,       24,   138000 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439254,       24,   138016 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439278,       24,   138032 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439302,       24,   138048 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439326,       24,   138064 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439350,       24,   138080 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439374,       24,   138096 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439398,       24,   138112 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439422,       24,   138128 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439446,       24,   138144 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439470,       24,   138160 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439494,       24,   138176 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439518,       24,   138192 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439542,       24,   138208 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439566,       24,   138224 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439590,       24,   138240 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439614,       24,   138256 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439638,       24,   138272 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439662,       24,   138288 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439686,       24,   138304 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439710,       24,   138320 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439734,       24,   138336 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439758,       24,   138352 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439782,       24,   138368 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439806,       24,   138384 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439830,       24,   138400 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439854,       24,   138416 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439878,       24,   138432 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439902,       24,   138448 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439926,       24,   138464 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439950,       24,   138480 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439974,       24,   138496 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   439998,       24,   138512 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440022,       24,   138528 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440046,       24,   138544 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440070,       24,   138560 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440094,       24,   138576 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440118,       24,   138592 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440142,       24,   138608 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440166,       24,   138624 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440190,       24,   138640 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440214,       24,   138656 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440238,       24,   138672 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440262,       24,   138688 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440286,       24,   138704 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440310,       24,   138720 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440334,       24,   138736 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440358,       24,   138752 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440382,       24,   138768 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440406,       24,   138784 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440430,       24,   138800 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440454,       24,   138816 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440478,       24,   138832 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440502,       24,   138848 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440526,       24,   138864 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440550,       24,   138880 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440574,       24,   138896 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440598,       24,   138912 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440622,       24,   138928 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440646,       24,   138944 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440670,       24,   138960 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440694,       24,   138976 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440718,       24,   138992 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440742,       24,   139008 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440766,       24,   139024 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440790,       24,   139040 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440814,       24,   139056 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440838,       24,   139072 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440862,       24,   139088 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440886,       24,   139104 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440910,       24,   139120 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440934,       24,   139136 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440958,       24,   139152 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   440982,       24,   139168 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   441006,       24,   139184 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   441030,       24,   139200 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   441054,       24,   139216 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   441078,       24,   139232 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   441102,       24,   139248 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   441126,       24,   139264 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   441150,       24,   139280 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   441174,       24,   139296 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   441198,       24,   139312 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   441222,       24,   139328 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   441246,       24,   139344 }, // subset0_blinnCatwalk
        {  41,  42,  -1,   441270,       24,   139360 }, // subset0_blinnCatwalk
        {  44,  45,  -1,   441294,     2826,   139376 }, // subset0_shelves3_diff
        {  35,  36,  -1,   444120,     2637,   139376 }, // subset1_shelves2_diff_0
        {  14,  15,  -1,   446757,      636,   139376 }, // subset2_Stack__Boxes_Diff03_0
        {  46,  47,  -1,   447393,       24,   141190 }, // subset0_blinn46
        {  46,  47,  -1,   447417,      306,   141206 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   447723,      306,   141325 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   448029,      306,   141443 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   448335,      306,   141561 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   448641,      216,   141677 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   448857,     1812,   141759 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   450669,      306,   142479 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   450975,      222,   142604 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   451197,      132,   142704 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   451329,      222,   142754 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   451551,      306,   142854 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   451857,      132,   142975 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   451989,      222,   143025 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   452211,      306,   143125 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   452517,      306,   143247 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   452823,      132,   143370 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   452955,      222,   143420 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   453177,      132,   143520 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   453309,     1368,   143570 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   454677,     2484,   143914 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   457161,     5790,   144731 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   462951,     1836,   147045 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   464787,     3816,   147679 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   468603,    54240,   149038 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   522843,      792,   158991 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   523635,      792,   159136 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   524427,      216,   159281 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   524643,      216,   159363 }, // subset0_phongMisc_Boss_3_2048
        {  46,  47,  -1,   524859,      216,   159445 }, // subset0_phongMisc_Boss_3_2048
        {  48,  42,  -1,   525075,     1050,   159527 }, // subset0_LightBulbsColor
        {  49,  50,  -1,   526125,     1128,   159773 }, // subset0_Hanghing_Light
        {  10,  11,  -1,   527253,      540,   160003 }, // subset0_lambert1
        {  10,  11,  -1,   527793,      225,   160123 }, // subset0_lambert1
        {  49,  50,  -1,   528018,     2160,   160202 }, // subset0_Hanghing_Light
        {  48,  42,  -1,   530178,     1050,   160707 }, // subset0_LightBulbsColor
        {  49,  50,  -1,   531228,     1128,   160966 }, // subset0_Hanghing_Light
        {  10,  11,  -1,   532356,      540,   161197 }, // subset0_lambert1
        {  10,  11,  -1,   532896,      225,   161317 }, // subset0_lambert1
        {  49,  50,  -1,   533121,     2160,   161391 }, // subset0_Hanghing_Light
        {  48,  42,  -1,   535281,     1134,   161896 }, // subset0_LightBulbsColor
        {  49,  50,  -1,   536415,     1128,   162168 }, // subset0_Hanghing_Light
        {  49,  50,  -1,   537543,      600,   162399 }, // subset0_Hanghing_Light
        {  49,  50,  -1,   538143,      225,   162531 }, // subset0_Hanghing_Light
        {  49,  50,  -1,   538368,     2160,   162610 }, // subset0_Hanghing_Light
        {  10,  51,  -1,   540528,      498,   163115 }, // subset0_Hanging_bundle
        {  10,  51,  -1,   541026,      300,   163321 }, // subset0_Hanging_bundle
        {  10,  51,  -1,   541326,      300,   163400 }, // subset0_Hanging_bundle
        {  10,  51,  -1,   541626,      600,   163479 }, // subset0_Hanging_bundle
        {  10,  51,  -1,   542226,      681,   163604 }, // subset0_Hanging_bundle
        {  10,  51,  -1,   542907,      300,   163755 }, // subset0_Hanging_bundle
        {  10,  51,  -1,   543207,      300,   163834 }, // subset0_Hanging_bundle
        {  10,  51,  -1,   543507,      144,   163913 }, // subset0_Hanging_bundle
        {  10,  51,  -1,   543651,      780,   163949 }, // subset0_Hanging_bundle
        {  10,  51,  -1,   544431,      168,   164135 }, // subset0_Hanging_bundle
        {  10,  51,  -1,   544599,      156,   164197 }, // subset0_Hanging_bundle
        {  10,  51,  -1,   544755,      390,   164255 }, // subset0_Hanging_bundle
        {  10,  51,  -1,   545145,     3702,   164393 }, // subset0_Hanging_bundle
        {  10,  51,  -1,   548847,      468,   165239 }, // subset0_Hanging_bundle
        {  10,  51,  -1,   549315,     1128,   165324 }, // subset0_Hanging_bundle
        {  10,  51,  -1,   550443,      810,   165562 }, // subset0_Hanging_bundle
        {  52,  53,  -1,   551253,      714,   165819 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   551967,      540,   165949 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   552507,     1080,   166048 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   553587,     1080,   166242 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   554667,      540,   166436 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   555207,      540,   166535 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   555747,     1080,   166642 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   556827,      540,   166836 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   557367,     1080,   166935 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   558447,     1080,   167129 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   559527,      540,   167323 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   560067,     1080,   167422 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   561147,     1080,   167616 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   562227,     1080,   167810 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   563307,     1080,   168001 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   564387,      714,   168195 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   565101,     1080,   168325 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   566181,      540,   168519 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   566721,      540,   168618 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   567261,      540,   168719 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   567801,     1080,   168818 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   568881,      540,   169012 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   569421,     1080,   169111 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   570501,      540,   169305 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   571041,      714,   169404 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   571755,     1080,   169534 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   572835,     1080,   169728 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   573915,      540,   169922 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   574455,      540,   170021 }, // subset0_Hanging_bundle_marble
        {  52,  53,  -1,   574995,     1080,   170120 }, // subset0_Hanging_bundle_marble
        {  10,  51,  -1,   576075,    17184,   170314 }, // subset0_Rope
        {  10,  51,  -1,   593259,      576,   176140 }, // subset0_Rope
        {  10,  51,  -1,   593835,     2556,   176293 }, // subset0_Rope
        {  10,  51,  -1,   596391,     4032,   176950 }, // subset0_Rope
        {  10,  51,  -1,   600423,     4284,   177857 }, // subset0_Rope
        {  10,  51,  -1,   604707,     4536,   178939 }, // subset0_Rope
        {  10,  51,  -1,   609243,     4536,   179837 }, // subset0_Rope
        {  10,  51,  -1,   613779,      612,   180733 }, // subset0_Rope
        {  44,  45,  -1,   614391,      396,   180879 }, // subset0_shelves3_diff
        {  44,  45,  -1,   614787,      600,   181009 }, // subset0_shelves3_diff
        {  44,  45,  -1,   615387,      600,   181216 }, // subset0_shelves3_diff
        {  44,  45,  -1,   615987,      744,   181423 }, // subset0_shelves3_diff
        {  44,  45,  -1,   616731,      729,   181680 }, // subset0_shelves3_diff
        {  44,  45,  -1,   617460,     1920,   181932 }, // subset0_shelves3_diff
        {  44,  45,  -1,   619380,      324,   182343 }, // subset0_shelves3_diff
        {  44,  45,  -1,   619704,     2274,   182475 }, // subset0_shelves3_diff
        {  44,  45,  -1,   621978,     1860,   182902 }, // subset0_shelves3_diff
        {  44,  45,  -1,   623838,     2376,   183256 }, // subset0_shelves3_diff
        {  44,  45,  -1,   626214,     2190,   183705 }, // subset0_shelves3_diff
        {  44,  45,  -1,   628404,     2280,   184118 }, // subset0_shelves3_diff
        {  44,  45,  -1,   630684,      840,   184550 }, // subset0_shelves3_diff
        {  44,  45,  -1,   631524,      840,   184754 }, // subset0_shelves3_diff
        {  44,  45,  -1,   632364,      840,   184958 }, // subset0_shelves3_diff
        {  44,  45,  -1,   633204,      840,   185162 }, // subset0_shelves3_diff
        {  44,  45,  -1,   634044,     1008,   185366 }, // subset0_shelves3_diff
        {  44,  45,  -1,   635052,     1332,   185608 }, // subset0_shelves3_diff
        {  44,  45,  -1,   636384,     1617,   185890 }, // subset0_shelves3_diff
        {  44,  45,  -1,   638001,     1410,   186290 }, // subset0_shelves3_diff
        {  44,  45,  -1,   639411,      948,   186589 }, // subset0_shelves3_diff
        {  44,  45,  -1,   640359,       90,   187035 }, // subset0_shelves3_diff
        {  44,  45,  -1,   640449,      504,   187055 }, // subset0_shelves3_diff
        {  35,  36,  -1,   640953,      378,   187197 }, // subset0_shelves2_diff
        {  35,  36,  -1,   641331,      432,   187284 }, // subset0_shelves2_diff
        {  35,  36,  -1,   641763,      378,   187378 }, // subset0_shelves2_diff
        {  35,  36,  -1,   642141,     3120,   187462 }, // subset0_shelves2_diff
        {  35,  36,  -1,   645261,      402,   188387 }, // subset0_shelves2_diff
        {  35,  36,  -1,   645663,     2580,   188525 }, // subset0_shelves2_diff
        {  35,  36,  -1,   648243,     3180,   189008 }, // subset0_shelves2_diff
        {  35,  36,  -1,   651423,     2100,   189678 }, // subset0_shelves2_diff
        {  35,  36,  -1,   653523,     3180,   190158 }, // subset0_shelves2_diff
        {  35,  36,  -1,   656703,      180,   190828 }, // subset0_shelves2_diff
        {  35,  36,  -1,   656883,      276,   190872 }, // subset0_shelves2_diff
        {  35,  36,  -1,   657159,      180,   190970 }, // subset0_shelves2_diff
        {  35,  36,  -1,   657339,      180,   191014 }, // subset0_shelves2_diff
        {  44,  45,  -1,   657519,      228,   191068 }, // subset0_shelves3_diff
        {  44,  45,  -1,   657747,      108,   191162 }, // subset0_shelves3_diff
        {  35,  36,  -1,   657855,      192,   191197 }, // subset0_shelves2_diff
        {  35,  36,  -1,   658047,      192,   191271 }, // subset0_shelves2_diff
        {  35,  36,  -1,   658239,      192,   191345 }, // subset0_shelves2_diff
        {  35,  36,  -1,   658431,      168,   191419 }, // subset0_shelves2_diff
        {  35,  36,  -1,   658599,      168,   191485 }, // subset0_shelves2_diff
        {  35,  36,  -1,   658767,      168,   191551 }, // subset0_shelves2_diff
        {  35,  36,  -1,   658935,      192,   191617 }, // subset0_shelves2_diff
        {  35,  36,  -1,   659127,      192,   191691 }, // subset0_shelves2_diff
        {  35,  36,  -1,   659319,      168,   191765 }, // subset0_shelves2_diff
        {  44,  45,  -1,   659487,     1617,   191831 }, // subset0_shelves3_diff
        {  44,  45,  -1,   661104,     1617,   192226 }, // subset0_shelves3_diff
        {  44,  45,  -1,   662721,     1617,   192605 }, // subset0_shelves3_diff
        {  44,  45,  -1,   664338,       90,   192995 }, // subset0_shelves3_diff
        {  54,  42,  -1,   664428,     1248,   193015 }, // subset0_Window
        {  55,  56,  -1,   665676,     3312,   193319 }, // subset0_Sliding_Steel_Door
        {  54,  57,  -1,   668988,      930,   194197 }, // subset0_window_Diff
        {  58,  59,  -1,   669918,    12912,   194433 }, // subset0_Door2
        {  60,  61,  -1,   682830,      969,   197161 }, // subset0_Floor
        {  62,  63,  -1,   683799,     2037,   197482 }, // subset0_wall_4
        {  62,  63,  -1,   685836,     3000,   198168 }, // subset0_wall_3
        {  64,  65,  -1,   688836,     2205,   199986 }, // subset0_wall_1
        {  64,  65,  -1,   691041,     1656,   200663 }, // subset0_wall_2
        {  66,  67,  -1,   692697,     5424,   201041 }, // subset0_roof
        {  68,  69,  -1,   698121,      939,   203677 }, // subset0_Pillar
        {  66,  67,  -1,   699060,     1536,   203892 }, // subset0_roof
        {  70,  71,  -1,   700596,      609,   204339 }, // subset0_Broken_Pillar_Diff
        {  33,  34,  -1,   701205,     1920,   204469 }, // subset0_Back_Alley_Drum
        {  33,  34,  -1,   703125,     1920,   205134 }, // subset0_Back_Alley_Drum
        {  33,  34,  -1,   705045,     1920,   205799 }, // subset0_Back_Alley_Drum
        {  35,  36,  -1,   706965,     7809,   206464 }, // subset0_shelves2_diff
        {  14,  15,  -1,   714774,     1920,   208128 }, // subset0_Stack__Boxes_Diff03
        {  33,  34,  -1,   716694,     1920,   208629 }, // subset0_Back_Alley_Drum
        {  33,  34,  -1,   718614,     1920,   209294 }, // subset0_Back_Alley_Drum
        {  33,  34,  -1,   720534,     1920,   209959 }, // subset0_Back_Alley_Drum
        {  35,  36,  -1,   722454,     7809,   210624 }, // subset0_shelves2_diff
        {  35,  36,  -1,   730263,     7809,   212294 }, // subset0_shelves2_diff
        {  35,  36,  -1,   738072,     7809,   213953 }, // subset0_shelves2_diff
        {  32,  32,  -1,   745881,      783,   215619 }, // subset0_gameCrates_02_Diff
        {  72,  73,  -1,   746664,    14451,   215808 }, // subset0_GolfBag:Golfclub
        {  18,  19,  -1,   761115,     3096,   219093 }, // subset0_Back_Alley_box
    };
}
