#pragma once

/// Holds code related to lighting (https://en.wikipedia.org/wiki/Computer_graphics_lighting).
namespace GRAPHICS::SHADING::LIGHTING
{
    /// The type of light.
    enum class LightType
    {
        /// Ambient light that basically illuminates an entire scene.
        AMBIENT = 0,
        /// Directional light that has a consistent direction and appears infinitely far away.
        DIRECTIONAL,
        /// A point light with a position in space and provides illumination in all directions
        POINT
    };
}
