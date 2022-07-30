#pragma once

#include <intrin.h>
#include "Graphics/Geometry/Triangle.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"

namespace GRAPHICS::GEOMETRY
{
    /// @todo
    struct TriangleSimd8xBarycentricCoordinateFormulaComponents
    {
        __m256 EdgeStartEndYDistance8x;
        __m256 EdgeEndStartXDistance8x;
        __m256 EdgeStartXEndYProduct8x;
        __m256 EdgeEndXStartYProduct8x;

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
    };
}
