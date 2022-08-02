#pragma once

#include <intrin.h>
#include "Graphics/Geometry/Triangle.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"

namespace GRAPHICS::GEOMETRY
{
    /// Components of a formula for computing barycentric coordinates of a triangle.
    /// Structure is designed to support computation of these components once for SIMD operations.
    /// See @ref    Triangle::SignedDistanceOfPointFromEdge2D for more details on this formula.
    struct TriangleSimd8xBarycentricCoordinateFormulaComponents
    {
        /// (edge_start_position.Y - edge_end_position.Y)
        __m256 EdgeStartEndYDistance8x;
        /// (edge_end_position.X - edge_start_position.X)
        __m256 EdgeEndStartXDistance8x;
        /// (edge_start_position.X * edge_end_position.Y)
        __m256 EdgeStartXEndYProduct8x;
        /// (edge_end_position.X * edge_start_position.Y)
        __m256 EdgeEndXStartYProduct8x;

        /// Computes the formula components in SIMD format based on the non-SIMD input edge positions.
        /// @param[in]  edge_start_position - The start position of the edge.
        /// @param[in]  edge_end_position - The end position of the edge.
        static TriangleSimd8xBarycentricCoordinateFormulaComponents Compute(const MATH::Vector2f& edge_start_position, const MATH::Vector2f& edge_end_position)
        {
            TriangleSimd8xBarycentricCoordinateFormulaComponents formula_components;

            float edge_start_end_y_distance = (edge_start_position.Y - edge_end_position.Y);
            formula_components.EdgeStartEndYDistance8x = _mm256_set1_ps(edge_start_end_y_distance);
            float edge_end_start_x_distance = (edge_end_position.X - edge_start_position.X);
            formula_components.EdgeEndStartXDistance8x = _mm256_set1_ps(edge_end_start_x_distance);
            float edge_start_x_end_y_product = (edge_start_position.X * edge_end_position.Y);
            formula_components.EdgeStartXEndYProduct8x = _mm256_set1_ps(edge_start_x_end_y_product);
            float edge_end_x_start_y_product = (edge_end_position.X * edge_start_position.Y);
            formula_components.EdgeEndXStartYProduct8x = _mm256_set1_ps(edge_end_x_start_y_product);

            return formula_components;
        }
    };

    /// A triangle in an 8-wide (8x meaning "8 times") SIMD format.
    /// This class helps simplify SIMD operations for improved performance.
    class TriangleSimd8x
    {
    public:
        // METHODS.
        static TriangleSimd8x Load(const Triangle& triangle);

        MATH::Vector3Simd8x BarycentricCoordinates2DOf(const MATH::Vector2<__m256>& points);
        static __m256 SignedDistanceOfPointsFromEdge2D(const TriangleSimd8xBarycentricCoordinateFormulaComponents& edge, const MATH::Vector2<__m256>& points);

        // BASE TRIANGLE DATA.
        MATH::Vector3Simd8x CenterVertexPosition;
        MATH::Vector3Simd8x LeftVertexPosition;
        MATH::Vector3Simd8x RightVertexPosition;

        // BARYCENTRIC COORDINATE DATA.
        __m256 SignedDistanceOfRightVertexFromLeftEdge;
        __m256 SignedDistanceOfLeftVertexFromRightEdge;
        TriangleSimd8xBarycentricCoordinateFormulaComponents LeftEdgeBarycentricCoordinateFormulaComponents;
        TriangleSimd8xBarycentricCoordinateFormulaComponents RightEdgeBarycentricCoordinateFormulaComponents;

        // COLORS.
        __m256 FirstVertexColorRed;
        __m256 FirstVertexColorGreen;
        __m256 FirstVertexColorBlue;

        __m256 SecondVertexColorRed;
        __m256 SecondVertexColorGreen;
        __m256 SecondVertexColorBlue;

        __m256 ThirdVertexColorRed;
        __m256 ThirdVertexColorGreen;
        __m256 ThirdVertexColorBlue;

        // TEXTURE COORDINATES.
        MATH::Vector2Simd8x FirstVertexTextureCoordinates;
        MATH::Vector2Simd8x SecondVertexTextureCoordinates;
        MATH::Vector2Simd8x ThirdVertexTextureCoordinates;
    };
}
