#pragma once

#include <vector>
#include "Graphics/Color.h"
#include "Graphics/Material.h"
#include "Graphics/RayTracing/RayObjectIntersection.h"
#include "Graphics/RenderingSettings.h"
#include "Graphics/Shading/Lighting/Light.h"
#include "Graphics/Shading/ShadingSettings.h"
#include "Graphics/VertexWithAttributes.h"
#include "Math/Vector3.h"

namespace GRAPHICS::SHADING
{
    /// Encapsulates domain knowledge for shading algorithms.
    class Shading
    {
    public:
        static Color Compute(
            const VertexWithAttributes& world_vertex,
            const MATH::Vector3f& unit_vertex_normal,
            const Material& material,
            const MATH::Vector3f& viewing_world_position,
            const std::vector<LIGHTING::Light>& lights,
            const RenderingSettings& rendering_settings);

        static Color Compute(
            const GRAPHICS::RAY_TRACING::RayObjectIntersection& intersection,
            const std::vector<LIGHTING::Light>& lights,
            const ShadingSettings& shading_settings,
            const std::vector<float> shadow_factors_by_light_index);

        static Color ComputeMaterialShading(
            const MATH::Vector3f& viewing_point,
            const std::vector<LIGHTING::Light>& lights,
            const Surface& surface,
            const MATH::Vector3f& surface_point,
            const ShadingSettings& shading_settings,
            const std::vector<float> shadow_factors_by_light_index);

        static Color ComputeMaterialShading(
            const MATH::Vector3f& viewing_point,
            const LIGHTING::Light& light,
            const Surface& surface,
            const MATH::Vector3f& surface_point,
            const ShadingSettings& shading_settings,
            float shadow_factor);
    };
}
