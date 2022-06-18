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
            VertexWithAttributes { .Position = MATH::Vector3f(top_x, top_y, z) },
            VertexWithAttributes { .Position = MATH::Vector3f(left_x, bottom_y, z) },
            VertexWithAttributes { .Position = MATH::Vector3f(right_x, bottom_y, z) }
        };
        return triangle;
    }

    /// Constructs a triangle with the given material and vertices.
    /// @param[in]  material - The material for the triangle.
    /// @param[in]  vertices - The vertices of the triangle.
    Triangle::Triangle(const std::shared_ptr<GRAPHICS::Material>& material, const std::array<VertexWithAttributes, Triangle::VERTEX_COUNT>& vertices):
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
        MATH::Vector3f first_edge = Vertices[1].Position - Vertices[0].Position;
        MATH::Vector3f second_edge = Vertices[2].Position - Vertices[0].Position;
        MATH::Vector3f surface_normal = MATH::Vector3f::CrossProduct(first_edge, second_edge);
        MATH::Vector3f normalized_surface_normal = MATH::Vector3f::Normalize(surface_normal);
        return normalized_surface_normal;
    }

    /// Checks for an intersection between a ray and the object.
    /// @param[in]  ray - The ray to check for intersection.
    /// @return A ray-object intersection, if one occurred; std::nullopt otherwise.
    std::optional<RAY_TRACING::RayObjectIntersection> Triangle::Intersect(const RAY_TRACING::Ray& ray) const
    {
        // GET THE TRIANGLE'S SURFACE NORMAL.
        MATH::Vector3f surface_normal = SurfaceNormal();

        // GET EACH OF THE TRIANGLES EDGES IN COUNTER-CLOCKWISE ORDER.
        MATH::Vector3f edge_a = Vertices[1].Position - Vertices[0].Position;
        MATH::Vector3f edge_b = Vertices[2].Position - Vertices[1].Position;
        MATH::Vector3f edge_c = Vertices[0].Position - Vertices[2].Position;

        // CHECK FOR INTERSECTION WITH THE PLANE.
        float distance_from_ray_to_object = MATH::Vector3f::DotProduct(surface_normal, Vertices[0].Position);
        distance_from_ray_to_object -= MATH::Vector3f::DotProduct(surface_normal, ray.Origin);
        distance_from_ray_to_object /= MATH::Vector3f::DotProduct(surface_normal, ray.Direction);
        bool intersection_in_front_of_current_view = (distance_from_ray_to_object >= 0.0f);
        if (!intersection_in_front_of_current_view)
        {
            // INDICATE THAT NO INTERSECTION OCCURRED.
            return std::nullopt;
        }

        // CHECK FOR INTERSECTION WITHIN THE TRIANGLE.
        MATH::Vector3f intersection_point = ray.Origin + MATH::Vector3f::Scale(distance_from_ray_to_object, ray.Direction);
        MATH::Vector3f edge_a_for_point = intersection_point - Vertices[0].Position;
        MATH::Vector3f edge_b_for_point = intersection_point - Vertices[1].Position;
        MATH::Vector3f edge_c_for_point = intersection_point - Vertices[2].Position;

        float dot_product_for_edge_a = MATH::Vector3f::DotProduct(surface_normal, MATH::Vector3f::CrossProduct(edge_a, edge_a_for_point));
        float dot_product_for_edge_b = MATH::Vector3f::DotProduct(surface_normal, MATH::Vector3f::CrossProduct(edge_b, edge_b_for_point));
        float dot_product_for_edge_c = MATH::Vector3f::DotProduct(surface_normal, MATH::Vector3f::CrossProduct(edge_c, edge_c_for_point));

        bool intersects_triangle = (dot_product_for_edge_a >= 0.0f) && (dot_product_for_edge_b >= 0.0f) && (dot_product_for_edge_c >= 0.0f);
        if (!intersects_triangle)
        {
            // INDICATE THAT NO INTERSECTION OCCURRED.
            return std::nullopt;
        }

        // RETURN INFORMATION ABOUT THE INTERSECTION.
        RAY_TRACING::RayObjectIntersection intersection;
        intersection.Ray = &ray;
        intersection.DistanceFromRayToObject = distance_from_ray_to_object;
        intersection.Object = this;
        return intersection;
    }

    /// Computes the barycentric coordinates of a point relative to the triangle.
    /// Note that this method only computes 2D barycentric coordinates.
    /// Barycentric coordinates are effectively coordinates that indicate where a point is relative to the inside of a triangle:
    /// - A point inside (including edges) will have all 3 coordinates between [0,1].
    /// - Positive or negative values indicate where a point is relative to a particular edge.
    /// @param[in]  point - The point for which to compute barycentric coordinates.
    /// @return The point in barycentric coordinates relative to the triangle.
    MATH::Vector3f Triangle::BarycentricCoordinates2DOf(const MATH::Vector2f& point) const
    {
        // DEFINE THE VERTICES FOR THE BARYCENTRIC COORDINATE SYSTEM.
        // Triangle coordinates are in counterclockwise order.
        // The names of the vertices below may not be strictly "correct",
        // but the basic idea holds for each vertex relative to each other.
        //             left_vertex
        //                 /\
        //                /  \
        // center_vertex /____\ right_vertex
        MATH::Vector2f center_vertex_position(Vertices[1].Position.X, Vertices[1].Position.Y);
        MATH::Vector2f left_vertex_position(Vertices[0].Position.X, Vertices[0].Position.Y);
        MATH::Vector2f right_vertex_position(Vertices[2].Position.X, Vertices[2].Position.Y);

        // COMPUTE THE BARYCENTRIC COORDINATE RELATIVE TO THE LEFT EDGE.
        float signed_distance_of_point_from_left_edge = SignedDistanceOfPointFromEdge2D(center_vertex_position, left_vertex_position, point);
        float signed_distance_of_right_vertex_from_left_edge = SignedDistanceOfPointFromEdge2D(center_vertex_position, left_vertex_position, right_vertex_position);
        float scaled_signed_distance_of_point_from_left_edge = (signed_distance_of_point_from_left_edge / signed_distance_of_right_vertex_from_left_edge);

        // COMPUTE THE BARYCENTRIC COORDINATE RELATIVE TO THE RIGHT EDGE.
        float signed_distance_of_point_from_right_edge = SignedDistanceOfPointFromEdge2D(center_vertex_position, right_vertex_position, point);
        float signed_distance_of_left_vertex_from_right_edge = SignedDistanceOfPointFromEdge2D(center_vertex_position, right_vertex_position, left_vertex_position);
        float scaled_signed_distance_of_point_from_right_edge = (signed_distance_of_point_from_right_edge / signed_distance_of_left_vertex_from_right_edge);

        // COMPUTE THE BARYCENTRIC COORDINATE FOR THE REMAINING EDGE.
        float scaled_signed_distance_of_point_from_opposite_edge = (
            1.0f -
            scaled_signed_distance_of_point_from_left_edge -
            scaled_signed_distance_of_point_from_right_edge);

        // RETURN THE FULL BARYCENTRIC COORDINATES FOR THE POINT.
        MATH::Vector3f point_barycentric_coordinates(
            scaled_signed_distance_of_point_from_opposite_edge,
            scaled_signed_distance_of_point_from_left_edge,
            scaled_signed_distance_of_point_from_right_edge);
        return point_barycentric_coordinates;
    }

    /// Computes the signed distance of a point from an edge.
    /// Note that this method only computes things on a 2D plane.
    /// @param[in]  edge_start_position - The start position of the edge.
    /// @param[in]  edge_end_position - The end position of the edge.
    /// @param[in]  point - The point for which to compute the signed distance.
    /// @return The signed distance of the point from an edge.
    float Triangle::SignedDistanceOfPointFromEdge2D(const MATH::Vector2f& edge_start_position, const MATH::Vector2f& edge_end_position, const MATH::Vector2f& point) const
    {
        // The equations below is simplified to final form for easier computation.
        // See books like "Fundamental of Computer Graphics" by Peter Shirley and Steve Marschner (3rd edition - section 2.7.1)
        // for more elaboration on these equations.
        float signed_distance_of_point_from_edge = (
            ((edge_start_position.Y - edge_end_position.Y) * point.X) +
            ((edge_end_position.X - edge_start_position.X) * point.Y) +
            (edge_start_position.X * edge_end_position.Y) -
            (edge_end_position.X * edge_start_position.Y));
        return signed_distance_of_point_from_edge;
    }

    /// Computes the barycentric coordinates of a point relative to the triangle, taking into account 3D space.
    /// Barycentric coordinates are effectively coordinates that indicate where a point is relative to the inside of a triangle:
    /// - A point inside (including edges) will have all 3 coordinates between [0,1].
    /// - Positive or negative values indicate where a point is relative to a particular edge.
    /// @param[in]  point - The point for which to compute barycentric coordinates.
    /// @return The point in barycentric coordinates relative to the triangle.
    MATH::Vector3f Triangle::BarycentricCoordinates3DOf(const MATH::Vector3f& point) const
    {
        // See books like "Fundamental of Computer Graphics" by Peter Shirley and Steve Marschner (3rd edition - section 2.7.2)
        // for more elaboration on these equations.

        // DEFINE THE VERTICES FOR THE BARYCENTRIC COORDINATE SYSTEM.
        // Triangle coordinates are in counterclockwise order.
        // The names of the vertices below may not be strictly "correct",
        // but the basic idea holds for each vertex relative to each other.
        //             left_vertex
        //                 /\
        //                /  \
        // center_vertex /____\ right_vertex
        const MATH::Vector3f& center_vertex_position = Vertices[1].Position;
        const MATH::Vector3f& left_vertex_position = Vertices[0].Position;
        const MATH::Vector3f& right_vertex_position = Vertices[2].Position;

        // COMPUTE THE SURFACE NORMAL FOR THE ENTIRE TRIANGLE.
        MATH::Vector3f first_edge = right_vertex_position - center_vertex_position;
        MATH::Vector3f second_edge = left_vertex_position - center_vertex_position;
        MATH::Vector3f surface_normal = MATH::Vector3f::CrossProduct(first_edge, second_edge);
        float surface_normal_length = surface_normal.Length();
        float surface_normal_squared_length = surface_normal_length * surface_normal_length;

        // COMPUTE NORMALS RELATIVE TO THE POINT.
        MATH::Vector3f edge_for_right_vertex = (left_vertex_position - right_vertex_position);
        MATH::Vector3f edge_for_point_and_right_vertex = (point - right_vertex_position);
        MATH::Vector3f point_and_right_vertex_normal = MATH::Vector3f::CrossProduct(edge_for_right_vertex, edge_for_point_and_right_vertex);

        MATH::Vector3f edge_for_left_vertex = (center_vertex_position - left_vertex_position);
        MATH::Vector3f edge_for_point_and_left_vertex = (point - left_vertex_position);
        MATH::Vector3f point_and_left_vertex_normal = MATH::Vector3f::CrossProduct(edge_for_left_vertex, edge_for_point_and_left_vertex);

        MATH::Vector3f edge_for_center_vertex = first_edge;
        MATH::Vector3f edge_for_point_and_center_vertex = (point - center_vertex_position);
        MATH::Vector3f point_and_center_vertex_normal = MATH::Vector3f::CrossProduct(edge_for_center_vertex, edge_for_point_and_center_vertex);

        // COMPUTE THE BARYCENTRIC COORDINATES.
        float first_barycentric_coordinate_dot_product = MATH::Vector3f::DotProduct(surface_normal, point_and_right_vertex_normal);
        float first_barycentric_coordinate = first_barycentric_coordinate_dot_product / surface_normal_squared_length;

        float second_barycentric_coordinate_dot_product = MATH::Vector3f::DotProduct(surface_normal, point_and_left_vertex_normal);
        float second_barycentric_coordinate = second_barycentric_coordinate_dot_product / surface_normal_squared_length;

        float third_barycentric_coordinate_dot_product = MATH::Vector3f::DotProduct(surface_normal, point_and_center_vertex_normal);
        float third_barycentric_coordinate = third_barycentric_coordinate_dot_product / surface_normal_squared_length;

        MATH::Vector3f point_barycentric_coordinates(
            first_barycentric_coordinate,
            second_barycentric_coordinate,
            third_barycentric_coordinate);
        return point_barycentric_coordinates;
    }
}
