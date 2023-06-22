//=================================================================================================================================
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//=================================================================================================================================

// ================================================================================================================================
// D3D12 Work Graph Sandbox shaders
// 
// There are two parts to this file split by #define EXECUTION_PARAMETERS.
// 
// (1) The EXECUTION_PARAMETERS part allows some constants describing the work graph to be fed back into the app .exe to 
//     be able to drive the graph with synthetic data.  This way the app can be tweaked by just editing the HLSL file
//     without having to recompile the app in a lot of cases.
// 
// (2) The rest of the file is where an arbitrary set of nodes can be defined that will be turned into a work graph and executed.
//     The app will just grab all the nodes that are defined and make a work graph out of it, no need to name the nodes in any
//     special way of mark entrypoints (unless you want to).  Nodes that are not fed by other nodes or that are tagged 
//     explicitly as [NodeIsProgramEntry] will be fed with synthetic data from the app.  The app will feed each entrypoint 
//     NUM_RECORDS_PER_ENTRYPOINT records, with the first uint in the record (if size >= 1 uint) initialized to i = 0,1,2 for 
//     record [i] going to the node.  The rest of the record gets initialized with the value (uint)3, repeated until full.
// 
//     Nodes in the graph can access a 16MB RWByteAddressBuffer UAV (it's called "UAV") to store results or cross communicate.
//     
//     NUM_UINTS_TO_PRINT_TO_CONSOLE defines how many uints from the beginning of the UAV to print out to console after 
//     execution. By setting up the graph to write some final results to this section of the UAV, they will be able to 
//     be read from the console to verify that the graph did what was expected.
// 
// ================================================================================================================================

#ifdef EXECUTION_PARAMETERS
// ================================================================================================================================
// Test parameters read back into the exe.

// First uint in each record sent to each entrypoint will be initialized to 0,1,2 etc. per entrypoint (if big enough record).
// The rest of the input record (if any space left) will be zeroed out
#define NUM_RECORDS_PER_ENTRYPOINT 5

// 1 to pass DispatchGraph inputs to the graph via GPU memory in command list recording.
// 0 to use CPU memory
#define FEED_GRAPH_INPUTS_FROM_GPU_MEMORY 0

// How many uints from the UAV to print to the console after the test completes
#define NUM_UINTS_TO_PRINT_TO_CONSOLE 5 

// ================================================================================================================================
// This shader is just for setup - it feeds the above constants into the app to drive the Dispatch() call
// and to tell it the expected number of tasks completed (where the "task" is just incrementing a count by 1)
RWStructuredBuffer<uint> ParameterReadbackUAV : register(u0,space1); // Reusing same buffer that main test uses below
[RootSignature("UAV(u0,space=1)")]
[numthreads(1,1,1)]
void executionParameters()
{
    ParameterReadbackUAV[0] = NUM_RECORDS_PER_ENTRYPOINT;
    ParameterReadbackUAV[1] = FEED_GRAPH_INPUTS_FROM_GPU_MEMORY;
    ParameterReadbackUAV[2] = NUM_UINTS_TO_PRINT_TO_CONSOLE;
    DeviceMemoryBarrier();
}

#else // Work Graphs portion here

// ================================================================================================================================
// Work Graph
// 
// Just define a set of nodes and the executable will make a graph out of them and execute them.
// 
// Note: It's ok to define multiple separate graphs together as a quick way to test multiple things.  
// It simply turns into one work graph that has multiple entrypoints.
// 
// The topology of arbitrary nodes in this contrived example is:
// 
// firstNode ---> secondNode
//      |  |   
//      |  +----> thirdNode,0  (array)
//      |         thirdNode,1
//      |       
//      +-------> fourthNode--+   fourthNode is also marked as a graph entry
//                    ^       |
//                    |       |  (recursion)
//                    +-------+
// 
//  fifthNode
//  
// ================================================================================================================================
// These root signatures as associated with all nodes by default (since explicit association's aren't made:
GlobalRootSignature globalRS = { "UAV(u0)" };
LocalRootSignature localRS = { "RootConstants(num32BitConstants=1,b0)" };

RWStructuredBuffer<uint> UAV : register(u0); // 16MB buffer from global root sig

struct LocalRootArguments {
    uint value; // at local root argument table index [i], value == i
};

// The root signature feeds NodeConstants via local root signature, so the constants are per-node
ConstantBuffer<LocalRootArguments> NodeConstants : register(b0);

struct entryRecord
{
    uint index;
    int data0;
    int data1;
    int data2;
};

struct entryRecord2
{
    uint3 grid : SV_DispatchGrid;
    uint index;
};

struct interiorRecord
{
    uint index;
};

struct interiorRecord2 // just contriving to make it clear the same record doesn't have to be used everywhere
{
    uint index2;
    uint value;
};



[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeLocalRootArgumentsTableIndex(1)] // fixed table location, others will autopopulate if not specified
[NodeDispatchGrid(10, 10, 10)]
[NumThreads(2,1,1)]
void firstNode(
    DispatchNodeInputRecord<entryRecord> inputData,
    [MaxRecords(10)] EmptyNodeOutput secondNode,
    [MaxRecords(2)] [NodeArraySize(2)] NodeOutputArray<interiorRecord> thirdNode,
    [MaxRecords(1)] NodeOutput<interiorRecord2> fourthNode,
    uint groupIndex : SV_Groupindex
)
{
    // Send records to each of the child nodes and accumulate data to UAV[1]

    // Send 5 empty records
    secondNode.GroupIncrementOutputCount(5);
  
    // NodeConstants.value in this example provides the UAV base offset via node local root arguments.
    // This node picks it's local root argument table slot via optional [NodeLocalRootArgumentsTableIndex(1)] above.
    // Which means the other two nodes will get runtime assigned slots since they don't explicitly pick.
    // The C++ code queries what the local root argument slots all ended up being for the graph and uses
    // that to feed back to the nodes their UAV offset via the NodeConstants constant buffer.

    // This shows an example of outputting to a node array, size 2 nodes in this case.
    // Here each thread outputs to a different node in the array, so 2 nodes total (see [MaxRecords(2)] above)
    uint nodeIndex = groupIndex % 2;
    ThreadNodeOutputRecords<interiorRecord> outRec = thirdNode[groupIndex].GetThreadNodeOutputRecords(1);

    // In a future language version, "->" will be available instead of ".Get()" to access record members
    uint total = inputData.Get().index + inputData.Get().data0 + inputData.Get().data1 + inputData.Get().data2;
    outRec.Get().index = total;
    InterlockedAdd(UAV[NodeConstants.value], outRec.Get().index);
    outRec.OutputComplete();
  
    GroupNodeOutputRecords<interiorRecord2> groupOutRec = fourthNode.GetGroupNodeOutputRecords(1);
    groupOutRec.Get().index2 = inputData.Get().index;
    groupOutRec.Get().value = NodeConstants.value;
    groupOutRec.OutputComplete();
}

[Shader("node")]
[NodeLaunch("thread")]
void secondNode()
{
    // Accumulate data to UAV
    // NodeConstants provides the base offset via node local root arguments
    InterlockedAdd(UAV[NodeConstants.value], 1);
}

// coalescing launch is useful if the shader can do some shared work in thread group, such as
// using group shared memory to share work (or the output limits for thread launch are too tight). 
// This example should be thread launch in real use, just using coalescing for illustration
[Shader("node")]
[NodeLaunch("coalescing")] 
[NumThreads(4, 1, 1)]
// NodeIDs default to entry name, array index 0 if explicit ID is not given:
// This shader's ID becomes NodeID("thirdNode",0)
void thirdNode( 
    [MaxRecords(4)] GroupNodeInputRecords<interiorRecord> inputData,
    uint threadIndex : SV_GroupIndex
)
{
    if(threadIndex >= inputData.Count())
        return;

    // Accumulate data to UAV
    // NodeConstants provides the base offset via node local root arguments
    InterlockedAdd(UAV[NodeConstants.value], inputData[threadIndex].index);
}

// Shader in a node array with the previous, given same node name but different array index
[NodeID("thirdNode", 1)]
[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(2, 1, 1)]
void thirdNodeAnotherShader(
    [MaxRecords(2)] GroupNodeInputRecords<interiorRecord> inputData,
    uint threadIndex : SV_GroupIndex
)
{
    if (threadIndex >= inputData.Count())
        return;

    // Accumulate data to UAV
    // NodeConstants provides the base offset via node local root arguments
    InterlockedAdd(UAV[NodeConstants.value], inputData[threadIndex].index);
    InterlockedOr(UAV[NodeConstants.value], 0x80000000);
}

[Shader("node")]
[NodeIsProgramEntry] // A node can be a program entry even if it is also interior to the graph
[NodeLaunch("thread")]
[NodeMaxRecursionDepth(3)]
void fourthNode(
    ThreadNodeInputRecord<interiorRecord2> inputData,
    [MaxRecords(5)] EmptyNodeOutput secondNode,
    [MaxRecords(1)] NodeOutput<interiorRecord2> fourthNode
)
{
    // In a future language version, "->" will be available instead of ".Get()" to access record members

    // Accumulate data to UAV
    // NodeConstants provides the base offset via node local root arguments
    InterlockedAdd(UAV[NodeConstants.value], inputData.Get().index2 + inputData.Get().value);

    if (GetRemainingRecursionLevels())
    {
        ThreadNodeOutputRecords<interiorRecord2> outRec = fourthNode.GetThreadNodeOutputRecords(1);
        outRec.Get().index2 = inputData.Get().index2 + 1;
        outRec.Get().value = NodeConstants.value;
        outRec.OutputComplete();
    }
    secondNode.ThreadIncrementOutputCount(4);
}

// Here's a node that's disconnected from the others in the graph.
// A single work graph can have disconnected subgraphs that can run in parallel.
[Shader("node")]
[NodeLaunch("broadcasting")]
// NodeMaxDispatchGrid means grid size is in the input record, see SV_DispatchGrid in entryRecord2
[NodeMaxDispatchGrid(100,100,100)] // normally, make this as tightly defined as possible
[NumThreads(1, 1, 1)]
void fifthNode(
    DispatchNodeInputRecord<entryRecord2> inputData
)
{
    // Accumulate data to UAV
    // NodeConstants provides the base offset via node local root arguments
    InterlockedAdd(UAV[NodeConstants.value], inputData.Get().index);
}

#endif // Work Graph
