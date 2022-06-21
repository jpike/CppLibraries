#include <catch.hpp>
#include "Graphics/Geometry/Sphere.h"

TEST_CASE("The surface normal of a sphere can be computed.", "[Sphere][SurfaceNormal]")
{
    // DEFINE AN ARBITRARY SPHERE.
    GRAPHICS::GEOMETRY::Sphere sphere =
    {
        .CenterPosition = MATH::Vector3f(0.0f, 0.0f, 0.0f),
        .Radius = 1.0f
    };
    
    // DEFINE AN ARBITRARY INTERSECTION POINT.
    MATH::Vector3f intersection_point(1.0f, 1.0f, 1.0f);

    // VERIFY THE CORRECT SURFACE NORMAL CAN BE COMPUTED.
    MATH::Vector3f actual_surface_normal = sphere.SurfaceNormal(intersection_point);
    CHECK(Approx(0.57735f) == actual_surface_normal.X);
    CHECK(Approx(0.57735f) == actual_surface_normal.Y);
    CHECK(Approx(0.57735f) == actual_surface_normal.Z);
}

TEST_CASE("A ray that doesn't intersect a sphere results in no intersection.", "[Sphere][Intersect]")
{
    // DEFINE AN ARBITRARY SPHERE.
    GRAPHICS::GEOMETRY::Sphere sphere =
    {
        .CenterPosition = MATH::Vector3f(0.0f, 0.0f, 0.0f),
        .Radius = 1.0f
    };

    // DEFINE A RAY THAT WON'T INTERSECT THE SPHERE.
    MATH::Vector3f ray_origin(2.0f, 2.0f, 2.0f);
    MATH::Vector3f ray_direction(2.0f, 2.0f, 2.0f);
    GRAPHICS::RAY_TRACING::Ray ray(ray_origin, ray_direction);

    // VERIFY THE RAY DOESN'T INTERSECT A SPHERE.
    std::optional<GRAPHICS::RAY_TRACING::RayObjectIntersection> intersection = sphere.Intersect(ray);
    CHECK_FALSE(intersection);
}

TEST_CASE("A ray that intersects a sphere results in an intersection.", "[Sphere][Intersect]")
{
    // DEFINE AN ARBITRARY SPHERE.
    GRAPHICS::GEOMETRY::Sphere sphere =
    {
        .CenterPosition = MATH::Vector3f(0.0f, 0.0f, 0.0f),
        .Radius = 1.0f
    };

    // DEFINE A RAY THAT WILL INTERSECT THE SPHERE.
    MATH::Vector3f ray_origin(2.0f, 2.0f, 2.0f);
    MATH::Vector3f ray_direction(-2.0f, -2.0f, -2.0f);
    GRAPHICS::RAY_TRACING::Ray ray(ray_origin, ray_direction);

    // VERIFY THE RAY INTERSECTS A SPHERE AT THE APPROPRIATE PLACE.
    std::optional<GRAPHICS::RAY_TRACING::RayObjectIntersection> intersection = sphere.Intersect(ray);
    REQUIRE(intersection);

    CHECK(&ray == intersection->Ray);
    CHECK(Approx(0.71132f) == intersection->DistanceFromRayToObject);
    const GRAPHICS::GEOMETRY::Sphere* const* intersected_sphere = std::get_if<const GRAPHICS::GEOMETRY::Sphere*>(&intersection->Object.Shape);
    REQUIRE(intersected_sphere);
    CHECK((*intersected_sphere) == &sphere);
}
