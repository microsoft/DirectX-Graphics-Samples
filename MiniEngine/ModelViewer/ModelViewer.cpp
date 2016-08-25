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
// Author(s):  Alex Nankervis
//             James Stanard
//

#include "GameCore.h"
#include "GraphicsCore.h"
#include "CameraController.h"
#include "BufferManager.h"
#include "Camera.h"
#include "Model.h"
#include "GpuBuffer.h"
#include "CommandContext.h"
#include "SamplerManager.h"
#include "MotionBlur.h"
#include "DepthOfField.h"
#include "PostEffects.h"
#include "SSAO.h"
#include "FXAA.h"
#include "SystemTime.h"
#include "TextRenderer.h"
#include "ShadowCamera.h"
#include "ParticleEffectManager.h"
#include "GameInput.h"

#include "CompiledShaders/DepthViewerVS.h"
#include "CompiledShaders/DepthViewerPS.h"
#include "CompiledShaders/ModelViewerVS.h"
#include "CompiledShaders/ModelViewerPS.h"

using namespace GameCore;
using namespace Math;
using namespace Graphics;

class ModelViewer : public GameCore::IGameApp
{
public:

	ModelViewer()
		: m_pCameraController(nullptr)
	{
	}

	virtual void Startup( void ) override;
	virtual void Cleanup( void ) override;

	virtual void Update( float deltaT ) override;
	virtual void RenderScene( void ) override;

private:

	void RenderObjects( GraphicsContext& Context, const Matrix4& ViewProjMat );
	void CreateParticleEffects();
	Camera m_Camera;
	CameraController* m_pCameraController;
	Matrix4 m_ViewProjMatrix;
	D3D12_VIEWPORT m_MainViewport;
	D3D12_RECT m_MainScissor;

	RootSignature m_RootSig;
	GraphicsPSO m_DepthPSO;
	GraphicsPSO m_ModelPSO;
	GraphicsPSO m_ShadowPSO;

	D3D12_CPU_DESCRIPTOR_HANDLE m_ExtraTextures[2];
	Model m_Model;

	Vector3 m_SunDirection;
	ShadowCamera m_SunShadow;
};

CREATE_APPLICATION( ModelViewer )

ExpVar m_SunLightIntensity("Application/Sun Light Intensity", 4.0f, 0.0f, 16.0f, 0.1f);
ExpVar m_AmbientIntensity("Application/Ambient Intensity", 0.1f, -16.0f, 16.0f, 0.1f);
NumVar m_SunOrientation("Application/Sun Orientation", -0.5f, -100.0f, 100.0f, 0.1f );
NumVar m_SunInclination("Application/Sun Inclination", 0.75f, 0.0f, 1.0f, 0.01f );
NumVar ShadowDimX("Application/Shadow Dim X", 5000, 1000, 10000, 100 );
NumVar ShadowDimY("Application/Shadow Dim Y", 3000, 1000, 10000, 100 );
NumVar ShadowDimZ("Application/Shadow Dim Z", 3000, 1000, 10000, 100 );

void ModelViewer::Startup( void )
{
	m_RootSig.Reset(6, 2);
	m_RootSig.InitStaticSampler(0, SamplerAnisoWrapDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSig.InitStaticSampler(1, SamplerShadowDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSig[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_RootSig[1].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSig[2].InitAsBufferSRV(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_RootSig[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 6, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSig[4].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 64, 3, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSig[5].InitAsConstants(1, 1, D3D12_SHADER_VISIBILITY_VERTEX);
	m_RootSig.Finalize(L"ModelViewer", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	DXGI_FORMAT ColorFormat = g_SceneColorBuffer.GetFormat();
	DXGI_FORMAT DepthFormat = g_SceneDepthBuffer.GetFormat();
	DXGI_FORMAT ShadowFormat = g_ShadowBuffer.GetFormat();

	D3D12_INPUT_ELEMENT_DESC vertElem[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	m_DepthPSO.SetRootSignature(m_RootSig);
	m_DepthPSO.SetRasterizerState(RasterizerDefault);
	m_DepthPSO.SetBlendState(BlendNoColorWrite);
	m_DepthPSO.SetDepthStencilState(DepthStateReadWrite);
	m_DepthPSO.SetInputLayout(_countof(vertElem), vertElem);
	m_DepthPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	m_DepthPSO.SetRenderTargetFormats(0, nullptr, DepthFormat);
	m_DepthPSO.SetVertexShader(g_pDepthViewerVS, sizeof(g_pDepthViewerVS));
	m_DepthPSO.SetPixelShader(g_pDepthViewerPS, sizeof(g_pDepthViewerPS));
	m_DepthPSO.Finalize();

	m_ShadowPSO = m_DepthPSO;
	m_ShadowPSO.SetRasterizerState(RasterizerShadow);
	m_ShadowPSO.SetRenderTargetFormats(0, nullptr, g_ShadowBuffer.GetFormat());
	m_ShadowPSO.Finalize();

	m_ModelPSO = m_DepthPSO;
	m_ModelPSO.SetBlendState(BlendDisable);
	m_ModelPSO.SetDepthStencilState(DepthStateTestEqual);
	m_ModelPSO.SetRenderTargetFormats(1, &ColorFormat, DepthFormat);
	m_ModelPSO.SetVertexShader( g_pModelViewerVS, sizeof(g_pModelViewerVS) );
	m_ModelPSO.SetPixelShader( g_pModelViewerPS, sizeof(g_pModelViewerPS) );
	m_ModelPSO.Finalize();

	m_ExtraTextures[0] = g_SSAOFullScreen.GetSRV();
	m_ExtraTextures[1] = g_ShadowBuffer.GetSRV();

	TextureManager::Initialize(L"Textures/");
	ASSERT(m_Model.Load("Models/sponza.h3d"), "Failed to load model");
	ASSERT(m_Model.m_Header.meshCount > 0, "Model contains no meshes");

	CreateParticleEffects();

	float modelRadius = Length(m_Model.m_Header.boundingBox.max - m_Model.m_Header.boundingBox.min) * .5f;
	const Vector3 eye = (m_Model.m_Header.boundingBox.min + m_Model.m_Header.boundingBox.max) * .5f + Vector3(modelRadius * .5f, 0.0f, 0.0f);
	m_Camera.SetEyeAtUp( eye, Vector3(kZero), Vector3(kYUnitVector) );
	m_Camera.SetZRange( 1.0f, 10000.0f );
	m_pCameraController = new CameraController(m_Camera, Vector3(kYUnitVector));

	MotionBlur::Enable = true;
	TemporalAA::Enable = true;
	FXAA::Enable = true;
	PostEffects::EnableHDR = true;
	PostEffects::EnableAdaptation = true;
}

void ModelViewer::Cleanup( void )
{
	m_Model.Clear();

	delete m_pCameraController;
	m_pCameraController = nullptr;
}

namespace Graphics
{
	extern EnumVar DebugZoom;
}

void ModelViewer::Update( float deltaT )
{
	ScopedTimer _prof(L"Update State");

	if (GameInput::IsFirstPressed(GameInput::kLShoulder))
		DebugZoom.Decrement();
	else if (GameInput::IsFirstPressed(GameInput::kRShoulder))
		DebugZoom.Increment();

	m_pCameraController->Update(deltaT);
	m_ViewProjMatrix = m_Camera.GetViewProjMatrix();

	float costheta = cosf(m_SunOrientation);
	float sintheta = sinf(m_SunOrientation);
	float cosphi = cosf(m_SunInclination * 3.14159f * 0.5f);
	float sinphi = sinf(m_SunInclination * 3.14159f * 0.5f);
	m_SunDirection = Normalize(Vector3( costheta * cosphi, sinphi, sintheta * cosphi ));

	// We use viewport offsets to jitter our color samples from frame to frame (with TAA.)
	// D3D has a design quirk with fractional offsets such that the implicit scissor
	// region of a viewport is floor(TopLeftXY) and floor(TopLeftXY + WidthHeight), so
	// having a negative fractional top left, e.g. (-0.25, -0.25) would also shift the
	// BottomRight corner up by a whole integer.  One solution is to pad your viewport
	// dimensions with an extra pixel.  My solution is to only use positive fractional offsets,
	// but that means that the average sample position is +0.5, which I use when I disable
	// temporal AA.
	if (TemporalAA::Enable && !DepthOfField::Enable)
	{
		uint64_t FrameIndex = Graphics::GetFrameCount();
#if 1
		// 2x super sampling with no feedback
		float SampleOffsets[2][2] =
		{
			{ 0.25f, 0.25f },
			{ 0.75f, 0.75f },
		};
		const float* Offset = SampleOffsets[FrameIndex & 1];
#else
		// 4x super sampling via controlled feedback
		float SampleOffsets[4][2] =
		{
			{ 0.125f, 0.625f },
			{ 0.375f, 0.125f },
			{ 0.875f, 0.375f },
			{ 0.625f, 0.875f }
		};
		const float* Offset = SampleOffsets[FrameIndex & 3];
#endif
		m_MainViewport.TopLeftX = Offset[0];
		m_MainViewport.TopLeftY = Offset[1];
	}
	else
	{
		m_MainViewport.TopLeftX = 0.5f;
		m_MainViewport.TopLeftY = 0.5f;
	}

	m_MainViewport.Width = (float)g_SceneColorBuffer.GetWidth();
	m_MainViewport.Height = (float)g_SceneColorBuffer.GetHeight();
	m_MainViewport.MinDepth = 0.0f;
	m_MainViewport.MaxDepth = 1.0f;

	m_MainScissor.left = 0;
	m_MainScissor.top = 0;
	m_MainScissor.right = (LONG)g_SceneColorBuffer.GetWidth();
	m_MainScissor.bottom = (LONG)g_SceneColorBuffer.GetHeight();
}

void ModelViewer::RenderObjects( GraphicsContext& gfxContext, const Matrix4& ViewProjMat )
{
	struct VSConstants
	{
		Matrix4 modelToProjection;
		Matrix4 modelToShadow;
		XMFLOAT3 viewerPos;
	} vsConstants;
	vsConstants.modelToProjection = ViewProjMat;
	vsConstants.modelToShadow = m_SunShadow.GetShadowMatrix();
	XMStoreFloat3(&vsConstants.viewerPos, m_Camera.GetPosition());

	gfxContext.SetDynamicConstantBufferView(0, sizeof(vsConstants), &vsConstants);

	uint32_t materialIdx = 0xFFFFFFFFul;

	uint32_t VertexStride = m_Model.m_VertexStride;

	for (unsigned int meshIndex = 0; meshIndex < m_Model.m_Header.meshCount; meshIndex++)
	{
		const Model::Mesh& mesh = m_Model.m_pMesh[meshIndex];

		uint32_t indexCount = mesh.indexCount;
		uint32_t startIndex = mesh.indexDataByteOffset / sizeof(uint16_t);
		uint32_t baseVertex = mesh.vertexDataByteOffset / VertexStride;

		if (mesh.materialIndex != materialIdx)
		{
			materialIdx = mesh.materialIndex;
			gfxContext.SetDynamicDescriptors(3, 0, 6, m_Model.GetSRVs(materialIdx) );
		}

		gfxContext.DrawIndexed(indexCount, startIndex, baseVertex);
	}
}

void ModelViewer::RenderScene( void )
{
	GraphicsContext& gfxContext = GraphicsContext::Begin(L"Scene Render");

	ParticleEffects::Update(gfxContext.GetComputeContext(), Graphics::GetFrameTime());

	__declspec(align(16)) struct
	{
		Vector3 sunDirection;
		Vector3 sunLight;
		Vector3 ambientLight;
		float ShadowTexelSize;
	} psConstants;

	psConstants.sunDirection = m_SunDirection;
	psConstants.sunLight = Vector3(1.0f, 1.0f, 1.0f) * m_SunLightIntensity;
	psConstants.ambientLight = Vector3(1.0f, 1.0f, 1.0f) * m_AmbientIntensity;
	psConstants.ShadowTexelSize = 1.0f / g_ShadowBuffer.GetWidth();

	{
		ScopedTimer _prof(L"Z PrePass", gfxContext);

		gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
		gfxContext.ClearDepth(g_SceneDepthBuffer);

		gfxContext.SetRootSignature(m_RootSig);
		gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		gfxContext.SetIndexBuffer(m_Model.m_IndexBuffer.IndexBufferView());
		gfxContext.SetVertexBuffer(0, m_Model.m_VertexBuffer.VertexBufferView());
		gfxContext.SetDynamicConstantBufferView(1, sizeof(psConstants), &psConstants);

		gfxContext.SetPipelineState(m_DepthPSO);
		gfxContext.SetDepthStencilTarget(g_SceneDepthBuffer.GetDSV());
		gfxContext.SetViewportAndScissor(m_MainViewport, m_MainScissor);
		RenderObjects(gfxContext, m_ViewProjMatrix );
	}

	SSAO::Render(gfxContext, m_Camera);

	if (!SSAO::DebugDraw)
	{
		ScopedTimer _prof(L"Main Render", gfxContext);

		gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
		gfxContext.ClearColor(g_SceneColorBuffer);

		// Set the default state for command lists
		auto& pfnSetupGraphicsState = [&](void)
		{
			gfxContext.SetRootSignature(m_RootSig);
			gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			gfxContext.SetIndexBuffer(m_Model.m_IndexBuffer.IndexBufferView());
			gfxContext.SetVertexBuffer(0, m_Model.m_VertexBuffer.VertexBufferView());
			gfxContext.SetDynamicDescriptors(4, 0, 2, m_ExtraTextures);
			gfxContext.SetDynamicConstantBufferView(1, sizeof(psConstants), &psConstants);
		};

		pfnSetupGraphicsState();

		{
			ScopedTimer _prof(L"Render Shadow Map", gfxContext);

			m_SunShadow.UpdateMatrix(-m_SunDirection, Vector3(0, -500.0f, 0), Vector3(ShadowDimX, ShadowDimY, ShadowDimZ),
				(uint32_t)g_ShadowBuffer.GetWidth(), (uint32_t)g_ShadowBuffer.GetHeight(), 16);

			gfxContext.SetPipelineState(m_ShadowPSO);
			g_ShadowBuffer.BeginRendering(gfxContext);
			RenderObjects(gfxContext, m_SunShadow.GetViewProjMatrix());
			g_ShadowBuffer.EndRendering(gfxContext);
		}

		if (SSAO::AsyncCompute)
		{
			gfxContext.Flush();
			pfnSetupGraphicsState();

			// Make the 3D queue wait for the Compute queue to finish SSAO
			g_CommandManager.GetGraphicsQueue().StallForProducer(g_CommandManager.GetComputeQueue());
		}

		{
			ScopedTimer _prof(L"Render Color", gfxContext);
			gfxContext.SetPipelineState(m_ModelPSO);
			gfxContext.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
			gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
			gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV(), g_SceneDepthBuffer.GetDSV_DepthReadOnly());
			gfxContext.SetViewportAndScissor(m_MainViewport, m_MainScissor);
			RenderObjects( gfxContext, m_ViewProjMatrix );
		}
	}

	ParticleEffects::Render(gfxContext, m_Camera, g_SceneColorBuffer, g_SceneDepthBuffer, g_LinearDepth);

	// Until I work out how to couple these two, it's "either-or".
	if (DepthOfField::Enable)
		DepthOfField::Render(gfxContext, m_Camera.GetNearClip(), m_Camera.GetFarClip());
	else
		MotionBlur::RenderCameraBlur(gfxContext, m_Camera);

	gfxContext.Finish();
}

void ModelViewer::CreateParticleEffects()
{
	ParticleEffectProperties Effect = ParticleEffectProperties();
	Effect.MinStartColor = Effect.MaxStartColor = Effect.MinEndColor = Effect.MaxEndColor = Color(1.0f, 1.0f, 1.0f, 0.0f);
	Effect.TexturePath = L"sparkTex.dds";

	Effect.TotalActiveLifetime = FLT_MAX;
	Effect.Size = Vector4(4.0f, 8.0f, 4.0f, 8.0f);
	Effect.Velocity = Vector4(20.0f, 200.0f, 50.0f, 180.0f);
	Effect.LifeMinMax = XMFLOAT2(1.0f, 3.0f);
	Effect.MassMinMax = XMFLOAT2(4.5f, 15.0f);
	Effect.EmitProperties.Gravity = XMFLOAT3(0.0f, -100.0f, 0.0f);
	Effect.EmitProperties.FloorHeight = -0.5f;
	Effect.EmitProperties.EmitPosW = Effect.EmitProperties.LastEmitPosW = XMFLOAT3(-1200.0f, 185.0f, -445.0f);
	Effect.EmitProperties.MaxParticles = 800;
	Effect.EmitRate = 64.0f;
	Effect.Spread.x = 20.0f;
	Effect.Spread.y = 50.0f;
	ParticleEffects::InstantiateEffect( &Effect );

	ParticleEffectProperties Smoke = ParticleEffectProperties();
	Smoke.TexturePath = L"smoke.dds";

	Smoke.TotalActiveLifetime = FLT_MAX;;
	Smoke.EmitProperties.MaxParticles = 25;
	Smoke.EmitProperties.EmitPosW = Smoke.EmitProperties.LastEmitPosW = XMFLOAT3(1120.0f, 185.0f, -445.0f);
	Smoke.EmitRate = 64.0f;
	Smoke.LifeMinMax = XMFLOAT2(2.5f, 4.0f);
	Smoke.Size = Vector4(60.0f, 108.0f, 30.0f, 208.0f);
	Smoke.Velocity = Vector4(30.0f, 30.0f, 10.0f, 40.0f);
	Smoke.MassMinMax = XMFLOAT2(1.0, 3.5);
	Smoke.Spread.x = 60.0f;
	Smoke.Spread.y = 70.0f;
	Smoke.Spread.z = 20.0f;
	ParticleEffects::InstantiateEffect( &Smoke );

	ParticleEffectProperties Fire = ParticleEffectProperties();
	Fire.MinStartColor = Fire.MaxStartColor = Fire.MinEndColor = Fire.MaxEndColor = Color(1.0f, 1.0f, 1.0f, 0.0f);
	Fire.TexturePath = L"fire.dds";

	Fire.TotalActiveLifetime = FLT_MAX;
	Fire.Size = Vector4(54.0f, 68.0f, 0.1f, 0.3f);
	Fire.Velocity = Vector4 (10.0f, 30.0f, 50.0f, 50.0f);
	Fire.LifeMinMax = XMFLOAT2(1.0f, 3.0f);
	Fire.MassMinMax = XMFLOAT2(10.5f, 14.0f);
	Fire.EmitProperties.Gravity = XMFLOAT3(0.0f, 1.0f, 0.0f);
	Fire.EmitProperties.EmitPosW = Fire.EmitProperties.LastEmitPosW = XMFLOAT3(1120.0f, 125.0f, 405.0f);
	Fire.EmitProperties.MaxParticles = 25;
	Fire.EmitRate = 64.0f;
	Fire.Spread.x = 1.0f;
	Fire.Spread.y = 60.0f;
	ParticleEffects::InstantiateEffect( &Fire );
}