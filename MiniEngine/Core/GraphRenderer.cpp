//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  Julia Careaga
//

#include "pch.h"
#include "GraphRenderer.h"
#include "CommandContext.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "BufferManager.h"
#include "GameInput.h"
#include "SystemTime.h"
#include "EngineProfiling.h"

#include "CompiledShaders/PerfGraphBackgroundVS.h"
#include "CompiledShaders/PerfGraphVS.h"
#include "CompiledShaders/PerfGraphPS.h"

#define PERF_GRAPH_ERROR uint32_t(0xFFFFFFFF)
#define MAX_GLOBAL_GRAPHS 2
#define MAX_PROFILE_GRAPHS 32
#define MAX_ACTIVE_PROFILE_GRAPHS 4
#define PROFILE_NODE_COUNT 256
#define GLOBAL_NODE_COUNT 512
#define PROFILE_DEBUG_VAR_COUNT 2

using namespace Graphics;
using namespace std;
using namespace GraphRenderer;
using namespace Math;

__declspec(align(16)) struct CBGraph
{	
    float RGB[3];
    float RcpXScale;
    uint32_t NodeCount;
    uint32_t FrameID;
};

class GraphVector;

class PerfGraph
{
friend GraphVector;
public:
    PerfGraph( uint32_t NodeCount, uint32_t debugVarCount, Color color = Color(1.0f, 0.0f, 0.5f), bool IsGraphed = false ) : m_IsGraphed(IsGraphed), 
        m_NodeCount(NodeCount), m_Color(color), m_DebugVarCount(debugVarCount)
    {
        for (uint32_t i = 0; i < debugVarCount; ++i)		
            m_PerfTimesCPUBuffer.emplace_back(new float[NodeCount]);
    }

    ~PerfGraph()
    {
        Clear();
    }

    void Clear(){ m_PerfTimesCPUBuffer.clear();} 
    bool IsGraphed(){ return m_IsGraphed; }
    Color GetColor(){ return m_Color; }
    void SetColor(Color color){m_Color = color;}
    void UpdateGraph( float* timeStamps, uint32_t frameID )
    {
        for(uint32_t i = 0; i < m_DebugVarCount; i++)
            m_PerfTimesCPUBuffer[i][frameID % m_NodeCount] = timeStamps[i];
    } 	

    //RenderGraph renders both graph backgrounds and line graphs 
    //
    //To render backgrounds, set s_GraphBackgroundPSO, set primitive topology to triangle strip, 
    //call RenderGraph without an object and with MaxArray = nullptr.  
    //
    //To render line graph, set s_RenderPerfGraphPSO, set primitive topology to line strip,
    //call RenderGraph on an associated PerfGraph object, and pass in MaxArray
    static void RenderGraph( GraphicsContext& Context, uint32_t vertexCount, D3D12_VIEWPORT& viewport,
        uint32_t debugVarCount, float topMargin);

    void RenderGraph(GraphicsContext& Context, uint32_t vertexCount, D3D12_VIEWPORT& viewport,
        uint32_t debugVarCount, float topMargin, float MaxValue, uint32_t frameID);

    void RenderGraph(GraphicsContext& Context, uint32_t vertexCount, D3D12_VIEWPORT& viewport,
        uint32_t debugVarCount, float topMargin, const float* MaxArray, uint32_t frameID);

private:
    std::vector<std::unique_ptr<float[]>> m_PerfTimesCPUBuffer;
    uint32_t m_NodeCount;
    bool m_IsGraphed;
    Color m_Color;
    uint32_t m_ColorKey;
    uint32_t m_DebugVarCount;
};


class GraphVector 
{
public:
    GraphVector(uint32_t MaxActiveGraphs, uint32_t DebugVarCount) : m_MaxActiveGraphs(MaxActiveGraphs),
        m_ActiveGraphs(0), m_DebugVarCount(DebugVarCount), m_MinAbs((float)PERF_GRAPH_ERROR),
        m_MaxAbs(0.0f), m_FrameOfMinAbs(0), m_FrameOfMaxAbs(0)
    {
        // Fill color array with set of possible graph colors (up to 8 different colors)
        m_ColorArray.reset(new Color[MaxActiveGraphs]);
        for (uint32_t i = 0; i < m_MaxActiveGraphs; ++i)
        {
            m_ColorKeyStack.push_back(i);
            uint32_t colorKey = i + 1;
            float R = (float)(colorKey & 1);
            float G = (float)((colorKey >> 1) & 1) + 0.3f;
            float B = (float)((colorKey >> 2) & 1) + 0.3f;
            m_ColorArray[i] = Color(R, G, B);
        }
        
        m_Max.reset(new float[DebugVarCount]); 
        m_Min.reset(new float[DebugVarCount]);
        m_FrameOfMax.reset(new uint32_t[DebugVarCount]);
        m_FrameOfMin.reset(new uint32_t[DebugVarCount]);
        m_PresetMax.reset(new float[DebugVarCount]);

        for (uint32_t i = 0; i < DebugVarCount; ++i)
        {
            m_Max[i] = 0.0f;
            m_Min[i] = (float)PERF_GRAPH_ERROR;
            m_FrameOfMax[i] = m_FrameOfMin[i] = 0;
            m_PresetMax[i] = 30.0f;
        }

    }

    void Clear()
    {
        m_Graphs.clear();
    }

    GraphHandle AddGraph(PerfGraph* graph)
    {
        GraphHandle ret = (GraphHandle)m_Graphs.size();
        m_Graphs.emplace_back(graph);	
        return ret;
    }

    bool Toggle(GraphHandle GraphID)
    {
        if (m_ActiveGraphs < m_MaxActiveGraphs && !m_Graphs[GraphID]->m_IsGraphed)
        {
            //add to active list
            m_Graphs[GraphID]->m_IsGraphed = true;
            ++m_ActiveGraphs;
            //set color 
            m_Graphs[GraphID]->m_ColorKey = m_ColorKeyStack.back();
            m_ColorKeyStack.pop_back();
            m_Graphs[GraphID]->m_Color = m_ColorArray[m_Graphs[GraphID]->m_ColorKey];
        }
        else if (m_Graphs[GraphID]->m_IsGraphed)
        {
            //take it off of active list
            m_ColorKeyStack.push_back(m_Graphs[GraphID]->m_ColorKey);
            m_Graphs[GraphID]->m_IsGraphed = false;
            --m_ActiveGraphs;
        }
        return m_Graphs[GraphID]->m_IsGraphed;
    }

    
    Color GetColor(GraphHandle GraphID){ return m_Graphs[GraphID]->m_Color;}
    uint32_t Size(){return (uint32_t)m_Graphs.size();}
    uint32_t GetActiveGraphCount(){return m_ActiveGraphs;}

    float* GetPresetMax(){return m_PresetMax.get();}
    float GetGlobalPresetMax()
    {
        float max = 0.0f;
        for (uint32_t i = 0; i < m_DebugVarCount; ++i)
        {
            if (m_PresetMax[i] > max)
                max = m_PresetMax[i];
        }
        return max;
    }
    float* GetMaxAbs(){return &m_MaxAbs;}
    float* GetMinAbs(){return &m_MinAbs;}
    float* GetMax(){return m_Max.get();}
    float* GetMin(){return m_Min.get();}
    
    void PresetMax(const float* maxArray)
    {
        for (uint32_t i = 0; i < m_DebugVarCount; ++i)
            m_PresetMax[i] = maxArray[i];
    }
    
    void ManageMax(float* InputNode, uint32_t nodeCount, uint32_t FrameID)
    {
        for (uint32_t i = 0; i < m_DebugVarCount; ++i)
        {
            //Absolute min max
            if (FrameID - m_FrameOfMinAbs > nodeCount)
                m_MinAbs = (float)PERF_GRAPH_ERROR;
        
            if (FrameID - m_FrameOfMaxAbs > nodeCount)
                m_MaxAbs = 0.0f;
        
            if (InputNode[i] > m_MaxAbs)
            {
                m_MaxAbs = InputNode[i];
                m_FrameOfMaxAbs = FrameID;
            }

            if (InputNode[i] < m_MinAbs)
            {
                m_MinAbs = InputNode[i];
                m_FrameOfMinAbs = FrameID;
            }

            //Relative min max
            if (FrameID - m_FrameOfMax[i] > nodeCount)
                m_Max[i] = 0.0f;
    
            if (FrameID - m_FrameOfMin[i] > nodeCount)
                m_Min[i] = (float)PERF_GRAPH_ERROR;

            if (InputNode[i] > m_Max[i])
            {
                m_Max[i] = InputNode[i];
                m_FrameOfMax[i] = FrameID;
            }
            if (InputNode[i] < m_Min[i])
            {
                m_Min[i] = InputNode[i];
                m_FrameOfMin[i] = FrameID;
            }
        }

    }

    std::vector<std::unique_ptr<PerfGraph>> m_Graphs; // this should be private

private:
    
    uint32_t m_ActiveGraphs;
    uint32_t m_MaxActiveGraphs;
    uint32_t m_DebugVarCount;
    std::unique_ptr<Color[]> m_ColorArray;
    std::vector<uint32_t> m_ColorKeyStack;
    
    float m_MaxAbs;
    float m_MinAbs;
    uint32_t m_FrameOfMaxAbs;
    uint32_t m_FrameOfMinAbs;

    std::unique_ptr<float[]> m_PresetMax;
    std::unique_ptr<float[]> m_Max; 
    std::unique_ptr<float[]> m_Min;
    std::unique_ptr<uint32_t[]> m_FrameOfMax;
    std::unique_ptr<uint32_t[]> m_FrameOfMin;
};

namespace
{
    RootSignature s_RootSignature;
    GraphicsPSO s_RenderPerfGraphPSO;
    GraphicsPSO s_GraphBackgroundPSO;
    uint32_t s_FrameID;
    GraphVector GlobalGraphs = GraphVector(2, 1);
    GraphVector ProfileGraphs = GraphVector(MAX_ACTIVE_PROFILE_GRAPHS, PROFILE_DEBUG_VAR_COUNT);
    uint32_t s_NumStamps = 0;
    uint32_t s_SelectedTimerIndex;
} // {anonymous} namespace



//---------------------------------------------------------------------
//
//	GraphRenderer Methods
//
//---------------------------------------------------------------------

static void DrawGraphHeaders(TextContext& Text, float leftMargin, float topMargin, float offsetY, float graphHeight, float* MinArray, 
    float* MaxArray, float* PresetMaxArray, bool GlobalScale,  uint32_t numDebugVar, std::string graphTitles[]);


void GraphRenderer::Initialize( void )
{
    s_RootSignature.Reset(4);
    s_RootSignature[0].InitAsConstantBuffer(0);
    s_RootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 2);
    s_RootSignature[2].InitAsBufferSRV(0, D3D12_SHADER_VISIBILITY_VERTEX);
    s_RootSignature[3].InitAsConstants(1, 3);
    s_RootSignature.Finalize(L"Graph Renderer");

    s_RenderPerfGraphPSO.SetRootSignature(s_RootSignature);
    s_RenderPerfGraphPSO.SetRasterizerState(RasterizerDefault);
    s_RenderPerfGraphPSO.SetBlendState(BlendTraditional);
    s_RenderPerfGraphPSO.SetDepthStencilState(Graphics::DepthStateReadOnly);
    s_RenderPerfGraphPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
    s_RenderPerfGraphPSO.SetRenderTargetFormats(1, &g_OverlayBuffer.GetFormat(), g_OverlayBuffer.GetFormat());
    s_RenderPerfGraphPSO.SetVertexShader(g_pPerfGraphVS, sizeof(g_pPerfGraphVS));
    s_RenderPerfGraphPSO.SetPixelShader(g_pPerfGraphPS, sizeof(g_pPerfGraphPS));
    s_RenderPerfGraphPSO.Finalize();

    s_GraphBackgroundPSO = s_RenderPerfGraphPSO;
    s_GraphBackgroundPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    s_GraphBackgroundPSO.SetVertexShader(g_pPerfGraphBackgroundVS, sizeof(g_pPerfGraphBackgroundVS));
    s_GraphBackgroundPSO.Finalize();

    s_FrameID = 0;

    // Preset max for global and profile graphs
    float profilePresetMax[PROFILE_DEBUG_VAR_COUNT] = {15.0f, 1.0f}; //improve this
    ProfileGraphs.PresetMax(profilePresetMax);
    
    // Create global CPU and GPU graphs
    for (uint32_t i = 0; i < 2; ++i)
    {	
        InitGraph( GraphType::Global );
        GlobalGraphs.m_Graphs[i]->SetColor(Color((float)i, 0.5, 0.5));
    }

    float globalPresetMax[1] = {15.0f};
    GlobalGraphs.PresetMax(globalPresetMax); 
}

void GraphRenderer::Shutdown(void)
{
    ProfileGraphs.Clear();
    GlobalGraphs.Clear();
}

GraphHandle GraphRenderer::InitGraph( GraphType type)
{
    if (type == GraphType::Profile)
        return ProfileGraphs.AddGraph(new PerfGraph(PROFILE_NODE_COUNT, 2));
    else if (type == GraphType::Global)
        return GlobalGraphs.AddGraph(new PerfGraph(GLOBAL_NODE_COUNT, 1));
    else
        return PERF_GRAPH_ERROR; 
}

bool GraphRenderer::ManageGraphs( GraphHandle GraphID, GraphType Type)
{
    if (GraphID == PERF_GRAPH_ERROR)
        return false;

    if (Type == GraphType::Profile)
        return ProfileGraphs.Toggle(GraphID);
    else // Type == GraphType::Global
        return GlobalGraphs.Toggle(GraphID);
}

//This is used to set the text color 
Color GraphRenderer::GetGraphColor( GraphHandle GraphID, GraphType Type)
{
    if (Type == GraphType::Profile)
        return ProfileGraphs.GetColor(GraphID);
    else // Type == GraphType::Global
        return GlobalGraphs.GetColor(GraphID);	
}

void GraphRenderer::Update( XMFLOAT2 InputNode, GraphHandle GraphID, GraphType Type)
{
    if (GraphID == PERF_GRAPH_ERROR)
        return;

    if (Type == GraphType::Profile)
    {
        float input[2] = {InputNode.x, InputNode.y};
        ProfileGraphs.m_Graphs[GraphID]->UpdateGraph(input, s_FrameID);
        if (ProfileGraphs.m_Graphs[GraphID]->IsGraphed())
        {
            float times[4] = {InputNode.x, InputNode.y};
            ProfileGraphs.ManageMax(times, PROFILE_NODE_COUNT, s_FrameID);
        }
    }
    else // Type == PerfGraph::Global
    {
        GlobalGraphs.m_Graphs[0]->UpdateGraph(&InputNode.x, s_FrameID);
        GlobalGraphs.m_Graphs[1]->UpdateGraph(&InputNode.y, s_FrameID);
        GlobalGraphs.ManageMax(&InputNode.x, GLOBAL_NODE_COUNT, s_FrameID);
        //GlobalGraphs.ManageMax(&InputNode.y, GLOBAL_NODE_COUNT, s_FrameID);
    }	
}

void DrawGraphHeaders(TextContext& Text, float leftMargin, float topMargin, float offsetY, float graphHeight, float* MinArray,
    float* MaxArray, float* PresetMaxArray, bool GlobalScale, uint32_t numDebugVar, std::string graphTitles[])
{		
    XMFLOAT2 textSpaceY = XMFLOAT2(0.02f * graphHeight, 0.067f * graphHeight); //top and bottom text space
    textSpaceY.y = graphHeight - topMargin - textSpaceY.x * 3.0f; // make this better
    float textSpaceX = 45.f;
    Text.SetColor(Color(1.0f, 1.0f, 1.0f));
    Text.SetTextSize(12.0f);
        
    float min = MinArray[0];
    float max = MaxArray[0];
    float presetMax = PresetMaxArray[0];

    for (uint32_t i = 0; i < numDebugVar; i++)
    {
        if (!GlobalScale)
        {
            min = MinArray[i];
            max = MaxArray[i];
            presetMax = PresetMaxArray[i];
        }

        Text.SetCursorY(topMargin / 2.0f + (i * graphHeight) + offsetY); // division needs to be a factor 
        Text.SetCursorX(leftMargin + (0.4f * textSpaceX));
        Text.DrawString(graphTitles[i]);
        Text.DrawFormattedString("Min:%3.3f   Max:%3.3f", min, max);

        Text.SetCursorX(leftMargin - textSpaceX);
        float topText = topMargin + (i * graphHeight);
        Text.SetCursorY(topText + textSpaceY.x + offsetY);
        Text.DrawFormattedString("%3.3f", presetMax);

        Text.SetCursorX(leftMargin - textSpaceX);
        Text.SetCursorY(topText + textSpaceY.y + offsetY);
        Text.DrawString("0.000");
    }
}

void GraphRenderer::RenderGraphs(GraphicsContext& Context, GraphType Type)
{
    if (Type == GraphType::Global && GlobalGraphs.Size() == 0 ||
        Type == GraphType::Profile && ProfileGraphs.Size() == 0)
    {
        s_FrameID++; // probably need to reset this after time = uint32_t max val
        return;
    }

    TextContext Text(Context);
    Text.Begin();

    if (Type == GraphType::Profile && ProfileGraphs.GetActiveGraphCount() > 0)
    {
        D3D12_VIEWPORT viewport;
        viewport.TopLeftX = (float)g_OverlayBuffer.GetWidth() / 1.3525f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = (float)g_OverlayBuffer.GetWidth() / 4.0f;
        viewport.Height = (float)g_OverlayBuffer.GetHeight() / (PROFILE_DEBUG_VAR_COUNT + 1);	
        viewport.MinDepth = 0.0;
        viewport.MaxDepth = 1.0;	

        std::string graphTitles[PROFILE_DEBUG_VAR_COUNT] = {"Inclusive CPU   ", "Inclusive GPU   "};
        float blankSpace = viewport.Height / (PROFILE_DEBUG_VAR_COUNT + 1); 
        XMFLOAT2 textSpace = XMFLOAT2(45.0f, 5.0f);
        DrawGraphHeaders(Text, (viewport.TopLeftX),  blankSpace, 0.0f, (viewport.Height + blankSpace), ProfileGraphs.GetMin(), 
            ProfileGraphs.GetMax(), ProfileGraphs.GetPresetMax(), false, PROFILE_DEBUG_VAR_COUNT, graphTitles);
        
        Context.SetRootSignature(s_RootSignature);
        Context.TransitionResource(g_OverlayBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
        Context.SetRenderTarget(g_OverlayBuffer.GetRTV());
        Context.SetPipelineState(s_GraphBackgroundPSO);
        Context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        // Render backgrounds
        PerfGraph::RenderGraph(Context, 4, viewport, PROFILE_DEBUG_VAR_COUNT, blankSpace);
        Context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);
        viewport.TopLeftY = 0.0f;
    
        for (auto iter = ProfileGraphs.m_Graphs.begin(); iter != ProfileGraphs.m_Graphs.end(); ++iter)
        {
            if ((*iter)->IsGraphed())
            {
                (*iter)->RenderGraph(Context, 256, viewport, PROFILE_DEBUG_VAR_COUNT, blankSpace, ProfileGraphs.GetPresetMax(), s_FrameID);
                viewport.TopLeftY = 0.0f;
            }
        }
    }
    else if (Type == GraphType::Global)
    {
        D3D12_VIEWPORT viewport;
        viewport.TopLeftX = (float)g_OverlayBuffer.GetWidth() / 4.0f;
        viewport.TopLeftY = (float)g_OverlayBuffer.GetHeight() / 1.3f;
        viewport.Width = (float)g_OverlayBuffer.GetWidth() / 2.0f;
        viewport.Height = (float)g_OverlayBuffer.GetHeight() / 8.0f;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
    
        float blankSpace = viewport.Height / 8.0f;
        XMFLOAT2 textSpace = XMFLOAT2(45.0f, 5.0f);
        std::string graphTitles[] = { "CPU - GPU      " };
        DrawGraphHeaders( Text, (viewport.TopLeftX), blankSpace,  (viewport.TopLeftY - blankSpace - textSpace.y), (viewport.Height + blankSpace), 
                                        GlobalGraphs.GetMinAbs(), GlobalGraphs.GetMaxAbs(), GlobalGraphs.GetPresetMax(), true, 1, graphTitles);

        Context.SetRootSignature(s_RootSignature);
        Context.TransitionResource(g_OverlayBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
        Context.SetRenderTarget(g_OverlayBuffer.GetRTV());
        Context.SetPipelineState(s_GraphBackgroundPSO);
        Context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        
        // Render background
        PerfGraph::RenderGraph(Context, 4, viewport, 1, 0.0f);
    
        // Render graphs
        Context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);
        for (auto iter = GlobalGraphs.m_Graphs.begin(); iter != GlobalGraphs.m_Graphs.end(); ++iter)
        {
            (*iter)->RenderGraph(Context, 512, viewport, 1, 0.0f, GlobalGraphs.GetPresetMax(), s_FrameID);
        }
    }
    s_FrameID++;
    Text.End();
    Context.SetViewport(0, 0, 1920, 1080);
}

void GraphRenderer::SetSelectedIndex(uint32_t selectedIndex)
{
    s_SelectedTimerIndex = selectedIndex;
}

//---------------------------------------------------------------------
//
//	PerfGraph Methods
//
//---------------------------------------------------------------------

void PerfGraph::RenderGraph(GraphicsContext& Context, uint32_t vertexCount, D3D12_VIEWPORT& viewport, uint32_t debugVarCount, float topMargin)
{
    viewport.TopLeftY += topMargin;

    Context.SetConstants(3, 0, 20.0f);

    for (uint32_t i = 0; i < debugVarCount; ++i)
    {
        Context.SetViewport(viewport);
        Context.Draw(vertexCount);
        if (debugVarCount > 1)
            viewport.TopLeftY += viewport.Height + topMargin;
    }
}

void PerfGraph::RenderGraph(GraphicsContext& Context, uint32_t vertexCount, D3D12_VIEWPORT& viewport, uint32_t debugVarCount, float topMargin, float YScale, uint32_t frameID)
{
    viewport.TopLeftY += topMargin;

    CBGraph graphConstants;
    graphConstants.RGB[0] = m_Color.R();
    graphConstants.RGB[1] = m_Color.G();
    graphConstants.RGB[2] = m_Color.B();
    graphConstants.RcpXScale = 2.0f / m_NodeCount;
    graphConstants.NodeCount = m_NodeCount;
    graphConstants.FrameID = frameID;
    Context.SetDynamicConstantBufferView(0, sizeof(CBGraph), &graphConstants);
    Context.SetPipelineState(s_RenderPerfGraphPSO);

    for (uint32_t i = 0; i < debugVarCount; ++i)
    {
        Context.SetDynamicSRV(2, sizeof(float) * m_NodeCount, m_PerfTimesCPUBuffer[i].get());
        Context.SetConstants(3, i, 1.0f / YScale);
        Context.SetViewport(viewport);
        Context.Draw(vertexCount);
        if (debugVarCount > 1)
            viewport.TopLeftY += viewport.Height + topMargin;
    }
}

void PerfGraph::RenderGraph( GraphicsContext& Context, uint32_t vertexCount, D3D12_VIEWPORT& viewport, uint32_t debugVarCount, float topMargin, const float* MaxArray, uint32_t frameID)
{
    ASSERT(MaxArray != nullptr);
    viewport.TopLeftY += topMargin;

    CBGraph graphConstants;
    graphConstants.RGB[0] = m_Color.R();
    graphConstants.RGB[1] = m_Color.G();
    graphConstants.RGB[2] = m_Color.B();
    graphConstants.RcpXScale = 2.0f / m_NodeCount;
    graphConstants.NodeCount = m_NodeCount;
    graphConstants.FrameID = frameID;
    Context.SetDynamicConstantBufferView(0, sizeof(CBGraph), &graphConstants);
    Context.SetPipelineState(s_RenderPerfGraphPSO);
    
    for (uint32_t i = 0; i < debugVarCount; ++i)
    {
        Context.SetDynamicSRV(2, sizeof(float) * m_NodeCount, m_PerfTimesCPUBuffer[i].get());
        Context.SetConstants(3, i, 1.0f / MaxArray[i]);
        Context.SetViewport(viewport);
        Context.Draw(vertexCount);
        if (debugVarCount > 1)
            viewport.TopLeftY += viewport.Height + topMargin;
    }
}
