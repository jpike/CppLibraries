#pragma once

#include <optional>
#include <dxgi1_6.h>

namespace GRAPHICS::DIRECT_X
{
    /// A DirectX display mode for how graphics get displayed on a computer screen.
    class DisplayMode
    {
    public:
        static std::optional<DXGI_MODE_DESC> GetSmallestMatching(
            const unsigned int width_in_pixels,
            const unsigned int height_in_pixels);
    };
}
