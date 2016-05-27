# DirectX12 + XAML Template
This is a *preview* of the DirectX12 + XAML template for Visual Studio. This template is provided in its current form as a starting point for developers who are interested in using DirectX12 hosted within the XAML framework.

## Known issues
* Device Removed recovery is not finalized.
  * The XAML framework keeps an additional reference to the DXGI swap chain and does not release it at the exact point in time that SetSwapChain(nullptr) is called. Because D3D12CreateDevice does not create a new device until the old device is deleted, it will return a reference to the existing device which has been removed. Trying to use the old, removed device will result in an application crash.