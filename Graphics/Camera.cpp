#include <cmath>
#include "Graphics/Camera.h"

namespace GRAPHICS
{
    /// Creates an orthographic projection matrix.
    /// @param[in]  left_x_world_boundary - The left (x) world boundary of the
    ///     orthographic view volume.
    /// @param[in]  right_x_world_boundary - The right (x) world boundary of the
    ///     orthographic view volume.
    /// @param[in]  bottom_y_world_boundary - The bottom (y) world boundary of the
    ///     orthographic view volume.
    /// @param[in]  top_y_world_boundary - The top (y) world boundary of the
    ///     orthographic view volume.
    /// @param[in]  near_z_world_boundary - The near (z) world boundary of the
    ///     orthographic view volume.
    /// @param[in]  far_z_world_boundary - The far (z) world boundary of the
    ///     orthographic view volume.
    /// @return The specified orthographic projection matrix.
    MATH::Matrix4x4f Camera::OrthographicProjection(
        const float left_x_world_boundary,
        const float right_x_world_boundary,
        const float bottom_y_world_boundary,
        const float top_y_world_boundary,
        const float near_z_world_boundary,
        const float far_z_world_boundary)
    {
        // CREATE A MATRIX TO TRANSLATE THE ORTHOGRAPHIC VIEW VOLUME TO THE ORIGIN.
        MATH::Vector3f translation_vector;

        float orthographic_view_volume_x_midpoint = (left_x_world_boundary + right_x_world_boundary) / 2.0f;
        translation_vector.X = -orthographic_view_volume_x_midpoint;

        float orthographic_view_volume_y_midpoint = (bottom_y_world_boundary + top_y_world_boundary) / 2.0f;
        translation_vector.Y = -orthographic_view_volume_y_midpoint;

        float orthographic_view_volume_z_midpoint = (near_z_world_boundary + far_z_world_boundary) / 2.0f;
        translation_vector.Z = -orthographic_view_volume_z_midpoint;

        MATH::Matrix4x4f translate_view_volume_matrix = MATH::Matrix4x4f::Translation(translation_vector);

        // CREATE A MATRIX TO SCALE THE ORTHOGRAPHICS VIEW VOLUME TO THE CANONICAL VIEW VOLUME.
        const float CANONICAL_VIEW_VOLUME_DIMENSION = 2.0f;
        MATH::Vector3f scale_vector;

        float orthographic_view_volume_width = right_x_world_boundary - left_x_world_boundary;
        scale_vector.X = CANONICAL_VIEW_VOLUME_DIMENSION / orthographic_view_volume_width;

        float orthographic_view_volume_height = top_y_world_boundary - bottom_y_world_boundary;
        scale_vector.Y = CANONICAL_VIEW_VOLUME_DIMENSION / orthographic_view_volume_height;

        float orthographic_view_volume_depth = near_z_world_boundary - far_z_world_boundary;
        scale_vector.Z = CANONICAL_VIEW_VOLUME_DIMENSION / orthographic_view_volume_depth;

        MATH::Matrix4x4f scale_view_volume_matrix = MATH::Matrix4x4f::Scale(scale_vector);

        // CREATE THE ORTHOGRAPHIC PROJECTION MATRIX.
        MATH::Matrix4x4f orthographic_projection_matrix = scale_view_volume_matrix * translate_view_volume_matrix;
        return orthographic_projection_matrix;
    }

    /// Creates a perspective projection matrix.
    /// @param[in]  vertical_field_of_view_in_degrees - The vertical field of view, in degrees.
    /// @param[in]  aspect_ratio_width_over_height - The aspect ratio (width over height) of the view.
    /// @param[in]  near_z_world_boundary - The near (z) world boundary of the
    ///     view frustum.
    /// @param[in]  far_z_world_boundary - The far (z) world boundary of the
    ///     view frustum.
    /// @return The specified perspective projection matrix.
    MATH::Matrix4x4f Camera::PerspectiveProjection(
        const MATH::Angle<float>::Degrees vertical_field_of_view_in_degrees,
        const float aspect_ratio_width_over_height,
        const float near_z_world_boundary,
        const float far_z_world_boundary)
    {
        // CREATE THE BASIC PERSPECTIVE MATRIX.
        MATH::Matrix4x4f perspective_matrix;
        // Multiples the x/y coordinates by the near z world boundary so that the x/y coordinates
        // can be properly scaled relative to the near plane and the corresponding z coordinate.
        perspective_matrix.Elements(0, 0) = near_z_world_boundary;
        perspective_matrix.Elements(1, 1) = near_z_world_boundary;
        // Ensures that points on the near and far z planes are left alone in terms of the z coordinate.
        perspective_matrix.Elements(2, 2) = near_z_world_boundary + far_z_world_boundary;
        perspective_matrix.Elements(3, 2) = -far_z_world_boundary * near_z_world_boundary;
        // Helps preserve the z coordinate.
        perspective_matrix.Elements(2, 3) = 1.0f;

        // CREATE THE ORTHOGRAPHIC MATRIX.
        // The tangent function requires the field of view in radians.
        MATH::Angle<float>::Radians vertical_field_of_view_in_radians = MATH::Angle<float>::DegreesToRadians(vertical_field_of_view_in_degrees);

        // Half of the field of view defines how "high" the view frustum's near plane should be.
        float half_vertical_field_of_view_in_radians = vertical_field_of_view_in_radians.Value / 2.0f;
        float half_vertical_field_of_view_tangent = tan(half_vertical_field_of_view_in_radians);

        // An orthographic projection can handle projecting to the near plane of the frustum.
        float near_z_distance = abs(near_z_world_boundary);
        float top_y_world_boundary = half_vertical_field_of_view_tangent * near_z_distance;
        float bottom_y_world_boundary = -top_y_world_boundary;
        float right_x_world_boundary = aspect_ratio_width_over_height * top_y_world_boundary;
        float left_x_world_boundary = -right_x_world_boundary;

        MATH::Matrix4x4f orthographic_matrix = OrthographicProjection(
            left_x_world_boundary,
            right_x_world_boundary,
            bottom_y_world_boundary,
            top_y_world_boundary,
            near_z_world_boundary,
            far_z_world_boundary);

        // COMPUTE THE FULL PERSPECTIVE PROJECTION MATRIX.
        MATH::Matrix4x4f perspective_projection_matrix = orthographic_matrix * perspective_matrix;

        return perspective_projection_matrix;
    }

    /// Creates a camera looking at the specified world position.
    /// It is assumed that this look at position doesn't match the camera's position.
    /// If that isn't true, this may not work.
    /// @param[in]  look_at_world_position - The world position the camera should look at.
    /// @return A camera looking at the specified position.
    Camera Camera::LookAt(const MATH::Vector3f& look_at_world_position)
    {
        // CREATE A CAMERA LOOKING FROM ITS DEFAULT WORLD POSITION.
        Camera camera;
        camera = Camera::LookAtFrom(look_at_world_position, camera.WorldPosition);
        return camera;
    }

    /// Creates a camera looking at the specified world position from another specified world position.
    /// It is assumed that this look at position doesn't match the camera's position.
    /// If that isn't true, this may not work.
    /// @param[in]  camera_world_position - The world position the camera should be placed at.
    /// @param[in]  look_at_world_position - The world position the camera should look at.
    /// @return A camera looking at the specified position from another specified position.
    Camera Camera::LookAtFrom(const MATH::Vector3f& look_at_world_position, const MATH::Vector3f& camera_world_position)
    {
        // CREATE A CAMERA AT THE SPECIFIED WORLD POSITION.
        Camera camera;
        camera.WorldPosition = camera_world_position;

        // COMPUTE THE VIEWING DIRECTION FOR THE CAMERA.
        MATH::Vector3f camera_view_direction = look_at_world_position - camera.WorldPosition;
        MATH::Vector3f normalized_camera_view_direction = MATH::Vector3f::Normalize(camera_view_direction);

        // COMPUTE THE COORDINATE FRAME FOR THE CAMERA.
        MATH::Vector3f negative_camera_view_direction = MATH::Vector3f::Scale(-1.0f, normalized_camera_view_direction);
        camera.CoordinateFrame = MATH::CoordinateFrame::FromUpAndForward(camera.CoordinateFrame.Up, negative_camera_view_direction);

        return camera;
    }

    /// Computes the view transformation of the camera to transform
    /// world space coordinates to camera space coordinates.
    /// @return The view transformation matrix for the camera.
    MATH::Matrix4x4f Camera::ViewTransform() const
    {
        // CALCULATE THE ORTHONORMAL BASIS FOR THE CAMERA'S COORDINATE SYSTEM.
        MATH::Vector3f view_direction = -CoordinateFrame.Forward;
        MATH::Vector3f negative_view_direction_basis_vector = -MATH::Vector3f::Normalize(view_direction);

        MATH::Vector3f unnormalized_sideways_vector = MATH::Vector3f::CrossProduct(CoordinateFrame.Up, negative_view_direction_basis_vector);
        MATH::Vector3f sideways_basis_vector = MATH::Vector3f::Normalize(unnormalized_sideways_vector);

        MATH::Vector3f near_up_direction_basis_vector = MATH::Vector3f::CrossProduct(negative_view_direction_basis_vector, sideways_basis_vector);

        // CREATE A MATRIX FOR TRANSLATING TO CAMERA TO THE WORLD ORIGIN.
        MATH::Matrix4x4f translate_camera_to_origin_matrix = MATH::Matrix4x4f::Translation(-WorldPosition);

        // CREATE A MATRIX FOR ALIGNING THE THE CAMERA'S COORDINATE SYSTEM WITH THE WORLD COORDINATE SYSTEM.
        MATH::Matrix4x4f align_camera_to_world_matrix = MATH::Matrix4x4f::Identity();
        align_camera_to_world_matrix.SetRow(0, sideways_basis_vector);
        align_camera_to_world_matrix.SetRow(1, near_up_direction_basis_vector);
        align_camera_to_world_matrix.SetRow(2, negative_view_direction_basis_vector);

        // FORM THE FINAL VIEW TRANSFORM MATRIX.
        MATH::Matrix4x4f view_transform = align_camera_to_world_matrix * translate_camera_to_origin_matrix;
        return view_transform;
    }

    /// Computes the projection transformation for the camera.
    /// @return The projection transform for the camera.
    MATH::Matrix4x4f Camera::ProjectionTransform() const
    {
        // CREATE THE ORTHOGRAPHIC MATRIX.
        // This is needed regardless of the type of projection transform.

        // Since the camera looks down the negative Z axis, the distances should be negated
        // to form the viewing boundaries.
        float near_z_world_boundary = -NearClipPlaneViewDistance;
        float far_z_world_boundary = -FarClipPlaneViewDistance;

        // The tangent function requires the field of view in radians.
        MATH::Angle<float>::Radians field_of_view_in_radians = MATH::Angle<float>::DegreesToRadians(FieldOfView);
        // Half of the field of view defines how "high" the view frustum's near plane should be.
        float half_field_of_view_in_radians = field_of_view_in_radians.Value / 2.0f;
        float half_field_of_view_tangent = tan(half_field_of_view_in_radians);

        // An orthographic projection can handle projecting to the near plane of the frustum.
        float near_z_distance = abs(near_z_world_boundary);
        float top_y_world_boundary = half_field_of_view_tangent * near_z_distance;
        float bottom_y_world_boundary = -top_y_world_boundary;
        float right_x_world_boundary = top_y_world_boundary;
        float left_x_world_boundary = -right_x_world_boundary;

        MATH::Matrix4x4f orthographic_matrix = OrthographicProjection(
            left_x_world_boundary,
            right_x_world_boundary,
            bottom_y_world_boundary,
            top_y_world_boundary,
            near_z_world_boundary,
            far_z_world_boundary);

        // RETURN THE PROPER PROJECTION MATRIX.
        bool is_perspective = (ProjectionType::PERSPECTIVE == Projection);
        if (is_perspective)
        {
            // CREATE THE BASIC PERSPECTIVE MATRIX.
            MATH::Matrix4x4f perspective_matrix;
            // Multiples the x/y coordinates by the near z world boundary so that the x/y coordinates
            // can be properly scaled relative to the near plane and the corresponding z coordinate.
            perspective_matrix.Elements(0, 0) = near_z_world_boundary;
            perspective_matrix.Elements(1, 1) = near_z_world_boundary;
            // Ensures that points on the near and far z planes are left alone in terms of the z coordinate.
            perspective_matrix.Elements(2, 2) = near_z_world_boundary + far_z_world_boundary;
            perspective_matrix.Elements(3, 2) = -far_z_world_boundary * near_z_world_boundary;
            // Helps preserve the z coordinate.
            perspective_matrix.Elements(2, 3) = 1.0f;
            // COMPUTE THE FULL PERSPECTIVE PROJECTION MATRIX.
            MATH::Matrix4x4f perspective_projection_matrix = orthographic_matrix * perspective_matrix;
            return perspective_projection_matrix;
        }
        else
        {
            // JUST USE THE ORTHOGRAPHIC MATRIX.
            return orthographic_matrix;
        }
    }
}
