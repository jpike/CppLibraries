#include "Graphics/Shading/WorldSpaceShading.h"

namespace GRAPHICS::SHADING
{
    /// Computes material-based shading for a point.
    /// @param[in]  surface_point - The point on the surface for which to compute shading.
    /// @param[in]  surface - The surface for which shading is being computed.
    /// @param[in]  viewing_point - The point from which the surface is being viewed.
    /// @param[in]  lights - The lights for which to compute shading at the surface point.
    /// @param[in]  shadow_factors_by_light_index - Shadowing factors to add in additional shadowing
    ///     (0 == full shadowing, 1 == no shadowing).  Must be computed externally since additional
    ///     scene information is needed.
    /// @param[in]  shading_settings - Settings controlling the shading.
    /// @return The computed light color.
    Color WorldSpaceShading::ComputeMaterialShading(
        const MATH::Vector3f& surface_point,
        const Surface& surface,
        const MATH::Vector3f& viewing_point,
        const std::vector<LIGHTING::Light>& lights,
        const std::vector<float> shadow_factors_by_light_index,
        const ShadingSettings& shading_settings)
    {
        // CHECK IF LIGHTING IS ENABLED.
        if (!shading_settings.Lighting.Enabled)
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
        std::size_t light_count = lights.size();
        for (std::size_t light_index = 0; light_index < light_count; ++light_index)
        {
            // GET THE CURRENT LIGHT.
            const LIGHTING::Light& light = lights.at(light_index);

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

            // ADD LIGHTING FROM THE CURRENT LIGHT.
            Color current_light_color = ComputeMaterialShading(
                surface_point,
                surface,
                viewing_point,
                light,
                shadow_factor,
                shading_settings);
            light_total_color += current_light_color;
        }

        // RETURN THE COMPUTED LIGHT COLOR.
        return light_total_color;
    }

    /// Computes material-based shading for a single light for a particular surface point.
    /// @param[in]  surface_point - The point on the surface for which to compute shading.
    /// @param[in]  surface - The surface for which lighting is being computed.
    /// @param[in]  viewing_point - The point from which the surface is being viewed.
    /// @param[in]  light - The light for which to compute shading at the surface point.
    /// @param[in]  shadow_factor - A shadowing factor to add in additional shadowing
    ///     (0 == full shadowing, 1 == no shadowing).
    /// @param[in]  shading_settings - Settings controlling the shading.
    /// @return The computed light color.
    Color WorldSpaceShading::ComputeMaterialShading(
        const MATH::Vector3f& surface_point,
        const Surface& surface,
        const MATH::Vector3f& viewing_point,
        const LIGHTING::Light& light,
        float shadow_factor,
        const ShadingSettings& shading_settings)
    {
        // ENSURE A MATERIAL EXISTS.
        std::shared_ptr<Material> material = surface.GetMaterial();
        ASSERT_THEN_IF_NOT(material)
        {
            // INDICATE THAT NO LIGHTING CAN EXIST WITHOUT A MATERIAL.
            return Color::BLACK;
        }

        // INITIALIZE THE LIGHTING WITH NO LIGHT.
        Color light_total_color = Color::BLACK;

        // ADD IN AMBIENT LIGHTING IF APPLICABLE.
        bool is_ambient_light = (LIGHTING::LightType::AMBIENT == light.Type);
        if (is_ambient_light)
        {
            if (shading_settings.Lighting.AmbientLightingEnabled)
            {
                Color ambient_color = AmbientShading::Compute(
                    shading_settings,
                    light.Color,
                    surface,
                    surface_point);
                light_total_color += ambient_color;
            }

            // RETURN THE CURRENT LIGHTING IF THE LIGHT IS AN AMBIENT LIGHT.
            // Ambient lights should not contribute to other kinds of lighting.
            return light_total_color;
        }

        // ADD IN DIFFUSE LIGHTING IF ENABLED.
        if (shading_settings.Lighting.DiffuseLightingEnabled)
        {
            Color diffuse_color = DiffuseReflection::Compute(
                shading_settings,
                light,
                shadow_factor,
                surface,
                surface_point);

            light_total_color += diffuse_color;
        }

        // ADD IN SPECULAR LIGHTING IF ENABLED.
        if (shading_settings.Lighting.SpecularLightingEnabled)
        {
            Color specular_color = SpecularReflection::Compute(
                shading_settings,
                viewing_point,
                light,
                shadow_factor,
                surface,
                surface_point);

            light_total_color += specular_color;
        }

        // RETURN THE COMPUTED LIGHT COLOR.
        return light_total_color;
    }
}
