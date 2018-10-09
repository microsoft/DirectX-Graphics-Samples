# DXGI AdapterRemovalSupport test sample

This sample test simulates D3D device removed events by disabling and enabling the hardware graphics/display adapters on the system. This helps determine if an application is able to support and survive adapter removals.

If the result of test is that the application seems to support adapter removal properly, then the application should declare it via DXGIDeclareAdapterRemovalSupport API. This allows Windows to identify such applications and act accordingly, for example, to allow safe detachment of the xGPU even when this application is rendering on it.

An example of an application that supports adapter removal behavior is the [D3D12xGPU sample application](../../Samples/Desktop/D3D12xGPU/readme.md).

### Command parameters usage
* -? (shows this help screen)
* -appPath \{proc.exe}  [-launch] (to launch the application)
* -appPID \{#} (process id of currently running app)
* [-wait \{#}] (waits # seconds before simulating device removal)

### Example commands
* DXGIAdapterRemovalSupportTest.exe -appPath notepad.exe -launch
* DXGIAdapterRemovalSupportTest.exe -appPath gameApp.exe
* DXGIAdapterRemovalSupportTest.exe -appPID 12345

## Requirements
* Windows 10 16299 or higher
* [Visual Studio 2017](https://www.visualstudio.com/) with the Windows 10 16299 SDK