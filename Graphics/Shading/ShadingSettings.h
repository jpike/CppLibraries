#pragma once

#include "Graphics/Shading/Lighting/LightingSettings.h"
#include "Graphics/Shading/ShadingType.h"

namespace GRAPHICS::SHADING
{
    /// Settings related specifically to shading.
    struct ShadingSettings
    {
        /// The type of shading to use.
        ShadingType ShadingType = ShadingType::WIREFRAME;
        /// Settings specifically related to lighting.
        LIGHTING::LightingSettings Lighting = {};
        /// True if texture mapping is enabled; false otherwise.
        bool TextureMappingEnabled = true;
    };
}
