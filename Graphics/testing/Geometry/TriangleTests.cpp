#include <catch.hpp>
#include "Graphics/Geometry/Triangle.h"

TEST_CASE("Barycentric coordinates for each vertex of a triangle can be computed.", "[Triangle][BarycentricCoordinates2DOf]")
{
    // DEFINE A TRIANGLE.
    // The triangle is designed to look like:
    // |\
    // |_\ 
    GRAPHICS::GEOMETRY::Triangle triangle;
    triangle.Vertices =
    {
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(-1.0f, 1.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(-1.0f, -1.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(1.0f, -1.0f, 0.0f),
        },
    };

    // COMPUTE BARYCENTRIC COORDINATES FOR EACH VERTEX OF THE TRIANGLE.
    MATH::Vector2f first_vertex_position(triangle.Vertices[0].Position.X, triangle.Vertices[0].Position.Y);
    MATH::Vector3f actual_first_vertex_barycentric_coordinates = triangle.BarycentricCoordinates2DOf(first_vertex_position);
    REQUIRE(0.0f == actual_first_vertex_barycentric_coordinates.X);
    REQUIRE(0.0f == actual_first_vertex_barycentric_coordinates.Y);
    REQUIRE(1.0f == actual_first_vertex_barycentric_coordinates.Z);

    MATH::Vector2f second_vertex_position(triangle.Vertices[1].Position.X, triangle.Vertices[1].Position.Y);
    MATH::Vector3f actual_second_vertex_barycentric_coordinates = triangle.BarycentricCoordinates2DOf(second_vertex_position);
    REQUIRE(1.0f == actual_second_vertex_barycentric_coordinates.X);
    REQUIRE(0.0f == actual_second_vertex_barycentric_coordinates.Y);
    REQUIRE(0.0f == actual_second_vertex_barycentric_coordinates.Z);

    MATH::Vector2f third_vertex_position(triangle.Vertices[2].Position.X, triangle.Vertices[2].Position.Y);
    MATH::Vector3f actual_third_vertex_barycentric_coordinates = triangle.BarycentricCoordinates2DOf(third_vertex_position);
    REQUIRE(0.0f == actual_third_vertex_barycentric_coordinates.X);
    REQUIRE(1.0f == actual_third_vertex_barycentric_coordinates.Y);
    REQUIRE(0.0f == actual_third_vertex_barycentric_coordinates.Z);
}

TEST_CASE("Barycentric coordinates for midpoints of each edge of a triangle can be computed.", "[Triangle][BarycentricCoordinates2DOf]")
{
    // DEFINE A TRIANGLE.
    // The triangle is designed to look like:
    // |\
    // |_\ 
    GRAPHICS::GEOMETRY::Triangle triangle;
    triangle.Vertices =
    {
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(-1.0f, 1.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(-1.0f, -1.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(1.0f, -1.0f, 0.0f),
        },
    };

    // COMPUTE BARYCENTRIC COORDINATES FOR MIDPOINTS ALONG EACH EDGE.
    MATH::Vector2f left_edge_midpoint(-1.0f, 0.0f);
    MATH::Vector3f actual_left_edge_midpoint_barycentric_coordinates = triangle.BarycentricCoordinates2DOf(left_edge_midpoint);
    REQUIRE(0.5f == actual_left_edge_midpoint_barycentric_coordinates.X);
    REQUIRE(0.0f == actual_left_edge_midpoint_barycentric_coordinates.Y);
    REQUIRE(0.5f == actual_left_edge_midpoint_barycentric_coordinates.Z);

    MATH::Vector2f right_edge_midpoint(0.0f, -1.0f);
    MATH::Vector3f actual_right_edge_midpoint_barycentric_coordinates = triangle.BarycentricCoordinates2DOf(right_edge_midpoint);
    REQUIRE(0.5f == actual_right_edge_midpoint_barycentric_coordinates.X);
    REQUIRE(0.5f == actual_right_edge_midpoint_barycentric_coordinates.Y);
    REQUIRE(0.0f == actual_right_edge_midpoint_barycentric_coordinates.Z);

    MATH::Vector2f opposite_edge_midpoint(0.0f, 0.0f);
    MATH::Vector3f actual_opposite_edge_midpoint_barycentric_coordinates = triangle.BarycentricCoordinates2DOf(opposite_edge_midpoint);
    REQUIRE(0.0f == actual_opposite_edge_midpoint_barycentric_coordinates.X);
    REQUIRE(0.5f == actual_opposite_edge_midpoint_barycentric_coordinates.Y);
    REQUIRE(0.5f == actual_opposite_edge_midpoint_barycentric_coordinates.Z);
}

TEST_CASE("Barycentric coordinates for a point outside and above a triangle can be correctly computed.", "[Triangle][BarycentricCoordinates2DOf]")
{
    // DEFINE A TRIANGLE.
    // The triangle is designed to look like:
    // |\
    // |_\ 
    GRAPHICS::GEOMETRY::Triangle triangle;
    triangle.Vertices =
    {
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(-1.0f, 1.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(-1.0f, -1.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(1.0f, -1.0f, 0.0f),
        },
    };

    // COMPUTE BARYCENTRIC COORDINATES FOR A POINT OUTSIDE AND ABOVE THE TRIANGLE.
    MATH::Vector2f point_outside_above(0.0f, 2.0f);
    MATH::Vector3f actual_point_barycentric_coordinates = triangle.BarycentricCoordinates2DOf(point_outside_above);
    CHECK(-1.0f == actual_point_barycentric_coordinates.X);
    CHECK(0.5f == actual_point_barycentric_coordinates.Y);
    CHECK(1.5f == actual_point_barycentric_coordinates.Z);
}

TEST_CASE("Barycentric coordinates for a point outside and left of a triangle can be correctly computed.", "[Triangle][BarycentricCoordinates2DOf]")
{
    // DEFINE A TRIANGLE.
    // The triangle is designed to look like:
    // |\
    // |_\ 
    GRAPHICS::GEOMETRY::Triangle triangle;
    triangle.Vertices =
    {
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(-1.0f, 1.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(-1.0f, -1.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(1.0f, -1.0f, 0.0f),
        },
    };

    // COMPUTE BARYCENTRIC COORDINATES FOR A POINT OUTSIDE AND LEFT OF THE TRIANGLE.
    MATH::Vector2f point_outside_left(-2.0f, 0.0f);
    MATH::Vector3f actual_point_barycentric_coordinates = triangle.BarycentricCoordinates2DOf(point_outside_left);
    CHECK(1.0f == actual_point_barycentric_coordinates.X);
    CHECK(-0.5f == actual_point_barycentric_coordinates.Y);
    CHECK(0.5f == actual_point_barycentric_coordinates.Z);
}

TEST_CASE("Barycentric coordinates for a point outside and below a triangle can be correctly computed.", "[Triangle][BarycentricCoordinates2DOf]")
{
    // DEFINE A TRIANGLE.
    // The triangle is designed to look like:
    // |\
    // |_\ 
    GRAPHICS::GEOMETRY::Triangle triangle;
    triangle.Vertices =
    {
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(-1.0f, 1.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(-1.0f, -1.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(1.0f, -1.0f, 0.0f),
        },
    };

    // COMPUTE BARYCENTRIC COORDINATES FOR A POINT OUTSIDE AND BELOW THE TRIANGLE.
    MATH::Vector2f point_outside_below(0.0f, -2.0f);
    MATH::Vector3f actual_point_barycentric_coordinates = triangle.BarycentricCoordinates2DOf(point_outside_below);
    CHECK(1.0f == actual_point_barycentric_coordinates.X);
    CHECK(0.5f == actual_point_barycentric_coordinates.Y);
    CHECK(-0.5f == actual_point_barycentric_coordinates.Z);
}

TEST_CASE("Barycentric coordinates for a point outside and right of a triangle can be correctly computed.", "[Triangle][BarycentricCoordinates2DOf]")
{
    // DEFINE A TRIANGLE.
    // The triangle is designed to look like:
    // |\
    // |_\ 
    GRAPHICS::GEOMETRY::Triangle triangle;
    triangle.Vertices =
    {
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(-1.0f, 1.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(-1.0f, -1.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(1.0f, -1.0f, 0.0f),
        },
    };

    // COMPUTE BARYCENTRIC COORDINATES FOR A POINT OUTSIDE AND RIGHT OF THE TRIANGLE.
    MATH::Vector2f point_outside_left(2.0f, 0.0f);
    MATH::Vector3f actual_point_barycentric_coordinates = triangle.BarycentricCoordinates2DOf(point_outside_left);
    CHECK(-1.0f == actual_point_barycentric_coordinates.X);
    CHECK(1.5f == actual_point_barycentric_coordinates.Y);
    CHECK(0.5f == actual_point_barycentric_coordinates.Z);
}

TEST_CASE("Barycentric coordinates for a point inside a triangle can be correctly computed.", "[Triangle][BarycentricCoordinates2DOf]")
{
    // DEFINE A TRIANGLE.
    // The triangle is designed to look like:
    // |\
    // |_\ 
    GRAPHICS::GEOMETRY::Triangle triangle;
    triangle.Vertices =
    {
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(-1.0f, 1.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(-1.0f, -1.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(1.0f, -1.0f, 0.0f),
        },
    };

    // COMPUTE BARYCENTRIC COORDINATES FOR A POINT INSIDE THE TRIANGLE.
    MATH::Vector2f point_inside_triangle(0.0f, -0.25f);
    MATH::Vector3f actual_point_barycentric_coordinates = triangle.BarycentricCoordinates2DOf(point_inside_triangle);
    CHECK(0.125f == actual_point_barycentric_coordinates.X);
    CHECK(0.5f == actual_point_barycentric_coordinates.Y);
    CHECK(0.375f == actual_point_barycentric_coordinates.Z);
}
