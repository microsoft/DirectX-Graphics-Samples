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
#include "DXSample.h"
#include "DynamicConstantBuffer.h"
#include "MemoryMappedPSOCache.h"
#include "MemoryMappedPipelineLibrary.h"
#include "SimpleVertexShader.hlsl.h"
#include "SimplePixelShader.hlsl.h"
#include "QuadVertexShader.hlsl.h"
#include "InvertPixelShader.hlsl.h"
#include "BlitPixelShader.hlsl.h"
#include "GrayScalePixelShader.hlsl.h"
#include "EdgeDetectPixelShader.hlsl.h"
#include "BlurPixelShader.hlsl.h"
#include "WarpPixelShader.hlsl.h"
#include "PixelatePixelShader.hlsl.h"
#include "DistortPixelShader.hlsl.h"
#include "WavePixelShader.hlsl.h"
#include "UberPixelShader.hlsl.h"

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace std;

enum PSOCachingMechanism
{
	CachedBlobs,

	// Enables applications to explicitly group PSOs which are expected to share data. Recommended over Cached Blobs.
	PipelineLibraries,

	PSOCachingMechanismCount
};

enum EffectPipelineType : UINT
{
	// These always get compiled at startup.
	BaseNormal3DRender,
	BaseUberShader,

	// These are compiled a la carte.
	PostBlit,
	PostInvert,
	PostGrayScale,
	PostEdgeDetect,
	PostBlur,
	PostWarp,
	PostPixelate,
	PostDistort,
	PostWave,
	EffectPipelineTypeCount
};

struct GraphicsShaderSet
{
	D3D12_INPUT_LAYOUT_DESC inputLayout;
	D3D12_SHADER_BYTECODE VS;
	D3D12_SHADER_BYTECODE PS;
	D3D12_SHADER_BYTECODE DS;
	D3D12_SHADER_BYTECODE HS;
	D3D12_SHADER_BYTECODE GS;
};

static const D3D12_INPUT_ELEMENT_DESC g_cSimpleInputElementDescs[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

static const D3D12_INPUT_ELEMENT_DESC g_cQuadInputElementDescs[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

static const D3D12_INPUT_LAYOUT_DESC g_cForwardRenderInputLayout = { g_cSimpleInputElementDescs, _countof(g_cSimpleInputElementDescs) };
static const D3D12_INPUT_LAYOUT_DESC g_cQuadInputLayout = { g_cQuadInputElementDescs, _countof(g_cQuadInputElementDescs) };

static const GraphicsShaderSet g_cEffectShaderData[EffectPipelineTypeCount] =
{
	{
		g_cForwardRenderInputLayout,
		CD3DX12_SHADER_BYTECODE(g_SimpleVertexShader, sizeof(g_SimpleVertexShader)),
		CD3DX12_SHADER_BYTECODE(g_SimplePixelShader, sizeof(g_SimplePixelShader)),
		{},
		{},
		{},
	},
	{
		g_cQuadInputLayout,
		CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
		CD3DX12_SHADER_BYTECODE(g_UberPixelShader, sizeof(g_UberPixelShader)),
		{},
		{},
		{},
	},
	{
		g_cQuadInputLayout,
		CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
		CD3DX12_SHADER_BYTECODE(g_BlitPixelShader, sizeof(g_BlitPixelShader)),
		{},
		{},
		{},
	},
	{
		g_cQuadInputLayout,
		CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
		CD3DX12_SHADER_BYTECODE(g_InvertPixelShader, sizeof(g_InvertPixelShader)),
		{},
		{},
		{},
	},
	{
		g_cQuadInputLayout,
		CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
		CD3DX12_SHADER_BYTECODE(g_GrayScalePixelShader, sizeof(g_GrayScalePixelShader)),
		{},
		{},
		{},
	},
	{
		g_cQuadInputLayout,
		CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
		CD3DX12_SHADER_BYTECODE(g_EdgeDetectPixelShader, sizeof(g_EdgeDetectPixelShader)),
		{},
		{},
		{},
	},
	{
		g_cQuadInputLayout,
		CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
		CD3DX12_SHADER_BYTECODE(g_BlurPixelShader, sizeof(g_BlurPixelShader)),
		{},
		{},
		{},
	},
	{
		g_cQuadInputLayout,
		CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
		CD3DX12_SHADER_BYTECODE(g_WarpPixelShader, sizeof(g_WarpPixelShader)),
		{},
		{},
		{},
	},
	{
		g_cQuadInputLayout,
		CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
		CD3DX12_SHADER_BYTECODE(g_PixelatePixelShader, sizeof(g_PixelatePixelShader)),
		{},
		{},
		{},
	},
	{
		g_cQuadInputLayout,
		CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
		CD3DX12_SHADER_BYTECODE(g_DistortPixelShader, sizeof(g_DistortPixelShader)),
		{},
		{},
		{},
	},
	{
		g_cQuadInputLayout,
		CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
		CD3DX12_SHADER_BYTECODE(g_WavePixelShader, sizeof(g_WavePixelShader)),
		{},
		{},
		{},
	},
};

static const LPWCH g_cPipelineLibraryFileName = L"pipelineLibrary.cache";

static const LPWCH g_cCacheFileNames[EffectPipelineTypeCount] =
{
	L"normal3dPSO.cache",
	L"ubershaderPSO.cache",
	L"blitEffectPSO.cache",
	L"invertEffectPSO.cache",
	L"grayscaleEffectPSO.cache",
	L"edgeDetectEffectPSO.cache",
	L"blurEffectPSO.cache",
	L"warpEffectPSO.cache",
	L"pixelateEffectPSO.cache",
	L"distortEffectPSO.cache",
	L"waveEffectPSO.cache",
};

static const LPWCH g_cEffectNames[EffectPipelineTypeCount] =
{
	L"Normal 3D",
	L"Generic post effect",
	L"Blit",
	L"Invert",
	L"Grayscale",
	L"Edge detect",
	L"Blur",
	L"Warp",
	L"Pixelate",
	L"Distort",
	L"Wave",
};

class PSOLibrary
{
public:
	PSOLibrary(UINT frameCount, UINT cbvRootSignatureIndex);
	~PSOLibrary();

	void Build(ID3D12Device* pDevice, ID3D12RootSignature* pRootSignature);

	void SetPipelineState(
		ID3D12Device* pDevice,
		ID3D12RootSignature* pRootSignature,
		ID3D12GraphicsCommandList* pCommandList,
		_In_range_(0, EffectPipelineTypeCount-1) EffectPipelineType type,
		UINT frameIndex);

	void EndFrame();
	void ClearPSOCache();
	void ToggleUberShader();
	void ToggleDiskLibrary();
	void SwitchPSOCachingMechanism();
	void DestroyShader(EffectPipelineType type);

	bool UberShadersEnabled() { return m_useUberShaders; }
	bool DiskCacheEnabled() { return m_useDiskLibraries; }
	PSOCachingMechanism GetPSOCachingMechanism() { return m_psoCachingMechanism; }

private:
	static const UINT BaseEffectCount = 2;

	struct CompilePSOThreadData
	{
		PSOLibrary* pLibrary;
		ID3D12Device* pDevice;
		ID3D12RootSignature* pRootSignature;
		EffectPipelineType type;

		HANDLE threadHandle;
	};

	// This will be used to tell the uber shader which effect to use.
	struct UberShaderConstantBuffer
	{
		UINT32 effectIndex;
	};

	static void CompilePSO(CompilePSOThreadData* pDataPackage);
	void WaitForThreads();

	ComPtr<ID3D12PipelineState> m_pipelineStates[EffectPipelineTypeCount];
	bool m_compiledPSOFlags[EffectPipelineTypeCount];
	bool m_inflightPSOFlags[EffectPipelineTypeCount];
	MemoryMappedPSOCache m_diskCaches[EffectPipelineTypeCount];	// Cached blobs.
	MemoryMappedPipelineLibrary m_pipelineLibrary; // Pipeline Library.
	HANDLE m_flagsMutex;
	CompilePSOThreadData m_workerThreads[EffectPipelineTypeCount];

	bool m_useUberShaders;
	bool m_useDiskLibraries;
	bool m_pipelineLibrariesSupported;
	PSOCachingMechanism m_psoCachingMechanism;
	std::wstring m_cachePath;

	UINT m_cbvRootSignatureIndex;
	UINT m_maxDrawsPerFrame;
	UINT m_drawIndex;

	DynamicConstantBuffer m_dynamicCB;
};
