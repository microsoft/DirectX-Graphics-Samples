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

PnpListener::PnpListener(std::wstring DeviceInstanceId) : m_DeviceInstanceId(DeviceInstanceId)
{
    // Create the manual reset events
    m_PnpStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_PnpStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!m_PnpStartEvent || !m_PnpStopEvent)
    {
        ScopedOutputColor outClr(ScopedOutputColor::COLOR_RED);
        std::cout << "Error: Failed to create manual reset events" << std::endl;
        m_CmNotification = nullptr;
        return;
    }

    // Register the listener
    CONFIGRET cr;
    CM_NOTIFY_FILTER Filter = {};
    Filter.cbSize = sizeof(Filter);

    Filter.FilterType = CM_NOTIFY_FILTER_TYPE_DEVICEINTERFACE;
    Filter.u.DeviceInterface.ClassGuid = GUID_DISPLAY_DEVICE_ARRIVAL;

    cr = CM_Register_Notification(&Filter, this, PnPStartListener, &m_CmNotification);
    if (cr != CR_SUCCESS)
    {
        ScopedOutputColor outClr(ScopedOutputColor::COLOR_RED);
        std::cout << "Error: CM_Register_Notification failed, error code " << cr << std::endl;
        m_CmNotification = nullptr;
    }
}

PnpListener::~PnpListener()
{
    // Un-register the notification
    if (m_CmNotification)
    {
        CM_Unregister_Notification(m_CmNotification);
        m_CmNotification = nullptr;
    }

    if (m_PnpStartEvent)
    {
        CloseHandle(m_PnpStartEvent);
        m_PnpStartEvent = nullptr;
    }

    if (m_PnpStopEvent)
    {
        CloseHandle(m_PnpStopEvent);
        m_PnpStopEvent = nullptr;
    }
}

DWORD WINAPI PnpListener::PnPStartListener(
    _In_ HCMNOTIFICATION hNotify,
    _In_opt_ PVOID pContext,
    _In_ CM_NOTIFY_ACTION Action,
    _In_reads_bytes_(EventDataSize) PCM_NOTIFY_EVENT_DATA EventData,
    _In_ DWORD EventDataSize
)
{
    PnpListener* pThis = (PnpListener*)pContext;

    UNREFERENCED_PARAMETER(hNotify);
    UNREFERENCED_PARAMETER(EventDataSize);

    ULONG BufferSize = MAX_DEVICE_ID_LEN * sizeof(WCHAR);
    DEVPROPTYPE PropType;
    WCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN] = {};
    if ((CR_SUCCESS != CM_Get_Device_Interface_Property(EventData->u.DeviceInterface.SymbolicLink,
        &DEVPKEY_Device_InstanceId,
        &PropType,
        (PBYTE)DeviceInstanceId,
        &BufferSize,
        0)) ||
        (PropType != DEVPROP_TYPE_STRING))
    {
        DeviceInstanceId[0] = L'\0';
    }

    if (!_wcsnicmp(pThis->m_DeviceInstanceId.c_str(), DeviceInstanceId, ARRAYSIZE(DeviceInstanceId)))
    {
        if (Action == CM_NOTIFY_ACTION_DEVICEINTERFACEARRIVAL)
        {
            SetEvent(pThis->m_PnpStartEvent);
        }
        else if ((Action == CM_NOTIFY_ACTION_DEVICEINTERFACEREMOVAL) ||
            (Action == CM_NOTIFY_ACTION_DEVICEREMOVECOMPLETE))
        {
            SetEvent(pThis->m_PnpStopEvent);
        }
    }

    return ERROR_SUCCESS;
}

bool PnpListener::WaitForPnpStart()
{
    DWORD Ret = WaitForSingleObject(m_PnpStartEvent, PnpListener::WaitTimeout);
    return Ret == WAIT_OBJECT_0;
}

bool PnpListener::WaitForPnpStop()
{
    DWORD Ret = WaitForSingleObject(m_PnpStopEvent, PnpListener::WaitTimeout);
    return Ret == WAIT_OBJECT_0;
}

bool PnpListener::HasError()
{
    return m_CmNotification == nullptr;
}
