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
      Texture irradianceTexture;
      Texture depthTexture;
      SDFGIProbeGrid probeGrid;
      StructuredBuffer probeBuffer;

      SDFGIManager(Vector3u probeCount, Vector3f probeSpacing, const Math::AxisAlignedBox &sceneBounds);

      void InitializeTextures();

      void InitializeProbeBuffer();
    };



    void Initialize(void);
    
    void Shutdown(void);
    
    void Render(GraphicsContext& context, const Math::Camera& camera, SDFGIManager *SDFGIManager);
    
    void UpdateProbeData(GraphicsContext& context);

    extern BoolVar Enable;
    extern BoolVar DebugDraw;
}
