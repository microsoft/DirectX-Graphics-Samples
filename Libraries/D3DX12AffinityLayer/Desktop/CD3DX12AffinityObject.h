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

#include "Utils.h"

class __declspec(uuid("BE1D71C8-88FD-4623-ABFA-D0E546D12FAF")) CD3DX12AffinityObject : public IUnknown
{
public:
#pragma region
    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid,
        __RPC__deref_out void __RPC_FAR* __RPC_FAR* ppvObject
    )
    {
        //Simple QueryInterface implementation, Not real COM
        *ppvObject = this;
        AddRef();
        return S_OK;
    }
#pragma endregion

    HRESULT STDMETHODCALLTYPE GetPrivateData(
        _In_  REFGUID guid,
        _Inout_  UINT* pDataSize,
        _Out_writes_bytes_opt_(*pDataSize)  void* pData,
        UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE SetPrivateData(
        _In_  REFGUID guid,
        _In_  UINT DataSize,
        _In_reads_bytes_opt_(DataSize)  const void* pData,
        UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(
        _In_  REFGUID guid,
        _In_opt_  const IUnknown* pData,
        UINT AffinityIndex = 0);

    HRESULT STDMETHODCALLTYPE SetName(
        _In_z_  LPCWSTR Name,
        UINT AffinityIndex = 0);

    virtual void STDMETHODCALLTYPE SetAffinity(
        _In_  UINT AffinityMask);

    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    CD3DX12AffinityObject(CD3DX12AffinityDevice* device, IUnknown** objects, UINT Count);
    virtual ~CD3DX12AffinityObject();

    CD3DX12AffinityDevice* GetParentDevice();

    IUnknown* mObjects[D3DX12_MAX_ACTIVE_NODES];
    UINT mAffinityMask = 0;
    UINT mReferenceCount;

#ifdef DEBUG_OBJECT_NAME
    std::wstring mObjectTypeName;
    std::wstring mObjectDebugName;
#endif
    CD3DX12AffinityDevice* mParentDevice;

    static UINT AffinityIndexToMask(UINT const Index);
    UINT GetNodeMask();

    UINT GetNodeCount();
    UINT GetActiveNodeIndex();

protected:
    virtual bool IsD3D() = 0;

private:
    // Non-copyable
    CD3DX12AffinityObject(CD3DX12AffinityObject const&);
    CD3DX12AffinityObject& operator=(CD3DX12AffinityObject const&);
};
