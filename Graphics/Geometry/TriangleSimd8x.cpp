#include "Graphics/Geometry/TriangleSimd8x.h"

namespace GRAPHICS::GEOMETRY
{
    /// Loads a triangle into 8-wide SIMD format.
    /// @param[in]  triangle - The triangle to load into SIMD format.
    /// @return The triangle in SIMD format.
    TriangleSimd8x TriangleSimd8x::Load(const Triangle& triangle)
    {
        TriangleSimd8x simd_triangle;

        // Triangle coordinates are in counterclockwise order.
        // The names of the vertices below may not be strictly "correct",
        // but the basic idea holds for each vertex relative to each other.
        //             left_vertex
        //                 /\
        //                /  \
        // center_vertex /____\ right_vertex
        const VertexWithAttributes& center_vertex = triangle.Vertices[1];
        const VertexWithAttributes& left_vertex = triangle.Vertices[0];
        const VertexWithAttributes& right_vertex = triangle.Vertices[2];

        simd_triangle.CenterVertexPosition.X = _mm256_set1_ps(center_vertex.Position.X);
        simd_triangle.CenterVertexPosition.Y = _mm256_set1_ps(center_vertex.Position.Y);
        simd_triangle.CenterVertexPosition.Z = _mm256_set1_ps(center_vertex.Position.Z);

        simd_triangle.LeftVertexPosition.X = _mm256_set1_ps(left_vertex.Position.X);
        simd_triangle.LeftVertexPosition.Y = _mm256_set1_ps(left_vertex.Position.Y);
        simd_triangle.LeftVertexPosition.Z = _mm256_set1_ps(left_vertex.Position.Z);

        simd_triangle.RightVertexPosition.X = _mm256_set1_ps(right_vertex.Position.X);
        simd_triangle.RightVertexPosition.Y = _mm256_set1_ps(right_vertex.Position.Y);
        simd_triangle.RightVertexPosition.Z = _mm256_set1_ps(right_vertex.Position.Z);

        MATH::Vector2f center_vertex_position_2d(center_vertex.Position.X, center_vertex.Position.Y);
        MATH::Vector2f left_vertex_position_2d(left_vertex.Position.X, left_vertex.Position.Y);
        MATH::Vector2f right_vertex_position_2d(right_vertex.Position.X, right_vertex.Position.Y);
        float signed_distance_of_right_vertex_from_left_edge = Triangle::SignedDistanceOfPointFromEdge2D(center_vertex_position_2d, left_vertex_position_2d, right_vertex_position_2d);
        simd_triangle.SignedDistanceOfRightVertexFromLeftEdge = _mm256_set1_ps(signed_distance_of_right_vertex_from_left_edge);
        float signed_distance_of_left_vertex_from_right_edge = Triangle::SignedDistanceOfPointFromEdge2D(center_vertex_position_2d, right_vertex_position_2d, left_vertex_position_2d);
        simd_triangle.SignedDistanceOfLeftVertexFromRightEdge = _mm256_set1_ps(signed_distance_of_left_vertex_from_right_edge);

        simd_triangle.LeftEdgeBarycentricCoordinateFormulaComponents = TriangleSimd8xBarycentricCoordinateFormulaComponents::Compute(center_vertex_position_2d, left_vertex_position_2d);
        simd_triangle.RightEdgeBarycentricCoordinateFormulaComponents = TriangleSimd8xBarycentricCoordinateFormulaComponents::Compute(center_vertex_position_2d, right_vertex_position_2d);

        simd_triangle.FirstVertexColorRed = _mm256_set1_ps(triangle.Vertices[0].Color.Red);
        simd_triangle.FirstVertexColorGreen = _mm256_set1_ps(triangle.Vertices[0].Color.Green);
        simd_triangle.FirstVertexColorBlue = _mm256_set1_ps(triangle.Vertices[0].Color.Blue);

        simd_triangle.SecondVertexColorRed = _mm256_set1_ps(triangle.Vertices[1].Color.Red);
        simd_triangle.SecondVertexColorGreen = _mm256_set1_ps(triangle.Vertices[1].Color.Green);
        simd_triangle.SecondVertexColorBlue = _mm256_set1_ps(triangle.Vertices[1].Color.Blue);

        simd_triangle.ThirdVertexColorRed = _mm256_set1_ps(triangle.Vertices[2].Color.Red);
        simd_triangle.ThirdVertexColorGreen = _mm256_set1_ps(triangle.Vertices[2].Color.Green);
        simd_triangle.ThirdVertexColorBlue = _mm256_set1_ps(triangle.Vertices[2].Color.Blue);

        return simd_triangle;
    }

    MATH::Vector3Simd8x TriangleSimd8x::BarycentricCoordinates2DOf(const MATH::Vector2<__m256>& points)
    {
        // COMPUTE THE BARYCENTRIC COORDINATE RELATIVE TO THE LEFT EDGE.
        __m256 signed_distances_of_points_from_left_edge = SignedDistanceOfPointsFromEdge2D(LeftEdgeBarycentricCoordinateFormulaComponents, points);
        __m256 scaled_signed_distances_of_points_from_left_edge = _mm256_div_ps(signed_distances_of_points_from_left_edge, SignedDistanceOfRightVertexFromLeftEdge);

        // COMPUTE THE BARYCENTRIC COORDINATE RELATIVE TO THE RIGHT EDGE.
        __m256 signed_distances_of_points_from_right_edge = SignedDistanceOfPointsFromEdge2D(RightEdgeBarycentricCoordinateFormulaComponents, points);
        __m256 scaled_signed_distances_of_points_from_right_edge = _mm256_div_ps(signed_distances_of_points_from_right_edge, SignedDistanceOfLeftVertexFromRightEdge);

        // COMPUTE THE BARYCENTRIC COORDINATE FOR THE REMAINING EDGE.
        const __m256 ONE = _mm256_set1_ps(1.0f);
        __m256 one_minus_scaled_signed_distances_of_points_from_left_edge = _mm256_sub_ps(ONE, scaled_signed_distances_of_points_from_left_edge);
        __m256 scaled_signed_distances_of_points_from_opposite_edge = _mm256_sub_ps(one_minus_scaled_signed_distances_of_points_from_left_edge, scaled_signed_distances_of_points_from_right_edge);

        // RETURN THE FULL BARYCENTRIC COORDINATES FOR THE POINT.
        MATH::Vector3Simd8x barycentric_coordinates;
        barycentric_coordinates.X = scaled_signed_distances_of_points_from_opposite_edge;
        barycentric_coordinates.Y = scaled_signed_distances_of_points_from_left_edge;
        barycentric_coordinates.Z = scaled_signed_distances_of_points_from_right_edge;
        return barycentric_coordinates;
    }

    __m256 TriangleSimd8x::SignedDistanceOfPointsFromEdge2D(const TriangleSimd8xBarycentricCoordinateFormulaComponents& edge, const MATH::Vector2<__m256>& points)
    {
        __m256 point_x_term = _mm256_mul_ps(edge.EdgeStartEndYDistance8x, points.X);
        __m256 point_y_term = _mm256_mul_ps(edge.EdgeEndStartXDistance8x, points.Y);
        __m256 point_sum = _mm256_add_ps(point_x_term, point_y_term);
        __m256 with_edge_start_end_added = _mm256_add_ps(point_sum, edge.EdgeStartXEndYProduct8x);
        __m256 signed_distances_of_points_from_edge = _mm256_sub_ps(with_edge_start_end_added, edge.EdgeEndXStartYProduct8x);
        return signed_distances_of_points_from_edge;
    }
}
