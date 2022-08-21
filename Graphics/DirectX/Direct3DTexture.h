#pragma once

#include <d3d11.h>
#include "Graphics/DirectX/Direct3DGpuResource.h"

namespace GRAPHICS::DIRECT_X
{
    /// A texture allocated using Direct3D.
    /// Combines multiple pieces of data together for easier use.
    struct Direct3DTexture
    {
        /// The main texture resource.
        Direct3DGpuResource<ID3D11Texture2D> Texture = nullptr;
        /// The texture view for shaders.
        Direct3DGpuResource<ID3D11ShaderResourceView> TextureView = nullptr;
    };
}
