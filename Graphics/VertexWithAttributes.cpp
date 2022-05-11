// This code is only compiled in if the bgfx library is available to allow
// the larger graphics library to be used without bgfx.
#if __has_include(<bgfx/bgfx.h>)

#include "Graphics/VertexWithAttributes.h"

namespace GRAPHICS
{
    /// Gets the layout for this type of vertex.
    /// A reference is returned to enable the reference to be statically allocated once and
    /// then re-ued without needing to allocate duplicate versions of the same layout.
    /// @return The layout for this type of vertex.
    bgfx::VertexLayout& VertexWithAttributes::Layout()
    {
        // STATICALLY ALLOCATE THE VERTEX LAYOUT SO THAT IT IS ONLY ALLOCATED ONCE.
        static bgfx::VertexLayout vertex_layout;

        // INITIALIZE THE VERTEX LAYOUT ONLY IF IT HAS NOT YET BEEN INITIALIZED.
        bool vertex_layout_initialized = (0 != vertex_layout.m_stride);
        if (!vertex_layout_initialized)
        {
            constexpr bool NORMALIZE_TO_FLOAT = true;
            vertex_layout
                .begin()
                // 3 position components (X, Y, Z).
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                // 2 texture coordinate components (U, V).
                .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
                // 4 color components (A, R, G, B), each as an 8-bit integer.
                // These will be converted to normalized [0, 1] floats in shaders.
                .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, NORMALIZE_TO_FLOAT)
                .end();
        }

        // RETURN THE VERTEX LAYOUT.
        return vertex_layout;
    }
}

#endif
