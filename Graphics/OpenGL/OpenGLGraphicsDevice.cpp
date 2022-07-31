#include <vector>
#include <gl3w/GL/gl3w.h>
#include "ErrorHandling/Asserts.h"
#include "Graphics/OpenGL/OpenGL.h"
#include "Graphics/OpenGL/OpenGLGraphicsDevice.h"
#include "Graphics/Shading/Lighting/Light.h"
#include "Graphics/Viewing/ViewingTransformations.h"
#include "Windowing/Win32Window.h"

static const char* DEFAULT_VERTEX_SHADER = R"GLSL( 
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
in vec4 vertex_normal;

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

static const char* DEFAULT_FRAGMENT_SHADER = R"GLSL(
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
)GLSL";

namespace GRAPHICS::OPEN_GL
{
    /// Attempts to connect a graphics device to the specified window for rendering via the OpenGL.
    /// @param[in,out]  window - The window in which to do OpenGL rendering.  Non-const since non-const access is sometimes needed.
    /// return  The OpenGL graphics device, if successfully connected to the window; null if an error occurs.
    std::unique_ptr<OpenGLGraphicsDevice> OpenGLGraphicsDevice::ConnectTo(WINDOWING::IWindow& window)
    {
        WINDOWING::Win32Window& win32_window = dynamic_cast<WINDOWING::Win32Window&>(window);
        HDC window_device_context = GetDC(win32_window.WindowHandle);
        ASSERT_THEN_IF(window_device_context)
        {
            bool open_gl_initialized = GRAPHICS::OPEN_GL::Initialize(window_device_context);
            ASSERT_THEN_IF(open_gl_initialized)
            {
                // CREATE THE OPEN GL RENDERING CONTEXT.
                const HGLRC NO_CONTEXT_TO_SHARE_WITH = nullptr;
                const int context_attribute_list[] =
                {
                    WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
                    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                    ATTRIBUTE_LIST_TERMINATOR
                };
                HGLRC open_gl_render_context = wglCreateContextAttribsARB(window_device_context, NO_CONTEXT_TO_SHARE_WITH, context_attribute_list);
                BOOL open_gl_context_made_current = wglMakeCurrent(window_device_context, open_gl_render_context);
                if (!open_gl_context_made_current)
                {
                    return nullptr;
                }

                MessageBoxA(0, (char*)glGetString(GL_VERSION), "OPENGL VERSION", 0);

                unsigned int width_in_pixels = window.GetWidthInPixels();
                unsigned int height_in_pixels = window.GetHeightInPixels();
                glViewport(0, 0, width_in_pixels, height_in_pixels);

                auto graphics_device = std::make_unique<OpenGLGraphicsDevice>();

                graphics_device->Window = &window;
                graphics_device->WindowDeviceContext = window_device_context;
                graphics_device->OpenGLRenderContext = open_gl_render_context;

                graphics_device->ShaderProgram = GRAPHICS::OPEN_GL::ShaderProgram::Build(
                    DEFAULT_VERTEX_SHADER,
                    DEFAULT_FRAGMENT_SHADER);

                return graphics_device;
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }

    /// Shutdowns the graphics device, freeing up allocated resources.
    void OpenGLGraphicsDevice::Shutdown()
    {
        // DELETE THE RENDERING CONTEXT.
        ShaderProgram.reset();
        wglDeleteContext(OpenGLRenderContext);
        OpenGLRenderContext = nullptr;
        WindowDeviceContext = nullptr;
    }

    /// Shuts down the graphics device to ensure all resources are freed.
    OpenGLGraphicsDevice::~OpenGLGraphicsDevice()
    {
        Shutdown();
    }

    /// Gets the type of the graphics device.
    /// @return The type of the graphics device.
    GRAPHICS::HARDWARE::GraphicsDeviceType OpenGLGraphicsDevice::Type() const
    {
        return GRAPHICS::HARDWARE::GraphicsDeviceType::OPEN_GL;
    }

    /// Does nothing since no additional loading is needed for 3D objects on the CPU.
    /// @param[in,out]  object_3D - The object to load.  Nothing is done since the object is already loaded into CPU memory.
    void OpenGLGraphicsDevice::Load(GRAPHICS::Object3D& object_3D)
    {
        // Reference the parameter to avoid compiler warnings.
        object_3D;
    }

    /// Clears the background screen of the graphics device in preparation for new rendering.
    /// @param[in]  color - The background color to clear to.
    void OpenGLGraphicsDevice::ClearBackground(const GRAPHICS::Color& color)
    {
        GLfloat background_color[] = { color.Red, color.Green, color.Blue, color.Alpha };
        const GLint NO_SPECIFIC_DRAW_BUFFER = 0;
        glClearBufferfv(GL_COLOR, NO_SPECIFIC_DRAW_BUFFER, background_color);

        /// @todo   Not sure if this is fully correct.
        GLfloat max_depth[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glClearBufferfv(GL_DEPTH, NO_SPECIFIC_DRAW_BUFFER, max_depth);
    }

    /// Renders the specified scene using the graphics device.
    /// @param[in]  scene - The scene to render.
    /// @param[in]  camera - The camera through which the scene is being viewed.
    /// @param[in]  rendering_settings - The settings to use for rendering.
    /// @todo   Use all of rendering settings.
    void OpenGLGraphicsDevice::Render(
        const GRAPHICS::Scene& scene,
        const VIEWING::Camera& camera,
        const GRAPHICS::RenderingSettings& rendering_settings)
    {
        if (rendering_settings.DepthBuffering)
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }

        ClearBackground(scene.BackgroundColor);

        VIEWING::ViewingTransformations viewing_transformations(camera);

        std::optional< std::vector<GRAPHICS::SHADING::LIGHTING::Light>> lights = std::vector<GRAPHICS::SHADING::LIGHTING::Light>();
        lights->emplace_back(
            GRAPHICS::SHADING::LIGHTING::Light
            {
                .Type = GRAPHICS::SHADING::LIGHTING::LightType::POINT,
                .Color = GRAPHICS::Color(1.0f, 1.0f, 1.0f, 1.0f),
                .PointLightWorldPosition = MATH::Vector3f(0.0f, 0.0f, 5.0f)
            });

        // USE THE OBJECT'S SHADER PROGRAM.
        glUseProgram(ShaderProgram->Id);

        // RENDER EACH OBJECT.
        for (const auto& object_3D : scene.Objects)
        {
            // SET UNIFORMS.
            GLint world_matrix_variable = glGetUniformLocation(ShaderProgram->Id, "world_transform");
            MATH::Matrix4x4f world_transform = object_3D.WorldTransform();
            const float* world_matrix_elements_in_row_major_order = world_transform.ElementsInRowMajorOrder();
            const GLsizei ONE_MATRIX = 1;
            const GLboolean ROW_MAJOR_ORDER = GL_TRUE;
            glUniformMatrix4fv(world_matrix_variable, ONE_MATRIX, ROW_MAJOR_ORDER, world_matrix_elements_in_row_major_order);

            GLint view_matrix_variable = glGetUniformLocation(ShaderProgram->Id, "view_transform");
            const float* view_matrix_elements_in_row_major_order = viewing_transformations.CameraViewTransform.ElementsInRowMajorOrder();
            glUniformMatrix4fv(view_matrix_variable, ONE_MATRIX, ROW_MAJOR_ORDER, view_matrix_elements_in_row_major_order);

            GLint projection_matrix_variable = glGetUniformLocation(ShaderProgram->Id, "projection_transform");
            const float* projection_matrix_elements_in_row_major_order = viewing_transformations.CameraProjectionTransform.ElementsInRowMajorOrder();
            glUniformMatrix4fv(projection_matrix_variable, ONE_MATRIX, ROW_MAJOR_ORDER, projection_matrix_elements_in_row_major_order);

            GLint texture_sampler_variable = glGetUniformLocation(ShaderProgram->Id, "texture_sampler");
            glUniform1i(texture_sampler_variable, 0);

            bool is_lit = lights.has_value();
            GLint is_lit_variable = glGetUniformLocation(ShaderProgram->Id, "is_lit");
            glUniform1i(is_lit_variable, is_lit);

            if (is_lit)
            {
                // Single arbitrary light for now.
                const SHADING::LIGHTING::Light& first_light = lights->at(0);

                GLint light_position_variable = glGetUniformLocation(ShaderProgram->Id, "light_position");
                glUniform4f(
                    light_position_variable,
                    first_light.PointLightWorldPosition.X,
                    first_light.PointLightWorldPosition.Y,
                    first_light.PointLightWorldPosition.Z,
                    1.0f);

                GLint light_color_variable = glGetUniformLocation(ShaderProgram->Id, "input_light_color");
                glUniform4f(
                    light_color_variable,
                    first_light.Color.Red,
                    first_light.Color.Green,
                    first_light.Color.Blue,
                    first_light.Color.Alpha);
            }

            /// @todo   Pass vertices for entire object at once!
            /// @todo   Look at https://github.com/jpike/OpenGLEngine/ for possible better handling of some stuff?
            for (const auto& [mesh_name, mesh] : object_3D.Model.MeshesByName)
            {
                for (const auto& triangle : mesh.Triangles)
                {
                    // ALLOCATE A TEXTURE IF APPLICABLE.
                    // Must be done outside of glBegin()/glEnd() (http://docs.gl/gl2/glGenTextures).
                    GLuint texture = 0;
                    bool is_textured = (nullptr != triangle.Material->DiffuseProperties.Texture);
                    if (is_textured)
                    {
                        glGenTextures(1, &texture);
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, texture);

                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

                        glTexImage2D(
                            GL_TEXTURE_2D,
                            0, // level of detail
                            GL_RGBA, // this is the only thing we currently support
                            triangle.Material->DiffuseProperties.Texture->GetWidthInPixels(),
                            triangle.Material->DiffuseProperties.Texture->GetHeightInPixels(),
                            0, // no border
                            GL_RGBA,
                            GL_UNSIGNED_BYTE, // one byte per color component
                            triangle.Material->DiffuseProperties.Texture->GetRawData());
                    }
                    GLint is_textured_variable = glGetUniformLocation(ShaderProgram->Id, "is_textured");
                    glUniform1i(is_textured_variable, is_textured);

                    // ALLOCATE A VERTEX ARRAY/BUFFER.
                    const GLsizei ONE_VERTEX_ARRAY = 1;
                    GLuint vertex_array_id = 0;
                    glGenVertexArrays(ONE_VERTEX_ARRAY, &vertex_array_id);
                    glBindVertexArray(vertex_array_id);

                    const GLsizei ONE_VERTEX_BUFFER = 1;
                    GLuint vertex_buffer_id = 0;
                    glGenBuffers(ONE_VERTEX_BUFFER, &vertex_buffer_id);

                    // FILL THE BUFFER WITH THE VERTEX DATA.
                    constexpr std::size_t POSITION_COORDINATE_COUNT_PER_VERTEX = 4;
                    constexpr std::size_t VERTEX_POSITION_COORDINATE_TOTAL_COUNT = POSITION_COORDINATE_COUNT_PER_VERTEX * GEOMETRY::Triangle::VERTEX_COUNT;
                    constexpr std::size_t COLOR_COMPONENT_COUNT_PER_VERTEX = 4;
                    constexpr std::size_t VERTEX_COLOR_COMPONENT_TOTAL_COUNT = COLOR_COMPONENT_COUNT_PER_VERTEX * GEOMETRY::Triangle::VERTEX_COUNT;
                    constexpr std::size_t TEXTURE_COORDINATE_COMPONENT_COUNT_PER_VERTEX = 2;
                    constexpr std::size_t TEXTURE_COORDINATE_COMPONENT_TOTAL_COUNT = TEXTURE_COORDINATE_COMPONENT_COUNT_PER_VERTEX * GEOMETRY::Triangle::VERTEX_COUNT;
                    constexpr std::size_t NORMAL_COORDINATE_COUNT_PER_VERTEX = 4;
                    constexpr std::size_t NORMAL_COORDINATE_COMPONENT_TOTAL_COUNT = NORMAL_COORDINATE_COUNT_PER_VERTEX * GEOMETRY::Triangle::VERTEX_COUNT;
                    constexpr std::size_t VERTEX_ATTRIBUTE_TOTAL_VALUE_COUNT = (
                        VERTEX_POSITION_COORDINATE_TOTAL_COUNT +
                        VERTEX_COLOR_COMPONENT_TOTAL_COUNT +
                        TEXTURE_COORDINATE_COMPONENT_TOTAL_COUNT +
                        NORMAL_COORDINATE_COMPONENT_TOTAL_COUNT);

                    std::vector<float> vertex_attribute_values;
                    vertex_attribute_values.reserve(VERTEX_ATTRIBUTE_TOTAL_VALUE_COUNT);

                    MATH::Vector3f surface_normal = triangle.SurfaceNormal();
                    for (std::size_t vertex_index = 0; vertex_index < GEOMETRY::Triangle::VERTEX_COUNT; ++vertex_index)
                    {
                        const VertexWithAttributes& vertex = triangle.Vertices[vertex_index];
                        vertex_attribute_values.emplace_back(vertex.Position.X);
                        vertex_attribute_values.emplace_back(vertex.Position.Y);
                        vertex_attribute_values.emplace_back(vertex.Position.Z);
                        constexpr float HOMOGENEOUS_VERTEX_W = 1.0f;
                        vertex_attribute_values.emplace_back(HOMOGENEOUS_VERTEX_W);

                        vertex_attribute_values.emplace_back(vertex.Color.Red);
                        vertex_attribute_values.emplace_back(vertex.Color.Green);
                        vertex_attribute_values.emplace_back(vertex.Color.Blue);
                        vertex_attribute_values.emplace_back(vertex.Color.Alpha);

                        vertex_attribute_values.emplace_back(vertex.TextureCoordinates.X);
                        vertex_attribute_values.emplace_back(vertex.TextureCoordinates.Y);

                        vertex_attribute_values.emplace_back(surface_normal.X);
                        vertex_attribute_values.emplace_back(surface_normal.Y);
                        vertex_attribute_values.emplace_back(surface_normal.Z);
                        vertex_attribute_values.emplace_back(1.0f);
                    }

                    GLsizeiptr vertex_data_size_in_bytes = sizeof(float) * VERTEX_ATTRIBUTE_TOTAL_VALUE_COUNT;
                    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
                    glBufferData(GL_ARRAY_BUFFER, vertex_data_size_in_bytes, vertex_attribute_values.data(), GL_STATIC_DRAW);

                    // SPECIFY HOW VERTEX BUFFER DATA MAPS TO SHADER INPUTS.
                    constexpr GLboolean NO_NORMALIZATION = GL_FALSE;
                    constexpr GLsizei SINGLE_VERTEX_ATTRIBUTE_VALUE_COUNT = (
                        POSITION_COORDINATE_COUNT_PER_VERTEX +
                        COLOR_COMPONENT_COUNT_PER_VERTEX +
                        TEXTURE_COORDINATE_COMPONENT_COUNT_PER_VERTEX +
                        NORMAL_COORDINATE_COUNT_PER_VERTEX);
                    constexpr GLsizei SINGLE_VERTEX_ENTIRE_DATA_SIZE_IN_BYTES = sizeof(float) * SINGLE_VERTEX_ATTRIBUTE_VALUE_COUNT;
                    constexpr uint64_t VERTEX_POSITION_STARTING_OFFSET_IN_BYTES = 0;
                    GLint local_vertex_position_variable_id = glGetAttribLocation(ShaderProgram->Id, "local_vertex");
                    glVertexAttribPointer(
                        local_vertex_position_variable_id,
                        POSITION_COORDINATE_COUNT_PER_VERTEX,
                        GL_FLOAT,
                        NO_NORMALIZATION,
                        SINGLE_VERTEX_ENTIRE_DATA_SIZE_IN_BYTES,
                        (void*)VERTEX_POSITION_STARTING_OFFSET_IN_BYTES);
                    glEnableVertexAttribArray(local_vertex_position_variable_id);

                    const uint64_t VERTEX_COLOR_STARTING_OFFSET_IN_BYTES = sizeof(float) * POSITION_COORDINATE_COUNT_PER_VERTEX;
                    GLint vertex_color_variable_id = glGetAttribLocation(ShaderProgram->Id, "input_vertex_color");
                    glVertexAttribPointer(
                        vertex_color_variable_id,
                        COLOR_COMPONENT_COUNT_PER_VERTEX,
                        GL_FLOAT,
                        NO_NORMALIZATION,
                        SINGLE_VERTEX_ENTIRE_DATA_SIZE_IN_BYTES,
                        (void*)VERTEX_COLOR_STARTING_OFFSET_IN_BYTES);
                    glEnableVertexAttribArray(vertex_color_variable_id);

                    constexpr std::size_t VERTEX_COLOR_SIZE_IN_BYTES = sizeof(float) * COLOR_COMPONENT_COUNT_PER_VERTEX;
                    const uint64_t TEXTURE_COORDINATE_STARTING_OFFSET_IN_BYTES = VERTEX_COLOR_STARTING_OFFSET_IN_BYTES + VERTEX_COLOR_SIZE_IN_BYTES;
                    GLint texture_coordinates_variable_id = glGetAttribLocation(ShaderProgram->Id, "input_texture_coordinates");
                    glVertexAttribPointer(
                        texture_coordinates_variable_id,
                        TEXTURE_COORDINATE_COMPONENT_COUNT_PER_VERTEX,
                        GL_FLOAT,
                        NO_NORMALIZATION,
                        SINGLE_VERTEX_ENTIRE_DATA_SIZE_IN_BYTES,
                        (void*)TEXTURE_COORDINATE_STARTING_OFFSET_IN_BYTES);
                    glEnableVertexAttribArray(texture_coordinates_variable_id);

                    constexpr std::size_t TEXTURE_COORDINATE_SIZE_IN_BYTES = sizeof(float) * TEXTURE_COORDINATE_COMPONENT_COUNT_PER_VERTEX;
                    const uint64_t NORMAL_STARTING_OFFSET_IN_BYTES = TEXTURE_COORDINATE_STARTING_OFFSET_IN_BYTES + TEXTURE_COORDINATE_SIZE_IN_BYTES;
                    GLint normal_variable_id = glGetAttribLocation(ShaderProgram->Id, "vertex_normal");
                    glVertexAttribPointer(
                        normal_variable_id,
                        NORMAL_COORDINATE_COUNT_PER_VERTEX,
                        GL_FLOAT,
                        NO_NORMALIZATION,
                        SINGLE_VERTEX_ENTIRE_DATA_SIZE_IN_BYTES,
                        (void*)NORMAL_STARTING_OFFSET_IN_BYTES);
                    glEnableVertexAttribArray(normal_variable_id);

                    // DRAW THE TRIANGLE.
                    const unsigned int FIRST_VERTEX = 0;
                    GLsizei vertex_count = static_cast<GLsizei>(GEOMETRY::Triangle::VERTEX_COUNT);
                    glDrawArrays(GL_TRIANGLES, FIRST_VERTEX, vertex_count);

                    /// @todo   Encapsulate this vertex array stuff with object?
                    glDeleteBuffers(ONE_VERTEX_BUFFER, &vertex_buffer_id);
                    glDeleteVertexArrays(ONE_VERTEX_ARRAY, &vertex_array_id);

                    if (is_textured)
                    {
                        glDeleteTextures(1, &texture);
                    }
                }
            }
        }
    }

    /// Displays the rendered image from the graphics device.
    /// @param[in]  window - The window in which to display the image.
    void OpenGLGraphicsDevice::DisplayRenderedImage(WINDOWING::IWindow& window)
    {
        /// @todo
        window;

        glFlush();

        GLenum error = glGetError();
        if (error != GL_NO_ERROR)
        {
            std::string open_gl_error_message = "\nOpenGL error: " + std::to_string(error);
            OutputDebugString(open_gl_error_message.c_str());
            error = error;
        }

        SwapBuffers(WindowDeviceContext);
    }
}
