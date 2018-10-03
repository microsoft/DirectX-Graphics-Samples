//=============================================================================================================================
// D3D12 Raytracing State Object Creation Helpers
// 
// Helper classes for creating new style state objects out of an arbitrary set of subobjects.
// Uses STL
//
// Start by instantiating CD3D12_STATE_OBJECT_DESC (see it's public methods).
// One of its methods is CreateSubobject(), which has a comment showing a couple of options for defining
// subobjects using the helper classes for each subobject (CD3D12_DXIL_LIBRARY_SUBOBJECT etc.).
// The subobject helpers each have methods specific to the subobject for configuring it's contents.
//=============================================================================================================================
#pragma once
#include <list>
#include <vector>
#include <atlbase.h>

class CD3D12_STATE_OBJECT_DESC
{
public:
    CD3D12_STATE_OBJECT_DESC()
    {
        Init(D3D12_STATE_OBJECT_TYPE_COLLECTION);
    }
    CD3D12_STATE_OBJECT_DESC(D3D12_STATE_OBJECT_TYPE Type)
    {
        Init(Type);
    }
    void SetStateObjectType(D3D12_STATE_OBJECT_TYPE Type) { m_Desc.Type = Type; }
    operator const D3D12_STATE_OBJECT_DESC&()
    {
        m_RepointedAssociations.clear();
        m_SubobjectArray.clear();
        m_SubobjectArray.reserve(m_Desc.NumSubobjects);
        // Flatten subobjects into an array (each flattened subobject still has a member that's a pointer to it's desc that's not flattened)
        for (std::list<SUBOBJECT_WRAPPER>::iterator Iter = m_SubobjectList.begin();
            Iter != m_SubobjectList.end(); Iter++)
        {
            m_SubobjectArray.push_back(*Iter);
            Iter->pSubobjectArrayLocation = &m_SubobjectArray.back(); // Store new location in array so we can redirect pointers contained in subobjects 
        }
        // For subobjects with pointer fields, create a new copy of those subobject definitions with fixed pointers
        for (UINT i = 0; i < m_Desc.NumSubobjects; i++)
        {
            if (m_SubobjectArray[i].Type == D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION)
            {
                const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION* pOriginalSubobjectAssociation = (const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION*)m_SubobjectArray[i].pDesc;
                D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION Repointed = *pOriginalSubobjectAssociation;
                const SUBOBJECT_WRAPPER* pWrapper = static_cast<const SUBOBJECT_WRAPPER*>(pOriginalSubobjectAssociation->pSubobjectToAssociate);
                Repointed.pSubobjectToAssociate = pWrapper->pSubobjectArrayLocation;
                m_RepointedAssociations.push_back(Repointed);
                m_SubobjectArray[i].pDesc = &m_RepointedAssociations.back();
            }
        }
        m_Desc.pSubobjects = m_SubobjectArray.data();
        return m_Desc;
    }
    operator const D3D12_STATE_OBJECT_DESC*()
    {
        return  &(const D3D12_STATE_OBJECT_DESC&)(*this);
    }

    // CreateSubobject creates a sububject helper (e.g. CD3D12_HIT_GROUP_SUBOBJECT) whose lifetime is owned by this class.
    // e.g. 
    // 
    //    CD3D12_STATE_OBJECT_DESC Collection1(D3D12_STATE_OBJECT_TYPE_COLLECTION);
    //    auto Lib0 = Collection1.CreateSubobject<CD3D12_DXIL_LIBRARY_SUBOBJECT>();
    //    Lib0->SetDXILLibrary(&pMyAppDxilLibs[0]);
    //    Lib0->DefineExport(L"rayGenShader0"); // in practice these export listings might be data/engine driven
    //    etc.
    //
    // Alternatively, users can instantiate sububject helpers explicitly, such as via local variables instead, 
    // passing the state object desc that should point to it into the helper constructor (or call mySubobjectHelper.AddToStateObject(Collection1)).  
    // In this alternative scenario, the user must keep the subobject alive as long as the state object it is associated with
    // is alive, else it's pointer references will be stale.
    // e.g.
    //
    //    CD3D12_STATE_OBJECT_DESC RaytracingState2(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);
    //    CD3D12_DXIL_LIBRARY_SUBOBJECT LibA(RaytracingState2);
    //    LibA.SetDXILLibrary(&pMyAppDxilLibs[4]); // not manually specifying exports - meaning all exports in the libraries are exported
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
    void Init(D3D12_STATE_OBJECT_TYPE Type)
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
        D3D12_STATE_SUBOBJECT* pSubobjectArrayLocation; // new location when flattened into array for repointing pointers in subobjects
    } SUBOBJECT_WRAPPER;
    D3D12_STATE_OBJECT_DESC m_Desc;
    std::list<SUBOBJECT_WRAPPER>   m_SubobjectList; // Pointers to list nodes handed out so 
                                                    // these can be edited live
    std::vector<D3D12_STATE_SUBOBJECT> m_SubobjectArray; // Built at the end, copying list contents
    std::list<D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION> m_RepointedAssociations; // subobject type that contains pointers to other subobjects, repointed to flattened array

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
        void clear() { m_Strings.clear(); }
    private:
        std::list<std::wstring> m_Strings;
    };

    class SUBOBJECT_HELPER_BASE
    {
    public:
        SUBOBJECT_HELPER_BASE() { Init(); };
        virtual ~SUBOBJECT_HELPER_BASE() {};
        virtual D3D12_STATE_SUBOBJECT_TYPE Type() = 0;
        void AddToStateObject(CD3D12_STATE_OBJECT_DESC& ContainingStateObject)
        {
            m_pSubobject = ContainingStateObject.TrackSubobject(Type(), Data());
        }
    protected:
        virtual void* Data() = 0;
        void Init() { m_pSubobject = nullptr; }
        D3D12_STATE_SUBOBJECT* m_pSubobject;
    };

    class OWNED_HELPER
    {
    public:
        OWNED_HELPER(const SUBOBJECT_HELPER_BASE* pHelper) { m_pHelper = pHelper; }
        ~OWNED_HELPER() { delete m_pHelper; }
        const SUBOBJECT_HELPER_BASE* m_pHelper;
    };

    std::list<OWNED_HELPER> m_OwnedSubobjectHelpers;

    friend class CD3D12_DXIL_LIBRARY_SUBOBJECT;
    friend class CD3D12_EXISTING_COLLECTION_SUBOBJECT;
    friend class CD3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT;
    friend class CD3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
    friend class CD3D12_HIT_GROUP_SUBOBJECT;
    friend class CD3D12_RAYTRACING_SHADER_CONFIG_SUBOBJECT;
    friend class CD3D12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT;
    friend class CD3D12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT;
    friend class CD3D12_LOCAL_ROOT_SIGNATURE_SUBOBJECT;
    friend class CD3D12_STATE_OBJECT_CONFIG_SUBOBJECT;
    friend class CD3D12_NODE_MASK_SUBOBJECT;
};

class CD3D12_DXIL_LIBRARY_SUBOBJECT : public CD3D12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3D12_DXIL_LIBRARY_SUBOBJECT()
    {
        Init();
    }
    CD3D12_DXIL_LIBRARY_SUBOBJECT(CD3D12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetDXILLibrary(D3D12_SHADER_BYTECODE*pCode) { static const D3D12_SHADER_BYTECODE Default = {}; m_Desc.DXILLibrary = pCode ? *pCode : Default; }
    void DefineExport(LPCWSTR Name, LPCWSTR ExportToRename = nullptr, D3D12_EXPORT_FLAGS Flags = D3D12_EXPORT_FLAG_NONE)
    {
        D3D12_EXPORT_DESC Export;
        Export.Name = m_Strings.LocalCopy(Name);
        Export.ExportToRename = m_Strings.LocalCopy(ExportToRename);
        Export.Flags = Flags;
        m_Exports.push_back(Export);
        m_Desc.pExports = m_Exports.data();
        m_Desc.NumExports = (UINT)m_Exports.size();
    }
    template<size_t N>
    void DefineExports(LPCWSTR(&Exports)[N])
    {
        for (UINT i = 0; i < N; i++)
        {
            DefineExport(Exports[i]);
        }
    }
    void DefineExports(LPCWSTR* Exports, UINT N)
    {
        for (UINT i = 0; i < N; i++)
        {
            DefineExport(Exports[i]);
        }
    }    
    D3D12_STATE_SUBOBJECT_TYPE Type() { return D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY; }
    operator const D3D12_STATE_SUBOBJECT&() const { return *m_pSubobject; }
    operator const D3D12_DXIL_LIBRARY_DESC&() const { return m_Desc; }
private:
    void Init()
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        m_Strings.clear();
        m_Exports.clear();
    }
    void* Data() { return &m_Desc; }
    D3D12_DXIL_LIBRARY_DESC m_Desc;
    CD3D12_STATE_OBJECT_DESC::StringContainer m_Strings;
    std::vector<D3D12_EXPORT_DESC> m_Exports;
};

class CD3D12_EXISTING_COLLECTION_SUBOBJECT : public CD3D12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3D12_EXISTING_COLLECTION_SUBOBJECT()
    {
        Init();
    }
    CD3D12_EXISTING_COLLECTION_SUBOBJECT(CD3D12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetExistingCollection(ID3D12StateObject*pExistingCollection) { m_Desc.pExistingCollection = pExistingCollection; m_CollectionRef = pExistingCollection; }
    void DefineExport(LPCWSTR Name, LPCWSTR ExportToRename = nullptr, D3D12_EXPORT_FLAGS Flags = D3D12_EXPORT_FLAG_NONE)
    {
        D3D12_EXPORT_DESC Export;
        Export.Name = m_Strings.LocalCopy(Name);
        Export.ExportToRename = m_Strings.LocalCopy(ExportToRename);
        Export.Flags = Flags;
        m_Exports.push_back(Export);
        m_Desc.pExports = m_Exports.data();
        m_Desc.NumExports = (UINT)m_Exports.size();
    }
    template<size_t N>
    void DefineExports(LPCWSTR(&Exports)[N])
    {
        for (UINT i = 0; i < N; i++)
        {
            DefineExport(Exports[i]);
        }
    }
    void DefineExports(LPCWSTR* Exports, UINT N)
    {
        for (UINT i = 0; i < N; i++)
        {
            DefineExport(Exports[i]);
        }
    }    
    D3D12_STATE_SUBOBJECT_TYPE Type() { return D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION; }
    operator const D3D12_STATE_SUBOBJECT&() const { return *m_pSubobject; }
    operator const D3D12_EXISTING_COLLECTION_DESC&() const { return m_Desc; }
private:
    void Init()
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        m_CollectionRef = nullptr;
        m_Strings.clear();
        m_Exports.clear();
    }
    void* Data() { return &m_Desc; }
    D3D12_EXISTING_COLLECTION_DESC m_Desc;
    CComPtr<ID3D12StateObject> m_CollectionRef;
    CD3D12_STATE_OBJECT_DESC::StringContainer m_Strings;
    std::vector<D3D12_EXPORT_DESC> m_Exports;
};

class CD3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT : public CD3D12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT()
    {
        Init();
    }
    CD3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT(CD3D12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetSubobjectToAssociate(const D3D12_STATE_SUBOBJECT& SubobjectToAssociate) { m_Desc.pSubobjectToAssociate = &SubobjectToAssociate; }
    void AddExport(LPCWSTR Export)
    {
        m_Desc.NumExports++;
        m_Exports.push_back(m_Strings.LocalCopy(Export));
        m_Desc.pExports = m_Exports.data();
    }

    template<size_t N>
    void AddExports(LPCWSTR(&Exports)[N])
    {
        for (UINT i = 0; i < N; i++)
        {
            AddExport(Exports[i]);
        }
    }

    void AddExports(LPCWSTR* Exports, UINT N)
    {
        for (UINT i = 0; i < N; i++)
        {
            AddExport(Exports[i]);
        }
    }

    D3D12_STATE_SUBOBJECT_TYPE Type() { return D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION; }
    operator const D3D12_STATE_SUBOBJECT&() const { return *m_pSubobject; }
    operator const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION&() const { return m_Desc; }
private:
    void Init()
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        m_Strings.clear();
        m_Exports.clear();
    }
    void* Data() { return &m_Desc; }
    D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION m_Desc;
    CD3D12_STATE_OBJECT_DESC::StringContainer m_Strings;
    std::vector<LPCWSTR> m_Exports;
};

class CD3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION : public CD3D12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION()
    {
        Init();
    }
    CD3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION(CD3D12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetSubobjectNameToAssociate(LPCWSTR SubobjectToAssociate) { m_Desc.SubobjectToAssociate = m_SubobjectName.LocalCopy(SubobjectToAssociate, true); }
    void AddExport(LPCWSTR Export)
    {
        m_Desc.NumExports++;
        m_Exports.push_back(m_Strings.LocalCopy(Export));
        m_Desc.pExports = m_Exports.data();
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() { return D3D12_STATE_SUBOBJECT_TYPE_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION; }
    operator const D3D12_STATE_SUBOBJECT&() const { return *m_pSubobject; }
    operator const D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION&() const { return m_Desc; }
private:
    void Init()
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        m_Strings.clear();
        m_SubobjectName.clear();
        m_Exports.clear();
    }
    void* Data() { return &m_Desc; }
    D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION m_Desc;
    CD3D12_STATE_OBJECT_DESC::StringContainer m_Strings;
    CD3D12_STATE_OBJECT_DESC::StringContainer m_SubobjectName;
    std::vector<LPCWSTR> m_Exports;
};

class CD3D12_HIT_GROUP_SUBOBJECT : public CD3D12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3D12_HIT_GROUP_SUBOBJECT()
    {
        Init();
    }
    CD3D12_HIT_GROUP_SUBOBJECT(CD3D12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetHitGroupExport(LPCWSTR exportName) { m_Desc.HitGroupExport = m_Strings[0].LocalCopy(exportName, true); }
    void SetHitGroupType(D3D12_HIT_GROUP_TYPE Type) { m_Desc.Type = Type; }
    void SetAnyHitShaderImport(LPCWSTR importName) { m_Desc.AnyHitShaderImport = m_Strings[1].LocalCopy(importName, true); }
    void SetClosestHitShaderImport(LPCWSTR importName) { m_Desc.ClosestHitShaderImport = m_Strings[2].LocalCopy(importName, true); }
    void SetIntersectionShaderImport(LPCWSTR importName) { m_Desc.IntersectionShaderImport = m_Strings[3].LocalCopy(importName, true); }
    D3D12_STATE_SUBOBJECT_TYPE Type() { return D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP; }
    operator const D3D12_STATE_SUBOBJECT&() const { return *m_pSubobject; }
    operator const D3D12_HIT_GROUP_DESC&() const { return m_Desc; }
private:
    void Init()
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
        for (UINT i = 0; i < m_NumStrings; i++)
        {
            m_Strings[i].clear();
        }
    }
    void* Data() { return &m_Desc; }
    D3D12_HIT_GROUP_DESC m_Desc;
    static const UINT m_NumStrings = 4;
    CD3D12_STATE_OBJECT_DESC::StringContainer m_Strings[m_NumStrings]; // one string for every entrypoint name
};

class CD3D12_RAYTRACING_SHADER_CONFIG_SUBOBJECT : public CD3D12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3D12_RAYTRACING_SHADER_CONFIG_SUBOBJECT()
    {
        Init();
    }
    CD3D12_RAYTRACING_SHADER_CONFIG_SUBOBJECT(CD3D12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void Config(UINT MaxPayloadSizeInBytes, UINT MaxAttributeSizeInBytes)
    {
        m_Desc.MaxPayloadSizeInBytes = MaxPayloadSizeInBytes;
        m_Desc.MaxAttributeSizeInBytes = MaxAttributeSizeInBytes;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() { return D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG; }
    operator const D3D12_STATE_SUBOBJECT&() const { return *m_pSubobject; }
    operator const D3D12_RAYTRACING_SHADER_CONFIG&() const { return m_Desc; }
private:
    void Init()
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
    }
    void* Data() { return &m_Desc; }
    D3D12_RAYTRACING_SHADER_CONFIG m_Desc;
};

class CD3D12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT : public CD3D12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3D12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT()
    {
        Init();
    }
    CD3D12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT(CD3D12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void Config(UINT MaxTraceRecursionDepth)
    {
        m_Desc.MaxTraceRecursionDepth = MaxTraceRecursionDepth;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() { return D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG; }
    operator const D3D12_STATE_SUBOBJECT&() const { return *m_pSubobject; }
    operator const D3D12_RAYTRACING_PIPELINE_CONFIG&() const { return m_Desc; }
private:
    void Init()
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
    }
    void* Data() { return &m_Desc; }
    D3D12_RAYTRACING_PIPELINE_CONFIG m_Desc;
};

class CD3D12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT : public CD3D12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3D12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT()
    {
        Init();
    }
    CD3D12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT(CD3D12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetRootSignature(ID3D12RootSignature* pRootSig)
    {
        m_pRootSig = pRootSig;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() { return D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE; }
    operator const D3D12_STATE_SUBOBJECT&() const { return *m_pSubobject; }
    operator ID3D12RootSignature*() const { return m_pRootSig; }
private:
    void Init()
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_pRootSig = nullptr;
    }
    void* Data() { return &m_pRootSig; }
    CComPtr<ID3D12RootSignature> m_pRootSig;
};

class CD3D12_LOCAL_ROOT_SIGNATURE_SUBOBJECT : public CD3D12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3D12_LOCAL_ROOT_SIGNATURE_SUBOBJECT()
    {
        Init();
    }
    CD3D12_LOCAL_ROOT_SIGNATURE_SUBOBJECT(CD3D12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetRootSignature(ID3D12RootSignature* pRootSig)
    {
        m_pRootSig = pRootSig;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() { return D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE; }
    operator const D3D12_STATE_SUBOBJECT&() const { return *m_pSubobject; }
    operator ID3D12RootSignature*() const { return m_pRootSig; }
private:
    void Init()
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_pRootSig = nullptr;
    }
    void* Data() { return &m_pRootSig; }
    CComPtr<ID3D12RootSignature> m_pRootSig;
};

class CD3D12_STATE_OBJECT_CONFIG_SUBOBJECT : public CD3D12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3D12_STATE_OBJECT_CONFIG_SUBOBJECT()
    {
        Init();
    }
    CD3D12_STATE_OBJECT_CONFIG_SUBOBJECT(CD3D12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetFlags(D3D12_STATE_OBJECT_FLAGS Flags)
    {
        m_Desc.Flags = Flags;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() { return D3D12_STATE_SUBOBJECT_TYPE_STATE_OBJECT_CONFIG; }
    operator const D3D12_STATE_SUBOBJECT&() const { return *m_pSubobject; }
    operator const D3D12_STATE_OBJECT_CONFIG&() const { return m_Desc; }
private:
    void Init()
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
    }
    void* Data() { return &m_Desc; }
    D3D12_STATE_OBJECT_CONFIG m_Desc;
};

class CD3D12_NODE_MASK_SUBOBJECT : public CD3D12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3D12_NODE_MASK_SUBOBJECT()
    {
        Init();
    }
    CD3D12_NODE_MASK_SUBOBJECT(CD3D12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetNodeMask(UINT NodeMask)
    {
        m_Desc.NodeMask = NodeMask;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() { return D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK; }
    operator const D3D12_STATE_SUBOBJECT&() const { return *m_pSubobject; }
    operator const D3D12_NODE_MASK&() const { return m_Desc; }
private:
    void Init()
    {
        SUBOBJECT_HELPER_BASE::Init();
        m_Desc = {};
    }
    void* Data() { return &m_Desc; }
    D3D12_NODE_MASK m_Desc;
};
