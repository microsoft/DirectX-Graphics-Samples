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

#define COM_IMPLEMENTATION()                                                   \
public:                                                                        \
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(                          \
        REFIID riid,                                                           \
        void **ppvObject)                                                      \
    {                                                                          \
        UNREFERENCED_PARAMETER(riid);                                          \
        *ppvObject = nullptr;                                                  \
        return E_NOINTERFACE;                                                  \
    }                                                                          \
                                                                               \
    virtual ULONG STDMETHODCALLTYPE AddRef(void)                               \
    {                                                                          \
        return InterlockedIncrement(&m_ReferenceCount);                        \
    }                                                                          \
                                                                               \
    virtual ULONG STDMETHODCALLTYPE Release(void)                              \
    {                                                                          \
        ULONG currentRefCount = InterlockedDecrement(&m_ReferenceCount);       \
        if (currentRefCount == 0)                                              \
        {                                                                      \
            delete this;                                                       \
        }                                                                      \
        return currentRefCount;                                                \
    }                                                                          \
private:                                                                       \
    UINT m_ReferenceCount = 1;                                                 

#define COM_IMPLEMENTATION_WITH_QUERYINTERFACE(pParentObject)                  \
public:                                                                        \
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(                          \
        REFIID riid,                                                           \
        void **ppvObject)                                                      \
    {                                                                          \
        if (riid == __uuidof(*pParentObject))                                  \
        {                                                                      \
            pParentObject->AddRef();                                           \
            *ppvObject = pParentObject;                                        \
            return S_OK;                                                       \
        }                                                                      \
        return pParentObject->QueryInterface(riid, ppvObject);                 \
    }                                                                          \
                                                                               \
    virtual ULONG STDMETHODCALLTYPE AddRef(void)                               \
    {                                                                          \
        return InterlockedIncrement(&m_ReferenceCount);                        \
    }                                                                          \
                                                                               \
    virtual ULONG STDMETHODCALLTYPE Release(void)                              \
    {                                                                          \
        ULONG currentRefCount = InterlockedDecrement(&m_ReferenceCount);       \
        if (currentRefCount == 0)                                              \
        {                                                                      \
            delete this;                                                       \
        }                                                                      \
        return currentRefCount;                                                \
    }                                                                          \
private:                                                                       \
    UINT m_ReferenceCount = 1;                                                 