#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "Graphics/Modeling/Model.h"

namespace GRAPHICS::DIRECT_X
{
    /// A buffer on the GPU to hold input data for vertices.
    struct VertexInputBuffer
    {
        // STATIC METHODS.
        static ID3D11Buffer* Fill(const MODELING::Model& model, ID3D11Device& device);

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
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
