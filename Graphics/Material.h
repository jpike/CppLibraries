#pragma once

#include <memory>
#include <string>
#include "Graphics/Color.h"
#include "Graphics/Shading/ColoredTexturedSurfaceProperties.h"
#include "Graphics/Shading/ShadingType.h"
#include "Graphics/Shading/SpecularSurfaceProperties.h"

namespace GRAPHICS
{
    /// A material defining properties of a surface and how it's shaded.
    class Material
    {
    public:
        /// An identifying name of the material.
        std::string Name = "";

        /// The type of shading for the material.
        SHADING::ShadingType Shading = SHADING::ShadingType::WIREFRAME;

        /// Ambient properties of the material.
        SHADING::ColoredTexturedSurfaceProperties AmbientProperties = {};
        /// Diffuse properties of the material.
        SHADING::ColoredTexturedSurfaceProperties DiffuseProperties = {};
        /// Specular properties of the material.
        SHADING::SpecularSurfaceProperties SpecularProperties = {};

        /// How reflective the material is as a proportion from [0, 1].
        float ReflectivityProportion = 0.0f;
        /// The emissive color if the material emits light.
        Color EmissiveColor = Color::BLACK;
    };
}
