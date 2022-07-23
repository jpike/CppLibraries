#pragma once

#include "Graphics/Color.h"
#include "Graphics/Shading/ShadingSettings.h"
#include "Graphics/Surface.h"
#include "Math/Vector3.h"

namespace GRAPHICS::SHADING
{
    /// Ambient shading that can be computed for a surface.
    class AmbientShading
    {
    public:
        static Color Compute(
            const ShadingSettings& shading_settings,
            const Color& ambient_light_color,
            const Surface& surface,
            const MATH::Vector3f& surface_point);
    };
}
