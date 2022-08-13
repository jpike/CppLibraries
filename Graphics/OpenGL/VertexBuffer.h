#pragma once

#include <vector>
#include <gl3w/GL/glcorearb.h>
#include "Graphics/Modeling/Model.h"

namespace GRAPHICS::OPEN_GL
{
    /// A buffer on a graphics device for holding vertices.
    /// This class holds both OpenGL vertex array and buffer IDs
    /// since it's easier to think of them together when thinking
    /// about how vertices get passed to graphics hardware,
    /// and any additional flexibility for keeping them separate
    /// has not yet been needed.
    class VertexBuffer
    {
    public:
        // DESTRUCTOR.
        ~VertexBuffer();

        // PUBLIC METHODS.
        void Fill(const MODELING::Model& model);

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// The ID of the vertex array associated with this buffer.
        GLuint ArrayId = 0;
        /// The ID of the vertex buffer.
        GLuint BufferId = 0;
    };
}
