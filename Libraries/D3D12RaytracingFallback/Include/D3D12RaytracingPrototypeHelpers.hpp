#pragma once
#include <list>
#include <vector>
#include <atlbase.h>
//=============================================================================================================================
// D3D12 Raytracing prototype state object creation helpers
// 
// Prototype Helper classes for creating new style state objects out of an arbitrary set of subobjects.
// Uses STL - clean implmentation could be made if needed.
//
// Start by instantiating CD3D12_STATE_OBJECT_DESC (see it's public methods).
// One of its methods is CreateSubobject(), which has a comment showing a couple of options for defining
// subobjects using the helper classes for each subobject (CD3D12_DXIL_LIBRARY_SUBOBJECT etc.).
// The subobject helpers each have methods specific to the subobject for configuring it's contents.
// 
// At the end of the file is a large comment block showing 3 example state object creations.
//
//=============================================================================================================================
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

    UINT NumSubbojects() { return m_Desc.NumSubobjects; }

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
    friend class CD3D12_ROOT_SIGNATURE_SUBOBJECT;
    friend class CD3D12_LOCAL_ROOT_SIGNATURE_SUBOBJECT;
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
    void SetExistingCollection(ID3D12StateObjectPrototype*pExistingCollection) { m_Desc.pExistingCollection = pExistingCollection; m_CollectionRef = pExistingCollection; }
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
    CComPtr<ID3D12StateObjectPrototype> m_CollectionRef;
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
    void AddExports(LPCWSTR (&Exports)[N])
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
    void SetSubobjectNameAssociate(LPCWSTR SubobjectToAssociate) { m_Desc.SubobjectToAssociate = m_SubobjectName.LocalCopy(SubobjectToAssociate, true); }
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

class CD3D12_ROOT_SIGNATURE_SUBOBJECT : public CD3D12_STATE_OBJECT_DESC::SUBOBJECT_HELPER_BASE
{
public:
    CD3D12_ROOT_SIGNATURE_SUBOBJECT()
    {
        Init();
    }
    CD3D12_ROOT_SIGNATURE_SUBOBJECT(CD3D12_STATE_OBJECT_DESC& ContainingStateObject)
    {
        Init();
        AddToStateObject(ContainingStateObject);
    }
    void SetRootSignature(ID3D12RootSignature* pRootSig)
    {
        m_pRootSig = pRootSig;
    }
    D3D12_STATE_SUBOBJECT_TYPE Type() { return D3D12_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE; }
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

//=============================================================================================================================
// Some dummy examples creating various state objects
//
// At the start is a bunch of init code, then 3 examples of creating state objects.
// As these examples were authored before state object validation has been authored in the D3D runtime,
// take these as examples of using the subobject helpers only.  They may not be creating valid state objects (yet),
// due to missing some configuration or whatnot.  Until validation is in place to help correctly build state objects,
// other samples that actually execute and the raytracing spec are better guides.
//=============================================================================================================================

//InitDeviceAndContext(&m_Ctx);
//CComPtr<ID3D12DeviceRaytracingPrototype> spFPDevice = m_Ctx.spFPDevice;
//CComPtr<ID3D12Device2> spDevice = m_Ctx.spDevice;
//CComPtr<ID3D12GraphicsCommandList1> spCL = m_Ctx.spCL;
//CComPtr<ID3D12CommandListRaytracingPrototype> spFPCL = m_Ctx.spFPCL;
//D3D12_SHADER_BYTECODE pMyAppDxilLibs[8] = {
//    CD3DX12_SHADER_BYTECODE(g_Lib1,sizeof(g_Lib1)),  // TODO, make actual different libraries with the appropriate content
//    CD3DX12_SHADER_BYTECODE(g_Lib1,sizeof(g_Lib1)),
//    CD3DX12_SHADER_BYTECODE(g_Lib1,sizeof(g_Lib1)),
//    CD3DX12_SHADER_BYTECODE(g_Lib1,sizeof(g_Lib1)),
//    CD3DX12_SHADER_BYTECODE(g_Lib1,sizeof(g_Lib1)),
//    CD3DX12_SHADER_BYTECODE(g_Lib1,sizeof(g_Lib1)),
//    CD3DX12_SHADER_BYTECODE(g_Lib1,sizeof(g_Lib1)),
//    CD3DX12_SHADER_BYTECODE(g_Lib1,sizeof(g_Lib1))
//};
//CComPtr<ID3D12RootSignature> pMyAppRootSigs[4] = {}; // in practice these would not be null

//for (UINT i = 0; i < _countof(pMyAppRootSigs); i++)
//{
//    // Make some dummy root signatures
//    CD3DX12_DESCRIPTOR_RANGE dr[4];
//    dr[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
//    dr[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
//    dr[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
//    dr[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
//    CD3DX12_ROOT_PARAMETER rp[5];
//    rp[0].InitAsDescriptorTable(3, dr);
//    rp[1].InitAsDescriptorTable(1, &dr[3]);
//    rp[2].InitAsConstantBufferView(2);
//    rp[3].InitAsUnorderedAccessView(2);
//    rp[4].InitAsShaderResourceView(2);
//    CD3DX12_ROOT_SIGNATURE_DESC rs;
//    rs.Init(5 - i, rp); // make each rs a bit different just for fun
//    CComPtr<ID3DBlob> pRSBlob;
//    VERIFY_SUCCEEDED(D3D12SerializeRootSignature(&rs, D3D_ROOT_SIGNATURE_VERSION_1, &pRSBlob, NULL));

//    CComPtr<ID3D12RootSignature> pRS;
//    VERIFY_SUCCEEDED(m_Ctx.spDevice->CreateRootSignature(GetFullNodeMask(m_Ctx.spDevice), pRSBlob->GetBufferPointer(), pRSBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pMyAppRootSigs[i]));
//}

//CComPtr<ID3D12StateObjectPrototype> spRaytracingPipelineState;
//CComPtr<ID3D12StateObjectPrototype> spRaytracingPipelineState2;

////=============================================================================================================================
//// Example 1:
////
////  Collection 1:
////   3 DXIL libraries, exporting various raytracing shaders
////   5 hit shader definitions built from exports in the DXIL libraries
////   2 raytracing shader configurations
////   3 root signatures
////
////  Collection 2:
////   2 DXIL libraries, exporting various raytracing shaders 
////                (one of the libs is same as previous collection, just using different exports)
////   4 hit shader definitions built from exports in the DXIL libraries
////   1 raytracing shader configurations
////   1 root signature
////
////  Raytracing PSO:
////   2 collections
////   1 raytracing configuration
////
////=============================================================================================================================
//{
//    //=========================================================================================================================
//    // Collection1
//    //=========================================================================================================================
//    CD3D12_STATE_OBJECT_DESC Collection1(D3D12_STATE_OBJECT_TYPE_COLLECTION);

//    auto Lib0 = Collection1.CreateSubobject<CD3D12_DXIL_LIBRARY_SUBOBJECT>();
//    Lib0->SetDXILLibrary(&pMyAppDxilLibs[0]);
//    Lib0->DefineExport(L"rayGenShader0"); // in practice these export listings might be data/engine driven
//    Lib0->DefineExport(L"rayGenShader1");
//    Lib0->DefineExport(L"rayGenShader2", L"rayGenShader99"); // rename rayGenShader99 in the lib to rayGenShader2
//    Lib0->DefineExport(L"anyHitShader0");
//    Lib0->DefineExport(L"anyHitShader1");
//    Lib0->DefineExport(L"anyHitShader2");
//    Lib0->DefineExport(L"callableShader0");
//    Lib0->DefineExport(L"intersectionShader0");
//    Lib0->DefineExport(L"intersectionShader1");
//    Lib0->DefineExport(L"closestHitShader0");
//    Lib0->DefineExport(L"intersectionShader2");
//    Lib0->DefineExport(L"closestHitShader1");
//    Lib0->DefineExport(L"callableShader1");
//    Lib0->DefineExport(L"callableShader2");
//    Lib0->DefineExport(L"closestHitShader2");

//    auto Lib1 = Collection1.CreateSubobject<CD3D12_DXIL_LIBRARY_SUBOBJECT>();
//    Lib1->SetDXILLibrary(&pMyAppDxilLibs[1]);
//    Lib1->DefineExport(L"rayGenShader3");
//    Lib1->DefineExport(L"anyHitShader3");
//    Lib1->DefineExport(L"closestHitShader3");
//    Lib1->DefineExport(L"intersectionShader3");
//    Lib1->DefineExport(L"hitGroup0"); // suppose this is defined in DXIL as {"anyHitShader4","closestHitShader1"}
//    Lib1->DefineExport(L"missShader0");

//    auto Lib2 = Collection1.CreateSubobject<CD3D12_DXIL_LIBRARY_SUBOBJECT>();
//    Lib2->SetDXILLibrary(&pMyAppDxilLibs[2]);
//    // Omitting export list for Lib2, which means export all the exports in the DXIL library.
//    // Suppose they are:
//    // anyHitShader4
//    // closestHitShader4
//    // intersectionShader4;
//    // hitGroup1 = {"anyHitShader0","closestHitShader4","intersectionShader2"}
//    // hitGroup2 = {"closestHitShader2"}
//    // hitGroup3 = {"closestHitShader3"}

//    auto LocalHitGroup = Collection1.CreateSubobject<CD3D12_HIT_GROUP_SUBOBJECT>();
//    LocalHitGroup->SetHitGroupExport(L"hitGroup4");
//    LocalHitGroup->SetAnyHitShaderImport(L"anyHitShader4");
//    LocalHitGroup->SetClosestHitShaderImport(L"closestHitShader2");

//    CD3D12_RAYTRACING_SHADER_CONFIG_SUBOBJECT* ShaderConfig[2];
//    for (UINT i = 0; i < _countof(ShaderConfig); i++) { ShaderConfig[i] = Collection1.CreateSubobject<CD3D12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>(); }
//    ShaderConfig[0]->Config(8, 8);
//    ShaderConfig[1]->Config(2, 2);

//    CD3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT* ShaderConfigAssociation[2];
//    for (UINT i = 0; i < _countof(ShaderConfigAssociation); i++) { ShaderConfigAssociation[i] = Collection1.CreateSubobject<CD3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>(); }

//    ShaderConfigAssociation[0]->SetSubobjectToAssociate(*ShaderConfig[0]);
//    ShaderConfigAssociation[0]->AddExport(L"rayGenShader0");
//    ShaderConfigAssociation[0]->AddExport(L"rayGenShader1");
//    ShaderConfigAssociation[0]->AddExport(L"hitShader0");
//    ShaderConfigAssociation[0]->AddExport(L"hitShader1");
//    ShaderConfigAssociation[0]->AddExport(L"hitShader2");
//    ShaderConfigAssociation[0]->AddExport(L"callableShader0");
//    ShaderConfigAssociation[0]->AddExport(L"callableShader1");

//    ShaderConfigAssociation[1]->SetSubobjectToAssociate(*ShaderConfig[1]);
//    ShaderConfigAssociation[1]->AddExport(L"rayGenShader2");
//    ShaderConfigAssociation[1]->AddExport(L"rayGenShader3");
//    ShaderConfigAssociation[1]->AddExport(L"hitShader3");
//    ShaderConfigAssociation[1]->AddExport(L"hitShader4");
//    ShaderConfigAssociation[1]->AddExport(L"missShader0");
//    ShaderConfigAssociation[1]->AddExport(L"callableShader2");
//    ShaderConfigAssociation[1]->AddExport(L"callableShader3");
//    ShaderConfigAssociation[1]->AddExport(L"callableShader4");

//    CD3D12_LOCAL_ROOT_SIGNATURE_SUBOBJECT* RootSig[3];
//    for (UINT i = 0; i < _countof(RootSig); i++) { RootSig[i] = Collection1.CreateSubobject<CD3D12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>(); }
//    RootSig[0]->SetRootSignature(pMyAppRootSigs[0]);
//    RootSig[1]->SetRootSignature(pMyAppRootSigs[1]);
//    RootSig[2]->SetRootSignature(pMyAppRootSigs[2]);

//    CD3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT* RootSigAssociation[3];
//    for (UINT i = 0; i < _countof(RootSigAssociation); i++) { RootSigAssociation[i] = Collection1.CreateSubobject<CD3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>(); }

//    RootSigAssociation[0]->SetSubobjectToAssociate(*RootSig[0]);
//    RootSigAssociation[0]->AddExport(L"rayGenShader0");
//    RootSigAssociation[0]->AddExport(L"rayGenShader1");
//    RootSigAssociation[0]->AddExport(L"rayGenShader2");
//    RootSigAssociation[0]->AddExport(L"rayGenShader3");

//    RootSigAssociation[1]->SetSubobjectToAssociate(*RootSig[1]);
//    RootSigAssociation[1]->AddExport(L"hitShader0");
//    RootSigAssociation[1]->AddExport(L"hitShader1");
//    RootSigAssociation[1]->AddExport(L"hitShader2");
//    RootSigAssociation[1]->AddExport(L"hitShader3");
//    RootSigAssociation[1]->AddExport(L"hitShader4");

//    RootSigAssociation[2]->SetSubobjectToAssociate(*RootSig[2]);
//    RootSigAssociation[2]->AddExport(L"callableShader0");
//    RootSigAssociation[2]->AddExport(L"callableShader1");
//    RootSigAssociation[2]->AddExport(L"callableShader2");
//    RootSigAssociation[2]->AddExport(L"missShader0");

//    CComPtr<ID3D12StateObjectPrototype> spCollection1;
//    VERIFY_SUCCEEDED(spFPDevice->CreateStateObject(Collection1, IID_PPV_ARGS(&spCollection1)));

//    //=========================================================================================================================
//    // Collection2
//    //=========================================================================================================================
//    CD3D12_STATE_OBJECT_DESC Collection2(D3D12_STATE_OBJECT_TYPE_COLLECTION);

//    auto Lib3 = Collection2.CreateSubobject<CD3D12_DXIL_LIBRARY_SUBOBJECT>();
//    Lib3->SetDXILLibrary(&pMyAppDxilLibs[0]); // Note, same DXIL library as used in earlier collection, 
//                                              // but we're taking different exports
//    Lib3->DefineExport(L"rayGenShaderA");
//    Lib3->DefineExport(L"rayGenShaderB");
//    Lib3->DefineExport(L"rayGenShaderC", L"rayGenShader99"); // rename rayGenShader99 in the lib to rayGenShaderC 
//                                                             // (was also renamed to rayGenShader2 in earlier lib)
//    Lib3->DefineExport(L"anyHitShaderA");
//    Lib3->DefineExport(L"anyHitShaderB");
//    Lib3->DefineExport(L"anyHitShaderC");
//    Lib3->DefineExport(L"callableShaderA");
//    Lib3->DefineExport(L"intersectionShaderA");
//    Lib3->DefineExport(L"intersectionShaderB");
//    Lib3->DefineExport(L"closestHitShaderA");
//    Lib3->DefineExport(L"intersectionShaderC");
//    Lib3->DefineExport(L"closestHitShaderB");
//    Lib3->DefineExport(L"callableShaderB");
//    Lib3->DefineExport(L"callableShaderC");
//    Lib3->DefineExport(L"closestHitShaderC");

//    auto Lib4 = Collection2.CreateSubobject<CD3D12_DXIL_LIBRARY_SUBOBJECT>();
//    Lib4->SetDXILLibrary(&pMyAppDxilLibs[3]);
//    Lib4->DefineExport(L"rayGenShaderD");
//    Lib4->DefineExport(L"anyHitShaderD");
//    Lib4->DefineExport(L"closestHitShaderD");
//    Lib4->DefineExport(L"intersectionShaderD");
//    Lib4->DefineExport(L"hitGroupA"); // suppose this is defined in DXIL as {"anyHitShaderD","closestHitShaderB"}
//    Lib4->DefineExport(L"missShaderA");

//    auto LocalHitGroupB = Collection2.CreateSubobject<CD3D12_HIT_GROUP_SUBOBJECT>();
//    LocalHitGroupB->SetHitGroupExport(L"hitGroupB");
//    LocalHitGroupB->SetAnyHitShaderImport(L"anyHitShaderA");
//    LocalHitGroupB->SetClosestHitShaderImport(L"closestHitShaderC");
//    LocalHitGroupB->SetIntersectionShaderImport(L"intersectionHitShaderB");

//    auto LocalHitGroupC = Collection2.CreateSubobject<CD3D12_HIT_GROUP_SUBOBJECT>();
//    LocalHitGroupC->SetHitGroupExport(L"hitGroupC");
//    LocalHitGroupC->SetAnyHitShaderImport(L"anyHitShaderA");
//    LocalHitGroupC->SetClosestHitShaderImport(L"closestHitShaderA");
//    LocalHitGroupC->SetIntersectionShaderImport(L"intersectionShaderA");

//    auto LocalHitGroupD = Collection2.CreateSubobject<CD3D12_HIT_GROUP_SUBOBJECT>();
//    LocalHitGroupD->SetHitGroupExport(L"hitGroupD");
//    LocalHitGroupD->SetAnyHitShaderImport(L"anyHitShaderD");

//    auto ShaderConfigA = Collection2.CreateSubobject<CD3D12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
//    ShaderConfigA->Config(8, 8); // not specifying associations, so by default will map to all relevant exports

//    auto RootSigA = Collection2.CreateSubobject<CD3D12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
//    RootSigA->SetRootSignature(pMyAppRootSigs[3]); // not specifying associations, so by default will map to all relevant exports

//    CComPtr<ID3D12StateObjectPrototype> spCollection2;
//    VERIFY_SUCCEEDED(spFPDevice->CreateStateObject(Collection2, IID_PPV_ARGS(&spCollection2)));

//    //=========================================================================================================================
//    // Raytracing Pipeline State
//    //=========================================================================================================================
//    CD3D12_STATE_OBJECT_DESC RaytracingPipelineState(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);

//    auto ExistingLib0 = RaytracingPipelineState.CreateSubobject<CD3D12_EXISTING_COLLECTION_SUBOBJECT>();
//    ExistingLib0->SetExistingCollection(spCollection1);
//    auto ExistingLib1 = RaytracingPipelineState.CreateSubobject<CD3D12_EXISTING_COLLECTION_SUBOBJECT>();
//    ExistingLib1->SetExistingCollection(spCollection2);

//    auto RaytracingConfig = RaytracingPipelineState.CreateSubobject<CD3D12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
//    RaytracingConfig->Config(8); // not specifying associations, so by default will map to all relevant exports

//    VERIFY_SUCCEEDED(spFPDevice->CreateStateObject(RaytracingPipelineState, IID_PPV_ARGS(&spRaytracingPipelineState)));
//}
////=============================================================================================================================
//// Example 2:
////
////  Raytracing PSO:
////   4 DXIL libraries (entrypoints in the dxil already have root signatures and raytracing shader configs where applicable)
////   1 raytracing configuration
////
//// This example uses alternative syntax for declaring subobjects, where the helpers for each subobject are declared as
//// stack variables.
////
////=============================================================================================================================
//{
//    CD3D12_STATE_OBJECT_DESC RaytracingState2(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);

//    CD3D12_DXIL_LIBRARY_SUBOBJECT LibA(RaytracingState2), LibB(RaytracingState2), LibC(RaytracingState2), LibD(RaytracingState2);
//    LibA.SetDXILLibrary(&pMyAppDxilLibs[4]); // not manually specifying exports - meaning all exports in the libraries are exported
//    LibB.SetDXILLibrary(&pMyAppDxilLibs[5]);
//    LibC.SetDXILLibrary(&pMyAppDxilLibs[6]);
//    LibD.SetDXILLibrary(&pMyAppDxilLibs[7]);

//    CD3D12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT RaytracingConfigA(RaytracingState2);
//    RaytracingConfigA.Config(16); // not specifying associations, so by default will map to all relevant exports

//    VERIFY_SUCCEEDED(spFPDevice->CreateStateObject(RaytracingState2, IID_PPV_ARGS(&spRaytracingPipelineState2)));
//}

