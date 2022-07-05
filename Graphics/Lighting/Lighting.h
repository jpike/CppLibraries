#pragma once

#include <vector>
#include "Graphics/Color.h"
#include "Graphics/Lighting/Light.h"
#include "Graphics/Material.h"
#include "Graphics/RayTracing/Surface.h"
#include "Graphics/RenderingSettings.h"
#include "Math/Vector3.h"

namespace GRAPHICS::LIGHTING
{
    /// Encapsulates domain knowledge for lighting algorithms.
    class Lighting
    {
    public:
        static Color Compute(
            const MATH::Vector3f& world_vertex,
            const MATH::Vector3f& unit_vertex_normal,
            const Material& material,
            const MATH::Vector3f& viewing_world_position,
            const std::vector<Light>& lights,
            const RenderingSettings& rendering_settings);

        static Color Compute(
            const MATH::Vector3f& viewing_point,
            const std::vector<Light>& lights,
            const RAY_TRACING::Surface& surface,
            const MATH::Vector3f& surface_point,
            const LightingSettings& lighting_settings,
            const std::vector<float> shadow_factors_by_light_index);

#if TODO
        static Color Compute(

        );
#endif
    };
}
