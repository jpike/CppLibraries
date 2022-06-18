#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include "Graphics/Material.h"
#include "Graphics/RayTracing/Ray.h"
#include "Graphics/RayTracing/RayObjectIntersection.h"
#include "Graphics/VertexWithAttributes.h"
#include "Math/Vector3.h"

namespace GRAPHICS
{
    /// A triangle that can be rendered.
    class Triangle
    {
    public:
        // STATIC CONSTANTS.
        /// The number of vertices in a triangle.
        static constexpr std::size_t VERTEX_COUNT = 3;

        // CONSTRUCTION.
        static Triangle CreateEquilateral(const std::shared_ptr<Material>& material);
        explicit Triangle() = default;
        explicit Triangle(const std::shared_ptr<Material>& material, const std::array<VertexWithAttributes, VERTEX_COUNT>& vertices);

        // OPERATORS.
        /// Default comparison operator.
        auto operator<=>(const Triangle&) const = default;

        // OTHER METHODS.
        MATH::Vector3f SurfaceNormal() const;
        std::optional<RAY_TRACING::RayObjectIntersection> Intersect(const RAY_TRACING::Ray& ray) const;
        MATH::Vector3f BarycentricCoordinates2DOf(const MATH::Vector2f& point) const;
        float SignedDistanceOfPointFromEdge2D(const MATH::Vector2f& edge_start_position, const MATH::Vector2f& edge_end_position, const MATH::Vector2f& point) const;
        MATH::Vector3f BarycentricCoordinates3DOf(const MATH::Vector3f& point) const;

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// The material of the triangle.
        std::shared_ptr<class Material> Material = nullptr;
        /// The vertices of the triangle.
        /// Should be in counter-clockwise order.
        std::array<VertexWithAttributes, VERTEX_COUNT> Vertices = std::array<VertexWithAttributes, VERTEX_COUNT>();
    };
}
