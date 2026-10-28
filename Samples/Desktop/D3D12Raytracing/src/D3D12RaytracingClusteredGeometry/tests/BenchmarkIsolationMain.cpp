#include "stdafx.h"
#include "D3D12RaytracingClusteredGeometry.h"

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = D3D12_PREVIEW_SDK_VERSION; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }

class BenchmarkIsolationSample : public D3D12RaytracingClusteredGeometry
{
public:
    BenchmarkIsolationSample()
        : D3D12RaytracingClusteredGeometry(1280, 720, L"Benchmark isolation test") {}

    void OnRender() override
    {
        if (GetEnvironmentVariableW(L"BENCHMARK_TEST_SCHEDULED_TRADITIONAL_TOGGLE", nullptr, 0) != 0 &&
            (m_frame == 9 || m_frame == 17))
        {
            const bool updateAllowed = (TraditionalAnimatedBuildFlags() &
                D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE) != 0;
            ThrowIfFalse(updateAllowed == (m_frame == 9),
                L"Scheduled traditional mode switch retained incorrect update flags.\n");
            SampleLog::LogF(L"[isolation-test] scheduled traditional toggle frame=%u update=%u\n",
                           m_frame, updateAllowed ? 1u : 0u);
        }
        if (GetEnvironmentVariableW(L"BENCHMARK_TEST_TRADITIONAL_TOGGLE", nullptr, 0) != 0 &&
            (m_frame == 8 || m_frame == 16))
        {
            OnKeyDown('F');
            const bool updateAllowed = (TraditionalAnimatedBuildFlags() &
                D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE) != 0;
            ThrowIfFalse(updateAllowed == (m_frame == 8) &&
                (TraditionalStaticBuildFlags() &
                 D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE) == 0,
                L"Traditional mode switch retained incorrect update flags.\n");
            SampleLog::LogF(L"[isolation-test] traditional toggle frame=%u update=%u\n",
                           m_frame, updateAllowed ? 1u : 0u);
        }
        if (m_frame == 8 && GetEnvironmentVariableW(L"BENCHMARK_TEST_RECOVERY", nullptr, 0) != 0)
        {
            GetDeviceResources()->HandleDeviceLost();
            SampleLog::Write(L"[isolation-test] device recovery completed\n");
        }
        if (!m_probed)
        {
            ThrowIfFalse(IsBenchmarkMode(), L"Isolation test requires --bench-seconds.\n");
            const HWND window = Win32Application::GetHwnd();
            ThrowIfFalse(!IsWindowEnabled(window) && GetForegroundWindow() != window,
                L"Benchmark accepted input or acquired foreground focus.\n");
            ThrowIfFalse((GetWindowLongPtr(window, GWL_EXSTYLE) & WS_EX_NOACTIVATE) != 0,
                L"Benchmark window is not non-activating.\n");
            const UINT keys[] = { 'W', 'V', 'P', 'L', 'N', 'T', 'M', VK_SPACE, VK_OEM_2 };
            for (const UINT key : keys)
            {
                SendMessage(window, WM_KEYDOWN, key, 0);
                SendMessage(window, WM_KEYUP, key, 0);
            }
            SendMessage(window, WM_SYSKEYDOWN, VK_RETURN, 1LL << 29);
            SendMessage(window, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
            SendMessage(window, WM_SIZE, SIZE_RESTORED, MAKELPARAM(1920, 1080));
            ThrowIfFalse(SetWindowPos(window, nullptr, 0, 0, 640, 480,
                SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER) != FALSE,
                L"Could not exercise benchmark resize rejection.\n");
            RECT client = {};
            ThrowIfFalse(GetClientRect(window, &client) != FALSE &&
                client.right == 1280 && client.bottom == 720 &&
                GetForegroundWindow() != window &&
                SendMessage(window, WM_MOUSEACTIVATE, 0, 0) == MA_NOACTIVATE,
                L"Benchmark size or focus changed after input/resize messages.\n");
            SampleLog::Write(L"[isolation-test] input ignored, resize rejected, no activation, 1280x720\n");
            m_probed = true;
        }
        D3D12RaytracingClusteredGeometry::OnRender();
        ++m_frame;
    }

private:
    bool m_probed = false;
    UINT m_frame = 0;
};

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int show)
{
    SampleLog::Init();
    const bool validation = GetEnvironmentVariableW(L"BENCHMARK_TEST_VALIDATION", nullptr, 0) != 0;
    if (validation)
    {
        ComPtr<ID3D12Debug> debug;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)));
        debug->EnableDebugLayer();
    }
    BenchmarkIsolationSample sample;
    const int result = Win32Application::Run(&sample, instance, show);
    if (validation && sample.GetDeviceResources())
    {
        ComPtr<ID3D12InfoQueue> info;
        ThrowIfFailed(sample.GetDeviceResources()->GetD3DDevice()->QueryInterface(IID_PPV_ARGS(&info)));
        for (UINT64 i = 0; i < info->GetNumStoredMessages(); ++i)
        {
            SIZE_T size = 0;
            ThrowIfFailed(info->GetMessage(i, nullptr, &size));
            std::vector<uint8_t> storage(size);
            auto* message = reinterpret_cast<D3D12_MESSAGE*>(storage.data());
            ThrowIfFailed(info->GetMessage(i, message, &size));
            if (message->Severity <= D3D12_MESSAGE_SEVERITY_ERROR)
            {
                OutputDebugStringA(message->pDescription);
                SampleLog::LogF(L"[isolation-test] D3D12 validation error ID=%u\n", message->ID);
                return EXIT_FAILURE;
            }
        }
        SampleLog::Write(L"[isolation-test] zero D3D12 validation errors\n");
    }
    return result;
}
