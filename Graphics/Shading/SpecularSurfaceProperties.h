#pragma once

#include <memory>
#include <gl3w/GL/glcorearb.h>
#include "Graphics/Color.h"
#include "Graphics/Images/Bitmap.h"

namespace GRAPHICS::SHADING
{
    /// Properties of a surface that can exhibit specular reflection (https://en.wikipedia.org/wiki/Specular_reflection).
    /// This struct helps encapsulate relevant properties.
    struct SpecularSurfaceProperties
    {
        /// The specular color of the surface.
        /// In addition to directly coloring a surface, this can be used to tint any texture.
        Color Color = Color::BLACK;
        /// The specular power defining the shininess of specular highlights.
        float SpecularPower = 0.0f;
        /// Any texture defining the specular look of the surface.
        std::shared_ptr<IMAGES::Bitmap> Texture = nullptr;
        /// Any OpenGL resource for the texture.
        GLuint OpenGLTextureId = 0;
    };
}
