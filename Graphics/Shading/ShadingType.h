#pragma once

/// Holds code related to shading (https://en.wikipedia.org/wiki/Shading).
namespace GRAPHICS::SHADING
{
    /// The different kinds of shading that are supported.
    enum class ShadingType
    {
        /// Objects are rendered as wireframes.
        /// Colors will be interpolated across vertices.
        /// Defaults to wireframe to ensure at least something gets rendered.
        WIREFRAME = 0,
        /// Objects are rendered with a solid, flat color.
        FLAT,
        /// Base shading the the material, with interpolation across the face and texture mapping if applicable.
        MATERIAL,
        /// An extra enum to indicate the number of different shading types.
        COUNT
    };
}