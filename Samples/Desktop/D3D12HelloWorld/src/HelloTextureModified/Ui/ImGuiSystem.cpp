#include "stdafx.h"

#include "ImGuiSystem.h"

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

namespace Engine
{

namespace
{
SimpleDescriptorHeapAllocator* g_allocator = nullptr;
}

void ImGuiSystem::Initialize(HWND hwnd,
                             GraphicsDevice& device,
                             ID3D12DescriptorHeap* srvHeap,
                             UINT frameCount,
                             DXGI_FORMAT rtvFormat)
{
    m_srvHeap = srvHeap;
    m_descriptorHeapAllocator.Init(device.Device(), m_srvHeap);
    g_allocator = &m_descriptorHeapAllocator;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);

    ImGui_ImplDX12_InitInfo initInfo = {};
    initInfo.Device = device.Device();
    initInfo.CommandQueue = device.CommandQueue();
    initInfo.NumFramesInFlight = frameCount;
    initInfo.RTVFormat = rtvFormat;
    initInfo.DSVFormat = DXGI_FORMAT_UNKNOWN;
    initInfo.SrvDescriptorHeap = m_srvHeap;
    initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*,
                                       D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle,
                                       D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle)
    { g_allocator->Alloc(outCpuHandle, outGpuHandle); };
    initInfo.SrvDescriptorFreeFn =
        [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
    { g_allocator->Free(&cpuHandle, &gpuHandle); };
    ImGui_ImplDX12_Init(&initInfo);

    m_initialized = true;
}

void ImGuiSystem::BeginFrame()
{
    if (!m_initialized)
    {
        return;
    }

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiSystem::EndFrame()
{
    if (!m_initialized)
    {
        return;
    }

    ImGui::Render();
}

void ImGuiSystem::Render(ID3D12GraphicsCommandList* commandList)
{
    if (!m_initialized)
    {
        return;
    }

    ID3D12DescriptorHeap* imguiHeaps[] = {m_srvHeap};
    commandList->SetDescriptorHeaps(1, imguiHeaps);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}

void ImGuiSystem::SetDisplaySize(UINT width, UINT height)
{
    if (!m_initialized)
    {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
}

void ImGuiSystem::Shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    m_initialized = false;
    m_srvHeap = nullptr;
    if (g_allocator == &m_descriptorHeapAllocator)
    {
        g_allocator = nullptr;
    }
}

} // namespace Engine
