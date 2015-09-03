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
// Author:  James Stanard 
//

#include "pch.h"
#include "GraphicsCore.h"
#include "GameCore.h"
#include "BufferManager.h"
#include "GpuTimeManager.h"
#include "PostEffects.h"
#include "SSAO.h"
#include "TextRenderer.h"
#include "ColorBuffer.h"
#include "SystemTime.h"
#include "SamplerManager.h"
#include "DescriptorHeap.h"
#include "CommandContext.h"
#include "CommandListManager.h"
#include "RootSignature.h"
#include "CommandSignature.h"
#include "ParticleEffectManager.h"

#if WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP
	#include <agile.h>
#endif

#include <dxgi1_4.h>	// For WARP

#include "CompiledShaders/ScreenQuadVS.h"
#include "CompiledShaders/BufferCopyPS.h"
#include "CompiledShaders/ConvertLDRToDisplayPS.h"
#include "CompiledShaders/DebugZoom2xPS.h"
#include "CompiledShaders/DebugZoom4xPS.h"
#include "CompiledShaders/BilinearUpsamplePS.h"
#include "CompiledShaders/BicubicUpsamplePS.h"
#include "CompiledShaders/GenerateMipsCS.h"
#include "CompiledShaders/GenerateMipsGammaCS.h"

#define SWAP_CHAIN_BUFFER_COUNT 3

DXGI_FORMAT SwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
DXGI_FORMAT OverlayPlaneFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if (x != nullptr) { x->Release(); x = nullptr; }
#endif

using namespace Math;
using namespace SystemTime;

namespace GameCore
{
#if WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
	extern HWND g_hWnd;
#else
	extern Platform::Agile<Windows::UI::Core::CoreWindow>  g_window;
#endif
}

namespace Graphics
{
#ifndef RELEASE
	const GUID WKPDID_D3DDebugObjectName = { 0x429b8c22,0x9188,0x4b0c, { 0x87,0x42,0xac,0xb0,0xbf,0x85,0xc2,0x00 }};
#endif

	uint32_t g_windowWidth;
	uint32_t g_windowHeight;

	ID3D12Device* g_Device = nullptr;
	ID3D12CommandQueue* g_CommandQueue = nullptr;

	CommandListManager g_CommandManager;

	D3D_FEATURE_LEVEL g_D3DFeatureLevel = D3D_FEATURE_LEVEL_11_0;

	ColorBuffer g_DisplayPlane[SWAP_CHAIN_BUFFER_COUNT];
	UINT g_CurrentBuffer = 0;

	IDXGISwapChain* s_PrimarySwapChain = nullptr;

	DescriptorAllocator g_DescriptorAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
	{
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV
	};

	D3D12_SAMPLER_DESC SamplerLinearWrapDesc;
	D3D12_SAMPLER_DESC SamplerAnisoWrapDesc;
	D3D12_SAMPLER_DESC SamplerShadowDesc;
	D3D12_SAMPLER_DESC SamplerLinearClampDesc;
	D3D12_SAMPLER_DESC SamplerVolumeWrapDesc;
	D3D12_SAMPLER_DESC SamplerPointClampDesc;
	D3D12_SAMPLER_DESC SamplerPointBorderDesc;
	D3D12_SAMPLER_DESC SamplerLinearBorderDesc;

	Sampler SamplerLinearWrap;
	Sampler SamplerAnisoWrap;
	Sampler SamplerShadow;
	Sampler SamplerLinearClamp;
	Sampler SamplerVolumeWrap;
	Sampler SamplerPointClamp;
	Sampler SamplerPointBorder;
	Sampler SamplerLinearBorder;

	D3D12_RASTERIZER_DESC RasterizerDefault;
	D3D12_RASTERIZER_DESC RasterizerDefaultCW;
	D3D12_RASTERIZER_DESC RasterizerTwoSided;
	D3D12_RASTERIZER_DESC RasterizerShadow;
	D3D12_RASTERIZER_DESC RasterizerShadowCW;

	D3D12_BLEND_DESC BlendNoColorWrite;
	D3D12_BLEND_DESC BlendDisable;
	D3D12_BLEND_DESC BlendPreMultiplied;
	D3D12_BLEND_DESC BlendTraditional;
	D3D12_BLEND_DESC BlendAdditive;
	D3D12_BLEND_DESC BlendTraditionalAdditive;

	D3D12_DEPTH_STENCIL_DESC DepthStateDisabled;
	D3D12_DEPTH_STENCIL_DESC DepthStateReadWrite;
	D3D12_DEPTH_STENCIL_DESC DepthStateReadOnly;
	D3D12_DEPTH_STENCIL_DESC DepthStateReadOnlyReversed;
	D3D12_DEPTH_STENCIL_DESC DepthStateTestEqual;

	CommandSignature DispatchIndirectCommandSignature(1);
	CommandSignature DrawIndirectCommandSignature(1);

	RootSignature s_PresentRS;
	GraphicsPSO s_BlendUIPSO;
	GraphicsPSO ConvertLDRToDisplayPS;
	GraphicsPSO DebugZoom2xPS;
	GraphicsPSO DebugZoom4xPS;
	GraphicsPSO BicubicUpsamplePS;
	GraphicsPSO BilinearUpsamplePS;
	GraphicsPSO TemporalUpsamplePS;

	RootSignature g_GenerateMipsRS;
	ComputePSO g_GenerateMipsPSO;
	ComputePSO g_GenerateMipsGammaPSO;

	BoolVar BicubicUpsample("Graphics/Scaling/Bicubic Upscale", true);
	NumVar BicubicUpsampleWeight("Graphics/Scaling/Cubic Weight", -0.75f, -1.0f, -0.25f, 0.25f);
	enum DebugZoomLevel { kDebugZoomOff, kDebugZoom2x, kDebugZoom4x };
	const char* DebugZoomLabels[] = { "Off", "2x Zoom", "4x Zoom" };
	EnumVar DebugZoom("Graphics/Scaling/Magnify Pixels", kDebugZoomOff, 3, DebugZoomLabels);

	enum TargetResolution { k1080p, k900p, k720p };
	const char* ResolutionLabels[] = { "1920x1080", "1600x900", "1280x720" };
	EnumVar TargetResolution("Graphics/Target Resolution", k1080p, 3, ResolutionLabels);

	void PreparePresent( GraphicsContext& Context );
}


// Initialize the DirectX resources required to run.
void Graphics::Initialize( uint32_t width, uint32_t height )
{
	// If the swap chain already exists, resize it, otherwise create one.
	if (s_PrimarySwapChain != nullptr)
	{
		ASSERT_SUCCEEDED(s_PrimarySwapChain->ResizeBuffers(SWAP_CHAIN_BUFFER_COUNT, width, height, SwapChainFormat, 0));
	}
	else
	{
		Microsoft::WRL::ComPtr<ID3D12Device> device;

#if _DEBUG
		Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface;
		if (SUCCEEDED(D3D12GetDebugInterface(MY_IID_PPV_ARGS(&debugInterface))))
			debugInterface->EnableDebugLayer();
		else
			Utility::Print("WARNING:  Unable to enable D3D12 debug validation layer\n");
#endif

		// And obtain the factory object that created it.
		Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;
		ASSERT_SUCCEEDED(CreateDXGIFactory(MY_IID_PPV_ARGS(&dxgiFactory)));

		static const bool bUseWarpDriver = false;

		// Create the D3D graphics device
		if (bUseWarpDriver)
		{
			Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
			Microsoft::WRL::ComPtr<IDXGIFactory4> pFactory;
			ASSERT_SUCCEEDED(CreateDXGIFactory2(0, IID_PPV_ARGS(&pFactory)));
			ASSERT_SUCCEEDED(pFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));
			ASSERT_SUCCEEDED(D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, MY_IID_PPV_ARGS(&device)));
		}
		else
		{
			ASSERT_SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, MY_IID_PPV_ARGS(&device)));
		}

		g_Device = device.Detach();

#if _DEBUG
		ID3D12InfoQueue* pInfoQueue = nullptr;
		if (SUCCEEDED(g_Device->QueryInterface(MY_IID_PPV_ARGS(&pInfoQueue))))
		{
			// Suppress whole categories of messages
			//D3D12_MESSAGE_CATEGORY Categories[] = {};

			// Suppress messages based on their severity level
			D3D12_MESSAGE_SEVERITY Severities[] = 
			{
				D3D12_MESSAGE_SEVERITY_INFO
			};

			// Suppress individual messages by their ID
			//D3D12_MESSAGE_ID DenyIds[] = {};

			D3D12_INFO_QUEUE_FILTER NewFilter = {};
			//NewFilter.DenyList.NumCategories = _countof(Categories);
			//NewFilter.DenyList.pCategoryList = Categories;
			NewFilter.DenyList.NumSeverities = _countof(Severities);
			NewFilter.DenyList.pSeverityList = Severities;
			//NewFilter.DenyList.NumIDs = _countof(DenyIds);
			//NewFilter.DenyList.pIDList = DenyIds;

			pInfoQueue->PushStorageFilter(&NewFilter);
			pInfoQueue->Release();
		}
#endif

		g_CommandManager.Create(g_Device);

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferDesc.Width = 1920;
		swapChainDesc.BufferDesc.Height = 1080;
		swapChainDesc.BufferDesc.Format = SwapChainFormat;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;// | DXGI_USAGE_UNORDERED_ACCESS;
		swapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		swapChainDesc.OutputWindow = GameCore::g_hWnd;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		ASSERT_SUCCEEDED(dxgiFactory->CreateSwapChain( g_CommandManager.GetCommandQueue(), &swapChainDesc, &s_PrimarySwapChain ));
	}

	for (uint32_t i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
	{
		ComPtr<ID3D12Resource> DisplayPlane;
		ASSERT_SUCCEEDED(s_PrimarySwapChain->GetBuffer(i, MY_IID_PPV_ARGS(&DisplayPlane)));
		g_DisplayPlane[i].CreateFromSwapChain(L"Primary SwapChain Buffer", DisplayPlane.Detach());
	}

	D3D12_SAMPLER_DESC SamplerDesc = {};

	SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU = SamplerDesc.AddressV = SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	SamplerDesc.MinLOD = 0;
	SamplerDesc.MaxLOD = FLT_MAX;
	SamplerLinearWrap.Create(SamplerLinearWrapDesc = SamplerDesc);

	SamplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
	SamplerDesc.AddressU = SamplerDesc.AddressV = SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	SamplerDesc.MaxAnisotropy = 8;
	SamplerDesc.MinLOD = 0;
	SamplerDesc.MaxLOD = FLT_MAX;
	SamplerAnisoWrap.Create(SamplerAnisoWrapDesc = SamplerDesc);

	ZeroMemory(&SamplerDesc, sizeof(SamplerDesc));
	SamplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	SamplerDesc.AddressU = SamplerDesc.AddressV = SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	SamplerShadow.Create(SamplerShadowDesc = SamplerDesc);

	SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU = SamplerDesc.AddressV = SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	SamplerDesc.MinLOD = -FLT_MAX;
	SamplerDesc.MaxLOD = FLT_MAX;
	SamplerDesc.MipLODBias = 0.0f;
	SamplerDesc.MaxAnisotropy = 16;
	SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	SamplerLinearClamp.Create(SamplerLinearClampDesc = SamplerDesc);

	SamplerDesc.AddressU = SamplerDesc.AddressV = SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	SamplerVolumeWrap.Create(SamplerVolumeWrapDesc = SamplerDesc);

	SamplerDesc.AddressU = SamplerDesc.AddressV = SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	SamplerPointClamp.Create(SamplerPointClampDesc = SamplerDesc);

	// Samplers
	ZeroMemory(&SamplerDesc, sizeof(SamplerDesc));
	SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	SamplerDesc.AddressU = SamplerDesc.AddressV = SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	SamplerDesc.BorderColor[0] = SamplerDesc.BorderColor[1] = SamplerDesc.BorderColor[2] = SamplerDesc.BorderColor[3] = 0.0f;
	SamplerLinearBorder.Create(SamplerLinearBorderDesc = SamplerDesc);

	SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	SamplerPointBorder.Create(SamplerPointBorderDesc = SamplerDesc);

	// Default rasterizer states
	RasterizerDefault.FillMode = D3D12_FILL_MODE_SOLID;
	RasterizerDefault.CullMode = D3D12_CULL_MODE_BACK;
	RasterizerDefault.FrontCounterClockwise = TRUE;
	RasterizerDefault.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	RasterizerDefault.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	RasterizerDefault.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	RasterizerDefault.DepthClipEnable = TRUE;
	RasterizerDefault.MultisampleEnable = FALSE;
	RasterizerDefault.AntialiasedLineEnable = FALSE;
	RasterizerDefault.ForcedSampleCount = 0;
	RasterizerDefault.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	RasterizerDefaultCW = RasterizerDefault;
	RasterizerDefaultCW.FrontCounterClockwise = FALSE;

	RasterizerTwoSided = RasterizerDefault;
	RasterizerTwoSided.CullMode = D3D12_CULL_MODE_NONE;

	// Shadows need their own rasterizer state so we can reverse the winding of faces
	RasterizerShadow = RasterizerDefault;
	//RasterizerShadow.CullMode = D3D12_CULL_FRONT;  // Hacked here rather than fixing the content
	RasterizerShadow.SlopeScaledDepthBias = -1.5f;
	RasterizerShadow.DepthBias = -100;

	RasterizerShadowCW = RasterizerShadow;
	RasterizerShadowCW.FrontCounterClockwise = FALSE;

	DepthStateDisabled.DepthEnable = FALSE;
	DepthStateDisabled.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	DepthStateDisabled.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	DepthStateDisabled.StencilEnable = FALSE;
	DepthStateDisabled.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	DepthStateDisabled.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	DepthStateDisabled.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	DepthStateDisabled.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	DepthStateDisabled.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	DepthStateDisabled.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	DepthStateDisabled.BackFace = DepthStateDisabled.FrontFace;

	DepthStateReadWrite = DepthStateDisabled;
	DepthStateReadWrite.DepthEnable = TRUE;
	DepthStateReadWrite.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	DepthStateReadWrite.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;

	DepthStateReadOnly = DepthStateReadWrite;
	DepthStateReadOnly.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

	DepthStateReadOnlyReversed = DepthStateReadOnly;
	DepthStateReadOnlyReversed.DepthFunc = D3D12_COMPARISON_FUNC_LESS;

	DepthStateTestEqual = DepthStateReadOnly;
	DepthStateTestEqual.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;

	D3D12_BLEND_DESC alphaBlend = {};
	alphaBlend.IndependentBlendEnable = FALSE;
	alphaBlend.RenderTarget[0].BlendEnable = FALSE;
	alphaBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	alphaBlend.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	alphaBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	alphaBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	alphaBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
	alphaBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	alphaBlend.RenderTarget[0].RenderTargetWriteMask = 0;
	BlendNoColorWrite = alphaBlend;

	alphaBlend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	BlendDisable = alphaBlend;

	alphaBlend.RenderTarget[0].BlendEnable = TRUE;
	BlendTraditional = alphaBlend;

	alphaBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	BlendPreMultiplied = alphaBlend;

	alphaBlend.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	BlendAdditive = alphaBlend;

	alphaBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	BlendTraditionalAdditive = alphaBlend;

	DispatchIndirectCommandSignature[0].Dispatch();
	DispatchIndirectCommandSignature.Finalize();

	DrawIndirectCommandSignature[0].Draw();
	DrawIndirectCommandSignature.Finalize();

	s_PresentRS.Reset(2, 1);
	s_PresentRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 2, D3D12_SHADER_VISIBILITY_PIXEL);
	s_PresentRS[1].InitAsConstants(0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	s_PresentRS.InitStaticSampler(0, SamplerLinearClampDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	s_PresentRS.Finalize();

	// Initialize PSOs
	s_BlendUIPSO.SetRootSignature(s_PresentRS);
	s_BlendUIPSO.SetRasterizerState( RasterizerTwoSided );
	s_BlendUIPSO.SetBlendState( BlendPreMultiplied );
	s_BlendUIPSO.SetDepthStencilState( DepthStateDisabled );
	s_BlendUIPSO.SetSampleMask(0xFFFFFFFF);
	s_BlendUIPSO.SetInputLayout(0, nullptr);
	s_BlendUIPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	s_BlendUIPSO.SetVertexShader( g_pScreenQuadVS, sizeof(g_pScreenQuadVS) );
	s_BlendUIPSO.SetPixelShader( g_pBufferCopyPS, sizeof(g_pBufferCopyPS) );
	s_BlendUIPSO.SetRenderTargetFormat(SwapChainFormat, DXGI_FORMAT_UNKNOWN);
	s_BlendUIPSO.Finalize();

#define CreatePSO( ObjName, ShaderByteCode ) \
	ObjName = s_BlendUIPSO; \
	ObjName.SetBlendState( BlendDisable ); \
	ObjName.SetPixelShader(ShaderByteCode, sizeof(ShaderByteCode) ); \
	ObjName.Finalize();

	CreatePSO(ConvertLDRToDisplayPS, g_pConvertLDRToDisplayPS);
	CreatePSO(DebugZoom2xPS, g_pDebugZoom2xPS);
	CreatePSO(DebugZoom4xPS, g_pDebugZoom4xPS);
	CreatePSO(BilinearUpsamplePS, g_pBilinearUpsamplePS);
	CreatePSO(BicubicUpsamplePS, g_pBicubicUpsamplePS);

	g_GenerateMipsRS.Reset(3, 1);
	g_GenerateMipsRS[0].InitAsConstants(0, 4);
	g_GenerateMipsRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
	g_GenerateMipsRS[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 4);
	g_GenerateMipsRS.InitStaticSampler(0, SamplerLinearClampDesc);
	g_GenerateMipsRS.Finalize();

	g_GenerateMipsPSO.SetRootSignature(g_GenerateMipsRS);
	g_GenerateMipsPSO.SetComputeShader(g_pGenerateMipsCS, sizeof(g_pGenerateMipsCS));
	g_GenerateMipsPSO.Finalize();

	g_GenerateMipsGammaPSO.SetRootSignature(g_GenerateMipsRS);
	g_GenerateMipsGammaPSO.SetComputeShader(g_pGenerateMipsGammaCS, sizeof(g_pGenerateMipsGammaCS));
	g_GenerateMipsGammaPSO.Finalize();

	GpuTimeManager::Initialize(4096);
	InitializeRenderingBuffers( width, height );
	PostEffects::Initialize();
	SSAO::Initialize();
	TextRenderer::Initialize();
	ParticleEffects::Initialize(1920, 1080);
}

void Graphics::Terminate(void)
{
	g_CommandManager.IdleGPU();
}

void Graphics::Shutdown(void)
{
	CommandContext::DestroyAllContexts();
	g_CommandManager.Shutdown();
	GpuTimeManager::Shutdown();
	s_PrimarySwapChain->Release();
	PSO::DestroyAll();
	RootSignature::DestroyAll();

	DispatchIndirectCommandSignature.Destroy();
	DrawIndirectCommandSignature.Destroy();
	DescriptorAllocator::DestroyAll();

	DestroyRenderingBuffers();
	PostEffects::Shutdown();
	SSAO::Shutdown();
	TextRenderer::Shutdown();
	ParticleEffects::Shutdown();
	TextureManager::Shutdown();

	for (UINT i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
		g_DisplayPlane[i].Destroy();

#ifdef _DEBUG
	ID3D12DebugDevice* debugInterface;
	if (SUCCEEDED(g_Device->QueryInterface(&debugInterface)))
	{
		debugInterface->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
		debugInterface->Release();
	}
#endif

	SAFE_RELEASE(g_Device);
}

void Graphics::PreparePresent( GraphicsContext& Context )
{
	ScopedTimer _prof(L"Present", Context);

	// We're going to be reading these buffers to write to the swap chain buffer(s)
	Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	Context.TransitionResource(g_OverlayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	Context.SetRootSignature(s_PresentRS);
	Context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Copy (and convert) the LDR buffer to the back buffer

	Context.SetRenderTarget(g_DisplayPlane[g_CurrentBuffer]);
	Context.SetDynamicDescriptor(0, 0, g_SceneColorBuffer.GetSRV());

	if (DebugZoom != kDebugZoomOff)
	{
		Context.SetPipelineState(DebugZoom == kDebugZoom4x ? DebugZoom4xPS : DebugZoom2xPS);
		Context.SetViewportAndScissor(0, 0, 1920, 1080);
	}
	else if (g_windowHeight == 1080)
	{
		Context.SetPipelineState(ConvertLDRToDisplayPS);
		Context.SetViewportAndScissor(0, 0, g_windowWidth, g_windowHeight);
	}
	else if (BicubicUpsample)
	{
		Context.SetPipelineState(BicubicUpsamplePS);
		Context.SetViewportAndScissor(0, 0, 1920, 1080);
		Context.SetConstants(1, (float)BicubicUpsampleWeight);
	}
	else
	{
		Context.SetPipelineState(BilinearUpsamplePS);
		Context.SetViewportAndScissor(0, 0, 1920, 1080);
	}
	Context.Draw(3);

	// Now blend (or write) the UI overlay
	Context.SetDynamicDescriptor(0, 0, g_OverlayBuffer.GetSRV());

	Context.SetPipelineState(s_BlendUIPSO);
	Context.Draw(3);
	Context.TransitionResource(g_DisplayPlane[g_CurrentBuffer], D3D12_RESOURCE_STATE_PRESENT);
}

void Graphics::Present( void )
{
	GraphicsContext& Context = GraphicsContext::Begin();

	// This marks the end of the frame for profiling purposes so that total time does not include the stall
	// in Present()
	EngineProfiling::EndFrame(Context);

	PreparePresent(Context);

	// After the last time stamp has been inserted, trigger a readback from the query heap, and insert
	// the sentinal time stamp.
	GpuTimeManager::EndFrame(Context);

	// Close the final context to be executed before frame present.
	Context.CloseAndExecute();

	UINT PresentInterval = SystemTime::EnableVSync ? min(4, (UINT)Round(SystemTime::FrameTime * 60.0f)) : 0;

	s_PrimarySwapChain->Present(PresentInterval, 0);

	g_CurrentBuffer = (g_CurrentBuffer + 1) % SWAP_CHAIN_BUFFER_COUNT;

	

	switch (TargetResolution)
	{
	case k1080p:
		if (g_windowHeight == 1080)
			return;

		g_windowWidth = 1920;
		g_windowHeight = 1080;
		break;

	case k900p:
		if (g_windowHeight == 900)
			return;

		g_windowWidth = 1600;
		g_windowHeight = 900;
		break;

	case k720p:
		if (g_windowHeight == 720)
			return;

		g_windowWidth = 1280;
		g_windowHeight = 720;
		break;

	default:
		break;
	}

	g_CommandManager.IdleGPU();

	// We won't get here unless the resolution changed.
	InitializeRenderingBuffers(g_windowWidth, g_windowHeight);
}
