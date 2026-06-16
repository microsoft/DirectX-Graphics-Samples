#pragma once

#include "../GraphicsDevice.h"
#include "../Renderer/SimpleDescriptorHeapAllocator.h"

namespace Engine
{

class ImGuiSystem
{
public:
    void Initialize(HWND hwnd,
                    GraphicsDevice& device,
                    ID3D12DescriptorHeap* srvHeap,
                    UINT frameCount,
                    DXGI_FORMAT rtvFormat);
    void BeginFrame();
    void EndFrame();
    void Render(ID3D12GraphicsCommandList* commandList);
    void SetDisplaySize(UINT width, UINT height);
    void Shutdown();

private:
    SimpleDescriptorHeapAllocator m_descriptorHeapAllocator;
    ID3D12DescriptorHeap* m_srvHeap = nullptr;
    bool m_initialized = false;
};

} // namespace Engine
