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
    }

private:
    bool m_probed = false;
};

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int show)
{
    SampleLog::Init();
    BenchmarkIsolationSample sample;
    return Win32Application::Run(&sample, instance, show);
}
