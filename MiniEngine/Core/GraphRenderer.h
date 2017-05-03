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

#include "GraphicsCore.h"

namespace GraphRenderer
{
    void Initialize();
    void Shutdown();

    enum class GraphType { Global, Profile };
    typedef uint32_t GraphHandle;

    bool ManageGraphs( GraphHandle graphID, GraphType Type );
    GraphHandle InitGraph( GraphType Type );
    Color GetGraphColor( GraphHandle GraphID, GraphType Type);
    XMFLOAT4 GetMaxAvg( GraphType Type );
    void Update( XMFLOAT2 InputNode, GraphHandle GraphID, GraphType Type);
    void RenderGraphs( GraphicsContext& Context, GraphType Type );

    void SetSelectedIndex(uint32_t selectedIndex);

} // namespace GraphRenderer
