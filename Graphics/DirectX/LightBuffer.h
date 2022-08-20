#pragma once

#include <DirectXMath.h>

namespace GRAPHICS::DIRECT_X
{
    /// The buffer on the GPU to hold lighting information.
    struct LightBuffer
    {
        /// The world position of the light.
        DirectX::XMFLOAT4 LightPosition;
        /// The color of the light.
        DirectX::XMFLOAT4 InputLightColor;
        /// True if texturing and lighting are enabled; false if not.
        DirectX::XMINT2 IsTexturedAndLit;
        /// Some additional padding to meet 16-byte width requirements
        /// (https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_buffer_desc).
        DirectX::XMINT2 PaddingToMeet16ByteMultiple;
    };
}
