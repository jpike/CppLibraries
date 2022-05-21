#pragma once

#include <unordered_map>
#include "Graphics/Modeling/Mesh.h"

namespace GRAPHICS::MODELING
{
    class Model
    {
    public:
        /// Meshes of the model, mapped by name.
        std::unordered_map<std::string, Mesh> MeshesByName = {};
    };
}
