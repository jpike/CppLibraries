#pragma once

#if _WIN32

#include <memory>
#include <string>
#include <Windows.h>
#include "Graphics/Images/Bitmap.h"
#include "Windowing/IWindow.h"

/// Holds code for graphical user interface windows.
namespace WINDOWING
{
    /// A window implemented using the Win32 API for the Windows operating system.
    class Win32Window : public IWindow
    {
    public:
        // CONSTRUCTION/DESTRUCTION.
        static std::unique_ptr<Win32Window> Create(
            const WNDCLASSEX& window_class,
            const std::string& window_title,
            const int width_in_pixels = CW_USEDEFAULT,
            const int height_in_pixels = CW_USEDEFAULT);
        explicit Win32Window(const HWND window_handle);
        /// Destructor to support proper destruction when inheriting from an interface.
        virtual ~Win32Window() = default;

        // DIMENSIONS.
        unsigned int GetWidthInPixels() const override;
        unsigned int GetHeightInPixels() const override;

        // RENDERING.
        void Display(const GRAPHICS::IMAGES::Bitmap& bitmap) override;
        void DisplayAt(const GRAPHICS::IMAGES::Bitmap& bitmap, int left_x, int top_y);

        // MEMBER VARIABLES.
        /// The handle to window.
        HWND WindowHandle;
    };
}

#endif
