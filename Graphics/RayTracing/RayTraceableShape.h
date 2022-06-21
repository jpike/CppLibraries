#pragma once

#include <memory>
#include <variant>
#include "Graphics/Material.h"
#include "Math/Vector3.h"

namespace GRAPHICS::GEOMETRY
{
    // Forward declarations.
    class Sphere;
    class Triangle;
}

namespace GRAPHICS::RAY_TRACING
{
    /// A shape that is supported for ray tracing.
    /// This class helps encapsulate some functionality related to ray tracing different kinds of shapes.
    class RayTraceableShape
    {
    public:
        // PUBLIC METHODS.
        std::shared_ptr<Material> GetMaterial() const;
        MATH::Vector3f GetSurfaceNormal(const MATH::Vector3f& surface_point) const;

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// The shape.  Memory is managed externally (outside of this class).
        std::variant<std::monostate, const GRAPHICS::GEOMETRY::Triangle*, const GRAPHICS::GEOMETRY::Sphere*> Shape = {};
    };
}
