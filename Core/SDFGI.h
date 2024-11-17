#pragma once
#include "Texture.h"
#include "Math/Vector.h"
#include "Math/BoundingBox.h"
#include "CommandContext.h"
#include "GpuBuffer.h"
#include "ColorBuffer.h"
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
  };

  struct SDFGIProbeData {
      XMMATRIX RandomRotation;
      
      Vector3 GridSize;

      Vector3 ProbeSpacing;
      unsigned int ProbeAtlasBlockResolution;

      Vector3 SceneMinBounds;
      unsigned int GutterSize;
  };

  struct SDFGIProbeGrid {
    // Number of probes along each axis (x, y, z). This is computed from probeSpacing.
    Vector3u probeCount;
    // Distance between probes in world space along each axis.
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

  struct DownsampleCB {
    Vector3 srcSize;
    Vector3 dstSize;
    Vector3 scale;
  };

  struct SDFGIResources {
    const D3D12_CPU_DESCRIPTOR_HANDLE &irradianceAtlasSRV;
  };

  // A lot of "Managers" in the codebase.
  class SDFGIManager {
  public:

    // Used to ensure that irradiance and cubemaps are only captured in the first render call.
    // TODO: don't do this when we support dynamic lights and scenes.
    bool irradianceCaptured = false;
    bool cubeMapsRendered = false;

    int probeCount;
    SDFGIProbeGrid probeGrid;
    const Math::AxisAlignedBox &sceneBounds;

    // Buffer of SDFGIProbe's.
    StructuredBuffer probeBuffer;

    int cubemapFaceResolution = 64;

    DescriptorHeap *externalHeap;

    uint32_t probeAtlasBlockResolution = 8;
    uint32_t gutterSize = 1;
    ColorBuffer irradianceAtlas;
    ColorBuffer &getIrradianceAtlas() { return irradianceAtlas; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetIrradianceAtlasGpuSRV() const;
    DescriptorHandle irradianceAtlasSRVHandle;
    DescriptorHandle &GetIrradianceAtlasDescriptorHandle() { return irradianceAtlasSRVHandle; }
    XMMATRIX randomRotation;

    ColorBuffer depthAtlas;

    // Individual cubemap faces for all probes.
    Texture **probeCubemapFaceTextures;
    D3D12_CPU_DESCRIPTOR_HANDLE **probeCubemapFaceUAVs;
    // A single texture array containing all cubemap faces.
    ColorBuffer probeCubemapArray;

    // A function/lambda for invoking the scene's render function. Used for rendering probe cubemaps.
    std::function<void(GraphicsContext&, const Math::Camera&, const D3D12_VIEWPORT&, const D3D12_RECT&, bool)> renderFunc;  

    SDFGIManager(
      const Math::AxisAlignedBox &sceneBounds,
      // A function/lambda for invoking the scene's render function. Used for rendering probe cubemaps.
      std::function<void(GraphicsContext&, const Math::Camera&, const D3D12_VIEWPORT&, const D3D12_RECT&, bool)> renderFunc,
      DescriptorHeap *externalHeap
    );

    ~SDFGIManager();

    // Initialize all textures needed.
    void InitializeTextures();
    // Initialize all views needed.
    void InitializeViews();

    // Probe positions.
    GraphicsPSO probeVizPSO;    
    RootSignature probeVizRS;
    void InitializeProbeBuffer();
    void InitializeProbeVizShader();
    void RenderProbeViz(GraphicsContext& context, const Math::Camera& camera);

    // Probe update: capture irradiance and depth.
    ComputePSO probeUpdatePSO;
    RootSignature probeUpdateRS;
    void InitializeProbeUpdateShader();
    void UpdateProbes(GraphicsContext& context);

    // Visualization: irradiance atlas (WIP: depth atlas).
    GraphicsPSO atlasVizPSO;    
    RootSignature atlasVizRS;
    void InitializeProbeAtlasVizShader();
    void RenderProbeAtlasViz(GraphicsContext& context, const Math::Camera& camera);

    // Probe cubemaps.
    ComputePSO downsamplePSO;
    RootSignature downsampleRS;
    void InitializeDownsampleShader();
    void RenderCubemapFace(
      GraphicsContext& context, DepthBuffer& depthBuffer, int probe, int face, const Math::Camera& camera, Vector3 &probePosition, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor
    );
    void RenderCubemapsForProbes(GraphicsContext& context, const Math::Camera& camera, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor);

    // Visualization: cubemap faces of a single probe.
    GraphicsPSO cubemapVizPSO;
    RootSignature cubemapVizRS;
    void InitializeCubemapVizShader();
    void RenderCubemapViz(GraphicsContext& context, const Math::Camera& camera);

    SDFGIResources GetResources();

    SDFGIProbeData GetProbeData();

    // Entry point for updating probes.
    void SDFGIManager::Update(GraphicsContext& context, const Math::Camera& camera, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor);

    // Entry point for rendering visualizations.
    void Render(GraphicsContext& context, const Math::Camera& camera);
  };
}
