#pragma once

#include <vector>
#include "Graphics/Color.h"
#include "Graphics/Material.h"
#include "Graphics/RenderingSettings.h"
#include "Graphics/Shading/Lighting/Light.h"
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
            const std::vector<LIGHTING::Light>& lights,
            const RenderingSettings& rendering_settings);
    };
}
