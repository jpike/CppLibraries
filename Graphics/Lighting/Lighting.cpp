#include "ErrorHandling/Asserts.h"
#include "Graphics/Lighting/Lighting.h"

namespace GRAPHICS::LIGHTING
{
    /// Computes lighting for a vertex.
    /// @param[in]  world_vertex - The world space vertex for which to compute lighting.
    /// @param[in]  unit_vertex_normal - The unit surface normal for the vertex.
    /// @param[in]  material - The material for the vertex.
    /// @param[in]  viewing_world_position - The world position from where the vertex is being viewed.
    /// @param[in]  lights - The lights potentially shining on the vertex.
    /// @return The computed light color.
    Color Lighting::Compute(
        const MATH::Vector3f& world_vertex,
        const MATH::Vector3f& unit_vertex_normal,
        const Material& material,
        const MATH::Vector3f& viewing_world_position,
        const std::vector<Light>& lights,
        const RenderingSettings& rendering_settings)
    {
        Color light_total_color = Color::BLACK;
        for (const Light& light : lights)
        {
            // COMPUTE SHADING BASED ON TYPE OF LIGHT.
            /// @todo   Cleanup.
            if ((LightType::AMBIENT == light.Type) && rendering_settings.LightingSettings.AmbientLightingEnabled)
            {
                if (ShadingType::MATERIAL == material.Shading)
                {
                    light_total_color += Color::ComponentMultiplyRedGreenBlue(light.Color, material.AmbientColor);
                }
                else
                {
                    light_total_color += light.Color;
                }
            }
            else
            {
                // GET THE DIRECTION OF THE LIGHT.
                MATH::Vector3f current_world_vertex = MATH::Vector3f(world_vertex.X, world_vertex.Y, world_vertex.Z);
                MATH::Vector3f direction_from_vertex_to_light;
                if (LightType::DIRECTIONAL == light.Type)
                {
                    // The computations are based on the opposite direction.
                    direction_from_vertex_to_light = MATH::Vector3f::Scale(-1.0f, light.DirectionalLightDirection);
                }
                else if (LightType::POINT == light.Type)
                {
                    direction_from_vertex_to_light = light.PointLightWorldPosition - current_world_vertex;
                }

                // ADD DIFFUSE COLOR FROM THE CURRENT LIGHT.
                // This is based on the Lambertian shading model.
                // An object is maximally illuminated when facing toward the light.
                // An object tangent to the light direction or facing away receives no illumination.
                // In-between, the amount of illumination is proportional to the cosine of the angle between
                // the light and surface normal (where the cosine can be computed via the dot product).
                MATH::Vector3f unit_direction_from_point_to_light = MATH::Vector3f::Normalize(direction_from_vertex_to_light);
                constexpr float NO_ILLUMINATION = 0.0f;
                float illumination_proportion = MATH::Vector3f::DotProduct(unit_vertex_normal, unit_direction_from_point_to_light);
                illumination_proportion = std::max(NO_ILLUMINATION, illumination_proportion);
                Color current_light_color = Color::ScaleRedGreenBlue(illumination_proportion, light.Color);
                /// @todo
                if ((ShadingType::MATERIAL == material.Shading) && rendering_settings.LightingSettings.DiffuseLightingEnabled)
                {
                    light_total_color += Color::ComponentMultiplyRedGreenBlue(current_light_color, material.DiffuseColor);
                }
                else
                {
                    light_total_color += current_light_color;
                }

                // ADD SPECULAR COLOR FROM THE CURRENT LIGHT.
                /// @todo   Is this how we want to handle specularity?
                /// @todo if (material.SpecularPower > 1.0f)
                if (rendering_settings.LightingSettings.SpecularLightingEnabled)
                {
                    MATH::Vector3f reflected_light_along_surface_normal = MATH::Vector3f::Scale(2.0f * illumination_proportion, unit_vertex_normal);
                    MATH::Vector3f reflected_light_direction = reflected_light_along_surface_normal - unit_direction_from_point_to_light;
                    MATH::Vector3f unit_reflected_light_direction = MATH::Vector3f::Normalize(reflected_light_direction);

                    MATH::Vector3f ray_from_vertex_to_camera = viewing_world_position - current_world_vertex;
                    MATH::Vector3f normalized_ray_from_vertex_to_camera = MATH::Vector3f::Normalize(ray_from_vertex_to_camera);
                    float specular_proportion = MATH::Vector3f::DotProduct(normalized_ray_from_vertex_to_camera, unit_reflected_light_direction);
                    specular_proportion = std::max(NO_ILLUMINATION, specular_proportion);
                    specular_proportion = std::pow(specular_proportion, material.SpecularPower);

                    Color current_light_specular_color = Color::ScaleRedGreenBlue(specular_proportion, light.Color);

                    if (ShadingType::MATERIAL == material.Shading)
                    {
                        light_total_color += Color::ComponentMultiplyRedGreenBlue(current_light_specular_color, material.SpecularColor);
                    }
                    else
                    {
                        light_total_color += current_light_specular_color;
                    }
                }
            }
        }

        // RETURN THE COMPUTED LIGHTING COLOR.
        return light_total_color;
    }

    /// Computes lighting for a point.
    /// @param[in]  viewing_point - The point from which the surface is being viewed.
    /// @param[in]  lights - The lights for which to compute lighting at the surface point.
    /// @param[in]  surface - The surface for which lighting is being computed.
    /// @param[in]  surface_point - The point on the surface for which to compute lighting.
    /// @param[in]  lighting_settings - Settings controlling the lighting.
    /// @param[in]  shadow_factors_by_light_index - Shadowing factors to add in additional shadowing
    ///     (0 == full shadowing, 1 == no shadowing).  Must be computed externally since additional
    ///     scene information is needed.
    /// @return The computed light color.
    Color Lighting::Compute(
        const MATH::Vector3f& viewing_point,
        const std::vector<Light>& lights,
        const RAY_TRACING::Surface& surface,
        const MATH::Vector3f& surface_point,
        const LightingSettings& lighting_settings,
        const std::vector<float> shadow_factors_by_light_index)
    {
        // CHECK IF LIGHTING IS ENABLED.
        if (!lighting_settings.Enabled)
        {
            // INDICATE THAT NO LIGHTING EXISTS ON THE SURFACE.
            return Color::BLACK;
        }

        // ENSURE A MATERIAL EXISTS.
        std::shared_ptr<Material> material = surface.GetMaterial();
        ASSERT_THEN_IF_NOT(material)
        {
            // INDICATE THAT NO LIGHTING CAN EXIST WITHOUT A MATERIAL.
            return Color::BLACK;
        }

        // INITIALIZE THE LIGHTING WITH NO LIGHT.
        Color light_total_color = Color::BLACK;

        // ADD LIGHTING FROM ALL LIGHTS.
        // The surface normal is needed for multiple computations.
        MATH::Vector3f unit_surface_normal = surface.GetNormal(surface_point);
        // A viewing ray is needed for certain computations.
        MATH::Vector3f ray_from_surface_point_to_viewing_point = viewing_point - surface_point;
        MATH::Vector3f normalized_ray_from_surface_point_to_viewing_point = MATH::Vector3f::Normalize(ray_from_surface_point_to_viewing_point);
        std::size_t light_count = lights.size();
        for (std::size_t light_index = 0; light_index < light_count; ++light_index)
        {
            // GET THE CURRENT LIGHT.
            const Light& light = lights.at(light_index);

            // ADD IN AMBIENT LIGHTING IF APPLICABLE.
            if (lighting_settings.AmbientLightingEnabled)
            {
                // ONLY ADD IN AMBIENT LIGHTING FOR AMBIENT LIGHTS.
                bool is_ambient_light = (LightType::AMBIENT == light.Type);
                if (is_ambient_light)
                {
                    Color ambient_light_color = Color::ComponentMultiplyRedGreenBlue(
                        material->AmbientColor,
                        light.Color);
                    light_total_color += ambient_light_color;
                }
            }

            // COMPUTE ILLUMINATION AMOUNTS FROM THE CURRENT LIGHT FOR THE POINT.
            // This is needed for multiple types of lighting.
            // The direction may need to be computed differently based on the type of light.
            MATH::Vector3f direction_from_point_to_light;
            if (LightType::DIRECTIONAL == light.Type)
            {
                // The computations are based on the opposite direction.
                direction_from_point_to_light = MATH::Vector3f::Scale(-1.0f, light.DirectionalLightDirection);
            }
            else if (LightType::POINT == light.Type)
            {
                direction_from_point_to_light = light.PointLightWorldPosition - surface_point;
            }

            // The illumination proportion is based on the Lambertian shading model.
            // An object is maximally illuminated when facing toward the light.
            // An object tangent to the light direction or facing away receives no illumination.
            // In-between, the amount of illumination is proportional to the cosine of the angle between
            // the light and surface normal (where the cosine can be computed via the dot product).
            MATH::Vector3f unit_direction_from_point_to_light = MATH::Vector3f::Normalize(direction_from_point_to_light);
            constexpr float NO_ILLUMINATION = 0.0f;
            float illumination_proportion = MATH::Vector3f::DotProduct(unit_surface_normal, unit_direction_from_point_to_light);
            illumination_proportion = std::max(NO_ILLUMINATION, illumination_proportion);

            // GET THE CURRENT LIGHT'S SHADOW FACTOR.
            // If shadow factors weren't provided, then a default of no shadowing will be used.
            constexpr float NO_SHADOWING = 1.0f;
            float shadow_factor = NO_SHADOWING;
            std::size_t shadow_factor_count = shadow_factors_by_light_index.size();
            bool shadow_factor_exists_for_light = (light_index < shadow_factor_count);
            if (shadow_factor_exists_for_light)
            {
                shadow_factor = shadow_factors_by_light_index.at(light_index);
            }

            // ADD IN DIFFUSE LIGHTING IF ENABLED.
            if (lighting_settings.DiffuseLightingEnabled)
            {
                // ADD THE CURRENT LIGHT'S COLOR.
                Color current_light_color = Color::ScaleRedGreenBlue(illumination_proportion, light.Color);
                current_light_color = Color::ScaleRedGreenBlue(shadow_factor, current_light_color);

                // The diffuse color is multiplied component-wise by the amount of light.
                Color diffuse_color = Color::ComponentMultiplyRedGreenBlue(
                    material->DiffuseColor,
                    current_light_color);
                light_total_color += diffuse_color;
            }

            // ADD IN SPECULAR LIGHTING IF ENABLED.
            // This is based on the Blinn-Phong model.
            if (lighting_settings.SpecularLightingEnabled)
            {
                // COMPUTE THE REFLECTED LIGHT DIRECTION.
                MATH::Vector3f reflected_light_along_surface_normal = MATH::Vector3f::Scale(2.0f * illumination_proportion, unit_surface_normal);
                MATH::Vector3f reflected_light_direction = reflected_light_along_surface_normal - unit_direction_from_point_to_light;
                MATH::Vector3f unit_reflected_light_direction = MATH::Vector3f::Normalize(reflected_light_direction);

                // COMPUTE THE SPECULAR AMOUNT.
                float specular_proportion = MATH::Vector3f::DotProduct(normalized_ray_from_surface_point_to_viewing_point, unit_reflected_light_direction);
                specular_proportion = std::max(NO_ILLUMINATION, specular_proportion);
                specular_proportion = std::pow(specular_proportion, material->SpecularPower);

                // ADD THE CURRENT LIGHT'S SPECULAR COLOR.
                float light_proportion = shadow_factor * specular_proportion;
                Color current_light_specular_color = Color::ScaleRedGreenBlue(light_proportion, light.Color);

                // The specular color is multiplied component-wise by the amount of light.
                Color specular_color = Color::ComponentMultiplyRedGreenBlue(
                    material->SpecularColor,
                    current_light_specular_color);
                light_total_color += specular_color;
            }
        }

        // RETURN THE COMPUTED LIGHT COLOR.
        return light_total_color;
    }

#if TODO
    Color Lighting::Compute()
    {

    }
#endif
}
