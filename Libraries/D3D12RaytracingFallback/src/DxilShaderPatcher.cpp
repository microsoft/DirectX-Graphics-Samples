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
#include "pch.h"

using namespace hlsl;
#define SPEW_SHADERS 0
namespace FallbackLayer
{
    void DxilShaderPatcher::VerifyResult(IDxcOperationResult *pResult)
    {
        HRESULT hr;
        ThrowInternalFailure(pResult->GetStatus(&hr));
        if (FAILED(hr))
        {
            CComPtr<IDxcBlobEncoding> pErrorText;
            pResult->GetErrorBuffer(&pErrorText);
            OutputDebugStringA((char *)pErrorText->GetBufferPointer());
        }
        ThrowFailure(hr, L"Failed to compile the shader");
    }

    void DxilShaderPatcher::CreateFallbackCompiler(IDxcDxrFallbackCompiler **ppCompiler)
    {
        ThrowFailure(dxcDxrFallbackSupport.CreateInstance(CLSID_DxcDxrFallbackCompiler, ppCompiler),
            L"Failed to create an instance of the Fallback Compiler. This suggests the version of DxrFallbackCompiler.dll "
            L"is being used that doesn't match up with the Fallback layer. Verify that the DxrFallbackCompiler.dll is from "
            L"same package as the Fallback.");
    }


    void DxilShaderPatcher::LinkCollection(UINT maxAttributeSize, const std::vector<DxilLibraryInfo> &dxilLibraries, const std::vector<LPCWSTR>& exportNames, std::vector<DxcShaderInfo>& shaderInfo, IDxcBlob** ppOutputBlob)
    {
        CComPtr<IDxcDxrFallbackCompiler> pFallbackCompiler;
        CreateFallbackCompiler(&pFallbackCompiler);

        std::vector<DxcShaderBytecode> pLibBlobPtrs(dxilLibraries.size());
        for (size_t i = 0; i < dxilLibraries.size(); ++i)
        {
            pLibBlobPtrs[i] = { (LPBYTE)dxilLibraries[i].pByteCode, (UINT32)dxilLibraries[i].BytecodeLength };
        }

        shaderInfo.resize(exportNames.size());
        CComPtr<IDxcOperationResult> pResult;
        pFallbackCompiler->Compile(pLibBlobPtrs.data(), (UINT32)pLibBlobPtrs.size(), exportNames.data(), shaderInfo.data(), (UINT32)exportNames.size(), maxAttributeSize, &pResult);

        VerifyResult(pResult);
        ThrowInternalFailure(pResult->GetResult(ppOutputBlob));
    }

    void DxilShaderPatcher::LinkStateObject(UINT maxAttributeSize, UINT stackSize, IDxcBlob* pLinkedBlob, const std::vector<LPCWSTR>& exportNames, std::vector<DxcShaderInfo>& shaderInfo, IDxcBlob** ppOutputBlob)
    {
        CComPtr<IDxcDxrFallbackCompiler> pFallbackCompiler;
        CreateFallbackCompiler(&pFallbackCompiler);

        shaderInfo.resize(exportNames.size());
        CComPtr<IDxcOperationResult> pResult;
        pFallbackCompiler->Link(L"main", &pLinkedBlob, 1, exportNames.data(), shaderInfo.data(), (UINT32)exportNames.size(), maxAttributeSize, stackSize, &pResult);

        VerifyResult(pResult);
        ThrowInternalFailure(pResult->GetResult(ppOutputBlob));
#ifdef DEBUG
        {
            //CComPtr<IDxcOperationResult> pValidatorResult;
            //GetValidator().Validate(*ppOutputBlob, DxcValidatorFlags_Default, &pValidatorResult);
            //VerifyResult(pValidatorResult);
        }

#if SPEW_SHADERS
        {
            CComPtr<IDxcBlobEncoding> pShaderText;
            m_pCompiler->Disassemble(*ppOutputBlob, &pShaderText);
            OutputDebugStringA((char *)pShaderText->GetBufferPointer());
            OutputDebugStringA("\n");
        }
#endif
#endif
    }

    void DxilShaderPatcher::RenameAndLink(const std::vector<DxilLibraryInfo> &dxilLibraries, std::vector<DxcExportDesc> exports, IDxcBlob** ppOutputBlob)
    {
        CComPtr<IDxcDxrFallbackCompiler> pFallbackCompiler;
        CreateFallbackCompiler(&pFallbackCompiler);

        CComPtr<IDxcOperationResult> pResult;
        std::vector<DxcShaderBytecode> pLibBlobPtrs(dxilLibraries.size());
        for (size_t i = 0; i < dxilLibraries.size(); ++i)
        {
            pLibBlobPtrs[i] = { (LPBYTE)dxilLibraries[i].pByteCode, (UINT32)dxilLibraries[i].BytecodeLength };
        }

        pFallbackCompiler->RenameAndLink(pLibBlobPtrs.data(), (UINT)pLibBlobPtrs.size(), exports.data(), (UINT)exports.size(), &pResult);

        VerifyResult(pResult);
        ThrowInternalFailure(pResult->GetResult(ppOutputBlob));
    }


    void DxilShaderPatcher::PatchShaderBindingTables(const BYTE *pShaderBytecode, UINT bytecodeLength, ShaderInfo *pShaderInfo, IDxcBlob** ppOutputBlob)
    {
        CComPtr<IDxcDxrFallbackCompiler> pFallbackCompiler;
        CreateFallbackCompiler(&pFallbackCompiler);

        CComPtr<IDxcOperationResult> pResult;
        DxcShaderBytecode shaderBytecode = { (LPBYTE)pShaderBytecode, bytecodeLength };
        pFallbackCompiler->PatchShaderBindingTables(pShaderInfo->ExportName, &shaderBytecode, pShaderInfo, &pResult);

        VerifyResult(pResult);
        ThrowInternalFailure(pResult->GetResult(ppOutputBlob));
    }
}
