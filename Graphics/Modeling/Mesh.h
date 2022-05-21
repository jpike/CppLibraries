#pragma once

#include <string>
#include <vector>

// This code is only compiled in if the bgfx library is available to allow
// the larger graphics library to be used without bgfx.
#if __has_include(<bgfx/bgfx.h>)
#include <bgfx/bgfx.h>
#endif

#include "Graphics/Material.h"
#include "Graphics/Triangle.h"

namespace GRAPHICS::MODELING
{
    class Mesh
    {
    public:
        /// The name of the mesh.
        std::string Name = "";
        /// True if the mesh should be rendered; false if not.
        bool Visible = true;
        /// The triangles that make up this mesh, in the local coordinate space of the mesh.
        std::vector<Triangle> Triangles = {};
// This code is only compiled in if the bgfx library is available to allow
// the larger graphics library to be used without bgfx.
#if __has_include(<bgfx/bgfx.h>)
        /// The vertex buffer allocated for the mesh.
        bgfx::VertexBufferHandle VertexBuffer = { bgfx::kInvalidHandle };
#endif
        /// The material used for rendering this mesh.
        /// @todo   This does not really belong here?
        Material Material = {};
    };
}