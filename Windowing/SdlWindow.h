#pragma once

// This code is only compiled in if the SDL library is available to allow
// the larger windowing library to be used without SDL.
#if __has_include(<SDL/SDL.h>)

#include <optional>
#include <SDL/SDL_syswm.h>
#include <SDL/SDL_video.h>

namespace WINDOWING
{
    /// A window using the SDL library.
    class SdlWindow
    {
    public:
        // METHODS.
        static std::optional<SdlWindow> Create(
            const char* const title,
            const unsigned int width_in_pixels,
            const unsigned int height_in_pixels);
        void Close();

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// The underlying SDL window.
        SDL_Window* UnderlyingWindow = nullptr;
        /// Additional system-dependent information about the window from the SDL library.
        SDL_SysWMinfo AdditionalInfo = {};
        /// The width (in pixels) of the client rendering area of the window.
        unsigned int WidthInPixels = 0;
        /// The height (in pixels) of the client rendering area of the window.
        unsigned int HeightInPixels = 0;
        /// True if the window is open; false if not.
        bool IsOpen = false;
    };
}

#endif
