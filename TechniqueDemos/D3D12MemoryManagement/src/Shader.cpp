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

#include "stdafx.h"

Shader::Shader(UINT32 VertexSize) :
	m_pPipelineState(nullptr),
	m_VertexSize(VertexSize)
{
}

Shader::~Shader()
{
}

HRESULT Shader::CreateDeviceDependentState(
	ID3D12Device* pDevice,
	const wchar_t* pShaderFile,
	D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignatureDesc,
	D3D12_INPUT_ELEMENT_DESC* pInputElements,
	UINT InputElementCount,
	bool bEnableAlpha)
{
	HRESULT hr;

	//
	// Create the root signature.
	//
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(pDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		ComPtr<ID3DBlob> pSignature;
		ComPtr<ID3DBlob> pErrors;
		hr = D3DX12SerializeVersionedRootSignature(pRootSignatureDesc, featureData.HighestVersion, &pSignature, &pErrors);
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to serialize root signature, hr=0x%.8x", hr);
			if (pErrors)
			{
				printf("Error description:\n%s", (LPCSTR)pErrors->GetBufferPointer());
			}
			return hr;
		}

		hr = pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature));
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create root signature, hr=0x%.8x", hr);
			return hr;
		}
	}

	//
	// Compile and create shaders and pipeline state.
	//
	{
		ComPtr<ID3DBlob> pVertexShader;
		ComPtr<ID3DBlob> pPixelShader;
		ComPtr<ID3DBlob> pErrors;

#if _DEBUG
		UINT CompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT CompileFlags = 0;
#endif

		WCHAR FileName[MAX_PATH];
		GetWorkingDir(FileName, MAX_PATH);
		wcscat_s(FileName, pShaderFile);

		static LPCSTR VSEntryPoint = "VShader";
		static LPCSTR VSCompilerTarget = "vs_5_0";

		hr = D3DCompileFromFile(FileName, nullptr, nullptr, VSEntryPoint, VSCompilerTarget, CompileFlags, 0, &pVertexShader, &pErrors);
		if (FAILED(hr))
		{
			LOG_ERROR(
				"Failed to compile vertex shader. File=%ls, EntryPoint=%s, Target=%s, hr=0x%.8x",
				FileName,
				VSEntryPoint,
				VSCompilerTarget,
				hr);
			if (pErrors)
			{
				printf("Error description:\n%s", (LPCSTR)pErrors->GetBufferPointer());
			}
			return hr;
		}

		static LPCSTR PSEntryPoint = "PShader";
		static LPCSTR PSCompilerTarget = "ps_5_0";

		hr = D3DCompileFromFile(FileName, nullptr, nullptr, PSEntryPoint, PSCompilerTarget, CompileFlags, 0, &pPixelShader, nullptr);
		if (FAILED(hr))
		{
			LOG_ERROR(
				"Failed to compile pixel shader. File=%ls, EntryPoint=%s, Target=%s, hr=0x%.8x",
				FileName,
				PSEntryPoint,
				PSCompilerTarget,
				hr);
			if (pErrors)
			{
				printf("Error description:\n%s", (LPCSTR)pErrors->GetBufferPointer());
			}
			return hr;
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc = {};
		PsoDesc.InputLayout = { pInputElements, InputElementCount };
		PsoDesc.pRootSignature = m_pRootSignature;
		PsoDesc.VS = { reinterpret_cast<UINT8*>(pVertexShader->GetBufferPointer()), pVertexShader->GetBufferSize() };
		PsoDesc.PS = { reinterpret_cast<UINT8*>(pPixelShader->GetBufferPointer()), pPixelShader->GetBufferSize() };
		PsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		PsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		if (bEnableAlpha)
		{
			PsoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
			PsoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			PsoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			PsoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			PsoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
			PsoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
			PsoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			PsoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = 0x0f;
		}
		PsoDesc.DepthStencilState.DepthEnable = FALSE;
		PsoDesc.DepthStencilState.StencilEnable = FALSE;
		PsoDesc.SampleMask = UINT_MAX;
		PsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PsoDesc.NumRenderTargets = 1;
		PsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		PsoDesc.SampleDesc.Count = 1;

		hr = pDevice->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&m_pPipelineState));
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create graphics pipeline state, hr=0x%.8x", hr);
			return hr;
		}
	}

	return S_OK;
}

void Shader::DestroyDeviceDependentState()
{
	SafeRelease(m_pPipelineState);
	SafeRelease(m_pRootSignature);
}

//
// TextureShader
//
TextureShader::TextureShader() :
	Shader(sizeof(VertexFormat))
{
}

HRESULT TextureShader::CreateDeviceDependentState(ID3D12Device* pDevice, const wchar_t* pShaderFile)
{
	CD3DX12_DESCRIPTOR_RANGE1 DescriptorRanges[1];
	DescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

	CD3DX12_ROOT_PARAMETER1 RootParameters[2];
	RootParameters[0].InitAsConstantBufferView(0);
	RootParameters[1].InitAsDescriptorTable(1, &DescriptorRanges[0], D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_STATIC_SAMPLER_DESC StaticSamplerDesc(0);
	StaticSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	StaticSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDesc;
	RootSignatureDesc.Init_1_1(_countof(RootParameters), RootParameters, 1, &StaticSamplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	D3D12_INPUT_ELEMENT_DESC InputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	return Shader::CreateDeviceDependentState(pDevice, pShaderFile, &RootSignatureDesc, InputElementDescs, _countof(InputElementDescs), false);
}

//
// ColorShader
//
ColorShader::ColorShader() :
	Shader(sizeof(VertexFormat))
{

}

HRESULT ColorShader::CreateDeviceDependentState(ID3D12Device* pDevice, const wchar_t* pShaderFile)
{
	CD3DX12_ROOT_PARAMETER1 RootParameters[1];
	RootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDesc;
	RootSignatureDesc.Init_1_1(_countof(RootParameters), RootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	D3D12_INPUT_ELEMENT_DESC InputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	return Shader::CreateDeviceDependentState(pDevice, pShaderFile, &RootSignatureDesc, InputElementDescs, _countof(InputElementDescs), true);
}
