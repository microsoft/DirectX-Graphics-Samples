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
#include "stdafx.h"

using namespace DirectX;

class Camera
{
public:
    Camera();
    ~Camera();

    void Get3DViewProjMatrices(XMFLOAT4X4 *view, XMFLOAT4X4 *proj, float fovInDegrees, float screenWidth, float screenHeight);
    void Reset();
    void Set(XMVECTOR eye, XMVECTOR at, XMVECTOR up);
    static Camera *get();
    void RotateYaw(float deg);
    void RotatePitch(float deg);
    void GetOrthoProjMatrices(XMFLOAT4X4 *view, XMFLOAT4X4 *proj, float width, float height);
    XMVECTOR mEye; // Where the camera is in world space. Z increases into of the screen when using LH coord system (which we are and DX uses)
    XMVECTOR mAt; // What the camera is looking at (world origin)
    XMVECTOR mUp; // Which way is up
private:
    static Camera* mCamera;
};
