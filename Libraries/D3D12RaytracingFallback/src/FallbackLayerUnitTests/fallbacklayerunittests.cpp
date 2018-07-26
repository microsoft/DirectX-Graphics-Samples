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
#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace FallbackLayer;

namespace FallbackLayerUnitTests
{
    const UINT FloatsPerMatrix = sizeof(D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC::Transform) / sizeof(float);
    bool IsFloatArrayEqual(float *pArray1, float *pArray2, unsigned int floatCount)
    {
        const float epsilon = 0.002f;
        for (unsigned int i = 0; i < floatCount; i++)
        {
            if (pArray1[i] < pArray2[i] - epsilon || pArray1[i] > pArray2[i] + epsilon)
            {
                return false;
            }
        }
        return true;
    }

    class DescriptorHeapStack
    {
    public:
        DescriptorHeapStack(ID3D12Device &device, UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT NodeMask) :
            m_device(device)
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.NumDescriptors = numDescriptors;
            desc.Type = type;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            desc.NodeMask = NodeMask;
            device.CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pDescriptorHeap));

            m_descriptorSize = device.GetDescriptorHandleIncrementSize(type);
            m_descriptorHeapCpuBase = m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        }

        ID3D12DescriptorHeap &GetDescriptorHeap() { return *m_pDescriptorHeap; }

        void AllocateDescriptor(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE &cpuHandle, _Out_ UINT &descriptorHeapIndex)
        {
            descriptorHeapIndex = m_descriptorsAllocated;
            cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_descriptorHeapCpuBase, descriptorHeapIndex, m_descriptorSize);
            m_descriptorsAllocated++;
        }

        UINT AllocateBufferSrv(_In_ ID3D12Resource &resource)
        {
            UINT descriptorHeapIndex;
            D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
            AllocateDescriptor(cpuHandle, descriptorHeapIndex);
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
            srvDesc.Buffer.NumElements = (UINT)(resource.GetDesc().Width / sizeof(UINT32));
            srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
            m_device.CreateShaderResourceView(&resource, &srvDesc, cpuHandle);

            return descriptorHeapIndex;
        }

        UINT AllocateBufferUav(_In_ ID3D12Resource &resource)
        {
            UINT descriptorHeapIndex;
            D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
            AllocateDescriptor(cpuHandle, descriptorHeapIndex);
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            uavDesc.Buffer.NumElements = (UINT)(resource.GetDesc().Width / sizeof(UINT32));
            uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
            uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;

            m_device.CreateUnorderedAccessView(&resource, nullptr, &uavDesc, cpuHandle);
            return descriptorHeapIndex;
        }

        D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(UINT descriptorIndex)
        {
            return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), descriptorIndex, m_descriptorSize);
        }

    private:
        ID3D12Device & m_device;
        CComPtr<ID3D12DescriptorHeap> m_pDescriptorHeap;
        UINT m_descriptorsAllocated = 0;
        UINT m_descriptorSize;
        D3D12_CPU_DESCRIPTOR_HANDLE m_descriptorHeapCpuBase;
    };

    D3D12_RAYTRACING_GEOMETRY_DESC GetGeometryDesc(
        const CpuGeometryDescriptor &geomDesc,
        ID3D12Resource *pVertexBuffer = nullptr,
        ID3D12Resource *pIndexBuffer = nullptr,
        D3D12_GPU_VIRTUAL_ADDRESS transform = 0)
    {
        D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
        geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
        geometryDesc.Triangles.IndexBuffer = pIndexBuffer ? pIndexBuffer->GetGPUVirtualAddress() : 0;
        geometryDesc.Triangles.IndexCount = geomDesc.m_numIndicies;
        geometryDesc.Triangles.IndexFormat = geomDesc.m_indexBufferFormat;
        geometryDesc.Triangles.Transform3x4 = transform;
        geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        geometryDesc.Triangles.VertexCount = geomDesc.m_numVerticies;
        geometryDesc.Triangles.VertexBuffer.StartAddress = pVertexBuffer ? pVertexBuffer->GetGPUVirtualAddress() : 0;
        geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(float) * 3;

        return geometryDesc;
    }

    void GenerateRandomTranformation(float *pMatrix)
    {
        // Identity matrix
        ZeroMemory(pMatrix, sizeof(float) * FloatsPerMatrix);
        pMatrix[0] = 1;
        pMatrix[5] = 1;
        pMatrix[10] = 1;

        enum { Translation = 0, Rotation, Scale, NumTransformations };
        uint tranformType = rand() % NumTransformations;

        float rand0 = (rand() / (float)RAND_MAX) * 100.0f - 50.0f;
        float rand1 = (rand() / (float)RAND_MAX) * 100.0f - 50.0f;
        float rand2 = (rand() / (float)RAND_MAX) * 100.0f - 50.0f;

        switch (tranformType)
        {
        case Translation:
            pMatrix[3] = rand0;
            pMatrix[7] = rand1;
            pMatrix[11] = rand2;
            break;
        case Rotation:
            pMatrix[0] = cos(rand0);
            pMatrix[1] = sin(rand0);
            pMatrix[4] = -sin(rand0);
            pMatrix[5] = cos(rand0);
            break;
        case Scale:
            pMatrix[0] = rand0;
            pMatrix[5] = rand1;
            pMatrix[10] = rand2;
            break;
        }
    }

#define ALIGN(alignment, num) (((num + alignment - 1) / alignment) * alignment)

    class BuilderWrapper
    {
    public:
        virtual void BuildRaytracingAccelerationStructure(
            _In_  ID3D12GraphicsCommandList *pCommandList,
            _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
            _In_ ID3D12DescriptorHeap *pCbvSrvUavDescriptorHeap
        ) = 0;

        virtual void CopyRaytracingAccelerationStructure(
            _In_  ID3D12GraphicsCommandList *pCommandList,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS DestAccelerationStructureData,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData,
            _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE Flags) = 0;

        virtual void GetRaytracingAccelerationStructurePrebuildInfo(
            _In_  ID3D12Device *pDevice,
            _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE Type,
            _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS Flags,
            _In_  UINT NumElements,
            _In_reads_opt_(NumElements)  const D3D12_RAYTRACING_GEOMETRY_DESC *pGeometries,
            _Out_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO *pInfo) = 0;
    };

    // Used when white box testing the internal builders for the Fallback Layer
    class InternalFallbackBuilder : public BuilderWrapper
    {
    public:
        InternalFallbackBuilder(FallbackLayer::IAccelerationStructureBuilder *pBuilder) : m_pBuilder(pBuilder) {}

        virtual void CopyRaytracingAccelerationStructure(
            _In_  ID3D12GraphicsCommandList *pCommandList,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS DestAccelerationStructureData,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData,
            _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE Flags)
        {
            m_pBuilder->CopyRaytracingAccelerationStructure(pCommandList,
                DestAccelerationStructureData,
                SourceAccelerationStructureData,
                Flags);
        }

        virtual void BuildRaytracingAccelerationStructure(
            _In_  ID3D12GraphicsCommandList *pCommandList,
            _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
            _In_ ID3D12DescriptorHeap *pCbvSrvUavDescriptorHeap)
        {
            m_pBuilder->BuildRaytracingAccelerationStructure(pCommandList, pDesc, pCbvSrvUavDescriptorHeap);
        }

        virtual void GetRaytracingAccelerationStructurePrebuildInfo(
            _In_  ID3D12Device *pDevice,
            _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE Type,
            _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS Flags,
            _In_  UINT NumElements,
            _In_reads_opt_(NumElements)  const D3D12_RAYTRACING_GEOMETRY_DESC *pGeometries,
            _Out_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO *pInfo)
        {
            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS desc;
            desc.Type = Type;
            desc.Flags = Flags;
            desc.NumDescs = NumElements;
            desc.pGeometryDescs = pGeometries;
            desc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
            m_pBuilder->GetRaytracingAccelerationStructurePrebuildInfo(&desc, pInfo);
        }
    private:
        FallbackLayer::IAccelerationStructureBuilder* m_pBuilder;
    };

    class AccelerationStructureBuilderHelper
    {
    public:
        AccelerationStructureBuilderHelper(D3D12Context &context, DescriptorHeapStack &descriptorHeapStack) :
            m_d3d12Context(context), m_descriptorHeapStack(descriptorHeapStack) {}

        void BuildTopLevelAccelerationStructure(
            BuilderWrapper &builder,
            CpuGeometryDescriptor *pGeomDescs,
            float **ppTransformations,
            UINT numGeoms,
            CComPtr<ID3D12Heap> &pHeap,
            CComPtr<ID3D12Resource> &pTopLevelResource,
            D3D12_ELEMENTS_LAYOUT layoutToTest,
            bool performUpdate = false
        )
        {
            ID3D12Device &device = m_d3d12Context.GetDevice();

            std::vector<D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC> instanceDescs(numGeoms);

            UINT64 totalSizeNeeded = 0;
            UINT sizeofInstanceDescs = (UINT)(instanceDescs.size() * sizeof(instanceDescs[0]));
            totalSizeNeeded += ALIGN(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, sizeofInstanceDescs);
            for (UINT i = 0; i < numGeoms; i++)
            {
                D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = GetSizeNeededForBottomLevelHeap(device, builder, &pGeomDescs[i], 1);
                totalSizeNeeded += ALIGN(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, prebuildInfo.ResultDataMaxSizeInBytes);
                totalSizeNeeded += ALIGN(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, prebuildInfo.ScratchDataSizeInBytes);
            }

            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
            if (performUpdate)
            {
                buildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
            }

            // Top level
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo;
            {
                builder.GetRaytracingAccelerationStructurePrebuildInfo(
                    &device,
                    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
                    buildFlags,
                    numGeoms,
                    nullptr,
                    &topLevelPrebuildInfo
                );
                totalSizeNeeded += ALIGN(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, topLevelPrebuildInfo.ResultDataMaxSizeInBytes);
                totalSizeNeeded += ALIGN(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, topLevelPrebuildInfo.ScratchDataSizeInBytes);
            }

            auto heapDesc = CD3DX12_HEAP_DESC(totalSizeNeeded, D3D12_HEAP_TYPE_DEFAULT);
            heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
            AssertSucceeded(device.CreateHeap(&heapDesc, IID_PPV_ARGS(&pHeap)));

            UINT64 heapOffset = 0;
            // Top Level Resource needs to be the first thing created
            auto topLevelResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(topLevelPrebuildInfo.ResultDataMaxSizeInBytes);
            AssertSucceeded(device.CreatePlacedResource(pHeap, heapOffset, &topLevelResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pTopLevelResource)));
            heapOffset += ALIGN(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, topLevelPrebuildInfo.ResultDataMaxSizeInBytes);

            D3D12_GPU_VIRTUAL_ADDRESS heapBaseAddress = pTopLevelResource->GetGPUVirtualAddress();

            auto instanceResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeofInstanceDescs);
            CComPtr<ID3D12Resource> pInstanceDescsResource;
            auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            AssertSucceeded(device.CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &instanceResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pInstanceDescsResource)));

            std::vector<CComPtr<ID3D12Resource>> pBottomLevelAccelerationStructures(numGeoms);

            UINT descriptorHeapIndex = m_descriptorHeapStack.AllocateBufferSrv(*pInstanceDescsResource);

            WRAPPED_GPU_POINTER baseHeapLocation = FallbackLayer::CreateWrappedPointer(descriptorHeapIndex);
            WRAPPED_GPU_POINTER instanceDescLocation = baseHeapLocation;
            std::vector<WRAPPED_GPU_POINTER> instanceDescLocationList(numGeoms);

            // Offset all bottom level pointers to make the the BVH building code is robust
            UINT pointerOffset = 256;

            for (UINT i = 0; i < numGeoms; i++)
            {
                BuildBottomLevelAccelerationStructure(builder, pGeomDescs[i], &pBottomLevelAccelerationStructures[i], pointerOffset);

                D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC &desc = instanceDescs[i];
                ZeroMemory(&desc, sizeof(desc));

                if (ppTransformations)
                {
                    memcpy(desc.Transform, ppTransformations[i], sizeof(D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC::Transform));
                }
                else
                {
                    // Identity matrix
                    desc.Transform[0][0] = 1.0f;
                    desc.Transform[1][1] = 1.0f;
                    desc.Transform[2][2] = 1.0f;
                }


                desc.AccelerationStructure = FallbackLayer::CreateWrappedPointer(
                    m_descriptorHeapStack.AllocateBufferUav(*pBottomLevelAccelerationStructures[i]));
                desc.AccelerationStructure.GpuVA += pointerOffset;
                instanceDescLocationList[i].GpuVA = instanceDescLocation.GpuVA + i * sizeof(D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC);
            }

            CComPtr<ID3D12GraphicsCommandList> pCommandList;
            m_d3d12Context.GetGraphicsCommandList(&pCommandList);

            ID3D12DescriptorHeap *heaps[] = { &m_descriptorHeapStack.GetDescriptorHeap() };
            pCommandList->SetDescriptorHeaps(1, heaps);

            // Upload instance information
            CComPtr<ID3D12Resource> pInstanceUploadResource;
            const UINT instanceDataSize = (UINT)(instanceDescs.size() * sizeof(instanceDescs[0]));
            m_d3d12Context.CreateResourceWithInitialData(instanceDescs.data(), instanceDataSize, &pInstanceUploadResource);
            pCommandList->CopyBufferRegion(pInstanceDescsResource, 0, pInstanceUploadResource, 0, instanceDataSize);

            auto transitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pInstanceDescsResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
            pCommandList->ResourceBarrier(1, &transitionBarrier);

            CComPtr<ID3D12Resource> pScratchResource;
            auto scratchResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(topLevelPrebuildInfo.ScratchDataSizeInBytes);
            AssertSucceeded(device.CreatePlacedResource(pHeap, heapOffset, &scratchResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pScratchResource)));
            heapOffset += ALIGN(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, topLevelPrebuildInfo.ScratchDataSizeInBytes);

            CComPtr<ID3D12Resource> pInstanceLocationList;
            if (layoutToTest == D3D12_ELEMENTS_LAYOUT_ARRAY_OF_POINTERS)
            {
                m_d3d12Context.CreateResourceWithInitialData(instanceDescLocationList.data(), (UINT)(instanceDescLocationList.size() * sizeof(instanceDescLocationList[0])), &pInstanceLocationList);
            }

            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelDesc = {};
            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &topLevelInput = topLevelDesc.Inputs;
            topLevelDesc.DestAccelerationStructureData = pTopLevelResource->GetGPUVirtualAddress();
            topLevelDesc.ScratchAccelerationStructureData = pScratchResource->GetGPUVirtualAddress();
            topLevelInput.Flags = buildFlags;
            topLevelInput.NumDescs = numGeoms;
            topLevelInput.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
            topLevelInput.InstanceDescs = layoutToTest == D3D12_ELEMENTS_LAYOUT_ARRAY ?
                pInstanceDescsResource->GetGPUVirtualAddress() :
                pInstanceLocationList->GetGPUVirtualAddress();
            topLevelInput.DescsLayout = layoutToTest;
            builder.BuildRaytracingAccelerationStructure(pCommandList, &topLevelDesc, &m_descriptorHeapStack.GetDescriptorHeap());

            if (performUpdate)
            {
                topLevelInput.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
                builder.BuildRaytracingAccelerationStructure(pCommandList, &topLevelDesc, &m_descriptorHeapStack.GetDescriptorHeap());
            }

            AssertSucceeded(pCommandList->Close());
            m_d3d12Context.ExecuteCommandList(pCommandList);

            m_d3d12Context.WaitForGpuWork();
        }

        void CopyAccelerationStructure(
            BuilderWrapper &builder,
            ID3D12Resource *pDestAccelerationStructure,
            ID3D12Resource *pSourceAccelerationStructure)
        {
            ID3D12Device &device = m_d3d12Context.GetDevice();
            CComPtr<ID3D12GraphicsCommandList> pCommandList;
            m_d3d12Context.GetGraphicsCommandList(&pCommandList);

            builder.CopyRaytracingAccelerationStructure(
                pCommandList,
                pDestAccelerationStructure->GetGPUVirtualAddress(),
                pSourceAccelerationStructure->GetGPUVirtualAddress(),
                D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_CLONE);

            auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
            pCommandList->ResourceBarrier(1, &uavBarrier);

            AssertSucceeded(pCommandList->Close());
            m_d3d12Context.ExecuteCommandList(pCommandList);
            m_d3d12Context.WaitForGpuWork();
        }



        void BuildBottomLevelAccelerationStructure(
            BuilderWrapper &builder,
            CpuGeometryDescriptor *pCpuGeometryDescriptors, UINT numGeometry,
            ID3D12Resource **ppBottomLevelAccelerationStructure,
            D3D12_ELEMENTS_LAYOUT layoutToTest = D3D12_ELEMENTS_LAYOUT_ARRAY,
            UINT offsetPointer = 0,
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD
        )
        {
            ID3D12Device &device = m_d3d12Context.GetDevice();

            std::vector<CComPtr<ID3D12Resource>> transformBuffers(numGeometry);
            std::vector<CComPtr<ID3D12Resource>> pVertexBuffer(numGeometry);
            std::vector<CComPtr<ID3D12Resource>> pIndexBuffer(numGeometry);
            std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs(numGeometry);
            std::vector<D3D12_RAYTRACING_GEOMETRY_DESC*> geometryDescPointers(numGeometry);
            for (UINT i = 0; i < numGeometry; i++)
            {
                CpuGeometryDescriptor &desc = pCpuGeometryDescriptors[i];
                m_d3d12Context.CreateResourceWithInitialData(pCpuGeometryDescriptors[i].transform.data(), sizeof(float) * 12, &transformBuffers[i]);
                m_d3d12Context.CreateResourceWithInitialData(desc.m_pVertexData, sizeof(float) * 3 * desc.m_numVerticies, &pVertexBuffer[i]);
                if (desc.m_pIndexBuffer)
                {
                    m_d3d12Context.CreateResourceWithInitialData(desc.m_pIndexBuffer, desc.GetSizeOfIndex() * desc.m_numIndicies, &pIndexBuffer[i]);
                }
                geometryDescs[i] = GetGeometryDesc(desc, pVertexBuffer[i], pIndexBuffer[i], transformBuffers[i]->GetGPUVirtualAddress());
            }

            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = GetSizeNeededForBottomLevelHeap(device, builder, pCpuGeometryDescriptors, numGeometry, buildFlags);

            auto bottomLevelResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(offsetPointer + prebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            AssertSucceeded(device.CreateCommittedResource(
                &defaultHeapProperties,
                D3D12_HEAP_FLAG_NONE,
                &bottomLevelResourceDesc,
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                IID_PPV_ARGS(ppBottomLevelAccelerationStructure)));

            CComPtr<ID3D12Resource> pScratchMemory;
            if (prebuildInfo.ScratchDataSizeInBytes)
            {
                auto scratchResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(prebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

                AssertSucceeded(device.CreateCommittedResource(
                    &defaultHeapProperties,
                    D3D12_HEAP_FLAG_NONE,
                    &scratchResourceDesc,
                    D3D12_RESOURCE_STATE_COMMON,
                    nullptr,
                    IID_PPV_ARGS(&pScratchMemory)));
            }

            CComPtr<ID3D12GraphicsCommandList> pCommandList;
            m_d3d12Context.GetGraphicsCommandList(&pCommandList);

            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelDesc = {};
            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &bottomLevelInput = bottomLevelDesc.Inputs;
            bottomLevelDesc.DestAccelerationStructureData = offsetPointer + (*ppBottomLevelAccelerationStructure)->GetGPUVirtualAddress();
            bottomLevelDesc.ScratchAccelerationStructureData = prebuildInfo.ScratchDataSizeInBytes > 0 ? pScratchMemory->GetGPUVirtualAddress() : 0;
            bottomLevelInput.Flags = buildFlags;
            bottomLevelInput.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
            bottomLevelInput.NumDescs = (UINT)geometryDescs.size();
            if (layoutToTest == D3D12_ELEMENTS_LAYOUT_ARRAY)
            {
                bottomLevelInput.pGeometryDescs = geometryDescs.data();
            }
            else
            {
                for (UINT i = 0; i < geometryDescs.size(); i++)
                {
                    geometryDescPointers[i] = &geometryDescs[i];
                }
                bottomLevelInput.ppGeometryDescs = geometryDescPointers.data();
            }
            bottomLevelInput.DescsLayout = layoutToTest;
            builder.BuildRaytracingAccelerationStructure(pCommandList, &bottomLevelDesc, &m_descriptorHeapStack.GetDescriptorHeap());

            AssertSucceeded(pCommandList->Close());
            m_d3d12Context.ExecuteCommandList(pCommandList);
            m_d3d12Context.WaitForGpuWork();

            // Debugging
#if 0
            std::vector<BYTE> data(prebuildInfo.ScratchDataSizeInBytes);
            m_d3d12Context.ReadbackResource(pScratchMemory, data.data(), prebuildInfo.ScratchDataSizeInBytes);
#endif
        }

        void BuildBottomLevelAccelerationStructure(
            BuilderWrapper &builder,
            CpuGeometryDescriptor &geometryDescriptor,
            ID3D12Resource **ppBottomLevelAccelerationStructure,
            UINT offsetToPointer = 0,
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD)
        {
            BuildBottomLevelAccelerationStructure(builder, &geometryDescriptor, 1, ppBottomLevelAccelerationStructure, D3D12_ELEMENTS_LAYOUT_ARRAY, offsetToPointer, buildFlags);
        }

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO GetSizeNeededForBottomLevelHeap(
            ID3D12Device &device,
            BuilderWrapper &builder,
            const CpuGeometryDescriptor *pGeomDescs,
            UINT geomCount,
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD)
        {
            std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> descs(geomCount);
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo;

            for (UINT i = 0; i < geomCount; i++)
            {
                descs[i] = GetGeometryDesc(pGeomDescs[i]);
            }

            builder.GetRaytracingAccelerationStructurePrebuildInfo(
                &device,
                D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,
                buildFlags,
                geomCount,
                descs.data(),
                &prebuildInfo);
            return prebuildInfo;
        }
    private:
        D3D12Context & m_d3d12Context;
        DescriptorHeapStack &m_descriptorHeapStack;
    };

#define VERTEX_COUNT(floatArray) (ARRAYSIZE(floatArray) / 3)

    static const float ReferenceVerticies0[] =
    {
        0.0, 0.0, 0.0f,
        1.0, 1.0, 0.0f,
        0.0, 1.0, 0.0f,

        0.0, 0.0, 1.0f,
        1.0, 1.0, 1.0f,
        0.0, 1.0, 1.0f,

        0.0, 0.0, 2.0f,
        1.0, 1.0, 2.0f,
        0.0, 1.0, 2.0f,
    };

    UINT16 ReferenceIndices0[]
    {
        0, 1, 2, 3, 4, 5, 6, 7, 8
    };

    UINT32 ReferenceR32Indices0[]
    {
        0, 1, 2, 3, 4, 5, 6, 7, 8
    };

    static const float ReferenceVerticies1[] =
    {
        1.0, 0.0, 0.0f,
        2.0, 1.0, 0.0f,
        1.0, 1.0, 0.0f,

        1.0, 0.0, 1.0f,
        2.0, 1.0, 1.0f,
        1.0, 1.0, 1.0f,

        1.0, 0.0, 2.0f,
        2.0, 1.0, 2.0f,
        1.0, 1.0, 2.0f,

        2.0, 0.0, 0.0f,
        3.0, 1.0, 0.0f,
        2.0, 1.0, 0.0f,

        2.0, 0.0, 1.0f,
        3.0, 1.0, 1.0f,
        2.0, 1.0, 1.0f,

        2.0, 0.0, 2.0f,
        3.0, 1.0, 2.0f,
        2.0, 1.0, 2.0f,
    };

    UINT16 ReferenceIndices1[]
    {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17
    };

    UINT32 ReferenceR32Indices1[]
    {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17
    };

    TEST_CLASS(AccelerationStructureUnitTests)
    {
    public:
        TEST_METHOD_INITIALIZE(MethodSetup)
        {
            m_pDescriptorHeapStack = std::unique_ptr<DescriptorHeapStack>(
                new DescriptorHeapStack(m_d3d12Context.GetDevice(), 100, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0));
            m_pBuilderHelper = std::unique_ptr<AccelerationStructureBuilderHelper>(
                new AccelerationStructureBuilderHelper(m_d3d12Context, *m_pDescriptorHeapStack));
        }

        TEST_METHOD(R16IndexBufferBottomLevelCpuBVHBuilder)
        {
            CpuGeometryDescriptor testCases[] =
            {
                CpuGeometryDescriptor(ReferenceVerticies0, VERTEX_COUNT(ReferenceVerticies0), ReferenceIndices0, ARRAYSIZE(ReferenceIndices0)),
                CpuGeometryDescriptor(ReferenceVerticies1, VERTEX_COUNT(ReferenceVerticies1), ReferenceIndices1, ARRAYSIZE(ReferenceIndices1))
            };

            for (UINT testIndex = 0; testIndex < ARRAYSIZE(testCases); testIndex++)
            {
                TestCpuBvh2Builder(testCases[testIndex]);
            }
        }

        TEST_METHOD(R16IndexBufferBottomLevelGpuBVHBuilder)
        {
            CpuGeometryDescriptor testCases[] =
            {
                CpuGeometryDescriptor(ReferenceVerticies0, VERTEX_COUNT(ReferenceVerticies0), ReferenceIndices0, ARRAYSIZE(ReferenceIndices0)),
                CpuGeometryDescriptor(ReferenceVerticies1, VERTEX_COUNT(ReferenceVerticies1), ReferenceIndices1, ARRAYSIZE(ReferenceIndices1))
            };

            for (UINT testIndex = 0; testIndex < ARRAYSIZE(testCases); testIndex++)
            {
                TestGpuBvh2Builder(testCases[testIndex]);
            }
        }

        TEST_METHOD(R32IndexBufferBottomLevelGpuBVHBuilder)
        {
            CpuGeometryDescriptor testCases[] =
            {
                CpuGeometryDescriptor(ReferenceVerticies0, VERTEX_COUNT(ReferenceVerticies0), ReferenceR32Indices0, ARRAYSIZE(ReferenceR32Indices0)),
                CpuGeometryDescriptor(ReferenceVerticies1, VERTEX_COUNT(ReferenceVerticies1), ReferenceR32Indices1, ARRAYSIZE(ReferenceR32Indices1))
            };

            for (UINT testIndex = 0; testIndex < ARRAYSIZE(testCases); testIndex++)
            {
                TestGpuBvh2Builder(testCases[testIndex]);
            }
        }

        TEST_METHOD(RedundantTrianglesBottomLevelGpuBVHBuilder)
        {
            std::vector<float> redundantTriangles;
            const uint numTriangles = 16;
            const uint floatsPerTriangle = 9;
            for (UINT i = 0; i < numTriangles; i++)
            {
                for (UINT j = 0; j < floatsPerTriangle; j++)
                {
                    redundantTriangles.push_back(ReferenceVerticies0[j]);
                }
            }

            CpuGeometryDescriptor testCase =
                CpuGeometryDescriptor(redundantTriangles.data(), (UINT)(redundantTriangles.size() / 3));
            TestGpuBvh2Builder(testCase);
        }

        TEST_METHOD(NoIndexBufferBottomLevelGpuBVHBuilder)
        {
            CpuGeometryDescriptor testCases[] =
            {
                CpuGeometryDescriptor(ReferenceVerticies0, VERTEX_COUNT(ReferenceVerticies0)),
                CpuGeometryDescriptor(ReferenceVerticies1, VERTEX_COUNT(ReferenceVerticies1))
            };

            for (UINT testIndex = 0; testIndex < ARRAYSIZE(testCases); testIndex++)
            {
                TestGpuBvh2Builder(testCases[testIndex]);
            }
        }

        TEST_METHOD(BottomLevelGpuBVHBuilderWithTransforms)
        {
            const UINT numGeoms = 10;
            float pMatrixStorage[numGeoms * 12];
            std::vector<CpuGeometryDescriptor> testCases;
            srand(10);
            for (UINT i = 0; i < numGeoms; i++)
            {
                float *pMatrix = pMatrixStorage + FloatsPerMatrix * i;
                GenerateRandomTranformation(pMatrix);
                testCases.push_back(
                    CpuGeometryDescriptor(ReferenceVerticies0, VERTEX_COUNT(ReferenceVerticies0), nullptr, 0, DXGI_FORMAT_UNKNOWN, pMatrix));
            }
            TestGpuBvh2Builder(testCases.data(), numGeoms);
        }

        TEST_METHOD(MultipleGeometrySingleBottomLevelGpuBVHBuilder_ArrayOfPointersLayout)
        {
            TestMultipleGeometrySingleBottomLevelGpuBVHBuilder(D3D12_ELEMENTS_LAYOUT_ARRAY_OF_POINTERS);
        }

        TEST_METHOD(MultipleGeometrySingleBottomLevelGpuBVHBuilder_ArrayLayout)
        {
            TestMultipleGeometrySingleBottomLevelGpuBVHBuilder(D3D12_ELEMENTS_LAYOUT_ARRAY);
        }

        void TestMultipleGeometrySingleBottomLevelGpuBVHBuilder(D3D12_ELEMENTS_LAYOUT layoutToTest)
        {
            CpuGeometryDescriptor testCases[] =
            {
                CpuGeometryDescriptor(ReferenceVerticies0, VERTEX_COUNT(ReferenceVerticies0), ReferenceIndices0, ARRAYSIZE(ReferenceIndices0)),
                CpuGeometryDescriptor(ReferenceVerticies1, VERTEX_COUNT(ReferenceVerticies1), ReferenceIndices1, ARRAYSIZE(ReferenceIndices1))
            };

            TestGpuBvh2Builder(testCases, ARRAYSIZE(testCases));
        }

        TEST_METHOD(StressBottomLevelGpuBVHBuilder)
        {
            std::vector<float> AutoGeneratedReferenceVertices;
            std::vector<UINT16> AutoGeneratedReferenceIndicies;
            for (UINT i = 0; i < 500; i++)
            {
                for (float f : ReferenceVerticies0)
                {
                    AutoGeneratedReferenceVertices.push_back(f + i);
                }

                for (UINT16 index : ReferenceIndices0)
                {
                    AutoGeneratedReferenceIndicies.push_back(index + (UINT16)ARRAYSIZE(ReferenceIndices0) * i);
                }
            }
            CpuGeometryDescriptor testCase(AutoGeneratedReferenceVertices.data(),
                (UINT)(AutoGeneratedReferenceVertices.size() / 3),
                AutoGeneratedReferenceIndicies.data(),
                (UINT)AutoGeneratedReferenceIndicies.size());

            TestGpuBvh2Builder(
                testCase);
        }

        TEST_METHOD(StressBottomLevelCpuBVHBuilder)
        {
            std::vector<float> AutoGeneratedReferenceVertices;
            std::vector<UINT16> AutoGeneratedReferenceIndicies;
            for (UINT i = 0; i < 1000; i++)
            {
                for (float f : ReferenceVerticies0)
                {
                    AutoGeneratedReferenceVertices.push_back(f + i);
                }

                for (UINT16 index : ReferenceIndices0)
                {
                    AutoGeneratedReferenceIndicies.push_back(index + (UINT16)ARRAYSIZE(ReferenceIndices0) * i);
                }
            }
            CpuGeometryDescriptor testCase(AutoGeneratedReferenceVertices.data(),
                (UINT)(AutoGeneratedReferenceVertices.size() / 3),
                AutoGeneratedReferenceIndicies.data(),
                (UINT)AutoGeneratedReferenceIndicies.size());

            TestCpuBvh2Builder(
                testCase);
        }

        template <UINT numBottomLevels>
        void SimpleTopLevelGpuBVHBuilder(
            D3D12_ELEMENTS_LAYOUT layoutToTest,
            bool applyRandomInstanceTransforms,
            bool testCopyAccelerationStructure = false,
            bool testWithUpdate = false) {
            const UINT referenceVertexArraySize = ARRAYSIZE(ReferenceVerticies0);
            const UINT referenceIndexArraySize = ARRAYSIZE(ReferenceIndices0);

            float *AutoGeneratedReferenceVertices[numBottomLevels];
            UINT16 *AutoGeneratedReferenceIndicies[numBottomLevels];

            std::unique_ptr<float[]> pVertices[numBottomLevels];
            std::unique_ptr<UINT16[]> pIndicies[numBottomLevels];

            std::vector<std::unique_ptr<float[]>> matrixStorage;
            float* pTransformations[numBottomLevels];
            srand(10);
            if (applyRandomInstanceTransforms)
            {
                for (UINT i = 0; i < numBottomLevels; i++)
                {
                    matrixStorage.push_back(std::unique_ptr<float[]>(new float[FloatsPerMatrix]));
                    float *pMatrix = matrixStorage.back().get();
                    pTransformations[i] = pMatrix;

                    GenerateRandomTranformation(pMatrix);
                }

            }

            AABB containingBoxes[numBottomLevels] = {};
            for (UINT i = 0; i < numBottomLevels; i++) {
                for (UINT axis = 0; axis < 3; axis++) {
                    containingBoxes[i].minArr[axis] = FLT_MAX;
                    containingBoxes[i].maxArr[axis] = -FLT_MAX;
                }
            }

            CpuGeometryDescriptor geomDescs[numBottomLevels] = {};
            for (UINT level = 0; level < numBottomLevels; level++) {
                pVertices[level] = std::unique_ptr<float[]>(new float[referenceVertexArraySize]);
                AutoGeneratedReferenceVertices[level] = pVertices[level].get();
                for (UINT i = 0; i < referenceVertexArraySize; i++) {
                    float newInput = ReferenceVerticies0[i] + level;
                    UINT axis = i % 3;
                    containingBoxes[level].minArr[axis] = std::min(newInput, containingBoxes[level].minArr[axis]);
                    containingBoxes[level].maxArr[axis] = std::max(newInput, containingBoxes[level].maxArr[axis]);

                    AutoGeneratedReferenceVertices[level][i] = newInput;
                }

                pIndicies[level] = std::unique_ptr<UINT16[]>(new UINT16[referenceIndexArraySize]);
                AutoGeneratedReferenceIndicies[level] = pIndicies[level].get();
                for (UINT i = 0; i < referenceIndexArraySize; i++) {
                    AutoGeneratedReferenceIndicies[level][i] = ReferenceIndices0[i];
                }

                geomDescs[level] = CpuGeometryDescriptor(pVertices[level].get(), referenceVertexArraySize / 3, pIndicies[level].get(), referenceIndexArraySize);
            }

            ID3D12Device &device = m_d3d12Context.GetDevice();
            std::unique_ptr<FallbackLayer::IAccelerationStructureBuilder> pBuilder =
                std::unique_ptr<FallbackLayer::IAccelerationStructureBuilder>(
                    new FallbackLayer::GpuBvh2Builder(&device, m_d3d12Context.GetTotalLaneCount(), 0));
            InternalFallbackBuilder builderWrapper(pBuilder.get());

            CComPtr<ID3D12Heap> pHeap;
            CComPtr<ID3D12Resource> pTopLevelResource;
            m_pBuilderHelper->BuildTopLevelAccelerationStructure(
                builderWrapper,
                geomDescs,
                applyRandomInstanceTransforms ? pTransformations : nullptr,
                ARRAYSIZE(geomDescs),
                pHeap,
                pTopLevelResource,
                layoutToTest,
                testWithUpdate);

            const UINT dataSize = (UINT)pTopLevelResource->GetDesc().Width;

            ID3D12Resource *pResourceToReadback = pTopLevelResource;
            CComPtr<ID3D12Resource> pTopLevelCopy;
            if (testCopyAccelerationStructure)
            {
                auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
                auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(dataSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
                device.CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pTopLevelCopy));

                m_pBuilderHelper->CopyAccelerationStructure(builderWrapper, pTopLevelCopy, pTopLevelResource);

                pResourceToReadback = pTopLevelCopy;
            }


            std::unique_ptr<BYTE[]> pData = std::unique_ptr<BYTE[]>(new BYTE[dataSize]);
            Assert::AreNotEqual(pData.get(), (BYTE *)nullptr, L"Failed to allocate output data");
            m_d3d12Context.ReadbackResource(pResourceToReadback, pData.get(), dataSize);

            std::wstring errorMessage;
            auto &validator = FallbackLayer::GetAccelerationStructureValidator(pBuilder->GetAccelerationStructureType());
            if (!validator.VerifyTopLevelOutput(containingBoxes, applyRandomInstanceTransforms ? pTransformations : nullptr, numBottomLevels, pData.get(), errorMessage)) {
                Assert::Fail(errorMessage.c_str());
            }
        }

        TEST_METHOD(SimpleTopLevelGpuBVHBuilderSingleBottomLevel)
        {
            SimpleTopLevelGpuBVHBuilder<1>(D3D12_ELEMENTS_LAYOUT_ARRAY, false);
        }

        TEST_METHOD(SimpleTopLevelGpuBVHBuilderWithCopy)
        {
            SimpleTopLevelGpuBVHBuilder<50>(D3D12_ELEMENTS_LAYOUT_ARRAY, false, true);
        }

        TEST_METHOD(SimpleTopLevelGpuBVHBuilder_ArrayLayout) {
            SimpleTopLevelGpuBVHBuilder<50>(D3D12_ELEMENTS_LAYOUT_ARRAY, false);
        }

        TEST_METHOD(SimpleTopLevelGpuBVHBuilder_ArrayOfPointersLayout)
        {
            SimpleTopLevelGpuBVHBuilder<50>(D3D12_ELEMENTS_LAYOUT_ARRAY_OF_POINTERS, false);
        }

        TEST_METHOD(TopLevelGpuBVHBuilderWithInstanceTransforms_ArrayLayout)
        {
            SimpleTopLevelGpuBVHBuilder<50>(D3D12_ELEMENTS_LAYOUT_ARRAY, true);
        }

        TEST_METHOD(TopLevelGpuBVHBuilderWithInstanceTransforms_ArrayOfPointersLayout)
        {
            SimpleTopLevelGpuBVHBuilder<50>(D3D12_ELEMENTS_LAYOUT_ARRAY_OF_POINTERS, true);
        }

        TEST_METHOD(SimpleTopLevelGpuBVHBuilderSingleBottomLevel_WithUpdate)
        {
            SimpleTopLevelGpuBVHBuilder<1>(D3D12_ELEMENTS_LAYOUT_ARRAY, false, true);
        }

        TEST_METHOD(SimpleTopLevelGpuBVHBuilder_ArrayLayout_WithUpdate) {
            SimpleTopLevelGpuBVHBuilder<50>(D3D12_ELEMENTS_LAYOUT_ARRAY, false, true);
        }

        TEST_METHOD(SimpleTopLevelGpuBVHBuilder_ArrayOfPointersLayout_WithUpdate)
        {
            SimpleTopLevelGpuBVHBuilder<50>(D3D12_ELEMENTS_LAYOUT_ARRAY_OF_POINTERS, false, true);
        }

        TEST_METHOD(TopLevelGpuBVHBuilderWithInstanceTransforms_ArrayLayout_WithUpdate)
        {
            SimpleTopLevelGpuBVHBuilder<50>(D3D12_ELEMENTS_LAYOUT_ARRAY, true, true);
        }

        TEST_METHOD(TopLevelGpuBVHBuilderWithInstanceTransforms_ArrayOfPointersLayout_WithUpdate)
        {
            SimpleTopLevelGpuBVHBuilder<50>(D3D12_ELEMENTS_LAYOUT_ARRAY_OF_POINTERS, true, true);
        }

        TEST_METHOD(EmitRaytracingAccelerationStructurePostBuildInfoTest)
        {
            const UINT numBottomLevels = 70;

            ID3D12Device &device = m_d3d12Context.GetDevice();
            std::unique_ptr<FallbackLayer::IAccelerationStructureBuilder> pBuilder =
                std::unique_ptr<FallbackLayer::IAccelerationStructureBuilder>(
                    new FallbackLayer::GpuBvh2Builder(&device, m_d3d12Context.GetTotalLaneCount(), 0));
            std::vector<CComPtr<ID3D12Resource>> pVertexBuffers(numBottomLevels);
            std::vector<CComPtr<ID3D12Resource>> pBottomLevelResources(numBottomLevels);
            std::vector<UINT32> pBottomLevelMaxResourceSizes(numBottomLevels);
            std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> pGeometryDescs(numBottomLevels);
            std::vector<D3D12_GPU_VIRTUAL_ADDRESS> pBottomLevelResourcesGpuVA(numBottomLevels);
            CComPtr<ID3D12Resource> pOutputCountBuffer;
            std::vector<float> pInitialData;

            auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            auto countBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT32) * numBottomLevels, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            device.CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &countBufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pOutputCountBuffer));

            const UINT floatsPerVertex = 3;
            const UINT verticesPerTriangle = 3;
            const UINT floatsPerTriangle = verticesPerTriangle * floatsPerVertex;
            for (UINT i = 0; i < numBottomLevels; i++)
            {
                for (UINT i = 0; i < floatsPerTriangle; i++)
                {
                    pInitialData.push_back((float)i);
                }
            }

            UINT64 scratchBufferSize = 0;
            for (UINT i = 0; i < numBottomLevels; i++)
            {
                UINT triangleCount = i + 1;

                m_d3d12Context.CreateResourceWithInitialData(
                    pInitialData.data(),
                    sizeof(float) * floatsPerTriangle * triangleCount,
                    &pVertexBuffers[i]);

                D3D12_RAYTRACING_GEOMETRY_DESC &geometryDesc = pGeometryDescs[i];
                geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
                geometryDesc.Triangles.IndexBuffer = 0;
                geometryDesc.Triangles.IndexCount = 0;
                geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_UNKNOWN;
                geometryDesc.Triangles.Transform3x4 = 0;
                geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
                geometryDesc.Triangles.VertexCount = triangleCount * verticesPerTriangle;
                geometryDesc.Triangles.VertexBuffer.StartAddress = pVertexBuffers[i]->GetGPUVirtualAddress();
                geometryDesc.Triangles.VertexBuffer.StrideInBytes = floatsPerVertex * sizeof(float);

                D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS getPrebuildDesc = {};
                getPrebuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
                getPrebuildDesc.NumDescs = 1;
                getPrebuildDesc.pGeometryDescs = &geometryDesc;
                getPrebuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

                D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo;

                pBuilder->GetRaytracingAccelerationStructurePrebuildInfo(
                    &getPrebuildDesc, &prebuildInfo);
                scratchBufferSize = std::max(scratchBufferSize, prebuildInfo.ScratchDataSizeInBytes);

                pBottomLevelMaxResourceSizes[i] = (UINT)(prebuildInfo.ResultDataMaxSizeInBytes);
                auto accelerationStructureDesc = CD3DX12_RESOURCE_DESC::Buffer(prebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
                device.CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &accelerationStructureDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pBottomLevelResources[i]));
                pBottomLevelResourcesGpuVA[i] = pBottomLevelResources[i]->GetGPUVirtualAddress();
            }

            CComPtr<ID3D12Resource> pScratchBuffer;
            auto scratchBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(scratchBufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            device.CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &scratchBufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pScratchBuffer));

            CComPtr<ID3D12GraphicsCommandList> pCommandList;
            m_d3d12Context.GetGraphicsCommandList(&pCommandList);
            for (UINT i = 0; i < numBottomLevels; i++)
            {
                D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelDesc = {};
                D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &bottomLevelInputs = bottomLevelDesc.Inputs;
                bottomLevelDesc.DestAccelerationStructureData = pBottomLevelResources[i]->GetGPUVirtualAddress();
                bottomLevelDesc.ScratchAccelerationStructureData = pScratchBuffer->GetGPUVirtualAddress();
                bottomLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
                bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
                bottomLevelInputs.NumDescs = 1;
                bottomLevelInputs.pGeometryDescs = &pGeometryDescs[i];
                bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;

                pBuilder->BuildRaytracingAccelerationStructure(
                    pCommandList,
                    &bottomLevelDesc,
                    nullptr);
            }

            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC postBuildDesc = {};
            postBuildDesc.DestBuffer = pOutputCountBuffer->GetGPUVirtualAddress();
            postBuildDesc.InfoType = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_CURRENT_SIZE;
            pBuilder->EmitRaytracingAccelerationStructurePostbuildInfo(
                pCommandList,
                &postBuildDesc,
                (UINT)pBottomLevelResourcesGpuVA.size(),
                pBottomLevelResourcesGpuVA.data());

            pCommandList->Close();
            m_d3d12Context.ExecuteCommandList(pCommandList);

            std::vector<UINT32> accelerationStructureSizes(numBottomLevels);
            m_d3d12Context.ReadbackResource(pOutputCountBuffer, accelerationStructureSizes.data(), (UINT)(accelerationStructureSizes.size() * sizeof(UINT32)));
            for (UINT i = 1; i < numBottomLevels; i++)
            {
                Assert::IsTrue(accelerationStructureSizes[i] != 0 &&
                    accelerationStructureSizes[i] > accelerationStructureSizes[i - 1] &&
                    accelerationStructureSizes[i] <= pBottomLevelMaxResourceSizes[i]
                    , L"Incorrect size returned from EmitRaytracingAccelerationStructurePostBuildInfo");
            }
        }

        TEST_METHOD(UpdatesAllowedAllocateMemoryGpuBVHBuilder) {
            ID3D12Device &device = m_d3d12Context.GetDevice();
            std::unique_ptr<FallbackLayer::IAccelerationStructureBuilder> pBuilder =
                std::unique_ptr<FallbackLayer::IAccelerationStructureBuilder>(
                    new FallbackLayer::GpuBvh2Builder(&device, m_d3d12Context.GetTotalLaneCount(), 0));
            InternalFallbackBuilder builderWrapper(pBuilder.get());

            UINT numVertices = VERTEX_COUNT(ReferenceVerticies0);
            UINT numTriangles = numVertices / 3;
            UINT totalNumNodes = numTriangles + numTriangles - 1; // A binary tree with N leaves will always have N - 1 internal nodes
            UINT updateExtraDataSize = (numTriangles * sizeof(UINT)) + (totalNumNodes * sizeof(UINT));

            CpuGeometryDescriptor cpuGeomDesc = CpuGeometryDescriptor(ReferenceVerticies0, numVertices);
            D3D12_RAYTRACING_GEOMETRY_DESC gpuGeomDesc = GetGeometryDesc(cpuGeomDesc);

            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelDesc = {};
            bottomLevelDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
            bottomLevelDesc.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
            bottomLevelDesc.NumDescs = 1;
            bottomLevelDesc.pGeometryDescs = &gpuGeomDesc;
            bottomLevelDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;

            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO infoNoUpdate = {};
            pBuilder->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelDesc, &infoNoUpdate);

            // Turn on updates, make sure the memory gets allocated.
            bottomLevelDesc.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;

            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO infoWithUpdate = {};
            pBuilder->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelDesc, &infoWithUpdate);

            Assert::AreEqual(infoNoUpdate.ScratchDataSizeInBytes, infoWithUpdate.ScratchDataSizeInBytes, L"Allowing update allocated scratch data.");
            Assert::IsTrue(infoWithUpdate.ResultDataMaxSizeInBytes == infoNoUpdate.ResultDataMaxSizeInBytes + updateExtraDataSize, L"Data allocated for update doesn't match expected.");
        }

        TEST_METHOD(StoreSortResultForUpdate) {
            ID3D12Device &device = m_d3d12Context.GetDevice();
            std::unique_ptr<FallbackLayer::IAccelerationStructureBuilder> pBuilder =
                std::unique_ptr<FallbackLayer::IAccelerationStructureBuilder>(
                    new FallbackLayer::GpuBvh2Builder(&device, m_d3d12Context.GetTotalLaneCount(), 0));
            InternalFallbackBuilder builderWrapper(pBuilder.get());

            UINT numVertices = VERTEX_COUNT(ReferenceVerticies1);
            UINT numTriangles = numVertices / 3;

            CpuGeometryDescriptor geomDesc = CpuGeometryDescriptor(ReferenceVerticies1, VERTEX_COUNT(ReferenceVerticies1));

            std::unique_ptr<BYTE[]> pData;
            BuildBottomLevelAccelerationStructureAndGetCpuData(
                builderWrapper,
                &geomDesc,
                1,
                pData,
                D3D12_ELEMENTS_LAYOUT_ARRAY,
                D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE);

            const BYTE *pOutputBVH = pData.get();
            BVHOffsets offsets = *(BVHOffsets*)pOutputBVH;
            AABBNode *pNodeArray = (AABBNode*)((BYTE *)pOutputBVH + offsets.offsetToBoxes);
            Primitive *pPrimitiveArray = (Primitive*)((BYTE *)pOutputBVH + offsets.offsetToVertices);
            UINT *pSortResults = (UINT *)((BYTE *)pOutputBVH + offsets.totalSize);

            for (UINT i = 0; i < numTriangles; i++) {
                UINT sortedIndex = pSortResults[i];
                Triangle sortedTriangle = pPrimitiveArray[sortedIndex].triangle;
                for (UINT vi = 0; vi < 3; vi++) {
                    Assert::AreEqual(sortedTriangle.v[vi].x, ReferenceVerticies1[i * 9 + vi * 3 + 0], L"x not equal");
                    Assert::AreEqual(sortedTriangle.v[vi].y, ReferenceVerticies1[i * 9 + vi * 3 + 1], L"y not equal");
                    Assert::AreEqual(sortedTriangle.v[vi].z, ReferenceVerticies1[i * 9 + vi * 3 + 2], L"z not equal");
                }
            }
        }

        TEST_METHOD(StoreParentIndicesForUpdate) {
            ID3D12Device &device = m_d3d12Context.GetDevice();
            std::unique_ptr<FallbackLayer::IAccelerationStructureBuilder> pBuilder =
                std::unique_ptr<FallbackLayer::IAccelerationStructureBuilder>(
                    new FallbackLayer::GpuBvh2Builder(&device, m_d3d12Context.GetTotalLaneCount(), 0));
            InternalFallbackBuilder builderWrapper(pBuilder.get());

            UINT numVertices = VERTEX_COUNT(ReferenceVerticies1);
            UINT numTriangles = numVertices / 3;
            UINT totalNumNodes = numTriangles + numTriangles - 1; // A binary tree with N leaves will always have N - 1 internal nodes

            CpuGeometryDescriptor geomDesc = CpuGeometryDescriptor(ReferenceVerticies1, VERTEX_COUNT(ReferenceVerticies1));

            std::unique_ptr<BYTE[]> pData;
            BuildBottomLevelAccelerationStructureAndGetCpuData(
                builderWrapper,
                &geomDesc,
                1,
                pData,
                D3D12_ELEMENTS_LAYOUT_ARRAY,
                D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE);

            const BYTE *pOutputBVH = pData.get();
            BVHOffsets offsets = *(BVHOffsets*)pOutputBVH;
            AABBNode *pNodeArray = (AABBNode*)((BYTE *)pOutputBVH + offsets.offsetToBoxes);
            Primitive *pPrimitiveArray = (Primitive*)((BYTE *)pOutputBVH + offsets.offsetToVertices);
            UINT *pUpdateInfoCache = (UINT *)((BYTE *)pOutputBVH + offsets.totalSize); // sort results + parent indices
            UINT *pParentIndexCache = pUpdateInfoCache + numTriangles;

            for (UINT parentIndex = 0; parentIndex < totalNumNodes; parentIndex++) {
                AABBNode parent = pNodeArray[parentIndex];
                if (!parent.leaf)
                {
                    UINT leftChildIndex = parent.internalNode.leftNodeIndex;
                    UINT rightChildIndex = parent.rightNodeIndex;
                    Assert::IsTrue(pParentIndexCache[leftChildIndex] == parentIndex, L"Left child parent index incorrectly assigned.");
                    Assert::IsTrue(pParentIndexCache[rightChildIndex] == parentIndex, L"Right child parent index incorrectly assigned.");
                }
            }
        }

        void BuildAndUpdateBottomLevelAccelerationStructure(
            const float *startVertices,
            const float *updatedVertices,
            int numVertices)
        {
            ID3D12Device &device = m_d3d12Context.GetDevice();
            std::unique_ptr<FallbackLayer::IAccelerationStructureBuilder> pBuilder =
                std::unique_ptr<FallbackLayer::IAccelerationStructureBuilder>(
                    new FallbackLayer::GpuBvh2Builder(&device, m_d3d12Context.GetTotalLaneCount(), 0));
            InternalFallbackBuilder builderWrapper(pBuilder.get());

            const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;

            const UINT floatsPerVertex = 3;
            const UINT verticesPerTriangle = 3;
            const UINT floatsPerTriangle = verticesPerTriangle * floatsPerVertex;
            UINT numFloats = numVertices * floatsPerVertex;
            UINT numTriangles = numVertices / 3;
            UINT totalNumNodes = numTriangles + numTriangles - 1; // A binary tree with N leaves will always have N - 1 internal nodes

            CComPtr<ID3D12Resource> pVertexBuffer;
            m_d3d12Context.CreateResourceWithInitialData(
                startVertices,
                sizeof(float) * numFloats,
                &pVertexBuffer);

            CComPtr<ID3D12Resource> pUpdatedVertexBuffer;
            m_d3d12Context.CreateResourceWithInitialData(
                updatedVertices,
                sizeof(float) * numFloats,
                &pUpdatedVertexBuffer);

            D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
            geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
            geometryDesc.Triangles.IndexBuffer = 0;
            geometryDesc.Triangles.IndexCount = 0;
            geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_UNKNOWN;
            geometryDesc.Triangles.Transform3x4 = 0;
            geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
            geometryDesc.Triangles.VertexCount = numVertices;
            geometryDesc.Triangles.VertexBuffer.StartAddress = pVertexBuffer->GetGPUVirtualAddress();
            geometryDesc.Triangles.VertexBuffer.StrideInBytes = floatsPerVertex * sizeof(float);

            auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelDesc = {};
            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &bottomLevelInputs = bottomLevelDesc.Inputs;
            bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
            bottomLevelInputs.NumDescs = 1;
            bottomLevelInputs.pGeometryDescs = &geometryDesc;
            bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
            bottomLevelInputs.Flags = buildFlags;

            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo;
            pBuilder->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &prebuildInfo);

            CComPtr<ID3D12Resource> pBottomLevelResource;
            D3D12_GPU_VIRTUAL_ADDRESS bottomLevelGpuVA;
            auto accelerationStructureDesc = CD3DX12_RESOURCE_DESC::Buffer(prebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            device.CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &accelerationStructureDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pBottomLevelResource));
            bottomLevelGpuVA = pBottomLevelResource->GetGPUVirtualAddress();

            CComPtr<ID3D12Resource> pScratchBufferResources[2];
            auto scratchBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(prebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            device.CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &scratchBufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pScratchBufferResources[0]));
            device.CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &scratchBufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pScratchBufferResources[1]));

            CComPtr<ID3D12GraphicsCommandList> pCommandList;
            m_d3d12Context.GetGraphicsCommandList(&pCommandList);

            bottomLevelDesc.DestAccelerationStructureData = bottomLevelGpuVA;
            bottomLevelDesc.ScratchAccelerationStructureData = pScratchBufferResources[0]->GetGPUVirtualAddress();

            pBuilder->BuildRaytracingAccelerationStructure(
                pCommandList,
                &bottomLevelDesc,
                nullptr);

            // Clear scratch buffer and signal perform update.
            bottomLevelDesc.ScratchAccelerationStructureData = pScratchBufferResources[1]->GetGPUVirtualAddress();
            bottomLevelInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
            geometryDesc.Triangles.VertexBuffer.StartAddress = pUpdatedVertexBuffer->GetGPUVirtualAddress();

            
            pBuilder->BuildRaytracingAccelerationStructure(
                pCommandList,
                &bottomLevelDesc,
                nullptr);
            
            pCommandList->Close();
            m_d3d12Context.ExecuteCommandList(pCommandList);
            m_d3d12Context.WaitForGpuWork();

            // Check AABBNode before & after, a little too similar
            std::unique_ptr<BYTE[]> outputData = std::unique_ptr<BYTE[]>(new BYTE[(UINT)prebuildInfo.ResultDataMaxSizeInBytes]);
            Assert::AreNotEqual(outputData.get(), (BYTE *)nullptr, L"Failed to allocate output data");

            m_d3d12Context.ReadbackResource(pBottomLevelResource, outputData.get(), (UINT)prebuildInfo.ResultDataMaxSizeInBytes);

            CpuGeometryDescriptor cpuGeomDescAfter = CpuGeometryDescriptor(updatedVertices, numVertices);

            std::wstring errorMessage;
            auto &validator = FallbackLayer::GetAccelerationStructureValidator(pBuilder->GetAccelerationStructureType());
            if (!validator.VerifyBottomLevelOutput(&cpuGeomDescAfter, 1, outputData.get(), errorMessage))
            {
                Assert::Fail(errorMessage.c_str());
            }
        }

        TEST_METHOD(RefitAABBsOnUpdate) {
            const UINT numVertices = VERTEX_COUNT(ReferenceVerticies1);
            // Scramble up the triangles
            std::vector<float> UpdatedVertices(numVertices * 3);
            for (UINT i = 0; i < UpdatedVertices.size(); i++)
            {
                UINT vertexIndex = i / 3;
                UINT triangleIndex = vertexIndex / 3;
                UpdatedVertices[i] = (((ReferenceVerticies1[i] + vertexIndex) * (vertexIndex % 2 == 0 ? -8 : 8))) * ((triangleIndex % 2 == 0) ? -12 : 12);
            }

            BuildAndUpdateBottomLevelAccelerationStructure(
                ReferenceVerticies1,
                UpdatedVertices.data(),
                numVertices
            );
        }

    private:
#define TEST_EPSILON 0.001
        bool IsChildContainedByParent(const AABB &parent, const AABB &child)
        {
            return
                parent.min.x - TEST_EPSILON <= child.min.x &&
                parent.min.y - TEST_EPSILON <= child.min.y &&
                parent.min.z - TEST_EPSILON <= child.min.z &&

                parent.max.x + TEST_EPSILON >= child.max.x &&
                parent.max.y + TEST_EPSILON >= child.max.y &&
                parent.max.z + TEST_EPSILON >= child.max.z;
        }

        void VerifyTopLevelAccelerationStructure(const void *pOutputData, UINT numBottomLevels, AABB *pExpectedBoxes)
        {
            UINT numElements = *(UINT *)pOutputData;
            Assert::IsTrue(numElements == numBottomLevels, L"Unexpected amount of bottom level structures reported");

            AABBNode *pBoxes = (AABBNode*)((BYTE*)pOutputData + sizeof(UINT32));


            std::vector<bool> isBoxFound(numBottomLevels);
            for (UINT i = 0; i < numBottomLevels; i++) isBoxFound[i] = false;

            for (UINT i = 0; i < numBottomLevels; i++)
            {
                Assert::IsTrue(pBoxes[i].leaf, L"Box not marked as leaf");
                AABB leafAABB;
                FallbackLayer::DecompressAABB(leafAABB, pBoxes[i]);

                for (UINT expectedBoxesIndex = 0; expectedBoxesIndex < numBottomLevels; expectedBoxesIndex++)
                {
                    if (!isBoxFound[expectedBoxesIndex] && IsChildContainedByParent(leafAABB, pExpectedBoxes[expectedBoxesIndex]))
                    {
                        isBoxFound[expectedBoxesIndex] = true;
                    }
                }
            }

            for (UINT i = 0; i < numBottomLevels; i++)
            {
                if (!isBoxFound[i])
                {
                    Assert::Fail(L"Bottom level BVH not contained in top level BVH");
                }
            }
        }

        void TestCpuBvh2Builder(CpuGeometryDescriptor *pGeomDescs, UINT numGeoms, D3D12_ELEMENTS_LAYOUT layoutToTest = D3D12_ELEMENTS_LAYOUT_ARRAY)
        {
            ID3D12Device &device = m_d3d12Context.GetDevice();
            std::unique_ptr<FallbackLayer::IAccelerationStructureBuilder> pBuilder =
                std::unique_ptr<FallbackLayer::IAccelerationStructureBuilder>(
                    new FallbackLayer::GpuBvh2Builder(&device, m_d3d12Context.GetTotalLaneCount(), 0));
            InternalFallbackBuilder builderWrapper(pBuilder.get());

            std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geomDescs(numGeoms);
            for (UINT i = 0; i < numGeoms; i++)
            {
                geomDescs[i].Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
                auto &triangleDesc = geomDescs[i].Triangles;
                triangleDesc.IndexBuffer = (D3D12_GPU_VIRTUAL_ADDRESS)pGeomDescs[i].m_pIndexBuffer;
                triangleDesc.VertexBuffer.StartAddress = (D3D12_GPU_VIRTUAL_ADDRESS)pGeomDescs[i].m_pVertexData;
                triangleDesc.IndexFormat = pGeomDescs[i].m_indexBufferFormat;
                triangleDesc.IndexCount = pGeomDescs[i].m_numIndicies;
                triangleDesc.VertexCount = pGeomDescs[i].m_numVerticies;
                triangleDesc.VertexBuffer.StrideInBytes = sizeof(float) * 3;
            }

            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo;
            builderWrapper.GetRaytracingAccelerationStructurePrebuildInfo(&device,
                D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,
                D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD,
                numGeoms,
                geomDescs.data(),
                &prebuildInfo);
            std::unique_ptr<BYTE[]> pData = std::unique_ptr<BYTE[]>(new BYTE[prebuildInfo.ResultDataMaxSizeInBytes]);

            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc = {};
            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &inputs = desc.Inputs;
            inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
            inputs.NumDescs = numGeoms;
            inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
            inputs.pGeometryDescs = geomDescs.data();

            BuildRaytracingAccelerationStructureOnCpu(&desc, pData.get());
            std::wstring errorMessage;
            auto &validator = FallbackLayer::GetAccelerationStructureValidator(pBuilder->GetAccelerationStructureType());
            if (!validator.VerifyBottomLevelOutput(pGeomDescs, numGeoms, pData.get(), errorMessage))
            {
                Assert::Fail(errorMessage.c_str());
            }
        }

        void TestCpuBvh2Builder(CpuGeometryDescriptor &geomDesc)
        {
            TestCpuBvh2Builder(&geomDesc, 1);
        }

        void TestGpuBvh2Builder(CpuGeometryDescriptor *pGeomDescs, UINT numGeoms, D3D12_ELEMENTS_LAYOUT layoutToTest = D3D12_ELEMENTS_LAYOUT_ARRAY)
        {
            ID3D12Device &device = m_d3d12Context.GetDevice();
            std::unique_ptr<FallbackLayer::IAccelerationStructureBuilder> pBuilder =
                std::unique_ptr<FallbackLayer::IAccelerationStructureBuilder>(
                    new FallbackLayer::GpuBvh2Builder(&device, m_d3d12Context.GetTotalLaneCount(), 0));
            InternalFallbackBuilder builderWrapper(pBuilder.get());

            std::unique_ptr<BYTE[]> pData;

            BuildBottomLevelAccelerationStructureAndGetCpuData(
                builderWrapper,
                pGeomDescs,
                numGeoms,
                pData,
                layoutToTest);

            std::wstring errorMessage;
            auto &validator = FallbackLayer::GetAccelerationStructureValidator(pBuilder->GetAccelerationStructureType());
            if (!validator.VerifyBottomLevelOutput(pGeomDescs, numGeoms, pData.get(), errorMessage))
            {
                Assert::Fail(errorMessage.c_str());
            }
        }

        void TestGpuBvh2Builder(CpuGeometryDescriptor &geomDesc)
        {
            TestGpuBvh2Builder(&geomDesc, 1);
        }

        void BuildBottomLevelAccelerationStructureAndGetCpuData(
            BuilderWrapper &builder,
            CpuGeometryDescriptor *pGeomDescs,
            UINT numDescs,
            std::unique_ptr<BYTE[]> &outputData,
            D3D12_ELEMENTS_LAYOUT layoutToTest = D3D12_ELEMENTS_LAYOUT_ARRAY,
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD)
        {
            ID3D12Device &device = m_d3d12Context.GetDevice();
            CComPtr<ID3D12Resource> pResource;
            m_pBuilderHelper->BuildBottomLevelAccelerationStructure(builder, pGeomDescs, numDescs, &pResource, layoutToTest, 0, buildFlags);

            outputData = std::unique_ptr<BYTE[]>(new BYTE[(UINT)pResource->GetDesc().Width]);
            Assert::AreNotEqual(outputData.get(), (BYTE *)nullptr, L"Failed to allocate output data");

            m_d3d12Context.ReadbackResource(pResource, outputData.get(), (UINT)pResource->GetDesc().Width);
        }

        D3D12Context m_d3d12Context;
        std::unique_ptr<DescriptorHeapStack> m_pDescriptorHeapStack;
        std::unique_ptr<AccelerationStructureBuilderHelper> m_pBuilderHelper;
    };

    void AllocateUAVBuffer(ID3D12Device &d3d12device, UINT64 bufferSize, ID3D12Resource **ppResource)
    {
        const auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        ThrowFailure(d3d12device.CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(ppResource)));
    }

#include "CompiledShaders/SimpleRaygen.h"
#include "CompiledShaders/ReadRootConstants.h"
#include "CompiledShaders/ReadDataTexture1D.h"
#include "CompiledShaders/ReadDataTexture1DArray.h"
#include "CompiledShaders/ReadDataTexture2D.h"
#include "CompiledShaders/ReadDataTexture2DArray.h"
#include "CompiledShaders/ReadDataBuffer.h"
#include "CompiledShaders/ReadDataRWTexture2D.h"
#include "CompiledShaders/ReadDataRWTexture2DArray.h"
#include "CompiledShaders/ReadDataRWTexture3D.h"
#include "CompiledShaders/ReadDataRWTexture1D.h"
#include "CompiledShaders/ReadDataRWTexture1DArray.h"
#include "CompiledShaders/ReadDataRWBuffer.h"
#include "CompiledShaders/ReadDataRWByteAddressBuffer.h"
#include "CompiledShaders/DynamicIndexTest.h"
    TEST_CLASS(DxilPatchingUnitTests)
    {
    public:

        void BuildEmptyTopLevelAccelerationStructure()
        {
            auto &d3d12device = m_d3d12Context.GetDevice();

            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &topLevelInputs = buildDesc.Inputs;
            topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
            topLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
            topLevelInputs.NumDescs = 0;
            topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
            topLevelInputs.pGeometryDescs = nullptr;
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo;
            m_pRaytracingDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);

            CComPtr<ID3D12Resource> pScratchResource;
            AllocateUAVBuffer(d3d12device, topLevelPrebuildInfo.ScratchDataSizeInBytes, &pScratchResource);
            AllocateUAVBuffer(d3d12device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_pTopLevelAccelerationStructure);

            UINT topLevelDescriptorIndex = m_pDescriptorHeapStack->AllocateBufferUav(*m_pTopLevelAccelerationStructure);
            m_TopLevelAccelerationStructurePointer = m_pRaytracingDevice->GetWrappedPointerSimple(topLevelDescriptorIndex, m_pTopLevelAccelerationStructure->GetGPUVirtualAddress());

            CComPtr<ID3D12GraphicsCommandList> pCommandList;
            m_d3d12Context.GetGraphicsCommandList(&pCommandList);
            CComPtr<ID3D12RaytracingFallbackCommandList> pRaytracingCommandList;
            m_pRaytracingDevice->QueryRaytracingCommandList(pCommandList, IID_PPV_ARGS(&pRaytracingCommandList));

            ID3D12DescriptorHeap *pDescriptorHeaps[] = { &m_pDescriptorHeapStack->GetDescriptorHeap() };
            pRaytracingCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);

            buildDesc.DestAccelerationStructureData = m_pTopLevelAccelerationStructure->GetGPUVirtualAddress();
            buildDesc.ScratchAccelerationStructureData = pScratchResource ? pScratchResource->GetGPUVirtualAddress() : 0;

            pRaytracingCommandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

            pCommandList->Close();
            m_d3d12Context.ExecuteCommandList(pCommandList);

            m_d3d12Context.WaitForGpuWork();
        }

        TEST_METHOD_INITIALIZE(MethodSetup)
        {
            m_pRaytracingDevice = CComPtr<FallbackLayer::RaytracingDevice>(new RaytracingDevice(&m_d3d12Context.GetDevice(), 1, CreateRaytracingFallbackDeviceFlags::None));
            InitializeFallbackRootSignature();
            InitializeDxcComponents();
            InitializeDescriptorHeaps();
            BuildEmptyTopLevelAccelerationStructure();
        }

        // Tests disabled due to existing DxCompiler issues that still need to be resolved
#if 0
        TEST_METHOD(ValidateDxilShaderRecordPatchingRootConstants)
        {
            CD3DX12_ROOT_PARAMETER rootConstant0;
            CD3DX12_ROOT_PARAMETER rootConstant1;
            CD3DX12_ROOT_PARAMETER rootConstant2;
            rootConstant0.InitAsConstants(4, 0);
            rootConstant1.InitAsConstants(4, 1);
            rootConstant2.InitAsConstants(8, 2);
            D3D12_ROOT_PARAMETER rootParams[] = { rootConstant0, rootConstant1, rootConstant2 };
            D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
            rootSignatureDesc.NumParameters = ARRAYSIZE(rootParams);
            rootSignatureDesc.pParameters = rootParams;
            CComPtr<ID3DBlob> pBlob;
            AssertSucceeded(m_pRaytracingDevice->D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pBlob, nullptr));

            CComPtr<ID3D12RootSignature> pRootSignature;
            m_pRaytracingDevice->CreateRootSignature(
                1,
                pBlob->GetBufferPointer(),
                pBlob->GetBufferSize(),
                IID_PPV_ARGS(&pRootSignature));

            float shaderTable[] = {
                1.0, 1.0, 0.0, 1.0, // Color 0

                0.0, 0.0, 1.0, 1.0, // Color 1

                -1.0, -1.0, -1.0, -1.0, // Unused padding
                0.0, 0.0, 1.0, 1.0,     // Color 2
            };
            auto shaderByteCode = CD3DX12_SHADER_BYTECODE((void *)g_pReadRootConstants, sizeof(g_pReadRootConstants));

            BYTE outputData[sizeof(shaderTable)];

            ValidateDxilShaderRecordPatching(shaderByteCode, pRootSignature, shaderTable, sizeof(shaderTable), outputData, sizeof(outputData));

            BYTE *inputData = (BYTE *)shaderTable;
            const unsigned int sizeofFloat4 = sizeof(float) * 4;
            Assert::IsTrue(memcmp(inputData, outputData, sizeofFloat4) == 0, L"Shader did not correctly output the expected color");
            Assert::IsTrue(memcmp(inputData + sizeofFloat4, outputData + sizeofFloat4, sizeofFloat4) == 0, L"Shader did not correctly output the expected color");

            // Trick case, make sure it didn't read from the padding
            Assert::IsTrue(memcmp(inputData + sizeofFloat4 * 3, outputData + sizeofFloat4 * 2, sizeofFloat4) == 0, L"Shader did not correctly output the expected color");
        }


        TEST_METHOD(ValidateDxilShaderRecordPatchingDescriptorTableSRVDynamicIndexing)
        {
            auto &device = m_d3d12Context.GetDevice();
            const UINT numSRVDescriptors = 3;

            CD3DX12_DESCRIPTOR_RANGE descriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, numSRVDescriptors, 0);
            CD3DX12_ROOT_PARAMETER rootParam;
            rootParam.InitAsDescriptorTable(1, &descriptorRange);
            const UINT descriptorSize = device.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
            rootSignatureDesc.NumParameters = 1;
            rootSignatureDesc.pParameters = &rootParam;
            CComPtr<ID3DBlob> pBlob;
            AssertSucceeded(m_pRaytracingDevice->D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pBlob, nullptr));

            CComPtr<ID3D12RootSignature> pRootSignature;
            AssertSucceeded(m_pRaytracingDevice->CreateRootSignature(
                1,
                pBlob->GetBufferPointer(),
                pBlob->GetBufferSize(),
                IID_PPV_ARGS(&pRootSignature)));

            float constantData[][4] = {
                { 1.0, 3.0, 0.0, 1.0, }, // Color 0
            { 0.0, 0.0, 1.0, 1.0, }, // Color 1
            { 0.0, 0.0, 1.0, 1.0, }  // Color 2
            };

            CComPtr<ID3D12Resource> pBuffers[numSRVDescriptors];
            const unsigned int sizeofFloat4 = sizeof(float) * 4;
            D3D12_CPU_DESCRIPTOR_HANDLE descriptorLocation;
            UINT descriptorSlot;
            m_pDescriptorHeapStack->AllocateDescriptor(descriptorLocation, descriptorSlot);
            for (UINT i = 0; i < numSRVDescriptors; i++)
            {
                const UINT constantSize = sizeofFloat4;
                m_d3d12Context.CreateResourceWithInitialData(constantData[i], constantSize, &pBuffers[i]);

                D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
                srvDesc.Buffer.NumElements = 1;

                device.CreateShaderResourceView(pBuffers[i], &srvDesc, descriptorLocation);
                descriptorLocation.ptr += descriptorSize;
            }

            D3D12_GPU_DESCRIPTOR_HANDLE shaderTable[numSRVDescriptors];
            auto gpuDescriptorHandleBase = m_pDescriptorHeapStack->GetGpuHandle(descriptorSlot);
            for (UINT i = 0; i < numSRVDescriptors; i++)
            {
                shaderTable[i].ptr = gpuDescriptorHandleBase.ptr + i * descriptorSize;
            }

            auto shaderByteCode = CD3DX12_SHADER_BYTECODE((void *)g_pDynamicIndexTest, sizeof(g_pDynamicIndexTest));

            BYTE outputData[sizeof(constantData)];

            ValidateDxilShaderRecordPatching(shaderByteCode, pRootSignature, shaderTable, sizeof(shaderTable), outputData, sizeof(outputData));
            Assert::IsTrue(memcmp(constantData, outputData, sizeof(constantData)) == 0, L"Shader did not correctly output the expected color");
        }

        TEST_METHOD(ValidateDxilShaderRecordPatchingDescriptorTableConstantsSingleTable)
        {
            ValidateDxilShaderRecordPatchingDescriptorTableConstants(false);
        }

        TEST_METHOD(ValidateDxilShaderRecordPatchingDescriptorTableConstantsMultiTable)
        {
            ValidateDxilShaderRecordPatchingDescriptorTableConstants(true);
        }
#endif

        TEST_METHOD(ValidateDxilShaderRecordPatchingDescriptorTableUAVBuffer)
        {
            auto texDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(float) * 4, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            uavDesc.Buffer.NumElements = 1;

            auto shaderByteCode = CD3DX12_SHADER_BYTECODE((void *)g_pReadDataRWBuffer, sizeof(g_pReadDataRWBuffer));

            ValidateUAVDescriptor(shaderByteCode, texDesc, uavDesc);
        }

        TEST_METHOD(ValidateDxilShaderRecordPatchingDescriptorTableUAVRawBuffer)
        {
            auto texDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(float) * 4, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            uavDesc.Buffer.NumElements = (UINT)texDesc.Width / sizeof(UINT32);
            uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

            auto shaderByteCode = CD3DX12_SHADER_BYTECODE((void *)g_pReadDataRWByteAddressBuffer, sizeof(g_pReadDataRWByteAddressBuffer));

            ValidateUAVDescriptor(shaderByteCode, texDesc, uavDesc, true);
        }

        TEST_METHOD(ValidateDxilShaderRecordPatchingDescriptorTableUAVTexture1D)
        {
            auto texDesc = CD3DX12_RESOURCE_DESC::Tex1D(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = texDesc.Format;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;

            auto shaderByteCode = CD3DX12_SHADER_BYTECODE((void *)g_pReadDataRWTexture1D, sizeof(g_pReadDataRWTexture1D));

            ValidateUAVDescriptor(shaderByteCode, texDesc, uavDesc);
        }

        TEST_METHOD(ValidateDxilShaderRecordPatchingDescriptorTableUAVTexture1DArray)
        {
            auto texDesc = CD3DX12_RESOURCE_DESC::Tex1D(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = texDesc.Format;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
            uavDesc.Texture1DArray.ArraySize = 1;

            auto shaderByteCode = CD3DX12_SHADER_BYTECODE((void *)g_pReadDataRWTexture1DArray, sizeof(g_pReadDataRWTexture1DArray));

            ValidateUAVDescriptor(shaderByteCode, texDesc, uavDesc);
        }

        TEST_METHOD(ValidateDxilShaderRecordPatchingDescriptorTableUAVTexture2D)
        {
            auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = texDesc.Format;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

            auto shaderByteCode = CD3DX12_SHADER_BYTECODE((void *)g_pReadDataRWTexture2D, sizeof(g_pReadDataRWTexture2D));

            ValidateUAVDescriptor(shaderByteCode, texDesc, uavDesc);
        }

        TEST_METHOD(ValidateDxilShaderRecordPatchingDescriptorTableUAVTexture2DArray)
        {
            auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = texDesc.Format;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
            uavDesc.Texture2DArray.ArraySize = 1;

            auto shaderByteCode = CD3DX12_SHADER_BYTECODE((void *)g_pReadDataRWTexture2DArray, sizeof(g_pReadDataRWTexture2DArray));

            ValidateUAVDescriptor(shaderByteCode, texDesc, uavDesc);
        }

        TEST_METHOD(ValidateDxilShaderRecordPatchingDescriptorTableUAVTexture3D)
        {
            auto texDesc = CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = texDesc.Format;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
            uavDesc.Texture3D.WSize = 1;

            auto shaderByteCode = CD3DX12_SHADER_BYTECODE((void *)g_pReadDataRWTexture3D, sizeof(g_pReadDataRWTexture3D));

            ValidateUAVDescriptor(shaderByteCode, texDesc, uavDesc);
        }

        TEST_METHOD(ValidateDxilShaderRecordPatchingDescriptorTableSRVTexture2D)
        {
            auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = texDesc.Format;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;

            auto shaderByteCode = CD3DX12_SHADER_BYTECODE((void *)g_pReadDataTexture2D, sizeof(g_pReadDataTexture2D));

            ValidateSRVDescriptor(shaderByteCode, texDesc, srvDesc);
        }

        TEST_METHOD(ValidateDxilShaderRecordPatchingDescriptorTableSRVTexture2DArray)
        {
            auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = texDesc.Format;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            srvDesc.Texture2DArray.MipLevels = 1;
            srvDesc.Texture2DArray.ArraySize = 1;

            auto shaderByteCode = CD3DX12_SHADER_BYTECODE((void *)g_pReadDataTexture2DArray, sizeof(g_pReadDataTexture2DArray));

            ValidateSRVDescriptor(shaderByteCode, texDesc, srvDesc);
        }

        TEST_METHOD(ValidateDxilShaderRecordPatchingDescriptorTableSRVTexture1DArray)
        {
            auto &device = m_d3d12Context.GetDevice();

            auto texDesc = CD3DX12_RESOURCE_DESC::Tex1D(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = texDesc.Format;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
            srvDesc.Texture1DArray.ArraySize = 1;
            srvDesc.Texture1DArray.MipLevels = 1;

            auto shaderByteCode = CD3DX12_SHADER_BYTECODE((void *)g_pReadDataTexture1DArray, sizeof(g_pReadDataTexture1DArray));
            ValidateSRVDescriptor(shaderByteCode, texDesc, srvDesc);
        }

        TEST_METHOD(ValidateDxilShaderRecordPatchingDescriptorTableSRVTexture1D)
        {
            auto &device = m_d3d12Context.GetDevice();

            auto texDesc = CD3DX12_RESOURCE_DESC::Tex1D(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = texDesc.Format;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
            srvDesc.Texture1D.MipLevels = 1;

            auto shaderByteCode = CD3DX12_SHADER_BYTECODE((void *)g_pReadDataTexture1D, sizeof(g_pReadDataTexture1D));
            ValidateSRVDescriptor(shaderByteCode, texDesc, srvDesc);
        }

        TEST_METHOD(ValidateDxilShaderRecordPatchingDescriptorTableSRVBuffer)
        {
            auto &device = m_d3d12Context.GetDevice();

            auto texDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(float) * 4, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srvDesc.Buffer.NumElements = 1;

            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = srvDesc.Format;
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_BUFFER;
            rtvDesc.Buffer.NumElements = 1;

            auto shaderByteCode = CD3DX12_SHADER_BYTECODE((void *)g_pReadDataTexture1D, sizeof(g_pReadDataTexture1D));
            ValidateSRVDescriptor(shaderByteCode, texDesc, srvDesc, &rtvDesc);
        }
    private:

        void ValidateDxilShaderRecordPatchingDescriptorTableConstants(bool GenerateTablePerDescriptor)
        {
            auto &device = m_d3d12Context.GetDevice();
            const UINT numCBVDescriptors = 3;

            UINT numParameters;
            CD3DX12_DESCRIPTOR_RANGE range[numCBVDescriptors];
            CD3DX12_ROOT_PARAMETER rootParam[numCBVDescriptors];
            if (GenerateTablePerDescriptor)
            {
                for (UINT i = 0; i < numCBVDescriptors; i++)
                {
                    range[i] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, i);
                    rootParam[i].InitAsDescriptorTable(1, &range[i]);
                }
                numParameters = numCBVDescriptors;
            }
            else
            {
                range[0] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, numCBVDescriptors, 0);
                rootParam[0].InitAsDescriptorTable(1, &range[0]);
                numParameters = 1;
            }

            const UINT descriptorSize = device.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
            rootSignatureDesc.NumParameters = numParameters;
            rootSignatureDesc.pParameters = rootParam;
            CComPtr<ID3DBlob> pBlob;
            AssertSucceeded(m_pRaytracingDevice->D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pBlob, nullptr));

            CComPtr<ID3D12RootSignature> pRootSignature;
            AssertSucceeded(m_pRaytracingDevice->CreateRootSignature(
                1,
                pBlob->GetBufferPointer(),
                pBlob->GetBufferSize(),
                IID_PPV_ARGS(&pRootSignature)));

            float constantData[][4] = {
                { 1.0, 3.0, 0.0, 1.0, }, // Color 0
            { 0.0, 0.0, 1.0, 1.0, }, // Color 1
            {}, // Padding
            { 0.0, 0.0, 1.0, 1.0, }  // Color 2
            };

            CComPtr<ID3D12Resource> pConstantResources[numCBVDescriptors];
            const unsigned int sizeofFloat4 = sizeof(float) * 4;
            D3D12_CPU_DESCRIPTOR_HANDLE descriptorLocation;
            UINT descriptorSlot;
            m_pDescriptorHeapStack->AllocateDescriptor(descriptorLocation, descriptorSlot);
            for (UINT i = 0; i < numCBVDescriptors; i++)
            {
                const UINT constantSize = i != 2 ? sizeofFloat4 : sizeofFloat4 * 2;
                m_d3d12Context.CreateResourceWithInitialData(constantData[i], constantSize, &pConstantResources[i]);

                D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc;
                cbDesc.SizeInBytes = std::max(constantSize, (UINT)D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
                cbDesc.BufferLocation = pConstantResources[i]->GetGPUVirtualAddress();
                device.CreateConstantBufferView(&cbDesc, descriptorLocation);

                descriptorLocation.ptr += descriptorSize;
            }

            D3D12_GPU_DESCRIPTOR_HANDLE shaderTable[numCBVDescriptors];
            auto gpuDescriptorHandleBase = m_pDescriptorHeapStack->GetGpuHandle(descriptorSlot);
            for (UINT i = 0; i < numCBVDescriptors; i++)
            {
                shaderTable[i].ptr = gpuDescriptorHandleBase.ptr + i * descriptorSize;
            }

            auto shaderByteCode = CD3DX12_SHADER_BYTECODE((void *)g_pReadRootConstants, sizeof(g_pReadRootConstants));

            BYTE outputData[sizeof(constantData)];

            ValidateDxilShaderRecordPatching(shaderByteCode, pRootSignature, shaderTable, sizeof(shaderTable), sizeof(D3D12_GPU_DESCRIPTOR_HANDLE), outputData, sizeof(outputData));
            BYTE *inputData = (BYTE *)constantData;
            Assert::IsTrue(memcmp(inputData, outputData, sizeofFloat4) == 0, L"Shader did not correctly output the expected color");
            Assert::IsTrue(memcmp(inputData + sizeofFloat4, outputData + sizeofFloat4, sizeofFloat4) == 0, L"Shader did not correctly output the expected color");

            // Trick case, make sure it didn't read from the padding
            Assert::IsTrue(memcmp(inputData + sizeofFloat4 * 3, outputData + sizeofFloat4 * 2, sizeofFloat4) == 0, L"Shader did not correctly output the expected color");
        }

        void ValidateSRVDescriptor(D3D12_SHADER_BYTECODE &computeShader, D3D12_RESOURCE_DESC &resDesc, D3D12_SHADER_RESOURCE_VIEW_DESC &srvDesc, D3D12_RENDER_TARGET_VIEW_DESC *pRTVDesc = nullptr)
        {
            auto &device = m_d3d12Context.GetDevice();

            CComPtr<ID3D12Resource> pTexture;
            auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            AssertSucceeded(device.CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, nullptr, IID_PPV_ARGS(&pTexture)));

            CComPtr<ID3D12DescriptorHeap> pRTVDescriptorHeap;
            D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
            descriptorHeapDesc.NumDescriptors = 1;
            descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            AssertSucceeded(device.CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&pRTVDescriptorHeap)));
            auto RTVHandle = pRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            device.CreateRenderTargetView(pTexture, pRTVDesc, RTVHandle);

            D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle;
            UINT descriptorSlot;
            m_pDescriptorHeapStack->AllocateDescriptor(descriptorHandle, descriptorSlot);
            device.CreateShaderResourceView(pTexture, &srvDesc, descriptorHandle);

            ValidateViewDescriptor(computeShader, pTexture, m_pDescriptorHeapStack->GetGpuHandle(descriptorSlot), RTVHandle, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, false);
        }

        void ValidateUAVDescriptor(D3D12_SHADER_BYTECODE &computeShader, D3D12_RESOURCE_DESC &resDesc, D3D12_UNORDERED_ACCESS_VIEW_DESC &uavDesc, bool clearUsingUINT = false)
        {
            auto &device = m_d3d12Context.GetDevice();

            CComPtr<ID3D12Resource> pTexture;
            auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            AssertSucceeded(device.CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&pTexture)));

            auto cpuUAVHandle = m_pCpuCbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

            D3D12_CPU_DESCRIPTOR_HANDLE cpuShaderVisibleUAVHandle;
            UINT gpuDescriptorSlot;
            m_pDescriptorHeapStack->AllocateDescriptor(cpuShaderVisibleUAVHandle, gpuDescriptorSlot);
            auto gpuUAVHandle = m_pDescriptorHeapStack->GetGpuHandle(gpuDescriptorSlot);
            device.CreateUnorderedAccessView(pTexture, nullptr, &uavDesc, cpuUAVHandle);
            device.CopyDescriptorsSimple(1, cpuShaderVisibleUAVHandle, cpuUAVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            ValidateViewDescriptor(computeShader, pTexture, gpuUAVHandle, cpuUAVHandle, D3D12_DESCRIPTOR_RANGE_TYPE_UAV, clearUsingUINT);
        }

        void ValidateViewDescriptor(D3D12_SHADER_BYTECODE &computeShader, ID3D12Resource *pTexture, D3D12_GPU_DESCRIPTOR_HANDLE gpuOutputViewHandle, D3D12_CPU_DESCRIPTOR_HANDLE cpuOutputViewHandle, D3D12_DESCRIPTOR_RANGE_TYPE viewType, bool bClearUsingUINT)
        {
            auto &device = m_d3d12Context.GetDevice();

            CD3DX12_DESCRIPTOR_RANGE viewRange(viewType, 1, 1);
            CD3DX12_ROOT_PARAMETER rootDescriptorTable;
            rootDescriptorTable.InitAsDescriptorTable(1, &viewRange);
            D3D12_ROOT_PARAMETER rootParams[] = { rootDescriptorTable };
            D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
            rootSignatureDesc.NumParameters = ARRAYSIZE(rootParams);
            rootSignatureDesc.pParameters = rootParams;
            rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
            CComPtr<ID3DBlob> pBlob;
            AssertSucceeded(m_pRaytracingDevice->D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pBlob, nullptr));

            CComPtr<ID3D12RootSignature> pRootSignature;
            AssertSucceeded(m_pRaytracingDevice->CreateRootSignature(
                1,
                pBlob->GetBufferPointer(),
                pBlob->GetBufferSize(),
                IID_PPV_ARGS(&pRootSignature)));

            CComPtr<ID3D12GraphicsCommandList> pCommandList;
            m_d3d12Context.GetGraphicsCommandList(&pCommandList);
            float clearColor[4] = { 1.0, 0.5, 0.0, 1.0, };
            UINT clearColorUINT[4] = { 255, 0, 0, 0 };
            if (viewType == D3D12_DESCRIPTOR_RANGE_TYPE_SRV)
            {
                pCommandList->ClearRenderTargetView(cpuOutputViewHandle, clearColor, 0, nullptr);
                auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(pTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
                pCommandList->ResourceBarrier(1, &barrier);
            }
            else if (viewType == D3D12_DESCRIPTOR_RANGE_TYPE_UAV)
            {
                if (bClearUsingUINT)
                {
                    pCommandList->ClearUnorderedAccessViewUint(gpuOutputViewHandle, cpuOutputViewHandle, pTexture, clearColorUINT, 0, nullptr);
                }
                else
                {
                    pCommandList->ClearUnorderedAccessViewFloat(gpuOutputViewHandle, cpuOutputViewHandle, pTexture, clearColor, 0, nullptr);
                }
            }
            pCommandList->Close();
            m_d3d12Context.ExecuteCommandList(pCommandList);

            D3D12_GPU_DESCRIPTOR_HANDLE shaderTable[] = {
                gpuOutputViewHandle
            };

            BYTE outputData[sizeof(clearColor)];
            ValidateDxilShaderRecordPatching(computeShader, pRootSignature, shaderTable, sizeof(shaderTable), sizeof(D3D12_GPU_DESCRIPTOR_HANDLE), outputData, sizeof(outputData));

            if (bClearUsingUINT)
            {
                // Only compare the first element since the clear will not use the other channels for byte address buffers
                Assert::IsTrue(memcmp(outputData, clearColorUINT, sizeof(clearColorUINT[0])) == 0, L"Shader did not correctly output the expected color");
            }
            else
            {
                Assert::IsTrue(IsFloatArrayEqual((float*)outputData, clearColor, ARRAYSIZE(clearColor)), L"Shader did not correctly output the expected color");
            }
        }

        void CreateStateObject(ID3D12RaytracingFallbackStateObject **ppStateObject, ID3D12RootSignature *localRootSignature, D3D12_SHADER_BYTECODE missShader, LPCWSTR missShaderExportName)
        {
            std::vector<D3D12_STATE_SUBOBJECT> subObjects;
            subObjects.reserve(10);

            D3D12_EXPORT_DESC raygenExport = { L"raygen", nullptr, D3D12_EXPORT_FLAG_NONE };

            D3D12_DXIL_LIBRARY_DESC libraryDesc = {};
            libraryDesc.DXILLibrary = CD3DX12_SHADER_BYTECODE((void *)g_pSimpleRaygen, sizeof(g_pSimpleRaygen));
            libraryDesc.NumExports = 1;
            libraryDesc.pExports = &raygenExport;
            D3D12_STATE_SUBOBJECT DxilLibrarySubObject = {};
            DxilLibrarySubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
            DxilLibrarySubObject.pDesc = &libraryDesc;
            subObjects.push_back(DxilLibrarySubObject);

            D3D12_EXPORT_DESC missExport = { missShaderExportName, nullptr, D3D12_EXPORT_FLAG_NONE };
            D3D12_DXIL_LIBRARY_DESC missLibraryDesc = {};
            missLibraryDesc.DXILLibrary = missShader;
            missLibraryDesc.NumExports = 1;
            missLibraryDesc.pExports = &missExport;
            D3D12_STATE_SUBOBJECT MissSubObject = {};
            MissSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
            MissSubObject.pDesc = &missLibraryDesc;
            subObjects.push_back(MissSubObject);

            D3D12_STATE_SUBOBJECT localRootSignatureSubObject;
            localRootSignatureSubObject.pDesc = &localRootSignature;
            localRootSignatureSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
            subObjects.push_back(localRootSignatureSubObject);
            D3D12_STATE_SUBOBJECT &localRSSubobjectRef = subObjects.back();

            D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION shaderAssociationDesc;
            shaderAssociationDesc.NumExports = 1;
            shaderAssociationDesc.pExports = &missShaderExportName;
            shaderAssociationDesc.pSubobjectToAssociate = &localRSSubobjectRef;

            D3D12_STATE_SUBOBJECT localRSAssociation;
            localRSAssociation.pDesc = &shaderAssociationDesc;
            localRSAssociation.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
            subObjects.push_back(localRSAssociation);

            D3D12_STATE_SUBOBJECT shaderConfigSubObject;
            D3D12_RAYTRACING_SHADER_CONFIG shaderConfig;
            shaderConfig.MaxAttributeSizeInBytes = shaderConfig.MaxPayloadSizeInBytes = 8;
            shaderConfigSubObject.pDesc = &shaderConfig;
            shaderConfigSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
            subObjects.push_back(shaderConfigSubObject);

            D3D12_STATE_SUBOBJECT pipelineConfigSubObject;
            D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig;
            pipelineConfig.MaxTraceRecursionDepth = 2;
            pipelineConfigSubObject.pDesc = &pipelineConfig;
            pipelineConfigSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
            subObjects.push_back(pipelineConfigSubObject);

            D3D12_STATE_SUBOBJECT rootSignatureSubObject;
            rootSignatureSubObject.pDesc = &m_pFallbackRootSignature.p;
            rootSignatureSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
            subObjects.push_back(rootSignatureSubObject);

            D3D12_STATE_OBJECT_DESC stateObject;
            stateObject.NumSubobjects = (UINT)subObjects.size();
            stateObject.pSubobjects = subObjects.data();
            stateObject.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

            AssertSucceeded(m_pRaytracingDevice->CreateStateObject(&stateObject, IID_PPV_ARGS(ppStateObject)));
        }

        void ValidateDxilShaderRecordPatching(D3D12_SHADER_BYTECODE &computeShader, ID3D12RootSignature *pRootSignature, void *pShaderTableData, UINT shaderTableSize, UINT shaderTableAlignment, void *pOutputData, UINT outputDataSize)
        {
            CComPtr<ID3D12VersionedRootSignatureDeserializer> pDeserializer;
            const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignatureDesc = GetDescFromRootSignature(pRootSignature, pDeserializer);

            CComPtr<ID3D12RaytracingFallbackStateObject> pStateObject;
            const LPCWSTR missExportName = L"miss";
            CreateStateObject(&pStateObject, pRootSignature, computeShader, missExportName);
            const UINT shaderIndentifierSize = m_pRaytracingDevice->GetShaderIdentifierSize();

            // Insert the shader identifier into the input data
            UINT alignedShaderIdentifierSize = ALIGN(shaderTableAlignment, shaderIndentifierSize);
            std::vector<BYTE> patchedShaderTable(alignedShaderIdentifierSize + shaderTableSize);
            memcpy(patchedShaderTable.data(), pStateObject->GetShaderIdentifier(missExportName), shaderIndentifierSize);
            memcpy(patchedShaderTable.data() + alignedShaderIdentifierSize, pShaderTableData, shaderTableSize);
            CComPtr<ID3D12Resource> pShaderRecord;
            m_d3d12Context.CreateResourceWithInitialData(patchedShaderTable.data(), patchedShaderTable.size(), &pShaderRecord);

            CComPtr<ID3D12Resource> pOutputBuffer;
            auto uavBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(outputDataSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            auto &device = m_d3d12Context.GetDevice();
            AssertSucceeded(device.CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavBufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pOutputBuffer)));

            CComPtr<ID3D12Resource> pRaygenTable;
            m_d3d12Context.CreateResourceWithInitialData(pStateObject->GetShaderIdentifier(L"raygen"), m_pRaytracingDevice->GetShaderIdentifierSize(), &pRaygenTable);

            CComPtr<ID3D12GraphicsCommandList> pCommandList;
            m_d3d12Context.GetGraphicsCommandList(&pCommandList);
            CComPtr<ID3D12RaytracingFallbackCommandList> pRaytracingCommandList;
            m_pRaytracingDevice->QueryRaytracingCommandList(pCommandList, IID_PPV_ARGS(&pRaytracingCommandList));

            pCommandList->SetComputeRootSignature(m_pFallbackRootSignature);
            ID3D12DescriptorHeap *pHeaps[] = { &m_pDescriptorHeapStack->GetDescriptorHeap(), m_pSamplerDescriptorHeap };
            pRaytracingCommandList->SetDescriptorHeaps(ARRAYSIZE(pHeaps), pHeaps);
            pCommandList->SetComputeRootUnorderedAccessView(GlobalRootSignatureSlots::UAVParam, pOutputBuffer->GetGPUVirtualAddress());

            D3D12_DISPATCH_RAYS_DESC dispatchRaysArg = {};
            dispatchRaysArg.Depth = dispatchRaysArg.Width = dispatchRaysArg.Height = 1;
            dispatchRaysArg.RayGenerationShaderRecord.StartAddress = pRaygenTable->GetGPUVirtualAddress();
            dispatchRaysArg.RayGenerationShaderRecord.SizeInBytes = shaderIndentifierSize;
            dispatchRaysArg.MissShaderTable.StartAddress = pShaderRecord->GetGPUVirtualAddress();
            dispatchRaysArg.MissShaderTable.StrideInBytes = dispatchRaysArg.MissShaderTable.SizeInBytes = patchedShaderTable.size();

            pRaytracingCommandList->SetTopLevelAccelerationStructure(GlobalRootSignatureSlots::AccelerationStructureParam, m_TopLevelAccelerationStructurePointer);
            pRaytracingCommandList->SetPipelineState1(pStateObject);
            pRaytracingCommandList->DispatchRays(&dispatchRaysArg);
            AssertSucceeded(pCommandList->Close());

            m_d3d12Context.ExecuteCommandList(pCommandList);
            m_d3d12Context.WaitForGpuWork();

            m_d3d12Context.ReadbackResource(pOutputBuffer, pOutputData, outputDataSize);
        }

        void InitializeDxcComponents()
        {
            ThrowFailure(m_dxcSupport.Initialize());
        }

        void InitializeDescriptorHeaps()
        {
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            heapDesc.NumDescriptors = 100;
            heapDesc.NodeMask = 0;
            AssertSucceeded(m_d3d12Context.GetDevice().CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_pCpuCbvSrvUavDescriptorHeap)));

            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            AssertSucceeded(m_d3d12Context.GetDevice().CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_pSamplerDescriptorHeap)));

            auto descriptorHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            m_pDescriptorHeapStack = std::unique_ptr<DescriptorHeapStack>(
                new DescriptorHeapStack(m_d3d12Context.GetDevice(), 100, descriptorHeapType, 0));
        }

        void InitializeFallbackRootSignature()
        {
            CD3DX12_ROOT_PARAMETER parameters[GlobalRootSignatureSlots::NumParameters];
            parameters[GlobalRootSignatureSlots::AccelerationStructureParam].InitAsShaderResourceView(0);
            parameters[GlobalRootSignatureSlots::UAVParam].InitAsUnorderedAccessView(0);

            D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
            rootSignatureDesc.NumParameters = ARRAYSIZE(parameters);
            rootSignatureDesc.pParameters = parameters;

            CComPtr<ID3DBlob> pBlob;
            CComPtr<ID3DBlob> pErrorBlob;
            if (FAILED(m_pRaytracingDevice->D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pBlob, &pErrorBlob)))
            {
                OutputDebugStringA((LPCSTR)pErrorBlob->GetBufferPointer());
                Assert::Fail();
            }
            AssertSucceeded(m_pRaytracingDevice->CreateRootSignature(
                0,
                pBlob->GetBufferPointer(),
                pBlob->GetBufferSize(),
                IID_PPV_ARGS(&m_pFallbackRootSignature)));
        }

        void PatchAndValidateShader(const BYTE *pShaderBytecode, UINT bytecodeLength, ShaderInfo *pShaderInfo, IDxcBlob** ppOutputBlob)
        {
            m_shaderPatcher.PatchShaderBindingTables(pShaderBytecode, bytecodeLength, pShaderInfo, ppOutputBlob);

            CComPtr<IDxcValidator> pValidator;
            CComPtr<IDxcOperationResult> pResult;
            m_dxcSupport.CreateInstance(CLSID_DxcValidator, &pValidator);
            pValidator->Validate(*ppOutputBlob, DxcValidatorFlags_Default, &pResult);

            HRESULT hr;
            AssertSucceeded(pResult->GetStatus(&hr));
            if (FAILED(hr))
            {
                CComPtr<IDxcBlobEncoding> pErrorText;
                pResult->GetErrorBuffer(&pErrorText);
                OutputDebugStringA((char *)pErrorText->GetBufferPointer());
            }
            AssertSucceeded(hr);
        }

        enum GlobalRootSignatureSlots
        {
            UAVParam = 0,
            AccelerationStructureParam,
            NumParameters
        };

        D3D12Context m_d3d12Context;
        CComPtr<FallbackLayer::RaytracingDevice> m_pRaytracingDevice;
        CComPtr<ID3D12RootSignature> m_pFallbackRootSignature;
        CComPtr<ID3D12DescriptorHeap> m_pCpuCbvSrvUavDescriptorHeap;
        CComPtr<ID3D12DescriptorHeap> m_pSamplerDescriptorHeap;
        std::unique_ptr<DescriptorHeapStack> m_pDescriptorHeapStack;
        CComPtr<ID3D12Resource> m_pTopLevelAccelerationStructure;
        WRAPPED_GPU_POINTER m_TopLevelAccelerationStructurePointer;

        FallbackLayer::DxilShaderPatcher m_shaderPatcher;

        dxc::DxcDllSupport m_dxcSupport;
    };

#include "CompiledShaders/SimpleRaytracing.h"

    enum ParameterSlots
    {
        RootConstant = 0,
        RootCBV,
        RootSRV,
        RootUAV,
        DescriptorTable,
        NumParameters,
    };
    enum DescriptorTable
    {
        UAVRange = 0,
        SRVRange,
        CBVRange,
        NumRanges,
    };

    template<typename TD3DX12_DESCRIPTOR_RANGE, typename TD3DX12_ROOT_PARAMETER>
    void InitializeRootSignatureParameters(TD3DX12_DESCRIPTOR_RANGE *ranges, TD3DX12_ROOT_PARAMETER* parameters)
    {
        UINT cbvsUsed = 0, srvsUsed = 0, uavsUsed = 0, samplersUsed = 0;

        ranges[UAVRange].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, uavsUsed++);
        ranges[SRVRange].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, srvsUsed++);
        ranges[CBVRange].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, cbvsUsed++);

        parameters[RootConstant].InitAsConstants(1, cbvsUsed++);
        parameters[RootCBV].InitAsConstantBufferView(cbvsUsed++);
        parameters[RootSRV].InitAsShaderResourceView(srvsUsed++);
        parameters[RootUAV].InitAsUnorderedAccessView(uavsUsed++);
        parameters[DescriptorTable].InitAsDescriptorTable(NumRanges, ranges);
    }

    TEST_CLASS(APIUnitTest)
    {
        TEST_METHOD(PrebuildUint32Overflow)
        {
            auto &d3d12device = m_d3d12Context.GetDevice();

            CComPtr<ID3D12RaytracingFallbackDevice> rayTracingDevice;
            D3D12CreateRaytracingFallbackDevice(
                &m_d3d12Context.GetDevice(),
                CreateRaytracingFallbackDeviceFlags::ForceComputeFallback | CreateRaytracingFallbackDeviceFlags::EnableRootDescriptorsInShaderRecords,
                0,
                IID_PPV_ARGS(&rayTracingDevice));

            // Test Top level
            {
                D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS prebuildInfo = {};
                prebuildInfo.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
                prebuildInfo.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
                prebuildInfo.NumDescs = UINT32_MAX;
                D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildOutput;
                rayTracingDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfo, &prebuildOutput);

                // If there's UINT32_MAX input elements, it's guaranteed these will be larger since
                // the memory footprint scales based on the number of elements
                Assert::IsTrue(prebuildOutput.ResultDataMaxSizeInBytes > UINT32_MAX);
                Assert::IsTrue(prebuildOutput.ScratchDataSizeInBytes > UINT32_MAX);
            }

            // Test Bottom level
            {
                D3D12_RAYTRACING_GEOMETRY_DESC bottomLevelDescs[2] = {};
                for (auto &geometryDesc : bottomLevelDescs)
                {
                    geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
                    geometryDesc.Triangles.VertexCount = UINT32_MAX;
                    geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
                }

                D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS prebuildInfo = {};
                prebuildInfo.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
                prebuildInfo.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
                prebuildInfo.NumDescs = ARRAYSIZE(bottomLevelDescs);
                prebuildInfo.pGeometryDescs = bottomLevelDescs;
                D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildOutput;
                rayTracingDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfo, &prebuildOutput);

                // If there's UINT32_MAX input elements, it's guaranteed these will be larger since
                // the memory footprint scales based on the number of elements
                Assert::IsTrue(prebuildOutput.ResultDataMaxSizeInBytes > UINT32_MAX);
                Assert::IsTrue(prebuildOutput.ScratchDataSizeInBytes > UINT32_MAX);
            }

        }

        TEST_METHOD(GlobalRootSignatureCreation)
        {
            RootSignatureCreation(D3D12_ROOT_SIGNATURE_FLAG_NONE);
        }

        TEST_METHOD(LocalRootSignatureCreation)
        {
            RootSignatureCreation(D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
        }

        void RootSignatureCreation(D3D12_ROOT_SIGNATURE_FLAGS flags)
        {
            auto &d3d12device = m_d3d12Context.GetDevice();

            CComPtr<ID3D12RaytracingFallbackDevice> rayTracingDevice;
            D3D12CreateRaytracingFallbackDevice(
                &m_d3d12Context.GetDevice(),
                CreateRaytracingFallbackDeviceFlags::ForceComputeFallback | CreateRaytracingFallbackDeviceFlags::EnableRootDescriptorsInShaderRecords,
                0,
                IID_PPV_ARGS(&rayTracingDevice));

            {
                CD3DX12_DESCRIPTOR_RANGE descriptorRanges[NumRanges];
                CD3DX12_ROOT_PARAMETER parameters[NumParameters];
                InitializeRootSignatureParameters(descriptorRanges, parameters);

                auto rootSignatureDesc = CD3DX12_ROOT_SIGNATURE_DESC(ARRAYSIZE(parameters), parameters, 0, nullptr, flags);

                {
                    CComPtr<ID3DBlob> pRootSignatureBlob;
                    CComPtr<ID3D12RootSignature> pRootSignature;

                    //AssertSucceeded(rayTracingDevice->D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pRootSignatureBlob, nullptr));
                    CComPtr<ID3DBlob> pErrorBlob;
                    if (FAILED(rayTracingDevice->D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pRootSignatureBlob, &pErrorBlob)))
                    {
                        OutputDebugStringA((LPCSTR)pErrorBlob->GetBufferPointer());
                        Assert::Fail();
                    }

                    AssertSucceeded(rayTracingDevice->CreateRootSignature(0, pRootSignatureBlob->GetBufferPointer(), pRootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&pRootSignature)));
                }

                // Do the same thing but with the versioned variant
                {
                    auto versionedRootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(rootSignatureDesc);

                    CComPtr<ID3DBlob> pRootSignatureBlob;
                    CComPtr<ID3D12RootSignature> pRootSignature;
                    AssertSucceeded(rayTracingDevice->D3D12SerializeVersionedRootSignature(&versionedRootSignatureDesc, &pRootSignatureBlob, nullptr));
                    AssertSucceeded(rayTracingDevice->CreateRootSignature(0, pRootSignatureBlob->GetBufferPointer(), pRootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&pRootSignature)));
                }
            }

            {
                CD3DX12_DESCRIPTOR_RANGE1 descriptorRanges[NumRanges];
                CD3DX12_ROOT_PARAMETER1 parameters[NumParameters];
                InitializeRootSignatureParameters(descriptorRanges, parameters);

                auto versionedRootSignatureDesc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(ARRAYSIZE(parameters), parameters, 0, nullptr, flags);
                {
                    CComPtr<ID3DBlob> pRootSignatureBlob;
                    CComPtr<ID3D12RootSignature> pRootSignature;
                    AssertSucceeded(rayTracingDevice->D3D12SerializeVersionedRootSignature(&versionedRootSignatureDesc, &pRootSignatureBlob, nullptr));
                    AssertSucceeded(rayTracingDevice->CreateRootSignature(0, pRootSignatureBlob->GetBufferPointer(), pRootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&pRootSignature)));
                }
            }
        }


        TEST_METHOD(StateObjectCollectionTesting)
        {
            auto &d3d12device = m_d3d12Context.GetDevice();

            LPCWSTR ClosestHitExportName = L"Hit";
            LPCWSTR MissExportName = L"Miss";
            LPCWSTR RayGenExportName = L"RayGen";
            LPCWSTR HitGroupExportName = L"HitGroup";

            CComPtr<ID3D12RaytracingFallbackDevice> rayTracingDevice;
            D3D12CreateRaytracingFallbackDevice(
                &m_d3d12Context.GetDevice(),
                CreateRaytracingFallbackDeviceFlags::ForceComputeFallback,
                0,
                IID_PPV_ARGS(&rayTracingDevice));

            enum RootSignatureParams
            {
                AccelerationStructureSlot = 0,
                OutputViewSlot,
                ViewportConstantSlot,
                NumParameters
            };
            CComPtr<ID3D12RootSignature> pRootSignature;

            struct RayGenViewport
            {
                float Left;
                float Top;
                float Right;
                float Bottom;
            };

            CD3DX12_DESCRIPTOR_RANGE UAVDescriptor;
            UAVDescriptor.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
            CD3DX12_ROOT_PARAMETER rootParameters[NumParameters];
            rootParameters[OutputViewSlot].InitAsDescriptorTable(1, &UAVDescriptor);
            rootParameters[AccelerationStructureSlot].InitAsShaderResourceView(0);
            rootParameters[ViewportConstantSlot].InitAsConstants(SizeOfInUint32(RayGenViewport), 0, 0);

            CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);

            CComPtr<ID3DBlob> pRootSignatureBlob;
            AssertSucceeded(rayTracingDevice->D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pRootSignatureBlob, nullptr));
            AssertSucceeded(rayTracingDevice->CreateRootSignature(
                1,
                pRootSignatureBlob->GetBufferPointer(),
                pRootSignatureBlob->GetBufferSize(),
                IID_PPV_ARGS(&pRootSignature)));

            CComPtr<ID3D12RaytracingFallbackStateObject> pBaseStateObject;
            {
                std::vector<D3D12_STATE_SUBOBJECT> subObjects;

                D3D12_STATE_OBJECT_CONFIG stateObjectConfig;
                stateObjectConfig.Flags = D3D12_STATE_OBJECT_FLAG_NONE;
                D3D12_STATE_SUBOBJECT stateObjectConfigSubobject;
                stateObjectConfigSubobject.pDesc = &stateObjectConfig;
                stateObjectConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_STATE_OBJECT_CONFIG;
                subObjects.push_back(stateObjectConfigSubobject);

                D3D12_STATE_SUBOBJECT nodeMaskSubObject;
                UINT nodeMask = 1;
                nodeMaskSubObject.pDesc = &nodeMask;
                nodeMaskSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK;
                subObjects.push_back(nodeMaskSubObject);

                D3D12_STATE_SUBOBJECT rootSignatureSubObject;
                rootSignatureSubObject.pDesc = &pRootSignature.p;
                rootSignatureSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
                subObjects.push_back(rootSignatureSubObject);

                D3D12_STATE_SUBOBJECT shaderConfigSubObject;
                D3D12_RAYTRACING_SHADER_CONFIG shaderConfig;
                shaderConfig.MaxAttributeSizeInBytes = shaderConfig.MaxPayloadSizeInBytes = 8;
                shaderConfigSubObject.pDesc = &shaderConfig;
                shaderConfigSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
                subObjects.push_back(shaderConfigSubObject);

                D3D12_EXPORT_DESC exports[] = {
                    { ClosestHitExportName, nullptr, D3D12_EXPORT_FLAG_NONE },
                    { RayGenExportName, nullptr, D3D12_EXPORT_FLAG_NONE },
                    { MissExportName, nullptr, D3D12_EXPORT_FLAG_NONE },
                };

                D3D12_STATE_SUBOBJECT pipelineConfigSubObject;
                D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig;
                pipelineConfig.MaxTraceRecursionDepth = 2;
                pipelineConfigSubObject.pDesc = &pipelineConfig;
                pipelineConfigSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
                subObjects.push_back(pipelineConfigSubObject);

                D3D12_STATE_SUBOBJECT hitGroupSubObject;
                D3D12_HIT_GROUP_DESC hitGroupDesc = {};
                hitGroupDesc.ClosestHitShaderImport = ClosestHitExportName;
                hitGroupDesc.HitGroupExport = HitGroupExportName;
                hitGroupSubObject.pDesc = &hitGroupDesc;
                hitGroupSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
                subObjects.push_back(hitGroupSubObject);

                D3D12_DXIL_LIBRARY_DESC libraryDesc = {};
                libraryDesc.DXILLibrary = CD3DX12_SHADER_BYTECODE((void *)g_pSimpleRayTracing, ARRAYSIZE(g_pSimpleRayTracing));
                libraryDesc.NumExports = ARRAYSIZE(exports);
                libraryDesc.pExports = exports;
                D3D12_STATE_SUBOBJECT DxilLibrarySubObject = {};
                DxilLibrarySubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
                DxilLibrarySubObject.pDesc = &libraryDesc;
                subObjects.push_back(DxilLibrarySubObject);

                D3D12_STATE_OBJECT_DESC stateObject;
                stateObject.NumSubobjects = (UINT)subObjects.size();
                stateObject.pSubobjects = subObjects.data();
                stateObject.Type = D3D12_STATE_OBJECT_TYPE_COLLECTION;

                AssertSucceeded(rayTracingDevice->CreateStateObject(&stateObject, IID_PPV_ARGS(&pBaseStateObject)));
            }

            std::vector<D3D12_STATE_SUBOBJECT> subObjects;

            


            D3D12_STATE_SUBOBJECT baseSubObject = {};
            D3D12_EXISTING_COLLECTION_DESC baseCollection = {};
            baseCollection.pExistingCollection = (ID3D12StateObjectPrototype *)pBaseStateObject.p;
            baseSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION;
            baseSubObject.pDesc = &baseCollection;
            subObjects.push_back(baseSubObject);

            D3D12_STATE_OBJECT_DESC stateObject;
            stateObject.NumSubobjects = (UINT)subObjects.size();
            stateObject.pSubobjects = subObjects.data();
            stateObject.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

            CComPtr<ID3D12RaytracingFallbackStateObject> pStateObject;
            AssertSucceeded(rayTracingDevice->CreateStateObject(&stateObject, IID_PPV_ARGS(&pStateObject)));

            Assert::IsNotNull(pStateObject->GetShaderIdentifier(HitGroupExportName));

            // Make sure it's not dependant on string pointers
            std::wstring stringCopy = HitGroupExportName;
            Assert::IsNotNull(pStateObject->GetShaderIdentifier(stringCopy.c_str()));
        }


        D3D12Context m_d3d12Context;
    };

    TEST_CLASS(LBVHBuilderTests)
    {
    public:
        float3 min(float3 a, float3 b)
        {
            float3 result;
            result.x = std::min(a.x, b.x);
            result.y = std::min(a.y, b.y);
            result.z = std::min(a.z, b.z);
            return result;
        }

        float3 max(float3 a, float3 b)
        {
            float3 result;
            result.x = std::max(a.x, b.x);
            result.y = std::max(a.y, b.y);
            result.z = std::max(a.z, b.z);
            return result;
        }

#define BIT(x) (1 << (x))
        // Calculates a 30-bit Morton code for the
        // given 3D point located within the unit cube [0,1].
        unsigned int GetMortonCodeFromUnitCoord(float3 &unitCoord)
        {
            unsigned int mortonCode = 0;
            const unsigned int numBits = 10;
            unsigned int maxCoord = (UINT)pow(2, numBits);

            float3 adjustedCoord = min(max(unitCoord * (float)maxCoord, { 0, 0, 0 }), { (float)maxCoord - 1, (float)maxCoord - 1, (float)maxCoord - 1 });
            const unsigned int numAxis = 3;
            UINT coords[] = { (UINT)adjustedCoord.y, (UINT)adjustedCoord.x, (UINT)adjustedCoord.z };
            for (UINT bitIndex = 0; bitIndex < numBits; bitIndex++)
            {
                for (UINT axis = 0; axis < numAxis; axis++)
                {
                    UINT bit = BIT(bitIndex) & coords[axis];
                    if (bit)
                    {
                        mortonCode |= BIT(bitIndex * numAxis + axis);
                    }
                }
            }
            return mortonCode;
        }

        unsigned int CalculateMortonCode(float3 &centroid, AABB sceneAABB)
        {
            Assert::IsTrue(centroid.x >= sceneAABB.min.x &&
                centroid.y >= sceneAABB.min.y &&
                centroid.z >= sceneAABB.min.z, L"Centroid is not contained by scene AABB");
            Assert::IsTrue(centroid.x <= sceneAABB.max.x &&
                centroid.y <= sceneAABB.max.y &&
                centroid.z <= sceneAABB.max.z, L"Centroid is not contained by scene AABB");

            float3 unitCoord = (centroid - sceneAABB.min) / (sceneAABB.max - sceneAABB.min);
            return GetMortonCodeFromUnitCoord(unitCoord);
        }

        unsigned int CalculateMortonCode(Triangle &triangle, AABB sceneAABB)
        {
            float3 centroid = (triangle.v0 + triangle.v1 + triangle.v2) / 3.0;
            return CalculateMortonCode(centroid, sceneAABB);
        }

        unsigned int CalculateMortonCode(AABBNode &box, AABB sceneAABB)
        {
            float3 centroid = { box.center[0], box.center[1], box.center[2] };
            return CalculateMortonCode(centroid, sceneAABB);
        }

        struct MortonCodeIndexPair
        {
            UINT32 MortonCode;
            UINT32 Index;
            bool operator<(const MortonCodeIndexPair &pair)
            {
                return MortonCode < pair.MortonCode;
            }
        };

        void GenerateSceneData(
            _In_ UINT numElements,
            _In_ SceneType sceneType,
            _Out_ std::vector<byte> &pOutputData,
            _Out_ AABB &containingAABB,
            _Out_opt_ std::vector<MortonCodeIndexPair> *pOutputMortonCodes = nullptr,
            _Out_opt_ std::vector<byte> *pOutputMetadata = nullptr)
        {
            if (pOutputMortonCodes)
            {
                pOutputMortonCodes->resize(numElements);
            }

            containingAABB.min = { FLT_MAX, FLT_MAX, FLT_MAX };
            containingAABB.max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

            srand(42);
            std::vector<Primitive> triangles;
            std::vector<PrimitiveMetaData> trianglesMetadata;
            std::vector<AABBNode> boxes;
            std::vector<BVHMetadata> boxMetadata;

            UINT dataSize;
            void *pSourceData;

            UINT metadataSize;
            void *pMetadata;
            switch (sceneType)
            {
            case SceneType::Triangles:
            {
                for (UINT i = 0; i < numElements; i++)
                {
                    Primitive primitive;
                    primitive.PrimitiveType = TRIANGLE_TYPE;
                    Triangle &tri = primitive.triangle;
                    for (uint vIdx = 0; vIdx < 3; vIdx++)
                    {
                        tri.v[vIdx].x = (rand() / (float)RAND_MAX) * 1000.0f - 500.0f;
                        tri.v[vIdx].y = (rand() / (float)RAND_MAX) * 1000.0f - 500.0f;
                        tri.v[vIdx].z = (rand() / (float)RAND_MAX) * 1000.0f - 500.0f;
                        containingAABB.min = min(containingAABB.min, tri.v[vIdx]);
                        containingAABB.max = max(containingAABB.max, tri.v[vIdx]);
                    }
                    triangles.push_back(primitive);

                    if (pOutputMetadata)
                    {
                        PrimitiveMetaData metadata;
                        metadata.GeometryContributionToHitGroupIndex = 0;
                        metadata.PrimitiveIndex = i;
                        metadata.GeometryFlags = 0;
                        trianglesMetadata.push_back(metadata);
                    }
                }

                if (pOutputMortonCodes)
                {
                    for (UINT i = 0; i < numElements; i++)
                    {
                        assert(triangles[i].PrimitiveType == TRIANGLE_TYPE);
                        (*pOutputMortonCodes)[i].MortonCode = CalculateMortonCode(triangles[i].triangle, containingAABB);
                        (*pOutputMortonCodes)[i].Index = i;
                    }
                }
                if (pOutputMetadata)
                {
                    metadataSize = (UINT)(trianglesMetadata.size() * sizeof(PrimitiveMetaData));
                    pMetadata = trianglesMetadata.data();
                }

                dataSize = (UINT)(triangles.size() * sizeof(Primitive));
                pSourceData = triangles.data();
                break;
            }
            case SceneType::BottomLevelBVHs:
            {
                for (UINT i = 0; i < numElements; i++)
                {
                    AABBNode box;
                    for (uint axis = 0; axis < 3; axis++)
                    {
                        box.center[axis] = (float)rand() - (RAND_MAX / 2.0f);
                        box.halfDim[axis] = (float)rand() / 2.0f;
                    }
                    float3 boxMin = {
                        box.center[0] - box.halfDim[0],
                        box.center[1] - box.halfDim[1],
                        box.center[2] - box.halfDim[2] };

                    float3 boxMax = {
                        box.center[0] + box.halfDim[0],
                        box.center[1] + box.halfDim[1],
                        box.center[2] + box.halfDim[2] };

                    containingAABB.min = min(containingAABB.min, boxMin);
                    containingAABB.max = max(containingAABB.max, boxMax);
                    boxes.push_back(box);

                    if (pOutputMetadata)
                    {
                        BVHMetadata metadata = {};
                        metadata.instanceDesc.AccelerationStructure.GpuVA = i;
                        for (UINT j = 0; j < ARRAYSIZE(metadata.instanceDesc.Transform); j++)
                        {
                            metadata.instanceDesc.Transform[j / 4][j % 4] = (float)(i + j);
                        }
                        boxMetadata.push_back(metadata);
                    }
                }

                if (pOutputMortonCodes)
                {
                    for (UINT i = 0; i < numElements; i++)
                    {
                        (*pOutputMortonCodes)[i].MortonCode = CalculateMortonCode(boxes[i], containingAABB);
                        (*pOutputMortonCodes)[i].Index = i;
                    }
                }

                dataSize = (UINT)(boxes.size() * sizeof(AABBNode));
                pSourceData = boxes.data();

                if (pOutputMetadata)
                {
                    metadataSize = (UINT)(boxMetadata.size() * sizeof(BVHMetadata));
                    pMetadata = boxMetadata.data();
                }

                break;
            }
            }

            pOutputData.resize(dataSize);
            memcpy(pOutputData.data(), pSourceData, dataSize);

            if (pOutputMetadata)
            {
                pOutputMetadata->resize(metadataSize);
                memcpy(pOutputMetadata->data(), pMetadata, metadataSize);
            }
        }

        TEST_METHOD(TestCalculateTriangleSceneAABBPassSmall)
        {
            TestCalculateSceneAABBPass(4, SceneType::Triangles);
        }

        TEST_METHOD(TestCalculateTriangleSceneAABBPassMedium)
        {
            TestCalculateSceneAABBPass(50, SceneType::Triangles);
        }

        TEST_METHOD(TestCalculateTriangleSceneAABBPassLarge)
        {
            TestCalculateSceneAABBPass(1000, SceneType::Triangles);
        }

        TEST_METHOD(TestCalculateBVHSceneAABBPassSmall)
        {
            TestCalculateSceneAABBPass(4, SceneType::BottomLevelBVHs);
        }

        TEST_METHOD(TestCalculateBVHSceneAABBPassLarge)
        {
            TestCalculateSceneAABBPass(1000, SceneType::BottomLevelBVHs);
        }

        void TestCalculateSceneAABBPass(UINT numElements, SceneType sceneType)
        {
            AABB expectedAABB;
            std::vector<byte> outputData;
            GenerateSceneData(numElements, sceneType, outputData, expectedAABB);

            SceneAABBCalculator sceneAABBCalculator(&m_d3d12Context.GetDevice(), 0);
            UINT scratchBufferSizeNeeded = SceneAABBCalculator::ScratchBufferSizeNeeded(numElements);
            auto scratchBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(scratchBufferSizeNeeded, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

            auto &d3d12Device = m_d3d12Context.GetDevice();

            CComPtr<ID3D12Resource> pScratchBuffer;
            D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            AssertSucceeded(d3d12Device.CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &scratchBufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&pScratchBuffer)));

            CComPtr<ID3D12Resource> pOutputAABBBuffer;
            auto outputAABBBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(AABB), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            AssertSucceeded(d3d12Device.CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &outputAABBBufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pOutputAABBBuffer)));

            CComPtr<ID3D12Resource> pInputBuffer;
            m_d3d12Context.CreateResourceWithInitialData(outputData.data(), (UINT)outputData.size(), &pInputBuffer);

            CComPtr<ID3D12GraphicsCommandList> pCommandList;
            m_d3d12Context.GetGraphicsCommandList(&pCommandList);

            sceneAABBCalculator.CalculateSceneAABB(pCommandList, sceneType, pInputBuffer->GetGPUVirtualAddress(), numElements, pScratchBuffer->GetGPUVirtualAddress(), pOutputAABBBuffer->GetGPUVirtualAddress());
            pCommandList->Close();

            m_d3d12Context.ExecuteCommandList(pCommandList);
            AABB calculatedAABB;
            m_d3d12Context.ReadbackResource(pOutputAABBBuffer, &calculatedAABB, sizeof(calculatedAABB));

            Assert::IsTrue(memcmp(&expectedAABB, &calculatedAABB, sizeof(expectedAABB)) == 0, L"Calculated AAB incorrect");
        }

        bool IsMortonCodeEqual(UINT codeA, UINT codeB)
        {
            UINT mask = ~(0x7); // Mask off the first few bits since these vary based on floating point precision
            return (codeA & mask) == (codeB & mask);
        }

        TEST_METHOD(CalculatingAndSortingMortonCodesMedium)
        {
            TestCalculatingAndSortingMortonCodes(300, SceneType::Triangles);
        }

        TEST_METHOD(CalculatingAndSortingMortonCodesLarge)
        {
            TestCalculatingAndSortingMortonCodes(5000, SceneType::Triangles);
        }

        TEST_METHOD(CalculatingAndSortingMortonCodesBVHMedium)
        {
            TestCalculatingAndSortingMortonCodes(300, SceneType::BottomLevelBVHs);
        }

        TEST_METHOD(CalculatingAndSortingMortonCodesBVHLarge)
        {
            TestCalculatingAndSortingMortonCodes(5000, SceneType::BottomLevelBVHs);
        }

        void TestSortingMortonCodes(UINT numTriangles, std::vector<MortonCodeIndexPair> &expectedMortonCodes, ID3D12Resource *pMortonCodeBuffer, ID3D12Resource *pIndexBuffer)
            // Now try the sorting pass
        {
            CComPtr<ID3D12GraphicsCommandList> pCommandList;
            m_d3d12Context.GetGraphicsCommandList(&pCommandList);

            D3D12_RESOURCE_BARRIER toUAVBarriers[] = {
                CD3DX12_RESOURCE_BARRIER::Transition(pMortonCodeBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
                CD3DX12_RESOURCE_BARRIER::Transition(pIndexBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
            };
            pCommandList->ResourceBarrier(ARRAYSIZE(toUAVBarriers), toUAVBarriers);

            BitonicSort bitonicSorter(&m_d3d12Context.GetDevice(), 0);
            bitonicSorter.Sort(pCommandList, pMortonCodeBuffer->GetGPUVirtualAddress(), pIndexBuffer->GetGPUVirtualAddress(), numTriangles, false, true);

            auto toReadBackBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pMortonCodeBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
            pCommandList->ResourceBarrier(1, &toReadBackBarrier);

            pCommandList->Close();
            m_d3d12Context.ExecuteCommandList(pCommandList);

            std::vector<UINT32> calculatedMortonCodes(numTriangles);
            std::vector<UINT32> calculatedIndices(numTriangles);
            m_d3d12Context.ReadbackResource(pMortonCodeBuffer, calculatedMortonCodes.data(), (UINT)(calculatedMortonCodes.size() * sizeof(UINT32)));
            m_d3d12Context.ReadbackResource(pIndexBuffer, calculatedIndices.data(), (UINT)(calculatedIndices.size() * sizeof(UINT32)));

            std::sort(expectedMortonCodes.begin(), expectedMortonCodes.end());
            for (UINT i = 0; i < numTriangles; i++)
            {
                Assert::IsTrue(expectedMortonCodes[i].Index == calculatedIndices[i] && IsMortonCodeEqual(expectedMortonCodes[i].MortonCode, calculatedMortonCodes[i]), L"Sorted morton codes incorrect");
            }
        }

        void TestCalculatingAndSortingMortonCodes(UINT numElements, SceneType sceneType)
        {
            AABB sceneAABB;
            std::vector<byte> outputData;
            std::vector<MortonCodeIndexPair> expectedMortonCodes;
            GenerateSceneData(numElements, sceneType, outputData, sceneAABB, &expectedMortonCodes);

            auto &d3d12Device = m_d3d12Context.GetDevice();
            D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

            MortonCodesCalculator mortonCodeCalculator(&m_d3d12Context.GetDevice(), 0);

            CComPtr<ID3D12Resource> pOutputIndexBuffer;
            auto outputIndexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(numElements * sizeof(UINT32), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            AssertSucceeded(d3d12Device.CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &outputIndexBufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&pOutputIndexBuffer)));

            CComPtr<ID3D12Resource> pOutputMortonCodeBuffer;
            auto mortonCodeBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(numElements * sizeof(UINT32), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            AssertSucceeded(d3d12Device.CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &mortonCodeBufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&pOutputMortonCodeBuffer)));

            CComPtr<ID3D12Resource> pTriangleBuffer;
            m_d3d12Context.CreateResourceWithInitialData(outputData.data(), (UINT)outputData.size(), &pTriangleBuffer);

            CComPtr<ID3D12GraphicsCommandList> pCommandList;
            m_d3d12Context.GetGraphicsCommandList(&pCommandList);

            CComPtr<ID3D12Resource> pSceneAABB;
            m_d3d12Context.CreateResourceWithInitialData(&sceneAABB, sizeof(sceneAABB), &pSceneAABB);

            mortonCodeCalculator.CalculateMortonCodes(pCommandList, sceneType, pTriangleBuffer->GetGPUVirtualAddress(), numElements, pSceneAABB->GetGPUVirtualAddress(), pOutputIndexBuffer->GetGPUVirtualAddress(), pOutputMortonCodeBuffer->GetGPUVirtualAddress());
            D3D12_RESOURCE_BARRIER uavToCopySourceBarriers[] = {
                CD3DX12_RESOURCE_BARRIER::Transition(pOutputMortonCodeBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE),
                CD3DX12_RESOURCE_BARRIER::Transition(pOutputIndexBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE)
            };
            pCommandList->ResourceBarrier(ARRAYSIZE(uavToCopySourceBarriers), uavToCopySourceBarriers);
            pCommandList->Close();

            m_d3d12Context.ExecuteCommandList(pCommandList);

            std::vector<UINT32> indices(numElements);
            m_d3d12Context.ReadbackResource(pOutputIndexBuffer, indices.data(), (UINT)(indices.size() * sizeof(UINT32)));
            for (UINT i = 0; i < numElements; i++)
            {
                Assert::IsTrue(i == indices[i], L"Calculated indices incorrect");
            }

            std::vector<UINT32> calculatedMortonCodes(numElements);
            m_d3d12Context.ReadbackResource(pOutputMortonCodeBuffer, calculatedMortonCodes.data(), (UINT)(calculatedMortonCodes.size() * sizeof(UINT32)));

            for (UINT i = 0; i < numElements; i++)
            {
                Assert::IsTrue(IsMortonCodeEqual(expectedMortonCodes[i].MortonCode, calculatedMortonCodes[i]), L"Calculated morton code is incorrect");
            }

            TestSortingMortonCodes(numElements, expectedMortonCodes, pOutputMortonCodeBuffer, pOutputIndexBuffer);
        }

        TEST_METHOD(TreeletReorderingFastTrace)
        {
            TestTreeletReordering(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE);
        }

        TEST_METHOD(TreeletReorderingFastBuild)
        {
            TestTreeletReordering(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD);
        }

        void TestTreeletReordering(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flag)
        {
            // WARP TDRs when running treelet re-ordering, other vendors appear to have no issues.
            // Currently assuming a WARP-specific issue, but warrants more investigation
            m_d3d12Context = D3D12Context(D3D12Context::CreationFlags::ForceHardware);

            auto &d3d12Device = m_d3d12Context.GetDevice();
            TreeletReorder treeletReorder(&d3d12Device, 0);

            const UINT numTriangles = 16;
            std::vector<Primitive> triangleBuffer(numTriangles);
            for (INT i = 0; i < numTriangles; i++)
            {
                Primitive &primitive = triangleBuffer[i];
                primitive.PrimitiveType = TRIANGLE_TYPE;
                Triangle &tri = primitive.triangle;
                if (i < numTriangles / 2)
                {
                    tri.v0 = tri.v1 = tri.v2 = { (float)-i, 0, 0 };
                }
                else
                {
                    tri.v0 = tri.v1 = tri.v2 = { (float)i, 0, 0 };
                }
            }

            CComPtr<ID3D12Resource> pTriangleBuffer;
            m_d3d12Context.CreateResourceWithInitialData(
                triangleBuffer.data(),
                (UINT)(triangleBuffer.size() * sizeof(*triangleBuffer.data())),
                &pTriangleBuffer);

            UINT numLeafNodes = (UINT)triangleBuffer.size();
            UINT numInternalNodes = numLeafNodes - 1;
            UINT numNodes = numLeafNodes + numInternalNodes;
            std::vector<HierarchyNode> hierarchy(numNodes);
            for (UINT i = 0; i < numNodes; i++)
            {
                hierarchy[i].ParentIndex = (i - 1) / 2;
                hierarchy[i].LeftChildIndex = i * 2 + 1;
                hierarchy[i].RightChildIndex = i * 2 + 2;
            }

            CComPtr<ID3D12Resource> pHierarchyBuffer;
            m_d3d12Context.CreateResourceWithInitialData(
                hierarchy.data(),
                (UINT)(hierarchy.size() * sizeof(*hierarchy.data())),
                &pHierarchyBuffer);

            CComPtr<ID3D12GraphicsCommandList> pCommandList;
            m_d3d12Context.GetGraphicsCommandList(&pCommandList);

            auto aabbBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(AABB) * numNodes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            CComPtr<ID3D12Resource> pAABBBuffer;
            AssertSucceeded(d3d12Device.CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &aabbBufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&pAABBBuffer)));

            auto nodeCountBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT) * numNodes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            CComPtr<ID3D12Resource> pNodeCountBuffer;
            AssertSucceeded(d3d12Device.CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &nodeCountBufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&pNodeCountBuffer)));

            auto baseTreeletBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(TreeletReorder::RequiredSizeForBaseTreeletBuffers(numLeafNodes), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            CComPtr<ID3D12Resource> pBaseTreeletBuffer;
            AssertSucceeded(d3d12Device.CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &baseTreeletBufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&pBaseTreeletBuffer)));

            treeletReorder.Optimize(
                pCommandList,
                numLeafNodes,
                pHierarchyBuffer->GetGPUVirtualAddress(),
                pNodeCountBuffer->GetGPUVirtualAddress(),
                pAABBBuffer->GetGPUVirtualAddress(),
                pTriangleBuffer->GetGPUVirtualAddress(),
                pBaseTreeletBuffer->GetGPUVirtualAddress(),
                pBaseTreeletBuffer->GetGPUVirtualAddress() + sizeof(UINT),
                flag);

            pCommandList->Close();
            m_d3d12Context.ExecuteCommandList(pCommandList);

            std::vector<HierarchyNode> outputHierarchy(numNodes);
            m_d3d12Context.ReadbackResource(pHierarchyBuffer, outputHierarchy.data(), (UINT)(outputHierarchy.size() * sizeof(*outputHierarchy.data())));

            // Not needed but helpful for debugging
            std::vector<AABB> outputAABBs(numNodes);
            m_d3d12Context.ReadbackResource(pAABBBuffer, outputAABBs.data(), (UINT)(outputAABBs.size() * sizeof(*outputAABBs.data())));

            std::vector<UINT> nodeStack;
            nodeStack.push_back(0);

            UINT leafNodesFound = 0;
            while (nodeStack.size() > 0)
            {
                auto nodeIndex = nodeStack.back();
                nodeStack.pop_back();
                bool isLeaf = nodeIndex >= numInternalNodes;
                if (isLeaf)
                {
                    leafNodesFound++;
                }
                else
                {
                    UINT leftNodeIndex = outputHierarchy[nodeIndex].LeftChildIndex;
                    UINT rightNodeIndex = outputHierarchy[nodeIndex].RightChildIndex;
                    Assert::IsTrue(outputHierarchy[leftNodeIndex].ParentIndex == nodeIndex, L"Incorrectly Parent Index");
                    Assert::IsTrue(outputHierarchy[rightNodeIndex].ParentIndex == nodeIndex, L"Incorrectly Parent Index");

                    nodeStack.push_back(leftNodeIndex);
                    nodeStack.push_back(rightNodeIndex);
                }
            }
            Assert::IsTrue(leafNodesFound == numLeafNodes, L"Incorrectly constructed hierarchy");
        }

        TEST_METHOD(LoadAABBs)
        {
            TestLoadPrimitives<AABB>(PROCEDURAL_PRIMITIVE_TYPE);
        }

        TEST_METHOD(LoadTriangles)
        {
            TestLoadPrimitives<Triangle>(TRIANGLE_TYPE);
        }

        TEST_METHOD(LoadSortedAABBs)
        {
            TestLoadPrimitives<AABB>(PROCEDURAL_PRIMITIVE_TYPE, true);
        }

        TEST_METHOD(LoadSortedTriangles)
        {
            TestLoadPrimitives<Triangle>(TRIANGLE_TYPE, true);
        }

        template <typename PrimitiveObjectType>
        void TestLoadPrimitives(UINT primitiveType, bool testCachedSort = false)
        {
            const UINT primitivesPerGeometry = 4;

            D3D12_RAYTRACING_GEOMETRY_FLAGS geometryFlags[] =
            {
                D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,
                D3D12_RAYTRACING_GEOMETRY_FLAG_NONE,
                D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION
            };
            const UINT numGeometryDescs = ARRAYSIZE(geometryFlags);

            const UINT totalPrimCount = numGeometryDescs * primitivesPerGeometry;
            const UINT primitiveSizeBytes = sizeof(PrimitiveObjectType);
            const UINT geomDescStartAddrStride = primitivesPerGeometry * primitiveSizeBytes;
            const UINT floatsPerPrimitive = primitiveSizeBytes / sizeof(float);

            std::vector<PrimitiveObjectType> inputPrimitives(totalPrimCount);
            for (UINT i = 0; i < totalPrimCount; i++)
            {
                float fi = (float)(i * floatsPerPrimitive);
                float f[floatsPerPrimitive];
                for (UINT j = 0; j < floatsPerPrimitive; j++)
                    f[j] = fi + j;

                memcpy(&inputPrimitives[i], f, floatsPerPrimitive);
            }

            ID3D12Device &d3d12Device = m_d3d12Context.GetDevice();
            LoadPrimitivesPass loadPrimitivesPass(&d3d12Device, 1);

            CComPtr<ID3D12GraphicsCommandList> pCommandList;
            m_d3d12Context.GetGraphicsCommandList(&pCommandList);

            CComPtr<ID3D12Resource> pInputPrimitiveBuffer;
            m_d3d12Context.CreateResourceWithInitialData(inputPrimitives.data(), inputPrimitives.size() * primitiveSizeBytes, &pInputPrimitiveBuffer);

            UINT cachedSortIndices[totalPrimCount];
            D3D12_GPU_VIRTUAL_ADDRESS cachedSortBuffer = 0;
            CComPtr<ID3D12Resource> pCachedSortBuffer;
            if (testCachedSort)
            {
                for (UINT i = 0; i < totalPrimCount; i++)
                {
                    cachedSortIndices[i] = (i + 1) % totalPrimCount;
                }
                m_d3d12Context.CreateResourceWithInitialData(cachedSortIndices, totalPrimCount * sizeof(UINT), &pCachedSortBuffer);
                cachedSortBuffer = pCachedSortBuffer->GetGPUVirtualAddress();
            }

            CComPtr<ID3D12Resource> pOutputBuffer, pOutputMetadataBuffer;

            D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

            auto outputBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(inputPrimitives.size() * sizeof(Primitive), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            AssertSucceeded(d3d12Device.CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &outputBufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&pOutputBuffer)));

            auto outputMetadataBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(inputPrimitives.size() * sizeof(PrimitiveMetaData), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            AssertSucceeded(d3d12Device.CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &outputMetadataBufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&pOutputMetadataBuffer)));

            D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC trianglesDesc = {};
            D3D12_RAYTRACING_GEOMETRY_AABBS_DESC aabbsDesc = {};
            switch (primitiveType) {
            case TRIANGLE_TYPE:
                trianglesDesc.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
                trianglesDesc.VertexCount = primitivesPerGeometry * 3;
                trianglesDesc.VertexBuffer.StartAddress = pInputPrimitiveBuffer->GetGPUVirtualAddress();
                trianglesDesc.VertexBuffer.StrideInBytes = sizeof(float3);
                trianglesDesc.IndexFormat = DXGI_FORMAT_UNKNOWN;
                trianglesDesc.IndexBuffer = 0;
                break;
            case PROCEDURAL_PRIMITIVE_TYPE:
                aabbsDesc.AABBCount = primitivesPerGeometry;
                aabbsDesc.AABBs.StartAddress = pInputPrimitiveBuffer->GetGPUVirtualAddress();
                aabbsDesc.AABBs.StrideInBytes = sizeof(AABB);
                break;
            }

            D3D12_RAYTRACING_GEOMETRY_DESC geometryDescs[numGeometryDescs];
            for (UINT i = 0; i < numGeometryDescs; i++)
            {
                D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
                geometryDesc.Flags = geometryFlags[i];
                switch (primitiveType) {
                case TRIANGLE_TYPE:
                    geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
                    geometryDesc.Triangles = trianglesDesc;
                    geometryDesc.Triangles.VertexBuffer.StartAddress += i * geomDescStartAddrStride;
                    break;
                case PROCEDURAL_PRIMITIVE_TYPE:
                    geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
                    geometryDesc.AABBs = aabbsDesc;
                    geometryDesc.AABBs.AABBs.StartAddress += i * geomDescStartAddrStride;
                    break;
                }

                geometryDescs[i] = geometryDesc;
            }

            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
            inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
            inputs.pGeometryDescs = geometryDescs;
            inputs.NumDescs = numGeometryDescs;
            inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

            loadPrimitivesPass.LoadPrimitives(
                pCommandList,
                inputs,
                totalPrimCount,
                pOutputBuffer->GetGPUVirtualAddress(),
                pOutputMetadataBuffer->GetGPUVirtualAddress(),
                cachedSortBuffer);

            pCommandList->Close();
            m_d3d12Context.ExecuteCommandList(pCommandList);

            std::vector<Primitive> outputPrimitives(totalPrimCount);
            m_d3d12Context.ReadbackResource(pOutputBuffer, outputPrimitives.data(), (UINT)outputPrimitives.size() * sizeof(*outputPrimitives.data()));

            std::vector<PrimitiveMetaData> outputMetadata(totalPrimCount);
            m_d3d12Context.ReadbackResource(pOutputMetadataBuffer, outputMetadata.data(), (UINT)outputMetadata.size() * sizeof(*outputMetadata.data()));

            for (UINT i = 0; i < totalPrimCount; i++)
            {
                const UINT sortedIndex = cachedSortBuffer ? cachedSortIndices[i] : i;
                const UINT geometryDescIndex = i / primitivesPerGeometry;
                const UINT targetGeomFlag = geometryFlags[geometryDescIndex];
                void *pUnwrappedPrimitive;

                switch (primitiveType) {
                case TRIANGLE_TYPE:
                    pUnwrappedPrimitive = &outputPrimitives[sortedIndex].triangle;
                    break;

                case PROCEDURAL_PRIMITIVE_TYPE:
                    pUnwrappedPrimitive = &outputPrimitives[sortedIndex].aabb;
                    break;
                }

                Assert::IsTrue(outputPrimitives[i].PrimitiveType == primitiveType, L"Loaded primitive is not properly marked with specified Primitive Type");
                Assert::IsTrue(memcmp(pUnwrappedPrimitive, &inputPrimitives[i], sizeof(inputPrimitives[i])) == 0, L"Loaded primitive does not match the input primitive");
                if (targetGeomFlag)
                    Assert::IsTrue(outputMetadata[sortedIndex].GeometryFlags == targetGeomFlag, L"Loaded primitive does not have correct geometry flag set");
                else
                    Assert::IsTrue(outputMetadata[sortedIndex].GeometryFlags == 0, L"Loaded primitive does not have geometry flags cleared");
            }
        }

        TEST_METHOD(RearrangingTriangles)
        {
            TestRearrangingTriangles(500, SceneType::Triangles);
        }

        TEST_METHOD(RearrangingAABBs)
        {
            TestRearrangingTriangles(500, SceneType::BottomLevelBVHs);
        }

        void VerifyDataIsReversed(SceneType sceneType, UINT numElements, void *pInputData, void *pOutputData, void *pInputMetadata, void *pOutputMetadata)
        {
            switch (sceneType)
            {
            case SceneType::Triangles:
            {
                Primitive *pInputTriangles = (Primitive *)pInputData;
                Primitive *pOutputTriangles = (Primitive *)pOutputData;
                PrimitiveMetaData* pInputPrimitiveMetaData = (PrimitiveMetaData*)pInputMetadata;
                PrimitiveMetaData* pOutputPrimitiveMetaData = (PrimitiveMetaData*)pOutputMetadata;

                for (UINT i = 0; i < numElements; i++)
                {
                    UINT inputIndex = numElements - (i + 1);
                    UINT outputIndex = i;
                    auto &inputTri = pInputTriangles[inputIndex];
                    Assert::IsTrue(
                        IsFloatArrayEqual((float*)&inputTri, (float *)&pOutputTriangles[outputIndex], 9),
                        L"Triangles in output buffers not correctly in reverse order");

                    Assert::IsTrue(
                        memcmp(&pInputPrimitiveMetaData[inputIndex], &pOutputPrimitiveMetaData[outputIndex], sizeof(*pInputPrimitiveMetaData)) == 0,
                        L"Metadata in output buffers not correctly in reverse order");
                }
                break;
            }
            case SceneType::BottomLevelBVHs:
            {
                AABBNode *pInputAABBs = (AABBNode *)pInputData;
                AABBNode *pOutputAABBs = (AABBNode *)pOutputData;
                BVHMetadata* pInputAABBMetadata = (BVHMetadata*)pInputMetadata;
                BVHMetadata* pOutputAABBMetadata = (BVHMetadata*)pOutputMetadata;
                for (UINT i = 0; i < numElements; i++)
                {
                    UINT inputIndex = numElements - (i + 1);
                    UINT outputIndex = i;

                    Assert::IsTrue(
                        memcmp(&pInputAABBs[inputIndex], &pOutputAABBs[outputIndex], sizeof(AABBNode)) == 0,
                        L"AABBs in output buffers not correctly in reverse order");

                    Assert::IsTrue(
                        memcmp(&pInputAABBMetadata[inputIndex], &pOutputAABBMetadata[outputIndex], sizeof(*pInputAABBMetadata)) == 0,
                        L"Metadata in output buffers not correctly in reverse order");
                }
                break;
            }
            }
        }

        void TestRearrangingTriangles(UINT numElements, SceneType sceneType)
        {
            auto &d3d12Device = m_d3d12Context.GetDevice();
            RearrangeElementsPass rearrangeTrianglePass(&d3d12Device, 0);

            AABB unused;
            std::vector<BYTE> outputData;
            std::vector<BYTE> outputMetadata;
            GenerateSceneData(numElements, sceneType, outputData, unused, nullptr, &outputMetadata);

            std::vector<uint> indexBuffer(numElements);
            for (UINT i = 0; i < numElements; i++)
            {
                indexBuffer[i] = numElements - i - 1;
            }

            CComPtr<ID3D12Resource> pInputBuffer;
            m_d3d12Context.CreateResourceWithInitialData(outputData.data(), outputData.size(), &pInputBuffer);

            CComPtr<ID3D12Resource> pMetadataBuffer;
            m_d3d12Context.CreateResourceWithInitialData(outputMetadata.data(), outputMetadata.size(), &pMetadataBuffer);

            CComPtr<ID3D12Resource> pIndexBuffer;
            m_d3d12Context.CreateResourceWithInitialData(indexBuffer.data(), indexBuffer.size() * sizeof(UINT), &pIndexBuffer);

            auto outputBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(outputData.size(), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            CComPtr<ID3D12Resource> pOutputBuffer;
            AssertSucceeded(d3d12Device.CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &outputBufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&pOutputBuffer)));

            CComPtr<ID3D12Resource> pOutputMetadata;
            auto outputMetadataBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(outputMetadata.size(), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            AssertSucceeded(d3d12Device.CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &outputMetadataBufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&pOutputMetadata)));

            CComPtr<ID3D12GraphicsCommandList> pCommandList;
            m_d3d12Context.GetGraphicsCommandList(&pCommandList);
            rearrangeTrianglePass.Rearrange(
                pCommandList,
                sceneType,
                numElements,
                pInputBuffer->GetGPUVirtualAddress(),
                pMetadataBuffer->GetGPUVirtualAddress(),
                pIndexBuffer->GetGPUVirtualAddress(),
                pOutputBuffer->GetGPUVirtualAddress(),
                pOutputMetadata->GetGPUVirtualAddress(), 0);

            D3D12_RESOURCE_BARRIER uavToCopySourceBarrier[] =
            {
                CD3DX12_RESOURCE_BARRIER::Transition(pOutputBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE),
                CD3DX12_RESOURCE_BARRIER::Transition(pOutputMetadata, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE)
            };
            pCommandList->ResourceBarrier(ARRAYSIZE(uavToCopySourceBarrier), uavToCopySourceBarrier);
            pCommandList->Close();

            m_d3d12Context.ExecuteCommandList(pCommandList);

            std::vector<BYTE> gpuOutputData(outputData.size());
            m_d3d12Context.ReadbackResource(pOutputBuffer, gpuOutputData.data(), (UINT)gpuOutputData.size());

            std::vector<BYTE> gpuMetadata(outputMetadata.size());
            m_d3d12Context.ReadbackResource(pOutputMetadata, gpuMetadata.data(), (UINT)gpuMetadata.size());

            VerifyDataIsReversed(sceneType, numElements, outputData.data(), gpuOutputData.data(), outputMetadata.data(), gpuMetadata.data());
        }

    private:
        D3D12Context m_d3d12Context;
    };

    TEST_CLASS(TracingTests)
    {
        const UINT cOutputBufferWidth = 6;
        const UINT cOutputBufferHeight = 4;
        const DXGI_FORMAT cOutputBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

        struct RayGenViewport
        {
            float Left;
            float Top;
            float Right;
            float Bottom;
            unsigned DispatchDimensionWidth;
            unsigned DispatchDimensionHeight;
            unsigned RayFlags;
            unsigned InstanceInclusionMask;
        };
        RayGenViewport viewport = { -1.0, -1.0, 1.0, 1.0, cOutputBufferWidth, cOutputBufferHeight, 0 };
        const float IdentityMatrix[12] = { 1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0 };

        enum RootSignatureParams
        {
            AccelerationStructureSlot = 0,
            OutputViewSlot,
            ViewportConstantSlot,
            NumParameters
        };

        enum VerifyType
        {
            HITS_ON_LEFT_HALF_OF_SCREEN,
            HITS_ON_RIGHT_HALF_OF_SCREEN,
            HITS_ON_TOP_HALF_OF_SCREEN,
            HITS_ON_BOTTOM_HALF_OF_SCREEN,
            ALL_HITS,
            ALL_MISS,
        };

        enum GeometryType
        {
            FULL_SCREEN_QUAD,
            LEFT_HALF_SCREEN_QUAD,
            RIGHT_HALF_SCREEN_QUAD
        };

        enum WindingType
        {
            Clockwise,
            CounterClockwise
        };

        CComPtr<ID3D12Device> m_pD3D12Device;
        CComPtr<ID3D12CommandQueue> m_pCommandQueue;

        CComPtr<ID3D12RootSignature> m_pRootSignature;
        CComPtr<ID3D12RaytracingFallbackDevice> m_pRaytracingDevice;
        CComPtr<ID3D12RaytracingFallbackStateObject> m_pRaytracingStateObject;

        CComPtr<ID3D12Resource> m_pRaytracingOutputResource;
        D3D12_GPU_DESCRIPTOR_HANDLE m_UAVGpuDescriptor;

        CComPtr<ID3D12Resource> m_pTopLevelAccelerationStructure;
        WRAPPED_GPU_POINTER m_TopLevelAccelerationStructurePointer;

        CComPtr<ID3D12Resource> m_pMissShaderTable, m_pHitShaderTable, m_pRayGenShaderTable;

        void BuildSimpleStateObject()
        {
            auto &d3d12device = m_d3d12Context.GetDevice();
            LPCWSTR ClosestHitExportName = L"Hit";
            LPCWSTR MissExportName = L"Miss";
            LPCWSTR RayGenExportName = L"RayGen";
            LPCWSTR HitGroupExportName = L"HitGroup";

            std::vector<D3D12_STATE_SUBOBJECT> subObjects;

            D3D12_EXPORT_DESC exports[] = {
                { ClosestHitExportName, nullptr, D3D12_EXPORT_FLAG_NONE },
                { RayGenExportName, nullptr, D3D12_EXPORT_FLAG_NONE },
                { MissExportName, nullptr, D3D12_EXPORT_FLAG_NONE },
            };

            D3D12_DXIL_LIBRARY_DESC libraryDesc = {};
            libraryDesc.DXILLibrary = CD3DX12_SHADER_BYTECODE((void *)g_pSimpleRayTracing, ARRAYSIZE(g_pSimpleRayTracing));
            libraryDesc.NumExports = ARRAYSIZE(exports);
            libraryDesc.pExports = exports;
            D3D12_STATE_SUBOBJECT DxilLibrarySubObject = {};
            DxilLibrarySubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
            DxilLibrarySubObject.pDesc = &libraryDesc;
            subObjects.push_back(DxilLibrarySubObject);

            D3D12_STATE_SUBOBJECT hitGroupSubObject;
            D3D12_HIT_GROUP_DESC hitGroupDesc = {};
            hitGroupDesc.ClosestHitShaderImport = ClosestHitExportName;
            hitGroupDesc.HitGroupExport = HitGroupExportName;
            hitGroupSubObject.pDesc = &hitGroupDesc;
            hitGroupSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
            subObjects.push_back(hitGroupSubObject);

            D3D12_STATE_SUBOBJECT nodeMaskSubObject;
            UINT nodeMask = 1;
            nodeMaskSubObject.pDesc = &nodeMask;
            nodeMaskSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK;
            subObjects.push_back(nodeMaskSubObject);

            ID3D12RootSignature *pGlobalRootSignature = m_pRootSignature;
            D3D12_STATE_SUBOBJECT rootSignatureSubObject;
            rootSignatureSubObject.pDesc = &pGlobalRootSignature;
            rootSignatureSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
            subObjects.push_back(rootSignatureSubObject);

            D3D12_STATE_SUBOBJECT shaderConfigSubObject;
            D3D12_RAYTRACING_SHADER_CONFIG shaderConfig;
            shaderConfig.MaxAttributeSizeInBytes = shaderConfig.MaxPayloadSizeInBytes = 8;
            shaderConfigSubObject.pDesc = &shaderConfig;
            shaderConfigSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
            subObjects.push_back(shaderConfigSubObject);

            D3D12_STATE_SUBOBJECT pipelineConfigSubObject;
            D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig;
            pipelineConfig.MaxTraceRecursionDepth = 2;
            pipelineConfigSubObject.pDesc = &pipelineConfig;
            pipelineConfigSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
            subObjects.push_back(pipelineConfigSubObject);

            D3D12_STATE_OBJECT_DESC stateObject;
            stateObject.NumSubobjects = (UINT)subObjects.size();
            stateObject.pSubobjects = subObjects.data();
            stateObject.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

            ThrowFailure(m_pRaytracingDevice->CreateStateObject(&stateObject, IID_PPV_ARGS(&m_pRaytracingStateObject)));

            // Create the Shader Tables
            UINT shaderIdentifierSize = m_pRaytracingDevice->GetShaderIdentifierSize();
            void *pMissShaderIdentifier = m_pRaytracingStateObject->GetShaderIdentifier(MissExportName);
            void *pHitShaderIdentifier = m_pRaytracingStateObject->GetShaderIdentifier(HitGroupExportName);
            void *pRayGenShaderIdentifier = m_pRaytracingStateObject->GetShaderIdentifier(RayGenExportName);

            m_d3d12Context.CreateResourceWithInitialData(pMissShaderIdentifier, shaderIdentifierSize, &m_pMissShaderTable);
            m_d3d12Context.CreateResourceWithInitialData(pHitShaderIdentifier, shaderIdentifierSize, &m_pHitShaderTable);
            m_d3d12Context.CreateResourceWithInitialData(pRayGenShaderIdentifier, shaderIdentifierSize, &m_pRayGenShaderTable);
        }

        void TraceRay(UINT rayFlags = 0, UINT InstanceInclusionMask = 0xff)
        {
            auto &d3d12device = m_d3d12Context.GetDevice();

            CComPtr<ID3D12GraphicsCommandList> pCommandList;
            m_d3d12Context.GetGraphicsCommandList(&pCommandList);

            CComPtr<ID3D12RaytracingFallbackCommandList> pRaytracingCommandList;
            m_pRaytracingDevice->QueryRaytracingCommandList(pCommandList, IID_PPV_ARGS(&pRaytracingCommandList));

            ID3D12DescriptorHeap *pDescriptorHeaps[] = { &m_pDescriptorHeapStack->GetDescriptorHeap() };
            pRaytracingCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);

            pCommandList->SetComputeRootSignature(m_pRootSignature);
            viewport.RayFlags = rayFlags;
            viewport.InstanceInclusionMask = InstanceInclusionMask;
            pCommandList->SetComputeRoot32BitConstants(ViewportConstantSlot, SizeOfInUint32(viewport), &viewport, 0);
            pCommandList->SetComputeRootDescriptorTable(OutputViewSlot, m_UAVGpuDescriptor);
            pRaytracingCommandList->SetTopLevelAccelerationStructure(AccelerationStructureSlot, m_TopLevelAccelerationStructurePointer);

            D3D12_DISPATCH_RAYS_DESC dispatchRaysDesc = {};
            dispatchRaysDesc.Width = cOutputBufferWidth;
            dispatchRaysDesc.Height = cOutputBufferHeight;
            dispatchRaysDesc.Depth = 1;
            dispatchRaysDesc.HitGroupTable.StartAddress = m_pHitShaderTable->GetGPUVirtualAddress();
            dispatchRaysDesc.HitGroupTable.SizeInBytes = m_pHitShaderTable->GetDesc().Width;
            dispatchRaysDesc.HitGroupTable.StrideInBytes = dispatchRaysDesc.HitGroupTable.SizeInBytes;
            dispatchRaysDesc.MissShaderTable.StartAddress = m_pMissShaderTable->GetGPUVirtualAddress();
            dispatchRaysDesc.MissShaderTable.SizeInBytes = m_pMissShaderTable->GetDesc().Width;
            dispatchRaysDesc.MissShaderTable.StrideInBytes = dispatchRaysDesc.MissShaderTable.SizeInBytes;
            dispatchRaysDesc.RayGenerationShaderRecord.StartAddress = m_pRayGenShaderTable->GetGPUVirtualAddress();
            dispatchRaysDesc.RayGenerationShaderRecord.SizeInBytes = m_pRayGenShaderTable->GetDesc().Width;

            pRaytracingCommandList->SetPipelineState1(m_pRaytracingStateObject);
            pRaytracingCommandList->DispatchRays(&dispatchRaysDesc);

            pCommandList->Close();
            m_d3d12Context.ExecuteCommandList(pCommandList);

            m_d3d12Context.WaitForGpuWork();
        }

        BYTE hitColor[4] = { 255, 0, 255, 255 };
        BYTE missColor[4] = { 255, 0, 0, 255 };
        void VerifyOutput(std::vector<bool> isHitExpected)
        {
            UINT numPartitions = (UINT)isHitExpected.size();
            std::vector<byte> outputData;
            UINT64 pitch;
            m_d3d12Context.ReadbackTexture(m_pRaytracingOutputResource, outputData, pitch);

            auto &desc = m_pRaytracingOutputResource->GetDesc();
            assert(desc.Width % isHitExpected.size() == 0);
            UINT64 pixelsPerPartition = desc.Width / isHitExpected.size();

            for (UINT y = 0; y < desc.Height; y++)
            {
                BYTE *pRowData = outputData.data() + pitch * y;
                for (UINT x = 0; x < desc.Width; x++)
                {
                    UINT64 partitionIndex = x / pixelsPerPartition;
                    BYTE *expectedColor = isHitExpected[partitionIndex] ? hitColor : missColor;

                    BYTE *pixel = &pRowData[x * 4];
                    Assert::IsTrue(memcmp(expectedColor, pixel, sizeof(BYTE) * 4) == 0,
                        L"Color not matching expected color");
                }
            }
        }

        void VerifyOutput(VerifyType verifyType)
        {
            BYTE hitColor[4] = { 255, 0, 255, 255 };
            BYTE missColor[4] = { 255, 0, 0, 255 };

            std::vector<byte> outputData;
            UINT64 pitch;
            m_d3d12Context.ReadbackTexture(m_pRaytracingOutputResource, outputData, pitch);

            auto &desc = m_pRaytracingOutputResource->GetDesc();
            for (UINT y = 0; y < desc.Height; y++)
            {
                BYTE *pRowData = outputData.data() + pitch * y;
                for (UINT x = 0; x < desc.Width; x++)
                {
                    BYTE *expectedColor;
                    bool isLeftHalf = (x < desc.Width / 2);
                    bool isTopHalf = (y < desc.Height / 2);
                    switch (verifyType)
                    {
                    case HITS_ON_LEFT_HALF_OF_SCREEN:
                        expectedColor = isLeftHalf ? hitColor : missColor;
                        break;
                    case HITS_ON_RIGHT_HALF_OF_SCREEN:
                        expectedColor = isLeftHalf ? missColor : hitColor;
                        break;
                    case HITS_ON_TOP_HALF_OF_SCREEN:
                        expectedColor = isTopHalf ? hitColor : missColor;
                        break;
                    case HITS_ON_BOTTOM_HALF_OF_SCREEN:
                        expectedColor = isTopHalf ? missColor : hitColor;
                        break;
                    case ALL_HITS:
                        expectedColor = hitColor;
                    case ALL_MISS:
                        expectedColor = missColor;
                        break;
                    }

                    // Hit shader should spew purple on the left and miss show spew red on the right
                    BYTE *pixel = &pRowData[x * 4];
                    Assert::IsTrue(memcmp(expectedColor, pixel, sizeof(BYTE) * 4) == 0,
                        L"Color not matching expected color");
                }
            }
        }

        void BuildBottomLevelAccelerationStructure(GeometryType geometryType, WindingType windingType, std::vector<CComPtr<ID3D12Resource>> &pBottomLevelAccStructs)
        {
            auto &d3d12device = m_d3d12Context.GetDevice();

            pBottomLevelAccStructs.push_back(nullptr);
            UINT16 indicies[] = {
                0, 1, 2,
                2, 1, 3
            };
            if (windingType == CounterClockwise)
            {
                std::swap(indicies[1], indicies[2]);
                std::swap(indicies[4], indicies[5]);
            }

            float geometryRightmostXValue = viewport.Right;
            float geometryLeftmostXValue = viewport.Left;
            switch (geometryType)
            {
            case FULL_SCREEN_QUAD:
                // do nothing
                break;
            case LEFT_HALF_SCREEN_QUAD:
                geometryRightmostXValue = (float)(viewport.Left + viewport.Right) / 2.0f;
                break;
            case RIGHT_HALF_SCREEN_QUAD:
                geometryLeftmostXValue = (float)(viewport.Left + viewport.Right) / 2.0f;
                break;
            }

            // Create a plane that takes up the left half of the screen
            const float depthValue = 1.0;
            float triangleVerts[] = {
                geometryLeftmostXValue,    viewport.Top,    depthValue,
                geometryLeftmostXValue,    viewport.Bottom, depthValue,
                geometryRightmostXValue,   viewport.Top,    depthValue,
                geometryRightmostXValue,   viewport.Bottom, depthValue,
            };


            const auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            CComPtr<ID3D12Resource> pVertexBuffer;
            m_d3d12Context.CreateResourceWithInitialData(triangleVerts, sizeof(triangleVerts), &pVertexBuffer);

            CComPtr<ID3D12Resource> pIndexBuffer;
            m_d3d12Context.CreateResourceWithInitialData(indicies, sizeof(indicies), &pIndexBuffer);

            D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
            geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
            geometryDesc.Triangles.IndexBuffer = pIndexBuffer->GetGPUVirtualAddress();
            geometryDesc.Triangles.IndexCount = ARRAYSIZE(indicies);
            geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
            geometryDesc.Triangles.Transform3x4 = 0;
            geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
            geometryDesc.Triangles.VertexCount = ARRAYSIZE(triangleVerts) / 3;
            geometryDesc.Triangles.VertexBuffer.StartAddress = pVertexBuffer->GetGPUVirtualAddress();
            geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(float) * 3;

            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &inputs = buildDesc.Inputs;
            inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
            inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
            inputs.NumDescs = 1;
            inputs.pGeometryDescs = &geometryDesc;
            inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo;
            m_pRaytracingDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &bottomLevelPrebuildInfo);

            CComPtr<ID3D12Resource> pScratchResource;
            AllocateUAVBuffer(d3d12device, bottomLevelPrebuildInfo.ScratchDataSizeInBytes, &pScratchResource);
            AllocateUAVBuffer(d3d12device, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &pBottomLevelAccStructs.back());

            CComPtr<ID3D12GraphicsCommandList> pCommandList;
            m_d3d12Context.GetGraphicsCommandList(&pCommandList);
            CComPtr<ID3D12RaytracingFallbackCommandList> pRaytracingCommandList;
            m_pRaytracingDevice->QueryRaytracingCommandList(pCommandList, IID_PPV_ARGS(&pRaytracingCommandList));

            ID3D12DescriptorHeap *pDescriptorHeaps[] = { &m_pDescriptorHeapStack->GetDescriptorHeap() };
            pRaytracingCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);

            buildDesc.DestAccelerationStructureData = pBottomLevelAccStructs.back()->GetGPUVirtualAddress();
            buildDesc.ScratchAccelerationStructureData = pScratchResource->GetGPUVirtualAddress();

            pRaytracingCommandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

            auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
            pCommandList->ResourceBarrier(1, &uavBarrier);

            pCommandList->Close();
            m_d3d12Context.ExecuteCommandList(pCommandList);
            m_d3d12Context.WaitForGpuWork();
        }

        void BuildTopLevelAccelerationStructure(
            std::vector<CComPtr<ID3D12Resource>> &pBottomLevelAccStructs,
            std::vector<const float *> &pTransforms,
            std::vector<UINT> &instanceFlags = std::vector<UINT>(),
            std::vector<UINT> &instanceMask = std::vector<UINT>())
        {
            auto &d3d12device = m_d3d12Context.GetDevice();

            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &inputs = buildDesc.Inputs;
            inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
            inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
            inputs.NumDescs = (UINT)pBottomLevelAccStructs.size();
            inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
            inputs.pGeometryDescs = nullptr;
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo;
            m_pRaytracingDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &topLevelPrebuildInfo);

            CComPtr<ID3D12Resource> pScratchResource;
            AllocateUAVBuffer(d3d12device, topLevelPrebuildInfo.ScratchDataSizeInBytes, &pScratchResource);
            AllocateUAVBuffer(d3d12device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_pTopLevelAccelerationStructure);

            std::vector<WRAPPED_GPU_POINTER> bottomLevelPointers;
            for (auto pResource : pBottomLevelAccStructs)
            {
                UINT bottomLevelDescriptorHeapIndex = m_pDescriptorHeapStack->AllocateBufferUav(*pResource);
                bottomLevelPointers.push_back(m_pRaytracingDevice->GetWrappedPointerSimple(bottomLevelDescriptorHeapIndex, pResource->GetGPUVirtualAddress()));
            }

            UINT topLevelDescriptorIndex = m_pDescriptorHeapStack->AllocateBufferUav(*m_pTopLevelAccelerationStructure);
            m_TopLevelAccelerationStructurePointer = m_pRaytracingDevice->GetWrappedPointerSimple(topLevelDescriptorIndex, m_pTopLevelAccelerationStructure->GetGPUVirtualAddress());

            std::vector<D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC> instanceDescs(bottomLevelPointers.size());
            for (uint i = 0; i < bottomLevelPointers.size(); i++)
            {
                auto &desc = instanceDescs[i];
                ZeroMemory(&desc, sizeof(desc));
                desc.AccelerationStructure = bottomLevelPointers[i];
                if (instanceFlags.size())
                {
                    desc.Flags = instanceFlags[i];
                }

                desc.InstanceMask = instanceMask.size() ? instanceMask[i] : 0xff;
                memcpy(desc.Transform, pTransforms[i], sizeof(desc.Transform));
            }
            CComPtr<ID3D12Resource> pInstanceDescs;
            m_d3d12Context.CreateResourceWithInitialData(instanceDescs.data(), instanceDescs.size() * sizeof(D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC), &pInstanceDescs);

            CComPtr<ID3D12GraphicsCommandList> pCommandList;
            m_d3d12Context.GetGraphicsCommandList(&pCommandList);
            CComPtr<ID3D12RaytracingFallbackCommandList> pRaytracingCommandList;
            m_pRaytracingDevice->QueryRaytracingCommandList(pCommandList, IID_PPV_ARGS(&pRaytracingCommandList));

            ID3D12DescriptorHeap *pDescriptorHeaps[] = { &m_pDescriptorHeapStack->GetDescriptorHeap() };
            pRaytracingCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);

            buildDesc.ScratchAccelerationStructureData = pScratchResource->GetGPUVirtualAddress();
            buildDesc.DestAccelerationStructureData = m_pTopLevelAccelerationStructure->GetGPUVirtualAddress();
            inputs.InstanceDescs = pInstanceDescs->GetGPUVirtualAddress();

            pRaytracingCommandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

            pCommandList->Close();
            m_d3d12Context.ExecuteCommandList(pCommandList);

            m_d3d12Context.WaitForGpuWork();
        }

        void BuildEmptyTopLevelAccelerationStructure()
        {
            auto &d3d12device = m_d3d12Context.GetDevice();

            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &inputs = buildDesc.Inputs;
            D3D12_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_DESC getPrebuildInfoDesc = {};
            inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
            inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
            inputs.NumDescs = 0;
            inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
            inputs.pGeometryDescs = nullptr;
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo;
            m_pRaytracingDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &topLevelPrebuildInfo);

            CComPtr<ID3D12Resource> pScratchResource;
            AllocateUAVBuffer(d3d12device, topLevelPrebuildInfo.ScratchDataSizeInBytes, &pScratchResource);
            AllocateUAVBuffer(d3d12device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_pTopLevelAccelerationStructure);

            UINT topLevelDescriptorIndex = m_pDescriptorHeapStack->AllocateBufferUav(*m_pTopLevelAccelerationStructure);
            m_TopLevelAccelerationStructurePointer = m_pRaytracingDevice->GetWrappedPointerSimple(topLevelDescriptorIndex, m_pTopLevelAccelerationStructure->GetGPUVirtualAddress());

            CComPtr<ID3D12GraphicsCommandList> pCommandList;
            m_d3d12Context.GetGraphicsCommandList(&pCommandList);
            CComPtr<ID3D12RaytracingFallbackCommandList> pRaytracingCommandList;
            m_pRaytracingDevice->QueryRaytracingCommandList(pCommandList, IID_PPV_ARGS(&pRaytracingCommandList));

            ID3D12DescriptorHeap *pDescriptorHeaps[] = { &m_pDescriptorHeapStack->GetDescriptorHeap() };
            pRaytracingCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);

            buildDesc.DestAccelerationStructureData = m_pTopLevelAccelerationStructure->GetGPUVirtualAddress();
            buildDesc.ScratchAccelerationStructureData = pScratchResource ? pScratchResource->GetGPUVirtualAddress() : 0;
            inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

            pRaytracingCommandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

            pCommandList->Close();
            m_d3d12Context.ExecuteCommandList(pCommandList);

            m_d3d12Context.WaitForGpuWork();
        }

        TEST_METHOD_INITIALIZE(MethodSetup)
        {
            auto &d3d12device = m_d3d12Context.GetDevice();
            D3D12CreateRaytracingFallbackDevice(
                &d3d12device,
                CreateRaytracingFallbackDeviceFlags::ForceComputeFallback,
                0,
                IID_PPV_ARGS(&m_pRaytracingDevice));

            CD3DX12_DESCRIPTOR_RANGE UAVDescriptor;
            UAVDescriptor.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
            CD3DX12_ROOT_PARAMETER rootParameters[NumParameters];
            rootParameters[OutputViewSlot].InitAsDescriptorTable(1, &UAVDescriptor);
            rootParameters[AccelerationStructureSlot].InitAsShaderResourceView(0);
            rootParameters[ViewportConstantSlot].InitAsConstants(SizeOfInUint32(RayGenViewport), 0, 0);

            CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);

            CComPtr<ID3DBlob> pRootSignatureBlob;
            m_pRaytracingDevice->D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pRootSignatureBlob, nullptr);
            m_pRaytracingDevice->CreateRootSignature(1, pRootSignatureBlob->GetBufferPointer(), pRootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature));

            auto descriptorHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            m_pDescriptorHeapStack = std::unique_ptr<DescriptorHeapStack>(
                new DescriptorHeapStack(m_d3d12Context.GetDevice(), 100, descriptorHeapType, 0));
            auto descriptorSize = d3d12device.GetDescriptorHandleIncrementSize(descriptorHeapType);

            auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            auto uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(cOutputBufferFormat, cOutputBufferWidth, cOutputBufferHeight, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            d3d12device.CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_pRaytracingOutputResource));

            D3D12_CPU_DESCRIPTOR_HANDLE uavDescriptorHandle;
            UINT uavDescriptorHeapIndex;
            m_pDescriptorHeapStack->AllocateDescriptor(uavDescriptorHandle, uavDescriptorHeapIndex);
            d3d12device.CreateUnorderedAccessView(m_pRaytracingOutputResource, nullptr, nullptr, uavDescriptorHandle);
            m_UAVGpuDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_pDescriptorHeapStack->GetDescriptorHeap().GetGPUDescriptorHandleForHeapStart(), uavDescriptorHeapIndex, descriptorSize);
        }

        void TestLeftScreenFillingSingleBottomLevel(const float *transform, VerifyType verifyType)
        {
            std::vector<CComPtr<ID3D12Resource>> bottomLevelResources;
            BuildSimpleStateObject();

            std::vector<const float *> transformations;
            BuildBottomLevelAccelerationStructure(LEFT_HALF_SCREEN_QUAD, Clockwise, bottomLevelResources);
            for (UINT i = 0; i < bottomLevelResources.size(); i++)
            {
                transformations.push_back(transform);
            }

            BuildTopLevelAccelerationStructure(bottomLevelResources, transformations);
            TraceRay();
            VerifyOutput(verifyType);
        }

        TEST_METHOD(BasicTrace)
        {
            TestLeftScreenFillingSingleBottomLevel(IdentityMatrix, HITS_ON_LEFT_HALF_OF_SCREEN);
        }

        TEST_METHOD(BasicTraceWithInstanceFlip)
        {
            float flipAcrossYAxisTransform[12] = {
                -1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0
            };
            TestLeftScreenFillingSingleBottomLevel(flipAcrossYAxisTransform, HITS_ON_RIGHT_HALF_OF_SCREEN);
        }

        TEST_METHOD(BasicTraceWithInstance90DegreeRotation)
        {
            const float PI = 3.14f;
            const float angleInRadians = PI / 2.0;
            float rotate90Degrees[12] = {
                cos(angleInRadians), sin(angleInRadians), 0, 0,
                -sin(angleInRadians), cos(angleInRadians), 0, 0,
                0, 0, 1, 0
            };
            TestLeftScreenFillingSingleBottomLevel(rotate90Degrees, HITS_ON_BOTTOM_HALF_OF_SCREEN);
        }

        TEST_METHOD(BasicTraceWithInstanceTranslation)
        {
            const float PI = 3.14f;
            const float angleInRadians = PI / 2.0;
            float translateToRightHalf[12] = {
                1, 0, 0, 1,
                0, 1, 0, 0,
                0, 0, 1, 0
            };
            TestLeftScreenFillingSingleBottomLevel(translateToRightHalf, HITS_ON_RIGHT_HALF_OF_SCREEN);
        }

        enum RAY_FLAG
        {
            RAY_FLAG_NONE = 0x00,
            RAY_FLAG_FORCE_OPAQUE = 0x01,
            RAY_FLAG_FORCE_NON_OPAQUE = 0x02,
            RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH = 0x04,
            RAY_FLAG_SKIP_CLOSEST_HIT_SHADER = 0x08,
            RAY_FLAG_CULL_BACK_FACING_TRIANGLES = 0x10,
            RAY_FLAG_CULL_FRONT_FACING_TRIANGLES = 0x20,
            RAY_FLAG_CULL_OPAQUE = 0x40,
            RAY_FLAG_CULL_NON_OPAQUE = 0x80,
        };

        void TransformFromFullScreenToScreenPartition(UINT partitionIndex, UINT totalPartitions, _Out_ float *pTransform)
        {
            const float screenWidth = viewport.Right - viewport.Left;
            float xScale = (1.0f / (float)totalPartitions);
            float partitionWidth = xScale * screenWidth;
            float xOffset = partitionWidth * partitionIndex + partitionWidth / 2.0f;

            float transform[] =
            {
                xScale, 0, 0, xOffset + viewport.Left,
                0,      1, 0, 0,
                0,      0, 1, 0
            };

            memcpy(pTransform, transform, sizeof(transform));
        }

        void TestCulling(RAY_FLAG cullFlag)
        {
            BuildSimpleStateObject();
            const UINT numTests = 6;
            std::vector<bool> hitExpected(numTests);
            std::vector<UINT> instanceFlags(numTests);
            std::vector<CComPtr<ID3D12Resource>> bottomLevelResources;
            float transformations[numTests * FloatsPerMatrix];
            std::vector<const float*> transformationsList;

            for (UINT i = 0; i < numTests; i++)
            {
                float *pTransform = transformations + FloatsPerMatrix * i;
                TransformFromFullScreenToScreenPartition(i, numTests, pTransform);
                transformationsList.push_back(pTransform);

                WindingType windingType = (i < numTests / 2) ? CounterClockwise : Clockwise;
                D3D12_RAYTRACING_INSTANCE_FLAGS instanceFlag;
                switch (i % 3)
                {
                case 0:
                    instanceFlag = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
                    break;
                case 1:
                    instanceFlag = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE;
                    break;
                case 2:
                    instanceFlag = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE;
                    break;
                }
                BuildBottomLevelAccelerationStructure(FULL_SCREEN_QUAD, windingType, bottomLevelResources);
                instanceFlags[i] = instanceFlag;

                bool isHitExpected = false;
                bool isFrontFacing = (windingType == Clockwise && (instanceFlag & D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE) == 0) ||
                    (windingType == CounterClockwise && (instanceFlag & D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE));
                if (instanceFlag == D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE || cullFlag == RAY_FLAG_NONE)
                {
                    isHitExpected = true;
                }
                else if (isFrontFacing)
                {
                    isHitExpected = (cullFlag & RAY_FLAG_CULL_FRONT_FACING_TRIANGLES) == 0;
                }
                else //isBackFacing
                {
                    isHitExpected = (cullFlag & RAY_FLAG_CULL_BACK_FACING_TRIANGLES) == 0;
                }
                hitExpected[i] = (isHitExpected);
            }

            BuildTopLevelAccelerationStructure(bottomLevelResources, transformationsList, instanceFlags);
            TraceRay(cullFlag);

            VerifyOutput(hitExpected);
        }

        TEST_METHOD(TraceFrontFaceCulling)
        {
            TestCulling(RAY_FLAG_CULL_FRONT_FACING_TRIANGLES);
        }

        TEST_METHOD(TraceEmptyAccelerationStructure)
        {
            std::vector<CComPtr<ID3D12Resource>> bottomLevelResources;
            BuildSimpleStateObject();

            BuildEmptyTopLevelAccelerationStructure();
            TraceRay();
            VerifyOutput(ALL_MISS);
        }

        TEST_METHOD(TraceBackFaceCulling)
        {
            TestCulling(RAY_FLAG_CULL_BACK_FACING_TRIANGLES);
        }

        TEST_METHOD(TraceNoCulling)
        {
            TestCulling(RAY_FLAG_NONE);
        }

        TEST_METHOD(TraceInstanceMasks)
        {
            BuildSimpleStateObject();
            const UINT numTests = 6;
            std::vector<bool> hitExpected(numTests);
            std::vector<UINT> instanceFlags(numTests);
            std::vector<UINT> instanceMasks(numTests);
            std::vector<CComPtr<ID3D12Resource>> bottomLevelResources;
            float transformations[numTests * FloatsPerMatrix];
            std::vector<const float*> transformationsList;

            UINT traceMask = 0x23;
            for (UINT i = 0; i < numTests; i++)
            {
                if (i == 0)
                {
                    BuildBottomLevelAccelerationStructure(FULL_SCREEN_QUAD, Clockwise, bottomLevelResources);
                }
                else
                {
                    // Just duplicate the first resource
                    bottomLevelResources.push_back(bottomLevelResources[0]);
                }

                float *pTransform = transformations + FloatsPerMatrix * i;
                TransformFromFullScreenToScreenPartition(i, numTests, pTransform);
                transformationsList.push_back(pTransform);
                UINT instanceMask = 1 << i;

                instanceFlags[i] = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
                instanceMasks[i] = instanceMask;
                hitExpected[i] = instanceMask & traceMask;
            }

            BuildTopLevelAccelerationStructure(bottomLevelResources, transformationsList, instanceFlags, instanceMasks);
            TraceRay(RAY_FLAG_NONE, traceMask);

            VerifyOutput(hitExpected);
        }

        // Force hardware due to WARP bugs when dealing with the uber shader
        D3D12Context m_d3d12Context = D3D12Context(D3D12Context::CreationFlags::ForceHardware);
        std::unique_ptr<DescriptorHeapStack> m_pDescriptorHeapStack;
    };
}
