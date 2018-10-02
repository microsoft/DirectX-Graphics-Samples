///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilLibraryReflection.h                                                   //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Defines shader reflection for runtime usage.                              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include "DxilConstants.h"

namespace hlsl {
namespace RDAT {

// Data Layout:
// -start:
//  RuntimeDataHeader header;
//  uint32_t offsets[header.PartCount];
//  - for each i in header.PartCount:
//    - at &header + offsets[i]:
//      RuntimeDataPartHeader part;
//    - if part.Type is a Table (Function or Resource):
//      RuntimeDataTableHeader table;
//      byte TableData[table.RecordCount][table.RecordStride];
//    - else if part.Type is String:
//      byte UTF8Data[part.Size];
//    - else if part.Type is Index:
//      uint32_t IndexData[part.Size / 4];

enum class RuntimeDataPartType : uint32_t { // TODO: Rename: PartType
  Invalid = 0,
  StringBuffer = 1,
  IndexArrays = 2,
  ResourceTable = 3,
  FunctionTable = 4,
};

enum RuntimeDataVersion {
  // Cannot be mistaken for part count from prerelease version
  RDAT_Version_0 = 0x10,
};

struct RuntimeDataHeader {
  uint32_t Version;
  uint32_t PartCount;
  // Followed by uint32_t array of offsets to parts
  // offsets are relative to the beginning of this header
  // offsets must be 4-byte aligned
  //  uint32_t offsets[];
};
struct RuntimeDataPartHeader {
  RuntimeDataPartType Type;
  uint32_t Size;  // Not including this header.  Must be 4-byte aligned.
  // Followed by part data
  //  byte Data[ALIGN4(Size)];
};

// For tables of records, such as Function and Resource tables
// Stride allows for extending records, with forward and backward compatibility
struct RuntimeDataTableHeader {
  uint32_t RecordCount;
  uint32_t RecordStride;  // Must be 4-byte aligned.
  // Followed by recordCount records of recordStride size
  // byte TableData[RecordCount * RecordStride];
};

// General purpose strided table reader with casting Row() operation that
// returns nullptr if stride is smaller than type, for record expansion.
class TableReader {
  const char *m_table;
  uint32_t m_count;
  uint32_t m_stride;

public:
  TableReader() : TableReader(nullptr, 0, 0) {}
  TableReader(const char *table, uint32_t count, uint32_t stride)
    : m_table(table), m_count(count), m_stride(stride) {}
  void Init(const char *table, uint32_t count, uint32_t stride) {
    m_table = table; m_count = count; m_stride = stride;
  }
  const char *Data() const { return m_table; }
  uint32_t Count() const { return m_count; }
  uint32_t Stride() const { return m_stride; }

  template<typename T>
  const T *Row(uint32_t index) const {
    if (index < m_count && sizeof(T) <= m_stride)
      return reinterpret_cast<const T*>(m_table + (m_stride * index));
    return nullptr;
  }
};


// Index table is a sequence of rows, where each row has a count as a first
// element followed by the count number of elements pre computing values
class IndexTableReader {
private:
  const uint32_t *m_table;
  uint32_t m_size;

public:
  class IndexRow {
  private:
    const uint32_t *m_values;
    const uint32_t m_count;

  public:
    IndexRow(const uint32_t *values, uint32_t count)
        : m_values(values), m_count(count) {}
    uint32_t Count() { return m_count; }
    uint32_t At(uint32_t i) { return m_values[i]; }
  };

  IndexTableReader() : m_table(nullptr), m_size(0) {}
  IndexTableReader(const uint32_t *table, uint32_t size)
      : m_table(table), m_size(size) {}

  void SetTable(const uint32_t *table) { m_table = table; }

  void SetSize(uint32_t size) { m_size = size; }

  IndexRow getRow(uint32_t i) { return IndexRow(&m_table[i] + 1, m_table[i]); }
};

class StringTableReader {
  const char *m_table;
  uint32_t m_size;
public:
  StringTableReader() : m_table(nullptr), m_size(0) {}
  StringTableReader(const char *table, uint32_t size)
      : m_table(table), m_size(size) {}
  const char *Get(uint32_t offset) const {
    _Analysis_assume_(offset < m_size && m_table &&
                      m_table[m_size - 1] == '\0');
    return m_table + offset;
  }
};

enum class DxilResourceFlag : uint32_t {
  None                      = 0,
  UAVGloballyCoherent       = 1 << 0,
  UAVCounter                = 1 << 1,
  UAVRasterizerOrderedView  = 1 << 2,
  DynamicIndexing           = 1 << 3,
};

struct RuntimeDataResourceInfo {
  uint32_t Class; // hlsl::DXIL::ResourceClass
  uint32_t Kind;  // hlsl::DXIL::ResourceKind
  uint32_t ID;    // id per class
  uint32_t Space;
  uint32_t LowerBound;
  uint32_t UpperBound;
  uint32_t Name;  // resource name as an offset for string table
  uint32_t Flags; // hlsl::RDAT::DxilResourceFlag
};

struct RuntimeDataFunctionInfo {
  uint32_t Name;                 // offset for string table
  uint32_t UnmangledName;        // offset for string table
  uint32_t Resources;            // index to an index table
  uint32_t FunctionDependencies; // index to a list of functions that function
                                 // depends on
  uint32_t ShaderKind;
  uint32_t PayloadSizeInBytes;   // 1) hit, miss, or closest shader: payload count
                                 // 2) call shader: parameter size 
  uint32_t AttributeSizeInBytes; // attribute size for closest hit and any hit
  uint32_t FeatureInfo1;         // first 32 bits of feature flag
  uint32_t FeatureInfo2;         // second 32 bits of feature flag
  uint32_t ShaderStageFlag;      // valid shader stage flag.
  uint32_t MinShaderTarget;      // minimum shader target.
};

class ResourceTableReader;
class FunctionTableReader;

struct RuntimeDataContext {
  StringTableReader *pStringTableReader;
  IndexTableReader *pIndexTableReader;
  ResourceTableReader *pResourceTableReader;
  FunctionTableReader *pFunctionTableReader;
};

class ResourceReader {
private:
  const RuntimeDataResourceInfo *m_ResourceInfo;
  RuntimeDataContext *m_Context;

public:
  ResourceReader(const RuntimeDataResourceInfo *resInfo,
                 RuntimeDataContext *context)
      : m_ResourceInfo(resInfo), m_Context(context) {}
  hlsl::DXIL::ResourceClass GetResourceClass() const {
    return !m_ResourceInfo ? hlsl::DXIL::ResourceClass::Invalid
                           : (hlsl::DXIL::ResourceClass)m_ResourceInfo->Class;
  }
  uint32_t GetSpace() const { return !m_ResourceInfo ? 0 : m_ResourceInfo->Space; }
  uint32_t GetLowerBound() const { return !m_ResourceInfo ? 0 : m_ResourceInfo->LowerBound; }
  uint32_t GetUpperBound() const { return !m_ResourceInfo ? 0 : m_ResourceInfo->UpperBound; }
  hlsl::DXIL::ResourceKind GetResourceKind() const {
    return !m_ResourceInfo ? hlsl::DXIL::ResourceKind::Invalid
                           : (hlsl::DXIL::ResourceKind)m_ResourceInfo->Kind;
  }
  uint32_t GetID() const { return !m_ResourceInfo ? 0 : m_ResourceInfo->ID; }
  const char *GetName() const {
    return !m_ResourceInfo ? ""
           : m_Context->pStringTableReader->Get(m_ResourceInfo->Name);
  }
  uint32_t GetFlags() const { return !m_ResourceInfo ? 0 : m_ResourceInfo->Flags; }
};

class ResourceTableReader {
private:
  TableReader m_Table;
  RuntimeDataContext *m_Context;
  uint32_t m_CBufferCount;
  uint32_t m_SamplerCount;
  uint32_t m_SRVCount;
  uint32_t m_UAVCount;

public:
  ResourceTableReader()
      : m_Context(nullptr), m_CBufferCount(0),
        m_SamplerCount(0), m_SRVCount(0), m_UAVCount(0){};

  void SetResourceInfo(const char *ptr, uint32_t count, uint32_t recordStride) {
    m_Table.Init(ptr, count, recordStride);
    // Assuming that resources are in order of CBuffer, Sampler, SRV, and UAV,
    // count the number for each resource class
    m_CBufferCount = 0;
    m_SamplerCount = 0;
    m_SRVCount = 0;
    m_UAVCount = 0;

    for (uint32_t i = 0; i < count; ++i) {
      const RuntimeDataResourceInfo *curPtr =
        m_Table.Row<RuntimeDataResourceInfo>(i);
      if (curPtr->Class == (uint32_t)hlsl::DXIL::ResourceClass::CBuffer)
        m_CBufferCount++;
      else if (curPtr->Class == (uint32_t)hlsl::DXIL::ResourceClass::Sampler)
        m_SamplerCount++;
      else if (curPtr->Class == (uint32_t)hlsl::DXIL::ResourceClass::SRV)
        m_SRVCount++;
      else if (curPtr->Class == (uint32_t)hlsl::DXIL::ResourceClass::UAV)
        m_UAVCount++;
    }
  }

  void SetContext(RuntimeDataContext *context) { m_Context = context; }

  uint32_t GetNumResources() const {
    return m_CBufferCount + m_SamplerCount + m_SRVCount + m_UAVCount;
  }
  ResourceReader GetItem(uint32_t i) const {
    _Analysis_assume_(i < GetNumResources());
    return ResourceReader(m_Table.Row<RuntimeDataResourceInfo>(i), m_Context);
  }

  uint32_t GetNumCBuffers() const { return m_CBufferCount; }
  ResourceReader GetCBuffer(uint32_t i) {
    _Analysis_assume_(i < m_CBufferCount);
    return ResourceReader(m_Table.Row<RuntimeDataResourceInfo>(i), m_Context);
  }

  uint32_t GetNumSamplers() const { return m_SamplerCount; }
  ResourceReader GetSampler(uint32_t i) {
    _Analysis_assume_(i < m_SamplerCount);
    uint32_t offset = (m_CBufferCount + i);
    return ResourceReader(m_Table.Row<RuntimeDataResourceInfo>(offset), m_Context);
  }

  uint32_t GetNumSRVs() const { return m_SRVCount; }
  ResourceReader GetSRV(uint32_t i) {
    _Analysis_assume_(i < m_SRVCount);
    uint32_t offset = (m_CBufferCount + m_SamplerCount + i);
    return ResourceReader(m_Table.Row<RuntimeDataResourceInfo>(offset), m_Context);
  }

  uint32_t GetNumUAVs() const { return m_UAVCount; }
  ResourceReader GetUAV(uint32_t i) {
    _Analysis_assume_(i < m_UAVCount);
    uint32_t offset = (m_CBufferCount + m_SamplerCount + m_SRVCount + i);
    return ResourceReader(m_Table.Row<RuntimeDataResourceInfo>(offset), m_Context);
  }
};

class FunctionReader {
private:
  const RuntimeDataFunctionInfo *m_RuntimeDataFunctionInfo;
  RuntimeDataContext *m_Context;

public:
  FunctionReader() : m_RuntimeDataFunctionInfo(nullptr), m_Context(nullptr) {}
  FunctionReader(const RuntimeDataFunctionInfo *functionInfo,
                 RuntimeDataContext *context)
      : m_RuntimeDataFunctionInfo(functionInfo), m_Context(context) {}

  const char *GetName() const {
    return !m_RuntimeDataFunctionInfo ? ""
      : m_Context->pStringTableReader->Get(m_RuntimeDataFunctionInfo->Name);
  }
  const char *GetUnmangledName() const {
    return !m_RuntimeDataFunctionInfo ? ""
      : m_Context->pStringTableReader->Get(
          m_RuntimeDataFunctionInfo->UnmangledName);
  }
  uint64_t GetFeatureFlag() const {
    return (static_cast<uint64_t>(GetFeatureInfo2()) << 32)
           | static_cast<uint64_t>(GetFeatureInfo1());
  }
  uint32_t GetFeatureInfo1() const {
    return !m_RuntimeDataFunctionInfo ? 0
      : m_RuntimeDataFunctionInfo->FeatureInfo1;
  }
  uint32_t GetFeatureInfo2() const {
    return !m_RuntimeDataFunctionInfo ? 0
      : m_RuntimeDataFunctionInfo->FeatureInfo2;
  }

  uint32_t GetShaderStageFlag() const {
    return !m_RuntimeDataFunctionInfo ? 0
      : m_RuntimeDataFunctionInfo->ShaderStageFlag;
  }
  uint32_t GetMinShaderTarget() const {
    return !m_RuntimeDataFunctionInfo ? 0
      : m_RuntimeDataFunctionInfo->MinShaderTarget;
  }
  uint32_t GetNumResources() const {
    if (!m_RuntimeDataFunctionInfo ||
        m_RuntimeDataFunctionInfo->Resources == UINT_MAX)
      return 0;
    return m_Context->pIndexTableReader->getRow(
      m_RuntimeDataFunctionInfo->Resources).Count();
  }
  ResourceReader GetResource(uint32_t i) const {
    if (!m_RuntimeDataFunctionInfo)
      return ResourceReader(nullptr, m_Context);
    uint32_t resIndex = m_Context->pIndexTableReader->getRow(
      m_RuntimeDataFunctionInfo->Resources).At(i);
    return m_Context->pResourceTableReader->GetItem(resIndex);
  }
  uint32_t GetNumDependencies() const {
    if (!m_RuntimeDataFunctionInfo ||
        m_RuntimeDataFunctionInfo->FunctionDependencies == UINT_MAX)
      return 0;
    return m_Context->pIndexTableReader->getRow(
      m_RuntimeDataFunctionInfo->FunctionDependencies).Count();
  }
  const char *GetDependency(uint32_t i) const {
    if (!m_RuntimeDataFunctionInfo)
      return "";
    uint32_t resIndex = m_Context->pIndexTableReader->getRow(
      m_RuntimeDataFunctionInfo->FunctionDependencies).At(i);
    return m_Context->pStringTableReader->Get(resIndex);
  }

  uint32_t GetPayloadSizeInBytes() const {
    return !m_RuntimeDataFunctionInfo ? 0
      : m_RuntimeDataFunctionInfo->PayloadSizeInBytes;
  }
  uint32_t GetAttributeSizeInBytes() const {
    return !m_RuntimeDataFunctionInfo ? 0
      : m_RuntimeDataFunctionInfo->AttributeSizeInBytes;
  }
  // payload (hit shaders) and parameters (call shaders) are mutually exclusive
  uint32_t GetParameterSizeInBytes() const {
    return !m_RuntimeDataFunctionInfo ? 0
      : m_RuntimeDataFunctionInfo->PayloadSizeInBytes;
  }
  hlsl::DXIL::ShaderKind GetShaderKind() const {
    return !m_RuntimeDataFunctionInfo ? hlsl::DXIL::ShaderKind::Invalid
      : (hlsl::DXIL::ShaderKind)m_RuntimeDataFunctionInfo->ShaderKind;
  }
};

class FunctionTableReader {
private:
  TableReader m_Table;
  RuntimeDataContext *m_context;

public:
  FunctionTableReader() : m_context(nullptr) {}

  FunctionReader GetItem(uint32_t i) const {
    return FunctionReader(m_Table.Row<RuntimeDataFunctionInfo>(i), m_context);
  }
  uint32_t GetNumFunctions() const { return m_Table.Count(); }

  void SetFunctionInfo(const char *ptr, uint32_t count, uint32_t recordStride) {
    m_Table.Init(ptr, count, recordStride);
  }
  void SetContext(RuntimeDataContext *context) { m_context = context; }
};

class DxilRuntimeData {
private:
  uint32_t m_TableCount;
  StringTableReader m_StringReader;
  IndexTableReader m_IndexTableReader;
  ResourceTableReader m_ResourceTableReader;
  FunctionTableReader m_FunctionTableReader;
  RuntimeDataContext m_Context;

public:
  DxilRuntimeData();
  DxilRuntimeData(const char *ptr, size_t size);
  // initializing reader from RDAT. return true if no error has occured.
  bool InitFromRDAT(const void *pRDAT, size_t size);
  // read prerelease data:
  bool InitFromRDAT_Prerelease(const void *pRDAT, size_t size);
  FunctionTableReader *GetFunctionTableReader();
  ResourceTableReader *GetResourceTableReader();
};

//////////////////////////////////
/// structures for library runtime

typedef struct DxilResourceDesc {
  uint32_t Class; // hlsl::DXIL::ResourceClass
  uint32_t Kind;  // hlsl::DXIL::ResourceKind
  uint32_t ID;    // id per class
  uint32_t Space;
  uint32_t UpperBound;
  uint32_t LowerBound;
  LPCWSTR Name;
  uint32_t Flags; // hlsl::RDAT::DxilResourceFlag
} DxilResourceDesc;

typedef struct DxilFunctionDesc {
  LPCWSTR Name;
  LPCWSTR UnmangledName;
  uint32_t NumResources;
  const DxilResourceDesc * const*Resources;
  uint32_t NumFunctionDependencies;
  const LPCWSTR *FunctionDependencies;
  uint32_t ShaderKind;
  uint32_t PayloadSizeInBytes;   // 1) hit, miss, or closest shader: payload count
                                 // 2) call shader: parameter size
  uint32_t AttributeSizeInBytes; // attribute size for closest hit and any hit
  uint32_t FeatureInfo1;         // first 32 bits of feature flag
  uint32_t FeatureInfo2;         // second 32 bits of feature flag
  uint32_t ShaderStageFlag;      // valid shader stage flag.
  uint32_t MinShaderTarget;      // minimum shader target.
} DxilFunctionDesc;

typedef struct DxilSubobjectDesc {
} DxilSubobjectDesc;

typedef struct DxilLibraryDesc {
  uint32_t NumFunctions;
  DxilFunctionDesc *pFunction;
  uint32_t NumResources;
  DxilResourceDesc *pResource;
  uint32_t NumSubobjects;
  DxilSubobjectDesc *pSubobjects;
} DxilLibraryDesc;

class DxilRuntimeReflection {
public:
  virtual ~DxilRuntimeReflection() {}
  // This call will allocate memory for GetLibraryReflection call
  virtual bool InitFromRDAT(const void *pRDAT, size_t size) = 0;
  // DxilRuntimeReflection owns the memory pointed to by DxilLibraryDesc
  virtual const DxilLibraryDesc GetLibraryReflection() = 0;
};

DxilRuntimeReflection *CreateDxilRuntimeReflection();

} // namespace RDAT
} // namespace hlsl
