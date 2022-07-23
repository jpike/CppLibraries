#pragma once

#include "Graphics/Color.h"
#include "Graphics/Shading/Lighting/Light.h"
#include "Graphics/Shading/ShadingSettings.h"
#include "Graphics/Surface.h"
#include "Math/Vector3.h"

namespace GRAPHICS::SHADING
{
    /// Specular reflection of light that reflects similarly to a mirror to provide shininess on a surface.
    /// This is based on the Blinn-Phong model.
    /// See the following for more details:
    /// - https://en.wikipedia.org/wiki/Specular_reflection
    class SpecularReflection
    {
    public:
        static Color Compute(
            const ShadingSettings& shading_settings,
            const MATH::Vector3f& viewing_point,
            const LIGHTING::Light& light,
            const float shadow_factor,
            const Surface& surface,
            const MATH::Vector3f& surface_point);
    };
}
