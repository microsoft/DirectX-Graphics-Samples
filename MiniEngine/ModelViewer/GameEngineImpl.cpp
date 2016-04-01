#include "pch.h"
#include "GameEngineImpl.h"

namespace Graphics
{
	extern EnumVar DebugZoom;
}


ExpVar m_SunLightIntensity("Application/Sun Intensity", 4.0f, 0.0f, 16.0f, 0.25f);
NumVar m_SunOrientation("Application/Sun Orientation", -0.5f, -100.0f, 100.0f, 0.1f);
NumVar m_SunInclination("Application/Sun Inclination", 0.75f, 0.0f, 1.0f, 0.01f);
NumVar ShadowDimX("Application/Shadow Dim X", 5000, 1000, 10000, 100);
NumVar ShadowDimY("Application/Shadow Dim Y", 3000, 1000, 10000, 100);
NumVar ShadowDimZ("Application/Shadow Dim Z", 3000, 1000, 10000, 100);




void GameEngineImpl::Update(float deltaT) 
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
	m_SunDirection = Normalize(Vector3(costheta * cosphi, sinphi, sintheta * cosphi));

	// We use viewport offsets to jitter our color samples from frame to frame (with TAA.)
	// D3D has a design quirk with fractional offsets such that the implicit scissor
	// region of a viewport is floor(TopLeftXY) and floor(TopLeftXY + WidthHeight), so
	// having a negative fractional top left, e.g. (-0.25, -0.25) would also shift the
	// BottomRight corner up by a whole integer.  One solution is to pad your viewport
	// dimensions with an extra pixel.  My solution is to only use positive fractional offsets,
	// but that means that the average sample position is +0.5, which I use when I disable
	// temporal AA.
	if (TemporalAA::Enable)
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


void GameEngineImpl::RenderObjects(GraphicsContext& gfxContext, const Matrix4& ViewProjMat)
{
	__declspec(align(64))  struct VSConstants
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
			gfxContext.SetDynamicDescriptors(3, 0, 6, m_Model.GetSRVs(materialIdx));
		}

#if USE_VERTEX_BUFFER
		gfxContext.DrawIndexed(indexCount, startIndex, baseVertex);
#else
		gfxContext.SetConstants(5, baseVertex);
		gfxContext.DrawIndexed(indexCount, startIndex);
#endif
	}
}



void GameEngineImpl::RenderScene(void)
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
	psConstants.ambientLight = Vector3(0.2f, 0.2f, 0.2f);
	psConstants.ShadowTexelSize = 1.0f / g_ShadowBuffer.GetWidth();

	{
		ScopedTimer _prof(L"Z PrePass", gfxContext);

		gfxContext.ClearDepth(g_SceneDepthBuffer);

		gfxContext.SetRootSignature(m_RootSig);
		gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		gfxContext.SetIndexBuffer(m_Model.m_IndexBuffer.IndexBufferView());
#if USE_VERTEX_BUFFER
		gfxContext.SetVertexBuffer(0, m_Model.m_VertexBuffer.VertexBufferView());
#elif USE_ROOT_BUFFER_SRV
		gfxContext.SetBufferSRV(2, m_Model.m_VertexBuffer);
#else
		gfxContext.SetDynamicDescriptor(2, 0, m_Model.m_VertexBuffer.GetSRV());
#endif
		gfxContext.SetDynamicConstantBufferView(1, sizeof(psConstants), &psConstants);

		gfxContext.SetPipelineState(m_DepthPSO);
		gfxContext.SetDepthStencilTarget(g_SceneDepthBuffer);
		gfxContext.SetViewportAndScissor(m_MainViewport, m_MainScissor);
		RenderObjects(gfxContext, m_ViewProjMatrix);
	}

	SSAO::Render(gfxContext, m_Camera);

	if (!SSAO::DebugDraw)
	{
		ScopedTimer _prof(L"Main Render", gfxContext);

		gfxContext.ClearColor(g_SceneColorBuffer);

		// Set the default state for command lists
		auto& pfnSetupGraphicsState = [&](void)
		{
			gfxContext.SetRootSignature(m_RootSig);
			gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			gfxContext.SetIndexBuffer(m_Model.m_IndexBuffer.IndexBufferView());
#if USE_VERTEX_BUFFER
			gfxContext.SetVertexBuffer(0, m_Model.m_VertexBuffer.VertexBufferView());
#elif USE_ROOT_BUFFER_SRV
			gfxContext.SetBufferSRV(2, m_Model.m_VertexBuffer);
#else
			gfxContext.SetDynamicDescriptor(2, 0, m_Model.m_VertexBuffer.GetSRV());
#endif
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
			gfxContext.SetRenderTarget(g_SceneColorBuffer, g_SceneDepthBuffer, true);
			gfxContext.SetViewportAndScissor(m_MainViewport, m_MainScissor);
			RenderObjects(gfxContext, m_ViewProjMatrix);
		}
	}

	ParticleEffects::Render(gfxContext, m_Camera, g_SceneColorBuffer, g_SceneDepthBuffer, g_LinearDepth);

	MotionBlur::RenderCameraBlur(gfxContext, m_Camera);

	gfxContext.Finish();
}
