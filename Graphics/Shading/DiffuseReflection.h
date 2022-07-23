#pragma once

#include "Graphics/Color.h"
#include "Graphics/Shading/Lighting/Light.h"
#include "Graphics/Surface.h"
#include "Math/Vector3.h"

namespace GRAPHICS::SHADING
{
    /// Diffuse reflection of light that scatters approximately equally in all directions on a surface.
    /// See the following more more details:
    /// - https://en.wikipedia.org/wiki/Diffuse_reflection
    /// - https://en.wikipedia.org/wiki/Lambertian_reflectance
    class DiffuseReflection
    {
    public:
        static Color Compute(
            const LIGHTING::Light& light,
            const float shadow_factor,
            const Surface& surface,
            const MATH::Vector3f& surface_point);
    };
}
