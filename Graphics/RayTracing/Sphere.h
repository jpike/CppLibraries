#pragma once

#include <memory>
#include "Graphics/Material.h"
#include "Graphics/RayTracing/Ray.h"
#include "Graphics/RayTracing/RayObjectIntersection.h"
#include "Math/Vector3.h"

namespace GRAPHICS::RAY_TRACING
{
    /// A sphere that can be ray traced.
    class Sphere
    {
    public:
        // PUBLIC METHODS.
        MATH::Vector3f SurfaceNormal(const MATH::Vector3f& surface_point) const;
        std::optional<RayObjectIntersection> Intersect(const Ray& ray) const;

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// The center of the sphere in world coordinates.
        MATH::Vector3f CenterPosition = MATH::Vector3f(0.0f, 0.0f, 0.0f);
        /// The radius of the sphere.
        float Radius = 0.0f;
        /// The material defining surface properties of the sphere.
        std::shared_ptr<Material> Material = nullptr;
    };
}
