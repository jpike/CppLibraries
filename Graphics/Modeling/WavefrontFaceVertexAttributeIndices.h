#pragma once

#include <cstddef>

namespace GRAPHICS::MODELING
{
    /// Indices for vertex attributes for a face as specified in a Wavefront object model.
    /// This helps with properly parsing a Wavefront .obj file and matching up attributes for face.
    /// The first index is 1 and increases corresponding to order items previously defined.
    struct WavefrontFaceVertexAttributeIndices
    {
        /// The value for an unset index for an attribute.
        /// Unset means there is no corresponding value for that attribute.
        constexpr static std::size_t UNSET_INDEX = 0;
        /// Since Wavefront indices start at 1, there is an offset from normal 0-based indices.
        constexpr static std::size_t OFFSET_FROM_ZERO_BASED_INDEX = 1;

        /// The index of the vertex position.
        std::size_t VertexPositionIndex = UNSET_INDEX;
        /// The index of the vertex color.
        std::size_t VertexColorIndex = UNSET_INDEX;
        /// The index of the vertex texture coordinates.
        std::size_t VertexTextureCoordinateIndex = UNSET_INDEX;
        /// The index of the vertex normal.
        std::size_t VertexNormalIndex = UNSET_INDEX;
    };
}
