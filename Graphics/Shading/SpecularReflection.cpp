#include <algorithm>
#include <cmath>
#include "ErrorHandling/Asserts.h"
#include "Graphics/Shading/SpecularReflection.h"
#include "Graphics/TextureMappingAlgorithm.h"

namespace GRAPHICS::SHADING
{
    /// Computes the specular reflection for a single light for a particular surface point.
    /// @param[in]  shading_settings - Settings affecting the shading.
    /// @param[in]  viewing_point - The point from which the surface is being viewed.
    /// @param[in]  light - The light for which to compute the specular reflection at the surface point.
    /// @param[in]  shadow_factor - A shadowing factor to add in additional shadowing
    ///     (0 == full shadowing, 1 == no shadowing).  If desired, this must be computed and passed in externally
    ///     as shadowing imposes too high a cost to dynamically compute all the time.
    /// @param[in]  surface - The surface for which specular reflection is being computed.
    /// @param[in]  surface_point - The point on the surface for which to compute the specular reflection.
    /// @return The color of specular reflection from the light at the specified surface point.
    Color SpecularReflection::Compute(
        const ShadingSettings& shading_settings,
        const MATH::Vector3f& viewing_point,
        const LIGHTING::Light& light,
        const float shadow_factor,
        const Surface& surface,
        const MATH::Vector3f& surface_point)
    {
        // ENSURE A MATERIAL EXISTS.
        std::shared_ptr<Material> material = surface.GetMaterial();
        ASSERT_THEN_IF_NOT(material)
        {
            // INDICATE THAT NO LIGHTING CAN EXIST WITHOUT A MATERIAL.
            return Color::BLACK;
        }

        // GET THE DIRECTION FROM THE SURFACE POINT TO THE LIGHT.
        MATH::Vector3f direction_from_point_to_light;
        if (LIGHTING::LightType::DIRECTIONAL == light.Type)
        {
            // The computations are based on the opposite direction.
            direction_from_point_to_light = MATH::Vector3f::Scale(-1.0f, light.DirectionalLightDirection);
        }
        else if (LIGHTING::LightType::POINT == light.Type)
        {
            direction_from_point_to_light = light.PointLightWorldPosition - surface_point;
        }
        else
        {
            // Other types of light do not result in any specular reflection.
            return Color::BLACK;
        }

        // COMPUTE THE PROPORTION OF THE SURFACE POINT THAT IS ILLUMINATED BY THE LIGHT.
        // The illumination proportion is based on the Lambertian shading model.
        // An object is maximally illuminated when facing toward the light.
        // An object tangent to the light direction or facing away receives no illumination.
        // In-between, the amount of illumination is proportional to the cosine of the angle between
        // the light and surface normal (where the cosine can be computed via the dot product).
        constexpr float NO_ILLUMINATION = 0.0f;
        MATH::Vector3f unit_surface_normal = surface.GetNormal(surface_point);
        MATH::Vector3f unit_direction_from_point_to_light = MATH::Vector3f::Normalize(direction_from_point_to_light);
        float illumination_proportion = MATH::Vector3f::DotProduct(unit_surface_normal, unit_direction_from_point_to_light);
        illumination_proportion = std::max(NO_ILLUMINATION, illumination_proportion);

        // COMPUTE THE REFLECTED LIGHT DIRECTION.
        MATH::Vector3f reflected_light_along_surface_normal = MATH::Vector3f::Scale(2.0f * illumination_proportion, unit_surface_normal);
        MATH::Vector3f reflected_light_direction = reflected_light_along_surface_normal - unit_direction_from_point_to_light;
        MATH::Vector3f unit_reflected_light_direction = MATH::Vector3f::Normalize(reflected_light_direction);

        // COMPUTE THE SPECULAR AMOUNT.
        // The closer the ray from the surface point to the viewing point is to the ideal (perfect) reflected direction,
        // the shinier (more specular reflection) occurs.
        MATH::Vector3f ray_from_surface_point_to_viewing_point = viewing_point - surface_point;
        MATH::Vector3f normalized_ray_from_surface_point_to_viewing_point = MATH::Vector3f::Normalize(ray_from_surface_point_to_viewing_point);
        float specular_proportion = MATH::Vector3f::DotProduct(normalized_ray_from_surface_point_to_viewing_point, unit_reflected_light_direction);
        specular_proportion = std::max(NO_ILLUMINATION, specular_proportion);
        specular_proportion = std::pow(specular_proportion, material->SpecularProperties.SpecularPower);

        // COMPUTE THE AMOUNT OF SPECULAR LIGHT SHINING ON THE SURFACE.
        float light_proportion = shadow_factor * specular_proportion;
        Color current_light_specular_color = Color::ScaleRedGreenBlue(light_proportion, light.Color);

        // COMPUTE THE RAW SPECULAR SURFACE COLOR.
        Color specular_surface_color = material->SpecularProperties.Color;
        /// @todo   Figure out better way to handle this triangle texture mapping stuff!
        const GEOMETRY::Triangle* const* triangle = std::get_if<const GEOMETRY::Triangle*>(&surface.Shape);
        if (shading_settings.TextureMappingEnabled && triangle && material->SpecularProperties.Texture)
        {
            MATH::Vector2f triangle_point(surface_point.X, surface_point.Y);
            Color texture_color = TextureMappingAlgorithm::LookupTexel(
                **triangle,
                triangle_point,
                *material->SpecularProperties.Texture);
            specular_surface_color = Color::ComponentMultiplyRedGreenBlue(specular_surface_color, texture_color);
        }

        // COMPUTE THE SPECULAR COLOR REFLECTED ON THE SURFACE.
        Color specular_color = Color::ComponentMultiplyRedGreenBlue(specular_surface_color, current_light_specular_color);
        return specular_color;
    }
}
