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

#include "pch.h"

// Globals
OutputColor g_OutClr;
bool g_StopEtwThreads = true;

#pragma region CommandLineArgs
std::wstring tolower(const std::wstring& s)
{
    std::wstring str = s;
    std::for_each(str.begin(), str.end(), [](WCHAR& c) { c = static_cast<WCHAR>(std::tolower(c)); });
    return str;
}

const WCHAR* GetOptional(const WCHAR* const* start, const WCHAR* const* end, const WCHAR* const defaultvalue, const std::wstring& name)
{
    auto found = std::find_if(start, end, [&name](const WCHAR* s) { return tolower(std::wstring(s)) == tolower(name); });
    if (found == end || ++found == end) return defaultvalue;
    return *found;
}

const WCHAR* GetRequired(const WCHAR* const* start, const WCHAR* const* end, const std::wstring& name)
{
    return GetOptional(start, end, nullptr, name);
}

bool SwitchExists(const WCHAR* const* start, const WCHAR* const* end, const std::wstring& name)
{
    auto found = std::find_if(start, end, [&name](const WCHAR* s) { return tolower(std::wstring(s)) == tolower(name); });
    return !(found == end);
}

bool UsageRequested(const WCHAR* const* start, const WCHAR* const* end)
{
    return SwitchExists(start, end, L"-usage") || SwitchExists(start, end, L"-?") || SwitchExists(start, end, L"/?");
}

void ShowUsage()
{
    std::wcout << "    Usage:" << std::endl;
    std::wcout << "      -? (shows this help screen)" << std::endl;
    std::wcout << "      -appPath {proc.exe}  [-launch] (to launch the application)" << std::endl;
    std::wcout << "      -appPID {#} (Process ID of currently running app)" << std::endl;
    std::wcout << "      [-wait {#}] (waits # seconds before simulating adapter removal)" << std::endl;
}
#pragma endregion

#pragma region ProcessHelperFunctions
std::wstring FilterOutProcessName(std::wstring& appFullPath)
{
    // Will return the process name, and add "" to wrap the appPath
    std::wstring appPath = appFullPath;
    unsigned appStrLength = (unsigned)appPath.size();
    std::wstring appProcessName;
    if (appStrLength > 4) // contains at least ".exe"
    {
        // appFullPath is surrounded by "" by the end of this
        if (appFullPath[0] == L'\"' && appFullPath[appStrLength - 1] == L'\"')
        {
            // Remove the \" at the ends
            appPath.erase(0, 1);
            appPath.pop_back();
            appStrLength = (unsigned)appPath.size();
        }
        else
        {
            appFullPath.insert(0, 1, L'\"');
            appFullPath.push_back(L'\"');
        }

        if (appStrLength > 4)
        {
            if (appPath.compare(appStrLength - 4, 4, L".exe") == 0) // it has to end with ".exe"
            {
                appProcessName = tolower(appPath.substr(appPath.find_last_of(L"\\") + 1));
            }
        }
    }
    return appProcessName;
}

HANDLE GetExisitingProcessID(const std::wstring& appProcessName, DWORD& outProcessID)
{
    HANDLE targetProcess = NULL;
    outProcessID = 0;

    // Get the list of process identifiers
    unsigned int maxNumProcesses = 0;
    DWORD processsReturned = 0;
    std::unique_ptr<DWORD[]> pProcesses;
    while (maxNumProcesses < 5120) // limit num of processes to enum to 5120
    {
        maxNumProcesses += 1024;
        pProcesses.reset(new DWORD[maxNumProcesses]);
        unsigned int sizeOfBuffer = sizeof(DWORD) * maxNumProcesses;
        if (!EnumProcesses(pProcesses.get(), sizeOfBuffer, &processsReturned))
        {
            ScopedOutputColor outClr(ScopedOutputColor::COLOR_RED);
            std::wcout << "Error: when trying to call EnumProcesses." << std::endl;
            return NULL;
        }
        if (processsReturned < sizeOfBuffer) // We got all the processes currently running
        {
            break;
        }
    }

    unsigned int numProcesses = processsReturned / sizeof(DWORD); // num process identifiers were returned

    // Find the pid for the app process we want to test
    for (unsigned int i = 0; i < numProcesses; i++)
    {
        if (pProcesses[i] != 0)
        {
            WCHAR szProcessName[MAX_PATH];
            wcsncpy_s(szProcessName, L"<unknown>", MAX_PATH - 1);

            // Get a handle to the process
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pProcesses[i]);

            // Get the process name
            if (NULL != hProcess)
            {
                HMODULE hMod; // first module returned is the process module
                DWORD moduleSizeReturned;
                if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &moduleSizeReturned))
                {
                    GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(char));
                }
            }

            if (appProcessName.compare(tolower(std::wstring(szProcessName))) == 0)
            {
                if (!targetProcess) // Always use the first matched process
                {
                    outProcessID = pProcesses[i];
                    targetProcess = hProcess; // leave the handle to process open
                    continue;
                }
                else
                {
                    ScopedOutputColor outClr(ScopedOutputColor::COLOR_YELLOW);
                    std::wcout << "Warning: multiple processes with the same name are running." << std::endl;
                    CloseHandle(hProcess);
                    break;
                }
            }

            // Release the handle to the process.
            CloseHandle(hProcess);
        }
    }

    return targetProcess;
}

HANDLE GetProcessFromID(DWORD processID, std::wstring& outAppProcessName)
{
    // Get a handle to the process
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

    // Get the process name
    if (NULL != hProcess)
    {
        WCHAR szProcessName[MAX_PATH];
        outAppProcessName = L"<unknown>";

        HMODULE hMod; // first module returned is the process module
        DWORD moduleSizeReturned;
        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &moduleSizeReturned))
        {
            if (GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(char)) != 0)
            {
                outAppProcessName = szProcessName;
            }
        }
    }

    return hProcess;
}
#pragma endregion

#pragma region DeviceHelperFunctions
struct AdapterDeviceInfo
{
    std::wstring m_deviceInterface;
    std::wstring m_deviceId;
    LUID m_Luid = {};
    DEVINST m_devInst = 0;
};

std::wstring GetDeviceIdFromDeviceInterface(std::wstring DeviceInterface)
{
    std::vector<WCHAR> DeviceId;
    CONFIGRET Return;
    do
    {
        ULONG BufferSize = 0;
        DEVPROPTYPE PropertyType;
        Return = CM_Get_Device_Interface_PropertyW(DeviceInterface.c_str(), &DEVPKEY_Device_InstanceId, &PropertyType, nullptr, &BufferSize, 0);
        if (Return != CR_BUFFER_SMALL)
        {
            ScopedOutputColor outClr(ScopedOutputColor::COLOR_RED);
            std::wcout << L"Error (" << (UINT)Return << L"): when calling CM_Get_Device_Interface_PropertyW for " << DeviceInterface.c_str() << std::endl;
            DeviceId.clear();
            break;
        }

        DeviceId.resize(BufferSize / sizeof(WCHAR));
        Return = CM_Get_Device_Interface_PropertyW(DeviceInterface.c_str(), &DEVPKEY_Device_InstanceId, &PropertyType,
            reinterpret_cast<PBYTE>(DeviceId.data()), &BufferSize, 0);
    } while (Return == CR_BUFFER_SMALL);

    return std::wstring(DeviceId.begin(), DeviceId.end());
}

std::vector<AdapterDeviceInfo> EnumerateAdapterDeviceInfo()
{
    // List out all the AdapterDevicePath of all graphics adapters, display or render
    std::vector<WCHAR> devicesStringBuffer;
    CONFIGRET Return;
    do
    {
        // Get the buffer size
        ULONG BufferSize = 0;
        Return = CM_Get_Device_Interface_List_SizeW(&BufferSize, const_cast<LPGUID>(&GUID_DISPLAY_DEVICE_ARRIVAL), nullptr, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
        if (Return != CR_SUCCESS)
        {
            break;
        }

        // Read the buffer
        devicesStringBuffer.resize(BufferSize);
        Return = CM_Get_Device_Interface_ListW(const_cast<LPGUID>(&GUID_DISPLAY_DEVICE_ARRIVAL), nullptr, devicesStringBuffer.data(), (ULONG)devicesStringBuffer.size(), CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
    } while (Return == CR_BUFFER_SMALL);

    // Parse the buffer into each adapter string
    std::vector<AdapterDeviceInfo> adapterInfos;
    auto startIt = devicesStringBuffer.begin();
    for (auto it = devicesStringBuffer.begin(); it != devicesStringBuffer.end(); ++it)
    {
        if (*it == L'\0')
        {
            // Only save strings if they are not empty
            if (it - startIt > 1)
            {
                adapterInfos.push_back(AdapterDeviceInfo());
                adapterInfos.back().m_deviceInterface = std::wstring(startIt, it);
            }

            // Continue to search for next string with one-past-this-null as the start
            startIt = it;
            ++startIt;
        }
    }

    // Retrieve respective adapter information
    bool bError = false;
    for (auto& adapterInfo : adapterInfos)
    {
        D3DKMT_OPENADAPTERFROMDEVICENAME OpenAdapterFromDeviceName = {};
        OpenAdapterFromDeviceName.pDeviceName = adapterInfo.m_deviceInterface.c_str();
        NTSTATUS Status = D3DKMTOpenAdapterFromDeviceName(&OpenAdapterFromDeviceName);
        if (!NT_SUCCESS(Status))
        {
            bError = true;
            continue;
        }
        adapterInfo.m_Luid = OpenAdapterFromDeviceName.AdapterLuid;

        D3DKMT_CLOSEADAPTER CloseAdapter = { OpenAdapterFromDeviceName.hAdapter };
        D3DKMTCloseAdapter(&CloseAdapter);

        adapterInfo.m_deviceId = GetDeviceIdFromDeviceInterface(adapterInfo.m_deviceInterface);

        // Locate the device node by path
        CONFIGRET Return = CM_Locate_DevNodeW(&adapterInfo.m_devInst, const_cast<DEVINSTID_W>(adapterInfo.m_deviceId.c_str()), CM_LOCATE_DEVNODE_NORMAL);
        if (Return != CR_SUCCESS)
        {
            bError = true;
            continue;
        }
    }

    if (bError)
    {
        ScopedOutputColor outClr(ScopedOutputColor::COLOR_YELLOW);
        std::wcout << L"Warning: Some adapter info were unable to be enumerated." << std::endl;
    }
    return adapterInfos;
}

DEVINST FindAdapterDevInst(LUID AdapterLuid, std::wstring& DeviceIdString, const std::vector<AdapterDeviceInfo>& adapterInfos)
{
    for (const auto& adapterInfo : adapterInfos)
    {
        if (RtlEqualLuid(&AdapterLuid, &adapterInfo.m_Luid))
        {
            DeviceIdString = adapterInfo.m_deviceId;
            return adapterInfo.m_devInst;
        }
    }

    ScopedOutputColor outClr(ScopedOutputColor::COLOR_RED);
    std::wcout << L"Error: Unable to locate device " << DeviceIdString.c_str() << L" from luid (" 
        << AdapterLuid.HighPart << L"," << AdapterLuid.LowPart << L")" << std::endl;
    return 0;
}
#pragma endregion

#pragma region ETWTraceConsumerHelperFunctions
struct EtwThreadArgs
{
    EtwThreadArgs(DWORD pid) : m_targetPID(pid)
    { }

    DWORD m_targetPID;
    UINT m_numPresents = 0;
    UINT m_numDevicesDestroyed = 0;
    UINT m_numDevicesCreated = 0;
};

struct __declspec(uuid("{802ec45a-1e99-4b83-9920-87c98277ba9d}")) DXGKRNL_PROVIDER_GUID_HOLDER;
static const auto DXGKRNL_PROVIDER_GUID = __uuidof(DXGKRNL_PROVIDER_GUID_HOLDER);

bool EtwThreadsShouldQuit()
{
    return g_StopEtwThreads;
}

void HandleDXGKEvent(EVENT_RECORD* pEventRecord, EtwThreadArgs* pArgs)
{
    auto& hdr = pEventRecord->EventHeader;

    if (hdr.ProviderId != DXGKRNL_PROVIDER_GUID ||
        hdr.ProcessId != pArgs->m_targetPID)
    {
        return;
    }

    enum
    {
        DxgKrnl_CreateDevice = 27,
        DxgKrnl_DestroyDevice = 28,
        DxgKrnl_Present = 184
    };
    if (hdr.EventDescriptor.Id == DxgKrnl_CreateDevice)
    {
        ++pArgs->m_numDevicesCreated;
    }
    else if (hdr.EventDescriptor.Id == DxgKrnl_DestroyDevice)
    {
        ++pArgs->m_numDevicesDestroyed;
    }
    else if (hdr.EventDescriptor.Id == DxgKrnl_Present)
    {
        ++pArgs->m_numPresents;
    }
}

void EtwConsumingThread(EtwThreadArgs* pArgs)
{
    if (EtwThreadsShouldQuit()) {
        return;
    }

    TraceSession session;
    session.AddProviderAndHandler(DXGKRNL_PROVIDER_GUID, TRACE_LEVEL_VERBOSE, 0, 0, (EventHandlerFn)&HandleDXGKEvent, pArgs);

    if (!session.InitializeRealtime("DeviceLostETW", &EtwThreadsShouldQuit))
    {
        ScopedOutputColor outClr(ScopedOutputColor::COLOR_RED);
        std::cout << "Error: Failure on the etw thread when trying to log ETW events." << std::endl;
        return;
    }

    // Will process ETW events until EtwThreadsShouldQuit returns true
    auto status = ProcessTrace(&session.traceHandle_, 1, NULL, NULL);
    status;

    session.Finalize();
}
#pragma endregion

#pragma region TestLogicFunctions
bool RetrieveProcessInfo(std::wstring appFullPath, bool bToLaunch,
    HANDLE& targetProcess, std::wstring& appProcessName, DWORD& appPID)
{
    if (appPID != 0)
    {
        // Open the process handle and retrieve the process name
        targetProcess = GetProcessFromID(appPID, appProcessName);

        if (!targetProcess)
        {
            ScopedOutputColor outClr(ScopedOutputColor::COLOR_RED);
            std::wcout << L"Error: Unable to find target app PID " << appPID << L"." << std::endl;
            return false;
        }
    }
    else // appFullPath
    {
        // Parse appFullPath to the process name
        appProcessName = FilterOutProcessName(appFullPath);
        if (appProcessName.empty())
        {
            ScopedOutputColor outClr(ScopedOutputColor::COLOR_RED);
            std::wcout << L"Error: param -appPath " << appFullPath.c_str() << L" is in an incorrect format. eg. C:\\...\\notepad.exe" << std::endl;
            return false;
        }

        // Launch or check if process is already running
        if (bToLaunch)
        {
            PROCESS_INFORMATION pi;
            STARTUPINFO si = { 0 };
            si.wShowWindow = SW_MINIMIZE;
            WCHAR commandLine[MAX_PATH];
            wcsncpy_s(commandLine, appFullPath.c_str(), MAX_PATH - 1);

            std::wcout << L"Launching target app " << appFullPath.c_str() << std::endl;
            if (!CreateProcess(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
            {
                ScopedOutputColor outClr(ScopedOutputColor::COLOR_RED);
                std::wcout << L"Error: Failed to start " << appFullPath.c_str() << L" application" << std::endl;
                return false;
            }

            appPID = pi.dwProcessId;
            targetProcess = pi.hProcess;
        }
        else
        {
            // Find the matching process ID and open the process handle
            targetProcess = GetExisitingProcessID(appProcessName, appPID);

            if (appPID == 0)
            {
                ScopedOutputColor outClr(ScopedOutputColor::COLOR_RED);
                std::wcout << L"Error: Unable to find target app process " << appProcessName.c_str() << L" in the list of currently running processes." << std::endl;
                return false;
            }
        }
    }
    return true;
}

std::vector<DXGI_ADAPTER_DESC1> EnumerateAllAdapters()
{
    std::vector<DXGI_ADAPTER_DESC1> adapterDescs;

    CComPtr<IDXGIFactory2> spFactory;
    HRESULT hr;
    if (FAILED(hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&spFactory))))
    {
        ScopedOutputColor outClr(ScopedOutputColor::COLOR_RED);
        std::wcout << L"Error: CreateDXGIFactory2 failed, HRESULT " << std::hex << hr << L"." << std::endl;
    }
    else
    {
        for (UINT i = 0; ; ++i)
        {
            CComPtr<IDXGIAdapter1> spAdapter1;
            if (FAILED(spFactory->EnumAdapters1(i, &spAdapter1)))
            {
                break;
            }

            DXGI_ADAPTER_DESC1 Desc;
            if (FAILED(spAdapter1->GetDesc1(&Desc)) ||
                ((Desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE) != 0)) // Don't add software adapters (eg. warp)
            {
                continue;
            }
            adapterDescs.push_back(Desc);
        }
    }

    return adapterDescs;
}

void TracePresents(const DWORD& appPID)
{
    const UINT numSecondsToTrace = 3; // trace for 3 sec

    EtwThreadArgs args(appPID);
    g_StopEtwThreads = false;
    std::thread etwThread = std::thread(EtwConsumingThread, &args);

    Sleep(numSecondsToTrace * 1000);

    g_StopEtwThreads = true;
    etwThread.join();

    std::wcout << L"App process is presenting at " << args.m_numPresents / numSecondsToTrace << L"FPS." << std::endl;
}
#pragma endregion

int _cdecl wmain(int argc, _In_reads_z_(argc) WCHAR** argv)
{
    try
    {
        // Print description of this test exe
        std::wcout << "This test will determine if an app is able to support and survive adapter removals." << std::endl <<
            "It will cycle through the hardware graphics adapters on the system, disable and re-enable them, while verifying that the app is still executing properly." << std::endl << std::endl;
        if (UsageRequested(argv, argv + argc) || argc <= 1)
        {
            ShowUsage(); // eg. AdapterRemovalSupportTest.exe -appPath C:\WINDOWS\system32\notepad.exe -wait 3 -launch
            return 0;
        }

        // Take in param: -appPath {proc.exe} (or -appPID {pid}) -launch -wait {sec}
        std::wstring appFullPath = GetOptional(argv, argv + argc, L"", L"-appPath");
        DWORD appPID = std::stoul(GetOptional(argv, argv + argc, L"0", L"-appPID"));
        bool bToLaunch = SwitchExists(argv, argv + argc, L"-launch");
        unsigned waitDuration = std::stoul(GetOptional(argv, argv + argc, L"2", L"-wait"));

        if (appFullPath.empty() && appPID == 0)
        {
            {
                ScopedOutputColor outClr(ScopedOutputColor::COLOR_RED);
                std::wcout << L"Error: either param -appPath or -appPID has to be specified." << std::endl;
            }
            ShowUsage();
            return 1;
        }

        HANDLE targetProcess = 0;
        std::wstring appProcessName;

        auto ProcessCleanup = MakeScopeExit([&]() {
            g_OutClr.SetDefaultColor(); // Reset back to default console color

            if (targetProcess)
            {
                if (bToLaunch)
                {
                    if (TerminateProcess(targetProcess, 0))
                    {
                        std::wcout << L"Now terminating launched app process." << std::endl;
                    }

                    WaitForSingleObject(targetProcess, INFINITE);
                }
                else
                {
                    // Release the handle to the process.
                    CloseHandle(targetProcess);
                }
            }

            std::wcout << L"End of test." << std::endl;
        });

        // Launch the app if requested, and get the pid, process handle and process name
        if (!RetrieveProcessInfo(appFullPath, bToLaunch, targetProcess, appProcessName, appPID))
        {
            return 1;
        }

        std::wcout << L"Waiting for " << waitDuration << L" sec, before running Adapter Removal test, on process " 
            << appProcessName.c_str() << L" (pid: " << appPID << ")" << std::endl << std::endl;
        Sleep(waitDuration * 1000);

        // Enum all adapters on system
        std::vector<DXGI_ADAPTER_DESC1> adapterDescs = EnumerateAllAdapters();
        // Able to run this test even if device only has 1 hardware graphics adapter, since the app will go to MSBDA when it is disabled
        if (adapterDescs.empty())
        {
            ScopedOutputColor outClr(ScopedOutputColor::COLOR_RED);
            std::wcout << L"Error: No hardware graphics adapters found, hence unable to run this test." << std::endl;
            return 1;
        }

        // Init adapter enumeration information
        std::vector<AdapterDeviceInfo> adapterInfos = EnumerateAdapterDeviceInfo();
        
        bool bSupportAdapterRemoval = true; // if the app truly supports adapter removal

        // For each graphics adapter
        for (UINT i = 0; i < adapterDescs.size(); ++i)
        {
            std::wcout << L"[" << i << L"], Targeting " << adapterDescs[i].Description << L" adapter." << std::endl;

            std::wstring DeviceId;
            DEVINST gpuDevNode = FindAdapterDevInst(adapterDescs[i].AdapterLuid, DeviceId, adapterInfos);
            if (gpuDevNode == 0)
            {
                return 1; // FindAdapterDevInst would have reported an error message
            }

            // Trigger a trace, to make sure app is already presenting normally
            TracePresents(appPID);

            // Start another trace, to observe device re-creation
            EtwThreadArgs argsAcrossPnP(appPID);
            g_StopEtwThreads = false;
            std::thread etwThread = std::thread(EtwConsumingThread, &argsAcrossPnP);

            // Stop adapter
            std::wcout << L"Stopping " << adapterDescs[i].Description << L" adapter (ID: " << DeviceId.c_str() << L")." << std::endl;
            PnpListener PnpWaiter(DeviceId);
            if (PnpWaiter.HasError())
            {
                return 1;
            }
            CONFIGRET Ret = CM_Disable_DevInst(gpuDevNode, CM_DISABLE_UI_NOT_OK);
            if (Ret != CR_SUCCESS)
            {
                ScopedOutputColor outClr(ScopedOutputColor::COLOR_RED);
                std::wcout << L"Error: Could not stop adapter " << adapterDescs[i].Description << L", error " << std::hex << Ret << L"." << std::endl;
                return 1;
            }
            if (!PnpWaiter.WaitForPnpStop())
            {
                ScopedOutputColor outClr(ScopedOutputColor::COLOR_RED);
                std::wcout << L"Error: Timed-out waiting to stop adapter " << adapterDescs[i].Description << L"." << std::endl;
                return 1;
            }

            // Wait a short time [1 sec, 2.5 sec] for app to recover from adapter removal
            for (UINT i = 0; i < 15; ++i)
            {
                if (argsAcrossPnP.m_numDevicesCreated > 0)
                {
                    break; // early out once we have detected some re-creation behavior
                }
                Sleep(100);
            }
            Sleep(1000);

            // Stop the trace
            g_StopEtwThreads = true;
            etwThread.join();

            std::wcout << L"App re-created " << argsAcrossPnP.m_numDevicesCreated << L" number of devices during this adapter removal." << std::endl;

            // Check if process is still alive
            DWORD processStatus = 0;
            bool bProcessSurvived = false;
            if (GetExitCodeProcess(targetProcess, &processStatus) && processStatus == STILL_ACTIVE)
            {
                bProcessSurvived = true;
                // Trigger a trace, to see if App is still presenting normally (after adapter removal)
                TracePresents(appPID);
            }
            else
            {
                ScopedOutputColor outClr(ScopedOutputColor::COLOR_RED);
                std::wcout << L"Error: App process is no longer still active." << std::endl;
                bSupportAdapterRemoval = false;
            }

            // Start the adapter back
            std::wcout << L"Starting " << adapterDescs[i].Description << L" adapter (ID: " << DeviceId.c_str() << L")." << std::endl << std::endl;
            Ret = CM_Enable_DevInst(gpuDevNode, 0);
            if (Ret != CR_SUCCESS)
            {
                ScopedOutputColor outClr(ScopedOutputColor::COLOR_RED);
                std::wcout << L"Error: Could not start adapter " << adapterDescs[i].Description << L", error " << std::hex << Ret << L"." << std::endl;
                return 1;
            }
            if (!PnpWaiter.WaitForPnpStart())
            {
                ScopedOutputColor outClr(ScopedOutputColor::COLOR_RED);
                std::wcout << L"Error: Timeout waiting to start adapter " << adapterDescs[i].Description << L"." << std::endl;
                return 1;
            }
            Sleep(1000); // wait a short time for adapter re-enable

            // Stop the test if the app is already 'not responding'
            if (!bSupportAdapterRemoval)
            {
                break;
            }
        }

        // Report test result
        if (bSupportAdapterRemoval)
        {
            ScopedOutputColor outClr(ScopedOutputColor::COLOR_GREEN);
            std::wcout << L"Result: " << appProcessName.c_str() << L" seems to support adapter removal properly." << std::endl;
        }
        else
        {
            ScopedOutputColor outClr(ScopedOutputColor::COLOR_YELLOW);
            std::wcout << L"Result: " << appProcessName.c_str() << L" does not properly support adapter removal yet." << std::endl;
        }
    }
    catch(const std::bad_alloc&)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}
