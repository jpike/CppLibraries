#include <GL/gl3w.h>
#include "Graphics/OpenGL/ShaderProgram.h"

namespace GRAPHICS::OPEN_GL
{
    const char* ShaderProgram::DEFAULT_VERTEX_SHADER_CODE = R"GLSL( 
#version 420 core

uniform mat4 world_transform;
uniform mat4 view_transform;
uniform mat4 projection_transform;

uniform bool is_lit;
uniform vec4 light_position;
uniform vec4 input_light_color;

in vec4 local_vertex;
in vec4 input_vertex_color;
in vec2 input_texture_coordinates;
in vec3 vertex_normal;

out VERTEX_SHADER_OUTPUT
{
    vec4 color;
    vec2 texture_coordinates;
    vec4 light_color;
} vertex_shader_output;

void main()
{
    vec4 world_vertex = world_transform * local_vertex;
    vec4 view_position = view_transform * world_vertex;
    vec4 projected_vertex = projection_transform * view_position;
    // See https://stackoverflow.com/questions/47233771/negative-values-for-gl-position-w
    // Not entirely sure why I had to do this manually.
    // Might have something to do with my particular projection matrices.
    gl_Position = vec4(
        projected_vertex.x / projected_vertex.w,
        projected_vertex.y / projected_vertex.w, 
        // My particular projection matrices differ a bit from OpenGL conventions.
        -projected_vertex.z / projected_vertex.w, 
        1.0);
    vertex_shader_output.color = input_vertex_color;
    vertex_shader_output.texture_coordinates = input_texture_coordinates;

    if (is_lit)
    {
        vec3 direction_from_vertex_to_light = light_position.xyz - world_vertex.xyz;
        vec3 unit_direction_from_point_to_light = normalize(direction_from_vertex_to_light);
        float illumination_proportion = dot(vertex_normal.xyz, unit_direction_from_point_to_light);
        float clamped_illumination = max(0, illumination_proportion);
        vec3 scaled_light_color = clamped_illumination * input_light_color.xyz;
        vertex_shader_output.light_color = vec4(scaled_light_color.rgb, 1.0);
    }
    else
    {
        vertex_shader_output.light_color = vec4(1.0, 1.0, 1.0, 1.0);
    }
}
)GLSL";

    const char* ShaderProgram::DEFAULT_FRAGMENT_SHADER_CODE = R"GLSL(
#version 420 core

uniform bool is_textured;
uniform sampler2D texture_sampler;

in VERTEX_SHADER_OUTPUT
{
    vec4 color;
    vec2 texture_coordinates;
    vec4 light_color;
} fragment_shader_input;

out vec4 fragment_color;

void main()
{
    if (is_textured)
    {
        vec4 texture_color = texture(texture_sampler, fragment_shader_input.texture_coordinates);
        vec4 lit_texture_color = texture_color * fragment_shader_input.light_color;
        /// @todo   Color components swapped for some reason.
        //fragment_color = vec4(lit_texture_color.xyz, 1.0);
        fragment_color = vec4(lit_texture_color.wzy, 1.0);
    }
    else
    {
        vec4 lit_color = fragment_shader_input.color * fragment_shader_input.light_color;
        /// @todo   Color components swapped for some reason.
        //fragment_color = vec4(lit_color.xyz, 1.0);
        fragment_color = vec4(lit_color.wzy, 1.0);
    }
}
)GLSL";;

    /// Checks if shader compilation succeeded, provided debug output if something fails.
    /// @param[in]  shader_id - The ID of the shader to check.
    void ShaderProgram::CheckShaderCompilation(const GLuint shader_id)
    {
        GLint shader_compile_status;
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &shader_compile_status);
        bool shader_compiled = (GL_TRUE == shader_compile_status);
        if (!shader_compiled)
        {
            // GET THE SHADER COMPILER ERROR.
            GLsizei* LENGTH_OF_LOG_NOT_NEEDED = NULL;
            char shader_compile_log_buffer[512];
            glGetShaderInfoLog(
                shader_id,
                sizeof(shader_compile_log_buffer) / sizeof(shader_compile_log_buffer[0]),
                LENGTH_OF_LOG_NOT_NEEDED,
                shader_compile_log_buffer);
            /// @todo   Log via a better mechanism.
            OutputDebugString("Shader compile error: ");
            OutputDebugString(shader_compile_log_buffer);
        }
    }

    /// Attempts to build a shader program.
    /// @param[in]  vertex_shader_code - The vertex shader code.
    /// @param[in]  fragment_shader_code - The fragment shader code.
    /// @return The built shader program; null if an error occurs.
    /// @todo   Error handling for failures?
    std::shared_ptr<ShaderProgram> ShaderProgram::Build(
        const char* const vertex_shader_code,
        const char* const fragment_shader_code)
    {
        // COMPILE THE VERTEX SHADER.
        GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
        const GLsizei SINGLE_SOURCE_CODE_STRING = 1;
        const GLint* const SOURCE_CODE_IS_NULL_TERMINATED = NULL;
        glShaderSource(vertex_shader_id, SINGLE_SOURCE_CODE_STRING, &vertex_shader_code, SOURCE_CODE_IS_NULL_TERMINATED);
        glCompileShader(vertex_shader_id);
        CheckShaderCompilation(vertex_shader_id);

        // COMPILE THE FRAGMENT SHADER.
        GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader_id, SINGLE_SOURCE_CODE_STRING, &fragment_shader_code, SOURCE_CODE_IS_NULL_TERMINATED);
        glCompileShader(fragment_shader_id);
        CheckShaderCompilation(fragment_shader_id);

        // CREATE THE COMBINED PROGRAM.
        auto shader_program = std::make_shared<ShaderProgram>();
        shader_program->Id = glCreateProgram();
        glAttachShader(shader_program->Id, vertex_shader_id);
        glAttachShader(shader_program->Id, fragment_shader_id);
        glLinkProgram(shader_program->Id);

        // FREE RESOURCES NO LONGER NEEDED.
        // They're stored with the shader program now.
        glDeleteShader(fragment_shader_id);
        glDeleteShader(vertex_shader_id);

        return shader_program;
    }

    /// Destructor to free shader program resources.
    /// @todo   Should this really be done automatically as opposed to
    ///     explicitly via the graphics device?
    ShaderProgram::~ShaderProgram()
    {
        glDeleteProgram(Id);
    }
}
