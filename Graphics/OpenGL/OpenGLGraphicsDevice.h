#pragma once

#include <memory>
#include <string>
#include <vector>
#include <gl/GL.h>
#include <GL/wglext.h>
// Older OpenGL functions must be undefined to avoid missing symbols.
#undef GL_VERSION_1_1
#include <gl3w/GL/gl3w.h>
#include <Windows.h>
#include "Graphics/Hardware/IGraphicsDevice.h"
#include "Graphics/OpenGL/ShaderProgram.h"
#include "Graphics/OpenGL/VertexBuffer.h"
#include "Windowing/IWindow.h"

/// Holds code related to the OpenGL graphics library.
namespace GRAPHICS::OPEN_GL
{
    /// A graphics device that can be interacted with using the OpenGL library.
    class OpenGLGraphicsDevice : public GRAPHICS::HARDWARE::IGraphicsDevice
    {
    public:
        // CONSTANTS.
        /// The value that marks the end of an OpenGL attribute list.
        static constexpr int ATTRIBUTE_LIST_TERMINATOR = 0;

        // CREATION/SHUTDOWN.
        static std::unique_ptr<OpenGLGraphicsDevice> ConnectTo(WINDOWING::IWindow& window);
        void Shutdown() override;
        virtual ~OpenGLGraphicsDevice();

        // INFORMATION RETRIEVAL.
        GRAPHICS::HARDWARE::GraphicsDeviceType Type() const override;

        // RESOURCE ALLOCATION.
        void Load(GRAPHICS::Object3D& object_3D) override;

        // RENDERING.
        void ClearBackground(const GRAPHICS::Color& color) override;
        void Render(
            const GRAPHICS::Scene& scene,
            const VIEWING::Camera& camera,
            const GRAPHICS::RenderingSettings& rendering_settings) override;
        void DisplayRenderedImage(WINDOWING::IWindow& window) override;

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// The version of OpenGL being used.
        std::string OpenGLVersion = "";
        /// The window the graphics device is connected to.
        WINDOWING::IWindow* Window = nullptr;
        /// The regular Windows device context.
        HDC WindowDeviceContext = nullptr;
        /// The OpenGL rendering context.
        HGLRC OpenGLRenderContext = nullptr;
        /// The default shader program.
        std::shared_ptr<ShaderProgram> ShaderProgram = nullptr;

    private:
        // PRIVATE HELPER METHODS.
        static void OpenGLDebugMessageCallback(
            GLenum source,
            GLenum type,
            GLuint id,
            GLenum severity,
            GLsizei length_in_characters,
            const GLchar* message,
            void* user_parameter);
        static bool InitializeOpenGL(const HDC device_context);

        // LOADED OPEN GL FUNCTIONS.
        /// The function for choosing pixel formats for OpenGL.
        inline static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
        /// The function for creating an OpenGL context with attributes.
        inline static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

        // INTERNALLY MANAGED RESOURCES.
        /// Vertex buffers allocated on the device.
        std::vector<std::shared_ptr<VertexBuffer>> VertexBuffers = {};
        /// IDs of textures allocated on the device.
        std::vector<GLuint> TextureIds = {};
    };
}
