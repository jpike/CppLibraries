#pragma once

namespace GRAPHICS::HARDWARE
{
    /// Different types of graphics devices.
    enum GraphicsDeviceType
    {
        /// Invalid value to effectively indicate "no" rendering and provide a reasonable default 0 value.
        NONE = 0,

        /// A graphics device using CPU rasterization for rendering.
        CPU_RASTERIZER,
        /// A graphics device using CPU ray tracing for rendering.
        CPU_RAY_TRACER,
        /// A graphics device using OpenGL for rendering.
        OPEN_GL,
        /// A graphics device using Direct3D for rendering.
        DIRECT_3D,
    };
}
