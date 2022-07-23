#include "Graphics/Shading/AmbientShading.h"
#include "Graphics/TextureMappingAlgorithm.h"

namespace GRAPHICS::SHADING
{
    /// Computes the ambient shading for a single light for a particular surface point.
    /// @param[in]  shading_settings - Settings affecting the shading.
    /// @param[in]  ambient_light_color - The color of ambient light shining on the surface.
    /// @param[in]  surface - The surface for which ambient shading is being computed.
    /// @param[in]  surface_point - The point on the surface for which to compute the ambient shading.
    /// @return The color of ambient shading from the light at the specified surface point.
    Color AmbientShading::Compute(
        const ShadingSettings& shading_settings,
        const Color& ambient_light_color,
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

        // GET THE RAW AMBIENT SURFACE COLOR.
        Color ambient_surface_color = material->AmbientProperties.Color;

        // ADD IN TEXTURE MAPPING IF ENABLED.
        /// @todo   Figure out better way to handle this triangle texture mapping stuff!
        const GEOMETRY::Triangle* const* triangle = std::get_if<const GEOMETRY::Triangle*>(&surface.Shape);
        if (shading_settings.TextureMappingEnabled && triangle && material->AmbientProperties.Texture)
        {
            MATH::Vector2f triangle_point(surface_point.X, surface_point.Y);
            Color texture_color = TextureMappingAlgorithm::LookupTexel(
                **triangle,
                triangle_point,
                *material->AmbientProperties.Texture);
            ambient_surface_color = Color::ComponentMultiplyRedGreenBlue(ambient_surface_color, texture_color);
        }

        // COMPUTE THE FINAL AMBIENT COLOR INCORPORATING LIGHTING.
        Color ambient_color = Color::ComponentMultiplyRedGreenBlue(ambient_surface_color, ambient_light_color);
        return ambient_color;
    }
}