//*********************************************************
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License (MIT).
//
//*********************************************************

#pragma once

#ifndef __cplusplus
#error D3DX12 requires C++
#endif

#include "d3d12.h"

//================================================================================================
// D3DX12 State Object Creation Helpers
//
// Helper classes for creating new style state objects out of an arbitrary set of subobjects.
// Uses STL
//
// Start by instantiating CD3DX12_STATE_OBJECT_DESC (see its public methods).
// One of its methods is CreateSubobject(), which has a comment showing a couple of options for
// defining subobjects using the helper classes for each subobject (CD3DX12_DXIL_LIBRARY_SUBOBJECT
// etc.). The subobject helpers each have methods specific to the subobject for configuring its
// contents.
//
//================================================================================================
#include <list>
#include <forward_list>
#include <vector>
#include <memory>
#include <string>
#include <vector>
#ifndef D3DX12_USE_ATL
#include <wrl/client.h>
#define D3DX12_COM_PTR Microsoft::WRL::ComPtr
#define D3DX12_COM_PTR_GET(x) x.Get()
#define D3DX12_COM_PTR_ADDRESSOF(x) x.GetAddressOf()
#else
#include <atlbase.h>
#define D3DX12_COM_PTR ATL::CComPtr
#define D3DX12_COM_PTR_GET(x) x.p
#define D3DX12_COM_PTR_ADDRESSOF(x) &x.p
#endif

//------------------------------------------------------------------------------------------------
class CD3DX12_STATE_OBJECT_DESC
{
public:
    CD3DX12_STATE_OBJECT_DESC() noexcept
    {
        Init(D3D12_STATE_OBJECT_TYPE_COLLECTION);
    }
    CD3DX12_STATE_OBJECT_DESC(D3D12_STATE_OBJECT_TYPE Type) noexcept
    {
        Init(Type);
    }
    void SetStateObjectType(D3D12_STATE_OBJECT_TYPE Type) noexcept { m_Desc.Type = Type; }
    operator const D3D12_STATE_OBJECT_DESC& ()
    {
        // Do final preparation work
        for (auto& ownedSubobject : m_OwnedSubobjectHelpers)
        {
#if(__cplusplus >= 201103L)
            ownedSubobject->Finalize();
#else
            ownedSubobject.m_pHelper->Finalize();
#endif
        }

        m_RepointedAssociations.clear();
        m_SubobjectArray.clear();
        m_SubobjectArray.reserve(m_Desc.NumSubobjects);
        // Flatten subobjects into an array (each flattened subobject still has a
        // member that's a pointer to its desc that's not flattened)
        for (auto Iter = m_SubobjectList.begin();
            Iter != m_SubobjectList.end(); Iter++)
        {
            m_SubobjectArray.push_back(*Iter);
            // Store new location in array so we can redirect pointers contained in subobjects
            Iter->pSubobjectArrayLocation = &m_SubobjectArray.back();
        }
        // For subobjects with pointer fields, create a new copy of those subobject definitions
        // with fixed pointers
        for (UINT i = 0; i < m_Desc.NumSubobjects; i++)
        {
            if (m_SubobjectArray[i].Type == D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION)
            {
                auto pOriginalSubobjectAssociation =
                    static_cast<const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(m_SubobjectArray[i].pDesc);
                D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION Repointed = *pOriginalSubobjectAssociation;
                auto pWrapper =
                    static_cast<const SUBOBJECT_WRAPPER*>(pOriginalSubobjectAssociation->pSubobjectToAssociate);
                Repointed.pSubobjectToAssociate = pWrapper->pSubobjectArrayLocation;
                m_RepointedAssociations.push_back(Repointed);
                m_SubobjectArray[i].pDesc = &m_RepointedAssociations.back();
            }
        }
        // Below: using ugly way to get pointer in case .data() is not defined
        m_Desc.pSubobjects = m_Desc.NumSubobjects ? &m_SubobjectArray[0] : nullptr;
        return m_Desc;
    }
    operator const D3D12_STATE_OBJECT_DESC* ()
    {
        // Cast calls the above final preparation work
        return &static_cast<const D3D12_STATE_OBJECT_DESC&>(*this);
    }

    // CreateSubobject creates a sububject helper (e.g. CD3DX12_HIT_GROUP_SUBOBJECT)
    // whose lifetime is owned by this class.
    // e.g.
    //
    //    CD3DX12_STATE_OBJECT_DESC Collection1(D3D12_STATE_OBJECT_TYPE_COLLECTION);
    //    auto Lib0 = Collection1.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    //    Lib0->SetDXILLibrary(&pMyAppDxilLibs[0]);
    //    Lib0->DefineExport(L"rayGenShader0"); // in practice these export listings might be
    //                                          // data/engine driven
    //    etc.
    //
    // Alternatively, users can instantiate sububject helpers explicitly, such as via local
    // variables instead, passing the state object desc that should point to it into the helper
    // constructor (or call mySubobjectHelper.AddToStateObject(Collection1)).
    // In this alternative scenario, the user must keep the subobject alive as long as the state
    // object it is associated with is alive, else its pointer references will be stale.
    // e.g.
    //
    //    CD3DX12_STATE_OBJECT_DESC RaytracingState2(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);
    //    CD3DX12_DXIL_LIBRARY_SUBOBJECT LibA(RaytracingState2);
    //    LibA.SetDXILLibrary(&pMyAppDxilLibs[4]); // not manually specifying exports
    //                                             // - meaning all exports in the libraries
    //                                             // are exported
    //    etc.

    template<typename T>
    T* CreateSubobject()
    {
        T* pSubobject = new T(*this);
        m_OwnedSubobjectHelpers.emplace_back(pSubobject);
        return pSubobject;
    }

private:
    D3D12_STATE_SUBOBJECT* TrackSubobject(D3D12_STATE_SUBOBJECT_TYPE Type, void* pDesc)
    {
        SUBOBJECT_WRAPPER Subobject;
        Subobject.pSubobjectArrayLocation = nullptr;
        Subobject.Type = Type;
        Subobject.pDesc = pDesc;
        m_SubobjectList.push_back(Subobject);
        m_Desc.NumSubobjects++;
        return &m_SubobjectList.back();
    }
    void Init(D3D12_STATE_OBJECT_TYPE Type) noexcept
    {
        SetStateObjectType(Type);
        m_Desc.pSubobjects = nullptr;
        m_Desc.NumSubobjects = 0;
        m_SubobjectList.clear();
        m_SubobjectArray.clear();
        m_RepointedAssociations.clear();
    }
    typedef struct SUBOBJECT_WRAPPER : public D3D12_STATE_SUBOBJECT
    {
        D3D12_STATE_SUBOBJECT* pSubobjectArrayLocation; // new location when flattened into array
                                                        // for repointing pointers in subobjects
    } SUBOBJECT_WRAPPER;
    D3D12_STATE_OBJECT_DESC m_Desc;
    std::list<SUBOBJECT_WRAPPER>   m_SubobjectList; // Pointers to list nodes handed out so
                                                    // these can be edited live
    std::vector<D3D12_STATE_SUBOBJECT> m_SubobjectArray; // Built at the end, copying list contents

    std::list<D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION>
        m_RepointedAssociations; // subobject type that contains pointers to other subobjects,
                                 // repointed to flattened array

    class StringContainer
    {
    public:
        LPCWSTR LocalCopy(LPCWSTR string, bool bSingleString = false)
        {
            if (string)
            {
                if (bSingleString)
                {
                    m_Strings.clear();
                    m_Strings.push_back(string);
                }
                else
                {
                    m_Strings.push_back(string);
                }
                return m_Strings.back().c_str();
            }
            else
            {
                return nullptr;
            }
        }
        void clear() noexcept { m_Strings.clear(); }
    private:
        std::list<std::wstring> m_Strings;
    };

    class SUBOBJECT_HELPER_BASE
    {
    public:
        SUBOBJECT_HELPER_BASE() noexcept { Init(); }
        virtual ~SUBOBJECT_HELPER_BASE() = default;
        virtual D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept = 0;
        void AddToStateObject(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
        {
            m_pSubobject = ContainingStateObject.TrackSubobject(Type(), Data());
        }
        virtual void Finalize() {};
    protected:
        virtual void* Data() noexcept = 0;
        void Init() noexcept { m_pSubobject = nullptr; }
        D3D12_STATE_SUBOBJECT* m_pSubobject;
    };

#if(__cplusplus >= 201103L)
    std::list<std::unique_ptr<SUBOBJECT_HELPER_BASE>> m_OwnedSubobjectHelpers;
#else
    class OWNED_HELPER
    {
    public:
        OWNED_HELPER(SUBOBJECT_HELPER_BASE* pHelper) noexcept { m_pHelper = pHelper; }
        ~OWNED_HELPER() { delete m_pHelper; }
        SUBOBJECT_HELPER_BASE* m_pHelper;
    };

    std::list<OWNED_HELPER> m_OwnedSubobjectHelpers;
#endif

    friend class CD3DX12_DXIL_LIBRARY_SUBOBJECT;
    friend class CD3DX12_EXISTING_COLLECTION_SUBOBJECT;
    friend class CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT;
    friend class CD3DX12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
    friend class CD3DX12_HIT_GROUP_SUBOBJECT;
    friend class CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT;
    friend class CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT;
    friend class CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT;
    friend class CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT;
    friend class CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT;
    friend class CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT;
    friend class CD3DX12_NODE_MASK_SUBOBJECT;
    friend class CD3DX12_WORK_GRAPH_SUBOBJECT;
    friend class CD3DX12_NODE_OUTPUT_OVERRIDES;
    friend class CD3DX12_SHADER_NODE;
    friend class CD3DX12_BROADCASTING_LAUNCH_NODE_OVERRIDES;
    friend class CD3DX12_COALESCING_LAUNCH_NODE_OVERRIDES;
    friend class CD3DX12_THREAD_LAUNCH_NODE_OVERRIDES;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_DXIL_LIBRARY_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_DXIL_LIBRARY_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_DXIL_LIBRARY_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetDXILLibrary(const D3D12_SHADER_BYTECODE* pCode) noexcept
    {
        static const D3D12_SHADER_BYTECODE Default = {};
        m_Desc.DXILLibrary = pCode ? *pCode : Default;
    }
    void DefineExport(
        LPCWSTR Name,
        LPCWSTR ExportToRename = nullptr,
        D3D12_EXPORT_FLAGS Flags = D3D12_EXPORT_FLAG_NONE)
    {
        D3D12_EXPORT_DESC Export;
        Export.Name = m_Strings.LocalCopy(Name);
        Export.ExportToRename = m_Strings.LocalCopy(ExportToRename);
        Export.Flags = Flags;
        m_Exports.push_back(Export);
        m_Desc.pExports = &m_Exports[0];  // using ugly way to get pointer in case .data() is not defined
        m_Desc.NumExports = static_cast<UINT>(m_Exports.size());
    }
    template<size_t N>
    void DefineExports(LPCWSTR(&Exports)[N])
    {
        for (UINT i = 0; i < N; i++)
        {
            DefineExport(Exports[i]);
        }
    }
    void DefineExports(const LPCWSTR* Exports, UINT N)
    {
        for (UINT i = 0; i < N; i++)
        {
            DefineExport(Exports[i]);
        }
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    }
    operator const D3D12_STATE_SUBOBJECT&() const noexcept { return *m_pSubobject; }
    operator const D3D12_DXIL_LIBRARY_DESC&() const noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        m_Strings.clear();
        m_Exports.clear();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_DXIL_LIBRARY_DESC m_Desc;
    CD3DX12_STATE_OBJECT_DESC::StringContainer m_Strings;
    std::vector<D3D12_EXPORT_DESC> m_Exports;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_EXISTING_COLLECTION_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_EXISTING_COLLECTION_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_EXISTING_COLLECTION_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetExistingCollection(ID3D12StateObject*pExistingCollection) noexcept
    {
        m_Desc.pExistingCollection = pExistingCollection;
        m_CollectionRef = pExistingCollection;
    }
    void DefineExport(
        LPCWSTR Name,
        LPCWSTR ExportToRename = nullptr,
        D3D12_EXPORT_FLAGS Flags = D3D12_EXPORT_FLAG_NONE)
    {
        D3D12_EXPORT_DESC Export;
        Export.Name = m_Strings.LocalCopy(Name);
        Export.ExportToRename = m_Strings.LocalCopy(ExportToRename);
        Export.Flags = Flags;
        m_Exports.push_back(Export);
        m_Desc.pExports = &m_Exports[0]; // using ugly way to get pointer in case .data() is not defined
        m_Desc.NumExports = static_cast<UINT>(m_Exports.size());
    }
    template<size_t N>
    void DefineExports(LPCWSTR(&Exports)[N])
    {
        for (UINT i = 0; i < N; i++)
        {
            DefineExport(Exports[i]);
        }
    }
    void DefineExports(const LPCWSTR* Exports, UINT N)
    {
        for (UINT i = 0; i < N; i++)
        {
            DefineExport(Exports[i]);
        }
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION;
    }
    operator const D3D12_STATE_SUBOBJECT&() const noexcept { return *m_pSubobject; }
    operator const D3D12_EXISTING_COLLECTION_DESC&() const noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        m_CollectionRef = nullptr;
        m_Strings.clear();
        m_Exports.clear();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_EXISTING_COLLECTION_DESC m_Desc;
    D3DX12_COM_PTR<ID3D12StateObject> m_CollectionRef;
    CD3DX12_STATE_OBJECT_DESC::StringContainer m_Strings;
    std::vector<D3D12_EXPORT_DESC> m_Exports;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetSubobjectToAssociate(const D3D12_STATE_SUBOBJECT& SubobjectToAssociate) noexcept
    {
        m_Desc.pSubobjectToAssociate = &SubobjectToAssociate;
    }
    void AddExport(LPCWSTR Export)
    {
        m_Desc.NumExports++;
        m_Exports.push_back(m_Strings.LocalCopy(Export));
        m_Desc.pExports = &m_Exports[0];  // using ugly way to get pointer in case .data() is not defined
    }
    template<size_t N>
    void AddExports(LPCWSTR (&Exports)[N])
    {
        for (UINT i = 0; i < N; i++)
        {
            AddExport(Exports[i]);
        }
    }
    void AddExports(const LPCWSTR* Exports, UINT N)
    {
        for (UINT i = 0; i < N; i++)
        {
            AddExport(Exports[i]);
        }
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
    }
    operator const D3D12_STATE_SUBOBJECT&() const noexcept { return *m_pSubobject; }
    operator const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION&() const noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        m_Strings.clear();
        m_Exports.clear();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION m_Desc;
    CD3DX12_STATE_OBJECT_DESC::StringContainer m_Strings;
    std::vector<LPCWSTR> m_Exports;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION() noexcept
    {
        Init();
    }
    CD3DX12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetSubobjectNameToAssociate(LPCWSTR SubobjectToAssociate)
    {
        m_Desc.SubobjectToAssociate = m_SubobjectName.LocalCopy(SubobjectToAssociate, true);
    }
    void AddExport(LPCWSTR Export)
    {
        m_Desc.NumExports++;
        m_Exports.push_back(m_Strings.LocalCopy(Export));
        m_Desc.pExports = &m_Exports[0];  // using ugly way to get pointer in case .data() is not defined
    }
    template<size_t N>
    void AddExports(LPCWSTR (&Exports)[N])
    {
        for (UINT i = 0; i < N; i++)
        {
            AddExport(Exports[i]);
        }
    }
    void AddExports(const LPCWSTR* Exports, UINT N)
    {
        for (UINT i = 0; i < N; i++)
        {
            AddExport(Exports[i]);
        }
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
    }
    operator const D3D12_STATE_SUBOBJECT&() const noexcept { return *m_pSubobject; }
    operator const D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION&() const noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        m_Strings.clear();
        m_SubobjectName.clear();
        m_Exports.clear();
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION m_Desc;
    CD3DX12_STATE_OBJECT_DESC::StringContainer m_Strings;
    CD3DX12_STATE_OBJECT_DESC::StringContainer m_SubobjectName;
    std::vector<LPCWSTR> m_Exports;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_HIT_GROUP_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_HIT_GROUP_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_HIT_GROUP_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetHitGroupExport(LPCWSTR exportName)
    {
        m_Desc.HitGroupExport = m_Strings[0].LocalCopy(exportName, true);
    }
    void SetHitGroupType(D3D12_HIT_GROUP_TYPE Type) noexcept { m_Desc.Type = Type; }
    void SetAnyHitShaderImport(LPCWSTR importName)
    {
        m_Desc.AnyHitShaderImport = m_Strings[1].LocalCopy(importName, true);
    }
    void SetClosestHitShaderImport(LPCWSTR importName)
    {
        m_Desc.ClosestHitShaderImport = m_Strings[2].LocalCopy(importName, true);
    }
    void SetIntersectionShaderImport(LPCWSTR importName)
    {
        m_Desc.IntersectionShaderImport = m_Strings[3].LocalCopy(importName, true);
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
    }
    operator const D3D12_STATE_SUBOBJECT&() const noexcept { return *m_pSubobject; }
    operator const D3D12_HIT_GROUP_DESC&() const noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        for (UINT i = 0; i < m_NumStrings; i++)
        {
            m_Strings[i].clear();
        }
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_HIT_GROUP_DESC m_Desc;
    static constexpr UINT m_NumStrings = 4;
    CD3DX12_STATE_OBJECT_DESC::StringContainer
        m_Strings[m_NumStrings]; // one string for every entrypoint name
};

//------------------------------------------------------------------------------------------------
class CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void Config(UINT MaxPayloadSizeInBytes, UINT MaxAttributeSizeInBytes) noexcept
    {
        m_Desc.MaxPayloadSizeInBytes = MaxPayloadSizeInBytes;
        m_Desc.MaxAttributeSizeInBytes = MaxAttributeSizeInBytes;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
    }
    operator const D3D12_STATE_SUBOBJECT&() const noexcept { return *m_pSubobject; }
    operator const D3D12_RAYTRACING_SHADER_CONFIG&() const noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_RAYTRACING_SHADER_CONFIG m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void Config(UINT MaxTraceRecursionDepth) noexcept
    {
        m_Desc.MaxTraceRecursionDepth = MaxTraceRecursionDepth;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
    }
    operator const D3D12_STATE_SUBOBJECT&() const noexcept { return *m_pSubobject; }
    operator const D3D12_RAYTRACING_PIPELINE_CONFIG&() const noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_RAYTRACING_PIPELINE_CONFIG m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_RAYTRACING_PIPELINE_CONFIG1_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void Config(UINT MaxTraceRecursionDepth, D3D12_RAYTRACING_PIPELINE_FLAGS Flags) noexcept
    {
        m_Desc.MaxTraceRecursionDepth = MaxTraceRecursionDepth;
        m_Desc.Flags = Flags;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG1;
    }
    operator const D3D12_STATE_SUBOBJECT&() const noexcept { return *m_pSubobject; }
    operator const D3D12_RAYTRACING_PIPELINE_CONFIG1&() const noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_RAYTRACING_PIPELINE_CONFIG1 m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetRootSignature(ID3D12RootSignature* pRootSig) noexcept
    {
        m_pRootSig = pRootSig;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    }
    operator const D3D12_STATE_SUBOBJECT&() const noexcept { return *m_pSubobject; }
    operator ID3D12RootSignature*() const noexcept { return D3DX12_COM_PTR_GET(m_pRootSig); }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_pRootSig = nullptr;
    }
    void* Data() noexcept override { return D3DX12_COM_PTR_ADDRESSOF(m_pRootSig); }
    D3DX12_COM_PTR<ID3D12RootSignature> m_pRootSig;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetRootSignature(ID3D12RootSignature* pRootSig) noexcept
    {
        m_pRootSig = pRootSig;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
    }
    operator const D3D12_STATE_SUBOBJECT&() const noexcept { return *m_pSubobject; }
    operator ID3D12RootSignature*() const noexcept { return D3DX12_COM_PTR_GET(m_pRootSig); }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_pRootSig = nullptr;
    }
    void* Data() noexcept override { return D3DX12_COM_PTR_ADDRESSOF(m_pRootSig); }
    D3DX12_COM_PTR<ID3D12RootSignature> m_pRootSig;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetFlags(D3D12_STATE_OBJECT_FLAGS Flags) noexcept
    {
        m_Desc.Flags = Flags;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_STATE_OBJECT_CONFIG;
    }
    operator const D3D12_STATE_SUBOBJECT&() const noexcept { return *m_pSubobject; }
    operator const D3D12_STATE_OBJECT_CONFIG&() const noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_STATE_OBJECT_CONFIG m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_NODE_MASK_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_NODE_MASK_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_NODE_MASK_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetNodeMask(UINT NodeMask) noexcept
    {
        m_Desc.NodeMask = NodeMask;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK;
    }
    operator const D3D12_STATE_SUBOBJECT&() const noexcept { return *m_pSubobject; }
    operator const D3D12_NODE_MASK&() const noexcept { return m_Desc; }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_NODE_MASK m_Desc;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_NODE_OUTPUT_OVERRIDES
{
public:
    CD3DX12_NODE_OUTPUT_OVERRIDES(const D3D12_NODE_OUTPUT_OVERRIDES** ppOwner, UINT* pNumOutputOverrides) noexcept
    {
        m_Desc.clear();
        m_ppOwner = ppOwner;
        *m_ppOwner = nullptr;
        m_pNumOutputOverrides = pNumOutputOverrides;
        *m_pNumOutputOverrides = 0;
    }
    void NewOutputOverride()
    {
        m_Desc.emplace_back(D3D12_NODE_OUTPUT_OVERRIDES{});
        *m_ppOwner = m_Desc.data();
        (*m_pNumOutputOverrides)++;
    }
    void OutputIndex(UINT index)
    {
        m_Desc.back().OutputIndex = index;
    }
    void NewName(LPCWSTR Name, UINT ArrayIndex = 0)
    {
        m_NodeIDs.emplace_front(D3D12_NODE_ID{ m_Strings.LocalCopy(Name),ArrayIndex });
        m_Desc.back().pNewName = &m_NodeIDs.front();
    }
    void AllowSparseNodes(BOOL bAllow)
    {
        m_UINTs.emplace_front((UINT)bAllow);
        m_Desc.back().pAllowSparseNodes = (BOOL*)&m_UINTs.front();
    }
    void MaxOutputRecords(UINT maxOutputRecords) noexcept
    {
        m_UINTs.emplace_front(maxOutputRecords);
        m_Desc.back().pMaxRecords = &m_UINTs.front();
    }
    void MaxOutputRecordsSharedWith(UINT outputIndex) noexcept
    {
        m_UINTs.emplace_front(outputIndex);
        m_Desc.back().pMaxRecordsSharedWithOutputIndex = &m_UINTs.front();
    }
private:
    std::vector<D3D12_NODE_OUTPUT_OVERRIDES> m_Desc;
    // Cached parameters
    CD3DX12_STATE_OBJECT_DESC::StringContainer m_Strings;
    std::forward_list<UINT> m_UINTs;
    std::forward_list<D3D12_NODE_ID> m_NodeIDs;
    const D3D12_NODE_OUTPUT_OVERRIDES** m_ppOwner;
    UINT* m_pNumOutputOverrides;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_NODE_HELPER_BASE
{
public:
    virtual ~CD3DX12_NODE_HELPER_BASE() = default;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_SHADER_NODE // Not specifying launch mode.
                          // Don't need to distinguish if no parameter overriding is happening
    : public CD3DX12_NODE_HELPER_BASE
{
public:
    CD3DX12_SHADER_NODE(
        D3D12_NODE* pNode,
        LPCWSTR _Shader = nullptr)
    {
        m_pDesc = pNode;
        m_pDesc->NodeType = D3D12_NODE_TYPE_SHADER;
        Shader(_Shader);
    }
    void Shader(LPCWSTR _Shader)
    {
        m_pDesc->Shader.Shader = m_Strings.LocalCopy(_Shader);
    }
    D3D12_NODE* m_pDesc;
private:
    CD3DX12_STATE_OBJECT_DESC::StringContainer m_Strings;
};

//------------------------------------------------------------------------------------------------
// Use this class when defining a broadcasting launch node where configuration parameters
// need to be overridden.  If overrides are not needed, just use CD3DX12_COMPUTE_SHADER_NODE
class CD3DX12_BROADCASTING_LAUNCH_NODE_OVERRIDES
    : public CD3DX12_NODE_HELPER_BASE
{
public:
    CD3DX12_BROADCASTING_LAUNCH_NODE_OVERRIDES(
        D3D12_NODE* pNode,
        LPCWSTR _Shader = nullptr) :
        m_NodeOutputOverrides(&Overrides.pOutputOverrides, &Overrides.NumOutputOverrides)
    {
        Overrides = {};
        m_pDesc = pNode;
        m_pDesc->NodeType = D3D12_NODE_TYPE_SHADER;
        m_pDesc->Shader.OverridesType = D3D12_NODE_OVERRIDES_TYPE_BROADCASTING_LAUNCH;
        m_pDesc->Shader.pBroadcastingLaunchOverrides = &Overrides;
        Shader(_Shader);
    }
    void Shader(LPCWSTR _Shader)
    {
        m_pDesc->Shader.Shader = m_Strings.LocalCopy(_Shader);
    }
    void LocalRootArgumentsTableIndex(UINT index)
    {
        m_UINTs.emplace_front(index);
        Overrides.pLocalRootArgumentsTableIndex = &m_UINTs.front();
    }
    void ProgramEntry(BOOL bIsProgramEntry)
    {
        m_UINTs.emplace_front(bIsProgramEntry);
        Overrides.pProgramEntry = (BOOL*)&m_UINTs.front();
    }
    void NewName(D3D12_NODE_ID NodeID)
    {
        m_NodeIDs.emplace_front(D3D12_NODE_ID{ m_Strings.LocalCopy(NodeID.Name),NodeID.ArrayIndex });
        Overrides.pNewName = &m_NodeIDs.front();
    }
    void ShareInputOf(D3D12_NODE_ID NodeID)
    {
        m_NodeIDs.emplace_front(D3D12_NODE_ID{ m_Strings.LocalCopy(NodeID.Name),NodeID.ArrayIndex });
        Overrides.pShareInputOf = &m_NodeIDs.front();
    }
    void DispatchGrid(UINT x, UINT y, UINT z)
    {
        m_UINT3s.emplace_front(UINT3{ x,y,z });
        Overrides.pDispatchGrid = (UINT*)&m_UINT3s.front();
    }
    void MaxDispatchGrid(UINT x, UINT y, UINT z)
    {
        m_UINT3s.emplace_front(UINT3{x,y,z});
        Overrides.pMaxDispatchGrid = (UINT*)&m_UINT3s.front();
    }
    CD3DX12_NODE_OUTPUT_OVERRIDES& NodeOutputOverrides()
    {
        return m_NodeOutputOverrides;
    }
    D3D12_BROADCASTING_LAUNCH_OVERRIDES Overrides;
    D3D12_NODE* m_pDesc;
private:
    // Cached parameters
    CD3DX12_STATE_OBJECT_DESC::StringContainer m_Strings;
    std::forward_list<UINT> m_UINTs;
    struct UINT3
    {
        UINT x;
        UINT y;
        UINT z;
    };
    std::forward_list<UINT3> m_UINT3s;
    std::forward_list<D3D12_NODE_ID> m_NodeIDs;
    CD3DX12_NODE_OUTPUT_OVERRIDES m_NodeOutputOverrides;
};

//------------------------------------------------------------------------------------------------
// Use this class when defining a coalescing launch node where configuration parameters
// need to be overridden.  If overrides are not needed, just use CD3DX12_COMPUTE_SHADER_NODE
class CD3DX12_COALESCING_LAUNCH_NODE_OVERRIDES
    : public CD3DX12_NODE_HELPER_BASE
{
public:
    CD3DX12_COALESCING_LAUNCH_NODE_OVERRIDES(
        D3D12_NODE* pNode,
        LPCWSTR _Shader = nullptr) :
        m_NodeOutputOverrides(&Overrides.pOutputOverrides, &Overrides.NumOutputOverrides)
    {
        Overrides = {};
        m_pDesc = pNode;
        m_pDesc->NodeType = D3D12_NODE_TYPE_SHADER;
        m_pDesc->Shader.OverridesType = D3D12_NODE_OVERRIDES_TYPE_COALESCING_LAUNCH;
        m_pDesc->Shader.pCoalescingLaunchOverrides = &Overrides;
        Shader(_Shader);
    }
    void Shader(LPCWSTR _Shader)
    {
        m_pDesc->Shader.Shader = m_Strings.LocalCopy(_Shader);
    }
    void LocalRootArgumentsTableIndex(UINT index)
    {
        m_UINTs.emplace_front(index);
        Overrides.pLocalRootArgumentsTableIndex = &m_UINTs.front();
    }
    void ProgramEntry(BOOL bIsProgramEntry)
    {
        m_UINTs.emplace_front(bIsProgramEntry);
        Overrides.pProgramEntry = (BOOL*)&m_UINTs.front();
    }
    void NewName(D3D12_NODE_ID NodeID)
    {
        m_NodeIDs.emplace_front(D3D12_NODE_ID{ m_Strings.LocalCopy(NodeID.Name),NodeID.ArrayIndex });
        Overrides.pNewName = &m_NodeIDs.front();
    }
    void ShareInputOf(D3D12_NODE_ID NodeID)
    {
        m_NodeIDs.emplace_front(D3D12_NODE_ID{ m_Strings.LocalCopy(NodeID.Name),NodeID.ArrayIndex });
        Overrides.pShareInputOf = &m_NodeIDs.front();
    }
    CD3DX12_NODE_OUTPUT_OVERRIDES& NodeOutputOverrides()
    {
        return m_NodeOutputOverrides;
    }
    D3D12_COALESCING_LAUNCH_OVERRIDES Overrides;
    D3D12_NODE* m_pDesc;
private:
    // Cached parameters
    CD3DX12_STATE_OBJECT_DESC::StringContainer m_Strings;
    std::forward_list<UINT> m_UINTs;
    struct UINT3
    {
        UINT x;
        UINT y;
        UINT z;
    };
    std::forward_list<UINT3> m_UINT3s;
    std::forward_list<D3D12_NODE_ID> m_NodeIDs;
    CD3DX12_NODE_OUTPUT_OVERRIDES m_NodeOutputOverrides;
};

//------------------------------------------------------------------------------------------------
// Use this class when defining a thread launch node where configuration parameters
// need to be overridden.  If overrides are not needed, just use CD3DX12_COMPUTE_SHADER_NODE
class CD3DX12_THREAD_LAUNCH_NODE_OVERRIDES
    : public CD3DX12_NODE_HELPER_BASE
{
public:
    CD3DX12_THREAD_LAUNCH_NODE_OVERRIDES(
        D3D12_NODE* pNode,
        LPCWSTR _Shader = nullptr) :
        m_NodeOutputOverrides(&Overrides.pOutputOverrides, &Overrides.NumOutputOverrides)
    {
        Overrides = {};
        m_pDesc = pNode;
        m_pDesc->NodeType = D3D12_NODE_TYPE_SHADER;
        m_pDesc->Shader.OverridesType = D3D12_NODE_OVERRIDES_TYPE_THREAD_LAUNCH;
        m_pDesc->Shader.pThreadLaunchOverrides = &Overrides;
        Shader(_Shader);
    }
    void Shader(LPCWSTR _Shader)
    {
        m_pDesc->Shader.Shader = m_Strings.LocalCopy(_Shader);
    }
    void LocalRootArgumentsTableIndex(UINT index)
    {
        m_UINTs.emplace_front(index);
        Overrides.pLocalRootArgumentsTableIndex = &m_UINTs.front();
    }
    void ProgramEntry(BOOL bIsProgramEntry)
    {
        m_UINTs.emplace_front(bIsProgramEntry);
        Overrides.pProgramEntry = (BOOL*)&m_UINTs.front();
    }
    void NewName(D3D12_NODE_ID NodeID)
    {
        m_NodeIDs.emplace_front(D3D12_NODE_ID{ m_Strings.LocalCopy(NodeID.Name),NodeID.ArrayIndex });
        Overrides.pNewName = &m_NodeIDs.front();
    }
    void ShareInputOf(D3D12_NODE_ID NodeID)
    {
        m_NodeIDs.emplace_front(D3D12_NODE_ID{ m_Strings.LocalCopy(NodeID.Name),NodeID.ArrayIndex });
        Overrides.pShareInputOf = &m_NodeIDs.front();
    }
    CD3DX12_NODE_OUTPUT_OVERRIDES& NodeOutputOverrides()
    {
        return m_NodeOutputOverrides;
    }
    D3D12_THREAD_LAUNCH_OVERRIDES Overrides;
    D3D12_NODE* m_pDesc;
private:
    // Cached parameters
    CD3DX12_STATE_OBJECT_DESC::StringContainer m_Strings;
    std::forward_list<UINT> m_UINTs;
    std::forward_list<D3D12_NODE_ID> m_NodeIDs;
    CD3DX12_NODE_OUTPUT_OVERRIDES m_NodeOutputOverrides;
};

//------------------------------------------------------------------------------------------------
class CD3DX12_WORK_GRAPH_SUBOBJECT
    : public CD3DX12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3DX12_WORK_GRAPH_SUBOBJECT() noexcept
    {
        Init();
    }
    CD3DX12_WORK_GRAPH_SUBOBJECT(CD3DX12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() const noexcept override
    {
        return D3D12_STATE_SUBOBJECT_TYPE_WORK_GRAPH;
    }
    void IncludeAllAvailableNodes()
    {
        m_Desc.Flags |= D3D12_WORK_GRAPH_FLAG_INCLUDE_ALL_AVAILABLE_NODES;
    }
    void SetProgramName(LPCWSTR ProgramName)
    {
        m_Desc.ProgramName = m_Strings.LocalCopy(ProgramName);
    }
    void AddEntrypoint(D3D12_NODE_ID Entrypoint)
    {
        m_Entrypoints.emplace_back(D3D12_NODE_ID{ m_Strings.LocalCopy(Entrypoint.Name),Entrypoint.ArrayIndex });
        m_NumEntrypoints++;
    }

    template<typename T>
    T* CreateNode()
    {
        m_NodeDescs.push_back({});
        m_NumNodes++;
        T* pNodeHelper = new T(&m_NodeDescs.back());
        m_OwnedNodeHelpers.emplace_back(pNodeHelper);
        return pNodeHelper;
    }
    CD3DX12_SHADER_NODE* CreateShaderNode(LPCWSTR Shader = nullptr)
    {
        auto pNode = CreateNode<CD3DX12_SHADER_NODE>();
        pNode->Shader(Shader);
        return pNode;
    }
    CD3DX12_BROADCASTING_LAUNCH_NODE_OVERRIDES* CreateBroadcastingLaunchNodeOverrides(LPCWSTR Shader = nullptr)
    {
        auto pNode = CreateNode<CD3DX12_BROADCASTING_LAUNCH_NODE_OVERRIDES>();
        pNode->Shader(Shader);
        return pNode;
    }
    CD3DX12_COALESCING_LAUNCH_NODE_OVERRIDES* CreateCoalescingLaunchNodeOverrides(LPCWSTR Shader = nullptr)
    {
        auto pNode = CreateNode<CD3DX12_COALESCING_LAUNCH_NODE_OVERRIDES>();
        pNode->Shader(Shader);
        return pNode;
    }
    CD3DX12_THREAD_LAUNCH_NODE_OVERRIDES* CreateThreadLaunchNodeOverrides(LPCWSTR Shader = nullptr)
    {
        auto pNode = CreateNode<CD3DX12_THREAD_LAUNCH_NODE_OVERRIDES>();
        pNode->Shader(Shader);
        return pNode;
    }
    operator const D3D12_STATE_SUBOBJECT& () noexcept
    {
        return *m_pSubobject;
    }
    operator const D3D12_WORK_GRAPH_DESC& () noexcept
    {
        return m_Desc;
    }
    virtual void Finalize()
    {
        m_EntrypointsVector.resize(m_NumEntrypoints);
        std::list<D3D12_NODE_ID>::iterator entryIt = m_Entrypoints.begin();
        for (UINT n = 0; n < m_NumEntrypoints; n++, entryIt++)
        {
            m_EntrypointsVector[n] = *entryIt;
        }
        m_Desc.NumEntrypoints = m_NumEntrypoints;
        m_Desc.pEntrypoints = m_EntrypointsVector.data();

        m_NodeDescsVector.resize(m_NumNodes);
        std::list<D3D12_NODE>::iterator nodeIt = m_NodeDescs.begin();
        for (UINT n = 0; n < m_NumNodes; n++, nodeIt++)
        {
            m_NodeDescsVector[n] = *nodeIt;
        }
        m_Desc.NumExplicitlyDefinedNodes = m_NumNodes;
        m_Desc.pExplicitlyDefinedNodes = m_NodeDescsVector.data();
    }
private:
    void Init() noexcept
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        m_NodeDescs.clear();
        m_NodeDescsVector.clear();
        m_NumNodes = 0;
        m_NumEntrypoints = 0;
    }
    void* Data() noexcept override { return &m_Desc; }
    D3D12_WORK_GRAPH_DESC m_Desc;
    std::list<D3D12_NODE_ID> m_Entrypoints;
    UINT m_NumEntrypoints;
    std::vector<D3D12_NODE_ID> m_EntrypointsVector;
    std::list<D3D12_NODE> m_NodeDescs;
    UINT m_NumNodes;
    std::vector<D3D12_NODE> m_NodeDescsVector;
    CD3DX12_STATE_OBJECT_DESC::StringContainer m_Strings;
    std::list<std::unique_ptr<const CD3DX12_NODE_HELPER_BASE>> m_OwnedNodeHelpers;
};

#undef D3DX12_COM_PTR
#undef D3DX12_COM_PTR_GET
#undef D3DX12_COM_PTR_ADDRESSOF
