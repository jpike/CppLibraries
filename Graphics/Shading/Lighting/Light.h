#pragma once

#include "Graphics/Color.h"
#include "Graphics/Shading/Lighting/LightType.h"
#include "Math/Vector3.h"

namespace GRAPHICS::SHADING::LIGHTING
{
    /// A light that an provide illumination in a 3D scene.
    class Light
    {
    public:
        MATH::Vector3f PointLightDirectionFrom(const MATH::Vector3f& other_world_position) const;

        /// The type of the light.
        LightType Type = LightType::AMBIENT;
        /// The color of the light.
        GRAPHICS::Color Color = GRAPHICS::Color::BLACK;
        /// The direction for a directional light.
        MATH::Vector3f DirectionalLightDirection = MATH::Vector3f();
        /// The world position for a point light.
        MATH::Vector3f PointLightWorldPosition = MATH::Vector3f();
    };
}
