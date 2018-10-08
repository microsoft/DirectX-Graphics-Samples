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
#define ENABLE_UAV_LOG 0

// Set to 1 to visualize acceleration structure. 
// Since this writes to a raytracing output during ray traversal, 
// the Fallback Layer must have an output that is used by the application defined and
// an application shaders must disable writing to the output (i.e. in a miss/hit shaders).
#define ENABLE_ACCELERATION_STRUCTURE_VISUALIZATION 0
