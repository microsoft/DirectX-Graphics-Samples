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

#include <algorithm>
#include <cmath>
#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <sys/stat.h>
#include "SampleApp.h"
#include "Scene\SceneFactory.h"
#include "imgui.h"
#include "ImGuiWidgets.h"

void RunStagedAllocatorTests(ID3D12Device* device);

namespace
{

const char* EnvironmentSourceLabel(Engine::EnvironmentSource source)
{
    switch (source)
    {
        case Engine::EnvironmentSource::AssetHdr:
            return "Asset HDR";
        case Engine::EnvironmentSource::ProceduralStudio:
            return "Procedural Studio";
        case Engine::EnvironmentSource::ProceduralSun:
            return "Procedural Sun";
        case Engine::EnvironmentSource::ProceduralColorPanels:
            return "Procedural Color Panels";
        case Engine::EnvironmentSource::ProceduralHorizon:
            return "Procedural Horizon";
        default:
            return "Unknown";
    }
}

void ApplyEnvironmentPreset(Engine::ProceduralEnvironmentSettings& settings, Engine::EnvironmentSource source)
{
    settings = {};
    settings.source = source;

    switch (source)
    {
        case Engine::EnvironmentSource::ProceduralStudio:
            settings.skyColor = {0.50f, 0.52f, 0.54f};
            settings.groundColor = {0.16f, 0.16f, 0.15f};
            settings.lightColor = {1.0f, 0.98f, 0.92f};
            settings.lightDirection = {0.25f, 0.85f, 0.35f};
            settings.backgroundIntensity = 0.35f;
            settings.lightIntensity = 4.0f;
            settings.lightSize = 0.22f;
            settings.fillIntensity = 0.08f;
            break;
        case Engine::EnvironmentSource::ProceduralSun:
            settings.skyColor = {0.25f, 0.43f, 0.75f};
            settings.groundColor = {0.09f, 0.075f, 0.055f};
            settings.lightColor = {1.0f, 0.82f, 0.52f};
            settings.lightDirection = {0.22f, 0.72f, 0.66f};
            settings.backgroundIntensity = 0.20f;
            settings.lightIntensity = 32.0f;
            settings.lightSize = 0.035f;
            settings.fillIntensity = 0.03f;
            break;
        case Engine::EnvironmentSource::ProceduralColorPanels:
            settings.skyColor = {0.02f, 0.02f, 0.025f};
            settings.groundColor = {0.015f, 0.015f, 0.015f};
            settings.lightColor = {1.0f, 1.0f, 1.0f};
            settings.lightDirection = {0.35f, 0.75f, 0.25f};
            settings.backgroundIntensity = 0.05f;
            settings.lightIntensity = 0.0f;
            settings.lightSize = 0.12f;
            settings.fillIntensity = 0.02f;
            settings.colorPanelIntensity = 3.5f;
            break;
        case Engine::EnvironmentSource::ProceduralHorizon:
            settings.skyColor = {0.34f, 0.50f, 0.86f};
            settings.groundColor = {0.18f, 0.15f, 0.10f};
            settings.lightColor = {1.0f, 0.86f, 0.62f};
            settings.lightDirection = {0.1f, 0.08f, 0.99f};
            settings.backgroundIntensity = 0.45f;
            settings.lightIntensity = 5.0f;
            settings.lightSize = 0.12f;
            settings.fillIntensity = 0.03f;
            settings.horizonSharpness = 0.035f;
            break;
        case Engine::EnvironmentSource::AssetHdr:
        default:
            break;
    }
}

XMFLOAT3 ProjectToArcball(int x, int y, UINT width, UINT height)
{
    const float minDimension = static_cast<float>((std::max)(1u, (std::min)(width, height)));
    const float sx = (2.0f * static_cast<float>(x) - static_cast<float>(width)) / minDimension;
    const float sy = (static_cast<float>(height) - 2.0f * static_cast<float>(y)) / minDimension;
    const float lengthSquared = sx * sx + sy * sy;

    XMVECTOR projected = {};
    if (lengthSquared <= 1.0f)
    {
        projected = XMVectorSet(sx, sy, std::sqrt(1.0f - lengthSquared), 0.0f);
    }
    else
    {
        projected = XMVector3Normalize(XMVectorSet(sx, sy, 0.0f, 0.0f));
    }

    XMFLOAT3 result = {};
    XMStoreFloat3(&result, projected);
    return result;
}

XMFLOAT4 ArcballDeltaQuaternion(const XMFLOAT3& from, const XMFLOAT3& to)
{
    const XMVECTOR fromVec = XMVector3Normalize(XMLoadFloat3(&from));
    const XMVECTOR toVec = XMVector3Normalize(XMLoadFloat3(&to));
    const float dot = XMVectorGetX(XMVector3Dot(fromVec, toVec));

    if (dot > 0.9999f)
    {
        return {0.0f, 0.0f, 0.0f, 1.0f};
    }

    XMVECTOR axis = XMVector3Cross(fromVec, toVec);
    if (dot < -0.9999f)
    {
        axis = XMVector3Cross(fromVec, XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));
        if (XMVectorGetX(XMVector3LengthSq(axis)) < 0.0001f)
        {
            axis = XMVector3Cross(fromVec, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
        }
    }

    const XMVECTOR quaternion = XMQuaternionNormalize(XMVectorSetW(axis, 1.0f + dot));
    XMFLOAT4 result = {};
    XMStoreFloat4(&result, quaternion);
    return result;
}

} // namespace

SampleApp::SampleApp(UINT width, UINT height, std::wstring name)
    : DXSample(width, height, name), m_prevTime(std::chrono::steady_clock::now()), m_engine(m_graphicsDevice)
{
}

void SampleApp::OnInit()
{
    CreateSampleScenes();

    GraphicsDeviceDesc deviceDesc = {};
    deviceDesc.hwnd = Win32Application::GetHwnd();
    deviceDesc.swapChainWidth = GetWidth();
    deviceDesc.swapChainHeight = GetHeight();
    deviceDesc.bufferCount = HelloTextureEngine::kSwapChainBufferCount;
    deviceDesc.swapChainFormat = HelloTextureEngine::kSwapChainFormat;
    deviceDesc.useWarpDevice = m_useWarpDevice;
    m_graphicsDevice.Initialize(deviceDesc);

    // Open debug log file and query ID3D12InfoQueue for D3D12 message capture.
    if (!m_logFilePath.empty())
    {
        int fd;
        errno_t err = _wsopen_s(&fd, m_logFilePath.c_str(), _O_WRONLY | _O_CREAT | _O_TRUNC | _O_TEXT,
                                _SH_DENYNO, _S_IREAD | _S_IWRITE);
        if (err == 0)
        {
            m_logFile = _fdopen(fd, "wt");
        }
        if (m_logFile)
        {
            m_graphicsDevice.Device()->QueryInterface(IID_PPV_ARGS(&m_d3d12InfoQueue));
            if (m_d3d12InfoQueue)
            {
                m_d3d12InfoQueue->SetMessageCountLimit(100000);
                FlushD3D12DebugMessages();
            }
        }
    }

    InitializeImGui();
    m_engine.SetUpdateHandler([this]() { UpdateSampleState(); });
    m_engine.SetLightingParams(m_lightingParams);
    m_engine.SetRenderingPath(m_renderingPath);
    m_engine.SetLightingPassDebugGradient(m_lightingPassDebugGradient);
    m_engine.SetBackBufferClearColor(m_backBufferClearColor);
    m_engine.SetDisplayInstanceCount(0);
    m_engine.SetToneMapParams(m_toneMapParams);
    m_engine.SetRenderViewMode(m_renderViewMode);

    m_engine.Initialize(GetWidth(), GetHeight());

    if (m_autoSelectGltfDamagedHelmet)
    {
        m_selectedSceneIndex = kDefaultSceneIndex;
        OpenSelectedScene();
    }
}

void SampleApp::UpdateSampleState()
{
    auto now = std::chrono::steady_clock::now();
    const float deltaTime = std::chrono::duration<float>(now - m_prevTime).count();
    m_prevTime = now;

    if (m_appMode == AppMode::SceneSelect)
    {
        m_engine.SetDisplayInstanceCount(0);
        return;
    }

    static constexpr float kCameraMoveSpeed = 0.01f;
    if (GetForegroundWindow() == Win32Application::GetHwnd())
    {
        auto& camera = LoadedScene().GetScene().camera;

        if (m_isRightDragging)
        {
            XMVECTOR localMove = XMVectorZero();

            if (GetAsyncKeyState('A') & 0x8000)
                localMove = XMVectorAdd(localMove, XMVectorSet(-kCameraMoveSpeed, 0.0f, 0.0f, 0.0f));
            if (GetAsyncKeyState('D') & 0x8000)
                localMove = XMVectorAdd(localMove, XMVectorSet(kCameraMoveSpeed, 0.0f, 0.0f, 0.0f));
            if (GetAsyncKeyState('W') & 0x8000)
                localMove = XMVectorAdd(localMove, XMVectorSet(0.0f, 0.0f, kCameraMoveSpeed, 0.0f));
            if (GetAsyncKeyState('S') & 0x8000)
                localMove = XMVectorAdd(localMove, XMVectorSet(0.0f, 0.0f, -kCameraMoveSpeed, 0.0f));
            if (GetAsyncKeyState('E') & 0x8000)
                localMove = XMVectorAdd(localMove, XMVectorSet(0.0f, kCameraVerticalSpeed, 0.0f, 0.0f));
            if (GetAsyncKeyState('Q') & 0x8000)
                localMove = XMVectorAdd(localMove, XMVectorSet(0.0f, -kCameraVerticalSpeed, 0.0f, 0.0f));
            if (GetAsyncKeyState('Z') & 0x8000)
                camera.fov = std::clamp(camera.fov - kCameraFovZoomSpeed, 20.0f, 150.0f);
            if (GetAsyncKeyState('C') & 0x8000)
                camera.fov = std::clamp(camera.fov + kCameraFovZoomSpeed, 20.0f, 150.0f);

            const float sy = std::sin(camera.rot.y);
            const float cy = std::cos(camera.rot.y);
            const XMVECTOR forward = XMVectorSet(sy, 0.0f, cy, 0.0f);
            const XMVECTOR right = XMVectorSet(cy, 0.0f, -sy, 0.0f);
            const XMVECTOR worldMove = XMVectorAdd(
                XMVectorAdd(
                    XMVectorScale(forward, XMVectorGetZ(localMove)),
                    XMVectorScale(right, XMVectorGetX(localMove))),
                XMVectorSet(0.0f, XMVectorGetY(localMove), 0.0f, 0.0f));
            XMFLOAT3 move = {};
            XMStoreFloat3(&move, worldMove);
            camera.pos.x += move.x;
            camera.pos.y += move.y;
            camera.pos.z += move.z;

            const XMMATRIX camRot = XMMatrixRotationRollPitchYaw(camera.rot.x, camera.rot.y, camera.rot.z);
            const XMVECTOR fwd = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), camRot);
            XMStoreFloat3(&camera.gazePoint, XMLoadFloat3(&camera.pos) + fwd);
        }
        else if (m_cameraMode == CameraMode::FreeLook)
        {
            XMVECTOR localMove = XMVectorZero();
            if (GetAsyncKeyState('A') & 0x8000)
                localMove = XMVectorAdd(localMove, XMVectorSet(-kCameraMoveSpeed, 0.0f, 0.0f, 0.0f));
            if (GetAsyncKeyState('D') & 0x8000)
                localMove = XMVectorAdd(localMove, XMVectorSet(kCameraMoveSpeed, 0.0f, 0.0f, 0.0f));
            if ((GetAsyncKeyState('W') & 0x8000) && (GetAsyncKeyState(VK_SHIFT) & 0x8000))
                localMove = XMVectorAdd(localMove, XMVectorSet(0.0f, kCameraVerticalSpeed, 0.0f, 0.0f));
            if ((GetAsyncKeyState('S') & 0x8000) && (GetAsyncKeyState(VK_SHIFT) & 0x8000))
                localMove = XMVectorAdd(localMove, XMVectorSet(0.0f, -kCameraVerticalSpeed, 0.0f, 0.0f));
            if ((GetAsyncKeyState('W') & 0x8000) && !(GetAsyncKeyState(VK_SHIFT) & 0x8000))
                localMove = XMVectorAdd(localMove, XMVectorSet(0.0f, 0.0f, kCameraMoveSpeed, 0.0f));
            if ((GetAsyncKeyState('S') & 0x8000) && !(GetAsyncKeyState(VK_SHIFT) & 0x8000))
                localMove = XMVectorAdd(localMove, XMVectorSet(0.0f, 0.0f, -kCameraMoveSpeed, 0.0f));

            const XMMATRIX cameraRotation = XMMatrixRotationRollPitchYaw(camera.rot.x, camera.rot.y, camera.rot.z);
            const XMVECTOR worldMove = XMVector3TransformNormal(localMove, cameraRotation);
            XMFLOAT3 move = {};
            XMStoreFloat3(&move, worldMove);
            camera.pos.x += move.x;
            camera.pos.y += move.y;
            camera.pos.z += move.z;

            const XMMATRIX camRot = XMMatrixRotationRollPitchYaw(camera.rot.x, camera.rot.y, camera.rot.z);
            const XMVECTOR fwd = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), camRot);
            XMStoreFloat3(&camera.gazePoint, XMLoadFloat3(&camera.pos) + fwd);
        }
        else
        {
            UpdateObjectViewerCamera();
        }
    }

    Engine::SampleSceneUpdateContext sceneUpdate = {};
    sceneUpdate.isPlaying = m_isPlaying;
    sceneUpdate.meshScale = m_meshScale;
    sceneUpdate.dragRotation = m_dragRotation;
    LoadedScene().Update(deltaTime, sceneUpdate);

    m_engine.SetScene(LoadedScene().GetScene());
    m_engine.SetDisplayInstanceCount(LoadedScene().DisplayInstanceCount());
}

void SampleApp::OnKeyDown(UINT8 key)
{
    if (m_appMode == AppMode::SceneSelect && key == VK_ESCAPE)
    {
        DestroyWindow(Win32Application::GetHwnd());
        return;
    }

    if (m_appMode == AppMode::Running && key == VK_ESCAPE)
    {
        CloseRunningScene();
        return;
    }

    if (m_appMode == AppMode::Running && key == VK_SPACE)
    {
        m_isPlaying = !m_isPlaying;
    }

    if (m_appMode == AppMode::Running && key == VK_TAB)
    {
        m_cameraMode = (m_cameraMode == CameraMode::Arcball) ? CameraMode::FreeLook : CameraMode::Arcball;
        if (m_cameraMode == CameraMode::Arcball)
        {
            InitObjectViewerFromCamera();
        }
    }
}

void SampleApp::OnKeyUp(UINT8 key) {}

void SampleApp::OnMouseDown(UINT8 button, int x, int y)
{
    if (m_appMode == AppMode::SceneSelect)
    {
        return;
    }

    if (button == VK_LBUTTON)
    {
        if (m_renderingPath == HelloTextureEngine::RenderingPath::Deferred && (GetAsyncKeyState(VK_CONTROL) & 0x8000))
        {
            m_engine.RequestPixelPick(x, y);
            return;
        }
        m_isDragging = true;
        m_lastMouseX = x;
        m_lastMouseY = y;
        m_lastArcballVector = ProjectToArcball(x, y, GetWidth(), GetHeight());
    }
    else if (button == VK_MBUTTON)
    {
        m_isMiddleDragging = true;
        m_lastMouseX = x;
        m_lastMouseY = y;
    }
    else if (button == VK_RBUTTON)
    {
        m_isRightDragging = true;
        m_lastMouseX = x;
        m_lastMouseY = y;
    }
}

void SampleApp::OnMouseUp(UINT8 button, int x, int y)
{
    if (button == VK_LBUTTON)
    {
        m_isDragging = false;
    }
    else if (button == VK_MBUTTON)
    {
        m_isMiddleDragging = false;
    }
    else if (button == VK_RBUTTON)
    {
        m_isRightDragging = false;
        if (m_cameraMode == CameraMode::Arcball)
        {
            InitObjectViewerFromCamera();
        }
    }
}

void SampleApp::OnMouseMove(int x, int y)
{
    if (m_appMode == AppMode::SceneSelect)
    {
        return;
    }

    auto& camera = LoadedScene().GetScene().camera;

    if (m_isRightDragging)
    {
        const int dx = x - m_lastMouseX;
        const int dy = y - m_lastMouseY;
        m_lastMouseX = x;
        m_lastMouseY = y;

        camera.rot.x = std::clamp(camera.rot.x + static_cast<float>(dy) * kMouseCameraRotationSpeed,
                                  -kCameraPitchLimit, kCameraPitchLimit);
        camera.rot.y += static_cast<float>(dx) * kMouseCameraRotationSpeed;

        const XMMATRIX camRot = XMMatrixRotationRollPitchYaw(camera.rot.x, camera.rot.y, camera.rot.z);
        const XMVECTOR fwd = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), camRot);
        XMStoreFloat3(&camera.gazePoint, XMLoadFloat3(&camera.pos) + fwd);
        return;
    }

    if (m_cameraMode == CameraMode::FreeLook)
    {
        if (m_isDragging)
        {
            const int dx = x - m_lastMouseX;
            const int dy = y - m_lastMouseY;
            m_lastMouseX = x;
            m_lastMouseY = y;

            camera.rot.x = std::clamp(camera.rot.x + static_cast<float>(dy) * kMouseCameraRotationSpeed,
                                      -kCameraPitchLimit, kCameraPitchLimit);
            camera.rot.y += static_cast<float>(dx) * kMouseCameraRotationSpeed;

            const XMMATRIX camRot = XMMatrixRotationRollPitchYaw(camera.rot.x, camera.rot.y, camera.rot.z);
            const XMVECTOR fwd = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), camRot);
            XMStoreFloat3(&camera.gazePoint, XMLoadFloat3(&camera.pos) + fwd);
        }
        else if (m_isMiddleDragging)
        {
            const int dx = x - m_lastMouseX;
            const int dy = y - m_lastMouseY;
            m_lastMouseX = x;
            m_lastMouseY = y;

            const XMVECTOR localPan = XMVectorSet(static_cast<float>(dx) * kMousePanSpeed,
                                                  -static_cast<float>(dy) * kMousePanSpeed, 0.0f, 0.0f);
            const XMMATRIX cameraRotation = XMMatrixRotationRollPitchYaw(camera.rot.x, camera.rot.y, camera.rot.z);
            const XMVECTOR worldPan = XMVector3TransformNormal(localPan, cameraRotation);
            XMFLOAT3 pan = {};
            XMStoreFloat3(&pan, worldPan);
            camera.pos.x += pan.x;
            camera.pos.y += pan.y;
            camera.pos.z += pan.z;
        }
    }
    else
    {
        if (m_isDragging)
        {
            const int dx = x - m_lastMouseX;
            int dy = y - m_lastMouseY;
            m_lastMouseX = x;
            m_lastMouseY = y;
            if (std::abs(dy) <= kObjectViewerOrbitPitchDeadZonePixels)
            {
                dy = 0;
            }

            const XMVECTOR pivot = XMLoadFloat3(&m_objectViewerPivot);
            XMVECTOR offset = XMLoadFloat3(&camera.pos) - pivot;
            if (dx != 0)
            {
                const XMMATRIX yawRotation =
                    XMMatrixRotationY(static_cast<float>(dx) * kMouseCameraRotationSpeed);
                offset = XMVector3TransformNormal(offset, yawRotation);
            }
            if (dy != 0)
            {
                const XMVECTOR lookDir = XMVector3Normalize(-offset);
                const XMVECTOR right = XMVector3Normalize(
                    XMVector3Cross(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), lookDir));
                const XMMATRIX pitchRotation =
                    XMMatrixRotationAxis(right, static_cast<float>(dy) * kMouseCameraRotationSpeed);
                offset = XMVector3TransformNormal(offset, pitchRotation);
            }
            SetObjectViewerOrbitFromOffset(offset);
            UpdateObjectViewerCamera();
        }
        else if (m_isMiddleDragging)
        {
            const int dx = x - m_lastMouseX;
            const int dy = y - m_lastMouseY;
            m_lastMouseX = x;
            m_lastMouseY = y;

            const XMVECTOR localPan = XMVectorSet(static_cast<float>(dx) * kObjectViewerPanSpeed,
                                                  -static_cast<float>(dy) * kObjectViewerPanSpeed, 0.0f, 0.0f);
            const XMMATRIX cameraRotation = XMMatrixRotationRollPitchYaw(camera.rot.x, camera.rot.y, camera.rot.z);
            const XMVECTOR worldPan = XMVector3TransformNormal(localPan, cameraRotation);
            XMFLOAT3 pan = {};
            XMStoreFloat3(&pan, worldPan);
            m_objectViewerPivot.x += pan.x;
            m_objectViewerPivot.y += pan.y;
            m_objectViewerPivot.z += pan.z;
        }
    }
}

void SampleApp::OnMouseWheel(int wheelDelta)
{
    if (m_appMode == AppMode::SceneSelect)
    {
        return;
    }

    auto& camera = LoadedScene().GetScene().camera;
    const float wheelSteps = static_cast<float>(wheelDelta) / static_cast<float>(WHEEL_DELTA);

    if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
    {
        camera.fov = std::clamp(camera.fov - wheelSteps * kMouseWheelFovSpeed, 20.0f, 150.0f);
        return;
    }

    if (m_cameraMode == CameraMode::FreeLook)
    {
        const XMVECTOR localMove = XMVectorSet(0.0f, 0.0f, wheelSteps * kMouseWheelCameraSpeed, 0.0f);
        const XMMATRIX cameraRotation = XMMatrixRotationRollPitchYaw(camera.rot.x, camera.rot.y, camera.rot.z);
        const XMVECTOR worldMove = XMVector3TransformNormal(localMove, cameraRotation);
        XMFLOAT3 move = {};
        XMStoreFloat3(&move, worldMove);
        camera.pos.x += move.x;
        camera.pos.y += move.y;
        camera.pos.z += move.z;
    }
    else
    {
        m_objectViewerDistance -= wheelSteps * kObjectViewerDollySpeed;
        m_objectViewerDistance = (std::max)(0.1f, m_objectViewerDistance);
    }
}

void SampleApp::OnWindowSizeChanged(UINT width, UINT height)
{
    m_engine.RequestResize(width, height);
    m_imguiSystem.SetDisplaySize(width, height);
}

void SampleApp::OnIdle()
{
    UpdateUiFrame();
    m_engine.RunFrame([this](ID3D12GraphicsCommandList* commandList) { m_imguiSystem.Render(commandList); });

    // Poll D3D12 debug messages and FPS logging.
    if (m_logFile)
    {
        if (m_d3d12InfoQueue)
        {
            FlushD3D12DebugMessages();
        }
        if (m_logFpsInterval > 0)
        {
            ++m_fpsLogFrameCounter;
            if (m_fpsLogFrameCounter % m_logFpsInterval == 0)
            {
                LogFpsToFile(m_engine.CpuFrameTimeMs());
            }
        }
    }
}

void SampleApp::OnUpdate()
{
    // Win32Application does not call OnUpdate() in the current DXSample loop.
    // It calls OnIdle(), which is delegated to HelloTextureEngine::RunFrame().
    // The engine invokes UpdateHandler at the appropriate point in its frame.
    //
    // Keep this override for the DXSample interface. If Win32Application starts
    // calling OnUpdate() in the future, SampleApp frame state should still update
    // through the same path used by the engine callback.
    UpdateSampleState(); // currently this is not called.
}

void SampleApp::OnRender()
{
    // Win32Application does not call OnRender() in the current DXSample loop.
    // Rendering is driven by HelloTextureEngine::RunFrame(), which calls the engine's
    // render path after its frame update.
    // Keep this override for the DXSample interface. If Win32Application starts
    // calling OnRender() in the future, delegate to the engine render path.
    UpdateUiFrame();
    m_engine.RenderFrame([this](ID3D12GraphicsCommandList* commandList) { m_imguiSystem.Render(commandList); });
}

void SampleApp::OnDestroy()
{
    if (m_logFile)
    {
        FlushD3D12DebugMessages();
    }
    m_engine.Shutdown();
    m_imguiSystem.Shutdown();
    m_imguiHeap.Reset();
    if (m_logFile)
    {
        FlushD3D12DebugMessages();
        fclose(m_logFile);
        m_logFile = nullptr;
    }
    m_d3d12InfoQueue.Reset();
}

void SampleApp::FlushD3D12DebugMessages()
{
    if (!m_d3d12InfoQueue || !m_logFile)
    {
        return;
    }

    const UINT64 count = m_d3d12InfoQueue->GetNumStoredMessages();
    if (count == 0)
    {
        return;
    }

    for (UINT64 i = 0; i < count; ++i)
    {
        SIZE_T len = 0;
        m_d3d12InfoQueue->GetMessage(static_cast<UINT>(i), nullptr, &len);
        std::vector<BYTE> buf(len);
        D3D12_MESSAGE* msg = reinterpret_cast<D3D12_MESSAGE*>(buf.data());
        if (SUCCEEDED(m_d3d12InfoQueue->GetMessage(static_cast<UINT>(i), msg, &len)))
        {
            const char* severity = "INFO";
            switch (msg->Severity)
            {
                case D3D12_MESSAGE_SEVERITY_CORRUPTION: severity = "CORRUPTION"; break;
                case D3D12_MESSAGE_SEVERITY_ERROR:      severity = "ERROR";      break;
                case D3D12_MESSAGE_SEVERITY_WARNING:    severity = "WARNING";    break;
                case D3D12_MESSAGE_SEVERITY_INFO:       severity = "INFO";       break;
                case D3D12_MESSAGE_SEVERITY_MESSAGE:    severity = "MESSAGE";    break;
            }
            fprintf(m_logFile, "[%s] %s\n", severity, msg->pDescription);
        }
    }
    m_d3d12InfoQueue->ClearStoredMessages();
    fflush(m_logFile);
}

void SampleApp::LogFpsToFile(float cpuFrameTimeMs)
{
    if (!m_logFile || cpuFrameTimeMs <= 0.0f)
    {
        return;
    }
    const float fps = 1000.0f / cpuFrameTimeMs;
    fprintf(m_logFile, "[FPS] Frame %llu: %.1f FPS (%.2f ms)\n",
            static_cast<unsigned long long>(m_fpsLogFrameCounter), fps, cpuFrameTimeMs);
    fflush(m_logFile);
}

void SampleApp::CreateSampleScenes()
{
    m_sampleScenes.clear();

    static const Engine::GltfAssetDesc gltfAssets[] = {
        {"DamagedHelmet", "Assets\\Models\\DamagedHelmet\\glTF\\DamagedHelmet.gltf", -10.0f, 0.5f},
        {"Avocado", "Assets\\Models\\Avocado\\glTF\\Avocado.gltf", -10.0f, 0.35f},
        {"BoomBox", "Assets\\Models\\BoomBox\\glTF\\BoomBox.gltf", -6.0f, 1.0f},
        {"Lantern", "Assets\\Models\\Lantern\\glTF\\Lantern.gltf", -10.0f, 0.5f},
        {"Sponza", "Assets\\Models\\Sponza\\glTF\\Sponza.gltf", -10.0f, 0.01f},
        {"FlightHelmet", nullptr, -10.0f, 0.5f},
        {"Suzanne", nullptr, -10.0f, 0.5f},
        {"BoxTextured", nullptr, -10.0f, 0.5f},
        {"CesiumMan", nullptr, -10.0f, 0.5f},
    };
    const int gltfAssetCount = ARRAYSIZE(gltfAssets);

    m_gltfViewerCount = gltfAssetCount;
    for (int i = 0; i < m_gltfViewerCount; i++)
    {
        m_sampleScenes.push_back(std::make_unique<Engine::GltfObjectViewerScene>(gltfAssets[i]));
    }

    m_gltfSceneCount = gltfAssetCount;
    for (int i = 0; i < m_gltfSceneCount; i++)
    {
        m_sampleScenes.push_back(
            std::make_unique<Engine::GltfGridBenchmarkScene>(gltfAssets[i], Engine::GltfGridBenchmarkScene::kMaxInstanceCount));
    }

    m_sampleScenes.push_back(std::make_unique<Engine::MetallicRoughnessSphereScene>(
        Engine::MetallicRoughnessSphereScene::kMaxInstanceCount));
    m_sampleScenes.push_back(std::make_unique<Engine::ShadowTestGroundCubesScene>(
        Engine::ShadowTestGroundCubesScene::kMaxInstanceCount));
    m_sampleScenes.push_back(
        std::make_unique<Engine::AnimatedShadowGridScene>(Engine::AnimatedShadowGridScene::kMaxInstanceCount));
    m_sampleScenes.push_back(
        std::make_unique<Engine::ContactShadowTestScene>(Engine::ContactShadowTestScene::kMaxInstanceCount));
    m_sampleScenes.push_back(
        std::make_unique<Engine::OccluderWallTestScene>(Engine::OccluderWallTestScene::kMaxInstanceCount));

    m_sampleScenes.push_back(Engine::SceneFactory::CreateCornellBox());
}

void SampleApp::LoadSceneCpuData(int sceneIndex)
{
    assert(sceneIndex >= 0 && sceneIndex < static_cast<int>(m_sampleScenes.size()));

    m_loadedSceneIndex = sceneIndex;
    m_loadedScene = m_sampleScenes[static_cast<size_t>(m_loadedSceneIndex)].get();
    m_selectedSceneIndex = m_loadedSceneIndex;
    m_loadedScene->Load();
    m_meshScale = m_loadedScene->DefaultMeshScale();
    m_displayInstanceCount = m_loadedScene->DisplayInstanceCount();
    m_selectedMaterialIndex = 0;
    m_lastArcballVector = {0.0f, 0.0f, 1.0f};
    m_dragRotation = {0.0f, 0.0f, 0.0f, 1.0f};
    m_objectViewerPivot = {0.0f, 0.0f, 0.0f};
    m_sceneResourcesLoaded = false;

    m_cameraMode = IsGltfViewerSceneIndex(m_loadedSceneIndex) ? CameraMode::Arcball : CameraMode::FreeLook;
    if (strstr(m_loadedScene->Name(), "Sponza") != nullptr)
    {
        m_cameraMode = CameraMode::FreeLook;
    }
    if (m_cameraMode == CameraMode::Arcball)
    {
        InitObjectViewerFromCamera();
    }
    else
    {
        auto& camera = LoadedScene().GetScene().camera;
        const XMVECTOR camPos = XMLoadFloat3(&camera.pos);
        const XMVECTOR dir = XMVector3Normalize(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f) - camPos);
        XMFLOAT3 dirF;
        XMStoreFloat3(&dirF, dir);
        camera.rot.x = std::asin(std::clamp(dirF.y, -1.0f, 1.0f));
        camera.rot.y = std::atan2(dirF.x, dirF.z);
        camera.gazePoint = {0.0f, 0.0f, 0.0f};
    }
}

void SampleApp::OpenSelectedScene()
{
    if (m_selectedSceneIndex != m_loadedSceneIndex)
    {
        LoadSceneCpuData(m_selectedSceneIndex);
    }

    if (!m_sceneResourcesLoaded)
    {
        m_engine.ReloadSceneResources(LoadedScene().GetScene());
        m_sceneResourcesLoaded = true;
    }

    m_displayInstanceCount = LoadedScene().DisplayInstanceCount();
    m_engine.SetDisplayInstanceCount(m_displayInstanceCount);
    m_appMode = AppMode::Running;
}

void SampleApp::CloseRunningScene()
{
    m_appMode = AppMode::SceneSelect;
    m_isPlaying = false;
    m_isDragging = false;
    m_isMiddleDragging = false;
    if (m_loadedSceneIndex >= 0)
    {
        m_selectedSceneIndex = m_loadedSceneIndex;
    }
    m_displayInstanceCount = 0;
    m_sceneResourcesLoaded = false;
    m_engine.SetDisplayInstanceCount(0);
    m_engine.CloseSceneResources();
}

bool SampleApp::IsGltfViewerSceneIndex(int index) const
{
    return index >= 0 && index < m_gltfViewerCount;
}

void SampleApp::InitObjectViewerFromCamera()
{
    auto& camera = LoadedScene().GetScene().camera;
    const XMVECTOR pivot = XMLoadFloat3(&m_objectViewerPivot);
    const XMVECTOR camPos = XMLoadFloat3(&camera.pos);
    SetObjectViewerOrbitFromOffset(camPos - pivot);
}

void SampleApp::SetObjectViewerOrbitFromOffset(DirectX::FXMVECTOR offset)
{
    m_objectViewerDistance = XMVectorGetX(XMVector3Length(offset));
    if (m_objectViewerDistance < 0.001f)
    {
        m_objectViewerDistance = 5.0f;
        m_objectViewerYaw = 0.0f;
        m_objectViewerPitch = 0.0f;
        return;
    }
    const XMVECTOR dir = XMVector3Normalize(offset);
    XMFLOAT3 dirF = {};
    XMStoreFloat3(&dirF, dir);
    m_objectViewerYaw = std::atan2(dirF.x, dirF.z);
    m_objectViewerPitch = std::asin(std::clamp(dirF.y, -1.0f, 1.0f));
}

void SampleApp::UpdateObjectViewerCamera()
{
    auto& camera = LoadedScene().GetScene().camera;
    m_objectViewerPitch = std::clamp(m_objectViewerPitch, -kObjectViewerPitchLimit, kObjectViewerPitchLimit);

    const float cp = std::cos(m_objectViewerPitch);
    const float sp = std::sin(m_objectViewerPitch);
    const float cy = std::cos(m_objectViewerYaw);
    const float sy = std::sin(m_objectViewerYaw);
    camera.pos.x = m_objectViewerPivot.x + m_objectViewerDistance * cp * sy;
    camera.pos.y = m_objectViewerPivot.y + m_objectViewerDistance * sp;
    camera.pos.z = m_objectViewerPivot.z + m_objectViewerDistance * cp * cy;

    const XMVECTOR toPivot = XMLoadFloat3(&m_objectViewerPivot) - XMLoadFloat3(&camera.pos);
    const XMVECTOR dir = XMVector3Normalize(toPivot);
    XMFLOAT3 dirF = {};
    XMStoreFloat3(&dirF, dir);
    camera.rot.x = std::asin(std::clamp(dirF.y, -1.0f, 1.0f));
    camera.rot.y = std::atan2(dirF.x, dirF.z);
    camera.rot.z = 0.0f;

    camera.gazePoint = m_objectViewerPivot;
}

void SampleApp::InitializeImGui()
{
    D3D12_DESCRIPTOR_HEAP_DESC imguiHeapDesc = {};
    imguiHeapDesc.NumDescriptors = kImGuiDescriptorCount;
    imguiHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    imguiHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(m_graphicsDevice.Device()->CreateDescriptorHeap(&imguiHeapDesc, IID_PPV_ARGS(&m_imguiHeap)));

    m_imguiSystem.Initialize(Win32Application::GetHwnd(),
                             m_graphicsDevice,
                             m_imguiHeap.Get(),
                             HelloTextureEngine::kSwapChainBufferCount,
                             HelloTextureEngine::kSwapChainFormat);
}

void SampleApp::UpdateUiFrame()
{
    m_imguiSystem.BeginFrame();
    DrawDebugUi(m_engine.GetUiFrameContext());
    m_imguiSystem.EndFrame();
}

Engine::SampleScene& SampleApp::LoadedScene()
{
    assert(m_loadedScene != nullptr);
    return *m_loadedScene;
}

const Engine::SampleScene& SampleApp::LoadedScene() const
{
    assert(m_loadedScene != nullptr);
    return *m_loadedScene;
}

void SampleApp::DrawSceneSelectUi()
{
    ImGui::SetNextWindowSize(ImVec2(360, 360), ImGuiCond_FirstUseEver);
    ImGui::Begin("Scene Select");

    const int sceneCount = static_cast<int>(m_sampleScenes.size());
    const int benchmarkStart = m_gltfViewerCount;
    const int demoSceneStart = m_gltfViewerCount + m_gltfSceneCount;

    ImGui::Text("glTF Viewer");
    ImGui::Separator();
    ImGui::PushID("gltf-viewer");
    for (int i = 0; i < m_gltfViewerCount; i++)
    {
        const bool available = m_sampleScenes[static_cast<size_t>(i)]->Available();
        ImGui::BeginDisabled(!available);
        ImGui::RadioButton(m_sampleScenes[static_cast<size_t>(i)]->Name(), &m_selectedSceneIndex, i);
        ImGui::EndDisabled();
    }
    ImGui::PopID();

    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::Text("glTF Grid Benchmark");
    ImGui::Separator();
    ImGui::PushID("gltf-grid-benchmark");
    for (int i = benchmarkStart; i < benchmarkStart + m_gltfSceneCount; i++)
    {
        const bool available = m_sampleScenes[static_cast<size_t>(i)]->Available();
        ImGui::BeginDisabled(!available);
        ImGui::RadioButton(m_sampleScenes[static_cast<size_t>(i)]->Name(), &m_selectedSceneIndex, i);
        ImGui::EndDisabled();
    }
    ImGui::PopID();

    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::Text("Demo Scenes");
    ImGui::Separator();
    for (int i = demoSceneStart; i < sceneCount; i++)
    {
        ImGui::RadioButton(m_sampleScenes[static_cast<size_t>(i)]->Name(), &m_selectedSceneIndex, i);
    }

    ImGui::Dummy(ImVec2(0.0f, 8.0f));
    if (ImGui::Button("Load Scene"))
    {
        OpenSelectedScene();
    }

    const bool hasLoadedScene = m_loadedSceneIndex >= 0;
    const bool selectedSceneIsLoaded = hasLoadedScene && m_selectedSceneIndex == m_loadedSceneIndex;
    if (!hasLoadedScene)
    {
        ImGui::Text("Scene CPU data not loaded");
    }
    else if (!selectedSceneIsLoaded)
    {
        ImGui::Text("Scene CPU data will reload");
    }
    else if (!m_sceneResourcesLoaded)
    {
        ImGui::Text("Scene GPU resources unloaded");
    }

    ImGui::End();
}

void SampleApp::DrawDebugUi(const HelloTextureEngine::UiFrameContext& context)
{
    using RenderingPath = HelloTextureEngine::RenderingPath;
    using RenderViewMode = HelloTextureEngine::RenderViewMode;

    if (m_appMode == AppMode::SceneSelect)
    {
        DrawSceneSelectUi();
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(400, 140), ImGuiCond_FirstUseEver);
    ImGui::Begin("Debug");

    Engine::SampleScene& loadedScene = LoadedScene();
    Engine::SceneMesh& sceneMesh = loadedScene.GetMesh();

    ImGui::Text("Hello ImGui");
    ImGui::Text("Scene: %s", loadedScene.Name());
    ImGui::Text("Loaded Scene Index: %d", m_loadedSceneIndex);
    ImGui::Text("FrameIndex: %d", context.frameIndex);
    ImGui::Text("Ray Tracing: %s (Tier %ls, raw=%d)",
                context.rayTracingSupported ? "Supported" : "Not supported",
                context.rayTracingTierName,
                context.rayTracingTierRaw);
    if (ImGui::Button("Close Scene"))
    {
        CloseRunningScene();
        ImGui::End();
        return;
    }
    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGuiWidgets::SliderFloatWithControls("FovH", &loadedScene.GetScene().camera.fov, 20.f, 150.f, 5.f, 60.f);
        int cameraMode = static_cast<int>(m_cameraMode);
        if (ImGui::Combo("Mode", &cameraMode, "FreeLook\0Arcball\0"))
        {
            m_cameraMode = static_cast<CameraMode>(cameraMode);
            if (m_cameraMode == CameraMode::Arcball)
            {
                InitObjectViewerFromCamera();
            }
        }
    }
    if (ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGuiWidgets::SliderIntWithControls("Display Instance Count", &m_displayInstanceCount, 0,
                                             loadedScene.MaxDisplayInstanceCount(), 1, 0);
        loadedScene.SetDisplayInstanceCount(m_displayInstanceCount);
        ImGuiWidgets::SliderFloatWithControls("Mesh Scale", &m_meshScale, 0.1f, 2.0f, 0.05f, 0.5f);
        ImGui::ColorEdit4("Background Color", m_backBufferClearColor.data());
    }
    if (ImGui::CollapsingHeader("PBR Lighting", ImGuiTreeNodeFlags_DefaultOpen))
    {
        {
            static constexpr float defaultDir[] = {0.0f, 1.0f, -1.0f};
            ImGuiWidgets::SliderFloat3WithControls("Light Direction", &m_lightingParams.lightDirection.x, -1.0f, 1.0f,
                                                    0.05f, defaultDir);
        }
        ImGui::SameLine();
        ImGuiWidgets::SliderFloatWithControls("Direct Light Intensity", &m_lightingParams.diffuseIntensity, 0.0f, 4.0f,
                                               0.1f, 1.0f);
        ImGui::ColorEdit3("Light Color", &m_lightingParams.lightColor.x);
        ImGui::Checkbox("IBL Enabled", &m_iblEnabled);
        ImGui::BeginDisabled(!m_iblEnabled);
        ImGuiWidgets::SliderFloatWithControls("IBL Intensity", &m_lightingParams.iblIntensity, 0.0f, 2.0f, 0.05f, 1.0f);
        ImGui::Checkbox("Diffuse IBL", &m_lightingParams.diffuseIblEnabled);
        ImGui::SameLine();
        ImGui::Checkbox("Specular IBL", &m_lightingParams.specularIblEnabled);
        ImGui::EndDisabled();
        ImGui::Checkbox("Direct Light", &m_lightingParams.directLightEnabled);
        ImGui::SameLine();
        ImGui::Checkbox("Emissive", &m_lightingParams.emissiveEnabled);
    }
    if (ImGui::CollapsingHeader("Environment Map", ImGuiTreeNodeFlags_DefaultOpen))
    {
        bool environmentApplyRequested = false;
        int environmentSource = static_cast<int>(m_environmentSettings.source);
        if (ImGui::Combo("Source",
                         &environmentSource,
                         "Asset HDR\0Procedural Studio\0Procedural Sun\0Procedural Color Panels\0Procedural Horizon\0"))
        {
            ApplyEnvironmentPreset(m_environmentSettings, static_cast<Engine::EnvironmentSource>(environmentSource));
            environmentApplyRequested = true;
        }
        ImGui::SameLine();
        ImGui::Text("%s", EnvironmentSourceLabel(m_environmentSettings.source));
        if (m_environmentSettings.source != Engine::EnvironmentSource::AssetHdr)
        {
            ImGui::SameLine();
            ImGui::Checkbox("Auto Update", &m_environmentAutoUpdate);
            environmentApplyRequested |= ImGui::ColorEdit3("Sky Color", &m_environmentSettings.skyColor.x);
            environmentApplyRequested |= ImGui::ColorEdit3("Ground Color", &m_environmentSettings.groundColor.x);
            const bool colorPanels = m_environmentSettings.source == Engine::EnvironmentSource::ProceduralColorPanels;
            if (!colorPanels)
            {
                environmentApplyRequested |= ImGui::ColorEdit3("Env Light Color", &m_environmentSettings.lightColor.x);
                static constexpr float defaultEnvLightDir[] = {0.35f, 0.75f, 0.25f};
                if (ImGuiWidgets::SliderFloat3WithControls("Env Light Direction", &m_environmentSettings.lightDirection.x,
                                                       -1.0f, 1.0f, 0.05f, defaultEnvLightDir))
                {
                    environmentApplyRequested = true;
                }
            }
            if (ImGuiWidgets::SliderFloatWithControls("Env Background",
                                                  &m_environmentSettings.backgroundIntensity,
                                                  0.0f,
                                                  4.0f,
                                                  0.05f,
                                                  0.6f))
            {
                environmentApplyRequested = true;
            }
            if (!colorPanels)
            {
                if (ImGuiWidgets::SliderFloatWithControls("Env Light Intensity",
                                                      &m_environmentSettings.lightIntensity,
                                                      0.0f,
                                                      40.0f,
                                                      0.5f,
                                                      6.0f))
                {
                    environmentApplyRequested = true;
                }
                if (ImGuiWidgets::SliderFloatWithControls("Env Light Size",
                                                      &m_environmentSettings.lightSize,
                                                      0.01f,
                                                      0.8f,
                                                      0.01f,
                                                      0.12f))
                {
                    environmentApplyRequested = true;
                }
            }
            if (ImGuiWidgets::SliderFloatWithControls(
                "Env Fill", &m_environmentSettings.fillIntensity, 0.0f, 2.0f, 0.05f, 0.12f))
            {
                environmentApplyRequested = true;
            }
            if (colorPanels)
            {
                if (ImGuiWidgets::SliderFloatWithControls("Color Panel Intensity",
                                                      &m_environmentSettings.colorPanelIntensity,
                                                      0.0f,
                                                      8.0f,
                                                      0.1f,
                                                      1.5f))
                {
                    environmentApplyRequested = true;
                }
            }
            if (m_environmentSettings.source == Engine::EnvironmentSource::ProceduralHorizon)
            {
                if (ImGuiWidgets::SliderFloatWithControls("Horizon Width",
                                                      &m_environmentSettings.horizonSharpness,
                                                      0.01f,
                                                      0.5f,
                                                      0.01f,
                                                      0.08f))
                {
                    environmentApplyRequested = true;
                }
            }
        }
        ImGui::Checkbox("Show Skybox", &m_lightingParams.skyboxEnabled);
        ImGui::Checkbox("Skybox Preview", &m_lightingParams.skyboxPreview);
        ImGui::BeginDisabled(!m_lightingParams.skyboxPreview);
        ImGuiWidgets::SliderFloatWithControls("Skybox Preview Exposure", &m_lightingParams.skyboxPreviewExposure, 0.0f,
                                              2.0f, 0.05f, 1.0f);
        ImGui::EndDisabled();
        if (environmentApplyRequested)
        {
            m_environmentReloadPending = true;
        }
        if (m_environmentReloadPending && m_environmentAutoUpdate && !ImGui::IsAnyItemActive())
        {
            m_engine.ReloadEnvironmentResources(m_environmentSettings);
            m_environmentReloadPending = false;
        }
    }

    if (!sceneMesh.materials.empty())
    {
        if (ImGui::CollapsingHeader("Material Controls", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const int materialCount = static_cast<int>(sceneMesh.materials.size());
            if (m_selectedMaterialIndex >= materialCount)
            {
                m_selectedMaterialIndex = materialCount - 1;
            }
            ImGuiWidgets::SliderIntWithControls("Material", &m_selectedMaterialIndex, 0, materialCount - 1, 1, 0);

            Engine::SceneMaterial& material = sceneMesh.materials[m_selectedMaterialIndex];
            bool materialChanged = false;
            materialChanged |= ImGuiWidgets::SliderFloatWithControls("Roughness", &material.roughnessFactor, 0.04f, 1.0f,
                                                                      0.02f, 0.5f);
            materialChanged |= ImGuiWidgets::SliderFloatWithControls("Metallic", &material.metallicFactor, 0.0f, 1.0f,
                                                                      0.05f, 0.0f);
            materialChanged |= ImGuiWidgets::SliderFloatWithControls("Indirect Occlusion",
                                                                      &material.ambientOcclusionFactor, 0.0f, 1.0f,
                                                                      0.05f, 1.0f);
            materialChanged |= ImGuiWidgets::SliderFloatWithControls("Emissive Luminance", &material.emissiveScale, 0.0f,
                                                                      4.0f, 0.1f, 1.0f);

            const float f0 = 0.04f * (1.0f - material.metallicFactor) + material.metallicFactor;
            ImGui::Text("Specular F0: %.2f", f0);

            if (materialChanged)
            {
                HelloTextureEngine::MaterialParams params = {};
                params.roughnessFactor = material.roughnessFactor;
                params.metallicFactor = material.metallicFactor;
                params.ambientOcclusionFactor = material.ambientOcclusionFactor;
                params.emissiveScale = material.emissiveScale;
                m_engine.SetMaterialParams(static_cast<UINT>(m_selectedMaterialIndex), params);
            }
        }
    }

    if (ImGui::CollapsingHeader("Tone Mapping", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::RadioButton("None", &m_toneMapParams.operatorIndex, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Reinhard", &m_toneMapParams.operatorIndex, 1);
        ImGui::SameLine();
        ImGui::RadioButton("ACES", &m_toneMapParams.operatorIndex, 2);
        ImGuiWidgets::SliderFloatWithControls("Exposure", &m_toneMapParams.exposure, 0.0f, 4.0f, 0.1f, 1.0f);
        ImGuiWidgets::SliderFloatWithControls("Paper White", &m_toneMapParams.paperWhiteNits, 80.0f, 500.0f, 10.f,
                                              300.0f, "%.0f nits");
        ImGuiWidgets::SliderFloatWithControls("Display Max", &m_toneMapParams.maxDisplayNits, 100.0f, 4000.0f, 50.f,
                                              1000.0f, "%.0f nits");
    }

    if (ImGui::CollapsingHeader("RayQuery Shadow", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto shadowSettings = m_engine.GetShadowSettings();
        bool changed = false;

        changed |= ImGui::Checkbox("Shadow Enable", &shadowSettings.enabled);

        changed |= ImGuiWidgets::SliderFloatWithControls(
            "Normal Bias", &shadowSettings.normalBias, 0.0f, 0.1f, 0.001f, 0.01f);

        changed |= ImGuiWidgets::SliderFloatWithControls(
            "Ray TMin", &shadowSettings.rayTMin, 0.0f, 0.1f, 0.001f, 0.001f);

        changed |= ImGuiWidgets::SliderFloatWithControls(
            "Ray TMax", &shadowSettings.rayTMax, 1.0f, 10000.0f, 100.0f, 10000.0f);

        ImGui::Separator();
        changed |= ImGui::Checkbox("Soft Shadow Enable", &shadowSettings.softShadowEnabled);

        changed |= ImGuiWidgets::SliderIntWithControls(
            "Sample Count", &shadowSettings.sampleCount, 1, 16, 1, 8);

        changed |= ImGuiWidgets::SliderFloatWithControls(
            "Light Angular Radius", &shadowSettings.lightAngularRadius, 0.0f, 0.1f, 0.001f, 0.1f, "%.4f");

        changed |= ImGuiWidgets::SliderFloatWithControls(
            "Jitter Strength", &shadowSettings.jitterStrength, 0.0f, 2.0f, 0.1f, 2.0f);

        if (changed)
        {
            m_engine.SetShadowSettings(shadowSettings);
        }
    }

    if (ImGui::CollapsingHeader("Debug", ImGuiTreeNodeFlags_DefaultOpen))
    {
        int renderingPath = static_cast<int>(m_renderingPath);
        ImGui::RadioButton("Forward", &renderingPath, static_cast<int>(RenderingPath::Forward));
        ImGui::SameLine();
        ImGui::RadioButton("Deferred", &renderingPath, static_cast<int>(RenderingPath::Deferred));
        m_renderingPath = static_cast<RenderingPath>(renderingPath);

        const bool deferredRendering = m_renderingPath == RenderingPath::Deferred;
        int renderViewMode = static_cast<int>(m_renderViewMode);
        ImGui::BeginDisabled(!deferredRendering);
        ImGui::RadioButton("Lit", &renderViewMode, static_cast<int>(RenderViewMode::LightPass));
        ImGui::RadioButton("Albedo", &renderViewMode, static_cast<int>(RenderViewMode::GBufferAlbedo));
        ImGui::SameLine();
        ImGui::RadioButton("Normal", &renderViewMode, static_cast<int>(RenderViewMode::GBufferNormal));
        ImGui::SameLine();
        ImGui::RadioButton("Material", &renderViewMode, static_cast<int>(RenderViewMode::GBufferMaterial));
        ImGui::RadioButton("MotionVector", &renderViewMode, static_cast<int>(RenderViewMode::GBufferMotionVector));
        ImGui::SameLine();
        ImGui::RadioButton("PBR Params", &renderViewMode, static_cast<int>(RenderViewMode::GBufferPBRParams));
        ImGui::SameLine();
        ImGui::RadioButton("Emissive", &renderViewMode, static_cast<int>(RenderViewMode::GBufferEmissive));
        ImGui::SameLine();
        ImGui::RadioButton("Depth", &renderViewMode, static_cast<int>(RenderViewMode::Depth));
        ImGui::SameLine();
        ImGui::RadioButton("ReflectionDir", &renderViewMode, static_cast<int>(RenderViewMode::ReflectionDirection));
        ImGui::RadioButton("ViewDir", &renderViewMode, static_cast<int>(RenderViewMode::ViewDirection));
        ImGui::SameLine();
        ImGui::RadioButton("WorldPos", &renderViewMode, static_cast<int>(RenderViewMode::WorldPosition));
        ImGui::SameLine();
        ImGui::RadioButton("NdotV", &renderViewMode, static_cast<int>(RenderViewMode::NdotV));
        ImGui::RadioButton("IBL Env", &renderViewMode, static_cast<int>(RenderViewMode::IblEnvironment));
        ImGui::SameLine();
        ImGui::RadioButton("IBL Irradiance", &renderViewMode, static_cast<int>(RenderViewMode::IblDiffuseIrradiance));
        ImGui::SameLine();
        ImGui::RadioButton("IBL Prefilter", &renderViewMode, static_cast<int>(RenderViewMode::IblSpecularPrefilter));
        ImGui::SameLine();
        ImGui::RadioButton("IBL BRDF LUT", &renderViewMode, static_cast<int>(RenderViewMode::IblBrdfLut));
        ImGui::BeginDisabled(!context.rayTracingSupported);
        ImGui::RadioButton("Shadow Mask", &renderViewMode, static_cast<int>(RenderViewMode::ShadowMask));
        ImGui::SameLine();
        ImGui::RadioButton("TLAS Debug", &renderViewMode, static_cast<int>(RenderViewMode::TlasDebug));
        ImGui::EndDisabled();
        m_renderViewMode = static_cast<RenderViewMode>(renderViewMode);
        if (!context.rayTracingSupported &&
            (m_renderViewMode == RenderViewMode::ShadowMask || m_renderViewMode == RenderViewMode::TlasDebug))
        {
            m_renderViewMode = RenderViewMode::LightPass;
        }
        const bool iblDebugView = m_renderViewMode == RenderViewMode::IblEnvironment ||
            m_renderViewMode == RenderViewMode::IblDiffuseIrradiance ||
            m_renderViewMode == RenderViewMode::IblSpecularPrefilter;
        ImGui::BeginDisabled(!iblDebugView);
        ImGuiWidgets::SliderFloatWithControls(
            "IBL Cube Exposure", &m_lightingParams.iblDebugExposure, 0.01f, 2.0f, 0.05f, 0.25f);
        ImGui::EndDisabled();
        ImGui::BeginDisabled(m_renderViewMode != RenderViewMode::IblSpecularPrefilter);
        ImGuiWidgets::SliderFloatWithControls("Prefilter Mip",
                                              &m_lightingParams.iblDebugMip,
                                              0.0f,
                                              static_cast<float>(HelloTextureEngine::kSpecularPrefilterMipCount - 1),
                                              1.0f,
                                              0.0f);
        ImGui::EndDisabled();
        ImGui::EndDisabled();

        if (!deferredRendering)
        {
            m_renderViewMode = RenderViewMode::LightPass;
        }

        const bool lightPassView = deferredRendering && m_renderViewMode == RenderViewMode::LightPass;
        ImGui::BeginDisabled(!lightPassView);
        ImGui::Checkbox("Debug LightPass Gradient", &m_lightingPassDebugGradient);
        ImGui::EndDisabled();

        if (lightPassView && m_lightingPassDebugGradient)
        {
            if (ImGui::Button("Validate HDR Gradient"))
            {
                m_requestHdrDump = true;
            }
        }

        ImGui::Separator();
        if (ImGui::Button("Run Descriptor Allocator Tests"))
        {
            RunStagedAllocatorTests(m_graphicsDevice.Device());
        }
    }

    if (ImGui::CollapsingHeader("Pixel Pick (Ctrl+Click)", ImGuiTreeNodeFlags_DefaultOpen))
    {
        const auto& pick = m_engine.GetPixelPickResult();
        if (pick.valid)
        {
            ImGui::Text("Screen: (%d, %d)", pick.screenX, pick.screenY);
            ImGui::Text("Depth (NDC): %.4f", pick.depthNdc);
            ImGui::Text("World Pos:   (%.3f, %.3f, %.3f)", pick.worldPos.x, pick.worldPos.y, pick.worldPos.z);
            ImGui::Text("Normal:      (%.3f, %.3f, %.3f)", pick.normal.x, pick.normal.y, pick.normal.z);
            ImGui::Text("View Dir:    (%.3f, %.3f, %.3f)", pick.viewDir.x, pick.viewDir.y, pick.viewDir.z);
            ImGui::Text("Reflect Dir: (%.3f, %.3f, %.3f)", pick.reflectionDir.x, pick.reflectionDir.y,
                         pick.reflectionDir.z);
        }
        else
        {
            ImGui::Text("Ctrl+Click on viewport to pick a pixel.");
        }
    }

    if (ImGui::CollapsingHeader("Specular Debug Lines", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto debugLines = m_engine.GetSpecularDebugLineSettings();

        ImGui::Checkbox("Enable Debug Lines", &debugLines.enabled);
        ImGui::SliderFloat("Line Length", &debugLines.lineLength, 0.1f, 5.0f, "%.1f");

        ImGui::Checkbox("View Ray (yellow)", &debugLines.showViewRay);
        ImGui::Checkbox("Normal (blue)", &debugLines.showNormal);
        ImGui::Checkbox("Reflection (magenta)", &debugLines.showReflection);

        // Compact legend
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "---");
        ImGui::SameLine();
        ImGui::Text("View Ray");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "---");
        ImGui::SameLine();
        ImGui::Text("Normal");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "---");
        ImGui::SameLine();
        ImGui::Text("Reflection");

        m_engine.SetSpecularDebugLineSettings(debugLines);
    }

    if (ImGui::CollapsingHeader("WorkMeter"))
    {
        ImGui::Text("CPU Frame: %.2f ms (%.1f FPS)", context.cpuFrameTime, 1000.0f / context.cpuFrameTime);

        const auto& gpuCheckPoints = context.gpuCheckPoints;
        const size_t gpuCheckPointCount = gpuCheckPoints.size();
        if (gpuCheckPointCount >= 2)
        {
            for (int i = 1; i < static_cast<int>(gpuCheckPointCount); i++)
            {
                const auto& checkPoint = gpuCheckPoints[i];
                if (i < static_cast<int>(gpuCheckPointCount) - 1)
                {
                    const float timeFromPrevious = checkPoint.timeStamp - gpuCheckPoints[i - 1].timeStamp;
                    ImGui::Text("GPU[%d] %s: %f ms", i, checkPoint.name.c_str(), timeFromPrevious);
                }
                else
                {
                    ImGui::Text("GPU[%d] Total: %f ms", i, checkPoint.timeStamp);
                }
            }
        }
    }

    ImGui::End();

    HelloTextureEngine::LightingParams lightingParams = m_lightingParams;
    if (!m_iblEnabled)
    {
        lightingParams.iblIntensity = 0.0f;
    }
    m_engine.SetLightingParams(lightingParams);
    m_engine.SetRenderingPath(m_renderingPath);
    m_engine.SetLightingPassDebugGradient(m_lightingPassDebugGradient);
    m_engine.SetBackBufferClearColor(m_backBufferClearColor);
    m_engine.SetDisplayInstanceCount(loadedScene.DisplayInstanceCount());
    m_engine.SetToneMapParams(m_toneMapParams);
    m_engine.SetRenderViewMode(m_renderViewMode);
    m_engine.SetRequestHdrDump(m_requestHdrDump);
    m_requestHdrDump = false;
}
