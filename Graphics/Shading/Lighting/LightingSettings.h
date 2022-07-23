#pragma once

namespace GRAPHICS::SHADING::LIGHTING
{
    /// Settings related to lighting.
    struct LightingSettings
    {
        /// True if lighting is enabled; false if not.
        bool Enabled = true;
        /// True if ambient lighting should be calculated; false otherwise.
        bool AmbientLightingEnabled = true;
        /// True if diffuse shading should be calculated; false otherwise.
        bool DiffuseLightingEnabled = true;
        /// True if specular shading should be calculated; false otherwise.
        bool SpecularLightingEnabled = true;
        /// True if shadows should be calculated; false otherwise.
        bool ShadowsEnabled = true;
        /// True if points lights should be visibly rendered (typically for debugging); false if not.
        bool RenderPointLights = false;
    };
}
