// This code is only compiled in if the SDL library is available to allow
// the larger windowing library to be used without SDL.
#if __has_include(<SDL/SDL.h>)

#include "ErrorHandling/Asserts.h"
#include "Windowing/SdlWindow.h"

namespace WINDOWING
{
    /// Attempts to created a window.
    /// @param[in]  title - The title to display in the window's title bar.
    /// @param[in]  width_in_pixels - The width (in pixels) of the client rendering area of the window.
    /// @param[in]  height_in_pixels - The height (in pixels) of the client rendering area of the window.
    /// @return The window, if successfully created; null if not.
    std::optional<SdlWindow> SdlWindow::Create(
        const char* title,
        const unsigned int width_in_pixels,
        const unsigned int height_in_pixels)
    {
        // TRY TO CREATE THE WINDOW.
        /// @todo   Allow switching between OpenGL, Direct3D, etc.
        constexpr SDL_WindowFlags WINDOW_SETTINGS = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        SDL_Window* sdl_window = SDL_CreateWindow(
            title,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width_in_pixels,
            height_in_pixels,
            WINDOW_SETTINGS);
        ASSERT_THEN_IF_NOT(sdl_window)
        {
            // INDICATE THAT THE WINDOW COULD NOT BE CREATED.
            return std::nullopt;
        }

        // GET ADDITIONAL INFORMATION ABOUT THE WINDOW.
        SDL_SysWMinfo window_info;
        SDL_VERSION(&window_info.version);
        bool window_info_obtained = SDL_GetWindowWMInfo(sdl_window, &window_info);
        ASSERT_THEN_IF_NOT(window_info_obtained)
        {
            // INDICATE THAT THE WINDOW COULD NOT BE CREATED.
            return std::nullopt;
        }

        // RETURN THE CREATED WINDOW.
        SdlWindow window =
        {
            .UnderlyingWindow = sdl_window,
            .AdditionalInfo = window_info,
            .WidthInPixels = width_in_pixels,
            .HeightInPixels = height_in_pixels,
            .IsOpen = true
        };
        return window;
    }

    /// Closes the window.
    void SdlWindow::Close()
    {
        SDL_DestroyWindow(UnderlyingWindow);
    }
}

#endif
