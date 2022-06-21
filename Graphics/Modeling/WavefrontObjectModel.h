#pragma once

#include <filesystem>
#include <optional>
#include "Graphics/Modeling/Model.h"
#include "Graphics/Object3D.h"

/// Holds code related to 3D models in computer graphics.
namespace GRAPHICS::MODELING
{
    /// @todo
    /// first index is 1 and increases corresponding to order items previously defined.
    struct FaceVertexAttributeIndices
    {
        constexpr static std::size_t UNSET_INDEX = 0;
        constexpr static std::size_t OFFSET_FROM_ZERO_BASED_INDEX = 1;

        std::size_t VertexPositionIndex = UNSET_INDEX;
        std::size_t VertexColorIndex = UNSET_INDEX;
        std::size_t VertexTextureCoordinateIndex = UNSET_INDEX;
        std::size_t VertexNormalIndex = UNSET_INDEX;
    };

    /// A model in Wavefront .obj format (https://en.wikipedia.org/wiki/Wavefront_.obj_file).
    /// This is generally the simplest widely-supported 3D model format that is readable as plain text.
    /// This class is named based on the "model" concept rather than a "file" concept since
    /// a 3D model may include additional files such as a .mtl material file.
    /// This class only supports files with triangulated faces (not quads).
    class WavefrontObjectModel
    {
    public:
        static std::optional<GRAPHICS::MODELING::Model> Load(const std::filesystem::path& filepath);
    };
}
