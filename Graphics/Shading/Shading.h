#pragma once

#include <optional>
#include <vector>
#include "Graphics/Lighting/Light.h"
#include "Graphics/Material.h"
#include "Graphics/VertexWithAttributes.h"
#include "Math/Vector3.h"

namespace GRAPHICS::SHADING
{
    /// Encapsulates domain knowledge for shading algorithms.
    class Shading
    {
    public:
        static Color Compute(
            const VertexWithAttributes& world_vertex,
            const MATH::Vector3f& unit_vertex_normal,
            const Material& material,
            const MATH::Vector3f& viewing_world_position,
            const std::optional<std::vector<LIGHTING::Light>>& lights);
    };
}