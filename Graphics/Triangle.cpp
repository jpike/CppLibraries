#include "Graphics/Triangle.h"

namespace GRAPHICS
{
    /// Creates an equilateral triangle with the specified material.
    /// @param[in]  material - The material of the triangle.
    /// @return An equilateral triangle of the specified material.
    Triangle Triangle::CreateEquilateral(const std::shared_ptr<GRAPHICS::Material>& material)
    {
        // DEFINE THE OBJECT-SPACE POSITIONS OF THE TRIANGLE VERTICES.
        // The Pythagorean theorem is used to compute the coordinates of the vertices:
        //      /|\
        //     /_|_\
        // height^2 + half_width^2 = side_length^2
        // The efficiency of these calculations has not been measured.
        float side_length = 1.0f;
        float side_length_squared = side_length * side_length;
        float half_width = side_length / 2.0f;
        float half_width_squared = half_width * half_width;
        float height_squared = side_length_squared - half_width_squared;
        float height = sqrt(height_squared);
        float half_height = height / 2.0f;
        float top_x = 0.0f;
        float top_y = half_height;
        float bottom_y = -half_height;
        float left_x = -half_width;
        float right_x = half_width;
        float z = 0.0f;

        // SET THE VERTICES IN A TRIANGLE.
        Triangle triangle;
        triangle.Material = material;
        triangle.Vertices =
        {
            MATH::Vector3f(top_x, top_y, z),
            MATH::Vector3f(left_x, bottom_y, z),
            MATH::Vector3f(right_x, bottom_y, z)
        };
        return triangle;
    }

    /// Constructs a triangle with the given material and vertices.
    /// @param[in]  material - The material for the triangle.
    /// @param[in]  vertices - The vertices of the triangle.
    Triangle::Triangle(const std::shared_ptr<GRAPHICS::Material>& material, const std::array<MATH::Vector3f, Triangle::VERTEX_COUNT>& vertices):
        Material(material),
        Vertices(vertices)
    {}

    /// Computes the surface normal of the triangle.
    /// @return The unit surface normal.
    MATH::Vector3f Triangle::SurfaceNormal() const
    {
        // The edges are calculated relative to the first vertex.
        // Since they're in a counter-clockwise order, the vertex for
        // the "first" edge should be the first component of the cross
        // product to get an outward-facing normal.
        MATH::Vector3f first_edge = Vertices[1] - Vertices[0];
        MATH::Vector3f second_edge = Vertices[2] - Vertices[0];
        MATH::Vector3f surface_normal = MATH::Vector3f::CrossProduct(first_edge, second_edge);
        MATH::Vector3f normalized_surface_normal = MATH::Vector3f::Normalize(surface_normal);
        return normalized_surface_normal;
    }
}
