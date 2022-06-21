#include "Graphics/RayTracing/Ray.h"

namespace GRAPHICS::RAY_TRACING
{
    /// Constructor.
    /// @param[in]  origin - See \ref Origin.
    /// @param[in]  direction - See \ref Direction.
    Ray::Ray(const MATH::Vector3f& origin, const MATH::Vector3f& direction) :
        Origin(origin),
        Direction(direction)
    {}
}
