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
// Author:  Julia Careaga

#pragma once
#include "pch.h"
#include "ParticleShaderStructs.h"

EmissionProperties* CreateEmissionProperties()
{
    EmissionProperties* emitProps = new EmissionProperties;	
    ZeroMemory(emitProps, sizeof(*emitProps));
    emitProps->EmitPosW = emitProps->LastEmitPosW = XMFLOAT3(0.0,0.0,0.0);
    emitProps->EmitDirW = XMFLOAT3(0.0,0.0,1.0); 
    emitProps->EmitRightW = XMFLOAT3(1.0,0.0,0.0); 
    emitProps->EmitUpW = XMFLOAT3(0.0,1.0,0.0);
    emitProps->Restitution = 0.6;
    emitProps->FloorHeight = -0.7;
    emitProps->EmitSpeed = 1.0;
    emitProps->Gravity = XMFLOAT3(0, -5, 0);
    emitProps->MaxParticles = 500;
    return emitProps;
};
