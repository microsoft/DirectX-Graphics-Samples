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

//
// Identifies a unique shader with its own unique root signature and pipeline state.
//
class Shader
{
protected:
	UINT32 m_VertexSize;
	ID3D12RootSignature* m_pRootSignature = nullptr;
	ID3D12PipelineState* m_pPipelineState = nullptr;

	HRESULT CreateDeviceDependentState(
		ID3D12Device* pDevice,
		const wchar_t* pShaderFile,
		D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignatureDesc,
		D3D12_INPUT_ELEMENT_DESC* pInputElements,
		UINT InputElementCount,
		bool bEnableAlpha);

public:
	Shader(UINT32 VertexSize);
	~Shader();

	void DestroyDeviceDependentState();

	UINT32 GetVertexSize() const
	{
		return m_VertexSize;
	}

	inline ID3D12RootSignature* GetRootSignature() const
	{
		return m_pRootSignature;
	}

	inline ID3D12PipelineState* GetPipelineState() const
	{
		return m_pPipelineState;
	}
};

//
// Creates a shader with a root signature and pipeline state that matches the vertex
// format used for a vertex containing UV coordinate information for textured quads.
//
class TextureShader : public Shader
{
public:
	struct VertexFormat
	{
		float Position[3];
		float TexCoords[2];
		float Color[4];
	};

	TextureShader();

	HRESULT CreateDeviceDependentState(ID3D12Device* pDevice, const wchar_t* pShaderFile);
};

//
// Creates a shader with a root signature and pipeline state that matches the vertex
// format used for a vertex containing color information for colored quads.
//
class ColorShader : public Shader
{
public:
	struct VertexFormat
	{
		float Position[3];
		float ColorRGBA[4];
	};

	ColorShader();

	HRESULT CreateDeviceDependentState(ID3D12Device* pDevice, const wchar_t* pShaderFile);
};
