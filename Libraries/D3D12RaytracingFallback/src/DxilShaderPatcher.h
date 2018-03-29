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

#include "RaytracingProgram.h"

namespace FallbackLayer
{
    struct DxilLibraryInfo {
        DxilLibraryInfo(void *pInputBytecode, size_t InputBytecodeLength) :
            pByteCode(pInputBytecode), BytecodeLength(InputBytecodeLength) {}

        void *pByteCode;
        size_t BytecodeLength;
    };

    class DxilShaderPatcher
    {
    public:
        DxilShaderPatcher()
        {
            ThrowFailure(dxcSupport.Initialize(), 
                L"Failed to load DxCompiler.dll, verify this executable is in the executable directory."
                L" The Fallback Layer is sensitive to the DxCompiler.dll version, make sure the" 
                L" DxCompiler.dll is the correct version packaged with the Fallback");

            ThrowFailure(dxcSupport.CreateInstance(CLSID_DxcLibrary, &m_pLibrary), L"Failed to load a DXC library instance");
            ThrowFailure(dxcSupport.CreateInstance(CLSID_DxcOptimizer, &m_pOptimizer), L"Failed to load a DXC Optimizer instance");
            ThrowFailure(dxcSupport.CreateInstance(CLSID_DxcAssembler, &m_pAssembler), L"Failed to load a DXC Assembler instance");
            ThrowFailure(dxcSupport.CreateInstance(CLSID_DxcContainerBuilder, &m_pContainerBuilder), L"Failed to load a DXC ContainerBuilder instance");
            ThrowFailure(dxcSupport.CreateInstance(CLSID_DxcContainerReflection, &m_pContainerReflection), L"Failed to load a DXC ContainterReflection instance");
            ThrowFailure(dxcSupport.CreateInstance(CLSID_DxcValidator, &m_pValidator), L"Failed to load a DXC Validator instance");

#ifdef DEBUG
            ThrowFailure(dxcSupport.CreateInstance(CLSID_DxcCompiler, &m_pCompiler));
#endif
        }

        void PatchShaderBindingTables(const BYTE *pShaderBytecode, UINT bytecodeLength, ShaderInfo *pShaderInfo, IDxcBlob** ppOutputBlob);
        
        void LinkShaders(UINT stackSize, const std::vector<DxilLibraryInfo> &dxilLibraries, const std::vector<LPCWSTR>& exportNames, std::vector<FallbackLayer::StateIdentifier>& shaderIdentifiers, IDxcBlob** ppOutputBlob);

        IDxcValidator &GetValidator() { return *m_pValidator; }
    private:
        void VerifyResult(IDxcOperationResult *pResult);

        // These DXIL helper functions were shamelessly stolen from PIX
        void ReplaceDxilBlobPart(
            const void * originalShaderBytecode,
            SIZE_T originalShaderLength,
            IDxcBlob * pNewDxilBlob,
            IDxcBlob** ppNewShaderOut);

        HRESULT DxilGetBlobPartImpl(
            _In_reads_bytes_(SrcDataSize) LPCVOID pSrcData,
            _In_ SIZE_T SrcDataSize,
            _In_ hlsl::DxilFourCC targetFourCC,
            _Out_ const UINT **ppTargetBlobStart,
            _Out_ IDxcBlob **ppTargetBlob);

        void GetDxilBlobPart(
            _In_reads_bytes_(SrcDataSize) const void * pSrcData,
            _In_ SIZE_T SrcDataSize,
            _Out_ IDxcBlob **ppTargetBlob);

        dxc::DxcDllSupport dxcSupport;

        CComPtr<IDxcOptimizer> m_pOptimizer;
        CComPtr<IDxcLibrary> m_pLibrary;
        CComPtr<IDxcAssembler> m_pAssembler;
        CComPtr<IDxcContainerBuilder> m_pContainerBuilder;
        CComPtr<IDxcContainerReflection> m_pContainerReflection;
        CComPtr<IDxcValidator> m_pValidator;

#ifdef DEBUG
        CComPtr<IDxcCompiler> m_pCompiler;
#endif
    };
}
