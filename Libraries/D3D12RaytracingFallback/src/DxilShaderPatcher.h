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
            ThrowFailure(dxcDxrFallbackSupport.InitializeForDll(L"DxrFallbackCompiler.dll", "DxcCreateDxrFallbackCompiler"),
              L"Failed to load DxrFallbackCompiler.dll, verify this executable is in the executable directory."
              L" The Fallback Layer is sensitive to the DxrFallbackCompiler.dll version, make sure the"
              L" DxrFallbackCompiler.dll is the correct version packaged with the Fallback");
        }

        void RenameAndLink(const std::vector<DxilLibraryInfo> &dxilLibraries, std::vector<DxcExportDesc> exports, IDxcBlob** ppOutputBlob);
        void PatchShaderBindingTables(const BYTE *pShaderBytecode, UINT bytecodeLength, ShaderInfo *pShaderInfo, IDxcBlob** ppOutputBlob);
        
        void LinkCollection(UINT maxAttributeSize, const std::vector<DxilLibraryInfo> &dxilLibraries, const std::vector<LPCWSTR>& exportNames, std::vector<DxcShaderInfo>& shaderInfo, IDxcBlob** ppOutputBlob);
        void LinkStateObject(UINT maxAttributeSize, UINT stackSize, IDxcBlob* pLinkedBlob, const std::vector<LPCWSTR>& exportNames, std::vector<DxcShaderInfo>& shaderInfo, IDxcBlob** ppOutputBlob);

    private:
        void VerifyResult(IDxcOperationResult *pResult);

        dxc::DxcDllSupport dxcDxrFallbackSupport;
        void CreateFallbackCompiler(IDxcDxrFallbackCompiler **ppCompiler);


#ifdef DEBUG
        CComPtr<IDxcCompiler> m_pCompiler;
#endif
    };
}
