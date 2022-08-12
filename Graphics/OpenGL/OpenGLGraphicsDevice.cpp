#include <vector>
#include "Debugging/Timer.h"
#include "ErrorHandling/Asserts.h"
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
        // ENSURE A COMPATIBLE WINDOW IS GIVEN.
        // So far, only Win32 windows are supported.
        WINDOWING::Win32Window* win32_window = dynamic_cast<WINDOWING::Win32Window*>(&window);
        ASSERT_THEN_IF_NOT(win32_window)
        {
            // INDICATE THAT NO OPEN GL GRAPHICS DEVICE COULD BE CONNECTED TO THE WINDOW.
            return nullptr;
        }

        // ENSURE A VALID GRAPHICS DEVICE CONTEXT EXISTS ON THE WINDOW.
        HDC window_device_context = GetDC(win32_window->WindowHandle);
        ASSERT_THEN_IF_NOT(window_device_context)
        {
            // INDICATE THAT NO OPEN GL GRAPHICS DEVICE COULD BE CONNECTED TO THE WINDOW.
            return nullptr;
        }

        // INITIALIZE OPEN GL.
        bool open_gl_initialized = InitializeOpenGL(window_device_context);
        ASSERT_THEN_IF_NOT(open_gl_initialized)
        {
            // INDICATE THAT NO OPEN GL GRAPHICS DEVICE COULD BE CONNECTED TO THE WINDOW.
            return nullptr;
        }
        
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
            // INDICATE THAT NO OPEN GL GRAPHICS DEVICE COULD BE CONNECTED TO THE WINDOW.
            return nullptr;
        }

        // INITIALIZE THE VIEWPORT TO UTILIZE THE ENTIRE WINDOW.
        constexpr GLint VIEWPORT_LOWER_LEFT_X = 0;
        constexpr GLint VIEWPORT_LOWER_LEFT_Y = 0;
        unsigned int width_in_pixels = window.GetWidthInPixels();
        unsigned int height_in_pixels = window.GetHeightInPixels();
        glViewport(VIEWPORT_LOWER_LEFT_X, VIEWPORT_LOWER_LEFT_Y, width_in_pixels, height_in_pixels);

        // INITIALIZE THE GRAPHICS DEVICE.
        auto graphics_device = std::make_unique<OpenGLGraphicsDevice>();

        graphics_device->OpenGLVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        graphics_device->Window = &window;
        graphics_device->WindowDeviceContext = window_device_context;
        graphics_device->OpenGLRenderContext = open_gl_render_context;

        // CONFIGURE THE SHADER PROGRAM TO USE.
        graphics_device->ShaderProgram = GRAPHICS::OPEN_GL::ShaderProgram::Build(
            DEFAULT_VERTEX_SHADER,
            DEFAULT_FRAGMENT_SHADER);
        glUseProgram(graphics_device->ShaderProgram->Id);

        // A single texture sampler is currently all that is used for all rendering.
        GLint texture_sampler_variable = glGetUniformLocation(graphics_device->ShaderProgram->Id, "texture_sampler");
        constexpr GLint FIRST_TEXTURE_UNIT = 0;
        glUniform1i(texture_sampler_variable, FIRST_TEXTURE_UNIT);

        return graphics_device;
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

    /// @todo   Loads the 3D object into the graphics device.
    /// @param[in,out]  object_3D - The object to load.
    void OpenGLGraphicsDevice::Load(GRAPHICS::Object3D& object_3D)
    {
        // Reference the parameter to avoid compiler warnings.
        object_3D;
    }

    /// Clears the background screen of the graphics device in preparation for new rendering.
    /// @param[in]  color - The background color to clear to.
    void OpenGLGraphicsDevice::ClearBackground(const GRAPHICS::Color& color)
    {
        // CLEAR THE COLOR BUFFER.
        GLfloat background_color[] = { color.Red, color.Green, color.Blue, color.Alpha };
        constexpr GLint NO_SPECIFIC_DRAW_BUFFER = 0;
        glClearBufferfv(GL_COLOR, NO_SPECIFIC_DRAW_BUFFER, background_color);

        // CLEAR THE DEPTH BUFFER.
        GLfloat MAX_DEPTH[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        constexpr GLint NO_SPECIFIC_DEPTH_BUFFER = 0;
        glClearBufferfv(GL_DEPTH, NO_SPECIFIC_DEPTH_BUFFER, MAX_DEPTH);
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
        // CONFIGURE DEPTH BUFFERING AS APPROPRIATE.
        {
            DEBUGGING::Timer timer("Depth buffering timer");

            if (rendering_settings.DepthBuffering)
            {
                glEnable(GL_DEPTH_TEST);
            }
            else
            {
                glDisable(GL_DEPTH_TEST);
            }
        }

        // CLEAR THE BACKGROUND TO THE APPROPRIATE COLOR.
        {
            DEBUGGING::Timer timer("Clear background timer");
            ClearBackground(scene.BackgroundColor);
        }

        const GLsizei ONE_MATRIX = 1;
        const GLboolean ROW_MAJOR_ORDER = GL_TRUE;
        {
            DEBUGGING::Timer timer("Camera timer");

            // SET THE CAMERA TRANSFORMS IN THE SHADER.
            // Viewing transformations are needed for all objects, so they are computed/set
            // once before the loop to avoid expensive operations for each object.
            VIEWING::ViewingTransformations viewing_transformations(camera);

            /// @todo const GLsizei ONE_MATRIX = 1;
            /// @todo const GLboolean ROW_MAJOR_ORDER = GL_TRUE;
            GLint view_matrix_variable = glGetUniformLocation(ShaderProgram->Id, "view_transform");
            const float* view_matrix_elements_in_row_major_order = viewing_transformations.CameraViewTransform.ElementsInRowMajorOrder();
            glUniformMatrix4fv(view_matrix_variable, ONE_MATRIX, ROW_MAJOR_ORDER, view_matrix_elements_in_row_major_order);

            GLint projection_matrix_variable = glGetUniformLocation(ShaderProgram->Id, "projection_transform");
            const float* projection_matrix_elements_in_row_major_order = viewing_transformations.CameraProjectionTransform.ElementsInRowMajorOrder();
            glUniformMatrix4fv(projection_matrix_variable, ONE_MATRIX, ROW_MAJOR_ORDER, projection_matrix_elements_in_row_major_order);
        }

        {
            DEBUGGING::Timer timer("Light timer");

            // CONFIGURE LIGHTING IN THE SHADER.
            bool is_lit = !scene.Lights.empty();
            GLint is_lit_variable = glGetUniformLocation(ShaderProgram->Id, "is_lit");
            glUniform1i(is_lit_variable, is_lit);
            if (is_lit)
            {
                /// @todo   Support all lights?
                // Single arbitrary light for now.
                const SHADING::LIGHTING::Light& first_light = scene.Lights.at(0);

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
        }

        {
            DEBUGGING::Timer timer("All objects rendering timer");
            // RENDER EACH OBJECT.
            for (const GRAPHICS::Object3D& object_3D : scene.Objects)
            {
                DEBUGGING::Timer timer("Single object rendering timer");

                // SET THE OBJECT'S WORLD TRANSFORM IN THE SHADER.
                GLint world_matrix_variable = glGetUniformLocation(ShaderProgram->Id, "world_transform");
                MATH::Matrix4x4f world_transform = object_3D.WorldTransform();
                const float* world_matrix_elements_in_row_major_order = world_transform.ElementsInRowMajorOrder();
                glUniformMatrix4fv(world_matrix_variable, ONE_MATRIX, ROW_MAJOR_ORDER, world_matrix_elements_in_row_major_order);

                // RENDER EACH MESH IN THE OBJECT.
                /// @todo   Pass vertices for entire object at once!
                /// @todo   Look at https://github.com/jpike/OpenGLEngine/ for possible better handling of some stuff?
                ///     Probably needed for performance (no glGen calls! - https://www.khronos.org/opengl/wiki/Common_Mistakes#glGenTextures_in_render_function).
                for (const auto& [mesh_name, mesh] : object_3D.Model.MeshesByName)
                {
                    DEBUGGING::Timer timer("Single mesh rendering timer");

                    // RENDER THE CURRENT TRIANGLE.
                    for (const auto& triangle : mesh.Triangles)
                    {
                        DEBUGGING::Timer timer("Single triangle rendering timer");

                        // ALLOCATE A TEXTURE IF APPLICABLE.
                        // Must be done outside of glBegin()/glEnd() (http://docs.gl/gl2/glGenTextures).
                        GLuint texture = 0;
                        /// @todo   Multiple textures?
                        bool is_textured = (nullptr != triangle.Material->DiffuseProperties.Texture);
                        if (is_textured)
                        {
                            DEBUGGING::Timer timer("Single triangle texture rendering timer");

                            // ALLOCATE AND BIND THE FIRST TEXTURE.
                            glGenTextures(1, &texture);
                            glActiveTexture(GL_TEXTURE0);
                            glBindTexture(GL_TEXTURE_2D, texture);

                            // CONFIGURE TEXTURE MAPPING PARAMETERS.
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

                            // SET THE TEXTURE DATA.
                            constexpr GLint BASE_LEVEL_OF_DETAIL = 0;
                            constexpr GLint BORDER_REQUIRED_AS_0 = 0;
                            constexpr GLenum ONE_BYTE_PER_COLOR_COMPONENT = GL_UNSIGNED_BYTE;
                            glTexImage2D(
                                GL_TEXTURE_2D,
                                BASE_LEVEL_OF_DETAIL,
                                GL_RGBA, // Only RGBA is currently supported.
                                triangle.Material->DiffuseProperties.Texture->GetWidthInPixels(),
                                triangle.Material->DiffuseProperties.Texture->GetHeightInPixels(),
                                BORDER_REQUIRED_AS_0,
                                GL_RGBA, // Only RGBA is currently supported.
                                ONE_BYTE_PER_COLOR_COMPONENT,
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
                        constexpr std::size_t NORMAL_COORDINATE_COUNT_PER_VERTEX = 3;
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

                            // SET THE VERTEX POSITION.
                            vertex_attribute_values.emplace_back(vertex.Position.X);
                            vertex_attribute_values.emplace_back(vertex.Position.Y);
                            vertex_attribute_values.emplace_back(vertex.Position.Z);
                            constexpr float HOMOGENEOUS_VERTEX_W = 1.0f;
                            vertex_attribute_values.emplace_back(HOMOGENEOUS_VERTEX_W);

                            // SET THE VERTEX COLOR.
                            vertex_attribute_values.emplace_back(vertex.Color.Red);
                            vertex_attribute_values.emplace_back(vertex.Color.Green);
                            vertex_attribute_values.emplace_back(vertex.Color.Blue);
                            vertex_attribute_values.emplace_back(vertex.Color.Alpha);

                            // SET VERTEX TEXTURE COORDINATES.
                            vertex_attribute_values.emplace_back(vertex.TextureCoordinates.X);
                            vertex_attribute_values.emplace_back(vertex.TextureCoordinates.Y);

                            // SET THE VERTEX NORMAL.
                            vertex_attribute_values.emplace_back(surface_normal.X);
                            vertex_attribute_values.emplace_back(surface_normal.Y);
                            vertex_attribute_values.emplace_back(surface_normal.Z);
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

                        // The local vertex position comes first.
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

                        // The vertex color comes next.
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

                        // The vertex texture coordinates come next.
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

                        // The vertex normal comes last.
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

                        // DE-ALLOCATE RESOURCES FOR THE CURRENT TRIANGLE.
                        /// @todo   Encapsulate this vertex array stuff with object?
                        glDeleteBuffers(ONE_VERTEX_BUFFER, &vertex_buffer_id);
                        glDeleteVertexArrays(ONE_VERTEX_ARRAY, &vertex_array_id);

                        if (is_textured)
                        {
                            // Only a single texture exists.
                            glDeleteTextures(1, &texture);
                        }
                    }
                }
            }
        }
    }

    /// Displays the rendered image from the graphics device.
    /// @param[in]  window - The window in which to display the image.
    ///     Currently unused since the device is connected to the window at creation.
    void OpenGLGraphicsDevice::DisplayRenderedImage(WINDOWING::IWindow& window)
    {
        // The window parameter is referenced to avoid compiler warnings.
        window;

        // TRY TO ENSURE RENDERING COMMANDS HAVE COMPLETED.
        // This does not guarantee the commands will finish, but in practice it has
        // been found to be helpful/necessary.
        glFlush();

        // PROVIDE VISIBILITY INTO ANY ERRORS.
        GLenum error = glGetError();
        bool error_occurred = (GL_NO_ERROR != error);
        if (error_occurred)
        {
            std::string open_gl_error_message = "\nOpenGL error: " + std::to_string(error);
            OutputDebugString(open_gl_error_message.c_str());
        }

        // DISPLAY THE RENDERED IMAGE IN THE WINDOW.
        SwapBuffers(WindowDeviceContext);
    }

    /// The callback function for OpenGL to call for debug messages.
    /// Prints the debug message for visibility.
    /// @param[in]  source - The source of the message.
    /// @param[in]  type - The type of the message.
    /// @param[in]  id - The ID of the message.
    /// @param[in]  severity - The severity of the message.
    /// @param[in]  length_in_characters - The length of the message in characters.
    /// @param[in]  message - The actual message.
    /// @param[in]  user_parameter - Any user parameter.
    void OpenGLGraphicsDevice::OpenGLDebugMessageCallback(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length_in_characters,
        const GLchar* message,
        void* user_parameter)
    {
        // PROVIDE MAXIMUM DEBUG VISIBILITY.
        std::stringstream debug_message;
        debug_message
            << "\nOpenGL debug:"
            << "\tsource = " << source
            << "\ttype = " << type
            << "\tid = " << id
            << "\tseverity = " << severity
            << "\tlength = " << length_in_characters
            << "\tmessage = " << message
            << "\tuser_parameter = " << user_parameter
            << "\n";
        OutputDebugString(debug_message.str().c_str());
    }

    /// Initializes OpenGL by loading the necessary functions.
    /// @param[in] device_context - The device context for which OpenGL should be initialized.
    /// @return True if initialization succeeds; false otherwise.
    bool OpenGLGraphicsDevice::InitializeOpenGL(const HDC device_context)
    {
        // SET THE PIXEL FORMAT.
        PIXELFORMATDESCRIPTOR pixel_format = {};
        pixel_format.nSize = sizeof(pixel_format);
        const WORD REQUIRED_VERSION = 1;
        pixel_format.nVersion = REQUIRED_VERSION;
        pixel_format.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pixel_format.iPixelType = PFD_TYPE_RGBA;
        pixel_format.cColorBits = 32;
        pixel_format.cDepthBits = 24;
        pixel_format.cStencilBits = 8;

        const int FAILED_TO_FIND_PIXEL_FORMAT = 0;
        int temp_pixel_format_index = ChoosePixelFormat(device_context, &pixel_format);
        bool pixel_format_chosen = (FAILED_TO_FIND_PIXEL_FORMAT != temp_pixel_format_index);
        if (!pixel_format_chosen)
        {
            return false;
        }

        BOOL pixel_format_set = SetPixelFormat(device_context, temp_pixel_format_index, &pixel_format);
        if (!pixel_format_set)
        {
            return false;
        }

        // CREATE A TEMPORARY OPEN GL RENDERING CONTEXT.
        // This is necessary in order to load the OpenGL functions.
        HGLRC temp_open_gl_render_context = wglCreateContext(device_context);
        bool temp_open_gl_render_context_created = (NULL != temp_open_gl_render_context);
        if (!temp_open_gl_render_context_created)
        {
            return false;
        }

        BOOL open_gl_context_made_current = wglMakeCurrent(device_context, temp_open_gl_render_context);
        if (!open_gl_context_made_current)
        {
            return false;
        }

        // LOAD THE OPEN GL FUNCTIONS.
        wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
        wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
        int regular_open_gl_loading_return_code = gl3wInit();

        // CHECK IF LOADING SUCCEEDED.
        constexpr int GL3W_SUCCESS_RETURN_CODE = 0;
        bool regular_open_gl_functions_loaded = (GL3W_SUCCESS_RETURN_CODE == regular_open_gl_loading_return_code);
        bool open_gl_functions_loaded = (wglChoosePixelFormatARB && wglCreateContextAttribsARB && regular_open_gl_functions_loaded);
        if (!open_gl_functions_loaded)
        {
            return false;
        }

        // CONFIGURE DEBUG LOGGING.
        // Only done in debug builds to avoid potential performance penalties during release builds.
#if _DEBUG
        constexpr void* NO_DEBUG_USER_DATA = nullptr;
        glDebugMessageCallback(OpenGLDebugMessageCallback, NO_DEBUG_USER_DATA);
        constexpr GLenum NO_SOURCE_DEBUG_MESSAGE_FILTERIING = GL_DONT_CARE;
        constexpr GLenum NO_TYPE_DEBUG_MESSAGE_FILTERIING = GL_DONT_CARE;
        constexpr GLenum NO_SEVERITY_DEBUG_MESSAGE_FILTERIING = GL_DONT_CARE;
        constexpr GLsizei NO_ID_DEBUG_MESSAGES_TO_FILTER_COUNT = 0;
        constexpr GLuint* NO_ID_DEBUG_MESSAGE_FILTERING = nullptr;
        constexpr GLboolean ENABLE_DEBUG_MESSAGE = GL_TRUE;
        glDebugMessageControl(
            NO_SOURCE_DEBUG_MESSAGE_FILTERIING,
            NO_TYPE_DEBUG_MESSAGE_FILTERIING,
            NO_SEVERITY_DEBUG_MESSAGE_FILTERIING,
            NO_ID_DEBUG_MESSAGES_TO_FILTER_COUNT,
            NO_ID_DEBUG_MESSAGE_FILTERING,
            ENABLE_DEBUG_MESSAGE);
#endif

        // SET THE PIXEL FORMAT.
        const int pixel_format_attribute_list[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB, pixel_format.cColorBits,
            WGL_DEPTH_BITS_ARB, pixel_format.cDepthBits,
            WGL_STENCIL_BITS_ARB, pixel_format.cStencilBits,
            ATTRIBUTE_LIST_TERMINATOR
        };
        const float* const NO_FLOATING_POINT_PIXEL_FORMAT_ATTRIBUTES = nullptr;
        const unsigned int MAX_PIXEL_FORMATS_TO_CHOOSE = 1;
        int pixel_format_index = 0;
        UINT pixel_format_count = 0;

        BOOL gl_pixel_format_chosen = wglChoosePixelFormatARB(
            device_context,
            pixel_format_attribute_list,
            NO_FLOATING_POINT_PIXEL_FORMAT_ATTRIBUTES,
            MAX_PIXEL_FORMATS_TO_CHOOSE,
            &pixel_format_index,
            &pixel_format_count);
        if (!gl_pixel_format_chosen)
        {
            return false;
        }
        SetPixelFormat(device_context, pixel_format_index, &pixel_format);

        // DELETE THE TEMPORARY OPEN GL RENDERING CONTEXT.
        wglDeleteContext(temp_open_gl_render_context);

        // INDICATE OPEN GL INITIALIZATION SUCCEEDED.
        return true;
    }
}
