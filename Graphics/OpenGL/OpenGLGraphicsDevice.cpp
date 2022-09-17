#include <vector>
#include "Debugging/Timer.h"
#include "ErrorHandling/Asserts.h"
#include "Graphics/OpenGL/OpenGL.h"
#include "Graphics/OpenGL/OpenGLGraphicsDevice.h"
#include "Graphics/Shading/Lighting/Light.h"
#include "Graphics/Viewing/ViewingTransformations.h"
#include "Windowing/SdlWindow.h"
#include "Windowing/Win32Window.h"

namespace GRAPHICS::OPEN_GL
{
    /// Attempts to connect a graphics device to the specified window for rendering via the OpenGL.
    /// @param[in,out]  window - The window in which to do OpenGL rendering.  Non-const since non-const access is sometimes needed.
    /// return  The OpenGL graphics device, if successfully connected to the window; null if an error occurs.
    std::unique_ptr<OpenGLGraphicsDevice> OpenGLGraphicsDevice::ConnectTo(WINDOWING::IWindow& window)
    {
        // CHECK FOR A VALID TYPE OF WINDOW.
        WINDOWING::Win32Window* win32_window = dynamic_cast<WINDOWING::Win32Window*>(&window);
        WINDOWING::SdlWindow* sdl_window = dynamic_cast<WINDOWING::SdlWindow*>(&window);
        bool window_valid = (win32_window || sdl_window);
        if (!window_valid)
        {
            // INDICATE THAT NO OPEN GL GRAPHICS DEVICE COULD BE CONNECTED TO THE WINDOW.
            return nullptr;
        }

        // INITIALIZE OPEN BASED ON THE TYPE OF WINDOW.
        HDC window_device_context = nullptr;
        HGLRC win32_open_gl_render_context = nullptr;
        SDL_GLContext sdl_open_gl_context = nullptr;
        if (win32_window)
        {
            // ENSURE A VALID GRAPHICS DEVICE CONTEXT EXISTS ON THE WINDOW.
            window_device_context = GetDC(win32_window->WindowHandle);
            ASSERT_THEN_IF_NOT(window_device_context)
            {
                // INDICATE THAT NO OPEN GL GRAPHICS DEVICE COULD BE CONNECTED TO THE WINDOW.
                return nullptr;
            }

            // INITIALIZE OPEN GL.
            bool open_gl_initialized = GRAPHICS::OPEN_GL::Initialize(window_device_context);
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
            win32_open_gl_render_context = wglCreateContextAttribsARB(window_device_context, NO_CONTEXT_TO_SHARE_WITH, context_attribute_list);
            BOOL open_gl_context_made_current = wglMakeCurrent(window_device_context, win32_open_gl_render_context);
            if (!open_gl_context_made_current)
            {
                // INDICATE THAT NO OPEN GL GRAPHICS DEVICE COULD BE CONNECTED TO THE WINDOW.
                return nullptr;
            }
        }
        else if (sdl_window)
        {
            // INITIALIZE OPEN GL FOR SDL.
            sdl_open_gl_context = SDL_GL_CreateContext(sdl_window->UnderlyingWindow);
            SDL_GL_MakeCurrent(sdl_window->UnderlyingWindow, sdl_open_gl_context);

            constexpr int VERTICAL_SYNC = 1;
            SDL_GL_SetSwapInterval(VERTICAL_SYNC);

            // LOAD OPEN GL FUNCTIONS.
            gl3wInit();
        }
        else
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
        graphics_device->Win32OpenGLRenderContext = win32_open_gl_render_context;
        graphics_device->SdlOpenGLContext = sdl_open_gl_context;

        // CONFIGURE THE SHADER PROGRAM TO USE.
        graphics_device->ShaderProgram = GRAPHICS::OPEN_GL::ShaderProgram::Build(
            GRAPHICS::OPEN_GL::ShaderProgram::DEFAULT_VERTEX_SHADER_CODE,
            GRAPHICS::OPEN_GL::ShaderProgram::DEFAULT_FRAGMENT_SHADER_CODE);
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
        // DELETE DEVICE RESOURCES.
        glDeleteTextures(static_cast<GLsizei>(TextureIds.size()), TextureIds.data());
        VertexBuffers.clear();
        ShaderProgram.reset();
        if (Win32OpenGLRenderContext)
        {
            wglDeleteContext(Win32OpenGLRenderContext);
            Win32OpenGLRenderContext = nullptr;
        }
        if (SdlOpenGLContext)
        {
            SDL_GL_DeleteContext(SdlOpenGLContext);
            SdlOpenGLContext = nullptr;
        }
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

    /// Loads the 3D object into the graphics device.
    /// @param[in,out]  object_3D - The object to load.
    void OpenGLGraphicsDevice::Load(GRAPHICS::Object3D& object_3D)
    {
        // SKIP OVER ANY OBJECTS THAT DO NOT HAVE MESHES.
        // Only meshes can fill vertex buffers.
        bool object_has_meshes = !object_3D.Model.MeshesByName.empty();
        if (!object_has_meshes)
        {
            return;
        }

        // FILL A VERTEX BUFFER FOR THE OBJECT.
        auto vertex_buffer = std::make_shared<VertexBuffer>();
        vertex_buffer->Fill(object_3D.Model);

        // STORE THE VERTEX BUFFER IN APPROPRIATE PLACES.
        // Since the object itself might move around in memory, the vertex buffer must be associated with it.
        object_3D.Model.OpenGLVertexBuffer = vertex_buffer;
        // The vertex buffer should be stored in this graphics device for proper memory management.
        VertexBuffers.emplace_back(vertex_buffer);

        // LOAD ANY TEXTURES.
        for (const auto& [mesh_name, mesh] : object_3D.Model.MeshesByName)
        {
            // LOAD TEXTURES FOR ALL TRIANGLES.
            for (const GEOMETRY::Triangle& triangle : mesh.Triangles)
            {
                // SKIP OVER ANY TRIANGLES WITHOUT MATERIALS.
                if (!triangle.Material)
                {
                    continue;
                }

                /// @todo   LOAD ANY AMBIENT TEXTURES.

                // LOAD ANY DIFFUSE TEXTURES.
                if (triangle.Material->DiffuseProperties.Texture)
                {
                    // ALLOCATE AND BIND THE FIRST TEXTURE.
                    // Must be done outside of glBegin()/glEnd() (http://docs.gl/gl2/glGenTextures).
                    glGenTextures(1, &triangle.Material->DiffuseProperties.OpenGLTextureId);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, triangle.Material->DiffuseProperties.OpenGLTextureId);

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

                /// @todo   LOAD ANY SPECULAR TEXTURES.
            }
        }
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
        if (rendering_settings.DepthBuffering)
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }

        // CLEAR THE BACKGROUND TO THE APPROPRIATE COLOR.
        ClearBackground(scene.BackgroundColor);

        // SET THE CAMERA TRANSFORMS IN THE SHADER.
        // Viewing transformations are needed for all objects, so they are computed/set
        // once before the loop to avoid expensive operations for each object.
        VIEWING::ViewingTransformations viewing_transformations(camera);

        constexpr GLsizei ONE_MATRIX = 1;
        constexpr GLboolean ROW_MAJOR_ORDER = GL_TRUE;
        GLint view_matrix_variable = glGetUniformLocation(ShaderProgram->Id, "view_transform");
        const float* view_matrix_elements_in_row_major_order = viewing_transformations.CameraViewTransform.ElementsInRowMajorOrder();
        glUniformMatrix4fv(view_matrix_variable, ONE_MATRIX, ROW_MAJOR_ORDER, view_matrix_elements_in_row_major_order);

        GLint projection_matrix_variable = glGetUniformLocation(ShaderProgram->Id, "projection_transform");
        const float* projection_matrix_elements_in_row_major_order = viewing_transformations.CameraProjectionTransform.ElementsInRowMajorOrder();
        glUniformMatrix4fv(projection_matrix_variable, ONE_MATRIX, ROW_MAJOR_ORDER, projection_matrix_elements_in_row_major_order);

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

        // SPECIFY HOW VERTEX BUFFER DATA MAPS TO SHADER INPUTS.
        constexpr GLboolean NO_NORMALIZATION = GL_FALSE;
        constexpr std::size_t POSITION_COORDINATE_COUNT_PER_VERTEX = 4;
        constexpr std::size_t COLOR_COMPONENT_COUNT_PER_VERTEX = 4;
        constexpr std::size_t TEXTURE_COORDINATE_COMPONENT_COUNT_PER_VERTEX = 2;
        constexpr std::size_t NORMAL_COORDINATE_COUNT_PER_VERTEX = 3;
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
        
        // RENDER EACH OBJECT.
        for (const GRAPHICS::Object3D& object_3D : scene.Objects)
        {
            // SET THE OBJECT'S WORLD TRANSFORM IN THE SHADER.
            GLint world_matrix_variable = glGetUniformLocation(ShaderProgram->Id, "world_transform");
            MATH::Matrix4x4f world_transform = object_3D.WorldTransform();
            const float* world_matrix_elements_in_row_major_order = world_transform.ElementsInRowMajorOrder();
            glUniformMatrix4fv(world_matrix_variable, ONE_MATRIX, ROW_MAJOR_ORDER, world_matrix_elements_in_row_major_order);

            // LOAD THE OBJECT'S VERTEX BUFFER.
            std::shared_ptr<VertexBuffer> object_vertex_buffer = object_3D.Model.OpenGLVertexBuffer.lock();
            if (!object_vertex_buffer)
            {
                // The vertex buffer is required for further rendering.
                continue;
            }
            glBindVertexArray(object_vertex_buffer->ArrayId);

            // RENDER EACH MESH IN THE OBJECT.
            // Since all model vertices share the same vertex buffer, the current triangle
            // out of the entire model must be tracked.
            GLint current_triangle_index = 0;
            for (const auto& [mesh_name, mesh] : object_3D.Model.MeshesByName)
            {
                // RENDER THE CURRENT TRIANGLE.
                for (const GEOMETRY::Triangle& triangle : mesh.Triangles)
                {
                    // ALLOCATE A TEXTURE IF APPLICABLE.
                    /// @todo   Multiple textures?  Constant for 0.
                    bool is_textured = (0 != triangle.Material->DiffuseProperties.OpenGLTextureId);
                    if (is_textured)
                    {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, triangle.Material->DiffuseProperties.OpenGLTextureId);
                    }
                    GLint is_textured_variable = glGetUniformLocation(ShaderProgram->Id, "is_textured");
                    glUniform1i(is_textured_variable, is_textured);

                    // DRAW THE TRIANGLE.
                    // Since texture information is currently on a per-triangle basis, only a single triangle can be drawn at once.
                    GLint starting_vertex_index = static_cast<GLint>(current_triangle_index * GEOMETRY::Triangle::VERTEX_COUNT);
                    GLsizei vertex_count = static_cast<GLsizei>(GEOMETRY::Triangle::VERTEX_COUNT);
                    glDrawArrays(GL_TRIANGLES, starting_vertex_index, vertex_count);

                    // MOVE TO THE NEXT TRIANGLE.
                    ++current_triangle_index;
                }
            }
        }
    }

    /// Displays the rendered image from the graphics device.
    /// @param[in]  window - The window in which to display the image.
    void OpenGLGraphicsDevice::DisplayRenderedImage(WINDOWING::IWindow& window)
    {
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
        if (SdlOpenGLContext)
        {
            WINDOWING::SdlWindow* sdl_window = dynamic_cast<WINDOWING::SdlWindow*>(&window);
            SDL_GL_SwapWindow(sdl_window->UnderlyingWindow);
        }
        else
        {
            SwapBuffers(WindowDeviceContext);
        }
    }
}
