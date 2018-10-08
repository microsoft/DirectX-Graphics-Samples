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
#define ThrowInternalFailure(expression) ThrowFailure(expression, L"Unexpected internal Failure: " #expression)

inline void ThrowFailure(HRESULT hr, LPCWSTR errorString = nullptr)
{
    if (FAILED(hr))
    {
        if (errorString)
        {
            OutputDebugString(L"\n");
            OutputDebugString(L"D3D12 Raytracing Fallback Error: ");
            OutputDebugString(errorString);
            OutputDebugString(L"\n");
        }
        throw _com_error(hr);
    }
}

#define SizeOfInUint32(obj) ((sizeof(obj) - 1) / sizeof(UINT32) + 1)

template <typename T>
T DivideAndRoundUp(T dividend, T divisor) { return (dividend - 1) / divisor + 1; }

__forceinline uint8_t Log2(uint32_t value)
{
    unsigned long mssb; // most significant set bit
    unsigned long lssb; // least significant set bit

                        // If perfect power of two (only one set bit), return index of bit.  Otherwise round up
                        // fractional log by adding 1 to most signicant set bit's index.
    if (BitScanReverse(&mssb, value) > 0 && BitScanForward(&lssb, value) > 0)
        return uint8_t(mssb + (mssb == lssb ? 0 : 1));
    else
        return 0;
}

template <typename T> T AlignPowerOfTwo(T value)
{
    return value == 0 ? 0 : 1 << Log2(value);
}

static void CreateRootSignatureHelper(ID3D12Device *pDevice, D3D12_VERSIONED_ROOT_SIGNATURE_DESC &desc, ID3D12RootSignature **ppRootSignature)
{
    CComPtr<ID3DBlob> pRootSignatureBlob;
    ThrowInternalFailure(::D3D12SerializeVersionedRootSignature(&desc, &pRootSignatureBlob, nullptr));

    ThrowInternalFailure(pDevice->CreateRootSignature(1, pRootSignatureBlob->GetBufferPointer(), pRootSignatureBlob->GetBufferSize(),
        IID_PPV_ARGS(ppRootSignature)));
}

#define COMPILED_SHADER(bytecodeArray) CD3DX12_SHADER_BYTECODE((void*)bytecodeArray, sizeof(bytecodeArray))

static void CreatePSOHelper(
    ID3D12Device *pDevice, 
    UINT nodeMask, 
    ID3D12RootSignature *pRootSignature, 
    const D3D12_SHADER_BYTECODE &byteCode, 
    ID3D12PipelineState **ppPSO)
{
    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.NodeMask = nodeMask;
    psoDesc.pRootSignature = pRootSignature;
    psoDesc.CS = byteCode;
    ThrowFailure(pDevice->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(ppPSO)));
}

static bool IsVertexBufferFormatSupported(DXGI_FORMAT format)
{
    switch (format)
    {
    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
        return true;
    default:
        return false;
    }
}

static bool IsIndexBufferFormatSupported(DXGI_FORMAT format)
{
    switch (format)
    {
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_UNKNOWN:
        return true;
    default:
        return false;
    }
}

static const D3D12_RAYTRACING_GEOMETRY_DESC &GetGeometryDesc(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &desc, UINT geometryIndex)
{
    switch (desc.DescsLayout)
    {
    case D3D12_ELEMENTS_LAYOUT_ARRAY:
        return desc.pGeometryDescs[geometryIndex];
    case D3D12_ELEMENTS_LAYOUT_ARRAY_OF_POINTERS:
        return *desc.ppGeometryDescs[geometryIndex];
    default:
        ThrowFailure(E_INVALIDARG, L"Unexpected value for D3D12_ELEMENTS_LAYOUT");
        return *(D3D12_RAYTRACING_GEOMETRY_DESC *)nullptr;
    }
}

static UINT GetPrimitiveCountFromGeometryDesc(const D3D12_RAYTRACING_GEOMETRY_DESC &geometryDesc)
{
    switch (geometryDesc.Type)
    {
        case D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES:
        {
            const D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC &triangles = geometryDesc.Triangles;
            if (!IsIndexBufferFormatSupported(triangles.IndexFormat))
            {
                ThrowFailure(E_NOTIMPL, L"Unsupported index buffer format provided");
            }

            const bool bNullIndexBuffer = (triangles.IndexFormat == DXGI_FORMAT_UNKNOWN);
            const UINT vertexCount = bNullIndexBuffer ? triangles.VertexCount : triangles.IndexCount;
            if (vertexCount % 3 != 0)
            {
                ThrowFailure(E_INVALIDARG, bNullIndexBuffer ?
                    L"Invalid vertex count provided, must be a multiple of 3 when there is no index buffer since geometry is always a triangle list" :
                    L"Invalid index count provided, must be a multiple of 3 since geometry is always a triangle list"
                );
            }
            return vertexCount / 3;
        }
        case D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS:
        {
            const D3D12_RAYTRACING_GEOMETRY_AABBS_DESC &aabbs = geometryDesc.AABBs;
            return static_cast<UINT>(aabbs.AABBCount);
        }
        default:
            ThrowFailure(E_INVALIDARG, L"Unrecognized D3D12_RAYTRACING_GEOMETRY_TYPE");
            return 0;
    }
}

static UINT GetTotalPrimitiveCount(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &desc)
{
    UINT totalTriangles = 0;
    for (UINT elementIndex = 0; elementIndex < desc.NumDescs; elementIndex++)
    {
        const D3D12_RAYTRACING_GEOMETRY_DESC &geometryDesc = GetGeometryDesc(desc, elementIndex);
        totalTriangles += GetPrimitiveCountFromGeometryDesc(geometryDesc);
    }
    return totalTriangles;
}

static UINT GetNumberOfInternalNodes(UINT numLeaves)
{
    // A binary tree with N leaves will always have N - 1 internal nodes
    return std::max(0, (INT)(numLeaves - 1));
}

static UINT GetNumParameters(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC &desc)
{
    UINT numParameters = (UINT)-1;
    switch (desc.Version)
    {
    case D3D_ROOT_SIGNATURE_VERSION_1_0:
        numParameters = desc.Desc_1_0.NumParameters;
        break;
    case D3D_ROOT_SIGNATURE_VERSION_1_1:
        numParameters = desc.Desc_1_0.NumParameters;
        break;
    default:
        ThrowFailure(E_NOTIMPL, L"Using an unrecognized D3D_ROOT_SIGNATURE_VERSION.");
    }
    return numParameters;
}
