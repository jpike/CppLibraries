#pragma once

#include <memory>
#include <gl3w/GL/glcorearb.h>
#include "Graphics/Color.h"
#include "Graphics/Images/Bitmap.h"

namespace GRAPHICS::SHADING
{
    /// Properties of a surface that can both be colored and textured.
    /// This struct helps encapsulate this combination for different
    /// kinds of shading (ambient, diffuse, etc.).
    struct ColoredTexturedSurfaceProperties
    {
        /// The color of the surface.
        /// In addition to directly coloring a surface, this can be used to tint any texture.
        Color Color = Color::BLACK;
        /// Any texture defining the look of the surface.
        std::shared_ptr<IMAGES::Bitmap> Texture = nullptr;
        /// Any OpenGL resource for the texture.
        GLuint OpenGLTextureId = 0;
    };
}
