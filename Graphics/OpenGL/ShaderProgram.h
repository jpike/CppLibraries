#pragma once

#include <memory>
#include <gl/GL.h>

namespace GRAPHICS::OPEN_GL
{
    /// An OpenGL shader program.
    class ShaderProgram
    {
    public:
        // STATIC CONSTANTS.
        /// The default vertex shader code.
        static const char* DEFAULT_VERTEX_SHADER_CODE;
        /// The default fragment shader code.
        static const char* DEFAULT_FRAGMENT_SHADER_CODE;

        // STATIC HELPER METHODS.
        static void CheckShaderCompilation(const GLuint shader_id);

        // CONSTRUCTION/DESTRUCTION.
        static std::shared_ptr<ShaderProgram> Build(
            const char* const vertex_shader_code,
            const char* const fragment_shader_code);
        ~ShaderProgram();

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// The OpenGL ID for the program (if it's an OpenGL program).
        GLuint Id = 0;
    };
}
