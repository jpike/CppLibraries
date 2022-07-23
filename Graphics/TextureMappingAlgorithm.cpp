#include "Graphics/TextureMappingAlgorithm.h"
#include "Math/Number.h"

namespace GRAPHICS
{
    /// Attempts to lookup a texel color from the texture at the given point on a triangle.
    /// @param[in]  triangle - The triangle for which to lookup a texel.
    /// @param[in]  triangle_point - The point potentially on the surface of the triangle for which to lookup the texel.
    ///     Assumed to be on the surface of the triangle.  If not, then the value will be clamped.
    /// @param[in]  texture - The texture in which to lookup the texel color.
    /// @return The color from the texture at the given surface point on the triangle.
    ///     If the point is not on the triangle, the point will be clamped to an appropriate edge for color lookup.
    Color TextureMappingAlgorithm::LookupTexel(
        const GEOMETRY::Triangle& triangle,
        const MATH::Vector2f& triangle_point,
        const IMAGES::Bitmap& texture)
    {
        // EXTRACT THE TEXTURE COORDINATES FROM THE TRIANGLE.
        VertexWithAttributes first_vertex = triangle.Vertices[0];
        VertexWithAttributes second_vertex = triangle.Vertices[1];
        VertexWithAttributes third_vertex = triangle.Vertices[2];

        const MATH::Vector2f& first_texture_coordinate = first_vertex.TextureCoordinates;
        const MATH::Vector2f& second_texture_coordinate = second_vertex.TextureCoordinates;
        const MATH::Vector2f& third_texture_coordinate = third_vertex.TextureCoordinates;

        // COMPUTE THE LOCATION OF THE POINT WITHIN THE TRIANGLE.
        MATH::Vector3f current_point_barycentric_coordinates = triangle.BarycentricCoordinates2DOf(triangle_point);

        // INTERPOLATE THE TEXTURE COORDINATES ACROSS THE TRIANGLE.
        MATH::Vector2f interpolated_texture_coordinate;
        interpolated_texture_coordinate.X = (
            (current_point_barycentric_coordinates.X * second_texture_coordinate.X) +
            (current_point_barycentric_coordinates.Y * third_texture_coordinate.X) +
            (current_point_barycentric_coordinates.Z * first_texture_coordinate.X));
        interpolated_texture_coordinate.Y = (
            (current_point_barycentric_coordinates.X * second_texture_coordinate.Y) +
            (current_point_barycentric_coordinates.Y * third_texture_coordinate.Y) +
            (current_point_barycentric_coordinates.Z * first_texture_coordinate.Y));

        // CLAMP THE TEXTURE COORDINATES TO THE VALID RANGE.
        constexpr float MIN_TEXTURE_COORDINATE = 0.0f;
        constexpr float MAX_TEXTURE_COORDINATE = 1.0f;
        interpolated_texture_coordinate.X = MATH::Number::Clamp<float>(interpolated_texture_coordinate.X, MIN_TEXTURE_COORDINATE, MAX_TEXTURE_COORDINATE);
        interpolated_texture_coordinate.Y = MATH::Number::Clamp<float>(interpolated_texture_coordinate.Y, MIN_TEXTURE_COORDINATE, MAX_TEXTURE_COORDINATE);

        // LOOK UP THE TEXTURE COLOR AT THE COORDINATES.
        unsigned int texture_width_in_pixels = texture.GetWidthInPixels();
        unsigned int max_texture_pixel_x_coordinate = texture_width_in_pixels - 1;
        unsigned int texture_pixel_x_coordinate = static_cast<unsigned int>(max_texture_pixel_x_coordinate * interpolated_texture_coordinate.X);

        unsigned int texture_height_in_pixels = texture.GetHeightInPixels();
        unsigned int max_texture_pixel_y_coordinate = texture_height_in_pixels - 1;
        unsigned int texture_pixel_y_coordinate = static_cast<unsigned int>(max_texture_pixel_y_coordinate * interpolated_texture_coordinate.Y);

        Color texel_color = texture.GetPixel(texture_pixel_x_coordinate, texture_pixel_y_coordinate);
        return texel_color;
    }
}
