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
#include "LoadPrimitivesBindings.h"
#include "CompiledShaders/LoadTrianglesFromR16IndexBuffer.h"
#include "CompiledShaders/LoadTrianglesFromR32IndexBuffer.h"
#include "CompiledShaders/LoadTrianglesNoIndexBuffer.h"
#include "CompiledShaders/LoadProceduralGeometry.h"

namespace FallbackLayer
{
    LoadPrimitivesPass::LoadPrimitivesPass(ID3D12Device *pDevice, UINT nodeMask)
    {
        CD3DX12_ROOT_PARAMETER1 rootParameters[NumRootParameters];
        rootParameters[ElementBufferSRV].InitAsShaderResourceView(ElementBufferRegister);
        rootParameters[IndexBufferSRV].InitAsShaderResourceView(IndexBufferRegister);
        rootParameters[TransformsBuffer].InitAsShaderResourceView(TransformRegister);
        rootParameters[OutputBuffer].InitAsUnorderedAccessView(OutputPrimitiveBufferRegister);
        rootParameters[OutputMetadataBuffer].InitAsUnorderedAccessView(OutputMetadataBufferRegister);
        rootParameters[InputRootConstants].InitAsConstants(SizeOfInUint32(LoadPrimitivesInputConstants), LoadInstancesConstantsRegister);

        auto rootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(ARRAYSIZE(rootParameters), rootParameters);
        CreateRootSignatureHelper(pDevice, rootSignatureDesc, &m_pRootSignature);

        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pLoadTrianglesFromR16IndexBuffer), &m_pPSOs[Index16Bit]);
        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pLoadTrianglesFromR32IndexBuffer), &m_pPSOs[Index32Bit]);
        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pLoadTrianglesNoIndexBuffer), &m_pPSOs[NoIndexBuffer]);
        CreatePSOHelper(pDevice, nodeMask, m_pRootSignature, COMPILED_SHADER(g_pLoadProceduralGeometry), &m_pLoadProceduralGeometryPSO);
    }

    LoadPrimitivesPass::IndexBufferType LoadPrimitivesPass::GetIndexBufferType(DXGI_FORMAT format)
    {
        switch (format)
        {
        case DXGI_FORMAT_UNKNOWN:
            return NoIndexBuffer;
        case DXGI_FORMAT_R32_UINT:
            return Index32Bit;
        case DXGI_FORMAT_R16_UINT:
            return Index16Bit;
        default:
            assert(false);
            return (LoadPrimitivesPass::IndexBufferType) - 1;
        }
    }

    void LoadPrimitivesPass::LoadPrimitives(ID3D12GraphicsCommandList *pCommandList, 
        const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC &buildDesc, 
        const UINT totalTriangleCount,
        D3D12_GPU_VIRTUAL_ADDRESS outputTriangleBuffer,
        D3D12_GPU_VIRTUAL_ADDRESS outputMetadataBuffer)
    {
        UINT numTrianglesLoaded = 0;
        for (UINT elementIndex = 0; elementIndex < buildDesc.NumDescs; elementIndex++)
        {
            const D3D12_RAYTRACING_GEOMETRY_DESC &geometryDesc = GetGeometryDesc(buildDesc, elementIndex);
            if (geometryDesc.Type == D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES)
            {
                const D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC &triangles = geometryDesc.Triangles;
                if (triangles.IndexBuffer == 0 && triangles.IndexFormat != DXGI_FORMAT_UNKNOWN)
                {
                    ThrowFailure(E_INVALIDARG, L"If the index buffer is null, the Index format must be DXGI_FORMAT_UNKNOWN");
                }
                if (!IsVertexBufferFormatSupported(triangles.VertexFormat))
                {
                    ThrowFailure(E_INVALIDARG, L"Invalid vertex format provided. Supported is limited to DXGI_FORMAT_R32G32B32_FLOAT/DXGI_FORMAT_R32G32B32A32_FLOAT");
                }
                const bool bNullIndexBuffer = (triangles.IndexFormat == DXGI_FORMAT_UNKNOWN);
                const UINT vertexCount = bNullIndexBuffer ? triangles.VertexCount : triangles.IndexCount;
                UINT numberOfTriangles = vertexCount / 3;

                CComPtr<ID3D12Device> pDevice;
                pCommandList->GetDevice(IID_PPV_ARGS(&pDevice));

                // DX12's SetComputeRootShaderResourceView requires that GPUVAs be 4-byte aligned. To handle
                // GPUVAs that are 2-byte aligned, we adjust the pointer to pass an offset for the 
                // compute shader to add on
                D3D12_GPU_VIRTUAL_ADDRESS indexBufferGPUVA = triangles.IndexBuffer;
                UINT indexBufferOffset = 0;
                if (triangles.IndexBuffer % 4 == 2)
                {
                    indexBufferGPUVA -= 2;
                    indexBufferOffset = 2;
                }
                assert(triangles.VertexBuffer.StrideInBytes < UINT32_MAX);
                LoadPrimitivesInputConstants constants = {};
                constants.IndexBufferOffset = indexBufferOffset;
                constants.NumTrianglesBound = numberOfTriangles;
                constants.TotalTriangleCount = totalTriangleCount;
                constants.TriangleOffset = numTrianglesLoaded;
                constants.VertexBufferStride = (UINT32)triangles.VertexBuffer.StrideInBytes;
                constants.GeometryContributionToHitGroupIndex = elementIndex;
                constants.HasValidTransform = (triangles.Transform != 0);

                pCommandList->SetComputeRootSignature(m_pRootSignature);
                pCommandList->SetComputeRoot32BitConstants(InputRootConstants, SizeOfInUint32(LoadPrimitivesInputConstants), &constants, 0);
                pCommandList->SetComputeRootShaderResourceView(ElementBufferSRV, triangles.VertexBuffer.StartAddress);
                pCommandList->SetComputeRootUnorderedAccessView(OutputBuffer, outputTriangleBuffer);
                pCommandList->SetComputeRootUnorderedAccessView(OutputMetadataBuffer, outputMetadataBuffer);
                if (!bNullIndexBuffer)
                {
                    pCommandList->SetComputeRootShaderResourceView(IndexBufferSRV, indexBufferGPUVA);
                }
                if (constants.HasValidTransform)
                {
                    pCommandList->SetComputeRootShaderResourceView(TransformsBuffer, triangles.Transform);
                }

                const UINT dispatchWidth = DivideAndRoundUp<UINT>(numberOfTriangles, THREAD_GROUP_1D_WIDTH);

                pCommandList->SetPipelineState(m_pPSOs[GetIndexBufferType(triangles.IndexFormat)]);
                pCommandList->Dispatch(dispatchWidth, 1, 1);

                numTrianglesLoaded += numberOfTriangles;
            }
            else
            {
                if (geometryDesc.Type != D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS)
                {
                    ThrowFailure(E_INVALIDARG, L"Unrecognized D3D12_RAYTRACING_GEOMETRY_TYPE");
                }

                const D3D12_RAYTRACING_GEOMETRY_AABBS_DESC &aabbs = geometryDesc.AABBs;
                if (aabbs.AABBs.StartAddress == 0 && aabbs.AABBCount > 0)
                {
                    ThrowFailure(E_INVALIDARG, L"Non-zero AABBCount provided with a null AABB buffer");
                }

                assert(aabbs.AABBCount < UINT32_MAX);
                UINT numberOfAABBs = static_cast<UINT>(aabbs.AABBCount);

                CComPtr<ID3D12Device> pDevice;
                pCommandList->GetDevice(IID_PPV_ARGS(&pDevice));

                assert(aabbs.AABBs.StrideInBytes < UINT32_MAX);
                LoadPrimitivesInputConstants constants = {};
                constants.IndexBufferOffset = 0;
                constants.NumTrianglesBound = numberOfAABBs;
                constants.TotalTriangleCount = totalTriangleCount;
                constants.TriangleOffset = numTrianglesLoaded;
                constants.VertexBufferStride = (UINT32)aabbs.AABBs.StrideInBytes;
                constants.GeometryContributionToHitGroupIndex = elementIndex;
                constants.HasValidTransform = false;

                pCommandList->SetComputeRootSignature(m_pRootSignature);
                pCommandList->SetComputeRoot32BitConstants(InputRootConstants, SizeOfInUint32(LoadPrimitivesInputConstants), &constants, 0);
                pCommandList->SetComputeRootShaderResourceView(ElementBufferSRV, aabbs.AABBs.StartAddress);
                pCommandList->SetComputeRootUnorderedAccessView(OutputBuffer, outputTriangleBuffer);
                pCommandList->SetComputeRootUnorderedAccessView(OutputMetadataBuffer, outputMetadataBuffer);
                const UINT dispatchWidth = DivideAndRoundUp<UINT>(numberOfAABBs, THREAD_GROUP_1D_WIDTH);

                pCommandList->SetPipelineState(m_pLoadProceduralGeometryPSO);
                pCommandList->Dispatch(dispatchWidth, 1, 1);

                // TODO: logic Duplicated in triangle path
                numTrianglesLoaded += numberOfAABBs;
            }
        }
        // We're only given the GPU VA not the resource itself so we need to resort to doing an overarching UAV barrier
        auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
        pCommandList->ResourceBarrier(1, &uavBarrier);
    }
}

