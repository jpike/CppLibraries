// This code is only compiled in if the SDL library is available to allow
// the larger windowing library to be used without SDL.
#if __has_include(<SDL/SDL.h>)

#include <cassert>
#include "ErrorHandling/Asserts.h"
#include "Windowing/SdlWindow.h"

namespace WINDOWING
{
    /// Attempts to created a window.
    /// @param[in]  title - The title to display in the window's title bar.
    /// @param[in]  width_in_pixels - The width (in pixels) of the client rendering area of the window.
    /// @param[in]  height_in_pixels - The height (in pixels) of the client rendering area of the window.
    /// @param[in]  graphics_device_type - The type of graphics device to be used for rendering to the window.
    /// @param[in]  additional_window_settings - Additional settings to apply to the window.
    ///     Should be any of the SDL_WindowFlags OR'd together.
    /// @return The window, if successfully created; null if not.
    std::unique_ptr<SdlWindow> SdlWindow::Create(
        const char* title,
        const unsigned int width_in_pixels,
        const unsigned int height_in_pixels,
        const GRAPHICS::HARDWARE::GraphicsDeviceType graphics_device_type,
        const Uint32 additional_window_settings)
    {
        // TRY TO CREATE THE WINDOW.
        SDL_WindowFlags window_settings = (SDL_WindowFlags)(SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | additional_window_settings);
        bool open_gl_used = (GRAPHICS::HARDWARE::GraphicsDeviceType::OPEN_GL == graphics_device_type);
        if (open_gl_used)
        {
            window_settings = (SDL_WindowFlags)(window_settings | SDL_WINDOW_OPENGL);
        }
        SDL_Window* sdl_window = SDL_CreateWindow(
            title,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width_in_pixels,
            height_in_pixels,
            window_settings);
        ASSERT_THEN_IF_NOT(sdl_window)
        {
            // INDICATE THAT THE WINDOW COULD NOT BE CREATED.
            return nullptr;
        }

        // GET ADDITIONAL INFORMATION ABOUT THE WINDOW.
        SDL_SysWMinfo window_info;
        SDL_VERSION(&window_info.version);
        bool window_info_obtained = SDL_GetWindowWMInfo(sdl_window, &window_info);
        ASSERT_THEN_IF_NOT(window_info_obtained)
        {
            // INDICATE THAT THE WINDOW COULD NOT BE CREATED.
            return nullptr;
        }

        // GET THE UPDATED WINDOW SIZE.
        // In case the window was maximized, the size might alter from what was passed in.
        int updated_window_width_in_pixels = static_cast<int>(width_in_pixels);
        int updated_window_height_in_pixels = static_cast<int>(height_in_pixels);
        SDL_GetWindowSize(sdl_window, &updated_window_width_in_pixels, &updated_window_height_in_pixels);

        // RETURN THE CREATED WINDOW.
        auto window = std::make_unique<SdlWindow>();
        window->UnderlyingWindow = sdl_window;
        window->AdditionalInfo = window_info;
        window->WidthInPixels = updated_window_width_in_pixels;
        window->HeightInPixels = updated_window_height_in_pixels;
        window->IsOpen = true;
        return window;
    }

    /// Closes the window.
    SdlWindow::~SdlWindow()
    {
        Close();
    }

    /// Closes the window.
    void SdlWindow::Close()
    {
        SDL_DestroyWindow(UnderlyingWindow);
        UnderlyingWindow = nullptr;
    }

    /// Gets the width of the window's display area.
    /// @return The width of the window's display area, in pixels.
    unsigned int SdlWindow::GetWidthInPixels() const
    {
        return WidthInPixels;
    }

    /// Gets the height of the window's display area.
    /// @return The height of the window's display area, in pixels.
    unsigned int SdlWindow::GetHeightInPixels() const
    {
        return HeightInPixels;
    }

    /// Displays a bitmap in the window, stretching to fill the window if necessary.
    /// @param[in]  bitmap - The bitmap to display.
    void SdlWindow::Display(const GRAPHICS::IMAGES::Bitmap& bitmap)
    {
        // MAKE SURE THE WINDOW SURFACE CAN BE OBTAINED.
        SDL_Surface* window_surface = SDL_GetWindowSurface(UnderlyingWindow);
        ASSERT_THEN_IF_NOT(window_surface)
        {
            return;
        }

        // DETERMINE THE APPROPRIATE SDL PIXEL FORMAT OF THE BITMAP.
        SDL_PixelFormatEnum sdl_source_pixel_format = SDL_PIXELFORMAT_RGBA8888;
        GRAPHICS::ColorFormat source_color_format = bitmap.GetColorFormat();
        switch (source_color_format)
        {
            case GRAPHICS::ColorFormat::RGBA:
            {
                sdl_source_pixel_format = SDL_PIXELFORMAT_RGBA8888;
                break;
            }
            case GRAPHICS::ColorFormat::ARGB:
            {
                sdl_source_pixel_format = SDL_PIXELFORMAT_ARGB8888;
                break;
            }
            default:
            {
                // USE A DEFAULT PIXEL FORMAT IF AN UNSUPPORTED FORMAT IS DETECTED.
                constexpr bool UNSUPPORTED_COLOR_FORMAT = false;
                ASSERT_THEN_IF(UNSUPPORTED_COLOR_FORMAT)
                {
                    sdl_source_pixel_format = SDL_PIXELFORMAT_RGBA8888;
                    break;
                }
            }
        }

        // COPY THE BITMAP'S PIXELS TO THE WINDOW'S SURFACE.
        /// @todo   This is not fully safe.  Get minimum of dimensions?  Or scale pixels based on some sampling rate?
        unsigned int source_bitmap_width_in_pixels = bitmap.GetWidthInPixels();
        unsigned int source_bitmap_height_in_pixels = bitmap.GetHeightInPixels();
        const uint32_t* source_bitmap_raw_pixels = bitmap.GetRawData();
        unsigned int source_bitmap_row_byte_count = bitmap.GetRowByteCount();
        int pixel_copy_return_code = SDL_ConvertPixels(
            static_cast<int>(source_bitmap_width_in_pixels),
            static_cast<int>(source_bitmap_height_in_pixels),
            sdl_source_pixel_format,
            source_bitmap_raw_pixels,
            source_bitmap_row_byte_count,
            window_surface->format->format,
            window_surface->pixels,
            window_surface->pitch);
        bool pixels_copied_successfully = (0 == pixel_copy_return_code);
        ASSERT_THEN_IF_NOT(pixels_copied_successfully)
        {
            return;
        }

        // DISPLAY THE RENDERED SURFACE TO SCREEN.
        int surface_display_return_code = SDL_UpdateWindowSurface(UnderlyingWindow);
        bool surface_displayed_successfully = (0 == surface_display_return_code);
        assert(surface_displayed_successfully);
    }
}

#endif
