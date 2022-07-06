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
    /// @todo   Move reflection classes to different files!

    /// Diffuse reflection of light that scatters approximately equally in all directions on a surface.
    /// See the following more more details:
    /// - https://en.wikipedia.org/wiki/Diffuse_reflection
    /// - https://en.wikipedia.org/wiki/Lambertian_reflectance
    class DiffuseReflection
    {
    public:
        static Color Compute(
            const Light& light,
            const float shadow_factor,
            const RAY_TRACING::Surface& surface,
            const MATH::Vector3f& surface_point);
    };

    /// Specular reflection of light that reflects similarly to a mirror to provide shininess on a surface.
    /// This is based on the Blinn-Phong model.
    /// See the following for more details:
    /// - https://en.wikipedia.org/wiki/Specular_reflection
    class SpecularReflection
    {
    public:
        static Color Compute(
            const MATH::Vector3f& viewing_point,
            const Light& light,
            const float shadow_factor,
            const RAY_TRACING::Surface& surface,
            const MATH::Vector3f& surface_point);
    };

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

        static Color Compute(
            const MATH::Vector3f& viewing_point,
            const Light& light,
            const RAY_TRACING::Surface& surface,
            const MATH::Vector3f& surface_point,
            const LightingSettings& lighting_settings,
            float shadow_factor);
    };
}
