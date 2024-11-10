#pragma once
#include "Texture.h"
#include "Math/Vector.h"
#include "Math/BoundingBox.h"
#include "CommandContext.h"
#include "GpuBuffer.h"
#include <array>

using namespace Math;

typedef Vector3 Vector3i;
typedef Vector3 uint3;
typedef Vector3 float3;

typedef std::array<uint32_t, 3> Vector3u;
typedef std::array<float, 3> Vector3f;

namespace Math { class Camera; }
class GraphicsContext;
class ComputeContext;
class BoolVar;

namespace SDFGI
{
    struct SDFGIProbe {
      // Position of the probe in world space.
      Vector3 position;
      float irradiance;
      float depth;
    };

    struct SDFGIProbeGrid {
      // Number of probes along each axis (x, y, z).
      Vector3u probeCount;
      // Distance between probes in world space.
      Vector3f probeSpacing;
      std::vector<SDFGIProbe> probes;

      SDFGIProbeGrid(Vector3 &sceneSize, Vector3 &sceneMin);

      void GenerateProbes(Vector3 &sceneMin);
    };

    struct CameraData {
        Matrix4 viewProjMatrix;
        Vector3 position;
        float pad;
    };

    // A lot of "Managers" in the codebase.
    class SDFGIManager {
    public:
      bool irradianceCaptured = false;
      bool cubeMapsRendered = false;
      Texture irradianceTexture;
      Texture irradianceAtlas;
      Texture depthTexture;
      Texture depthAtlas;
      SDFGIProbeGrid probeGrid;
      StructuredBuffer probeBuffer;
      ComputePSO probeUpdateComputePSO;
      RootSignature probeUpdateComputeRootSignature;
      D3D12_CPU_DESCRIPTOR_HANDLE irradianceUAV;
      D3D12_CPU_DESCRIPTOR_HANDLE irradianceAtlasUAV;
      D3D12_CPU_DESCRIPTOR_HANDLE depthUAV;
      D3D12_CPU_DESCRIPTOR_HANDLE depthAtlasUAV;
      const Math::AxisAlignedBox &sceneBounds;
      GraphicsPSO textureVisualizationPSO;    
      RootSignature textureVisualizationRootSignature;
      Texture probeIrradianceCubemap;
      D3D12_CPU_DESCRIPTOR_HANDLE cubemapRTVs[6];
      std::function<void(GraphicsContext&, const Math::Camera&, const D3D12_VIEWPORT&, const D3D12_RECT&, D3D12_CPU_DESCRIPTOR_HANDLE*, Texture*)> renderFunc;
      Texture **intermediateTextures;
      D3D12_CPU_DESCRIPTOR_HANDLE **intermediateRTVs;
      int probeCount;
      GraphicsPSO cubemapVisualizationPSO;
      RootSignature cubemapVisualizationRootSignature;
      GraphicsPSO BasicPipelineState;
      RootSignature BasicRootSignature;
      D3D12_VERTEX_BUFFER_VIEW pentagonVertexBufferView = {};
      D3D12_INDEX_BUFFER_VIEW pentagonIndexBufferView = {};
      int frameCount = 0;

      SDFGIManager(
        Vector3u probeCount, Vector3f probeSpacing, const Math::AxisAlignedBox &sceneBounds, 
        std::function<void(GraphicsContext&, const Math::Camera&, const D3D12_VIEWPORT&, const D3D12_RECT&, D3D12_CPU_DESCRIPTOR_HANDLE*, Texture*)> renderFunc
      );

      struct Vertex
      {
          float x, y, z;
          float r, g, b, a;
      };

      void InitializeProbeUpdateShader();

      void InitializeTextureVizShader();

      void InitializeTextures();

      void InitializeViews();

      void InitializeProbeBuffer();

      void CaptureIrradianceAndDepth(GraphicsContext& context);

      void RenderIrradianceDepthViz(GraphicsContext& context, const Math::Camera& camera, int sliceIndex, float maxDepthDistance);

      void RenderToCubemapFace(
        GraphicsContext& context, DepthBuffer& depthBuffer, int probe, int face, int cubemapResolution, const Math::Camera& camera, Vector3 &probePosition, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor
      );

      Matrix4 GetViewMatrixForCubemapFace(int faceIndex, const Vector3& probePosition);

      void RenderCubemapsForProbes(GraphicsContext& context, const Math::Camera& camera, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor);

      void CopyCubemapFaceToIntermediate(GraphicsContext& context, int face, int cubemapResolution);

      void InitializeCubemapVisualizationShader();

      void RenderCubemapViz(GraphicsContext& context, int cubemapResolution, const Math::Camera& camera);

      void SimpleRenderFunc(GraphicsContext& context, const Math::Camera& camera, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor);

      void InitializeSimpleQuadPipelineState();

      void InitializePentagon();
    };



    void Initialize(void);
    
    void Shutdown(void);
    
    void Render(GraphicsContext& context, const Math::Camera& camera, SDFGIManager *SDFGIManager, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor);
    
    void UpdateProbeData(GraphicsContext& context);

    extern BoolVar Enable;
    extern BoolVar DebugDraw;
}
