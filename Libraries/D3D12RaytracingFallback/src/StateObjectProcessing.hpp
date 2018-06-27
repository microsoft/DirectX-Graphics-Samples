//==================================================================================================================================
//
// D3D12 State Object API Description Parsing, Validation and Reflection
//
// See CStateObjectInfo
//
//==================================================================================================================================
#pragma once
using namespace hlsl::RDAT;
using namespace hlsl::DXIL;
class CStateObjectInfo; // forward declaration
class CDXILLibraryCache; // forward declaration

//----------------------------------------------------------------------------------------------------------------------------------
// PFN_CALLBACK_GET_DXIL_RUNTIME_DATA:
//      Callback used by CStateObjectInfo::ParseStateObject asking the caller to provdide the CStateObjectInfo instance
//      for an existing collection subobject encountered during state object parsing.
//----------------------------------------------------------------------------------------------------------------------------------
typedef HRESULT
(*PFN_CALLBACK_GET_DXIL_RUNTIME_DATA)(
    const void *pShaderByteCode, const UINT **ppRuntimeData, UINT *pRuntimeDataSizeInBytes);

//----------------------------------------------------------------------------------------------------------------------------------
// PFN_CALLBACK_GET_STATE_OBJECT_INFO_FOR_EXISTING_COLLECTION:
//      Callback used by CStateObjectInfo::ParseStateObject asking the caller to provdide the CStateObjectInfo instance
//      for an existing collection subobject encountered during state object parsing.
//----------------------------------------------------------------------------------------------------------------------------------
typedef CStateObjectInfo* 
            (*PFN_CALLBACK_GET_STATE_OBJECT_INFO_FOR_EXISTING_COLLECTION)(ID3D12StateObject* pExistingCollection);

//----------------------------------------------------------------------------------------------------------------------------------
// PFN_CALLBACK_GET_ROOT_SIGNATURE_DESERIALIZER:
//      Callback used by CStateObjectInfo::ParseStateObject asking the caller to provdide a root signature deserializer
//      for an existing root signature object encountered during state object parsing.
//      !!!! The callback must AddRef() before returning the pointer. !!!!
//----------------------------------------------------------------------------------------------------------------------------------
#ifndef SKIP_BINDING_VALIDATION
typedef ID3D12VersionedRootSignatureDeserializer* 
            (*PFN_CALLBACK_GET_ROOT_SIGNATURE_DESERIALIZER)(ID3D12RootSignature* pRootSignature);
#endif

//----------------------------------------------------------------------------------------------------------------------------------
// EXPORTED_FUNCTION: Returned for each state object function export by CStateObjectInfo::CExportedFunctionIterator below
//----------------------------------------------------------------------------------------------------------------------------------
typedef struct EXPORTED_FUNCTION
{
    LPCWSTR MangledName;
    LPCWSTR UnmangledName;
    const DxilFunctionDesc* pDXILFunction; // Names and dependencies in this struct are relative to original DXIL 
                                        // (not counting any renames during export)
    bool bUnresolvedFunctions;
#ifndef SKIP_BINDING_VALIDATION
    bool bUnresolvedResourceBindings;
#endif                       
    bool bUnresolvedAssociations;
} EXPORTED_FUNCTION;

//----------------------------------------------------------------------------------------------------------------------------------
// EXPORTED_HIT_GROUP: Returned for each state object hit group export by CStateObjectInfo::CExportedFunctionIterator below
//----------------------------------------------------------------------------------------------------------------------------------
typedef struct EXPORTED_HIT_GROUP
{
    const D3D12_STATE_SUBOBJECT* pHitGroup;
    bool bUnresolvedFunctions;
#ifndef SKIP_BINDING_VALIDATION
    bool bUnresolvedResourceBindings;
#endif
    bool bUnresolvedAssociations;
} EXPORTED_HIT_GROUP;

//=================================================================================================================================
// CStateObjectInfo
//
// Parse a state object while validating it and provide reflection services if parsing succeeds.
//
// Initialize via CStateObjectInfo::ParseStateObject().
//
// This inputs a state object and callback functions for the caller to help decode certain subobjects:
//      - Existing collection subobjects need their CStateObjectInfo class retrieved
//      - Root signature subobjects require a deserializer interface (which the caller should already have)
//
//==================================================================================================================================
class CStateObjectInfo
{
private:
    // Forward declaration
    class CWrappedAssociation;
    class CExportInfo;
public:
    //==============================================================================================================================
    // Public methods
    //==============================================================================================================================

    //------------------------------------------------------------------------------------------------------------------------------
    // UseExperimentalInterface: Use interfaces from the DXR prototype (this functionality is to be deleted once not needed)
    //------------------------------------------------------------------------------------------------------------------------------
    void UseExperimentalInterface() {m_bUsePrototypeInterface = true;} // Call before ParseStateObject

    //------------------------------------------------------------------------------------------------------------------------------
    // SetDefaultNodeMask: Default NodeMask to use (0x1 if not called)
    //------------------------------------------------------------------------------------------------------------------------------
    void SetDefaultNodeMask(UINT NodeMask) {m_NodeMask.NodeMask = NodeMask;} // Call before ParseStateObject

    //------------------------------------------------------------------------------------------------------------------------------
    // SetSupportedShaderModel: Shader model device supports
    //------------------------------------------------------------------------------------------------------------------------------
    void SetSupportedShaderModel(D3D_SHADER_MODEL ShaderModel) {m_ShaderModel = ShaderModel;} // Call before ParseStateObject

    //------------------------------------------------------------------------------------------------------------------------------
    // SetShaderFeaturesSupported: Features the device supports (see ShaderFeatureInfo_* in DxilContainer.h).
    // If this isn't called, it is just assumed that the device supports every shader feature.
    //------------------------------------------------------------------------------------------------------------------------------
    // Call before ParseStateObject
    void SetShaderFeaturesSupported(UINT64 ShaderFeaturesSupported) {m_ShaderFeaturesSupported = ShaderFeaturesSupported;} 
    
    //------------------------------------------------------------------------------------------------------------------------------
    // ParseStateObject: Main worker function, injests state object description with the help of some callback functions.
    //------------------------------------------------------------------------------------------------------------------------------
    HRESULT ParseStateObject(
        const D3D12_STATE_OBJECT_DESC* pStateObject,
        PFN_CALLBACK_GET_STATE_OBJECT_INFO_FOR_EXISTING_COLLECTION pfnGetStateObjectInfoForExistingCollection,
        PFN_CALLBACK_GET_DXIL_RUNTIME_DATA pfnGetDxilRuntimeData
#ifndef SKIP_BINDING_VALIDATION
        ,PFN_CALLBACK_GET_ROOT_SIGNATURE_DESERIALIZER pfnGetRootSignatureDeserializer
#endif
        ,CDXILLibraryCache* pDXILLibraryCache // cache of DXIL bytecode copies the caller owns
        );

    //------------------------------------------------------------------------------------------------------------------------------
    // Type
    //------------------------------------------------------------------------------------------------------------------------------
    D3D12_STATE_OBJECT_TYPE Type() {assert(m_bStateObjectSet); return m_SOType;}

    //------------------------------------------------------------------------------------------------------------------------------
    // GetGloballyAssociatedSubobject: Retrieve subobject For subobject types that must be associated globally in the state object.
    // Returns nullptr if the subobject isn't present (e.g. for state objects where the app has opted in to allowing unresolved
    // references), or it is a subobject type that isn't required to be globablly associated.
    // 
    // To see eligible state objects, see the definition of the table m_sAssociateableSubobjectData and look for entries that
    // specify MatchScope_FullStateObject.
    // If MatchRule_IfExistsMustExistAndMatchForAllExports is specified, a default subobject is returned if state object
    // didn't declare one.
    //
    // The state object may contain multiple redundant copies of a subobject (which are valiated to all have the same contenst
    // for a valid state object if the subobject type requires it).  Note that this function just returns one of them, so 
    // the caller must only interpret the contents rather than inferring anything about the specific pointer value returned.
    //------------------------------------------------------------------------------------------------------------------------------
    const D3D12_STATE_SUBOBJECT* GetGloballyAssociatedSubobject(D3D12_STATE_SUBOBJECT_TYPE Type);

    //------------------------------------------------------------------------------------------------------------------------------
    // CExportedFunctionIterator: Enumerates function exports for state object
    //------------------------------------------------------------------------------------------------------------------------------
    class CExportedFunctionIterator
    {
    public:
        CExportedFunctionIterator(CStateObjectInfo*pStateObjectInfo);
        size_t GetCount();
        void Next(EXPORTED_FUNCTION const* pOutExportedFunction);
        void Reset(); // only needs to be called if repeating an iteration after the first time
    private:
        CStateObjectInfo* m_pSOI;
        size_t m_Index;
        size_t m_Count;
    };
    friend class CExportedFunctionIterator;

    //------------------------------------------------------------------------------------------------------------------------------
    // CExportedHitGroupIterator: Enumerates hit group exports for state object
    //------------------------------------------------------------------------------------------------------------------------------
    class CExportedHitGroupIterator
    {
    public:
        CExportedHitGroupIterator(CStateObjectInfo*pStateObjectInfo);
        size_t GetCount();
        void Next(EXPORTED_HIT_GROUP const* pOutExportedHitGroup);
        void Reset(); // only needs to be called if repeating an iteration after the first time
    private:
        CStateObjectInfo* m_pSOI;
        size_t m_Index;
        size_t m_Count;
    };
    friend class CExportedHitGroupIterator;

    //------------------------------------------------------------------------------------------------------------------------------
    // LookupExportedHitGroup: Get info for an exported hit group
    //------------------------------------------------------------------------------------------------------------------------------
    void LookupExportedHitGroup(LPCWSTR NameToLookup, EXPORTED_HIT_GROUP const* pOutExportedHitGroup);

#ifndef SKIP_BINDING_VALIDATION
    //------------------------------------------------------------------------------------------------------------------------------
    // EligibleForShaderIdentifier: Is an export eligible for an app to ask a driver for a shader identifier that can be used in 
    //                              shader tables? (e.g. exists, of eligible shader type, no unresolved refs)
    //------------------------------------------------------------------------------------------------------------------------------
    bool EligibleForShaderIdentifier(LPCWSTR NameToLookup);

    //------------------------------------------------------------------------------------------------------------------------------
    // EligibleForStackSizeQuery: Is an export eligible for an app to ask a driver for it's stack size requirement? 
    //                            (e.g. exists, of eligible shader type, no unresolved refs)
    //------------------------------------------------------------------------------------------------------------------------------
    bool EligibleForStackSizeQuery(LPCWSTR NameToLookup);
#endif

    //------------------------------------------------------------------------------------------------------------------------------
    // CExportedFunctionLookup: Enumerates function exports that match the given name (if unmangled could yield multiple hits)
    //------------------------------------------------------------------------------------------------------------------------------
    class CExportedFunctionLookup
    {
    public:
        CExportedFunctionLookup(CStateObjectInfo*pStateObjectInfo);
        void ResetAndSelectExport(LPCWSTR NameToLookup); // Needs to be called first to pick export
        size_t GetCount();
        void Next(EXPORTED_FUNCTION const* pOutExportedFunction);
    private:
        CStateObjectInfo* m_pSOI;
        size_t m_Index;
        size_t m_Count;
        bool m_bUsingUnmangledToMangledMatches;
        std::pair<std::unordered_multimap<LPCWSTR,LPCWSTR>::iterator,std::unordered_multimap<LPCWSTR,LPCWSTR>::iterator> 
                                                           m_UnmangledToMangledMatchRange;
        const CStateObjectInfo::CExportInfo* m_pSingleMatch;
    };
    friend class CExportedFunctionLookup;

    //------------------------------------------------------------------------------------------------------------------------------
    // CAssociatedSubobjectIterator: Enumerates subobjects associated with any given export
    // that has been enumerated by CExportedFunctionIterator above.
    // Start by calling ResetAndSelectExport to pick an export for which to enumerate associations.
    // The pointers to associated subobjects that this enumeration returns are a subset of the
    // full set returned by CExportedSubobjectIterator below (which lists all exported subobjects for the state object).
    // So pointers to the same subobject will match across both iterators.
    //------------------------------------------------------------------------------------------------------------------------------
    class CAssociatedSubobjectIterator
    {
    public:
        CAssociatedSubobjectIterator(CStateObjectInfo*pStateObjectInfo);
        void ResetAndSelectExport(LPCWSTR MangledExportNameOrHitGroupName, // Needs to be called first to pick export
                                D3D12_STATE_SUBOBJECT_TYPE Type); // set -1 to enumerate all
        size_t GetCount();
        const D3D12_STATE_SUBOBJECT* Next();
    private:
        CStateObjectInfo* m_pSOI;
        std::list<CWrappedAssociation*>* m_pAssociationLists;
        bool m_bIteratingAllTypes;
        size_t m_CurrAssociateableSubobjectType;
        std::list<CWrappedAssociation*>::iterator m_Iterator;
        size_t m_TotalCount;
    };    
    friend class CAssociatedSubobjectIterator;

    //------------------------------------------------------------------------------------------------------------------------------
    // CExportedSubobjectIterator: Enumerates all subobjects that have been exported (so non-exported ones aren't in the list)
    //------------------------------------------------------------------------------------------------------------------------------
    class CExportedSubobjectIterator
    {
    public:
        CExportedSubobjectIterator(CStateObjectInfo*pStateObjectInfo);
        void Reset(); // only needs to be called if repeating an iteration after the first time
        size_t GetCount();
        const D3D12_STATE_SUBOBJECT* Next();
    private:
        CStateObjectInfo* m_pSOI;
        size_t m_Index;
        size_t m_Count;
    };    
    friend class CExportedSubobjectIterator;

    //------------------------------------------------------------------------------------------------------------------------------
    // GetLog(): Returns a list of strings describing issues encountered during parsing,
    //           useful when ParseStateObject returns failure.
    //------------------------------------------------------------------------------------------------------------------------------
#ifdef INCLUDE_MESSAGE_LOG
    const std::list<std::wstring>& GetLog();
#endif

private:
    //==============================================================================================================================
    // Implementation
    //==============================================================================================================================

    //------------------------------------------------------------------------------------------------------------------------------
    // Functions for adding subobjects
    //------------------------------------------------------------------------------------------------------------------------------   
    void SetStateObjectType(D3D12_STATE_OBJECT_TYPE SOType);
    void AddLibrary(const D3D12_DXIL_LIBRARY_DESC* pLibrary);
    void AddCollection(const D3D12_EXISTING_COLLECTION_DESC* pCollection);
    void AddHitGroup(const D3D12_HIT_GROUP_DESC* pHitGroup, CStateObjectInfo* pOwningStateObject);
    void AddRaytracingShaderConfig(const D3D12_RAYTRACING_SHADER_CONFIG* pShaderConfig, const void* pEnclosingSubobject);
    void AddRaytracingPipelineConfig(const D3D12_RAYTRACING_PIPELINE_CONFIG* pPipelineConfig, const void* pEnclosingSubobject);
    void AddGlobalRootSignature(const D3D12_GLOBAL_ROOT_SIGNATURE* pGlobalRS, const void* pEnclosingSubobject);
    void AddLocalRootSignature(const D3D12_LOCAL_ROOT_SIGNATURE* pLocalRS, const void* pEnclosingSubobject );
    void AddNodeMask(const D3D12_NODE_MASK* pNodeMask, const void* pEnclosingSubobject);
    void AddStateObjectConfig(const D3D12_STATE_OBJECT_CONFIG* pSOConfig, const void* pEnclosingSubobject );
    void AddSubobjectToExportsAssociation(const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION* pAssociation);

    //------------------------------------------------------------------------------------------------------------------------------
    // Functions directing the phases of state object parsing
    //------------------------------------------------------------------------------------------------------------------------------   
    void FinishedAddingSubobjects();
    void ResolveFunctionDependencies(); 
    void PrepareForInformationReflection();
    void TranslateSubobjectAssociations();
    void ResolveSubobjectAssociations();
#ifndef SKIP_BINDING_VALIDATION
    void ResolveResourceBindings();
#endif SKIP_BINDING_VALIDATION
    void ValidateMiscAssociations();
    void ValidateShaderFeatures();

    //------------------------------------------------------------------------------------------------------------------------------
    // Whether to allow local dependencies on external definitions - result depends on type of state object and state object configuration flags
    //------------------------------------------------------------------------------------------------------------------------------   
    bool AllowLocalDependenciesOnExternalDefinitions() const;

    //------------------------------------------------------------------------------------------------------------------------------
    // Whether to allow external dependencies on local defininitions - result depends on type of state object and state object configuration flags
    //------------------------------------------------------------------------------------------------------------------------------   
    bool AllowExternalDependenciesOnLocalDefinitions() const;

    //------------------------------------------------------------------------------------------------------------------------------
    // Utilities
    //------------------------------------------------------------------------------------------------------------------------------   
    bool SupportedShaderType(ShaderKind shader) const;
    LPCWSTR GetHitGroupDependencyTypeName(UINT i) const;
    LPCWSTR RenameMangledName(LPCWSTR OriginalMangledName, LPCWSTR OriginalUnmangledName, LPCWSTR NewUnmangledName);
    //------------------------------------------------------------------------------------------------------------------------------
    // LocalUniqueCopy():  allocates a copy of a string stored locally.  
    // Data structures like unordered_maps can hash on the pointer to the string,
    // and references to strings passed in from outside don't need to be held.
    //------------------------------------------------------------------------------------------------------------------------------
public: // TODO: Make these private once experimental code stops needing to point to this class, using reflection iterators instead.
    LPCWSTR LocalUniqueCopy(LPCWSTR string);
    static LPCWSTR LocalUniqueCopy(LPCWSTR string,std::unordered_set<std::wstring>&stringContainer);
private:
    // Strings stored by LocalUniqueCopy()
    std::unordered_set<std::wstring> m_StringContainer;

    //------------------------------------------------------------------------------------------------------------------------------
    // State variables
    //------------------------------------------------------------------------------------------------------------------------------   
    bool m_bFoundError = false;
#ifndef SKIP_BINDING_VALIDATION
    bool m_bUnresolvedResourceBindings = false;
#endif
    bool m_bUnresolvedFunctions = false;
    bool m_bUnresolvedHitGroups = false;
    bool m_bAssociationsToUnresolvedFunctions = false;
    bool m_bFunctionsWithUnresolvedAssociations = false;
    bool m_bStateObjectSet = false;
    bool m_bStateObjectTypeSelected = false;
    bool m_bPreparedForReflection = false;
    bool m_bUsePrototypeInterface = false; // delete when not needed any longer
    PFN_CALLBACK_GET_STATE_OBJECT_INFO_FOR_EXISTING_COLLECTION m_pfnGetStateObjectInfoForExistingCollection = nullptr;
    PFN_CALLBACK_GET_DXIL_RUNTIME_DATA m_pfnGetDxilRuntimeData = nullptr;
    D3D12_STATE_OBJECT_TYPE m_SOType;
    D3D_SHADER_MODEL m_ShaderModel = D3D_SHADER_MODEL_6_3;
    UINT64 m_ShaderFeaturesSupported = (UINT64)-1;
    CDXILLibraryCache* m_pDXILLibraryCache = nullptr;

    //------------------------------------------------------------------------------------------------------------------------------
    // When issues are discovered during parsing, depending on #define, either include message strings and note errors, 
    // or just record error.
    //------------------------------------------------------------------------------------------------------------------------------
#ifdef INCLUDE_MESSAGE_LOG
    #define LOG(x) {std::wostringstream temp; temp << x; Log(temp);}
    #define LOG_IN_CALLBACK(x) {std::wostringstream temp; temp << x; pThis->Log(temp);}
    #define LOG_ERROR(x) {LOG(x); m_bFoundError = true;}
    #define LOG_ERROR_IN_CALLBACK(x) {LOG_IN_CALLBACK(x); pThis->m_bFoundError = true;}
    void Log(std::wostringstream& message);
    std::list<std::wstring> m_Log;
    LPCWSTR PrettyPrintPossiblyMangledName(LPCWSTR name);
#else
    #define LOG(x) 
    #define LOG_IN_CALLBACK(x) 
    #define LOG_ERROR(x) {m_bFoundError = true;}
    #define LOG_ERROR_IN_CALLBACK(x) {pThis->m_bFoundError = true;}
    #define LOG_ERROR_NOMESSAGE {m_bFoundError = true;}
#endif 

    //------------------------------------------------------------------------------------------------------------------------------
    // CWrappedDXILLibrary:  Tracking information for DXIL Libraries used by the state object.
    //                       Containes DxilRuntimeReflection which is information about a DXIL library custom presented in a 
    //                       form convenient for runtime state object API parsing.
    //------------------------------------------------------------------------------------------------------------------------------
    class CWrappedDXILLibrary
    {
    public:
        bool Init(const D3D12_DXIL_LIBRARY_DESC* pLibrary, CDXILLibraryCache* pDXILLibraryCache, PFN_CALLBACK_GET_DXIL_RUNTIME_DATA pfnGetRuntimeData);
        ~CWrappedDXILLibrary();
        const DxilLibraryDesc GetLibraryReflection();
        D3D12_STATE_SUBOBJECT m_LocalSubobjectDefinition = {};
        D3D12_DXIL_LIBRARY_DESC m_LocalLibraryDesc = {};
    private:
        std::vector<D3D12_EXPORT_DESC> m_Exports;
        std::unordered_set<std::wstring> m_StringContainer; // local string container so this can be inherited by collections cleanly
        std::unique_ptr<DxilRuntimeReflection> m_pReflection;
        CDXILLibraryCache* m_pDXILLibraryCache = nullptr;
    };
    std::list<std::shared_ptr<CWrappedDXILLibrary>> m_DXILLibraryList;


public:
    //------------------------------------------------------------------------------------------------------------------------------
    // CDxilLibraryIterator: Enumerates Dxil Libraries for state object
    //------------------------------------------------------------------------------------------------------------------------------
    class CDxilLibraryIterator
    {
    public:
        CDxilLibraryIterator(CStateObjectInfo*pStateObjectInfo);
        size_t GetCount();
        void Next(D3D12_DXIL_LIBRARY_DESC *pDxilLibraryDesc);
        void Reset(); // only needs to be called if repeating an iteration after the first time
    private:
        CStateObjectInfo * m_pSOI;
        size_t m_Count;
        decltype(m_DXILLibraryList)::const_iterator m_libIterator;
    };
    friend class CDxilLibraryIterator;
private:

    //------------------------------------------------------------------------------------------------------------------------------
    // CWrappedExistingCollection:  Tracking information for existing collection state objects.
    //------------------------------------------------------------------------------------------------------------------------------
    class CWrappedExistingCollection
    {
    public:
        void Init(const D3D12_EXISTING_COLLECTION_DESC* pCollection);
        D3D12_STATE_SUBOBJECT m_LocalSubobjectDefinition = {};
    private:
        D3D12_EXISTING_COLLECTION_DESC m_LocalCollectionDesc = {};
        std::vector<D3D12_EXPORT_DESC> m_Exports;
        std::unordered_set<std::wstring> m_StringContainer;
    };
    std::list<CWrappedExistingCollection> m_ExistingCollectionList;

    //------------------------------------------------------------------------------------------------------------------------------
    // Translations for external subobject identifier (unknown type) to internal subobject copy, 
    // cleared out when not needed any more
    //------------------------------------------------------------------------------------------------------------------------------
    std::unordered_map<const void*,void*> m_EnclosingSubobjectPointerToInternal;

    //------------------------------------------------------------------------------------------------------------------------------
    // CAssociateableSubobjectInfo: Base class for tracking data for subobjects that are capable of being associated with exports
    //                              Instantiated via inheritance, e.g. CSimpleAssociateableSubobjectWrapper below.
    //------------------------------------------------------------------------------------------------------------------------------
    class CAssociateableSubobjectInfo
    {
    public:
        virtual ~CAssociateableSubobjectInfo() {};
        virtual bool Compare(CAssociateableSubobjectInfo*pOther) = 0;
        bool m_bUsedInExplicitAssociation = false;
        bool m_bReferenced = false;
        D3D12_STATE_SUBOBJECT m_LocalSubobjectDefinition; // contains local D3D12_RAYTRACING_SHADER_CONFIG, etc. from child class
    };

    //------------------------------------------------------------------------------------------------------------------------------
    // CSimpleAssociateableSubobjectWrapper: For subobjects that can be associated with exports, this wrapper
    //      holds the root subobject definition, rootType (e.g. D3D12_RAYTRACING_SHADER_CONFIG), as well as 
    //      tracking information defined by CAssociateableSubobject info above.
    //      
    // For comparing duplicate subobjects of a given type, Compare() is implemented, simply doing a memory compare.
    // Should there be more complex situations where subobjects could be considered duplicate even if not bit identical,
    // implement a different wrapper with a custom Compare().
    // 
    //------------------------------------------------------------------------------------------------------------------------------
    template<typename rootType>
    class CSimpleAssociateableSubobjectWrapper : public rootType, public CAssociateableSubobjectInfo
    {
    public:
        bool Compare(CAssociateableSubobjectInfo*pOther) override
        {
            auto pCompare = (CSimpleAssociateableSubobjectWrapper<rootType>*)pOther;
            return (0 == memcmp((rootType*)(pCompare),(rootType*)(this),sizeof(rootType)));
        }
    };

    //------------------------------------------------------------------------------------------------------------------------------
    // TrackAssociateableSubobject: When an associateable subobject is encountered, build a wrapper (see above) to track it.
    //                              This will make an instance of a template declared above 
    //                              (e.g. CSimpleAssociateableSubobjectWrapper).
    //------------------------------------------------------------------------------------------------------------------------------
    CAssociateableSubobjectInfo* TrackAssociateableSubobject(
        D3D12_STATE_SUBOBJECT_TYPE SubobjectType,
        const void* pStateSubobjectDesc, // depends on SubobjectType, D3D12_RAYTRACING_PIPELINE_CONFIG etc.
        const void* pEnclosingSubobject); // null if inherited from existing state object
  
    //------------------------------------------------------------------------------------------------------------------------------
    // ASSOCIATEABLE_SUBOBJECT_NAME: Flat enum of subobject types that can be associated with exports.
    // Useful for arrays where each element is related to a given associateable subobject.
    //------------------------------------------------------------------------------------------------------------------------------   
    typedef enum ASSOCIATEABLE_SUBOBJECT_NAME
    {
        ASN_RAYTRACING_SHADER_CONFIG = 0,
        ASN_RAYTRACING_PIPELINE_CONFIG,
        ASN_GLOBAL_ROOT_SIGNATURE,
        ASN_LOCAL_ROOT_SIGNATURE,
        ASN_NODE_MASK,
        ASN_STATE_OBJECT_CONFIG,
        NUM_ASSOCIATEABLE_SUBOBJECT_TYPES,
        ASN_INVALID = NUM_ASSOCIATEABLE_SUBOBJECT_TYPES
    } ASSOCIATEABLE_SUBOBJECT_NAME;
    ASSOCIATEABLE_SUBOBJECT_NAME AssociateableSubobjectName(D3D12_STATE_SUBOBJECT_TYPE sub) const; // returns ASN_INVALID for invalid subobject

    //------------------------------------------------------------------------------------------------------------------------------
    // ASSOCIATEABLE_SUBOBJECT_DATA: 
    //------------------------------------------------------------------------------------------------------------------------------
    class ASSOCIATEABLE_SUBOBJECT_DATA
    {
    public:
        ~ASSOCIATEABLE_SUBOBJECT_DATA();
        CWrappedAssociation* pLocalDefaultSubobject = nullptr;
        CWrappedAssociation* pUsedInExplicitDefaultAssociation = nullptr;
        std::list<CAssociateableSubobjectInfo*> Associateable;
        std::list<CAssociateableSubobjectInfo*> InheritedSoNotAssociateable; // e.g. from collection
#ifdef INCLUDE_MESSAGE_LOG
        bool bContinuePrintingMissingSubobjectMessages = true;
#endif
    };
    ASSOCIATEABLE_SUBOBJECT_DATA m_AssociateableSubobjectData[NUM_ASSOCIATEABLE_SUBOBJECT_TYPES];  

    //------------------------------------------------------------------------------------------------------------------------------
    // MATCH_SCOPE: When defining a rule about consistency of subobject associaitons across multiple functions in a state object,
    //              for which scope within the state object should the rule apply?
    //------------------------------------------------------------------------------------------------------------------------------
    typedef enum MATCH_SCOPE
    {
        MatchScope_NoRequirements,
        MatchScope_FullStateObject,
        MatchScope_LocalStateObject,
        MatchScope_CallGraph,
    } MATCH_SCOPE;

    //------------------------------------------------------------------------------------------------------------------------------
    // MATCH_RULE: For a given associateable subobject type, what rule should be enforced within MATCH_SCOPE in a state object
    //             regarding consistency of associations across exports?
    //------------------------------------------------------------------------------------------------------------------------------
    typedef enum MATCH_RULE
    {
        MatchRule_NoRequirements,
        MatchRule_RequiredAndMatchingForAllExports,
        MatchRule_IfExistsMustExistAndMatchForAllExports,
        MatchRule_IfExistsMustMatchOthersThatExistPlusShaderEntry, // "shader entry" bit only apples for MatchScope_CallGraph
    } MATCH_RULE;

    //------------------------------------------------------------------------------------------------------------------------------
    // ASSOCIATEABLE_SUBOBJECT_STATIC_DATA: Invariant properties of subobjects that are capable of being associated
    //                                      m_sAccociateableSubobjectData is a static table of this type.
    //------------------------------------------------------------------------------------------------------------------------------
    class ASSOCIATEABLE_SUBOBJECT_STATIC_DATA
    {
    public:
        D3D12_STATE_SUBOBJECT_TYPE APIName;
        LPCWSTR StringAPIName;
        MATCH_RULE MatchRule;
        MATCH_SCOPE MatchScope;
        bool bAtMostOneAssociationPerExport;
    };
    static const ASSOCIATEABLE_SUBOBJECT_STATIC_DATA m_sAssociateableSubobjectData[NUM_ASSOCIATEABLE_SUBOBJECT_TYPES]; 
  
    //------------------------------------------------------------------------------------------------------------------------------
    // CWrappedAssociation: Tracking data for associations that have been defined (but may or may not be resolved yet)
    //------------------------------------------------------------------------------------------------------------------------------
    class CWrappedAssociation
    {
    public:
        std::unordered_set<LPCWSTR> m_Exports; // Which exports the subobject is associated with
        D3D12_STATE_SUBOBJECT_TYPE m_SubobjectType;
        const void* m_pUntranslatedExternalSubobject = nullptr; // when non-null, this gets translated to m_pSubobject below 
                                                                // when all subobjects seen.
        CAssociateableSubobjectInfo* m_pSubobject = nullptr; // type is a child of this class that depends on m_SubobjectType
    };
    std::list<CWrappedAssociation> m_SubobjectToExportsAssociations[NUM_ASSOCIATEABLE_SUBOBJECT_TYPES];

    //------------------------------------------------------------------------------------------------------------------------------
    // Default subobject definitions for optional subobjects (overridden if definitions come along)
    //------------------------------------------------------------------------------------------------------------------------------   
    D3D12_NODE_MASK m_NodeMask = {0x1};
    D3D12_STATE_OBJECT_CONFIG m_StateObjectConfig = {D3D12_STATE_OBJECT_FLAG_NONE};
    // m_GlobalSubobjectDefinitions only has entries with MatchScope_FullStateObject defined and if the subobect is actually present
    // and globablly unique.  If not defined, the contained subobject pointer is null
    D3D12_STATE_SUBOBJECT m_GlobalSubobjectDefinitions[NUM_ASSOCIATEABLE_SUBOBJECT_TYPES] = {};

#ifndef SKIP_BINDING_VALIDATION
    //------------------------------------------------------------------------------------------------------------------------------
    // Function -> Root Signature validation state
    //------------------------------------------------------------------------------------------------------------------------------
    static void ReportLibraryFunctionErrorCallback(void* pContext, LPCWSTR pError,UINT ErrorFlags);    
    class RLFECallbackContext
    {
    public:
        CStateObjectInfo* pThis;
        CExportInfo* pExportInfo;
        LPCWSTR pLibraryFunction;
    };

    PFN_CALLBACK_GET_ROOT_SIGNATURE_DESERIALIZER m_pfnGetRootSignatureDeserializer = nullptr;
    class CRootSigPair
    {
    public:
        CSimpleAssociateableSubobjectWrapper<D3D12_GLOBAL_ROOT_SIGNATURE>* m_pGlobal = nullptr;
        CSimpleAssociateableSubobjectWrapper<D3D12_LOCAL_ROOT_SIGNATURE>* m_pLocal = nullptr;
    };
    class CRootSigVerifier
    {
    public:
        bool m_bRootSigsValidTogether = false;
#ifndef SKIP_BINDING_VALIDATION
        RootSignatureVerifier m_RSV;
#endif
    };

    struct CRootSigPairHash
    {
        size_t operator()(CRootSigPair const& RootSig) const noexcept
        {
            size_t h1 = std::hash<void*>{}(RootSig.m_pGlobal);
            size_t h2 = std::hash<void*>{}(RootSig.m_pLocal);
            return h1 ^ h2 + 0x9e3779b9 + (h1 << 6) + (h2 >> 2);
        }
        constexpr bool operator()(const CRootSigPair& lhs, const CRootSigPair& rhs) const
        {
            return ((lhs.m_pGlobal == rhs.m_pGlobal)&&(lhs.m_pLocal == rhs.m_pLocal));
        }
    };
    std::unordered_map<CRootSigPair,CRootSigVerifier*,CRootSigPairHash,CRootSigPairHash> m_ValidatedRootSigPairs;
    std::list<CRootSigVerifier> m_RootSigPairVerifierList;   
#endif

    //------------------------------------------------------------------------------------------------------------------------------
    // CExportInfo:  Tracking information for functions exported by the state object
    //------------------------------------------------------------------------------------------------------------------------------
    class CExportInfo
    {
    public:
        LPCWSTR m_MangledName = nullptr;
        LPCWSTR m_UnmangledName = nullptr;
        const DxilFunctionDesc* m_pFunctionInfo = nullptr;
        std::list<CWrappedAssociation*> m_Associations[NUM_ASSOCIATEABLE_SUBOBJECT_TYPES];
        CStateObjectInfo* m_pOwningStateObject = nullptr;

        // The following are used during various graph traversals
        UINT64 m_VisitedOnGraphTraversalIndex = (UINT64)-1;
        CAssociateableSubobjectInfo* m_pFirstSubobjectInLibraryFunctionSubtree = nullptr;
        LPCWSTR m_pNameOfFirstExportWithSubobjectInSubtree = nullptr;
        UINT m_SubtreeValidShaderStageFlag = 0;
        bool m_bExternalDependenciesOnThisExportAllowed = false;
#ifndef SKIP_BINDING_VALIDATION
        bool m_bUnresolvedResourceBindings = false;
#endif
        bool m_bUnresolvedFunctions = false;
        bool m_bUnresolvedAssociations = false;
#ifndef SKIP_BINDING_VALIDATION
        std::unordered_set<CRootSigPair,CRootSigPairHash,CRootSigPairHash> m_RootSigsValidatedOnSubtree;
#endif        

        UINT m_GraphTraversalFlags = 0;
        enum GraphTraversalFlags
        {
            GTF_SubtreeAlreadyCheckedForCycles = 0x1,
            GTF_CycleFound = 0x2
        };
    };

    //------------------------------------------------------------------------------------------------------------------------------
    // Export related functions, including dependency graph traversal functions
    //------------------------------------------------------------------------------------------------------------------------------
    void AddExport(LPCWSTR pExternalNameMangled, 
                   LPCWSTR pExternalNameUnmangled, 
                   const DxilFunctionDesc* pInfo, 
                   CStateObjectInfo* pOwningStateObject,
                   bool bExternalDependenciesOnThisExportAllowed);
    void TraverseFunctionsInitialValidation(LPCWSTR function);
    class CAssociateableSubobjectInfo;
    CAssociateableSubobjectInfo* TraverseFunctionsFindFirstSubobjectInLibraryFunctionSubtrees(LPCWSTR function);
    void TraverseFunctionsSubobjectConsistency(LPCWSTR function);
#ifndef SKIP_BINDING_VALIDATION
    void TraverseFunctionsResourceBindingValidation(LPCWSTR function);
    void ValidateRootSignaturePair(const CRootSigPair& RootSigs, CRootSigVerifier* pVerifier);
#endif
    UINT TraverseFunctionsShaderStageValidation(LPCWSTR function);
    static void FillExportedFunction(EXPORTED_FUNCTION* pEF, const CExportInfo* pEI);
    //------------------------------------------------------------------------------------------------------------------------------
    // Export related data
    //------------------------------------------------------------------------------------------------------------------------------
    std::list<CExportInfo> m_ExportInfoList; // Instances of CExportInfo that structures like m_ExportInfoMap below can point to
    std::unordered_map<LPCWSTR, CExportInfo*> m_ExportInfoMap; // mangled name -> CExportInfo*
    std::unordered_multimap<LPCWSTR, LPCWSTR> m_ExportNameUnmangledToMangled; // unmangled name -> mangled name exported
    std::unordered_map<LPCWSTR, LPCWSTR> m_ExportNameMangledToUnmangled; // exported mangled name -> unmangled name
    std::unordered_multimap<LPCWSTR,CExportInfo*> m_Dependencies; // multiple callers (CExportInfo* for each) can depend 
                                                                  // on any given function
    std::unordered_set<LPCWSTR> m_UsedUnmangledFunctionNames; // unmangled function names and non-function (e.g. hitgroup) names 
                                                              // can't collide, for simplicity
    std::unordered_set<LPCWSTR> m_UsedNonFunctionNames;                                                                  

    //------------------------------------------------------------------------------------------------------------------------------
    // TRAVERSAL_GLOBALS: Global data referenced during various function graph traversals,
    //                    each of which may only use a subset of the state here.
    //------------------------------------------------------------------------------------------------------------------------------
    class TRAVERSAL_GLOBALS
    {
    public:
        UINT64  GraphTraversalIndex = 0;
        ASSOCIATEABLE_SUBOBJECT_NAME  AssociateableSubobjectIndex = (ASSOCIATEABLE_SUBOBJECT_NAME)0;
        CAssociateableSubobjectInfo* pReferenceSubobject = nullptr;
        bool bRootIsEntryFunction = false;
        bool bAssignedRef = false;
#ifndef SKIP_BINDING_VALIDATION        
        CRootSigVerifier* pRootSigVerifier;
        CRootSigPair RootSigs;
#endif
#ifdef INCLUDE_MESSAGE_LOG
        LPCWSTR pNameOfExportWithReferenceSubobject = nullptr;
#endif
    };
    TRAVERSAL_GLOBALS m_TraversalGlobals;

    //------------------------------------------------------------------------------------------------------------------------------
    // CWrappedHitGroup:  Tracking information for hit groups exported by the state object
    //------------------------------------------------------------------------------------------------------------------------------
    class CWrappedHitGroup : public D3D12_HIT_GROUP_DESC
    {
    public:
        bool m_bUnresolvedFunctions = false;
        CStateObjectInfo* m_pOwningStateObject = nullptr;
        D3D12_STATE_SUBOBJECT m_LocalSubobjectDefinition = {};
        std::list<CWrappedAssociation*> m_Associations[NUM_ASSOCIATEABLE_SUBOBJECT_TYPES];
    };

    //------------------------------------------------------------------------------------------------------------------------------
    // Hit group data
    //------------------------------------------------------------------------------------------------------------------------------
    std::list<CWrappedHitGroup> m_HitGroupList;
    std::unordered_map<LPCWSTR,CWrappedHitGroup*> m_HitGroups; // hit group name -> hit group desc
    static void ReflectHitGroup(const CWrappedHitGroup* pHitGroup, EXPORTED_HIT_GROUP* pOut, CStateObjectInfo* pSOI);

    //------------------------------------------------------------------------------------------------------------------------------
    // Arrays for efficient reflection
    //------------------------------------------------------------------------------------------------------------------------------
    std::vector<const CExportInfo*> m_ExportArrayForReflection;
    std::vector<const CWrappedHitGroup*> m_HitGroupArrayForReflection;
    std::vector<const D3D12_STATE_SUBOBJECT*> m_ExportedSubobjectsForReflection;
};

//----------------------------------------------------------------------------------------------------------------------------------
// Utility functions
//----------------------------------------------------------------------------------------------------------------------------------
#ifdef INCLUDE_MESSAGE_LOG
LPCWSTR ShaderStageName(ShaderKind shaderKind);
#endif

//==================================================================================================================================
// CDXILLibraryCache
//==================================================================================================================================
class CDXILLibraryCache
{
public:
    // Will only make a local copy for each unique input API pointer
    // Doesn't dedupe different input API pointers to duplicate copies.
    D3D12_SHADER_BYTECODE LocalUniqueCopy(const D3D12_SHADER_BYTECODE* pAPILibrary);
    void Release(const void* pLocalShaderBytecode);

private:
    class ScopedLock;
    class CriticalSection
    {
        friend class ScopedLock;
    public:
        CriticalSection()
        {
            InitializeCriticalSectionAndSpinCount(&CS, 8);
        }

        ~CriticalSection()
        {
            DeleteCriticalSection(&CS);
        }

    private:
        CRITICAL_SECTION CS;
    };

    class ScopedLock
    {
    public:

        ScopedLock() : pCS(nullptr) {};
        ScopedLock(CriticalSection* pCSIn) : pCS(pCSIn)
        {
            if (pCS)
            {
                EnterCriticalSection(&pCS->CS);
            }
        };

        ~ScopedLock()
        {
            if (pCS)
            {
                LeaveCriticalSection(&pCS->CS);
            }
        }

    private:

        CriticalSection* pCS;
    };

    CriticalSection m_Lock;    
    class LIST_NODE
    {
    public:
        unsigned int RefCount = 0;
        std::vector<BYTE> ShaderBytecode;
        const void* pOriginalAPIBytecode = nullptr;
    };
    
    std::unordered_map<const void*,std::list<LIST_NODE>::iterator> m_APIToUniqueCopy;
    std::unordered_map<const void*,std::list<LIST_NODE>::iterator> m_LocalToUniqueCopy;
    std::list<LIST_NODE> m_LocalCopies;
};

//==================================================================================================================================
