#pragma once

#include "Graphics/Color.h"
#include "Graphics/Geometry/Triangle.h"
#include "Graphics/Images/Bitmap.h"
#include "Math/Vector2.h"

namespace GRAPHICS
{
    /// An algorithm to assist in mapping a texture onto a surface
    /// (https://en.wikipedia.org/wiki/Texture_mapping).
    class TextureMappingAlgorithm
    {
    public:
        static Color LookupTexel(
            const GEOMETRY::Triangle& triangle,
            const MATH::Vector2f& triangle_point,
            const IMAGES::Bitmap& texture);
    };
}
