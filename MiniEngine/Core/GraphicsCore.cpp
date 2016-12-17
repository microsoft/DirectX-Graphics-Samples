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
#include "GraphRenderer.h"

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	#include <agile.h>
#endif

#include <dxgi1_4.h>	// For WARP

#include "CompiledShaders/ScreenQuadVS.h"
#include "CompiledShaders/BufferCopyPS.h"
#include "CompiledShaders/ConvertLDRToDisplayPS.h"
#include "CompiledShaders/ConvertHDRToDisplayPS.h"
#include "CompiledShaders/MagnifyPixelsPS.h"
#include "CompiledShaders/BilinearUpsamplePS.h"
#include "CompiledShaders/BicubicHorizontalUpsamplePS.h"
#include "CompiledShaders/BicubicVerticalUpsamplePS.h"
#include "CompiledShaders/SharpeningUpsamplePS.h"
#include "CompiledShaders/GenerateMipsLinearCS.h"
#include "CompiledShaders/GenerateMipsLinearOddCS.h"
#include "CompiledShaders/GenerateMipsLinearOddXCS.h"
#include "CompiledShaders/GenerateMipsLinearOddYCS.h"
#include "CompiledShaders/GenerateMipsGammaCS.h"
#include "CompiledShaders/GenerateMipsGammaOddCS.h"
#include "CompiledShaders/GenerateMipsGammaOddXCS.h"
#include "CompiledShaders/GenerateMipsGammaOddYCS.h"

#define SWAP_CHAIN_BUFFER_COUNT 3

DXGI_FORMAT SwapChainFormat = DXGI_FORMAT_R10G10B10A2_UNORM;

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if (x != nullptr) { x->Release(); x = nullptr; }
#endif

using namespace Math;

namespace GameCore
{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	extern HWND g_hWnd;
#else
	extern Platform::Agile<Windows::UI::Core::CoreWindow>  g_window;
#endif
}

namespace
{
	float s_FrameTime = 0.0f;
	uint64_t s_FrameIndex = 0;
	int64_t s_FrameStartTick = 0;

	BoolVar s_EnableVSync("Timing/VSync", true);
	BoolVar s_LimitTo30Hz("Timing/Limit To 30Hz", false);
	BoolVar s_DropRandomFrames("Timing/Drop Random Frames", false);
}

namespace Graphics
{
	void PreparePresentLDR();
	void PreparePresentHDR();

#ifndef RELEASE
	const GUID WKPDID_D3DDebugObjectName = { 0x429b8c22,0x9188,0x4b0c, { 0x87,0x42,0xac,0xb0,0xbf,0x85,0xc2,0x00 }};
#endif

	enum eResolution { k720p, k900p, k1080p, k1440p, k2160p };

	const uint32_t kMaxNativeWidth = 3840;
	const uint32_t kMaxNativeHeight = 2160;
	const uint32_t kNumPredefinedResolutions = 5;

	const char* ResolutionLabels[] = { "1280x720", "1600x900", "1920x1080", "2560x1440", "3840x2160" };
	EnumVar TargetResolution("Graphics/Display/Native Resolution", k1080p, kNumPredefinedResolutions, ResolutionLabels);

	bool g_bTypedUAVLoadSupport_R11G11B10_FLOAT = false;
	bool g_bEnableHDROutput = false;
	NumVar g_HDRPaperWhite("Graphics/Display/Paper White (nits)", 200.0f, 80.0f, 520.0f, 40.0f);
	NumVar g_MaxDisplayLuminance("Graphics/Display/Peak Brightness (nits)", 600.0f, 400.0f, 2000.0f, 50.0f);
	const char* HDRModeLabels[] = { "HDR", "LDR", "Side-by-Side" };
	EnumVar HDRDebugMode("Graphics/Display/HDR Debug Mode", 0, 3, HDRModeLabels);

	uint32_t g_NativeWidth = 1920;
	uint32_t g_NativeHeight = 1080;
	uint32_t g_DisplayWidth = 1920;
	uint32_t g_DisplayHeight = 1080;
	ColorBuffer g_PreDisplayBuffer;

	void SetNativeResolution(void)
	{
		uint32_t NativeWidth, NativeHeight;

		switch (eResolution((int)TargetResolution))
		{
		case k720p:
			NativeWidth = 1280;
			NativeHeight = 720;
			break;
		case k900p:
			NativeWidth = 1600;
			NativeHeight = 900;
			break;
		case k1080p:
			NativeWidth = 1920;
			NativeHeight = 1080;
			break;
		case k1440p:
			NativeWidth = 2560;
			NativeHeight = 1440;
			break;
		case k2160p:
			NativeWidth = 3840;
			NativeHeight = 2160;
			break;
		}

		if (g_NativeWidth == NativeWidth && g_NativeHeight == NativeHeight)
			return;

		DEBUGPRINT("Changing native resolution to %ux%u", NativeWidth, NativeHeight);

		g_NativeWidth = NativeWidth;
		g_NativeHeight = NativeHeight;

		g_CommandManager.IdleGPU();

		InitializeRenderingBuffers(NativeWidth, NativeHeight);
	}

	ID3D12Device* g_Device = nullptr;

	CommandListManager g_CommandManager;
	ContextManager g_ContextManager;

	D3D_FEATURE_LEVEL g_D3DFeatureLevel = D3D_FEATURE_LEVEL_11_0;

	ColorBuffer g_DisplayPlane[SWAP_CHAIN_BUFFER_COUNT];
	UINT g_CurrentBuffer = 0;

	IDXGISwapChain1* s_SwapChain1 = nullptr;

	DescriptorAllocator g_DescriptorAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
	{
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
	};

	SamplerDesc SamplerLinearWrapDesc;
	SamplerDesc SamplerAnisoWrapDesc;
	SamplerDesc SamplerShadowDesc;
	SamplerDesc SamplerLinearClampDesc;
	SamplerDesc SamplerVolumeWrapDesc;
	SamplerDesc SamplerPointClampDesc;
	SamplerDesc SamplerPointBorderDesc;
	SamplerDesc SamplerLinearBorderDesc;

	SamplerDescriptor SamplerLinearWrap;
	SamplerDescriptor SamplerAnisoWrap;
	SamplerDescriptor SamplerShadow;
	SamplerDescriptor SamplerLinearClamp;
	SamplerDescriptor SamplerVolumeWrap;
	SamplerDescriptor SamplerPointClamp;
	SamplerDescriptor SamplerPointBorder;
	SamplerDescriptor SamplerLinearBorder;

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
	GraphicsPSO ConvertHDRToDisplayPS;
	GraphicsPSO MagnifyPixelsPS;
	GraphicsPSO SharpeningUpsamplePS;
	GraphicsPSO BicubicHorizontalUpsamplePS;
	GraphicsPSO BicubicVerticalUpsamplePS;
	GraphicsPSO BilinearUpsamplePS;

	RootSignature g_GenerateMipsRS;
	ComputePSO g_GenerateMipsLinearPSO[4];
	ComputePSO g_GenerateMipsGammaPSO[4];

	enum { kBilinear, kBicubic, kSharpening, kFilterCount };
	const char* FilterLabels[] = { "Bilinear", "Bicubic", "Sharpening" };
	EnumVar UpsampleFilter("Graphics/Display/Upsample Filter", kFilterCount - 1, kFilterCount, FilterLabels);
	NumVar BicubicUpsampleWeight("Graphics/Display/Bicubic Filter Weight", -0.75f, -1.0f, -0.25f, 0.25f);
	NumVar SharpeningSpread("Graphics/Display/Sharpness Sample Spread", 1.0f, 0.7f, 2.0f, 0.1f);
	NumVar SharpeningRotation("Graphics/Display/Sharpness Sample Rotation", 45.0f, 0.0f, 90.0f, 15.0f);
	NumVar SharpeningStrength("Graphics/Display/Sharpness Strength", 0.10f, 0.0f, 1.0f, 0.01f);

	enum DebugZoomLevel { kDebugZoomOff, kDebugZoom2x, kDebugZoom4x };
	const char* DebugZoomLabels[] = { "Off", "2x Zoom", "4x Zoom" };
	EnumVar DebugZoom("Graphics/Display/Magnify Pixels", kDebugZoomOff, 3, DebugZoomLabels);
}

void Graphics::Resize(uint32_t width, uint32_t height)
{
	ASSERT(s_SwapChain1 != nullptr);

	g_CommandManager.IdleGPU();

	g_DisplayWidth = width;
	g_DisplayHeight = height;

	DEBUGPRINT("Changing display resolution to %ux%u", width, height);

	g_PreDisplayBuffer.Create(L"PreDisplay Buffer", width, height, 1, SwapChainFormat);

	for (uint32_t i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
		g_DisplayPlane[i].Destroy();

	ASSERT_SUCCEEDED(s_SwapChain1->ResizeBuffers(SWAP_CHAIN_BUFFER_COUNT, width, height, SwapChainFormat, 0));

	for (uint32_t i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
	{
		ComPtr<ID3D12Resource> DisplayPlane;
		ASSERT_SUCCEEDED(s_SwapChain1->GetBuffer(i, MY_IID_PPV_ARGS(&DisplayPlane)));
		g_DisplayPlane[i].CreateFromSwapChain(L"Primary SwapChain Buffer", DisplayPlane.Detach());
	}

	g_CurrentBuffer = 0;
}

// Initialize the DirectX resources required to run.
void Graphics::Initialize(void)
{
	ASSERT(s_SwapChain1 == nullptr, "Graphics has already been initialized");

	Microsoft::WRL::ComPtr<ID3D12Device> pDevice;

#if _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface;
	if (SUCCEEDED(D3D12GetDebugInterface(MY_IID_PPV_ARGS(&debugInterface))))
		debugInterface->EnableDebugLayer();
	else
		Utility::Print("WARNING:  Unable to enable D3D12 debug validation layer\n");
#endif

	// Obtain the DXGI factory
	Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
	ASSERT_SUCCEEDED(CreateDXGIFactory2(0, MY_IID_PPV_ARGS(&dxgiFactory)));

	// Create the D3D graphics device
	Microsoft::WRL::ComPtr<IDXGIAdapter1> pAdapter;

	static const bool bUseWarpDriver = false;

	if (!bUseWarpDriver)
	{
		for (uint32_t Idx = 0; DXGI_ERROR_NOT_FOUND != dxgiFactory->EnumAdapters1(Idx, &pAdapter); ++Idx)
		{
			DXGI_ADAPTER_DESC1 desc;
			pAdapter->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;

			if (SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, MY_IID_PPV_ARGS(&pDevice))))
			{
				pAdapter->GetDesc1(&desc);
				Utility::Printf(L"D3D12-capable hardware found:  %s (%u MB)\n", desc.Description, desc.DedicatedVideoMemory >> 20);
				g_Device = pDevice.Detach();
				break;
			}
		}
	}

	if (g_Device == nullptr)
	{
		if (bUseWarpDriver)
			Utility::Print("WARP software adapter requested.  Initializing...\n");
		else
			Utility::Print("Failed to find a hardware adapter.  Falling back to WARP.\n");
		ASSERT_SUCCEEDED(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pAdapter)));
		ASSERT_SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, MY_IID_PPV_ARGS(&pDevice)));
		g_Device = pDevice.Detach();
	}
	
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
		D3D12_MESSAGE_ID DenyIds[] =
		{
			// This occurs when there are uninitialized descriptors in a descriptor table, even when a
			// shader does not access the missing descriptors.  I find this is common when switching
			// shader permutations and not wanting to change much code to reorder resources.
			D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,

			// Triggered when a shader does not export all color components of a render target, such as
			// when only writing RGB to an R10G10B10A2 buffer, ignoring alpha.
			D3D12_MESSAGE_ID_CREATEGRAPHICSPIPELINESTATE_PS_OUTPUT_RT_OUTPUT_MISMATCH,

			// This occurs when a descriptor table is unbound even when a shader does not access the missing
			// descriptors.  This is common with a root signature shared between disparate shaders that
			// don't all need the same types of resources.
			D3D12_MESSAGE_ID_COMMAND_LIST_DESCRIPTOR_TABLE_NOT_SET
		};

		D3D12_INFO_QUEUE_FILTER NewFilter = {};
		//NewFilter.DenyList.NumCategories = _countof(Categories);
		//NewFilter.DenyList.pCategoryList = Categories;
		NewFilter.DenyList.NumSeverities = _countof(Severities);
		NewFilter.DenyList.pSeverityList = Severities;
		NewFilter.DenyList.NumIDs = _countof(DenyIds);
		NewFilter.DenyList.pIDList = DenyIds;

		pInfoQueue->PushStorageFilter(&NewFilter);
		pInfoQueue->Release();
	}
#endif

	// We like to do read-modify-write operations on UAVs during post processing.  To support that, we
	// need to either have the hardware do typed UAV loads of R11G11B10_FLOAT or we need to manually
	// decode an R32_UINT representation of the same buffer.  This code determines if we get the hardware
	// load support.
	D3D12_FEATURE_DATA_D3D12_OPTIONS FeatureData = {};
	if (SUCCEEDED(g_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &FeatureData, sizeof(FeatureData))))
	{
		if (FeatureData.TypedUAVLoadAdditionalFormats)
		{
			D3D12_FEATURE_DATA_FORMAT_SUPPORT Support =
			{
				DXGI_FORMAT_R11G11B10_FLOAT, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE
			};

			if (SUCCEEDED(g_Device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &Support, sizeof(Support))) &&
				(Support.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
			{
				g_bTypedUAVLoadSupport_R11G11B10_FLOAT = true;
			}
		}
	}

	g_CommandManager.Create(g_Device);

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = g_DisplayWidth;
	swapChainDesc.Height = g_DisplayHeight;
	swapChainDesc.Format = SwapChainFormat;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	ASSERT_SUCCEEDED(dxgiFactory->CreateSwapChainForHwnd(g_CommandManager.GetCommandQueue(), GameCore::g_hWnd, &swapChainDesc, nullptr, nullptr, &s_SwapChain1));
#else
	ASSERT_SUCCEEDED(dxgiFactory->CreateSwapChainForCoreWindow(g_CommandManager.GetCommandQueue(), (IUnknown*)GameCore::g_window.Get(), &swapChainDesc, nullptr, &s_SwapChain1));
#endif

	for (uint32_t i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
	{
		ComPtr<ID3D12Resource> DisplayPlane;
		ASSERT_SUCCEEDED(s_SwapChain1->GetBuffer(i, MY_IID_PPV_ARGS(&DisplayPlane)));
		g_DisplayPlane[i].CreateFromSwapChain(L"Primary SwapChain Buffer", DisplayPlane.Detach());
	}

	SamplerLinearWrapDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerLinearWrap.Create(SamplerLinearWrapDesc);

	SamplerAnisoWrapDesc.MaxAnisotropy = 8;
	SamplerAnisoWrap.Create(SamplerAnisoWrapDesc);

	SamplerShadowDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	SamplerShadowDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	SamplerShadowDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	SamplerShadow.Create(SamplerShadowDesc);

	SamplerLinearClampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerLinearClampDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	SamplerLinearClamp.Create(SamplerLinearClampDesc);

	SamplerVolumeWrapDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	SamplerVolumeWrap.Create(SamplerVolumeWrapDesc);

	SamplerPointClampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	SamplerPointClampDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	SamplerPointClamp.Create(SamplerPointClampDesc);

	SamplerLinearBorderDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerLinearBorderDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_BORDER);
	SamplerLinearBorderDesc.SetBorderColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
	SamplerLinearBorder.Create(SamplerLinearBorderDesc);

	SamplerPointBorderDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	SamplerPointBorderDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_BORDER);
	SamplerPointBorderDesc.SetBorderColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
	SamplerPointBorder.Create(SamplerPointBorderDesc);

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

	s_PresentRS.Reset(4, 2);
	s_PresentRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 2);
	s_PresentRS[1].InitAsConstants(0, 6, D3D12_SHADER_VISIBILITY_PIXEL);
	s_PresentRS[2].InitAsBufferSRV(2, D3D12_SHADER_VISIBILITY_PIXEL);
	s_PresentRS[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
	s_PresentRS.InitStaticSampler(0, SamplerLinearClampDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	s_PresentRS.InitStaticSampler(1, SamplerPointClampDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	s_PresentRS.Finalize(L"Present");

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
	CreatePSO(ConvertHDRToDisplayPS, g_pConvertHDRToDisplayPS);
	CreatePSO(MagnifyPixelsPS, g_pMagnifyPixelsPS);
	CreatePSO(BilinearUpsamplePS, g_pBilinearUpsamplePS);
	CreatePSO(BicubicHorizontalUpsamplePS, g_pBicubicHorizontalUpsamplePS);
	CreatePSO(BicubicVerticalUpsamplePS, g_pBicubicVerticalUpsamplePS);
	CreatePSO(SharpeningUpsamplePS, g_pSharpeningUpsamplePS);

#undef CreatePSO

	g_GenerateMipsRS.Reset(3, 1);
	g_GenerateMipsRS[0].InitAsConstants(0, 4);
	g_GenerateMipsRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
	g_GenerateMipsRS[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 4);
	g_GenerateMipsRS.InitStaticSampler(0, SamplerLinearClampDesc);
	g_GenerateMipsRS.Finalize(L"Generate Mips");

#define CreatePSO(ObjName, ShaderByteCode ) \
	ObjName.SetRootSignature(g_GenerateMipsRS); \
	ObjName.SetComputeShader(ShaderByteCode, sizeof(ShaderByteCode) ); \
	ObjName.Finalize();

	CreatePSO(g_GenerateMipsLinearPSO[0], g_pGenerateMipsLinearCS);
	CreatePSO(g_GenerateMipsLinearPSO[1], g_pGenerateMipsLinearOddXCS);
	CreatePSO(g_GenerateMipsLinearPSO[2], g_pGenerateMipsLinearOddYCS);
	CreatePSO(g_GenerateMipsLinearPSO[3], g_pGenerateMipsLinearOddCS);
	CreatePSO(g_GenerateMipsGammaPSO[0], g_pGenerateMipsGammaCS);
	CreatePSO(g_GenerateMipsGammaPSO[1], g_pGenerateMipsGammaOddXCS);
	CreatePSO(g_GenerateMipsGammaPSO[2], g_pGenerateMipsGammaOddYCS);
	CreatePSO(g_GenerateMipsGammaPSO[3], g_pGenerateMipsGammaOddCS);

	g_PreDisplayBuffer.Create(L"PreDisplay Buffer", g_DisplayWidth, g_DisplayHeight, 1, SwapChainFormat);

	GpuTimeManager::Initialize(4096);
	InitializeRenderingBuffers(g_NativeWidth, g_NativeHeight);
	PostEffects::Initialize();
	SSAO::Initialize();
	TextRenderer::Initialize();
	GraphRenderer::Initialize();
	ParticleEffects::Initialize(kMaxNativeWidth, kMaxNativeHeight);
}

void Graphics::Terminate( void )
{
	g_CommandManager.IdleGPU();
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	s_SwapChain1->SetFullscreenState(FALSE, nullptr);
#endif
}

void Graphics::Shutdown( void )
{
	CommandContext::DestroyAllContexts();
	g_CommandManager.Shutdown();
	GpuTimeManager::Shutdown();
	s_SwapChain1->Release();
	PSO::DestroyAll();
	RootSignature::DestroyAll();

	DispatchIndirectCommandSignature.Destroy();
	DrawIndirectCommandSignature.Destroy();
	DescriptorAllocator::DestroyAll();

	DestroyRenderingBuffers();
	PostEffects::Shutdown();
	SSAO::Shutdown();
	TextRenderer::Shutdown();
	GraphRenderer::Shutdown();
	ParticleEffects::Shutdown();
	TextureManager::Shutdown();

	for (UINT i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
		g_DisplayPlane[i].Destroy();

	g_PreDisplayBuffer.Destroy();

#if defined(_DEBUG)
	ID3D12DebugDevice* debugInterface;
	if (SUCCEEDED(g_Device->QueryInterface(&debugInterface)))
	{
		debugInterface->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
		debugInterface->Release();
	}
#endif

	SAFE_RELEASE(g_Device);
}

void Graphics::PreparePresentHDR(void)
{
	GraphicsContext& Context = GraphicsContext::Begin(L"Present");

	// We're going to be reading these buffers to write to the swap chain buffer(s)
	Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	Context.TransitionResource(g_OverlayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	Context.TransitionResource(g_DisplayPlane[g_CurrentBuffer], D3D12_RESOURCE_STATE_RENDER_TARGET);

	Context.SetRootSignature(s_PresentRS);
	Context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Context.SetDynamicDescriptor(0, 0, g_SceneColorBuffer.GetSRV());
	Context.SetDynamicDescriptor(0, 1, g_OverlayBuffer.GetSRV());

	D3D12_CPU_DESCRIPTOR_HANDLE RTVs[] =
	{
		g_DisplayPlane[g_CurrentBuffer].GetRTV()
	};

	float ToeStrength = PostEffects::g_ToeStrength;
	ToeStrength = ToeStrength < 1e-6f ? 1e32f : 1.0f / ToeStrength;

	Context.SetPipelineState(ConvertHDRToDisplayPS);
	Context.SetRenderTargets(_countof(RTVs), RTVs);
	Context.SetViewportAndScissor(0, 0, g_NativeWidth, g_NativeHeight);
	Context.SetConstants(1, (float)g_HDRPaperWhite, (float)g_MaxDisplayLuminance, ToeStrength, (int)HDRDebugMode);
	Context.Draw(3);

	Context.TransitionResource(g_DisplayPlane[g_CurrentBuffer], D3D12_RESOURCE_STATE_PRESENT);

	// Close the final context to be executed before frame present.
	Context.Finish();
}

void Graphics::PreparePresentLDR(void)
{
	GraphicsContext& Context = GraphicsContext::Begin(L"Present");

	// We're going to be reading these buffers to write to the swap chain buffer(s)
	Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	Context.SetRootSignature(s_PresentRS);
	Context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Copy (and convert) the LDR buffer to the back buffer

	Context.SetDynamicDescriptor(0, 0, g_SceneColorBuffer.GetSRV());

	ColorBuffer& UpsampleDest = (DebugZoom == kDebugZoomOff ? g_DisplayPlane[g_CurrentBuffer] : g_PreDisplayBuffer);

	if (g_NativeWidth == g_DisplayWidth && g_NativeHeight == g_DisplayHeight)
	{
		Context.SetPipelineState(ConvertLDRToDisplayPS);
		Context.TransitionResource(UpsampleDest, D3D12_RESOURCE_STATE_RENDER_TARGET);
		Context.SetRenderTarget(UpsampleDest.GetRTV());
		Context.SetViewportAndScissor(0, 0, g_NativeWidth, g_NativeHeight);
		Context.Draw(3);
	}
	else if (UpsampleFilter == kBicubic)
	{
		Context.TransitionResource(g_HorizontalBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		Context.SetRenderTarget(g_HorizontalBuffer.GetRTV());
		Context.SetViewportAndScissor(0, 0, g_DisplayWidth, g_NativeHeight);
		Context.SetPipelineState(BicubicHorizontalUpsamplePS);
		Context.SetConstants(1, g_NativeWidth, g_NativeHeight, (float)BicubicUpsampleWeight);
		Context.Draw(3);

		Context.TransitionResource(g_HorizontalBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		Context.TransitionResource(UpsampleDest, D3D12_RESOURCE_STATE_RENDER_TARGET);
		Context.SetRenderTarget(UpsampleDest.GetRTV());
		Context.SetViewportAndScissor(0, 0, g_DisplayWidth, g_DisplayHeight);
		Context.SetPipelineState(BicubicVerticalUpsamplePS);
		Context.SetConstants(1, g_DisplayWidth, g_NativeHeight, (float)BicubicUpsampleWeight);
		Context.SetDynamicDescriptor(0, 0, g_HorizontalBuffer.GetSRV());
		Context.Draw(3);
	}
	else if (UpsampleFilter == kSharpening)
	{
		Context.SetPipelineState(SharpeningUpsamplePS);
		Context.TransitionResource(UpsampleDest, D3D12_RESOURCE_STATE_RENDER_TARGET);
		Context.SetRenderTarget(UpsampleDest.GetRTV());
		Context.SetViewportAndScissor(0, 0, g_DisplayWidth, g_DisplayHeight);
		float TexelWidth = 1.0f / g_NativeWidth;
		float TexelHeight = 1.0f / g_NativeHeight;
		float X = Math::Cos((float)SharpeningRotation / 180.0f * 3.14159f) * (float)SharpeningSpread;
		float Y = Math::Sin((float)SharpeningRotation / 180.0f * 3.14159f) * (float)SharpeningSpread;
		const float WA = (float)SharpeningStrength;
		const float WB = 1.0f + 4.0f * WA;
		float Constants[] = { X * TexelWidth, Y * TexelHeight, Y * TexelWidth, -X * TexelHeight, WA, WB };
		Context.SetConstants(1, _countof(Constants), Constants);
		Context.Draw(3);
	}
	else if (UpsampleFilter == kBilinear)
	{
		Context.SetPipelineState(BilinearUpsamplePS);
		Context.TransitionResource(UpsampleDest, D3D12_RESOURCE_STATE_RENDER_TARGET);
		Context.SetRenderTarget(UpsampleDest.GetRTV());
		Context.SetViewportAndScissor(0, 0, g_DisplayWidth, g_DisplayHeight);
		Context.Draw(3);
	}

	if (DebugZoom != kDebugZoomOff)
	{
		Context.TransitionResource(g_PreDisplayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		Context.SetPipelineState(MagnifyPixelsPS);
		Context.TransitionResource(g_DisplayPlane[g_CurrentBuffer], D3D12_RESOURCE_STATE_RENDER_TARGET);
		Context.SetRenderTarget(g_DisplayPlane[g_CurrentBuffer].GetRTV());
		Context.SetViewportAndScissor(0, 0, g_DisplayWidth, g_DisplayHeight);
		Context.SetConstants(1, DebugZoom == kDebugZoom4x ? 0.25f : 0.5f);
		Context.SetDynamicDescriptor(0, 0, g_PreDisplayBuffer.GetSRV());
		Context.Draw(3);
	}

	// Now blend (or write) the UI overlay
	Context.TransitionResource(g_OverlayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	Context.SetDynamicDescriptor(0, 0, g_OverlayBuffer.GetSRV());
	Context.SetPipelineState(s_BlendUIPSO);
	Context.Draw(3);

	Context.TransitionResource(g_DisplayPlane[g_CurrentBuffer], D3D12_RESOURCE_STATE_PRESENT);

	// Close the final context to be executed before frame present.
	Context.Finish();
}

void Graphics::Present(void)
{
	if (g_bEnableHDROutput)
		PreparePresentHDR();
	else
		PreparePresentLDR();

	g_CurrentBuffer = (g_CurrentBuffer + 1) % SWAP_CHAIN_BUFFER_COUNT;

	UINT PresentInterval = s_EnableVSync ? std::min(4, (int)Round(s_FrameTime * 60.0f)) : 0;

	s_SwapChain1->Present(PresentInterval, 0);

	// Test robustness to handle spikes in CPU time
	//if (s_DropRandomFrames)
	//{
	//	if (std::rand() % 25 == 0)
	//		BusyLoopSleep(0.010);
	//}

	int64_t CurrentTick = SystemTime::GetCurrentTick();

	if (s_EnableVSync)
	{
		// With VSync enabled, the time step between frames becomes a multiple of 16.666 ms.  We need
		// to add logic to vary between 1 and 2 (or 3 fields).  This delta time also determines how
		// long the previous frame should be displayed (i.e. the present interval.)
		s_FrameTime = (s_LimitTo30Hz ? 2.0f : 1.0f) / 60.0f;
		if (s_DropRandomFrames)
		{
			if (std::rand() % 50 == 0)
				s_FrameTime += (1.0f / 60.0f);
		}
	}
	else
	{
		// When running free, keep the most recent total frame time as the time step for
		// the next frame simulation.  This is not super-accurate, but assuming a frame
		// time varies smoothly, it should be close enough.
		s_FrameTime = (float)SystemTime::TimeBetweenTicks(s_FrameStartTick, CurrentTick);
	}

	s_FrameStartTick = CurrentTick;

	++s_FrameIndex;

	SetNativeResolution();
}

uint64_t Graphics::GetFrameCount(void)
{
	return s_FrameIndex;
}

float Graphics::GetFrameTime(void)
{
	return s_FrameTime;
}

float Graphics::GetFrameRate(void)
{
	return s_FrameTime == 0.0f ? 0.0f : 1.0f / s_FrameTime;
}
