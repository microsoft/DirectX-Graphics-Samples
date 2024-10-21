#pragma once
//===============================================================================
// desc: A collection of constants and utilities to aid all aspects of code modification
// auth: Aliyaan Zulfiqar
//===============================================================================

// A macro to clearly identify my contributions to the starting code
#define AZB_MOD 1	// Change to 0 to exclude my modifications and run unmodified sample code


// This is defined in Display.cpp
constexpr int SWAP_CHAIN_BUFFER_COUNT = 3;
constexpr DXGI_FORMAT SWAP_CHAIN_FORMAT = DXGI_FORMAT_R10G10B10A2_UNORM;

// Set in GameCore.cpp
extern bool g_bMouseExclusive;	// Used to keep track of if the mouse is being exclusively used by the MiniEngine. Set to 0 by default, app starts non-exclusive