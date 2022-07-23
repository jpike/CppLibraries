#pragma once

#include "Graphics/Hardware/GraphicsDeviceType.h"
#include "Graphics/Material.h"
#include "Graphics/Shading/Lighting/LightingSettings.h"
#include "Graphics/Shading/ShadingType.h"
#include "Graphics/Viewing/Camera.h"

namespace GRAPHICS
{
    /// Various settings controlling rendering.
    struct RenderingSettings
    {
        /// The type of renderer to use.
        GRAPHICS::HARDWARE::GraphicsDeviceType GraphicsDeviceType = GRAPHICS::HARDWARE::GraphicsDeviceType::CPU_RASTERIZER;
        /// The camera used for rendering.
        GRAPHICS::VIEWING::Camera Camera = {};
        /// The type of shading to use.
        SHADING::ShadingType Shading = SHADING::ShadingType::WIREFRAME;
        /// True if backface culling should occur; false if not.
        bool CullBackfaces = false;
        /// True if depth buffering should be used; false if not.
        bool DepthBuffering = false;
        /// Settings specifically related to lighting.
        SHADING::LIGHTING::LightingSettings LightingSettings = {};
        /// True if reflections should be calculated; false otherwise.
        bool Reflections = true;
        /// The maximum number of reflections to computer (if reflections are enabled).
        /// More reflections will take longer to render an image.
        unsigned int MaxReflectionCount = 5;
        /// True if texture mapping is enabled; false otherwise.
        bool TextureMapping = true;
    };
}
