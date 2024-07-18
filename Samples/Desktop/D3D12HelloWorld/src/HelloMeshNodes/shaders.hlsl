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

struct PSInput
{
    float4 position : SV_POSITION;
    nointerpolation float  redChannel : RED;
};

struct MeshNodesGlobalStruct {
    float greenChannel;
};

ConstantBuffer<MeshNodesGlobalStruct> MeshNodesGlobalData : register(b0);

struct MeshNodesLocalStruct {
    float blueChannel;
};

ConstantBuffer<MeshNodesLocalStruct> MeshNodesLocalData : register(b1);

#ifdef LIB_TARGET

GlobalRootSignature MeshNodesGlobalRS = 
{
    "RootConstants(num32BitConstants=1,b0) "
};

LocalRootSignature MeshNodesLocalRS =
{
    "RootConstants(num32BitConstants=1,b1) "
};

struct BinningRecord
{
    uint16_t materialID;
    float4 position;
    float redChannel;
};

struct MeshNodeRecord
{
    float4 position;
    float redChannel;
};

[Shader("node")]
[NodeLaunch("thread")]
[NodeIsProgramEntry]
[NumThreads(1,1,1)]
void Root(
    ThreadNodeInputRecord<BinningRecord> input,
    [UnboundedSparseNodes] [MaxRecords(1)] [NodeID("Materials")] NodeOutputArray<MeshNodeRecord> bins
)
{
    ThreadNodeOutputRecords<MeshNodeRecord> outRec = bins[input.Get().materialID].GetThreadNodeOutputRecords(1);
    outRec.Get().position = input.Get().position;
    outRec.Get().redChannel = input.Get().redChannel;
    outRec.OutputComplete();
}

struct MeshOutVert
{
    float4 position : SV_POSITION;
};

struct MeshOutPrim
{
    float redChannel : RED;
};

[Shader("node")]
[NodeLaunch("mesh")]
[NumThreads(1,1,1)]
[NodeDispatchGrid(1,1,1)]
[NodeIsProgramEntry] // allow mesh node to also act as direct program entry (for fun)
[NodeID("Materials",0)]
//[NodeIsProgramEntry] // allow mesh node to also act as direct program entry (for fun)
[OutputTopology("triangle")]
void Materials(
    DispatchNodeInputRecord<MeshNodeRecord> input,
    out vertices MeshOutVert verts[3],
    out primitives MeshOutPrim prim[1],
    out indices uint3 idx[1]
)
{
    SetMeshOutputCounts(3, 1);
    float4 center = input.Get().position;
    verts[0].position = center + float4(0, 0.1f, 0, 0);
    verts[1].position = center + float4(0.1f, -0.1f, 0, 0);
    verts[2].position = center + float4(-0.1f, -0.1f, 0, 0);
    prim[0].redChannel = input.Get().redChannel;
    idx[0] = uint3(0, 1, 2);
}

#else

float4 PSMain(PSInput input) : SV_TARGET
{
    // Half green channel
    return float4(input.redChannel,MeshNodesGlobalData.greenChannel/2,MeshNodesLocalData.blueChannel,1);
}

float4 PSMain2(PSInput input) : SV_TARGET
{
    return float4(input.redChannel,MeshNodesGlobalData.greenChannel,MeshNodesLocalData.blueChannel,1);
}

#endif

