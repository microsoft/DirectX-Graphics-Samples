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


    void DxilShaderPatcher::ReplaceDxilBlobPart(
        const void * originalShaderBytecode,
        SIZE_T originalShaderLength,
        IDxcBlob * pNewDxilBlob,
        IDxcBlob** ppNewShaderOut)
    {
        CComPtr<IDxcBlob> pNewContainer;

        // Use the container assembler to build a new container from the recently-modified DXIL bitcode. This container will
        // contain new copies of things like input signature etc., which will supersede the ones from the original
        // compiled shader's container.
        {
            CComPtr<IDxcOperationResult> pAssembleResult;
            ThrowInternalFailure(m_pAssembler->AssembleToContainer(pNewDxilBlob, &pAssembleResult));

            CComPtr<IDxcBlobEncoding> pAssembleErrors;
            ThrowInternalFailure(pAssembleResult->GetErrorBuffer(&pAssembleErrors));

            if (pAssembleErrors && pAssembleErrors->GetBufferSize() != 0)
            {
                OutputDebugStringW(static_cast<LPCWSTR>(pAssembleErrors->GetBufferPointer()));
                ThrowFailure(E_FAIL, L"Failed to assemble a DXC Container");
            }

            ThrowInternalFailure(pAssembleResult->GetResult(&pNewContainer));
        }

        // Now copy over the blobs from the original container that won't have been invalidated by changing the shader
        // code itself, using the container reflection API
        {
            // Wrap the original code in a container blob
            CComPtr<IDxcBlobEncoding> pContainer;
            ThrowFailure(m_pLibrary->CreateBlobWithEncodingFromPinned(
                static_cast<LPBYTE>(const_cast<void*>(originalShaderBytecode)),
                static_cast<UINT32>(originalShaderLength),
                CP_ACP,
                &pContainer),
                L"Failed to create a blob for the provided shader");

            // Load the reflector from the original shader
            ThrowInternalFailure(m_pContainerReflection->Load(pContainer));

            UINT32 partIndex;

            if (SUCCEEDED(m_pContainerReflection->FindFirstPartKind(hlsl::DFCC_PrivateData, &partIndex)))
            {
                CComPtr<IDxcBlob> pPart;
                ThrowInternalFailure(m_pContainerReflection->GetPartContent(partIndex, &pPart));

                ThrowInternalFailure(m_pContainerBuilder->Load(pNewContainer));

                ThrowInternalFailure(m_pContainerBuilder->AddPart(hlsl::DFCC_PrivateData, pPart));

                CComPtr<IDxcOperationResult> pBuildResult;

                ThrowInternalFailure(m_pContainerBuilder->SerializeContainer(&pBuildResult));

                CComPtr<IDxcBlobEncoding> pBuildErrors;
                ThrowInternalFailure(pBuildResult->GetErrorBuffer(&pBuildErrors));

                if (pBuildErrors && pBuildErrors->GetBufferSize() != 0)
                {
                    OutputDebugStringW(reinterpret_cast<LPCWSTR>(pBuildErrors->GetBufferPointer()));
                    ThrowFailure(E_FAIL, L"Failed to serialize DXC container");
                }

                ThrowInternalFailure(pBuildResult->GetResult(&pNewContainer));
                pNewContainer.Release();
            }
        }

        *ppNewShaderOut = pNewContainer.Detach();
    }

    HRESULT DxilShaderPatcher::DxilGetBlobPartImpl(
        _In_reads_bytes_(SrcDataSize) LPCVOID pSrcData,
        _In_ SIZE_T SrcDataSize,
        _In_ DxilFourCC targetFourCC,
        _Out_ const UINT **ppTargetBlobStart,
        _Out_ IDxcBlob **ppTargetBlob)
    {
        if (SrcDataSize < sizeof(DxilContainerHeader))
        {
            return E_INVALIDARG;
        }

        const BYTE* pSourceBytes = reinterpret_cast<const BYTE*>(pSrcData);
        const DxilContainerHeader* pContainerHeader = reinterpret_cast<const DxilContainerHeader*>(pSourceBytes);

        // According to DxilContainer.h the container header is followed by uint32_t PartOffset[PartCount] containing offsets to each DxilPartHeader.
        const UINT32* pPartOffsets = reinterpret_cast<const UINT32*>(pSourceBytes + sizeof(DxilContainerHeader));

        for (uint32_t i = 0; i < pContainerHeader->PartCount; ++i)
        {
            const DxilPartHeader* pDxilPartHeader = reinterpret_cast<const DxilPartHeader*>(pSourceBytes + pPartOffsets[i]);
            if (pDxilPartHeader->PartFourCC == (UINT)targetFourCC)
            {
                // Create a dxc blob from the source data.
                CComPtr<IDxcBlobEncoding> pSource;
                HRESULT hr = m_pLibrary->CreateBlobWithEncodingFromPinned((LPBYTE)pSrcData, static_cast<UINT32>(SrcDataSize), CP_ACP, &pSource);
                if (FAILED(hr)) return hr;

                // Determine the part offset.
                DxilProgramHeader *pDxilProgramHeader = (DxilProgramHeader*)(pDxilPartHeader + 1);
                const char* pBitcode = reinterpret_cast<const char *>(&pDxilProgramHeader->BitcodeHeader) + pDxilProgramHeader->BitcodeHeader.BitcodeOffset;
                UINT32 bitcodeLength = pDxilProgramHeader->BitcodeHeader.BitcodeSize;
                UINT32 bitcodeOffset = (UINT32)(pBitcode - (const char *)pSrcData);

                if (ppTargetBlobStart != nullptr)
                {
                    *ppTargetBlobStart = (UINT*)pDxilProgramHeader;
                }

                if (ppTargetBlob != nullptr)
                {
                    return m_pLibrary->CreateBlobFromBlob(pSource, bitcodeOffset, bitcodeLength, ppTargetBlob);
                }

                return S_OK;
            }
        }

        return E_FAIL;
    }

    void DxilShaderPatcher::GetDxilBlobPart(
        _In_reads_bytes_(SrcDataSize) const void * pSrcData,
        _In_ SIZE_T SrcDataSize,
        _Out_ IDxcBlob **ppTargetBlob)
    {
        ThrowInternalFailure(DxilGetBlobPartImpl(
            pSrcData,
            SrcDataSize,
            hlsl::DFCC_DXIL,
            nullptr,
            ppTargetBlob));
    }

    void DxilShaderPatcher::LinkShaders(UINT stackSize, const std::vector<DxilLibraryInfo> &dxilLibraries, const std::vector<LPCWSTR>& exportNames, std::vector<FallbackLayer::StateIdentifier>& shaderIdentifiers, IDxcBlob** ppOutputBlob)
    {
        CComPtr<IDxcDxrFallbackCompiler> pFallbackCompiler;
        ThrowFailure(dxcSupport.CreateInstance(CLSID_DxcDxrFallbackCompiler, &pFallbackCompiler), 
            L"Failed to create an instance of the Fallback Compiler. This suggest a version of DxCompiler.dll "
            L"is being used that doesn't match up with the Fallback layer. Verify that the DxCompiler.dll is from "
            L"same package as the Fallback.");

        std::vector<CComPtr<IDxcBlobEncoding>> pLibBlobs(dxilLibraries.size());
        std::vector<IDxcBlob*> pLibBlobPtrs(dxilLibraries.size());
        for (size_t i=0; i < dxilLibraries.size(); ++i)
        {

          ThrowInternalFailure(m_pLibrary->CreateBlobWithEncodingFromPinned(
                static_cast<LPBYTE>(dxilLibraries[i].pByteCode),
                static_cast<UINT32>(dxilLibraries[i].BytecodeLength),
                CP_ACP,
                &pLibBlobs[i]));
          pLibBlobPtrs[i] = pLibBlobs[i];
        }

        shaderIdentifiers.resize(exportNames.size());
        CComPtr<IDxcOperationResult> pResult;
        pFallbackCompiler->Compile(L"main", pLibBlobPtrs.data(), (UINT32)pLibBlobPtrs.size(), exportNames.data(), (int*)shaderIdentifiers.data(), (UINT32)exportNames.size(), stackSize, &pResult);

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

    void DxilShaderPatcher::PatchShaderBindingTables(const BYTE *pShaderBytecode, UINT bytecodeLength, ShaderInfo *pShaderInfo, IDxcBlob** ppOutputBlob)
    {
        dxc::DxcDllSupport dxcSupport;
        ThrowFailure(dxcSupport.Initialize(), 
            L"Failed to load DxCompiler.dll, verify this executable is in the executable directory."
            L" The Fallback Layer is sensitive to the DxCompiler.dll version, make sure the"
            L" DxCompiler.dll is the correct version packaged with the Fallback");

        CComPtr<IDxcBlob> pShaderBlob;
        GetDxilBlobPart(pShaderBytecode, bytecodeLength, &pShaderBlob);

        CComPtr<IDxcOperationResult> pResult;
        CComPtr<IDxcBlob> pPatchedBlob;
        CComPtr<IDxcBlobEncoding> pErrorMessage;

        wchar_t dxilPatchShaderRecordString[200];
        ThrowFailure(StringCchPrintfW(dxilPatchShaderRecordString, _countof(dxilPatchShaderRecordString),
            L"-hlsl-dxil-patch-shader-record-bindings,root-signature=%p",
            pShaderInfo));

        std::vector<LPCWSTR> arguments;
        arguments.push_back(dxilPatchShaderRecordString);
        HRESULT hr = m_pOptimizer->RunOptimizer(pShaderBlob, arguments.data(), (UINT32)arguments.size(), &pPatchedBlob, &pErrorMessage);

#if SPEW_SHADERS
        CComPtr<IDxcBlobEncoding> pShaderText;
        m_pCompiler->Disassemble(pPatchedBlob, &pShaderText);
        OutputDebugStringA((char *)pShaderText->GetBufferPointer());
#endif

        if (FAILED(hr))
        {
            std::wstring ret;
            auto pChars = reinterpret_cast<const std::wstring::value_type *>(pErrorMessage->GetBufferPointer());
            ret.assign(pChars, pChars + pErrorMessage->GetBufferSize());
            OutputDebugString(ret.c_str());

            ThrowFailure(hr, L"Failed to apply shader record bindings from the local root signature");
        }

        ReplaceDxilBlobPart(pShaderBytecode, bytecodeLength, pPatchedBlob, ppOutputBlob);
    }
}
