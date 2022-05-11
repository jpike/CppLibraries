#pragma once

#include <cstdint>
// Parts of this code are only compiled in if the bgfx library is available to allow
// the larger graphics library to be used without bgfx.
#if __has_include(<bgfx/bgfx.h>)
#include <bgfx/bgfx.h>
#endif

namespace GRAPHICS
{
    /// Vertex data containing both position and other attributes needed for rendering.
    /// See https://en.wikipedia.org/wiki/Vertex_(computer_graphics).
    class VertexWithAttributes
    {
    public:
        // METHODS.
        #if __has_include(<bgfx/bgfx.h>)
        static bgfx::VertexLayout& Layout();
        #endif

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// The X position of the vertex.
        float X = 0.0f;
        /// The Y position of the vertex.
        float Y = 0.0f;
        /// The Z position of the vertex.
        float Z = 0.0f;
        /// The texture U (horiziontal) coordinate.
        float TextureU = 0.0f;
        /// The texture V (vertical) coordinate.
        float TextureV = 0.0f;
        /// The color in a packed ARGB format (8 bits per color component).
        uint32_t ColorAlphaRedGreenBlue = 0;
    };
}
