#pragma once

#include "Graphics/Images/Bitmap.h"

namespace WINDOWING
{
    /// A base interface to support different types of windows.
    class IWindow
    {
    public:
        // DESTRUCTOR.
        /// Virtual destructor to support proper derived class destruction.
        virtual ~IWindow() = default;

        // DIMENSIONS.
        /// Gets the width of the window's display area.
        /// @return The width of the window's display area, in pixels.
        virtual unsigned int GetWidthInPixels() const = 0;
        /// Gets the height of the window's display area.
        /// @return The height of the window's display area, in pixels.
        virtual unsigned int GetHeightInPixels() const = 0;

        // RENDERING.
        /// Displays the specified bitmap in the window.
        /// @param[in]  bitmap - The bitmap to display in the window.
        virtual void Display(const GRAPHICS::IMAGES::Bitmap& bitmap) = 0;
    };
}
