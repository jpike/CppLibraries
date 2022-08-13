#pragma once

#include <memory>
#include <unordered_map>
#include "Graphics/Mesh.h"

// Forward declarations.
namespace GRAPHICS::OPEN_GL
{
    class VertexBuffer;
}

namespace GRAPHICS::MODELING
{
    /// A 3D model that may be composed of multiple meshes.
    /// The structure is based on how models are often done in 3D modeling software.
    class Model
    {
    public:
        /// Meshes of the model, mapped by name.
        std::unordered_map<std::string, Mesh> MeshesByName = {};

        /// OpenGL vertex buffer, if populated with all mesh data.
        std::weak_ptr<GRAPHICS::OPEN_GL::VertexBuffer> OpenGLVertexBuffer = {};
    };
}
