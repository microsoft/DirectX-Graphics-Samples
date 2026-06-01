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

#include "stdafx.h"

#include "SampleApp.h"

SampleApp::SampleApp(UINT width, UINT height, std::wstring name)
    : DXSample(width, height, name), m_engine(width, height, name)
{
}

void SampleApp::OnInit()
{
    m_engine.SetUseWarpDevice(m_useWarpDevice);
    m_engine.OnInit();
}

void SampleApp::OnUpdate()
{
    m_engine.OnUpdate();
}

void SampleApp::OnRender()
{
    m_engine.OnRender();
}

void SampleApp::OnDestroy()
{
    m_engine.OnDestroy();
}

void SampleApp::OnKeyDown(UINT8 key)
{
    m_engine.OnKeyDown(key);
}

void SampleApp::OnKeyUp(UINT8 key)
{
    m_engine.OnKeyUp(key);
}

void SampleApp::OnMouseDown(UINT8 button, int x, int y)
{
    m_engine.OnMouseDown(button, x, y);
}

void SampleApp::OnMouseUp(UINT8 button, int x, int y)
{
    m_engine.OnMouseUp(button, x, y);
}

void SampleApp::OnMouseMove(int x, int y)
{
    m_engine.OnMouseMove(x, y);
}

void SampleApp::OnWindowSizeChanged(UINT width, UINT height)
{
    m_engine.OnWindowSizeChanged(width, height);
}

void SampleApp::OnIdle()
{
    m_engine.OnIdle();
}
