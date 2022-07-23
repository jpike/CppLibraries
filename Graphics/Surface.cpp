#include "Graphics/Surface.h"

namespace GRAPHICS
{
    /// Gets the material associated with the surface, if one exists.
    /// @return The material associated with the surface, if one exists; null otherwise.
    std::shared_ptr<Material> Surface::GetMaterial() const
    {
        // GET THE MATERIAL ASSOCIATED WITH THE APPOPRIATE KIND OF SHAPE.
        const GEOMETRY::Triangle* const* triangle = std::get_if<const GEOMETRY::Triangle*>(&Shape);
        const GEOMETRY::Sphere* const* sphere = std::get_if<const GEOMETRY::Sphere*>(&Shape);
        if (triangle)
        {
            return (*triangle)->Material;
        }
        else if (sphere)
        {
            return (*sphere)->Material;
        }
        else
        {
            // INDICATE THAT NO MATERIAL COULD BE FOUND.
            return nullptr;
        }
    }

    /// Gets the surface normal of the shape at the specified point.
    /// @param[in]  surface_point - The point on the shape for which to get the normal.
    /// @return The surface normal of the shape at the specified point.
    MATH::Vector3f Surface::GetNormal(const MATH::Vector3f& surface_point) const
    {
        // GET THE SURFACE NORMAL FOR THE APPOPRIATE KIND OF SHAPE.
        const GEOMETRY::Triangle* const* triangle = std::get_if<const GEOMETRY::Triangle*>(&Shape);
        const GEOMETRY::Sphere* const* sphere = std::get_if<const GEOMETRY::Sphere*>(&Shape);
        if (triangle)
        {
            // The surface normal is the same across the entire shape.
            MATH::Vector3f unit_surface_normal = (*triangle)->SurfaceNormal();
            return unit_surface_normal;
        }
        else if (sphere)
        {
            MATH::Vector3f unit_surface_normal = (*sphere)->SurfaceNormal(surface_point);
            return unit_surface_normal;
        }
        else
        {
            // INDICATE THAT NO SURFACE NORMAL COULD BE FOUND.
            return MATH::Vector3f(0.0f, 0.0f, 0.0f);
        }
    }
}
