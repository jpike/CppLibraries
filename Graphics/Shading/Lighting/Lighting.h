#pragma once

#include <vector>
#include "Graphics/Color.h"
#include "Graphics/Material.h"
#include "Graphics/RenderingSettings.h"
#include "Graphics/Shading/Lighting/Light.h"
#include "Graphics/Surface.h"
#include "Math/Vector3.h"

namespace GRAPHICS::SHADING::LIGHTING
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
            const Surface& surface,
            const MATH::Vector3f& surface_point,
            const LightingSettings& lighting_settings,
            const std::vector<float> shadow_factors_by_light_index);

        static Color Compute(
            const MATH::Vector3f& viewing_point,
            const Light& light,
            const Surface& surface,
            const MATH::Vector3f& surface_point,
            const LightingSettings& lighting_settings,
            float shadow_factor);
    };
}
