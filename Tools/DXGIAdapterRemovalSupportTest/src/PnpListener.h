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

#include <cfgmgr32.h>
#include <iostream>

class PnpListener
{
public:
    PnpListener(std::wstring DeviceInstanceId);
    ~PnpListener();

    bool WaitForPnpStart();
    bool WaitForPnpStop();

    bool HasError();

private:
    static DWORD WINAPI PnPStartListener(
        _In_ HCMNOTIFICATION hNotify,
        _In_opt_ PVOID pContext,
        _In_ CM_NOTIFY_ACTION Action,
        _In_reads_bytes_(EventDataSize) PCM_NOTIFY_EVENT_DATA EventData,
        _In_ DWORD EventDataSize);

    const UINT32 WaitTimeout = 10 * 1000;   // 10 seconds

    std::wstring m_DeviceInstanceId;
    HANDLE m_PnpStartEvent = nullptr;
    HANDLE m_PnpStopEvent = nullptr;
    HCMNOTIFICATION m_CmNotification = nullptr;
};
