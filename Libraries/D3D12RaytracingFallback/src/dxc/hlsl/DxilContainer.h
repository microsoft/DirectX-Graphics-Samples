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
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilContainer.h                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides declarations for the DXIL container format.                      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __DXC_CONTAINER__
#define __DXC_CONTAINER__

#include <stdint.h>
#include <iterator>
#include <functional>
#include "dxc/HLSL/DxilConstants.h"

struct IDxcContainerReflection;
namespace llvm { class Module; }

namespace hlsl {

class AbstractMemoryStream;
class RootSignatureHandle;
class DxilModule;

#pragma pack(push, 1)

static const size_t DxilContainerHashSize = 16;
static const uint16_t DxilContainerVersionMajor = 1;  // Current major version
static const uint16_t DxilContainerVersionMinor = 0;  // Current minor version
static const uint32_t DxilContainerMaxSize = 0x80000000; // Max size for container.

/// Use this type to represent the hash for the full container.
struct DxilContainerHash {
  uint8_t Digest[DxilContainerHashSize];
};

struct DxilContainerVersion {
  uint16_t Major;
  uint16_t Minor;
};

/// Use this type to describe a DXIL container of parts.
struct DxilContainerHeader {
  uint32_t              HeaderFourCC;
  DxilContainerHash     Hash;
  DxilContainerVersion  Version;
  uint32_t              ContainerSizeInBytes; // From start of this header
  uint32_t              PartCount;
  // Structure is followed by uint32_t PartOffset[PartCount];
  // The offset is to a DxilPartHeader.
};

/// Use this type to describe the size and type of a DXIL container part.
struct DxilPartHeader {
  uint32_t  PartFourCC; // Four char code for part type.
  uint32_t  PartSize;   // Byte count for PartData.
  // Structure is followed by uint8_t PartData[PartSize].
};

#define DXIL_FOURCC(ch0, ch1, ch2, ch3) (                            \
  (uint32_t)(uint8_t)(ch0)        | (uint32_t)(uint8_t)(ch1) << 8  | \
  (uint32_t)(uint8_t)(ch2) << 16  | (uint32_t)(uint8_t)(ch3) << 24   \
  )

enum DxilFourCC {
  DFCC_Container                = DXIL_FOURCC('D', 'X', 'B', 'C'), // for back-compat with tools that look for DXBC containers
  DFCC_ResourceDef              = DXIL_FOURCC('R', 'D', 'E', 'F'),
  DFCC_InputSignature           = DXIL_FOURCC('I', 'S', 'G', '1'),
  DFCC_OutputSignature          = DXIL_FOURCC('O', 'S', 'G', '1'),
  DFCC_PatchConstantSignature   = DXIL_FOURCC('P', 'S', 'G', '1'),
  DFCC_ShaderStatistics         = DXIL_FOURCC('S', 'T', 'A', 'T'),
  DFCC_ShaderDebugInfoDXIL      = DXIL_FOURCC('I', 'L', 'D', 'B'),
  DFCC_ShaderDebugName          = DXIL_FOURCC('I', 'L', 'D', 'N'),
  DFCC_FeatureInfo              = DXIL_FOURCC('S', 'F', 'I', '0'),
  DFCC_PrivateData              = DXIL_FOURCC('P', 'R', 'I', 'V'),
  DFCC_RootSignature            = DXIL_FOURCC('R', 'T', 'S', '0'),
  DFCC_DXIL                     = DXIL_FOURCC('D', 'X', 'I', 'L'),
  DFCC_PipelineStateValidation  = DXIL_FOURCC('P', 'S', 'V', '0'),
};

#undef DXIL_FOURCC

// DFCC_FeatureInfo is a uint64_t value with these flags.
static const uint64_t ShaderFeatureInfo_Doubles = 0x0001;
static const uint64_t ShaderFeatureInfo_ComputeShadersPlusRawAndStructuredBuffersViaShader4X = 0x0002;
static const uint64_t ShaderFeatureInfo_UAVsAtEveryStage = 0x0004;
static const uint64_t ShaderFeatureInfo_64UAVs = 0x0008;
static const uint64_t ShaderFeatureInfo_MinimumPrecision = 0x0010;
static const uint64_t ShaderFeatureInfo_11_1_DoubleExtensions = 0x0020;
static const uint64_t ShaderFeatureInfo_11_1_ShaderExtensions = 0x0040;
static const uint64_t ShaderFeatureInfo_LEVEL9ComparisonFiltering = 0x0080;
static const uint64_t ShaderFeatureInfo_TiledResources = 0x0100;
static const uint64_t ShaderFeatureInfo_StencilRef = 0x0200;
static const uint64_t ShaderFeatureInfo_InnerCoverage = 0x0400;
static const uint64_t ShaderFeatureInfo_TypedUAVLoadAdditionalFormats = 0x0800;
static const uint64_t ShaderFeatureInfo_ROVs = 0x1000;
static const uint64_t ShaderFeatureInfo_ViewportAndRTArrayIndexFromAnyShaderFeedingRasterizer = 0x2000;
static const uint64_t ShaderFeatureInfo_WaveOps = 0x4000;
static const uint64_t ShaderFeatureInfo_Int64Ops = 0x8000;
static const uint64_t ShaderFeatureInfo_ViewID = 0x10000;
static const uint64_t ShaderFeatureInfo_Barycentrics = 0x20000;
static const uint64_t ShaderFeatureInfo_NativeLowPrecision = 0x40000;

static const unsigned ShaderFeatureInfoCount = 19;

struct DxilShaderFeatureInfo {
  uint64_t FeatureFlags;
};

// DXIL program information.
struct DxilBitcodeHeader {
  uint32_t DxilMagic;       // ACSII "DXIL".
  uint32_t DxilVersion;     // DXIL version.
  uint32_t BitcodeOffset;   // Offset to LLVM bitcode (from start of header).
  uint32_t BitcodeSize;     // Size of LLVM bitcode.
};
static const uint32_t DxilMagicValue = 0x4C495844; // 'DXIL'

struct DxilProgramHeader {
  uint32_t          ProgramVersion;   /// Major and minor version, including type.
  uint32_t          SizeInUint32;     /// Size in uint32_t units including this header.
  DxilBitcodeHeader BitcodeHeader;    /// Bitcode-specific header.
  // Followed by uint8_t[BitcodeHeader.BitcodeOffset]
};

struct DxilProgramSignature {
  uint32_t ParamCount;
  uint32_t ParamOffset;
};

enum class DxilProgramSigMinPrecision : uint32_t {
  Default = 0,
  Float16 = 1,
  Float2_8 = 2,
  Reserved = 3,
  SInt16 = 4,
  UInt16 = 5,
  Any16 = 0xf0,
  Any10 = 0xf1
};

// Corresponds to D3D_NAME and D3D10_SB_NAME
enum class DxilProgramSigSemantic : uint32_t {
  Undefined = 0,
  Position = 1,
  ClipDistance = 2,
  CullDistance = 3,
  RenderTargetArrayIndex = 4,
  ViewPortArrayIndex = 5,
  VertexID = 6,
  PrimitiveID = 7,
  InstanceID = 8,
  IsFrontFace = 9,
  SampleIndex = 10,
  FinalQuadEdgeTessfactor = 11,
  FinalQuadInsideTessfactor = 12,
  FinalTriEdgeTessfactor = 13,
  FinalTriInsideTessfactor = 14,
  FinalLineDetailTessfactor = 15,
  FinalLineDensityTessfactor = 16,
  Barycentrics = 23,
  Target = 64,
  Depth = 65,
  Coverage = 66,
  DepthGE = 67,
  DepthLE = 68,
  StencilRef = 69,
  InnerCoverage = 70,
};

enum class DxilProgramSigCompType : uint32_t {
  Unknown = 0,
  UInt32 = 1,
  SInt32 = 2,
  Float32 = 3,
  UInt16 = 4,
  SInt16 = 5,
  Float16 = 6,
  UInt64 = 7,
  SInt64 = 8,
  Float64 = 9,
};

static const uint8_t DxilProgramSigMaskX = 1;
static const uint8_t DxilProgramSigMaskY = 2;
static const uint8_t DxilProgramSigMaskZ = 4;
static const uint8_t DxilProgramSigMaskW = 8;

struct DxilProgramSignatureElement {
  uint32_t Stream;                    // Stream index (parameters must appear in non-decreasing stream order)
  uint32_t SemanticName;              // Offset to LPCSTR from start of DxilProgramSignature.
  uint32_t SemanticIndex;             // Semantic Index
  DxilProgramSigSemantic SystemValue; // Semantic type. Similar to DxilSemantic::Kind, but a serialized rather than processing rep.
  DxilProgramSigCompType CompType;    // Type of bits.
  uint32_t Register;                  // Register Index (row index)
  uint8_t Mask;                       // Mask (column allocation)
  union                         // Unconditional cases useful for validation of shader linkage.
  {
    uint8_t NeverWrites_Mask;   // For an output signature, the shader the signature belongs to never
                                // writes the masked components of the output register.
    uint8_t AlwaysReads_Mask;   // For an input signature, the shader the signature belongs to always
                                // reads the masked components of the input register.
  };
  uint16_t Pad;
  DxilProgramSigMinPrecision MinPrecision; // Minimum precision of input/output data
};

// Easy to get this wrong. Earlier assertions can help determine
static_assert(sizeof(DxilProgramSignatureElement) == 0x20, "else DxilProgramSignatureElement is misaligned");

struct DxilShaderDebugName {
  uint16_t Flags;       // Reserved, must be set to zero.
  uint16_t NameLength;  // Length of the debug name, without null terminator.
  // Followed by NameLength bytes of the UTF-8-encoded name.
  // Followed by a null terminator.
  // Followed by [0-3] zero bytes to align to a 4-byte boundary.
};
static const size_t MinDxilShaderDebugNameSize = sizeof(DxilShaderDebugName) + 4;

#pragma pack(pop)

/// Gets a part header by index.
inline const DxilPartHeader *
GetDxilContainerPart(const DxilContainerHeader *pHeader, uint32_t index) {
  const uint8_t *pLinearContainer = reinterpret_cast<const uint8_t *>(pHeader);
  const uint32_t *pPartOffsetTable =
      reinterpret_cast<const uint32_t *>(pHeader + 1);
  return reinterpret_cast<const DxilPartHeader *>(
      pLinearContainer + pPartOffsetTable[index]);
}

/// Gets a part header by index.
inline DxilPartHeader *GetDxilContainerPart(DxilContainerHeader *pHeader,
                                            uint32_t index) {
  return const_cast<DxilPartHeader *>(GetDxilContainerPart(
      reinterpret_cast<const DxilContainerHeader *>(pHeader), index));
}

/// Gets the part data from the header.
inline const char *GetDxilPartData(const DxilPartHeader *pPart) {
  return reinterpret_cast<const char *>(pPart + 1);
}

/// Gets the part data from the header.
inline char *GetDxilPartData(DxilPartHeader *pPart) {
  return reinterpret_cast<char *>(pPart + 1);
}
/// Gets a part header by fourCC
DxilPartHeader *GetDxilPartByType(DxilContainerHeader *pHeader,
                                           DxilFourCC fourCC);
/// Gets a part header by fourCC 
const DxilPartHeader *
GetDxilPartByType(const DxilContainerHeader *pHeader,
                           DxilFourCC fourCC);

/// Returns valid DxilProgramHeader. nullptr if does not exist.
DxilProgramHeader *GetDxilProgramHeader(DxilContainerHeader *pHeader, DxilFourCC fourCC);

/// Returns valid DxilProgramHeader. nullptr if does not exist.
const DxilProgramHeader *
GetDxilProgramHeader(const DxilContainerHeader *pHeader, DxilFourCC fourCC);

/// Initializes container with the specified values.
void InitDxilContainer(_Out_ DxilContainerHeader *pHeader, uint32_t partCount,
                       uint32_t containerSizeInBytes);

/// Checks whether pHeader claims by signature to be a DXIL container.
const DxilContainerHeader *IsDxilContainerLike(const void *ptr, size_t length);

/// Checks whether the DXIL container is valid and in-bounds.
bool IsValidDxilContainer(const DxilContainerHeader *pHeader, size_t length);

/// Use this type as a unary predicate functor.
struct DxilPartIsType {
  uint32_t IsFourCC;
  DxilPartIsType(uint32_t FourCC) : IsFourCC(FourCC) { }
  bool operator()(const DxilPartHeader *pPart) const {
    return pPart->PartFourCC == IsFourCC;
  }
};

/// Use this type as an iterator over the part headers.
struct DxilPartIterator : public std::iterator<std::input_iterator_tag,
                                               const DxilContainerHeader *> {
  const DxilContainerHeader *pHeader;
  uint32_t index;

  DxilPartIterator(const DxilContainerHeader *h, uint32_t i)
      : pHeader(h), index(i) {}

  // increment
  DxilPartIterator &operator++() {
    ++index;
    return *this;
  }
  DxilPartIterator operator++(int) {
    DxilPartIterator result(pHeader, index);
    ++index;
    return result;
  }

  // input iterator - compare and deref
  bool operator==(const DxilPartIterator &other) const {
    return index == other.index && pHeader == other.pHeader;
  }
  bool operator!=(const DxilPartIterator &other) const {
    return index != other.index || pHeader != other.pHeader;
  }
  const DxilPartHeader *operator*() const {
    return GetDxilContainerPart(pHeader, index);
  }
};

DxilPartIterator begin(const DxilContainerHeader *pHeader);
DxilPartIterator end(const DxilContainerHeader *pHeader);

inline bool IsValidDxilBitcodeHeader(const DxilBitcodeHeader *pHeader,
                                     uint32_t length) {
  return length > sizeof(DxilBitcodeHeader) &&
         pHeader->BitcodeOffset + pHeader->BitcodeSize >
             pHeader->BitcodeOffset &&
         length >= pHeader->BitcodeOffset + pHeader->BitcodeSize &&
         pHeader->DxilMagic == DxilMagicValue;
}

inline void InitBitcodeHeader(DxilBitcodeHeader &header,
  uint32_t dxilVersion,
  uint32_t bitcodeSize) {
  header.DxilMagic = DxilMagicValue;
  header.DxilVersion = dxilVersion;
  header.BitcodeOffset = sizeof(DxilBitcodeHeader);
  header.BitcodeSize = bitcodeSize;
}

inline void GetDxilProgramBitcode(const DxilProgramHeader *pHeader,
                                  const char **pBitcode,
                                  uint32_t *pBitcodeLength) {
  *pBitcode = reinterpret_cast<const char *>(&pHeader->BitcodeHeader) +
              pHeader->BitcodeHeader.BitcodeOffset;
  *pBitcodeLength = pHeader->BitcodeHeader.BitcodeSize;
}

inline bool IsValidDxilProgramHeader(const DxilProgramHeader *pHeader,
  uint32_t length) {
  return length >= sizeof(DxilProgramHeader) &&
    length >= (pHeader->SizeInUint32 * sizeof(uint32_t)) &&
    IsValidDxilBitcodeHeader(
      &pHeader->BitcodeHeader,
      length - offsetof(DxilProgramHeader, BitcodeHeader));
}

inline void InitProgramHeader(DxilProgramHeader &header, uint32_t shaderVersion,
                              uint32_t dxilVersion,
                              uint32_t bitcodeSize) {
  header.ProgramVersion = shaderVersion;
  header.SizeInUint32 =
    sizeof(DxilProgramHeader) / sizeof(uint32_t) +
    bitcodeSize / sizeof(uint32_t) + ((bitcodeSize % 4) ? 1 : 0);
  InitBitcodeHeader(header.BitcodeHeader, dxilVersion, bitcodeSize);
}

inline const char *GetDxilBitcodeData(const DxilProgramHeader *pHeader) {
  const DxilBitcodeHeader *pBCHdr = &(pHeader->BitcodeHeader);
  return (const char *)pBCHdr + pBCHdr->BitcodeOffset;
}

inline uint32_t GetDxilBitcodeSize(const DxilProgramHeader *pHeader) {
  return pHeader->BitcodeHeader.BitcodeSize;
}

/// Extract the shader type from the program version value.
inline DXIL::ShaderKind GetVersionShaderType(uint32_t programVersion) {
  return (DXIL::ShaderKind)((programVersion & 0xffff0000) >> 16);
}
inline uint32_t GetVersionMajor(uint32_t programVersion) {
  return (programVersion & 0xf0) >> 4;
}
inline uint32_t GetVersionMinor(uint32_t programVersion) {
  return (programVersion & 0xf);
}
inline uint32_t EncodeVersion(DXIL::ShaderKind shaderType, uint32_t major,
  uint32_t minor) {
  return ((unsigned)shaderType << 16) | (major << 4) | minor;
}

inline bool IsDxilShaderDebugNameValid(const DxilPartHeader *pPart) {
  if (pPart->PartFourCC != DFCC_ShaderDebugName) return false;
  if (pPart->PartSize < MinDxilShaderDebugNameSize) return false;
  const DxilShaderDebugName *pDebugNameContent = reinterpret_cast<const DxilShaderDebugName *>(GetDxilPartData(pPart));
  uint16_t ExpectedSize = sizeof(DxilShaderDebugName) + pDebugNameContent->NameLength + 1;
  if (ExpectedSize & 0x3) {
    ExpectedSize += 0x4;
    ExpectedSize &= ~(0x3);
  }
  if (pPart->PartSize != ExpectedSize) return false;
  return true;
}

inline bool GetDxilShaderDebugName(const DxilPartHeader *pDebugNamePart,
  const char **ppUtf8Name, _Out_opt_ uint16_t *pUtf8NameLen) {
  *ppUtf8Name = nullptr;
  if (!IsDxilShaderDebugNameValid(pDebugNamePart)) {
    return false;
  }
  const DxilShaderDebugName *pDebugNameContent = reinterpret_cast<const DxilShaderDebugName *>(GetDxilPartData(pDebugNamePart));
  if (pUtf8NameLen) {
    *pUtf8NameLen = pDebugNameContent->NameLength;
  }
  *ppUtf8Name = (const char *)(pDebugNameContent + 1);
  return true;
}

class DxilPartWriter {
public:
  virtual ~DxilPartWriter() {}
  virtual uint32_t size() const = 0;
  virtual void write(AbstractMemoryStream *pStream) = 0;
};

DxilPartWriter *NewProgramSignatureWriter(const DxilModule &M, DXIL::SignatureKind Kind);
DxilPartWriter *NewRootSignatureWriter(const RootSignatureHandle &S);
DxilPartWriter *NewFeatureInfoWriter(const DxilModule &M);
DxilPartWriter *NewPSVWriter(const DxilModule &M, uint32_t PSVVersion = 0);

class DxilContainerWriter : public DxilPartWriter  {
public:
  typedef std::function<void(AbstractMemoryStream*)> WriteFn;
  virtual ~DxilContainerWriter() {}
  virtual void AddPart(uint32_t FourCC, uint32_t Size, WriteFn Write) = 0;
};

DxilContainerWriter *NewDxilContainerWriter();

enum class SerializeDxilFlags : uint32_t {
  None = 0,                     // No flags defined.
  IncludeDebugInfoPart = 1,     // Include the debug info part in the container.
  IncludeDebugNamePart = 2,     // Include the debug name part in the container.
  DebugNameDependOnSource = 4   // Make the debug name depend on source (and not just final module).
};
inline SerializeDxilFlags& operator |=(SerializeDxilFlags& l, const SerializeDxilFlags& r) {
  l = static_cast<SerializeDxilFlags>(static_cast<int>(l) | static_cast<int>(r));
  return l;
}
inline SerializeDxilFlags& operator &=(SerializeDxilFlags& l, const SerializeDxilFlags& r) {
  l = static_cast<SerializeDxilFlags>(static_cast<int>(l) & static_cast<int>(r));
  return l;
}
inline int operator&(SerializeDxilFlags l, SerializeDxilFlags r) {
  return static_cast<int>(l) & static_cast<int>(r);
}
inline SerializeDxilFlags operator~(SerializeDxilFlags l) {
  return static_cast<SerializeDxilFlags>(~static_cast<uint32_t>(l));
}

void SerializeDxilContainerForModule(hlsl::DxilModule *pModule,
                                     AbstractMemoryStream *pModuleBitcode,
                                     AbstractMemoryStream *pStream,
                                     SerializeDxilFlags Flags);
void SerializeDxilContainerForRootSignature(hlsl::RootSignatureHandle *pRootSigHandle,
                                     AbstractMemoryStream *pStream);

void CreateDxcContainerReflection(IDxcContainerReflection **ppResult);

// Converts uint32_t partKind to char array object.
inline char * PartKindToCharArray(uint32_t partKind, _Out_writes_(5) char* pText) {
  pText[0] = (char)((partKind & 0x000000FF) >> 0);
  pText[1] = (char)((partKind & 0x0000FF00) >> 8);
  pText[2] = (char)((partKind & 0x00FF0000) >> 16);
  pText[3] = (char)((partKind & 0xFF000000) >> 24);
  pText[4] = '\0';
  return pText;
}

inline size_t GetOffsetTableSize(uint32_t partCount) {
  return sizeof(uint32_t) * partCount;
}
// Compute total size of the dxil container from parts information
inline size_t GetDxilContainerSizeFromParts(uint32_t partCount, uint32_t partsSize) {
  return partsSize + (uint32_t)sizeof(DxilContainerHeader) +
         GetOffsetTableSize(partCount) +
         (uint32_t)sizeof(DxilPartHeader) * partCount;
}

} // namespace hlsl

#endif // __DXC_CONTAINER__
