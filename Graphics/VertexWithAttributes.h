#pragma once

#include "Graphics/Color.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"

namespace GRAPHICS
{
    /// Vertex data containing both position and other attributes needed for rendering.
    /// See https://en.wikipedia.org/wiki/Vertex_(computer_graphics).
    class VertexWithAttributes
    {
    public:
        // OPERATORS.
        /// Default comparison operator.
        auto operator<=>(const VertexWithAttributes&) const = default;

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// The position of the vertex.
        MATH::Vector3f Position = MATH::Vector3f(0.0f, 0.0f, 0.0f);
        /// The color of the vertex.
        Color Color = GRAPHICS::Color::WHITE;
        /// Any texture coordinates of the vertex.
        MATH::Vector2f TextureCoordinates = MATH::Vector2f(0.0f, 0.0f);
        /// Any normal of the vertex.
        MATH::Vector3f Normal = MATH::Vector3f(0.0f, 0.0f, 0.0f);
    };
}
