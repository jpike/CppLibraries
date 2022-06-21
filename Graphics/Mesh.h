#pragma once

#include <string>
#include <vector>
#include "Graphics/Geometry/Triangle.h"

namespace GRAPHICS
{
    /// A triangle mesh (https://en.wikipedia.org/wiki/Triangle_mesh).
    class Mesh
    {
    public:
        /// The name of the mesh.
        std::string Name = "";
        /// True if the mesh should be rendered; false if not.
        bool Visible = true;
        /// The triangles that make up this mesh, in the local coordinate space of the mesh.
        std::vector<GEOMETRY::Triangle> Triangles = {};
    };
}