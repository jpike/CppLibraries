#include <gl3w/GL/gl3w.h>
#include "Graphics/OpenGL/OpenGLGraphicsDevice.h"

namespace GRAPHICS::OPEN_GL
{
    /// Initialize a graphics device that uses OpenGL to render to the specified window.
    /// @param[in]  window - The window to render to.
    /// @return The graphics device, if successfully initialized; null if not.
    std::optional<OpenGLGraphicsDevice> OpenGLGraphicsDevice::Initialize(const WINDOWING::SdlWindow& window)
    {
        // INITIALIZE OPEN GL FOR SDL.
        /// @todo   Error handling!
        SDL_GLContext gl_context = SDL_GL_CreateContext(window.UnderlyingWindow);
        SDL_GL_MakeCurrent(window.UnderlyingWindow, gl_context);
        constexpr int VERTICAL_SYNC = 1;
        SDL_GL_SetSwapInterval(VERTICAL_SYNC);

        // LOAD OPEN GL.
        /// @todo gl3wInit();

        // RETURN THE INITIALIZED GRAPHICS DEVICE.
        OpenGLGraphicsDevice graphics_device =
        {
            .Context = gl_context
        };
        return graphics_device;
    }

    /// Clears the background screen of the graphics device in preparation for new rendering.
    void OpenGLGraphicsDevice::Clear()
    {
        /// @todo   More self-documenting clear color.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    /// Displays the rendered frame on screen.
    /// @param[in]  window - The window in which to display the rendered frame on screen.
    void OpenGLGraphicsDevice::Display(const WINDOWING::SdlWindow& window)
    {
        SDL_GL_SwapWindow(window.UnderlyingWindow);
    }

    /// Shutdowns the graphics device, freeing up allocated resources.
    void OpenGLGraphicsDevice::Shutdown()
    {
        SDL_GL_DeleteContext(Context);
        Context = nullptr;
    }
}
