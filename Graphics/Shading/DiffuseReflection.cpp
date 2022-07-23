#include <algorithm>
#include "ErrorHandling/Asserts.h"
#include "Graphics/Shading/DiffuseReflection.h"

namespace GRAPHICS::SHADING
{
    /// Computes the diffuse reflection for a single light for a particular surface point.
    /// @param[in]  light - The light for which to compute the diffuse reflection at the surface point.
    /// @param[in]  shadow_factor - A shadowing factor to add in additional shadowing
    ///     (0 == full shadowing, 1 == no shadowing).  If desired, this must be computed and passed in externally
    ///     as shadowing imposes too high a cost to dynamically compute all the time.
    /// @param[in]  surface - The surface for which diffuse reflection is being computed.
    /// @param[in]  surface_point - The point on the surface for which to compute the diffuse reflection.
    /// @return The color of diffuse reflection from the light at the specified surface point.
    Color DiffuseReflection::Compute(
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
            // Other types of light do not result in any diffuse reflection.
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

        // COMPUTE THE AMOUNT OF LIGHT SHINING ON THE SURFACE.
        Color current_light_color = Color::ScaleRedGreenBlue(illumination_proportion, light.Color);
        current_light_color = Color::ScaleRedGreenBlue(shadow_factor, current_light_color);

        // COMPUTE THE DIFFUSE COLOR REFLECTED ON THE SURFACE.
        Color diffuse_color = Color::ComponentMultiplyRedGreenBlue(material->DiffuseProperties.Color, current_light_color);
        return diffuse_color;
    }
}
