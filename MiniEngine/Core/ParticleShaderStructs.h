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
// Author(s):  Julia Careaga
//             James Stanard
//

#pragma once

using namespace Math;
//Emission Properties and other particle structs

__declspec(align(16)) struct EmissionProperties
{
    XMFLOAT3 LastEmitPosW;
    float EmitSpeed;
    XMFLOAT3 EmitPosW;
    float FloorHeight;
    XMFLOAT3 EmitDirW;
    float Restitution;
    XMFLOAT3 EmitRightW;
    float EmitterVelocitySensitivity;
    XMFLOAT3 EmitUpW;
    UINT MaxParticles;
    XMFLOAT3 Gravity;
    UINT TextureID;
    XMFLOAT3 EmissiveColor;
    float pad1;	
    XMUINT4 RandIndex[64];
};

EmissionProperties* CreateEmissionProperties();

struct ParticleSpawnData
{
    float AgeRate;
    float RotationSpeed;
    float StartSize;
    float EndSize;
    XMFLOAT3 Velocity; float Mass;
    XMFLOAT3 SpreadOffset; float Random;
    Color StartColor;
    Color EndColor;
};


struct ParticleMotion
{
    XMFLOAT3 Position;
    float Mass;	
    XMFLOAT3 Velocity;
    float Age;
    float Rotation;
    UINT ResetDataIndex;
};

struct ParticleVertex
{
    XMFLOAT3 Position;
    XMFLOAT4 Color;
    float Size;
    UINT TextureID;
};

struct ParticleScreenData
{
    float Corner[2];
    float RcpSize[2];
    float Color[4];
    float Depth;
    float TextureIndex;
    float TextureLevel;
    uint32_t Bounds;
};



