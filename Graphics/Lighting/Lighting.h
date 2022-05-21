#pragma once

#include <vector>
#include "Graphics/Color.h"
#include "Graphics/Lighting/Light.h"
#include "Graphics/Material.h"
#include "Math/Vector3.h"

namespace GRAPHICS::LIGHTING
{
    /// Encapsulates domain knowledge for lighting algorithms.
    class Lighting
    {
    public:
        static Color Compute(
            const MATH::Vector3f& world_vertex,
            const MATH::Vector3f& unit_vertex_normal,
            const Material& material,
            const MATH::Vector3f& viewing_world_position,
            const std::vector<Light>& lights);
    };
}
