#pragma once

#include <DirectXMath.h>

namespace GRAPHICS::DIRECT_X
{
    /// The buffer on the GPU to hold transformation matrices.
    struct TransformationMatrixBuffer
    {
        /// The world transformation matrix.
        DirectX::XMMATRIX WorldMatrix;
        /// The view transformation matrix.
        DirectX::XMMATRIX ViewMatrix;
        /// The projection transformation matrix.
        DirectX::XMMATRIX ProjectionMatrix;
    };
}
