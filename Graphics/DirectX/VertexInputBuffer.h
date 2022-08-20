#pragma once

#include <DirectXMath.h>

namespace GRAPHICS::DIRECT_X
{
    /// A buffer on the GPU to hold input data for vertices.
    struct VertexInputBuffer
    {
        /// The vertex position.
        DirectX::XMFLOAT4 Position;
        /// The color of the vertex.
        DirectX::XMFLOAT4 Color;
        /// The normal of the vertex.
        DirectX::XMFLOAT4 Normal;
        /// The texture coordinates of the vertex.
        DirectX::XMFLOAT2 TextureCoordinates;
    };
}
