#pragma once

// This code is only compiled in if the SDL library is available to allow
// the larger windowing library to be used without SDL.
#if __has_include(<SDL/SDL.h>)

#include <memory>
#include <SDL/SDL_syswm.h>
#include <SDL/SDL_video.h>
#include "Graphics/Hardware/GraphicsDeviceType.h"
#include "Graphics/Images/Bitmap.h"
#include "Windowing/IWindow.h"

namespace WINDOWING
{
    /// A window using the SDL library.
    class SdlWindow : public IWindow
    {
    public:
        // CREATION/DESTRUCTION.
        static std::unique_ptr<SdlWindow> Create(
            const char* const title,
            const unsigned int width_in_pixels,
            const unsigned int height_in_pixels,
            const GRAPHICS::HARDWARE::GraphicsDeviceType graphics_device_type);
        virtual ~SdlWindow();
        void Close();

        // DIMENSIONS.
        unsigned int GetWidthInPixels() const override;
        unsigned int GetHeightInPixels() const override;

        // RENDERING.
        void Display(const GRAPHICS::IMAGES::Bitmap& bitmap) override;

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
