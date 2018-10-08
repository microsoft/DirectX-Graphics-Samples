#include "pch.h"
#include "dxc/HLSL/DxilRuntimeReflection.inl"
//==================================================================================================================================
// CStateObjectInfo
//==================================================================================================================================
//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::m_sAssociateableSubobjectData
// Table of static properties of subobject types that are associateable.
// Array can be indexed via enum: ASSOCIATEABLE_SUBOBJECT_NAME.
//----------------------------------------------------------------------------------------------------------------------------------
const CStateObjectInfo::ASSOCIATEABLE_SUBOBJECT_STATIC_DATA CStateObjectInfo::m_sAssociateableSubobjectData[] = 
{
#pragma push_macro("DECLNAME")
#define DECLNAME(x) x, L#x
    //APIName                                                             //MatchRule                                                   //MatchScope                    //bAtMostOneAssociationPerExport
    {DECLNAME(D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG),       MatchRule_RequiredAndMatchingForAllExports,                   MatchScope_FullStateObject,     true},
    {DECLNAME(D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG),     MatchRule_RequiredAndMatchingForAllExports,                   MatchScope_FullStateObject,     true},
    {DECLNAME(D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE),          MatchRule_IfExistsMustMatchOthersThatExistPlusShaderEntry,    MatchScope_CallGraph,           true},
    {DECLNAME(D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE),           MatchRule_IfExistsMustMatchOthersThatExistPlusShaderEntry,    MatchScope_CallGraph,           true},
    {DECLNAME(D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK),                      MatchRule_IfExistsMustExistAndMatchForAllExports,             MatchScope_FullStateObject,     true},
    {DECLNAME(D3D12_STATE_SUBOBJECT_TYPE_STATE_OBJECT_CONFIG),            MatchRule_IfExistsMustExistAndMatchForAllExports,             MatchScope_LocalStateObject,    true}
#pragma pop_macro("DECLNAME")
};

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CWrappedDXILLibrary::Init
//----------------------------------------------------------------------------------------------------------------------------------
bool CStateObjectInfo::CWrappedDXILLibrary::Init(const D3D12_DXIL_LIBRARY_DESC* pLibrary, CDXILLibraryCache* pDXILLibraryCache, PFN_CALLBACK_GET_DXIL_RUNTIME_DATA pfnGetRuntimeData)
{
    UINT* pRD = nullptr;
    UINT RdatSize = 0;
    if(!pLibrary->DXILLibrary.pShaderBytecode || (pLibrary->DXILLibrary.BytecodeLength == 0))
    {
        return false;
    }
    m_pDXILLibraryCache = pDXILLibraryCache;
    m_LocalLibraryDesc.DXILLibrary = pDXILLibraryCache ? m_pDXILLibraryCache->LocalUniqueCopy(&pLibrary->DXILLibrary) 
                                                       : pLibrary->DXILLibrary;
    if (FAILED(pfnGetRuntimeData((void *)m_LocalLibraryDesc.DXILLibrary.pShaderBytecode, (const UINT **)&pRD, &RdatSize)))
    {
        return false;
    }
    m_pReflection.reset(CreateDxilRuntimeReflection());
    bool bSuccess = m_pReflection->InitFromRDAT(pRD, RdatSize);
    if(!bSuccess)
    {
        return bSuccess;
    }
    m_LocalLibraryDesc.NumExports = pLibrary->NumExports;
    m_Exports.resize(pLibrary->NumExports);
    UINT i = 0;
    for(auto& ex : m_Exports)
    {
        ex.Name = CStateObjectInfo::LocalUniqueCopy(pLibrary->pExports[i].Name,m_StringContainer);
        ex.ExportToRename = CStateObjectInfo::LocalUniqueCopy(pLibrary->pExports[i].ExportToRename,m_StringContainer);
        ex.Flags = pLibrary->pExports[i].Flags;
        i++;
    }
    m_LocalLibraryDesc.pExports = m_Exports.data();
    m_LocalSubobjectDefinition.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    m_LocalSubobjectDefinition.pDesc = &m_LocalLibraryDesc;
    return bSuccess;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CWrappedDXILLibrary::~CWrappedDXILLibrary
//----------------------------------------------------------------------------------------------------------------------------------
 CStateObjectInfo::CWrappedDXILLibrary::~CWrappedDXILLibrary()
 {
     if(m_pDXILLibraryCache)
     {
         m_pDXILLibraryCache->Release(m_LocalLibraryDesc.DXILLibrary.pShaderBytecode);
     }
 }

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CWrappedDXILLibrary::GetLibraryReflection
//----------------------------------------------------------------------------------------------------------------------------------
const DxilLibraryDesc CStateObjectInfo::CWrappedDXILLibrary::GetLibraryReflection()
{
    return m_pReflection->GetLibraryReflection();
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CWrappedExistingCollection::Init
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::CWrappedExistingCollection::Init(const D3D12_EXISTING_COLLECTION_DESC* pCollection)
{
    m_LocalCollectionDesc.NumExports = pCollection->NumExports;
    m_Exports.resize(pCollection->NumExports);
    UINT i = 0;
    for(auto& ex : m_Exports)
    {
        ex.Name = CStateObjectInfo::LocalUniqueCopy(pCollection->pExports[i].Name,m_StringContainer);
        ex.ExportToRename = CStateObjectInfo::LocalUniqueCopy(pCollection->pExports[i].ExportToRename,m_StringContainer);
        ex.Flags = pCollection->pExports[i].Flags;
        i++;
    }
    m_LocalCollectionDesc.pExistingCollection = pCollection->pExistingCollection;
    m_LocalCollectionDesc.pExports = m_Exports.data();
    m_LocalSubobjectDefinition.Type = D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION;
    m_LocalSubobjectDefinition.pDesc = &m_LocalCollectionDesc;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::ParseStateObject
//----------------------------------------------------------------------------------------------------------------------------------
HRESULT CStateObjectInfo::ParseStateObject(
    const D3D12_STATE_OBJECT_DESC* pStateObject,
    PFN_CALLBACK_GET_STATE_OBJECT_INFO_FOR_EXISTING_COLLECTION pfnGetStateObjectInfoForExistingCollection,
    PFN_CALLBACK_GET_DXIL_RUNTIME_DATA pfnGetDxilRuntimeData
#ifndef SKIP_BINDING_VALIDATION
        ,PFN_CALLBACK_GET_ROOT_SIGNATURE_DESERIALIZER pfnGetRootSignatureDeserializer
#endif    
    ,CDXILLibraryCache* pDXILLibraryCache
    )
{
    if(m_bStateObjectSet)
    {
        LOG_ERROR(L"State object already set.");
        return E_FAIL;
    }
    m_bStateObjectSet = true;
    if(!pStateObject)
    {
        LOG_ERROR(L"Null state object specified.");
        return E_INVALIDARG;
    }
    m_pfnGetDxilRuntimeData = pfnGetDxilRuntimeData;
    m_pfnGetStateObjectInfoForExistingCollection = pfnGetStateObjectInfoForExistingCollection;
    m_pDXILLibraryCache = pDXILLibraryCache;
#ifndef SKIP_BINDING_VALIDATION
    m_pfnGetRootSignatureDeserializer = pfnGetRootSignatureDeserializer;
#endif
    SetStateObjectType(pStateObject->Type);
    if( pStateObject->NumSubobjects && !pStateObject->pSubobjects )
    {
        LOG_ERROR(L"Null pSubobjects specified.");
        return E_INVALIDARG;
    }
    try
    {
        for(UINT i = 0; i < pStateObject->NumSubobjects; i++)
        {
            auto pEnclosingSubobject = &pStateObject->pSubobjects[i];
            auto pDesc = pEnclosingSubobject->pDesc;
            switch(pStateObject->pSubobjects[i].Type)
            {
            case D3D12_STATE_SUBOBJECT_TYPE_STATE_OBJECT_CONFIG:
                AddStateObjectConfig((const D3D12_STATE_OBJECT_CONFIG*)pDesc,pEnclosingSubobject);
                break;
            case D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE:
                AddGlobalRootSignature((const D3D12_GLOBAL_ROOT_SIGNATURE*)pDesc,pEnclosingSubobject);
                break;
            case D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE:
                AddLocalRootSignature((const D3D12_LOCAL_ROOT_SIGNATURE*)pDesc,pEnclosingSubobject);
                break;
            case D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK: 
                AddNodeMask((const D3D12_NODE_MASK*)pDesc,pEnclosingSubobject);
                break;
            case D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY: 
                AddLibrary((const D3D12_DXIL_LIBRARY_DESC*)pDesc);
                break;
            case D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION: 
                AddCollection((const D3D12_EXISTING_COLLECTION_DESC*)pDesc);
                break;
            case D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
                AddSubobjectToExportsAssociation((const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION*)pDesc);
                break;
            case D3D12_STATE_SUBOBJECT_TYPE_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION: // D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION
                assert(false); // TODO: support this
                break;
            case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG:
                AddRaytracingShaderConfig((const D3D12_RAYTRACING_SHADER_CONFIG*)pDesc,pEnclosingSubobject);
                break;
            case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG:
                AddRaytracingPipelineConfig((const D3D12_RAYTRACING_PIPELINE_CONFIG*)pDesc,pEnclosingSubobject);
                break;
            case D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP:
                if(m_bUsePrototypeInterface)
                {
                    D3D12_HIT_GROUP_DESC TranslatedHitGroup = {}; // delete once prototype interface no longer needed
                    auto pExperimental = (const D3D12_EXPERIMENTAL_HIT_GROUP_DESC*)pDesc;
                    const void* pHitGroupDescToUse = pDesc;
                    if(pExperimental)
                    {
                        TranslatedHitGroup.HitGroupExport = pExperimental->HitGroupExport;
                        TranslatedHitGroup.AnyHitShaderImport = pExperimental->AnyHitShaderImport;
                        TranslatedHitGroup.ClosestHitShaderImport = pExperimental->ClosestHitShaderImport;
                        TranslatedHitGroup.IntersectionShaderImport = pExperimental->IntersectionShaderImport;
                        TranslatedHitGroup.Type = pExperimental->IntersectionShaderImport ? D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE : D3D12_HIT_GROUP_TYPE_TRIANGLES;
                        pHitGroupDescToUse = &TranslatedHitGroup;
                    }                    
                    AddHitGroup((const D3D12_HIT_GROUP_DESC*)pHitGroupDescToUse,this);
                    break;
                }
                AddHitGroup((const D3D12_HIT_GROUP_DESC*)pDesc,this);
                break;
            default:
                LOG_ERROR(L"pSubobjects[" << i << L"] is of unrecognized type: " << pStateObject->pSubobjects[i].Type);
                break;
            }
        }
        FinishedAddingSubobjects();    
    }
    catch(HRESULT& hrEx)
    {
        return hrEx;
    }
    return m_bFoundError ? E_INVALIDARG : S_OK;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::SetStateObjectType
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::SetStateObjectType(D3D12_STATE_OBJECT_TYPE SOType)
{
    switch(SOType)
    {
    case D3D12_STATE_OBJECT_TYPE_COLLECTION:
    case D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE:
        break;
    default:
        LOG_ERROR(L"Invalid state object type specified: " << SOType );
        m_bStateObjectTypeSelected = false;
        throw E_INVALIDARG; // don't even continue
    }
    m_SOType = SOType;
    m_bStateObjectTypeSelected = true;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::FinishedAddingSubobjects
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::FinishedAddingSubobjects()
{
    assert(m_bStateObjectTypeSelected);
    ResolveFunctionDependencies(); 
    TranslateSubobjectAssociations();
    ResolveSubobjectAssociations();
#ifndef SKIP_BINDING_VALIDATION
    ResolveResourceBindings();
#endif
    ValidateMiscAssociations();
    ValidateShaderFeatures();
    PrepareForInformationReflection();
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::RenameMangledName
//----------------------------------------------------------------------------------------------------------------------------------
LPCWSTR CStateObjectInfo::RenameMangledName(LPCWSTR OriginalMangledName, LPCWSTR OriginalUnmangledName, LPCWSTR NewUnmangledName)
{
    size_t originalUnmangledLength = wcslen(OriginalUnmangledName);
    size_t originalMangledLength = wcslen(OriginalMangledName);
    static const unsigned int mangledPrefixSize = 2;
    if ((originalMangledLength == originalUnmangledLength) && (0 == wcscmp(OriginalMangledName,OriginalUnmangledName)))
    {
        // renaming an unmangled name
        return LocalUniqueCopy(NewUnmangledName);
    }
    else if(!((originalMangledLength >= originalUnmangledLength + 4) &&
             (L'\01' == OriginalMangledName[0]) &&
             (L'?' == OriginalMangledName[1]) &&
             (L'@' == OriginalMangledName[mangledPrefixSize+originalUnmangledLength]) &&
             (0 == wcsncmp(&OriginalMangledName[mangledPrefixSize], OriginalUnmangledName,originalUnmangledLength))))
    {
        LOG_ERROR(L"Badly formatted mangled export name in library: " << OriginalMangledName << L".");
        throw E_INVALIDARG; // don't bother trying to continue        
    }
    else if(wcsncmp(OriginalMangledName+mangledPrefixSize,OriginalUnmangledName,originalUnmangledLength))
    {
        LOG_ERROR(L"Badly formatted mangled export name in library: " << OriginalMangledName << L", expected to contain string: " << OriginalUnmangledName);
        throw E_INVALIDARG; // don't bother trying to continue                
    }
    size_t newUnmangledLength = wcslen(NewUnmangledName);
    std::wstring newString = 
        std::wstring(OriginalMangledName, 0, mangledPrefixSize) + 
        NewUnmangledName + 
        std::wstring(OriginalMangledName, mangledPrefixSize + originalUnmangledLength, originalMangledLength - mangledPrefixSize - originalUnmangledLength);
    assert(newString.size() == originalMangledLength - originalUnmangledLength + newUnmangledLength);
    return LocalUniqueCopy(newString.c_str());
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::PrettyPrintPossiblyMangledName
//----------------------------------------------------------------------------------------------------------------------------------
#ifdef INCLUDE_MESSAGE_LOG
LPCWSTR CStateObjectInfo::PrettyPrintPossiblyMangledName(LPCWSTR name)
{
    std::wostringstream snippet;
    auto match = m_ExportNameMangledToUnmangled.find(name);
    size_t count = (match == m_ExportNameMangledToUnmangled.end()) ? 0 : m_ExportNameUnmangledToMangled.count(match->second);
    size_t length = wcslen(name);
    if(1 == count) // no need to disambiguate with mangled name
    {
        snippet << L"\"" << match->second << L"\"";
    }
    else if((length >= 2) &&
        (L'\01' == name[0]) &&
        (L'?' == name[1]))
    {   
        // Cleanup mangled name     
        size_t newLength = length+3; // incl terminating null
        std::wstring newString = L"\\01?" + std::wstring(name + 2);
        auto pNewString = newString.c_str();

        // Find end of mangled portion
        auto end = std::find(newString.begin() + 4, newString.end(), L'@');
        
        // Make unmangled name
        std::wstring newUnmangledString = std::wstring(newString.begin() + 4, end);
        auto pNewUnmangledString = newUnmangledString.c_str();

        // Print string with both unmangled and mangled for clarity
        snippet << L"\"" << pNewUnmangledString << L"\" (mangled name: \"" << pNewString << L"\")";
    }
    else
    {
        snippet << L"\"" << name << L"\"";
    }
    return LocalUniqueCopy(snippet.str().c_str());
}
#endif

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::AddExport
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::AddExport(
    LPCWSTR pExternalNameMangled, 
    LPCWSTR pExternalNameUnmangled, 
    const DxilFunctionDesc* pInfo, 
    CStateObjectInfo* pOwningStateObject,
    bool bExternalDependenciesOnThisExportAllowed)
{
    auto ret = m_ExportInfoMap.find(LocalUniqueCopy(pExternalNameMangled));
    if (ret != m_ExportInfoMap.end())
    {
        LOG_ERROR(L"Export " << PrettyPrintPossiblyMangledName(pExternalNameMangled) << L" already defined.");
        return; // continue, to be able to find other errors
    }
    CExportInfo* pExportInfo = nullptr;
    m_ExportInfoList.emplace_back();
    pExportInfo = &m_ExportInfoList.back();
    pExportInfo->m_pFunctionInfo = pInfo;
    pExportInfo->m_pOwningStateObject = pOwningStateObject;
    pExportInfo->m_bExternalDependenciesOnThisExportAllowed = bExternalDependenciesOnThisExportAllowed;

    LPCWSTR pUniqueExternalNameMangled = LocalUniqueCopy(pExternalNameMangled);
    LPCWSTR pUniqueExternalNameUnmangled = LocalUniqueCopy(pExternalNameUnmangled);
    pExportInfo->m_MangledName = pUniqueExternalNameMangled;
    pExportInfo->m_UnmangledName = pUniqueExternalNameUnmangled;
    m_ExportInfoMap.insert({ pUniqueExternalNameMangled,pExportInfo });
    m_ExportNameUnmangledToMangled.insert({ pUniqueExternalNameUnmangled,pUniqueExternalNameMangled });
    m_ExportNameMangledToUnmangled.insert({ pUniqueExternalNameMangled,pUniqueExternalNameUnmangled });

    UINT NumFunctionDependencies = pExportInfo->m_pFunctionInfo->NumFunctionDependencies;
    for (UINT i = 0; i < NumFunctionDependencies; i++)
    {
        // pExportInfo calls FunctionDependencies[i]
        m_Dependencies.insert({ LocalUniqueCopy(pExportInfo->m_pFunctionInfo->FunctionDependencies[i]), pExportInfo });
    }

    m_UsedUnmangledFunctionNames.insert(pUniqueExternalNameUnmangled); //unmangled name could already be in set (overload), that's ok  
    if(m_UsedNonFunctionNames.find(pUniqueExternalNameUnmangled) != m_UsedNonFunctionNames.end())
    {
        LOG_ERROR(L"Export name " << pUniqueExternalNameUnmangled << L" already used used by another non-function export.");
    }    
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::AddLibrary
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::AddLibrary(const D3D12_DXIL_LIBRARY_DESC* pLibrary)
{
    // Don't keep any references to pLibrary or it's contents -> app memory.
    // Exception is library code pointer, which won't get dereferenced after library is added, but is kept 
    // for the purposes of reflection of the used subobjects back out. 

    m_DXILLibraryList.push_back( std::make_shared<CWrappedDXILLibrary>() );
    CWrappedDXILLibrary* pWrappedLibrary = m_DXILLibraryList.back().get();
    if (!pWrappedLibrary->Init(pLibrary,m_pDXILLibraryCache, m_pfnGetDxilRuntimeData))
    {
        LOG_ERROR(L"Failed to parse DXILibrary.pShaderBytecode: " << pLibrary->DXILLibrary.pShaderBytecode );
        return;
    }
    DxilLibraryDesc libDesc = pWrappedLibrary->GetLibraryReflection();
    D3D12_DXIL_LIBRARY_DESC& LocalLibrary = pWrappedLibrary->m_LocalLibraryDesc;

    // Multimap of internal export names to external export(s) the library desc manually listed (if any)
    std::unordered_multimap<std::wstring, const D3D12_EXPORT_DESC*> ExportsToUse;
    std::unordered_set<const D3D12_EXPORT_DESC*> ExportMissing;
    for (UINT i = 0; i < LocalLibrary.NumExports; i++)
    {
        LPCWSTR InternalName = LocalLibrary.pExports[i].ExportToRename ? LocalLibrary.pExports[i].ExportToRename : LocalLibrary.pExports[i].Name;
        ExportsToUse.insert({ InternalName,&LocalLibrary.pExports[i] });
        ExportMissing.insert(&LocalLibrary.pExports[i]);
    }
    // If there's a manual export list, only add matching exports
    if (ExportsToUse.size())
    {
        for (UINT i = 0; i < libDesc.NumFunctions; i++)
        {
            const DxilFunctionDesc* pFunc = &libDesc.pFunction[i];
            for (UINT j = 0; j < 2; j++) // 0 == unmangled name, 1 == mangled name
            {
                LPCWSTR nameToMatch = LocalUniqueCopy(j == 0 ? pFunc->UnmangledName : pFunc->Name);
                auto matches = ExportsToUse.equal_range(nameToMatch);
                if (matches.first != ExportsToUse.end())
                {
                    for (auto match = matches.first; match != matches.second; match++)
                    {
                        // cases: (1) ExportToRename is an unmangled name, Name is unmangled
                        //        (2) ExportToRename is a mangled name, Name is unmangled
                        //        (3) ExportToRename is null, Name is unmangled
                        //        (4) ExportToRename is null, Name is mangled
                        // AddExport will throw if the same mangled external name is exported multiple times from the state object.

                        if (match->second->ExportToRename)
                        {
                            // (1) or (2) - do a rename
                            if (wcscmp(match->second->ExportToRename, nameToMatch) == 0)
                            {
                                AddExport(RenameMangledName(pFunc->Name, pFunc->UnmangledName, match->second->Name), match->second->Name, pFunc,this,true);
                                ExportMissing.erase(match->second);
                            }
                        }
                        else
                        {
                            // (3) or (4) - no rename
                            if (wcscmp(match->second->Name, nameToMatch) == 0)
                            {
                                AddExport(pFunc->Name, pFunc->UnmangledName, pFunc,this,true);
                                ExportMissing.erase(match->second);
                            }
                        }
                    }
                }
            }
        }
#ifdef INCLUDE_MESSAGE_LOG
        for (auto missingExport : ExportMissing)
        {
            size_t i = (missingExport - LocalLibrary.pExports); // index into export list
            LPCWSTR InternalName = LocalLibrary.pExports[i].ExportToRename ? LocalLibrary.pExports[i].ExportToRename : LocalLibrary.pExports[i].Name;
            LOG_ERROR(L"Manually listed export [" << i << L"], " << PrettyPrintPossiblyMangledName(InternalName) << L", doesn't exist in DXILLibrary.pShaderBytecode: " << pLibrary->DXILLibrary.pShaderBytecode << L".");
        }
#else
        if(ExportMissing.size())
        {
            LOG_ERROR_NOMESSAGE;
        }
#endif

    }
    else // add all exports
    {
        for (UINT i = 0; i < libDesc.NumFunctions; i++)
        {
            const DxilFunctionDesc* pFunc = &libDesc.pFunction[i];
            AddExport(pFunc->Name, pFunc->UnmangledName, pFunc,this,true);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::TraverseFunctionsInitialValidation
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::TraverseFunctionsInitialValidation(LPCWSTR function)
{
    auto ex = m_ExportInfoMap.find(LocalUniqueCopy(function));
    if(ex == m_ExportInfoMap.end())
    {
        return; // ignore unresolved exports
    }
    auto pFuncInfo = ex->second->m_pFunctionInfo;
    auto& flags = ex->second->m_GraphTraversalFlags;      
    if(!(flags & CExportInfo::GTF_CycleFound) && (m_TraversalGlobals.GraphTraversalIndex == ex->second->m_VisitedOnGraphTraversalIndex))
    {
#ifdef INCLUDE_MESSAGE_LOG            
        auto unmangledName = m_ExportNameMangledToUnmangled.find(LocalUniqueCopy(function));
        LOG_ERROR(L"Cycle in function call graph involving export " <<
            PrettyPrintPossiblyMangledName(function) << L".");
#else
        LOG_ERROR_NOMESSAGE;
#endif   
        flags |= CExportInfo::GTF_CycleFound;
        return;             
    }
    if(flags & CExportInfo::GTF_SubtreeAlreadyCheckedForCycles)
    {
        return;
    }
    flags |= CExportInfo::GTF_SubtreeAlreadyCheckedForCycles;
    ex->second->m_VisitedOnGraphTraversalIndex = m_TraversalGlobals.GraphTraversalIndex;
    for(UINT i = 0; i < pFuncInfo->NumFunctionDependencies; i++)
    {
        TraverseFunctionsInitialValidation(pFuncInfo->FunctionDependencies[i]);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::ResolveFunctionDependencies
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::ResolveFunctionDependencies()
{
    // Function dependencies
    for (auto dep : m_Dependencies)
    {
        auto match = m_ExportInfoMap.find(dep.first); // entries in dep are already unique strings, so map lookup is safe
        if (match == m_ExportInfoMap.end())
        {
            if (!AllowLocalDependenciesOnExternalDefinitions())
            {          
                LOG_ERROR(L"Unresolved reference to function " << PrettyPrintPossiblyMangledName(dep.first) <<
                    L" by export " << PrettyPrintPossiblyMangledName(dep.second->m_MangledName) << L"." <<
                    ((D3D12_STATE_OBJECT_TYPE_COLLECTION == m_SOType) ? 
                    L" If the intent is this will be resolved later, when this state object is combined with other state object(s), "
                    L"use a D3D12_STATE_OBJECT_CONFIG subobject with D3D12_STATE_OBJECT_FLAG_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITIONS set in Flags." : L""));
            }
            dep.second->m_bUnresolvedFunctions = true;
            m_bUnresolvedFunctions = true;
        }
        else if(!match->second->m_bExternalDependenciesOnThisExportAllowed && (dep.second->m_pOwningStateObject != match->second->m_pOwningStateObject))
        {
            LOG_ERROR(L"Function " << PrettyPrintPossiblyMangledName(match->second->m_MangledName) <<
                L" comes from a state object that did not opt in to allowing external dependencies on local definitions. Thus, export " << 
                PrettyPrintPossiblyMangledName(dep.second->m_MangledName) << L", which resides in a different state object, can't depend on \"" 
                << match->second->m_UnmangledName <<
                L"\". To allow this linkage across state objects, the state object exporting \"" << match->second->m_UnmangledName << L"\" must specify a D3D12_STATE_OBJECT_CONFIG subobject with the flag " <<
                L"D3D12_STATE_OBJECT_FLAG_ALLOW_EXTERNAL_DEPENDENCIES_ON_LOCAL_DEFINITIONS." );
        }
    }
    // Check for cycles or library functions calling entrypoints
    for(auto& function : m_ExportInfoList)
    {
        function.m_GraphTraversalFlags = 0;
        function.m_VisitedOnGraphTraversalIndex = (UINT64)-1;
    }
    for(auto& ex : m_ExportInfoList)
    {
        TraverseFunctionsInitialValidation(ex.m_MangledName);
        m_TraversalGlobals.GraphTraversalIndex++;
    }

    // Hit group dependencies
    for (auto& hg : m_HitGroups)
    {
        CWrappedHitGroup& hgDesc = *hg.second;
        for (UINT s = 0; s < 3; s++)
        {
            LPCWSTR pDependency = nullptr;
            ShaderKind expectedShader = ShaderKind::Invalid;
            switch (s)
            {
            case 0:
                pDependency = hgDesc.AnyHitShaderImport;
                expectedShader = ShaderKind::AnyHit;
                break;
            case 1:
                pDependency = hgDesc.ClosestHitShaderImport;
                expectedShader = ShaderKind::ClosestHit;
                break;
            case 2:
                pDependency = hgDesc.IntersectionShaderImport;
                expectedShader = ShaderKind::Intersection;
                break;
            default:
                assert(false);
                break;
            }
            if (!pDependency)
            {
                continue;
            }
            // Does unmangled search give hit(s)?  If so there better only be one.
            CExportInfo* pMatch = nullptr;
            size_t count = m_ExportNameUnmangledToMangled.count(pDependency);
            switch (count)
            {
            case 0:
            {
                // Try mangled
                auto match = m_ExportInfoMap.find(pDependency);
                if (match != m_ExportInfoMap.end())
                {
                    pMatch = match->second;
                }
                else
                {
                    if (!AllowLocalDependenciesOnExternalDefinitions())
                    {                             
                        LOG_ERROR(L"HitGroupExport \"" << hgDesc.HitGroupExport <<
                            L"\" imports " << GetHitGroupDependencyTypeName(s) << L" named " << PrettyPrintPossiblyMangledName(pDependency) <<
                            L" but there are no exports matching that name." <<
                            ((D3D12_STATE_OBJECT_TYPE_COLLECTION == m_SOType) ? 
                            L" If the intent is this will be resolved later, when this state object is combined with other state object(s), "
                            L"use a D3D12_STATE_OBJECT_CONFIG subobject with D3D12_STATE_OBJECT_FLAG_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITIONS set in Flags." : L""));                            
                    }
                    m_bUnresolvedHitGroups = true;
                    hgDesc.m_bUnresolvedFunctions = true;
                }
                break;
            }
            case 1:
            {
                auto mangledMatch = m_ExportNameUnmangledToMangled.find(pDependency);
                auto match = m_ExportInfoMap.find(mangledMatch->second);
                pMatch = match->second;
                break;
            }
            default:
                LOG_ERROR(L"HitGroupExport \"" << hgDesc.HitGroupExport <<
                    L"\" imports " << GetHitGroupDependencyTypeName(s) << L" named " << PrettyPrintPossiblyMangledName(pDependency) <<
                    L" but there are " << count << L"overloads matching that name. " <<
                    L"Either make the names unique or use mangled naming (which includes signature information in the name).");
                break;
            }
            if (pMatch)
            {
                if ((ShaderKind)pMatch->m_pFunctionInfo->ShaderKind != expectedShader)
                {
                    LOG_ERROR(L"HitGroupExport \"" << hgDesc.HitGroupExport <<
                        L"\" imports " << GetHitGroupDependencyTypeName(s) << L" named " << PrettyPrintPossiblyMangledName(pDependency) <<
                        L" but the shader with that name is the wrong shader type.");
                }
                else if(!pMatch->m_bExternalDependenciesOnThisExportAllowed && (hgDesc.m_pOwningStateObject != pMatch->m_pOwningStateObject))
                {
                    LOG_ERROR(L"Function " << PrettyPrintPossiblyMangledName(pMatch->m_MangledName) <<
                        L" comes from a state object that did not opt-in to external dependencies on local definitions. Thus, HitGroupExport \"" << hgDesc.HitGroupExport <<
                        L"\" which resides in a different state object, can't depend on \"" 
                        << pMatch->m_UnmangledName <<
                        L"\". To allow this linkage across state objects, the state object exporting \"" << pMatch->m_UnmangledName << L"\" must specify a D3D12_STATE_OBJECT_CONFIG subobject with the flag " <<
                        L"D3D12_STATE_OBJECT_FLAG_ALLOW_EXTERNAL_DEPENDENCIES_ON_LOCAL_DEFINITIONS." );
                }
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::TranslateSubobjectAssociations
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::TranslateSubobjectAssociations()
{
    for(ASSOCIATEABLE_SUBOBJECT_NAME i = (ASSOCIATEABLE_SUBOBJECT_NAME)0; i < NUM_ASSOCIATEABLE_SUBOBJECT_TYPES; ((UINT&)i)++)
    {
        for(auto& a : m_SubobjectToExportsAssociations[i])
        {
            if(a.m_pUntranslatedExternalSubobject)
            {
                auto match = m_EnclosingSubobjectPointerToInternal.find(a.m_pUntranslatedExternalSubobject); 
                if( match != m_EnclosingSubobjectPointerToInternal.end()) // imported subobjects are already translated (e.g. from a collection)
                {
                    a.m_pSubobject = (CAssociateableSubobjectInfo*)match->second;
                }
                a.m_pUntranslatedExternalSubobject = nullptr;
            }
        }
    }
    m_EnclosingSubobjectPointerToInternal.clear(); // don't need this any longer
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::TraverseFunctionsFindFirstSubobjectInLibraryFunctionSubtrees
//----------------------------------------------------------------------------------------------------------------------------------
CStateObjectInfo::CAssociateableSubobjectInfo* CStateObjectInfo::TraverseFunctionsFindFirstSubobjectInLibraryFunctionSubtrees(
    LPCWSTR function)
{
    auto ex = m_ExportInfoMap.find(LocalUniqueCopy(function));
    if(ex == m_ExportInfoMap.end())
    {
        return nullptr; // ignore unresolved exports
    }    
    auto pFuncInfo = ex->second->m_pFunctionInfo;
    if(ex->second->m_GraphTraversalFlags & CExportInfo::GTF_CycleFound)
    {
        return nullptr; // skip graph cycles 
    }
    CExportInfo* pExportInfo = ex->second;
    if(pExportInfo->m_VisitedOnGraphTraversalIndex == m_TraversalGlobals.GraphTraversalIndex)
    {
        return pExportInfo->m_pFirstSubobjectInLibraryFunctionSubtree;
    }
    assert(m_sAssociateableSubobjectData[m_TraversalGlobals.AssociateableSubobjectIndex].bAtMostOneAssociationPerExport);
    auto& currAssociation = pExportInfo->m_Associations[m_TraversalGlobals.AssociateableSubobjectIndex];
    auto pCurrSubobject = currAssociation.size() ? currAssociation.front()->m_pSubobject : nullptr; // just take first  
    pExportInfo->m_VisitedOnGraphTraversalIndex = m_TraversalGlobals.GraphTraversalIndex;
    if(pCurrSubobject)
    {
        pExportInfo->m_pFirstSubobjectInLibraryFunctionSubtree = pCurrSubobject;
    }
    for(UINT i = 0; i < pFuncInfo->NumFunctionDependencies; i++)
    {
        auto pMatch = TraverseFunctionsFindFirstSubobjectInLibraryFunctionSubtrees(pFuncInfo->FunctionDependencies[i]);
        if(!pExportInfo->m_pFirstSubobjectInLibraryFunctionSubtree)
        {
            pExportInfo->m_pFirstSubobjectInLibraryFunctionSubtree = pMatch;
        }
    }
    return pExportInfo->m_pFirstSubobjectInLibraryFunctionSubtree;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::TraverseFunctionsSubobjectConsistency
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::TraverseFunctionsSubobjectConsistency(LPCWSTR function)
{
    auto ex = m_ExportInfoMap.find(LocalUniqueCopy(function));
    if(ex == m_ExportInfoMap.end())
    {
        return; // ignore unresolved exports
    }    
    auto pFuncInfo = ex->second->m_pFunctionInfo;
    auto& flags = ex->second->m_GraphTraversalFlags;
    if(flags & CExportInfo::GTF_CycleFound)
    {
        return; // skip graph cycles 
    }
    if(ex->second->m_VisitedOnGraphTraversalIndex == m_TraversalGlobals.GraphTraversalIndex)
    {
        return;
    }
    assert(m_sAssociateableSubobjectData[m_TraversalGlobals.AssociateableSubobjectIndex].bAtMostOneAssociationPerExport);
    auto& currAssociation = ex->second->m_Associations[m_TraversalGlobals.AssociateableSubobjectIndex];
    auto pCurrSubobject = currAssociation.size() ? currAssociation.front()->m_pSubobject : nullptr; // just take first  
    auto& pRefSubobject = m_TraversalGlobals.pReferenceSubobject;
    const auto& MatchRule = m_sAssociateableSubobjectData[m_TraversalGlobals.AssociateableSubobjectIndex].MatchRule;
    if(m_TraversalGlobals.bAssignedRef)
    {
        switch(MatchRule)
        {
        case MatchRule_RequiredAndMatchingForAllExports: // elsewhere validated that ever export has an association, so now just make sure they match if non-null
        case MatchRule_IfExistsMustMatchOthersThatExistPlusShaderEntry:      
            if(pCurrSubobject && pRefSubobject && !pRefSubobject->Compare(pCurrSubobject))
            {
                LOG_ERROR(L"For subobjects of type " << 
                m_sAssociateableSubobjectData[m_TraversalGlobals.AssociateableSubobjectIndex].StringAPIName << 
                ((MatchRule_RequiredAndMatchingForAllExports == MatchRule)? 
                    L", for any function in a call graph that has this type of subobject associated, it must either match the subobject associated with other functions in the graph, or if there are different subobjects their respective definitions must match. "
                    : m_TraversalGlobals.bRootIsEntryFunction ? L" it is optional to associate them to any given function, but for any function in a call graph that has this type of subobject associated, it must either match the subobject (if any) associated at the shader entrypoint in the graph, or if there are different subobjects their respective definitions must match the association at the entrypoint. "
                    : L" it is optional to associate them to any given function, but for any function in a library function call graph that has this type of subobject associated, it must either match the subobject (if any) associated with other functions in the graph, or if there are different subobjects their respective definitions must match. ")
                << L"In this case function " << PrettyPrintPossiblyMangledName(function) << L" has a different definition for this subobject type than another function in the same call graph: " <<
                PrettyPrintPossiblyMangledName(m_TraversalGlobals.pNameOfExportWithReferenceSubobject) << L".");                   
            }
            break;
        case MatchRule_IfExistsMustExistAndMatchForAllExports:
            if(((pCurrSubobject != nullptr) ^ (pRefSubobject != nullptr))||(pCurrSubobject && pRefSubobject && !pRefSubobject->Compare(pCurrSubobject)))
            {
                LOG_ERROR(L"For subobjects of type " << 
                m_sAssociateableSubobjectData[m_TraversalGlobals.AssociateableSubobjectIndex].StringAPIName << 
                    L", if any function in a call graph has this type of subobject associated, every function in the call graph must either match the subobject associated with other functions in the graph, or if there are different subobjects their respective definitions must match. "
                << L"In this case function " << PrettyPrintPossiblyMangledName(function) << L" has a different definition for (or presence of) this subobject type than another function in the same call graph: " <<
                PrettyPrintPossiblyMangledName(m_TraversalGlobals.pNameOfExportWithReferenceSubobject) << L".");                                   
            }
            break;
        }
    }
    else
    {
        switch(MatchRule)
        {
        case MatchRule_IfExistsMustMatchOthersThatExistPlusShaderEntry:
            assert(m_TraversalGlobals.bRootIsEntryFunction); // if not we would have assigned a ref before recursing
            break;
        case MatchRule_IfExistsMustExistAndMatchForAllExports:
            break;            
        case MatchRule_RequiredAndMatchingForAllExports:
        case MatchRule_NoRequirements:
        default:
            assert(false);
        }
        m_TraversalGlobals.bAssignedRef = true;
        pRefSubobject = pCurrSubobject;
#ifdef INCLUDE_MESSAGE_LOG
        if(pRefSubobject)
        {
            m_TraversalGlobals.pNameOfExportWithReferenceSubobject = function;
        }
#endif
    }
    if(pRefSubobject)
    {
        // if we've found a reference subobject we will have checked the subgraph against this reference
        ex->second->m_VisitedOnGraphTraversalIndex = m_TraversalGlobals.GraphTraversalIndex;        
        // otherwise don't count this function as visited yet (don't optimize out future visits to it)
    }

    for(UINT i = 0; i < pFuncInfo->NumFunctionDependencies; i++)
    {
        TraverseFunctionsSubobjectConsistency(pFuncInfo->FunctionDependencies[i]);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::ResolveSubobjectAssociations
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::ResolveSubobjectAssociations()
{
    for(ASSOCIATEABLE_SUBOBJECT_NAME i = (ASSOCIATEABLE_SUBOBJECT_NAME)0; i < NUM_ASSOCIATEABLE_SUBOBJECT_TYPES; ((UINT&)i)++)
    {
        for(auto& association : m_SubobjectToExportsAssociations[i])
        {
            if(association.m_Exports.size())
            {
                // Mark that the object being associated has been used in an explicit association, if applicable,
                // (so it doesn't apply for use in default associations)
                assert(ASN_INVALID != AssociateableSubobjectName(association.m_SubobjectType));
                association.m_pSubobject->m_bUsedInExplicitAssociation = true;
                association.m_pSubobject->m_bReferenced = true;
            }
            for( auto ex : association.m_Exports)
            {
                bool bFoundGoodMatch = false;
                auto match = m_ExportInfoMap.find(ex);
                if(match == m_ExportInfoMap.end())
                {
                    // Try unmangled name searching
                    if(m_ExportNameUnmangledToMangled.count(ex))
                    {
                        auto mangledMatches = m_ExportNameUnmangledToMangled.equal_range(ex);
                        for(auto mangledMatch = mangledMatches.first; mangledMatch != mangledMatches.second; mangledMatch++)
                        {
                            match = m_ExportInfoMap.find(mangledMatch->second);
                            match->second->m_Associations[AssociateableSubobjectName(association.m_SubobjectType)].push_back(&association);                        
                        }
                        bFoundGoodMatch = true;
                    };
                }
                else
                {
                    bFoundGoodMatch = true;
                    match->second->m_Associations[AssociateableSubobjectName(association.m_SubobjectType)].push_back(&association);
                }
                // Try hit groups
                auto hgMatch = m_HitGroups.find(ex);
                if(hgMatch != m_HitGroups.end())
                {
                    bFoundGoodMatch = true;
                    hgMatch->second->m_Associations[AssociateableSubobjectName(association.m_SubobjectType)].push_back(&association);
                }

                if(!bFoundGoodMatch)
                {
                    if(!AllowExternalDependenciesOnLocalDefinitions())
                    {
                        LOG_ERROR(L"Subobject association of type " <<  m_sAssociateableSubobjectData[AssociateableSubobjectName(association.m_SubobjectType)].StringAPIName 
                                    << L" made to export named " << PrettyPrintPossiblyMangledName(ex) << L" which doesn't exist." <<
                                    ((D3D12_STATE_OBJECT_TYPE_COLLECTION == m_SOType) ? 
                                    L" To allow this subobject to be associated with external functions (to be resolved later, when this state object is combined with other state object(s)), "
                                    L"use a D3D12_STATE_OBJECT_CONFIG subobject with D3D12_STATE_OBJECT_FLAG_ALLOW_EXTERNAL_DEPENDENCIES_ON_LOCAL_DEFINITIONS set in Flags." : L""));
                    }
                    m_bAssociationsToUnresolvedFunctions = true;
                }
            }
        }
        // For each associatable subobject type, determine if there's a suitable default subobject for the current scope.
        // Meaning: a single subobject of a given type that eiter:
        // (a) has an association defined with no exports explicitly setting it as the default
        // (b) doesn't have any explicit associations, implicitly setting it as the default if (a) isn't satisified
        auto& AssociateableData = m_AssociateableSubobjectData[i];
        if(AssociateableData.pUsedInExplicitDefaultAssociation)
        {
            assert(ASN_INVALID != AssociateableSubobjectName(AssociateableData.pUsedInExplicitDefaultAssociation->m_SubobjectType));
            AssociateableData.pLocalDefaultSubobject = AssociateableData.pUsedInExplicitDefaultAssociation;
        }
        if(!AssociateableData.pLocalDefaultSubobject)
        {
            bool bFoundSingleUnassociated = false;
            CAssociateableSubobjectInfo* pDefaultAssociationCandidate = nullptr;
            for(auto a : AssociateableData.Associateable)
            {
                if(!a->m_bUsedInExplicitAssociation)
                {
                    if(bFoundSingleUnassociated)
                    {
                        if(pDefaultAssociationCandidate->Compare(a))
                        {
                            // Found duplicate identical unassociated subobjects, still consider that a default candidate. 
                            continue;
                        }
                        bFoundSingleUnassociated = false;
                        break;
                    }
                    pDefaultAssociationCandidate = a;
                    bFoundSingleUnassociated = true;
                }
            }
            if(bFoundSingleUnassociated)
            {
                // Found a default, now log it as an association that exports can point to
                m_SubobjectToExportsAssociations[i].emplace_back();
                CWrappedAssociation* pWrapped = &m_SubobjectToExportsAssociations[i].back();
                pWrapped->m_pSubobject = pDefaultAssociationCandidate;
                pWrapped->m_SubobjectType = pDefaultAssociationCandidate->m_LocalSubobjectDefinition.Type;  
                AssociateableData.pLocalDefaultSubobject = pWrapped;
            }
        }
    }

    bool bHitGroupAssociationsApplied = false;
    auto ApplyHitGroupAssociations = [this,&bHitGroupAssociationsApplied]()
    {
        if(bHitGroupAssociationsApplied)
        {
            return;
        }
        bHitGroupAssociationsApplied = true;
        // Apply hit group associations to any constituent shaders that don't have an equivalent association.
        // If there are inconsistencies with existing associations they will be discovered further below.
        // Also check if there are too many associations to a hit group
        for(auto& hg : m_HitGroups)
        {
            for(ASSOCIATEABLE_SUBOBJECT_NAME i = (ASSOCIATEABLE_SUBOBJECT_NAME)0; i < NUM_ASSOCIATEABLE_SUBOBJECT_TYPES; ((UINT&)i)++)
            {
                auto& associations = hg.second->m_Associations[i];
                if(0 == associations.size())
                {
                    continue;
                }
                for(auto& a : associations)
                {
                    for(UINT s = 0; s < 3; s++)
                    {
                        LPCWSTR pDependency = nullptr;
                        switch (s)
                        {
                        case 0:
                            pDependency = hg.second->AnyHitShaderImport;                      
                            break;
                        case 1:
                            pDependency = hg.second->ClosestHitShaderImport;
                            break;
                        case 2:
                            pDependency = hg.second->IntersectionShaderImport;
                            break;
                        default:
                            assert(false);
                            break;
                        }
                        if (!pDependency)
                        {
                            continue;
                        }
                        // Does unmangled search give hit(s)?  
                        CExportInfo* pMatch = nullptr;
                        LPCWSTR pMatchMangledName = nullptr;
                        size_t count = m_ExportNameUnmangledToMangled.count(pDependency);
                        switch (count)
                        {
                        case 0:
                        {
                            // Try mangled
                            auto match = m_ExportInfoMap.find(pDependency);
                            if (match != m_ExportInfoMap.end())
                            {
                                pMatch = match->second;
                                pMatchMangledName = match->first;
                            }
                            break;
                        }
                        case 1:
                        {
                            auto mangledMatch = m_ExportNameUnmangledToMangled.find(pDependency);
                            auto match = m_ExportInfoMap.find(mangledMatch->second);
                            pMatch = match->second;
                            pMatchMangledName = match->first;
                            break;
                        }
                        default:
                            // Error case validated elsewhere, ignore
                            break;
                        }
                        if (pMatch)
                        {
                            pMatch->m_Associations[i].push_back(a);
                            a->m_Exports.insert(pMatchMangledName);
                            a->m_pSubobject->m_bReferenced = true;
                        }
                    }
                }
            }
        }
    };
   
    // Apply default associations while looking to see if there are too many...
    for(auto ex : m_ExportInfoMap)
    {
        CExportInfo* pExportInfo = ex.second;
        if(SupportedShaderType((ShaderKind)ex.second->m_pFunctionInfo->ShaderKind))
        {            
            for(ASSOCIATEABLE_SUBOBJECT_NAME i = (ASSOCIATEABLE_SUBOBJECT_NAME)0; i < NUM_ASSOCIATEABLE_SUBOBJECT_TYPES; ((UINT&)i)++)
            {
                size_t count = pExportInfo->m_Associations[i].size();
                switch(count)
                {
                case 0:
                    if(m_AssociateableSubobjectData[i].pLocalDefaultSubobject) 
                    {
                        pExportInfo->m_Associations[i].push_back(m_AssociateableSubobjectData[i].pLocalDefaultSubobject);
                        m_AssociateableSubobjectData[i].pLocalDefaultSubobject->m_Exports.insert(ex.first);
                        m_AssociateableSubobjectData[i].pLocalDefaultSubobject->m_pSubobject->m_bReferenced = true;
                    }
                    else
                    {
                        ApplyHitGroupAssociations();
                        if(MatchRule_RequiredAndMatchingForAllExports == m_sAssociateableSubobjectData[i].MatchRule)
                        {
                            if(!AllowLocalDependenciesOnExternalDefinitions())
                            {
                                auto unMangled = m_ExportNameMangledToUnmangled.find(ex.first);
                                assert(unMangled != m_ExportNameMangledToUnmangled.end());
    #ifdef INCLUDE_MESSAGE_LOG
                                if(m_AssociateableSubobjectData[i].bContinuePrintingMissingSubobjectMessages)
                                {
                                    if((m_AssociateableSubobjectData[i].Associateable.size() == 0) && 
                                    (m_AssociateableSubobjectData[i].InheritedSoNotAssociateable.size() == 0))
                                    {
                                        LOG_ERROR(L"Subobject association of type " << m_sAssociateableSubobjectData[i].StringAPIName 
                                                    << L" must be defined for all relevant exports, yet no such subobject exists at all.  And example of an export needing this association is " <<
                                                    PrettyPrintPossiblyMangledName(ex.first) << L"." <<
                                                    ((D3D12_STATE_OBJECT_TYPE_COLLECTION == m_SOType) ? 
                                                    L" If the intent is this will be resolved later, when this state object is combined with other state object(s), "
                                                    L"use a D3D12_STATE_OBJECT_CONFIG subobject with D3D12_STATE_OBJECT_FLAG_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITIONS set in Flags." : L""));                            

                                        m_AssociateableSubobjectData[i].bContinuePrintingMissingSubobjectMessages = false;
                                    }
                                    else
    #endif
                                    {
                                        LOG_ERROR(L"Export " << PrettyPrintPossiblyMangledName(ex.first) << L" is missing a required subobject association of type " 
                                        << m_sAssociateableSubobjectData[i].StringAPIName << L"." <<
                                        ((D3D12_STATE_OBJECT_TYPE_COLLECTION == m_SOType) ? 
                                            L" If the intent is this will be resolved later, when this state object is combined with other state object(s), "
                                            L"use a D3D12_STATE_OBJECT_CONFIG subobject with D3D12_STATE_OBJECT_FLAG_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITIONS set in Flags." : L""));                            
    
                                    }
    #ifdef INCLUDE_MESSAGE_LOG
                                }
    #endif                            
                            }
                            ex.second->m_bUnresolvedAssociations = true;
                            m_bFunctionsWithUnresolvedAssociations = true;
                        }
                    }
                    break;
                case 1:
                    break;
                default:
                    if(m_sAssociateableSubobjectData[i].bAtMostOneAssociationPerExport)
                    {
                        CAssociateableSubobjectInfo* pRefSubobject = nullptr;
                        for(auto a : pExportInfo->m_Associations[i] )
                        {
                            if(pRefSubobject)
                            {
                                if(!pRefSubobject->Compare(a->m_pSubobject))
                                {
                                    auto unMangled = m_ExportNameMangledToUnmangled.find(ex.first);
                                    assert(unMangled != m_ExportNameMangledToUnmangled.end());
                                    LOG_ERROR( L"Export " << PrettyPrintPossiblyMangledName(ex.first) << L" has multiple subobject associations of type " 
                                    << m_sAssociateableSubobjectData[i].StringAPIName << L" when only one is expected, or if there are multiple subobjects associated they must have matching definitions.");
                                    break;                                    
                                }
                            }
                            else
                            {
                                pRefSubobject = a->m_pSubobject;
                            }
                        }
                    }
                    break;
                }
            }           
        }
    }
 
    // Check subobject association consistency across call graphs and hit groups
    m_TraversalGlobals.AssociateableSubobjectIndex = (ASSOCIATEABLE_SUBOBJECT_NAME)0;
    for(auto& i = m_TraversalGlobals.AssociateableSubobjectIndex; i < NUM_ASSOCIATEABLE_SUBOBJECT_TYPES; ((UINT&)i)++)
    {
        assert(m_sAssociateableSubobjectData[i].bAtMostOneAssociationPerExport); // this code doesn't support
                                                                                 // validating that if multiple associations of 
                                                                                 // the same type are allowed per export, that 
                                                                                 // the call graph has the same set per node.
        const auto& MatchRule = m_sAssociateableSubobjectData[i].MatchRule;
        switch(m_sAssociateableSubobjectData[i].MatchScope)
        {
        case MatchScope_FullStateObject:
        case MatchScope_LocalStateObject:
        {
            bool bMatchScopeLocal = (MatchScope_LocalStateObject == m_sAssociateableSubobjectData[i].MatchScope);
            bool bSkip = true;
            switch(MatchRule)
            {
            case MatchRule_RequiredAndMatchingForAllExports:
            case MatchRule_IfExistsMustExistAndMatchForAllExports:
            case MatchRule_IfExistsMustMatchOthersThatExistPlusShaderEntry:
                bSkip = false;
                break;
            case MatchRule_NoRequirements:
                break;
            default:
                assert(false);
                break;
            }
            if(bSkip)
            {
                break;
            }
            auto& pRefSubobject = m_TraversalGlobals.pReferenceSubobject;
            pRefSubobject = nullptr;
            bool bAssignedRef = false;
            for(auto function : m_ExportInfoMap)
            {
                if(bMatchScopeLocal && (function.second->m_pOwningStateObject != this))
                {
                    continue;
                }
                auto& currAssociation = function.second->m_Associations[i];
                auto pCurrSubobject = currAssociation.size() ? currAssociation.front()->m_pSubobject : nullptr; // just take first
                if(bAssignedRef)
                {
                    switch(MatchRule)
                    {
                    case MatchRule_RequiredAndMatchingForAllExports:
                    case MatchRule_IfExistsMustMatchOthersThatExistPlusShaderEntry: // "ShaderEntry" clause isn't meanigful for full graph matching but the rest is
                        assert(!bMatchScopeLocal); // would need to differentiate error message if this case is added.
                        if(pRefSubobject && pCurrSubobject && !pRefSubobject->Compare(pCurrSubobject)) // seperately validated they all have to be assiged, so here only need to compare if they both exist
                        {
                            LOG_ERROR(L"For subobjects of type " <<
                            m_sAssociateableSubobjectData[i].StringAPIName << 
                            ((MatchRule_RequiredAndMatchingForAllExports == MatchRule)? 
                                L", every function in a state object must be associated to either the same sububject definition, or if there are different subobjects their respective definitions must match. "
                            : L" it is optional to associate them to any given function, but for any function in a state object that has this type of subobject associated, it must either match the subobject (if any) associated with other functions in the state object, or if there are different subobjects their respective definitions must match. ")
                            << L"In this case function " << PrettyPrintPossiblyMangledName(function.first) << L" has a different definition for this subobject type than another function in the same state object: " <<
                            PrettyPrintPossiblyMangledName(m_TraversalGlobals.pNameOfExportWithReferenceSubobject) << L".");            
                        }
                        break;
                    case MatchRule_IfExistsMustExistAndMatchForAllExports:
                        if(((pCurrSubobject != nullptr) ^ (pRefSubobject != nullptr))||(pCurrSubobject && pRefSubobject && !pRefSubobject->Compare(pCurrSubobject)))
                        {
                            LOG_ERROR(L"For subobjects of type " <<
                            m_sAssociateableSubobjectData[i].StringAPIName << 
                            L" it is optional to associate them to any given function, but once any function defined in this state object " <<
                            ((D3D12_STATE_OBJECT_TYPE_COLLECTION == m_SOType) 
                            ? (bMatchScopeLocal ? L"(not including definition in any state objects that enclose this one or are peers)" : L"(including definition in any state objects that enclose this collection or are peers) ")
                            : (bMatchScopeLocal ? L"(not including definition in any contained collections)" : L"(including definition in any contained collections) ")) <<
                            L"has this type of subobject associated, all functions either have the same subobject associated, or if there are different subobjects their respective definitions must match. "
                            << L"In this case function " <<
                            PrettyPrintPossiblyMangledName(function.first) << L" has a different definition for (or presence of) this subobject type than another function in the same state object: " <<
                            PrettyPrintPossiblyMangledName(m_TraversalGlobals.pNameOfExportWithReferenceSubobject) << L".");   
                        }
                        break;
                    default:
                        assert(false);
                        break;
                    }
                }
                else
                {
                    switch(MatchRule)
                    {
                    case MatchRule_RequiredAndMatchingForAllExports:
                    case MatchRule_IfExistsMustMatchOthersThatExistPlusShaderEntry:
                        if(pCurrSubobject)
                        {
                            bAssignedRef = true;
                        }
                        break;
                    case MatchRule_IfExistsMustExistAndMatchForAllExports:
                        bAssignedRef = true;
                        break;
                    default:
                        assert(false);
                        break;
                    }
                    pRefSubobject = pCurrSubobject;
#ifdef INCLUDE_MESSAGE_LOG
                    m_TraversalGlobals.pNameOfExportWithReferenceSubobject = function.first;
#endif                    
                }
            }            
            break;
        }
        case MatchScope_CallGraph:
        {
            switch(MatchRule)
            {
            case MatchRule_IfExistsMustMatchOthersThatExistPlusShaderEntry:
            case MatchRule_IfExistsMustExistAndMatchForAllExports:
            {
                for(auto& ex : m_ExportInfoList)
                {
                    if(ShaderKind::Library == (ShaderKind)ex.m_pFunctionInfo->ShaderKind)
                    {
                        TraverseFunctionsFindFirstSubobjectInLibraryFunctionSubtrees(ex.m_MangledName);
                    }
                }
                m_TraversalGlobals.GraphTraversalIndex++; // considering traversals for all exports as one merge graph traversal for efficiency 
                break;
            case MatchRule_NoRequirements:
            case MatchRule_RequiredAndMatchingForAllExports:
                break;                
            default:
                assert(false);
                break;
            }
            }
            for(auto& ex : m_ExportInfoList)
            {            
                m_TraversalGlobals.bRootIsEntryFunction = (ShaderKind::Library != (ShaderKind)ex.m_pFunctionInfo->ShaderKind);
                if(  ((MatchRule_IfExistsMustMatchOthersThatExistPlusShaderEntry == MatchRule) && m_TraversalGlobals.bRootIsEntryFunction)
                  || (MatchRule_IfExistsMustExistAndMatchForAllExports == MatchRule))
                {
                    m_TraversalGlobals.bAssignedRef = false;
                    m_TraversalGlobals.pReferenceSubobject = nullptr; // will get assigned on first node visit
                }
                else
                {
                    m_TraversalGlobals.bAssignedRef = true;
                    m_TraversalGlobals.pReferenceSubobject = ex.m_pFirstSubobjectInLibraryFunctionSubtree;
                }
                TraverseFunctionsSubobjectConsistency(ex.m_MangledName);
            }
            m_TraversalGlobals.GraphTraversalIndex++; // considering traversals for all exports as one merge graph traversal for efficiency            
            break;
        }
        case MatchScope_NoRequirements:
            break;
        default:
            assert(false);
            break;
        }
        // Check hit groups
        switch(m_sAssociateableSubobjectData[i].MatchScope)
        {
        case MatchScope_CallGraph:
        case MatchScope_FullStateObject:
        case MatchScope_LocalStateObject:
        {
            bool bMatchScopeLocal = (MatchScope_LocalStateObject == m_sAssociateableSubobjectData[i].MatchScope);
            for(auto& hg : m_HitGroups)
            {
                auto& currHitGroupGlobalAssociations = hg.second->m_Associations[i];
                auto pHitGroupSubobject = currHitGroupGlobalAssociations.size() ? currHitGroupGlobalAssociations.front()->m_pSubobject : nullptr; // just take first
                auto pRefSubobject = pHitGroupSubobject; // initial reference is what's assigned to the hit group, but if it's null, don't count it as being unassociated
                if(bMatchScopeLocal && (this != hg.second->m_pOwningStateObject))
                {
                    pRefSubobject = nullptr;
                }
                const auto& MatchRule = m_sAssociateableSubobjectData[i].MatchRule;
                if(!pRefSubobject && ((MatchScope_FullStateObject == m_sAssociateableSubobjectData[i].MatchScope) || bMatchScopeLocal) && (MatchRule_RequiredAndMatchingForAllExports == MatchRule))
                {
                    break; // for FullStateObject, would have already validated that all exports match, and if the HitGroup has no association, nothing to validate
                }
                UINT DependencyIndexWhereSubobjectCameFrom = (UINT)-1;
                const CWrappedHitGroup& hgDesc = *hg.second;
                LPCWSTR pNameOfHitGroupDependencyForRefSubobject = nullptr;
                for(UINT s = 0; s < 3; s++)
                {
                    LPCWSTR pDependency = nullptr;
                    switch (s)
                    {
                    case 0:
                        pDependency = hgDesc.AnyHitShaderImport;
                        break;
                    case 1:
                        pDependency = hgDesc.ClosestHitShaderImport;
                        break;
                    case 2:
                        pDependency = hgDesc.IntersectionShaderImport;
                        break;
                    default:
                        assert(false);
                        break;
                    }
                    if(!pDependency)
                    {
                        continue;
                    }
                    // Does unmangled search give hit(s)?  
                    CExportInfo* pMatch = nullptr;
                    size_t count = m_ExportNameUnmangledToMangled.count(pDependency);
                    switch (count)
                    {
                    case 0:
                    {
                        // Try mangled
                        auto match = m_ExportInfoMap.find(pDependency);
                        if (match != m_ExportInfoMap.end())
                        {
                            pMatch = match->second;
                        }
                        break;
                    }
                    case 1:
                    {
                        auto mangledMatch = m_ExportNameUnmangledToMangled.find(pDependency);
                        auto match = m_ExportInfoMap.find(mangledMatch->second);
                        pMatch = match->second;
                        break;
                    }
                    default:
                        // Error case validated elsewhere, ignore
                        break;
                    }
                    if(bMatchScopeLocal && pMatch && (this != pMatch->m_pOwningStateObject))
                    {
                        continue;
                    }
                    if (pMatch)
                    {
                        auto& currAssociations = pMatch->m_Associations[i];
                        auto pNewSubobject = currAssociations.size() ? currAssociations.front()->m_pSubobject : nullptr; // just take first
                        switch(MatchRule)
                        {
                        case MatchRule_RequiredAndMatchingForAllExports:
                            break;
                        case MatchRule_IfExistsMustMatchOthersThatExistPlusShaderEntry:
                        case MatchRule_IfExistsMustExistAndMatchForAllExports:
                            break;
                        case MatchRule_NoRequirements:
                        default:
                            assert(false);
                            break;
                        }
                        switch(MatchRule)
                        {
                        case MatchRule_RequiredAndMatchingForAllExports:
                        case MatchRule_IfExistsMustMatchOthersThatExistPlusShaderEntry:
                            if(pRefSubobject && pNewSubobject && !pRefSubobject->Compare(pNewSubobject))
                            {
                                assert(!bMatchScopeLocal); // would need to differentiate error message if this case is needed
#ifdef INCLUDE_MESSAGE_LOG
                                bool bSubobjectCameFromHitGroup = (DependencyIndexWhereSubobjectCameFrom == -1) ? true : false;
#endif                                
                                LOG_ERROR(L"For subobjects of type " <<
                                m_sAssociateableSubobjectData[i].StringAPIName << 
                                (MatchRule_RequiredAndMatchingForAllExports == m_sAssociateableSubobjectData[i].MatchRule? 
                                    L", for any member of a HitGroup that has this type of subobject associated, it must either match the subobject associated with other members, or if there are different subobjects their respective definitions must match. "
                                : L" it is optional to associate them to any given member of a HitGroup, but for any member that has this type of subobject associated, it must either match the subobject (if any) associated with other members of the HitGroup, or if there are different subobjects their respective definitions must match. ") <<
                                L"In this case " << 
                                (bSubobjectCameFromHitGroup ? L"overall HitGroupExport" : GetHitGroupDependencyTypeName(DependencyIndexWhereSubobjectCameFrom)) << 
                                L" \"" <<
                                (bSubobjectCameFromHitGroup ? hgDesc.HitGroupExport : pNameOfHitGroupDependencyForRefSubobject) <<
                                L"\" has a different definition for this subobject type than " << GetHitGroupDependencyTypeName(s) << L": " <<
                                PrettyPrintPossiblyMangledName(pDependency) << L".");                                        
                            }
                            break;
                        case MatchRule_IfExistsMustExistAndMatchForAllExports:
                            if( ( ((pRefSubobject != nullptr) || (DependencyIndexWhereSubobjectCameFrom != -1)) && // hit group contributed a non-null subobject, or else a hit group member provided the reference subobject
                                  ((pRefSubobject != nullptr) ^ (pNewSubobject != nullptr))
                                )                                
                                || (pRefSubobject && pNewSubobject && !pRefSubobject->Compare(pNewSubobject)))
                            {
#ifdef INCLUDE_MESSAGE_LOG
                                bool bSubobjectCameFromHitGroup = (DependencyIndexWhereSubobjectCameFrom == -1) ? true : false;
#endif                                
                                LOG_ERROR(L"For subobjects of type " <<
                                m_sAssociateableSubobjectData[i].StringAPIName <<                                     
                                    L", if any member of a HitGroup defined in this state object " << 
                                    ((D3D12_STATE_OBJECT_TYPE_COLLECTION == m_SOType) 
                                    ? (bMatchScopeLocal ? L"(not defined in any state objects that enclose this one or are peers)" : L"(including definition in state objects that enclose this collection or are peers) ")
                                    : (bMatchScopeLocal ? L"(not defined within any contained collections)" : L"(including definition within any contained collections) ")) <<
                                    L"has this type of subobject associated, all other members of the HitGroup must either have the same subobject associated, or if there are different subobjects their respective definitions must match. "
                                        L"In this case " << 
                                (bSubobjectCameFromHitGroup ? L"overall HitGroupExport" : GetHitGroupDependencyTypeName(DependencyIndexWhereSubobjectCameFrom)) << 
                                L" \"" <<
                                (bSubobjectCameFromHitGroup ? hgDesc.HitGroupExport : pNameOfHitGroupDependencyForRefSubobject) <<
                                L"\" has a different definition for this subobject type than " << GetHitGroupDependencyTypeName(s) << L": " <<
                                PrettyPrintPossiblyMangledName(pDependency) << L".");                                        
                            }
                            break;
                        }
                        if(!pRefSubobject)
                        {
                            pRefSubobject = pNewSubobject;
                            pNameOfHitGroupDependencyForRefSubobject = pDependency;                            
#ifdef INCLUDE_MESSAGE_LOG
                            DependencyIndexWhereSubobjectCameFrom = s;
#endif
                        }
                    }
                }
            }
            break;
        }
        default:
            assert(false);
            break;
        }        
    }    
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::ResolveResourceBindings
//----------------------------------------------------------------------------------------------------------------------------------
#ifndef SKIP_BINDING_VALIDATION
void CStateObjectInfo::ResolveResourceBindings()
{
    for(auto& ex : m_ExportInfoList)
    {
        CRootSigPair& RSP = m_TraversalGlobals.RootSigs;
        RSP.m_pGlobal = ex.m_Associations[ASN_GLOBAL_ROOT_SIGNATURE].size() 
            ? (CSimpleAssociateableSubobjectWrapper<D3D12_GLOBAL_ROOT_SIGNATURE>*)ex.m_Associations[ASN_GLOBAL_ROOT_SIGNATURE].front()->m_pSubobject : nullptr;
        RSP.m_pLocal = ex.m_Associations[ASN_LOCAL_ROOT_SIGNATURE].size() 
            ? (CSimpleAssociateableSubobjectWrapper<D3D12_LOCAL_ROOT_SIGNATURE>*)ex.m_Associations[ASN_LOCAL_ROOT_SIGNATURE].front()->m_pSubobject : nullptr;
        auto pairValidationResult = m_ValidatedRootSigPairs.find(RSP);
        bool bPairValidationSucceeded = true;
        if(pairValidationResult == m_ValidatedRootSigPairs.end())
        {
            // Validate pair
            m_RootSigPairVerifierList.emplace_back();
            auto& pVerifier = m_TraversalGlobals.pRootSigVerifier;
            pVerifier = &m_RootSigPairVerifierList.back();
            ValidateRootSignaturePair(RSP,pVerifier);
            bPairValidationSucceeded = pVerifier->m_bRootSigsValidTogether;
            m_ValidatedRootSigPairs.insert({RSP,pVerifier});
        }
        else
        {
            bPairValidationSucceeded = pairValidationResult->second->m_bRootSigsValidTogether;
            m_TraversalGlobals.pRootSigVerifier = pairValidationResult->second;
        }
        if(bPairValidationSucceeded)
        {
            TraverseFunctionsResourceBindingValidation(ex.m_MangledName);
            // Don't need to increment graph traversal index since this traversal doesn't touch the index: m_TraversalGlobals.GraphTraversalIndex++;
        }
    }
    for(auto& ex : m_ExportInfoList)
    {
        ex.m_RootSigsValidatedOnSubtree.clear();
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::TraverseFunctionsResourceBindingValidation
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::TraverseFunctionsResourceBindingValidation(LPCWSTR function)
{
    auto ex = m_ExportInfoMap.find(LocalUniqueCopy(function));
    if(ex == m_ExportInfoMap.end())
    {
        return; // ignore unresolved exports
    }    
    auto pFuncInfo = ex->second->m_pFunctionInfo;
    auto& flags = ex->second->m_GraphTraversalFlags;
    if(flags & CExportInfo::GTF_CycleFound)
    {
        return; // skip graph cycles 
    }        
    if(ex->second->m_RootSigsValidatedOnSubtree.find(m_TraversalGlobals.RootSigs) != ex->second->m_RootSigsValidatedOnSubtree.end())
    {
        return; // already validated this subtree against these root signatures
    }
    // Validate this function against root signatures    
    RLFECallbackContext cc;
    cc.pLibraryFunction = function;
    cc.pExportInfo = ex->second;
    cc.pThis = this;
    m_TraversalGlobals.pRootSigVerifier->m_RSV.VerifyLibraryFunction(pFuncInfo,&cc,ReportLibraryFunctionErrorCallback);

    // Validate subtree against root signatures
    for(UINT i = 0; i < pFuncInfo->NumFunctionDependencies; i++)
    {
        TraverseFunctionsResourceBindingValidation(pFuncInfo->FunctionDependencies[i]);
    }
    ex->second->m_RootSigsValidatedOnSubtree.insert(m_TraversalGlobals.RootSigs);
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::ReportLibraryFunctionErrorCallback
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::ReportLibraryFunctionErrorCallback(void* pContext,LPCWSTR pError,UINT ErrorFlags)
{
    auto pCallbackContext = (RLFECallbackContext*)pContext;
    auto pThis = pCallbackContext->pThis;
    if(((ErrorFlags & RootSignatureVerifier::VLF_UNRESOLVED_REFERENCE) && !pThis->AllowLocalDependenciesOnExternalDefinitions()) ||
       (!(ErrorFlags & RootSignatureVerifier::VLF_UNRESOLVED_REFERENCE))) // another type of error
    {
#ifdef INCLUDE_MESSAGE_LOG
        auto& RootSigs = pThis->m_TraversalGlobals.RootSigs;
#else
        UNREFERENCED_PARAMETER(pError);
#endif
        LOG_ERROR_IN_CALLBACK(L"Resource bindings for function " << pThis->PrettyPrintPossiblyMangledName(pCallbackContext->pLibraryFunction) << L" not compatible with associated root signatures (if any): local root signature object: 0x" << 
                (RootSigs.m_pLocal ? RootSigs.m_pLocal->pLocalRootSignature : 0) << L", global root signature object: 0x" <<
                (RootSigs.m_pGlobal ? RootSigs.m_pGlobal->pGlobalRootSignature : 0) << L". Error detail: " << pError <<
                  ( ((D3D12_STATE_OBJECT_TYPE_COLLECTION == pThis->m_SOType) &&
                    (ErrorFlags == RootSignatureVerifier::VLF_UNRESOLVED_REFERENCE) && !pThis->AllowLocalDependenciesOnExternalDefinitions()) ? 
                    L" If the intent is this will be resolved later when this state object is combined with other state object(s), "
                    L"use a D3D12_STATE_OBJECT_CONFIG subobject with D3D12_STATE_OBJECT_FLAG_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITIONS set in Flags." : L""
                  )
                );          
    }
    if(ErrorFlags & RootSignatureVerifier::VLF_UNRESOLVED_REFERENCE)
    {
        pThis->m_bUnresolvedResourceBindings = true;
        pCallbackContext->pExportInfo->m_bUnresolvedResourceBindings = true;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::ValidateRootSignaturePair
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::ValidateRootSignaturePair(const CRootSigPair& RootSigs, CRootSigVerifier* pVerifier)
{
    CComPtr<ID3D12VersionedRootSignatureDeserializer> pLocalRSDeserializer = nullptr;
    CComPtr<ID3D12VersionedRootSignatureDeserializer> pGlobalRSDeserializer = nullptr;
    if(RootSigs.m_pLocal && RootSigs.m_pLocal->pLocalRootSignature)
    {
         pLocalRSDeserializer = m_pfnGetRootSignatureDeserializer(RootSigs.m_pLocal->pLocalRootSignature);
    }
    if(RootSigs.m_pGlobal && RootSigs.m_pGlobal->pGlobalRootSignature)
    {
        pGlobalRSDeserializer = m_pfnGetRootSignatureDeserializer(RootSigs.m_pGlobal->pGlobalRootSignature);
    }
    const D3D12_VERSIONED_ROOT_SIGNATURE_DESC *pDescLocal = nullptr, *pDescGlobal = nullptr;
    HRESULT hr;
    if(pLocalRSDeserializer)
    {
        hr = pLocalRSDeserializer->GetRootSignatureDescAtVersion(D3D_ROOT_SIGNATURE_VERSION_1_1,&pDescLocal);
        if(FAILED(hr))
        {
            LOG_ERROR(L"Failed to parse local root signature 0x" << RootSigs.m_pLocal->pLocalRootSignature );
            pVerifier->m_bRootSigsValidTogether = false;
            return;
        }
    }
    if(pGlobalRSDeserializer)
    {
        hr = pGlobalRSDeserializer->GetRootSignatureDescAtVersion(D3D_ROOT_SIGNATURE_VERSION_1_1,&pDescGlobal);
        if(FAILED(hr))
        {
            LOG_ERROR(L"Failed to parse global root signature 0x" << RootSigs.m_pGlobal->pGlobalRootSignature );
            pVerifier->m_bRootSigsValidTogether = false;
            return;
        }
    }

    pVerifier->m_RSV.AllowRaytracing(true);
    CComPtr<ID3DBlob> pErrorBlob = nullptr;
    hr = pVerifier->m_RSV.VerifyRootSignaturePair(pDescLocal,pDescGlobal,&pErrorBlob);
    if(FAILED(hr))
    {
        if(!pErrorBlob)
        {
            LOG_ERROR(L"Verifying global and local root signatures against each other failed without explanation.  Root signature objects: 0x" << 
                (RootSigs.m_pGlobal ? RootSigs.m_pGlobal->pGlobalRootSignature : 0) << L", 0x" << (RootSigs.m_pLocal ? RootSigs.m_pLocal->pLocalRootSignature : 0));
            pVerifier->m_bRootSigsValidTogether = false;
            return;
        }
        LPCSTR charMessage = (LPCSTR)pErrorBlob->GetBufferPointer();
        size_t size = strlen(charMessage) + 1;
        std::vector<WCHAR> newString(size);
        mbstowcs_s(nullptr, newString.data(), size, charMessage, _TRUNCATE);
        LOG_ERROR(L"Local root signature 0x" << RootSigs.m_pLocal->pLocalRootSignature << L" can't be combined with global root signature 0x" <<
                RootSigs.m_pGlobal->pGlobalRootSignature << L", error expressed in terms of the global root signature: " << newString.data() );
        pVerifier->m_bRootSigsValidTogether = false;
        return;
    }
    pVerifier->m_bRootSigsValidTogether = true;
    return;
}
#endif

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::ValidateMiscAssociations
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::ValidateMiscAssociations()
{
    for(auto& ex : m_ExportInfoList)
    {
        auto pSOInfo = ex.m_Associations[ASN_RAYTRACING_SHADER_CONFIG].size() ? ex.m_Associations[ASN_RAYTRACING_SHADER_CONFIG].front()->m_pSubobject : nullptr;
        auto pConfig = pSOInfo ? (const D3D12_RAYTRACING_SHADER_CONFIG*)pSOInfo->m_LocalSubobjectDefinition.pDesc : nullptr;
        auto pFuncInfo = ex.m_pFunctionInfo;
        if(!pConfig)
        {
            continue;
        }
        if(pFuncInfo->AttributeSizeInBytes > pConfig->MaxAttributeSizeInBytes)
        {
#ifdef INCLUDE_MESSAGE_LOG            
        auto unmangledName = m_ExportNameMangledToUnmangled.find(ex.m_MangledName);
        LOG_ERROR(L"Raytracing shader config specifies MaxAttributeSizeInBytes of " << pConfig->MaxAttributeSizeInBytes <<
         L" but function this config is associated with, " <<
             PrettyPrintPossiblyMangledName(ex.m_MangledName) << 
            L", has a larger attribute size: " << pFuncInfo->AttributeSizeInBytes << L" bytes.");
#else
        LOG_ERROR_NOMESSAGE;
#endif   
        }
        if(pFuncInfo->PayloadSizeInBytes > pConfig->MaxPayloadSizeInBytes)
        {
#ifdef INCLUDE_MESSAGE_LOG            
        auto unmangledName = m_ExportNameMangledToUnmangled.find(ex.m_MangledName);
        LOG_ERROR(L"Raytracing shader config specifies MaxPayloadSizeInBytes of " << pConfig->MaxPayloadSizeInBytes <<
         L" but function this config is associated with, " <<
            PrettyPrintPossiblyMangledName(ex.m_MangledName) << 
            L", has a larger payload size: " << pFuncInfo->PayloadSizeInBytes << L" bytes.");
#else
        LOG_ERROR_NOMESSAGE;
#endif   
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::ValidateShaderFeatures
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::ValidateShaderFeatures()
{
    for(auto& ex : m_ExportInfoList)
    {
        //static const UINT MAJOR_VERSION_MASK  0x000000f0
        //static const UINT MAJOR_VERSION_SHIFT 4
        //static const UINT MINOR_VERSION_MASK  0x0000000f
        if((D3D_SHADER_MODEL)(ex.m_pFunctionInfo->MinShaderTarget & 0xff) > m_ShaderModel)
        {
#ifdef INCLUDE_MESSAGE_LOG            
            auto unmangledName = m_ExportNameMangledToUnmangled.find(ex.m_MangledName);
            LOG_ERROR(L"Export " << PrettyPrintPossiblyMangledName(ex.m_MangledName) << 
            L" expects shader model (D3D_SHADER_MODEL enum value) " << ex.m_pFunctionInfo->MinShaderTarget << 
            L" but device supports D3D_SHADER_MODEL " << m_ShaderModel << L".");
#else
        LOG_ERROR_NOMESSAGE;
#endif   
        }
        UINT64 FeaturesNeeded = ((UINT64)ex.m_pFunctionInfo->FeatureInfo1) | (((UINT64)ex.m_pFunctionInfo->FeatureInfo2)<<32);
        if(FeaturesNeeded & ~m_ShaderFeaturesSupported )
        {
#ifdef INCLUDE_MESSAGE_LOG            
            auto unmangledName = m_ExportNameMangledToUnmangled.find(ex.m_MangledName);
            LOG_ERROR(L"Export " << PrettyPrintPossiblyMangledName(ex.m_MangledName) << 
            L" uses shader feature(s) not supported by the device.");
#else
        LOG_ERROR_NOMESSAGE;
#endif               
        }
        TraverseFunctionsShaderStageValidation(ex.m_MangledName);
    }
    m_TraversalGlobals.GraphTraversalIndex++;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::TraverseFunctionsShaderStageValidation
//----------------------------------------------------------------------------------------------------------------------------------
UINT CStateObjectInfo::TraverseFunctionsShaderStageValidation(LPCWSTR function)
{
    auto ex = m_ExportInfoMap.find(LocalUniqueCopy(function));
    if(ex == m_ExportInfoMap.end())
    {
        return 0; // ignore unresolved exports
    }    
    auto pFuncInfo = ex->second->m_pFunctionInfo;
    auto& flags = ex->second->m_GraphTraversalFlags;
    if(ex->second->m_VisitedOnGraphTraversalIndex == m_TraversalGlobals.GraphTraversalIndex)
    {
        return ex->second->m_SubtreeValidShaderStageFlag;
    }
    ex->second->m_VisitedOnGraphTraversalIndex = m_TraversalGlobals.GraphTraversalIndex;  
    ex->second->m_SubtreeValidShaderStageFlag |= pFuncInfo->ShaderStageFlag | 0xffffffff; // TODO: remove 0xfffffff when DXC supports this
    if(flags & CExportInfo::GTF_CycleFound)
    {
        return ex->second->m_SubtreeValidShaderStageFlag; // skip graph cycles 
    }
    for(UINT i = 0; i < pFuncInfo->NumFunctionDependencies; i++)
    {
        ex->second->m_SubtreeValidShaderStageFlag |= TraverseFunctionsShaderStageValidation(pFuncInfo->FunctionDependencies[i]);
    }
    switch((ShaderKind)pFuncInfo->ShaderKind)
    {
    case ShaderKind::Library:
        break;
    default:
        if(!((1<<pFuncInfo->ShaderKind) & ex->second->m_SubtreeValidShaderStageFlag))
        {
#ifdef INCLUDE_MESSAGE_LOG            
            auto unmangledName = m_ExportNameMangledToUnmangled.find(LocalUniqueCopy(function));
            LOG_ERROR(ShaderStageName((ShaderKind)pFuncInfo->ShaderKind) << " shader named " <<
                PrettyPrintPossiblyMangledName(function) << 
                L" calls library function(s) where somewhere in the call graph features are used which are not compatible with this shader stage." );
#else
            LOG_ERROR_NOMESSAGE;
#endif   
        }
    }
    return ex->second->m_SubtreeValidShaderStageFlag;    
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::FillExportedFunction
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::FillExportedFunction(EXPORTED_FUNCTION* pEF, const CExportInfo* pEI)
{
    pEF->MangledName = pEI->m_MangledName;
    pEF->UnmangledName = pEI->m_UnmangledName;
    pEF->pDXILFunction = pEI->m_pFunctionInfo;
    pEF->bUnresolvedAssociations = pEI->m_bUnresolvedAssociations;
    pEF->bUnresolvedFunctions = pEI->m_bUnresolvedFunctions;
#ifndef SKIP_BINDING_VALIDATION
    pEF->bUnresolvedResourceBindings = pEI->m_bUnresolvedResourceBindings;
#endif
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::AddCollection
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::AddCollection(const D3D12_EXISTING_COLLECTION_DESC* pCollection)
{
    if(!pCollection || !pCollection->pExistingCollection)
    {
        LOG_ERROR(L"NULL specified for D3D12_EXISTING_COLLECTION_DESC or pExistingCollection.");
        return;
    }
    CStateObjectInfo* pColInfo = m_pfnGetStateObjectInfoForExistingCollection(pCollection->pExistingCollection);
    if (!pColInfo)
    {
        LOG_ERROR(L"Failed to extract collection information from D3D12_EXISTING_COLLECTION_DESC.pExistingCollection: " 
            << pCollection->pExistingCollection);
        return;
    }
    m_ExistingCollectionList.emplace_back();
    auto& wrappedCollection = m_ExistingCollectionList.back();
    wrappedCollection.Init(pCollection);

    // Don't keep any references to pCollection, pColInfo or their contents -> app memory
    // Exception is DXILLibraryReflection, which is a shared pointer (dependency taken below)
    for(auto lib : pColInfo->m_DXILLibraryList) // copy shared pointer references
    {
        m_DXILLibraryList.push_back(lib);
    }

    // No manual export list, export everything
    if (0 == pCollection->NumExports)
    {
        for (auto exportInfo : pColInfo->m_ExportInfoMap)
        {
            auto exportNameUnmangled = pColInfo->m_ExportNameMangledToUnmangled.find(exportInfo.first);
            assert(exportNameUnmangled != pColInfo->m_ExportNameMangledToUnmangled.end());
            AddExport(
                exportInfo.first, 
                exportNameUnmangled->second, 
                exportInfo.second->m_pFunctionInfo,
                exportInfo.second->m_pOwningStateObject,
                pColInfo->AllowExternalDependenciesOnLocalDefinitions());
        }
        for(auto& hitGroup : pColInfo->m_HitGroupList)
        {
            AddHitGroup(&hitGroup,hitGroup.m_pOwningStateObject);
        }
        for(ASSOCIATEABLE_SUBOBJECT_NAME i = (ASSOCIATEABLE_SUBOBJECT_NAME)0; i < NUM_ASSOCIATEABLE_SUBOBJECT_TYPES; ((UINT&)i)++)
        {
            for(auto& association : pColInfo->m_SubobjectToExportsAssociations[i])
            {
                if(association.m_Exports.size() == 0)
                {
                    continue; // skip default associations as they don't have affect outside their scope
                }
                CWrappedAssociation* pNewOrExistingAssociation = nullptr;
                for(auto& existing : m_SubobjectToExportsAssociations[i])
                {
                    if(association.m_pSubobject->Compare(existing.m_pSubobject))
                    {
                        // found a matching existing association
                        pNewOrExistingAssociation = &existing;
                    }
                }
                if(!pNewOrExistingAssociation)
                {
                    m_SubobjectToExportsAssociations[i].emplace_back();
                    pNewOrExistingAssociation = &m_SubobjectToExportsAssociations[i].back();
                    pNewOrExistingAssociation->m_pSubobject = TrackAssociateableSubobject(
                        association.m_SubobjectType,association.m_pSubobject->m_LocalSubobjectDefinition.pDesc,nullptr);
                    pNewOrExistingAssociation->m_SubobjectType = association.m_SubobjectType;

                }
                for(auto ex : association.m_Exports)
                {
                    pNewOrExistingAssociation->m_Exports.insert(LocalUniqueCopy(ex));
                }
            }            
        }
    }
    else
    {
        // Manual export list

        // Multimap of internal export names to external export(s) the library desc manually listed (if any)
        std::unordered_multimap<std::wstring, const D3D12_EXPORT_DESC*> ExportsToUse;
        std::unordered_set<const D3D12_EXPORT_DESC*> ExportMissing;

        for (UINT i = 0; i < pCollection->NumExports; i++)
        {
            LPCWSTR InternalName = pCollection->pExports[i].ExportToRename ? pCollection->pExports[i].ExportToRename : pCollection->pExports[i].Name;
            ExportsToUse.insert({ InternalName,&pCollection->pExports[i] });
            ExportMissing.insert(&pCollection->pExports[i]);
        }

        // Multimap of subobjects that were associated -> the exports associated to each one
        // The subobjects (CWrappedAssociation*) are from the incoming collection, after the map is 
        // completely generated, need to make a local CWrappedAssociation for each one
        std::unordered_multimap<const CWrappedAssociation*,LPCWSTR> ReferencedAssociations[NUM_ASSOCIATEABLE_SUBOBJECT_TYPES];
        std::unordered_set<const CWrappedAssociation*> UniqueReferencedAssociations[NUM_ASSOCIATEABLE_SUBOBJECT_TYPES];
        std::unordered_map<const CWrappedAssociation*,CWrappedAssociation*> OldToNewWrappedAssociations;
        std::unordered_set<LPCWSTR> OldMangledNamesExported;
        std::unordered_set<LPCWSTR> OldMangledNamesRenamed;
        // Examine functions
        {
            auto AddExportWrapper = [&ReferencedAssociations,&UniqueReferencedAssociations,&OldMangledNamesExported,&pColInfo,this]
                (LPCWSTR pExternalNameMangled, LPCWSTR pExternalNameUnmangled, LPCWSTR pOldMangledName, const CExportInfo* pColExportInfo)
            {
                for(ASSOCIATEABLE_SUBOBJECT_NAME i = (ASSOCIATEABLE_SUBOBJECT_NAME)0; i < NUM_ASSOCIATEABLE_SUBOBJECT_TYPES; ((UINT&)i)++)
                {
                    for(auto a : pColExportInfo->m_Associations[i])
                    {
                        ReferencedAssociations[i].insert({a,LocalUniqueCopy(pExternalNameMangled)});
                        UniqueReferencedAssociations[i].insert(a);
                    }
                }
                OldMangledNamesExported.insert(pOldMangledName);
                AddExport(
                    pExternalNameMangled,
                    pExternalNameUnmangled,
                    pColExportInfo->m_pFunctionInfo,
                    pColExportInfo->m_pOwningStateObject,
                    pColInfo->AllowExternalDependenciesOnLocalDefinitions());
            };

            for (UINT i = 0; i < pCollection->NumExports; i++)
            {
                LPCWSTR InternalName = pCollection->pExports[i].ExportToRename ? pCollection->pExports[i].ExportToRename : pCollection->pExports[i].Name;
                auto matchesUnmangled = pColInfo->m_ExportNameUnmangledToMangled.equal_range(pColInfo->LocalUniqueCopy(InternalName));
                // cases: (1) ExportToRename is an unmangled name, Name is unmangled
                //        (2) ExportToRename is a mangled name, Name is unmangled
                //        (3) ExportToRename is null, Name is unmangled
                //        (4) ExportToRename is null, Name is mangled
                // AddExport will throw if the same external name is exported multiple times from the state object.
                if (matchesUnmangled.first != pColInfo->m_ExportNameUnmangledToMangled.end())
                {
                    for (auto matchUnmangledToMangled = matchesUnmangled.first; matchUnmangledToMangled != matchesUnmangled.second; matchUnmangledToMangled++)
                    {
                        auto matchMangledExportInfo = pColInfo->m_ExportInfoMap.find(matchUnmangledToMangled->second);
                        assert(matchMangledExportInfo != pColInfo->m_ExportInfoMap.end());

                        if (pCollection->pExports[i].ExportToRename)
                        {
                            // (1) - do a rename
                            AddExportWrapper(RenameMangledName(matchUnmangledToMangled->second, InternalName, pCollection->pExports[i].Name),
                                pCollection->pExports[i].Name, matchUnmangledToMangled->second, matchMangledExportInfo->second);
                            OldMangledNamesRenamed.insert(matchUnmangledToMangled->second);
                        }
                        else
                        {
                            // (3) - no rename
                            AddExportWrapper(matchUnmangledToMangled->second, InternalName, matchUnmangledToMangled->second, matchMangledExportInfo->second);
                        }
                        ExportMissing.erase(&pCollection->pExports[i]);
                    }
                }
                else
                {
                    auto matchMangledExportInfo = pColInfo->m_ExportInfoMap.find(pColInfo->LocalUniqueCopy(InternalName));
                    if (matchMangledExportInfo != pColInfo->m_ExportInfoMap.end())
                    {
                        if (pCollection->pExports[i].ExportToRename)
                        {
                            // (2) - do a rename
                            auto mangledOriginalName = pColInfo->LocalUniqueCopy(pCollection->pExports[i].ExportToRename);
                            auto unmangledOriginalExportName = pColInfo->m_ExportNameMangledToUnmangled.find(mangledOriginalName);
                            assert(unmangledOriginalExportName != pColInfo->m_ExportNameMangledToUnmangled.end());
                            AddExportWrapper(RenameMangledName(pCollection->pExports[i].ExportToRename, unmangledOriginalExportName->second, pCollection->pExports[i].Name),
                                pCollection->pExports[i].Name, mangledOriginalName, matchMangledExportInfo->second);
                            OldMangledNamesRenamed.insert(mangledOriginalName);
                        }
                        else
                        {
                            // (4) - no rename
                            auto mangledOriginalName = pColInfo->LocalUniqueCopy(pCollection->pExports[i].Name);
                            auto unmangledOriginalExportName = pColInfo->m_ExportNameMangledToUnmangled.find(mangledOriginalName);
                            assert(unmangledOriginalExportName != pColInfo->m_ExportNameMangledToUnmangled.end());
                            AddExportWrapper(pCollection->pExports[i].Name, unmangledOriginalExportName->second, mangledOriginalName, matchMangledExportInfo->second);
                        }
                        ExportMissing.erase(&pCollection->pExports[i]);
                    }
                }
            }
        }
        
        // Try hit groups
        {
            std::unordered_set<const D3D12_EXPORT_DESC*> HitGroupFound;
            for(auto& ex : ExportMissing)
            {
                LPCWSTR InternalName = ex->ExportToRename ? ex->ExportToRename : ex->Name;
                auto match = pColInfo->m_HitGroups.find(pColInfo->LocalUniqueCopy(InternalName));
                if(match != pColInfo->m_HitGroups.end())
                {
                    D3D12_HIT_GROUP_DESC newHgDesc = *match->second;
                    newHgDesc.HitGroupExport = ex->Name;
                    // See if the constituent shaders were renamed
                    const CWrappedHitGroup& hgDesc = *match->second;
                    for (UINT s = 0; s < 3; s++)
                    {
                        LPCWSTR pDependency = nullptr;
                        LPCWSTR DependencyType = nullptr;
                        switch (s)
                        {
                        case 0:
                            pDependency = hgDesc.AnyHitShaderImport;
                            DependencyType = L"AnyHitShaderImport";                      
                            break;
                        case 1:
                            pDependency = hgDesc.ClosestHitShaderImport;
                            DependencyType = L"ClosestHitShaderImport";
                            break;
                        case 2:
                            pDependency = hgDesc.IntersectionShaderImport;
                            DependencyType = L"IntersectionShaderImport";
                            break;
                        default:
                            assert(false);
                            break;
                        }
                        if (!pDependency)
                        {
                            continue;
                        }
                        // Was this dependency renamed?
                        LPCWSTR pName = nullptr;
                        auto matches = ExportsToUse.equal_range(pDependency);

                        size_t count = ExportsToUse.count(pDependency);
                        switch (count)
                        {
                        case 0:
                            // Currently an unresolved export, ok.
                            pName = pDependency;
                            break;
                        case 1:
                        {
                            auto match = ExportsToUse.find(pDependency);
                            pName = match->second->Name;
                            break;
                        }
                        default:
                            LOG_ERROR(L"HitGroupExport \"" << hgDesc.HitGroupExport <<
                                L"\" imports " << DependencyType << L" named " << PrettyPrintPossiblyMangledName(pDependency) <<
                                L" which is being renamed as part of collection creation to multiple export names. "
                                L"so it is ambiguous which rename applies to " << PrettyPrintPossiblyMangledName(pDependency) << L".");
                            break;
                        }
                        if (pName)
                        {
                            switch (s)
                            {
                            case 0:
                                newHgDesc.AnyHitShaderImport = pName;
                                break;
                            case 1:
                                newHgDesc.ClosestHitShaderImport = pName;
                                break;
                            case 2:
                                newHgDesc.IntersectionShaderImport = pName;
                                break;
                            default:
                                assert(false);
                                break;
                            }                        
                        }
                    }
                    for(ASSOCIATEABLE_SUBOBJECT_NAME i = (ASSOCIATEABLE_SUBOBJECT_NAME)0; i < NUM_ASSOCIATEABLE_SUBOBJECT_TYPES; ((UINT&)i)++)
                    {
                        for(auto a : match->second->m_Associations[i])
                        {
                            ReferencedAssociations[i].insert({a,LocalUniqueCopy(newHgDesc.HitGroupExport)});
                            UniqueReferencedAssociations[i].insert(a);
                        }
                    }
                    OldMangledNamesExported.insert(match->first);
                    if(ex->ExportToRename)
                    {
                        OldMangledNamesRenamed.insert(match->first);
                    }
                    AddHitGroup(&newHgDesc,hgDesc.m_pOwningStateObject);
                    HitGroupFound.insert(ex);
                } // else unresolved reference so far, ok                
            }
            
            for(auto& found : HitGroupFound)
            {
                ExportMissing.erase(found);
            }
        }

        // Add associations
        for(ASSOCIATEABLE_SUBOBJECT_NAME i = (ASSOCIATEABLE_SUBOBJECT_NAME)0; i < NUM_ASSOCIATEABLE_SUBOBJECT_TYPES; ((UINT&)i)++)
        {
            for(auto& association : pColInfo->m_SubobjectToExportsAssociations[i])
            {
                if(association.m_Exports.size() == 0)
                {
                    continue; // skip default associations as they don't have affect outside their scope
                }
                CWrappedAssociation* pNewOrExistingAssociation = nullptr;
                for(auto& existing : m_SubobjectToExportsAssociations[i])
                {
                    if(association.m_pSubobject->Compare(existing.m_pSubobject))
                    {
                        // found a matching existing association
                        pNewOrExistingAssociation = &existing;
                    }
                }
                if(!pNewOrExistingAssociation)
                {
                    m_SubobjectToExportsAssociations[i].emplace_back();
                    pNewOrExistingAssociation = &m_SubobjectToExportsAssociations[i].back();
                    pNewOrExistingAssociation->m_pSubobject = TrackAssociateableSubobject(
                        association.m_SubobjectType,association.m_pSubobject->m_LocalSubobjectDefinition.pDesc,nullptr);
                    pNewOrExistingAssociation->m_SubobjectType = association.m_SubobjectType;

                }
                for(auto ex : association.m_Exports)
                {
                    // Export only if it wasn't in the collection's incoming export list and renamed,
                    // and it isn't an export that was resolved in the old collection but not now.
                    // The destination of these renames handled by the UniqueReferencedAssociations 
                    // processing further below.
                    bool bOldNameNoLongerExists = 
                            ((pColInfo->m_ExportInfoMap.find(ex) != pColInfo->m_ExportInfoMap.end()) &&
                             (OldMangledNamesExported.find(ex) == OldMangledNamesExported.end()));
                    bool bOldNameWasRenamed = (OldMangledNamesRenamed.find(ex) != OldMangledNamesRenamed.end());
                    if(!(bOldNameNoLongerExists || bOldNameWasRenamed))
                    {
                        pNewOrExistingAssociation->m_Exports.insert(LocalUniqueCopy(ex));
                    }
                }
                OldToNewWrappedAssociations.insert({&association,pNewOrExistingAssociation});
            }

            for(auto association : UniqueReferencedAssociations[i])
            {
                // Make a new wrapped association for each unique referenced wrapped association in collection
                auto match = OldToNewWrappedAssociations.find(association);
                CWrappedAssociation* pNewAssociation = nullptr;
                if(match == OldToNewWrappedAssociations.end()) // didn't get added above because it was an empty export list, explicit default association
                {
                    m_SubobjectToExportsAssociations[i].emplace_back();
                    pNewAssociation = &m_SubobjectToExportsAssociations[i].back();
                    pNewAssociation->m_pSubobject = TrackAssociateableSubobject(
                        association->m_SubobjectType,association->m_pSubobject->m_LocalSubobjectDefinition.pDesc,nullptr);
                    pNewAssociation->m_SubobjectType = association->m_SubobjectType;
                }
                else
                {
                    pNewAssociation = match->second;
                }

                // For each export referencing this association, point it to the new local wrapper
                auto refs = ReferencedAssociations[i].equal_range(association);
                assert(refs.first != ReferencedAssociations[i].end());
                for(auto ref = refs.first; ref != refs.second; ref++)
                {
                    pNewAssociation->m_Exports.insert(ref->second);
                }
            }
        }      

#ifdef INCLUDE_MESSAGE_LOG
        for(auto& ex : ExportMissing)
        {
            LPCWSTR InternalName = ex->ExportToRename ? ex->ExportToRename : ex->Name;
            size_t i = (ex - &pCollection->pExports[0]);
            LOG_ERROR(L"Manually listed export [" << i << L"], " << PrettyPrintPossiblyMangledName(InternalName) << L", doesn't exist in collection " << pCollection->pExistingCollection << L".");
        }
#else
        if(ExportMissing.size())
        {
            LOG_ERROR_NOMESSAGE;
        }
#endif 
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::AddHitGroup
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::AddHitGroup(const D3D12_HIT_GROUP_DESC* pHitGroup, CStateObjectInfo* pOwningStateObject)
{
    // Don't keep any references to pHitGroup or it's contents -> app memory
    CWrappedHitGroup desc;
    desc.m_pOwningStateObject = pOwningStateObject;
    desc.HitGroupExport = LocalUniqueCopy(pHitGroup->HitGroupExport);
    desc.Type = pHitGroup->Type;
    switch(pHitGroup->Type)
    {
    case D3D12_HIT_GROUP_TYPE_TRIANGLES:
        if(pHitGroup->IntersectionShaderImport)
        {
            LOG_ERROR(L"Hit groups of type TRIANGLES cannot have IntersectionShaderImport defined.");                
        }
        break;
    case D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE:
        if(!pHitGroup->IntersectionShaderImport)
        {
            LOG_ERROR(L"Hit groups of type PROCEDURAL_PRIMITIVE must have IntersectionShaderImport defined.");                
        }
        break;
    default:
        LOG_ERROR(L"Unrecognized hit group type: " << desc.Type << L".");    
        break;
    }
    desc.AnyHitShaderImport = LocalUniqueCopy(pHitGroup->AnyHitShaderImport);
    desc.ClosestHitShaderImport = LocalUniqueCopy(pHitGroup->ClosestHitShaderImport);
    desc.IntersectionShaderImport = LocalUniqueCopy(pHitGroup->IntersectionShaderImport);
    auto exists = m_HitGroups.find(desc.HitGroupExport);
    if (exists == m_HitGroups.end())
    {
        m_HitGroupList.emplace_back(desc);
        auto& newHg = m_HitGroupList.back();
        newHg.m_LocalSubobjectDefinition.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
        newHg.m_LocalSubobjectDefinition.pDesc = &newHg;
        m_HitGroups.insert({ desc.HitGroupExport,&newHg });
    }
    else
    { 
        LOG_ERROR(L"Hit group \"" << desc.HitGroupExport << L"\" already defined.");
    }
    if((m_UsedUnmangledFunctionNames.find(desc.HitGroupExport) != m_UsedUnmangledFunctionNames.end()) ||
       (m_UsedNonFunctionNames.find(desc.HitGroupExport) != m_UsedNonFunctionNames.end()))
    {
        LOG_ERROR(L"Hit group name \"" << desc.HitGroupExport << L"\" already used used by another non hit group export.");
    }
    m_UsedNonFunctionNames.insert(desc.HitGroupExport);
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::TrackAssociateableSubobject
//----------------------------------------------------------------------------------------------------------------------------------
CStateObjectInfo::CAssociateableSubobjectInfo* CStateObjectInfo::TrackAssociateableSubobject(
    D3D12_STATE_SUBOBJECT_TYPE SubobjectType,
    const void* pStateSubobjectDesc,
    const void* pEnclosingSubobject)
{
    CAssociateableSubobjectInfo* pBaseWrapper = nullptr;
    #pragma push_macro("COPYDESC")
    #define CREATE_AND_PARTIALLY_INIT_WRAPPER(D3D12_DESC_STRUCT) \
            pBaseWrapper = new CSimpleAssociateableSubobjectWrapper<D3D12_DESC_STRUCT>; \
            pBaseWrapper->m_LocalSubobjectDefinition.Type = SubobjectType; \
            *(D3D12_DESC_STRUCT*)(CSimpleAssociateableSubobjectWrapper<D3D12_DESC_STRUCT>*)pBaseWrapper = *(D3D12_DESC_STRUCT*)pStateSubobjectDesc; \
            pBaseWrapper->m_LocalSubobjectDefinition.pDesc = (D3D12_DESC_STRUCT*)(CSimpleAssociateableSubobjectWrapper<D3D12_DESC_STRUCT>*)pBaseWrapper;

    switch(SubobjectType)
    {
    case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG:
    {
        CREATE_AND_PARTIALLY_INIT_WRAPPER(D3D12_RAYTRACING_SHADER_CONFIG);
        break;
    }
    case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG:
    {
        CREATE_AND_PARTIALLY_INIT_WRAPPER(D3D12_RAYTRACING_PIPELINE_CONFIG);
        break;
    }
    case D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE:
    {
        CREATE_AND_PARTIALLY_INIT_WRAPPER(D3D12_GLOBAL_ROOT_SIGNATURE);
        break;
    }
    case D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE:
    {
        CREATE_AND_PARTIALLY_INIT_WRAPPER(D3D12_LOCAL_ROOT_SIGNATURE);
        break;
    }
    case D3D12_STATE_SUBOBJECT_TYPE_STATE_OBJECT_CONFIG:
    {
        CREATE_AND_PARTIALLY_INIT_WRAPPER(D3D12_STATE_OBJECT_CONFIG);
        break;
    }
    case D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK:
    {
        CREATE_AND_PARTIALLY_INIT_WRAPPER(D3D12_NODE_MASK);
        break;        
    }
    default:
        assert(false);
    }
    #pragma pop_macro("COPYDESC")
    auto Name = AssociateableSubobjectName(SubobjectType);
    assert(ASN_INVALID != Name);
    auto& subobjectData = m_AssociateableSubobjectData[Name];

    if(pEnclosingSubobject)
    {
        subobjectData.Associateable.push_back(pBaseWrapper);
        m_EnclosingSubobjectPointerToInternal.insert({pEnclosingSubobject,pBaseWrapper});
    }
    else
    {
        subobjectData.InheritedSoNotAssociateable.push_back(pBaseWrapper);
    }
    return pBaseWrapper;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::AddRaytracingShaderConfig
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::AddRaytracingShaderConfig(
    const D3D12_RAYTRACING_SHADER_CONFIG* pShaderConfig, 
    const void* pEnclosingSubobject)
{
    if(!pShaderConfig)
    {
        LOG_ERROR(L"Null D3D12_RAYTRACING_SHADER_CONFIG specified.");
        return;
    }
    if(pShaderConfig->MaxAttributeSizeInBytes > D3D12_RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES)
    {
        LOG_ERROR(L"D3D12_RAYTRACING_SHADER_CONFIG.MaxAttributeSizeInBytes cannot be larger than D3D12_RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES ("
        << D3D12_RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES << L"). " << pShaderConfig->MaxAttributeSizeInBytes << L" specified.");
        return;
    }

    TrackAssociateableSubobject(D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG,pShaderConfig,pEnclosingSubobject);
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::AddRaytracingPipelineConfig
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::AddRaytracingPipelineConfig(
    const D3D12_RAYTRACING_PIPELINE_CONFIG* pPipelineConfig, 
    const void* pEnclosingSubobject)
{
    if(!pPipelineConfig)
    {
        LOG_ERROR(L"Null D3D12_RAYTRACING_SHADER_CONFIG specified in D3D12_STATE_SUBOBJECT " << pEnclosingSubobject);
        return;
    }

    TrackAssociateableSubobject(D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG,pPipelineConfig,pEnclosingSubobject);    
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::AddGlobalRootSignature
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::AddGlobalRootSignature(
    const D3D12_GLOBAL_ROOT_SIGNATURE* pGlobalRS, 
    const void* pEnclosingSubobject)
{
    if(!pGlobalRS)
    {
        LOG_ERROR(L"Null D3D12_GLOBAL_ROOT_SIGNATURE specified in D3D12_STATE_SUBOBJECT " << pEnclosingSubobject <<L" (pGlobalRootSignature member can be null if desired).");
        return;
    }
#ifndef SKIP_BINDING_VALIDATION
    if(pGlobalRS->pGlobalRootSignature)
    {
        CComPtr<ID3D12VersionedRootSignatureDeserializer> pDeserialized = m_pfnGetRootSignatureDeserializer(pGlobalRS->pGlobalRootSignature);
        const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pDescGlobal;
        HRESULT hr = pDeserialized->GetRootSignatureDescAtVersion(D3D_ROOT_SIGNATURE_VERSION_1_1,&pDescGlobal);
        if(FAILED(hr))
        {
            LOG_ERROR(L"Failed to parse global root signature 0x" << pGlobalRS->pGlobalRootSignature );
            return;
        }
        if(pDescGlobal->Desc_1_1.Flags & D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE)
        {
            LOG_ERROR(L"Root signature passed in, 0x" << pGlobalRS->pGlobalRootSignature << L", to a global root signature subject has root signature flag " <<
            L"D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE set, meaning it is a local root signature, not a global root signature." );
            return;            
        }
    }
#endif

    TrackAssociateableSubobject(D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE,pGlobalRS,pEnclosingSubobject);    
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::AddLocalRootSignature
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::AddLocalRootSignature(
    const D3D12_LOCAL_ROOT_SIGNATURE* pLocalRS, 
    const void* pEnclosingSubobject)
{
    if(!pLocalRS)
    {
        LOG_ERROR(L"Null D3D12_LOCAL_ROOT_SIGNATURE specified in D3D12_STATE_SUBOBJECT " << pEnclosingSubobject <<L" (pLocalRootSignature member can be null if desired).");
        return;
    }
#ifndef SKIP_BINDING_VALIDATION
    if(pLocalRS->pLocalRootSignature)
    {
        CComPtr<ID3D12VersionedRootSignatureDeserializer> pDeserialized = m_pfnGetRootSignatureDeserializer(pLocalRS->pLocalRootSignature);
        const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pDescLocal;
        HRESULT hr = pDeserialized->GetRootSignatureDescAtVersion(D3D_ROOT_SIGNATURE_VERSION_1_1,&pDescLocal);
        if(FAILED(hr))
        {
            LOG_ERROR(L"Failed to parse local root signature 0x" << pLocalRS->pLocalRootSignature );
            return;
        }
        if(!(pDescLocal->Desc_1_1.Flags & D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE))
        {
            LOG_ERROR(L"Root signature passed in, 0x" << pLocalRS->pLocalRootSignature << L", to a local root signature subject does not have root signature flag " <<
            L"D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE set." );
            return;            
        }
    }
#endif
    TrackAssociateableSubobject(D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE,pLocalRS,pEnclosingSubobject);    
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::AddNodeMask
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::AddNodeMask(const D3D12_NODE_MASK* pNodeMask, const void* pEnclosingSubobject)
{
    if(!pNodeMask)
    {
        LOG_ERROR(L"Null D3D12_NODE_MASK specified in D3D12_STATE_SUBOBJECT " << pEnclosingSubobject);
        return;
    }
    m_NodeMask = *pNodeMask; // later validation will ensure everything picks the same value
    auto Type = D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK;
    auto ASN = AssociateableSubobjectName(Type);
    m_GlobalSubobjectDefinitions[ASN].Type = Type;
    m_GlobalSubobjectDefinitions[ASN].pDesc = &m_NodeMask;
    TrackAssociateableSubobject(Type,pNodeMask,pEnclosingSubobject);        
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::AddStateObjectConfig
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::AddStateObjectConfig(
    const D3D12_STATE_OBJECT_CONFIG* pSOConfig, 
    const void* pEnclosingSubobject)
{
    if(!pSOConfig)
    {
        LOG_ERROR(L"Null D3D12_STATE_OBJECT_CONFIG specified in D3D12_STATE_SUBOBJECT " << pEnclosingSubobject);
        return;
    }
#ifndef SKIP_STATE_OBJECT_MASK_VALIDATION
    if(pSOConfig->Flags & ~D3D12_STATE_OBJECT_FLAG_VALID_MASK)
    {
        LOG_ERROR(L"Invalid D3D12_STATE_OBJECT_FLAGS: 0x" << pSOConfig->Flags);
        return;
    }
#endif
    m_StateObjectConfig = *pSOConfig;
    auto Type = D3D12_STATE_SUBOBJECT_TYPE_STATE_OBJECT_CONFIG;
    auto ASN = AssociateableSubobjectName(Type);
    m_GlobalSubobjectDefinitions[ASN].Type = Type;
    m_GlobalSubobjectDefinitions[ASN].pDesc = &m_StateObjectConfig;
    TrackAssociateableSubobject(Type,pSOConfig,pEnclosingSubobject);    
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::AddSubobjectToExportsAssociation
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::AddSubobjectToExportsAssociation(
    const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION* pAssociation)
{
    if(!pAssociation || !pAssociation->pSubobjectToAssociate)
    {
        LOG_ERROR(L"Null D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION or pSubobjectToAssociate specified.");
        return;        
    }
    auto Name = AssociateableSubobjectName(pAssociation->pSubobjectToAssociate->Type);
    if(ASN_INVALID == Name)
    {
        LOG_ERROR(L"D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION specifies a subobject which is of a type (D3D12_STATE_SUBOBJECT_TYPE enum value " <<
            pAssociation->pSubobjectToAssociate->Type << L") that it doesn't make sense to define associations for.");
        return;                
    }
    m_SubobjectToExportsAssociations[Name].emplace_back();
    CWrappedAssociation* pWrapped = &m_SubobjectToExportsAssociations[Name].back();
    pWrapped->m_pUntranslatedExternalSubobject = pAssociation->pSubobjectToAssociate; // will translate this pointer after all subobjects have been ingested
    pWrapped->m_SubobjectType = pAssociation->pSubobjectToAssociate->Type;
    pWrapped->m_pSubobject = nullptr;
    if(pAssociation->NumExports)
    {
        for(UINT i = 0; i < pAssociation->NumExports; i++)
        {
            pWrapped->m_Exports.insert(LocalUniqueCopy(pAssociation->pExports[i]));
        }
    }
    else // explicit default
    {
        if(m_AssociateableSubobjectData[Name].pUsedInExplicitDefaultAssociation)
        {
            LOG_ERROR(L"D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION specifies a subobject of type " <<
                m_sAssociateableSubobjectData[Name].StringAPIName << L" and an empty export list, meaning this is to be treated "
                L"as an explicit default association.  That would be fine, except another subobject association of the same type with an empty export list "
                L"already exists.");
        }
        else
        {
            m_AssociateableSubobjectData[Name].pUsedInExplicitDefaultAssociation = pWrapped;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::GetLog
//----------------------------------------------------------------------------------------------------------------------------------
#ifdef INCLUDE_MESSAGE_LOG
const std::list<std::wstring>& CStateObjectInfo::GetLog()
{
    return m_Log;
}
#endif

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::Log
//----------------------------------------------------------------------------------------------------------------------------------
#ifdef INCLUDE_MESSAGE_LOG
void CStateObjectInfo::Log(std::wostringstream& message)
{
    m_Log.emplace_back(message.str());
}
#endif

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::LocalUniqueCopy
//----------------------------------------------------------------------------------------------------------------------------------
LPCWSTR CStateObjectInfo::LocalUniqueCopy(LPCWSTR string)
{
    return LocalUniqueCopy(string,m_StringContainer);
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::LocalUniqueCopy (with external container)
//----------------------------------------------------------------------------------------------------------------------------------
LPCWSTR CStateObjectInfo::LocalUniqueCopy(LPCWSTR string, std::unordered_set<std::wstring>& stringContainer)
{
    if (string == nullptr)
    {
        return nullptr;
    }
    auto result = stringContainer.insert(string);
    return result.first->c_str();
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::AllowLocalDependenciesOnExternalDefinitions
//----------------------------------------------------------------------------------------------------------------------------------
bool CStateObjectInfo::AllowLocalDependenciesOnExternalDefinitions() const
{
    assert(m_bStateObjectTypeSelected);
    switch(m_SOType)
    {
    case D3D12_STATE_OBJECT_TYPE_COLLECTION:
        return (m_StateObjectConfig.Flags & D3D12_STATE_OBJECT_FLAG_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITIONS);
    }
    return false;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::AllowExternalDependenciesOnLocalDefinitions()
//----------------------------------------------------------------------------------------------------------------------------------
bool CStateObjectInfo::AllowExternalDependenciesOnLocalDefinitions() const
{
    assert(m_bStateObjectTypeSelected);
    switch(m_SOType)
    {
    case D3D12_STATE_OBJECT_TYPE_COLLECTION:
        return (m_StateObjectConfig.Flags & D3D12_STATE_OBJECT_FLAG_ALLOW_EXTERNAL_DEPENDENCIES_ON_LOCAL_DEFINITIONS);
    }
    return false;
}


//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::SupportedShaderType
//----------------------------------------------------------------------------------------------------------------------------------
bool CStateObjectInfo::SupportedShaderType(ShaderKind shader) const
{
    switch(shader)
    {
    case ShaderKind::RayGeneration:
    case ShaderKind::Intersection:
    case ShaderKind::AnyHit:
    case ShaderKind::ClosestHit:
    case ShaderKind::Miss:
    case ShaderKind::Callable:
    case ShaderKind::Library:
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::AssociateableSubobjectName
//----------------------------------------------------------------------------------------------------------------------------------
CStateObjectInfo::ASSOCIATEABLE_SUBOBJECT_NAME CStateObjectInfo::AssociateableSubobjectName(D3D12_STATE_SUBOBJECT_TYPE sub) const
{
    switch(sub)
    {
    case D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE: 
        return ASN_GLOBAL_ROOT_SIGNATURE;
    case D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE:
        return ASN_LOCAL_ROOT_SIGNATURE;
    case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG:
        return ASN_RAYTRACING_SHADER_CONFIG;
    case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG:
        return ASN_RAYTRACING_PIPELINE_CONFIG;
    case D3D12_STATE_SUBOBJECT_TYPE_STATE_OBJECT_CONFIG:
        return ASN_STATE_OBJECT_CONFIG;
    case D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK:
        return ASN_NODE_MASK;
    }
    return ASN_INVALID;
}


//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::GetHitGroupDependencyTypeName
//----------------------------------------------------------------------------------------------------------------------------------
LPCWSTR CStateObjectInfo::GetHitGroupDependencyTypeName(UINT i) const
{
    LPCWSTR pDependencyType = L"";
    switch(i)
    {
    case 0:
        pDependencyType = L"AnyHitShaderImport";
        break;
    case 1:
        pDependencyType = L"ClosestHitShaderImport";
        break;
    case 2:
        pDependencyType = L"IntersectionShaderImport";
        break;
    default:
        assert(false);
        break;
    }
    return pDependencyType;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::ASSOCIATEABLE_SUBOBJECT_DATA::~ASSOCIATEABLE_SUBOBJECT_DATA
//----------------------------------------------------------------------------------------------------------------------------------
CStateObjectInfo::ASSOCIATEABLE_SUBOBJECT_DATA::~ASSOCIATEABLE_SUBOBJECT_DATA()
{
    for(auto a : Associateable) { delete a; }
    for(auto i : InheritedSoNotAssociateable) { delete i; }
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::PrepareForInformationReflection
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::PrepareForInformationReflection()
{
    if(!m_bStateObjectSet || m_bFoundError)
    {
        return;
    }
    m_ExportArrayForReflection.resize(m_ExportInfoList.size());
    size_t index = 0;
    for(auto& ex : m_ExportInfoList)
    {
        m_ExportArrayForReflection[index++] = &ex;
    }
    index = 0;
    m_HitGroupArrayForReflection.resize(m_HitGroupList.size());
    for(auto& hg : m_HitGroupList)
    {
        m_HitGroupArrayForReflection[index++] = &hg;
    }
    // Count referenced associateable subobjects
    size_t NumReferencedAssociateableSubobjects = 0;
    for(ASSOCIATEABLE_SUBOBJECT_NAME i = (ASSOCIATEABLE_SUBOBJECT_NAME)0; i < NUM_ASSOCIATEABLE_SUBOBJECT_TYPES; ((UINT&)i)++)
    {
        for( auto subobject : m_AssociateableSubobjectData[i].Associateable )
        {
            if(subobject->m_bReferenced)
            {
                NumReferencedAssociateableSubobjects++;
            }
        }
        for( auto subobject : m_AssociateableSubobjectData[i].InheritedSoNotAssociateable )
        {
            if(subobject->m_bReferenced)
            {
                NumReferencedAssociateableSubobjects++;
            }
        }
    }

    // Add exported subobjects to a single flat list
    m_ExportedSubobjectsForReflection.resize(
        m_DXILLibraryList.size() + m_ExistingCollectionList.size() + m_HitGroupArrayForReflection.size() + NumReferencedAssociateableSubobjects);
    index = 0;
    for(auto& lib : m_DXILLibraryList)
    {
        m_ExportedSubobjectsForReflection[index++] = &lib->m_LocalSubobjectDefinition;
    }
    for(auto& col : m_ExistingCollectionList)
    {
        m_ExportedSubobjectsForReflection[index++] = &col.m_LocalSubobjectDefinition;
    }
    for(auto& hg : m_HitGroupArrayForReflection)
    {
        m_ExportedSubobjectsForReflection[index++] = &hg->m_LocalSubobjectDefinition;
    }
    for(ASSOCIATEABLE_SUBOBJECT_NAME i = (ASSOCIATEABLE_SUBOBJECT_NAME)0; i < NUM_ASSOCIATEABLE_SUBOBJECT_TYPES; ((UINT&)i)++)
    {
        for( auto subobject : m_AssociateableSubobjectData[i].Associateable )
        {
            if(subobject->m_bReferenced)
            {
                m_ExportedSubobjectsForReflection[index++] = &subobject->m_LocalSubobjectDefinition;
                NumReferencedAssociateableSubobjects++;
            }
        }
        for( auto subobject : m_AssociateableSubobjectData[i].InheritedSoNotAssociateable )
        {
            if(subobject->m_bReferenced)
            {
                m_ExportedSubobjectsForReflection[index++] = &subobject->m_LocalSubobjectDefinition;
                NumReferencedAssociateableSubobjects++;
            }
        }
    }
    // Determine any globally associated subobjects (MatchScope_FullStateObject or MatchScope_LocalStateObject)
    // First, any subobjects of MatchRule_IfExistsMustExistAndMatchForAllExports, MatchScope_FullStateObject that have not been defined go to defaults:
    if(m_GlobalSubobjectDefinitions[ASN_NODE_MASK].pDesc == nullptr)
    {
        m_GlobalSubobjectDefinitions[ASN_NODE_MASK].Type = D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK;
        m_GlobalSubobjectDefinitions[ASN_NODE_MASK].pDesc = &m_NodeMask;
    }
    if(m_GlobalSubobjectDefinitions[ASN_STATE_OBJECT_CONFIG].pDesc == nullptr)
    {
        m_GlobalSubobjectDefinitions[ASN_STATE_OBJECT_CONFIG].Type = D3D12_STATE_SUBOBJECT_TYPE_STATE_OBJECT_CONFIG;
        m_GlobalSubobjectDefinitions[ASN_STATE_OBJECT_CONFIG].pDesc = &m_StateObjectConfig;
    }
    // Then look over all subobjects to determine any global associations
    for(ASSOCIATEABLE_SUBOBJECT_NAME i = (ASSOCIATEABLE_SUBOBJECT_NAME)0; i < NUM_ASSOCIATEABLE_SUBOBJECT_TYPES; ((UINT&)i)++)    
    {
        switch(m_sAssociateableSubobjectData[i].MatchScope)
        {
        case MatchScope_FullStateObject:
        case MatchScope_LocalStateObject:
            break;
        default:
            continue;
        }

        switch(m_sAssociateableSubobjectData[i].MatchRule)
        {
        case MatchRule_RequiredAndMatchingForAllExports:
        {
            auto pMatch = m_AssociateableSubobjectData[i].Associateable.size() 
                            ? m_AssociateableSubobjectData[i].Associateable.front() : nullptr;
            if(!pMatch)
            {
                pMatch = m_AssociateableSubobjectData[i].InheritedSoNotAssociateable.size() ? 
                            m_AssociateableSubobjectData[i].InheritedSoNotAssociateable.front() : nullptr;
            }
            if(pMatch)
            {
                m_GlobalSubobjectDefinitions[i] = pMatch->m_LocalSubobjectDefinition;
            }
            break;
        }
        case MatchRule_IfExistsMustExistAndMatchForAllExports:
            // These subobjects already update m_GlobalSubobjectDefinitions[i] as they are encountered
            break;
        default:
            assert(false);
            break;
        }
    }
    m_bPreparedForReflection = true;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::GetGloballyAssociatedSubobject
//----------------------------------------------------------------------------------------------------------------------------------
const D3D12_STATE_SUBOBJECT* CStateObjectInfo::GetGloballyAssociatedSubobject(D3D12_STATE_SUBOBJECT_TYPE Type)
{
    ASSOCIATEABLE_SUBOBJECT_NAME ASN = AssociateableSubobjectName(Type);
    if((ASN_INVALID == Type) || (!m_bPreparedForReflection))
    {
        return nullptr;
    }
    switch(m_sAssociateableSubobjectData[ASN].MatchScope)
    {
    case MatchScope_FullStateObject:
    case MatchScope_LocalStateObject:
        break;
    default:
        return nullptr;
    }
    switch(m_sAssociateableSubobjectData[ASN].MatchRule)
    {
    case MatchRule_RequiredAndMatchingForAllExports:
    case MatchRule_IfExistsMustExistAndMatchForAllExports:
        if(m_GlobalSubobjectDefinitions[ASN].pDesc)
        {
            return &m_GlobalSubobjectDefinitions[ASN];
        }
        break;
    default:
        assert(false);
        break;
    }
    return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CDxilLibraryIterator::CDxilLibraryIterator
//----------------------------------------------------------------------------------------------------------------------------------
CStateObjectInfo::CDxilLibraryIterator::CDxilLibraryIterator(CStateObjectInfo*pStateObjectInfo) : 
    m_pSOI(nullptr), m_Count(0)
{
    if (pStateObjectInfo && pStateObjectInfo->m_bPreparedForReflection)
    {
        m_pSOI = pStateObjectInfo;
        m_Count = m_pSOI->m_DXILLibraryList.size();
    }
    Reset();
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CExportedFunctionIteraor::GetCount
//----------------------------------------------------------------------------------------------------------------------------------
size_t CStateObjectInfo::CDxilLibraryIterator::GetCount()
{
    return m_Count;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CDxilLibraryIterator::Reset
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::CDxilLibraryIterator::Reset()
{
    if (m_pSOI)
    {
        m_libIterator = m_pSOI->m_DXILLibraryList.begin();
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CDxilLibraryIterator::Next
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::CDxilLibraryIterator::Next(D3D12_DXIL_LIBRARY_DESC *pOutDxilLibraryDesc)
{
    if (!m_pSOI || m_libIterator == m_pSOI->m_DXILLibraryList.end())
    {
        *pOutDxilLibraryDesc = {};
        return;
    }
    *pOutDxilLibraryDesc = (*m_libIterator)->m_LocalLibraryDesc;
    m_libIterator++;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CExportedFunctionIterator::CExportedFunctionIterator
//----------------------------------------------------------------------------------------------------------------------------------
CStateObjectInfo::CExportedFunctionIterator::CExportedFunctionIterator(CStateObjectInfo*pStateObjectInfo)
{
    if(pStateObjectInfo && pStateObjectInfo->m_bPreparedForReflection)
    {
        m_pSOI = pStateObjectInfo;
        m_Count = m_pSOI->m_ExportArrayForReflection.size();
    }
    else
    {
        m_pSOI = nullptr;
        m_Count = 0;
    }
    Reset();
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CExportedFunctionIterator::Reset
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::CExportedFunctionIterator::Reset()
{
    m_Index = 0;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CExportedFunctionIteraor::GetCount
//----------------------------------------------------------------------------------------------------------------------------------
size_t CStateObjectInfo::CExportedFunctionIterator::GetCount()
{
    return m_Count;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CExportedFunctionIteraor::Next
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::CExportedFunctionIterator::Next(EXPORTED_FUNCTION const* pOutExportedFunction)
{
    auto pOut = const_cast<EXPORTED_FUNCTION*>(pOutExportedFunction);
    if(m_Index >= m_Count)
    {
        *pOut = {};
        return;
    }
    auto ex = m_pSOI->m_ExportArrayForReflection[m_Index];
    m_pSOI->FillExportedFunction(pOut,ex);
    m_Index++;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CExportedHitGroupIterator::CExportedHitGroupIterator
//----------------------------------------------------------------------------------------------------------------------------------
CStateObjectInfo::CExportedHitGroupIterator::CExportedHitGroupIterator(CStateObjectInfo*pStateObjectInfo)
{
    if(pStateObjectInfo && pStateObjectInfo->m_bPreparedForReflection)
    {
        m_pSOI = pStateObjectInfo;
        m_Count = m_pSOI->m_HitGroupArrayForReflection.size();
    }
    else
    {
        m_pSOI = nullptr;
        m_Count = 0;
    }
    Reset();
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CExportedHitGroupIterator::Reset
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::CExportedHitGroupIterator::Reset()
{
    m_Index = 0;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CExportedHitGroupIterator::GetCount
//----------------------------------------------------------------------------------------------------------------------------------
size_t CStateObjectInfo::CExportedHitGroupIterator::GetCount()
{
    return m_Count;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CExportedHitGroupIterator::Next
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::CExportedHitGroupIterator::Next(EXPORTED_HIT_GROUP const* pOutExportedHitGroup)
{
    auto pOut = const_cast<EXPORTED_HIT_GROUP*>(pOutExportedHitGroup);
    if(m_Index >= m_Count)
    {
        *pOut = {};
        return;
    }
    auto hg = m_pSOI->m_HitGroupArrayForReflection[m_Index];
    ReflectHitGroup(hg,pOut,m_pSOI);
    m_Index++;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::LookupExportedHitGroup
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::LookupExportedHitGroup(LPCWSTR NameToLookup, EXPORTED_HIT_GROUP const* pOutExportedHitGroup)
{
    auto pOut = const_cast<EXPORTED_HIT_GROUP*>(pOutExportedHitGroup);
    auto Match = m_HitGroups.find(LocalUniqueCopy(NameToLookup));
    if(Match == m_HitGroups.end())
    {
        *pOut = {};
        return;
    }
    ReflectHitGroup(Match->second,pOut,this);
}

#ifndef SKIP_BINDING_VALIDATION
//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::EligibleForShaderIdentifier
//----------------------------------------------------------------------------------------------------------------------------------
bool CStateObjectInfo::EligibleForShaderIdentifier(LPCWSTR NameToLookup)
{
    EXPORTED_HIT_GROUP ehg;
    LookupExportedHitGroup(NameToLookup,&ehg);
    if(ehg.pHitGroup)
    {
        if(ehg.bUnresolvedFunctions || ehg.bUnresolvedResourceBindings || ehg.bUnresolvedAssociations)
        {
            return false;
        }
        return true;
    }
    CExportedFunctionLookup funcLookup(this);
    funcLookup.ResetAndSelectExport(NameToLookup);
    if(funcLookup.GetCount() != 1)
    {
        return false;
    }
    EXPORTED_FUNCTION ef;
    funcLookup.Next(&ef);
    if(ef.bUnresolvedFunctions || ef.bUnresolvedResourceBindings || ef.bUnresolvedAssociations)
    {
        return false;
    }
    switch((ShaderKind)ef.pDXILFunction->ShaderKind)
    {
    case ShaderKind::RayGeneration:
    case ShaderKind::Miss:
    case ShaderKind::Callable:
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::EligibleForStackSizeQuery
//----------------------------------------------------------------------------------------------------------------------------------
bool CStateObjectInfo::EligibleForStackSizeQuery(LPCWSTR NameToLookup)
{
    CExportedFunctionLookup funcLookup(this);
    funcLookup.ResetAndSelectExport(NameToLookup);
    if(funcLookup.GetCount() != 1)
    {
        return false;
    }
    EXPORTED_FUNCTION ef;
    funcLookup.Next(&ef);
    if(ef.bUnresolvedFunctions || ef.bUnresolvedResourceBindings || ef.bUnresolvedAssociations)
    {
        return false;
    }
    switch((ShaderKind)ef.pDXILFunction->ShaderKind)
    {
    case ShaderKind::Library:
        return false;
    }
    return true;
}
#endif

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::ReflectHitGroup
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::ReflectHitGroup(const CWrappedHitGroup* pHitGroup, EXPORTED_HIT_GROUP* pOut, CStateObjectInfo* pSOI)
{
    pOut->pHitGroup = &pHitGroup->m_LocalSubobjectDefinition;
    pOut->bUnresolvedFunctions = pHitGroup->m_bUnresolvedFunctions;
#ifndef SKIP_BINDING_VALIDATION
    pOut->bUnresolvedResourceBindings = false;
#endif
    pOut->bUnresolvedAssociations = false;
    for(UINT s = 0; s < 3; s++)
    {
        LPCWSTR pDependency = nullptr;
        switch (s)
        {
        case 0:
            pDependency = pHitGroup->AnyHitShaderImport;
            break;
        case 1:
            pDependency = pHitGroup->ClosestHitShaderImport;
            break;
        case 2:
            pDependency = pHitGroup->IntersectionShaderImport;
            break;
        default:
            assert(false);
            break;
        }
        if (!pDependency)
        {
            continue;
        }
        CExportInfo* pMatch = nullptr;
        size_t count = pSOI->m_ExportNameUnmangledToMangled.count(pDependency);
        switch (count)
        {
        case 0:
        {
            // Try mangled
            auto match = pSOI->m_ExportInfoMap.find(pDependency);
            if (match != pSOI->m_ExportInfoMap.end())
            {
                pMatch = match->second;
            }
            break;
        }
        case 1:
        {
            auto mangledMatch = pSOI->m_ExportNameUnmangledToMangled.find(pDependency);
            auto match = pSOI->m_ExportInfoMap.find(mangledMatch->second);
            pMatch = match->second;
            break;
        }
        default:
            assert(false);
            break;
        }
        if(pMatch)
        {
            pOut->bUnresolvedFunctions |= pMatch->m_bUnresolvedFunctions;
#ifndef SKIP_BINDING_VALIDATION
            pOut->bUnresolvedResourceBindings |= pMatch->m_bUnresolvedResourceBindings;
#endif
            pOut->bUnresolvedAssociations |= pMatch->m_bUnresolvedAssociations;
        }        
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CExportedFunctionLookup::CExportedFunctionLookup
//----------------------------------------------------------------------------------------------------------------------------------
CStateObjectInfo::CExportedFunctionLookup::CExportedFunctionLookup(CStateObjectInfo*pStateObjectInfo)
{
    if(pStateObjectInfo && pStateObjectInfo->m_bPreparedForReflection)
    {
        m_pSOI = pStateObjectInfo;
    }
    else
    {
        m_pSOI = nullptr;
    }
    ResetAndSelectExport(nullptr);    
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CExportedFunctionLookup::ResetAndSelectExport
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::CExportedFunctionLookup::ResetAndSelectExport(LPCWSTR NameToLookup)
{
    m_Index = 0;
    if(!m_pSOI)
    {
        m_Count = 0;
        m_bUsingUnmangledToMangledMatches = false;
        m_pSingleMatch = nullptr;
        return;
    }
    // Try unmangled search
    LPCWSTR LocalName = m_pSOI->LocalUniqueCopy(NameToLookup);
    m_Count = m_pSOI->m_ExportNameUnmangledToMangled.count(LocalName);
    if(m_Count)
    {
        m_UnmangledToMangledMatchRange = m_pSOI->m_ExportNameUnmangledToMangled.equal_range(LocalName);
        m_bUsingUnmangledToMangledMatches = true;
        m_pSingleMatch = nullptr;
    }
    else
    {
        m_bUsingUnmangledToMangledMatches = false;
        auto Match = m_pSOI->m_ExportInfoMap.find(LocalName);
        if(Match != m_pSOI->m_ExportInfoMap.end())
        {
            m_Count = 1;
            m_pSingleMatch = Match->second;
        }
        else
        {
            m_Count = 0;
            m_pSingleMatch = nullptr;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CExportedFunctionLookup::GetCount
//----------------------------------------------------------------------------------------------------------------------------------
size_t CStateObjectInfo::CExportedFunctionLookup::GetCount()
{
    return m_Count;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CExportedFunctionLookup::Next
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::CExportedFunctionLookup::Next(EXPORTED_FUNCTION const* pOutExportedFunction)
{
    auto pOut = const_cast<EXPORTED_FUNCTION*>(pOutExportedFunction);
    if(m_Index >= m_Count)
    {
        *pOut = {};
        return;
    }
    if(m_bUsingUnmangledToMangledMatches)
    {
        auto Match = m_pSOI->m_ExportInfoMap.find(m_UnmangledToMangledMatchRange.first->second);
        m_pSOI->FillExportedFunction(pOut,Match->second);
        m_UnmangledToMangledMatchRange.first++;
    }
    else
    {
        FillExportedFunction(pOut,m_pSingleMatch);
    }
    m_Index++;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CAssociatedSubbjectIterator::CAssociatedSubobjectIterator
//----------------------------------------------------------------------------------------------------------------------------------
CStateObjectInfo::CAssociatedSubobjectIterator::CAssociatedSubobjectIterator(CStateObjectInfo*pStateObjectInfo)
{
    if(pStateObjectInfo && pStateObjectInfo->m_bPreparedForReflection)
    {
        m_pSOI = pStateObjectInfo;
    }
    else
    {
        m_pSOI = nullptr;
    }
    ResetAndSelectExport(nullptr,(D3D12_STATE_SUBOBJECT_TYPE)-1);
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CAssociatedSubbjectIterator::ResetAndSelectExport
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::CAssociatedSubobjectIterator::ResetAndSelectExport(
    LPCWSTR MangledExportNameOrHitGroupName,
    D3D12_STATE_SUBOBJECT_TYPE Type)
{
    std::unordered_map<LPCWSTR,CExportInfo*>::iterator ex;
    if(!m_pSOI)
    {
        return;
    }
    if(nullptr == MangledExportNameOrHitGroupName)
    {
        goto Clear;
    }
    auto ASN = m_pSOI->AssociateableSubobjectName(Type);
    if(((int)Type != -1) && (ASN_INVALID == ASN))
    {
        goto Clear;
    }
    ex = m_pSOI->m_ExportInfoMap.find(m_pSOI->LocalUniqueCopy(MangledExportNameOrHitGroupName));
    if( ex != m_pSOI->m_ExportInfoMap.end() )
    {
        m_pAssociationLists = ex->second->m_Associations;
    }
    else
    {
        auto hg = m_pSOI->m_HitGroups.find(m_pSOI->LocalUniqueCopy(MangledExportNameOrHitGroupName));
        if(hg != m_pSOI->m_HitGroups.end())
        {
            m_pAssociationLists = hg->second->m_Associations;
        }
        else
        {
            goto Clear;
        }
    }
    if(-1 == (int)Type)
    {
        m_bIteratingAllTypes = true;
        m_TotalCount = 0;
        for(ASSOCIATEABLE_SUBOBJECT_NAME i = (ASSOCIATEABLE_SUBOBJECT_NAME)0; i < NUM_ASSOCIATEABLE_SUBOBJECT_TYPES; ((UINT&)i)++)
        {
            m_TotalCount += m_pAssociationLists[i].size();
        }
        // Start iterator at first subobject
        for( m_CurrAssociateableSubobjectType = 0; m_CurrAssociateableSubobjectType < NUM_ASSOCIATEABLE_SUBOBJECT_TYPES; m_CurrAssociateableSubobjectType++)
        {
            m_Iterator = m_pAssociationLists[m_CurrAssociateableSubobjectType].begin();
            if(m_Iterator != m_pAssociationLists[m_CurrAssociateableSubobjectType].end())
            {
                break;
            }
        }
    }
    else
    {
        m_bIteratingAllTypes = false;
        m_CurrAssociateableSubobjectType = ASN;
        m_TotalCount = m_pAssociationLists[ASN].size();
        m_Iterator = m_pAssociationLists[ASN].begin();
    }
    return;
Clear:
    m_TotalCount = 0;
    m_CurrAssociateableSubobjectType = NUM_ASSOCIATEABLE_SUBOBJECT_TYPES;
    m_pAssociationLists = nullptr;
    m_bIteratingAllTypes = false;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CAssociatedSubbjectIterator::GetCount
//----------------------------------------------------------------------------------------------------------------------------------
size_t CStateObjectInfo::CAssociatedSubobjectIterator::GetCount()
{
    return m_TotalCount;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CAssociatedSubbjectIterator::Next
//----------------------------------------------------------------------------------------------------------------------------------
const D3D12_STATE_SUBOBJECT* CStateObjectInfo::CAssociatedSubobjectIterator::Next()
{
    if(m_CurrAssociateableSubobjectType == NUM_ASSOCIATEABLE_SUBOBJECT_TYPES)
    {
        return nullptr; 
    }
    auto pRet = &(*m_Iterator)->m_pSubobject->m_LocalSubobjectDefinition;

    // Advance iterator
    m_Iterator++;
    if(m_Iterator == m_pAssociationLists[m_CurrAssociateableSubobjectType].end())
    {
        if(m_bIteratingAllTypes)
        {
            for(m_CurrAssociateableSubobjectType++; m_CurrAssociateableSubobjectType < NUM_ASSOCIATEABLE_SUBOBJECT_TYPES; m_CurrAssociateableSubobjectType++)
            {
                m_Iterator = m_pAssociationLists[m_CurrAssociateableSubobjectType].begin();
                if(m_Iterator != m_pAssociationLists[m_CurrAssociateableSubobjectType].end())
                {
                    break;
                }
            }        
        }
        else
        {
            m_CurrAssociateableSubobjectType = NUM_ASSOCIATEABLE_SUBOBJECT_TYPES;
        }
    }
    return pRet;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CExportedSubobjectIterator::CExportedSubobjectIterator
//----------------------------------------------------------------------------------------------------------------------------------
CStateObjectInfo::CExportedSubobjectIterator::CExportedSubobjectIterator(CStateObjectInfo*pStateObjectInfo)
{
    if(pStateObjectInfo && pStateObjectInfo->m_bPreparedForReflection)
    {
        m_pSOI = pStateObjectInfo;
        m_Count = m_pSOI->m_ExportedSubobjectsForReflection.size();
    }
    else
    {
        m_pSOI = nullptr;
        m_Count = 0;
    }
    Reset();
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CExportedSubobjectIterator::Reset
//----------------------------------------------------------------------------------------------------------------------------------
void CStateObjectInfo::CExportedSubobjectIterator::Reset()
{
    m_Index = 0;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CExportedSubobjectIterator::GetCount
//----------------------------------------------------------------------------------------------------------------------------------
size_t CStateObjectInfo::CExportedSubobjectIterator::GetCount()
{
    return m_Count;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CStateObjectInfo::CExportedSubobjectIterator::Next
//----------------------------------------------------------------------------------------------------------------------------------
const D3D12_STATE_SUBOBJECT* CStateObjectInfo::CExportedSubobjectIterator::Next()
{
    if(m_Index >= m_Count)
    {
        return nullptr;;
    }
    return m_pSOI->m_ExportedSubobjectsForReflection[m_Index++];
}

#ifdef INCLUDE_MESSAGE_LOG
//----------------------------------------------------------------------------------------------------------------------------------
// ShaderStageName
//----------------------------------------------------------------------------------------------------------------------------------
LPCWSTR ShaderStageName(ShaderKind shaderKind)
{
    switch(shaderKind)
    {
    case ShaderKind::Pixel: return L"Pixel";
    case ShaderKind::Vertex: return L"Vertex";
    case ShaderKind::Geometry: return L"Geometry";
    case ShaderKind::Hull: return L"Hull";
    case ShaderKind::Domain: return L"Domain";
    case ShaderKind::Compute: return L"Compute";
    case ShaderKind::Library: return L"Library";
    case ShaderKind::RayGeneration: return L"RayGeneration";
    case ShaderKind::Intersection: return L"Intersection";
    case ShaderKind::AnyHit: return L"AnyHit";
    case ShaderKind::ClosestHit: return L"ClosestHit";
    case ShaderKind::Miss: return L"Miss";
    case ShaderKind::Callable: return L"Callable";
    default:
        assert(false);
        break;
    }
    return L"Unknown type";
}
#endif

//==================================================================================================================================
// CDXILLibraryCache
//==================================================================================================================================
//----------------------------------------------------------------------------------------------------------------------------------
// CDXILLibraryCache::LocalUniqueCopy
//----------------------------------------------------------------------------------------------------------------------------------
D3D12_SHADER_BYTECODE CDXILLibraryCache::LocalUniqueCopy(const D3D12_SHADER_BYTECODE* pAPILibrary)
{
    ScopedLock Lock(&m_Lock);
    D3D12_SHADER_BYTECODE localLibrary;
    localLibrary.BytecodeLength = pAPILibrary->BytecodeLength;
    {
        auto match = m_APIToUniqueCopy.find(pAPILibrary->pShaderBytecode);
        if(match != m_APIToUniqueCopy.end())
        {
            match->second->RefCount++;
            localLibrary.pShaderBytecode = match->second->ShaderBytecode.data();
            return localLibrary;
        }
    }
    m_LocalCopies.emplace_front();
    auto localCopy = m_LocalCopies.begin();
    localCopy->ShaderBytecode.resize(pAPILibrary->BytecodeLength);
    localCopy->RefCount = 1;
    localCopy->pOriginalAPIBytecode = pAPILibrary->pShaderBytecode;
    memcpy(localCopy->ShaderBytecode.data(),pAPILibrary->pShaderBytecode,pAPILibrary->BytecodeLength);
    localLibrary.pShaderBytecode = localCopy->ShaderBytecode.data();
    m_APIToUniqueCopy.insert({pAPILibrary->pShaderBytecode,localCopy});
    m_LocalToUniqueCopy.insert({localLibrary.pShaderBytecode,localCopy});
    return localLibrary;
}

//----------------------------------------------------------------------------------------------------------------------------------
// CDXILLibraryCache::Release
//----------------------------------------------------------------------------------------------------------------------------------
void CDXILLibraryCache::Release(const void* pLocalShaderBytecode)
{
    ScopedLock Lock(&m_Lock);
    auto match = m_LocalToUniqueCopy.find(pLocalShaderBytecode);
    assert(match->second->RefCount > 0);
    match->second->RefCount--;
    if(0 == match->second->RefCount)
    {
        m_APIToUniqueCopy.erase(match->second->pOriginalAPIBytecode);
        m_LocalCopies.erase(match->second);
        m_LocalToUniqueCopy.erase(pLocalShaderBytecode);
    }
}

//==================================================================================================================================
