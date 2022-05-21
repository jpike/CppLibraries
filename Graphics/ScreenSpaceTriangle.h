#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include "Graphics/Material.h"
#include "Graphics/VertexWithAttributes.h"

namespace GRAPHICS
{
    /// A triangle that has been transformed into screen-space and is ready to be
    /// rasterized directly to a 2D pixel grid.  This separate representation of
    /// a triangle helps simplify passing of data to 2D rasterization algorithms.
    struct ScreenSpaceTriangle
    {
        // STATIC CONSTANTS.
        /// The number of vertices in a triangle.
        static constexpr std::size_t VERTEX_COUNT = 3;

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// The material of the triangle.
        std::shared_ptr<class Material> Material = nullptr;
        /// The vertices of the triangle.  Should be in counter-clockwise order.
        /// Z position coordinats are included to support depth-testing.
        std::array<VertexWithAttributes, VERTEX_COUNT> Vertices = std::array<VertexWithAttributes, VERTEX_COUNT>();
    };
}
