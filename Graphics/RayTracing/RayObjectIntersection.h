#pragma once

#include <limits>
#include <limits>
#include "Graphics/RayTracing/Ray.h"
#include "Graphics/Surface.h"
#include "Math/Vector3.h"

namespace GRAPHICS::RAY_TRACING
{
    /// An intersection between a ray and an object in a 3D scene.
    class RayObjectIntersection
    {
    public:
        // COMPUTATION.
        MATH::Vector3f IntersectionPoint() const;

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// The ray that intersected an object.  Memory is managed externally (outside of this class).
        const Ray* Ray = nullptr;
        /// The distance along the ray to the intersection of the object (in units of the ray).
        /// Initialized to infinity to avoid accidental intersections caused by checking
        /// if this distance is closer between two intersections.
        float DistanceFromRayToObject = std::numeric_limits<float>::infinity();
        /// The intersected object.
        Surface Object = {};
    };
}
