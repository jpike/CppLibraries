#pragma once

#include <vector>
#include "Graphics/Color.h"
#include "Graphics/Shading/Lighting/Light.h"
#include "Graphics/Shading/ShadingSettings.h"
#include "Graphics/Surface.h"
#include "Math/Vector3.h"

namespace GRAPHICS::SHADING
{
    /// Encapsulates domain knowledge for shading algorithms done in world-space.
    class WorldSpaceShading
    {
    public:
        // MULTI-LIGHT SHADING.
        static Color ComputeMaterialShading(
            const MATH::Vector3f& surface_point,
            const Surface& surface,
            const MATH::Vector3f& viewing_point,
            const std::vector<LIGHTING::Light>& lights,
            const std::vector<float> shadow_factors_by_light_index,
            const ShadingSettings& shading_settings);

        // SINGLE LIGHT SHADING.
        static Color ComputeMaterialShading(
            const MATH::Vector3f& surface_point,
            const Surface& surface,
            const MATH::Vector3f& viewing_point,
            const LIGHTING::Light& light,
            float shadow_factor,
            const ShadingSettings& shading_settings);
    };
}
