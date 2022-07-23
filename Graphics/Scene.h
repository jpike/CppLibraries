#pragma once

#include <optional>
#include <vector>
#include "Graphics/Color.h"
#include "Graphics/Object3D.h"
#include "Graphics/Shading/Lighting/Light.h"

namespace GRAPHICS
{
    /// A scene consisting of objects within a 3D space.
    class Scene
    {
    public:
        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// The background color of the scene.
        GRAPHICS::Color BackgroundColor = GRAPHICS::Color::BLACK;
        /// All lights in the scene.
        std::vector<SHADING::LIGHTING::Light> Lights = {};
        /// All objects in the scene.
        std::vector<Object3D> Objects = {};
    };
}
