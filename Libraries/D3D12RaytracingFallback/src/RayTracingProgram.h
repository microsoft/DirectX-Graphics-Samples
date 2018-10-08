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

namespace FallbackLayer
{
    // StateIdentifiers are a more fine-grain identifier that corresponds
    // to exactly 1 state. This is distinct from ShaderIdentifiers which
    // can contain multiple states. The ShaderIdentifier for a HitGroup 
    // for example contains 3 StateIdentifiers: ClosestHit, AnyHit, and 
    // Intersection
    typedef UINT32 StateIdentifier;


    struct ShaderIdentifier
    {
        StateIdentifier StateId;
        StateIdentifier AnyHitId;
        StateIdentifier IntersectionShaderId;

        UINT32 padding[5];
    };
    static_assert(sizeof(ShaderIdentifier) == D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, L"Shader Identifier size must match up with size in D3D12 header");

    class IRaytracingProgram
    {
    public:
        virtual ~IRaytracingProgram() {}

        virtual void DispatchRays(
            ID3D12GraphicsCommandList *pCommandList, 
            ID3D12DescriptorHeap *pSrvCbvUavDescriptorHeap,
            ID3D12DescriptorHeap *pSamplerDescriptorHeap,
            const std::unordered_map<UINT, WRAPPED_GPU_POINTER> &boundAccelerationStructures,
            const D3D12_DISPATCH_RAYS_DESC &desc) = 0;

        virtual ShaderIdentifier *GetShaderIdentifier(LPCWSTR pExportName) = 0;
        virtual UINT64 GetShaderStackSize(LPCWSTR pExportName) = 0;

        virtual void SetPredispatchCallback(std::function<void(ID3D12GraphicsCommandList *, UINT)> callback) = 0;
    };
}
