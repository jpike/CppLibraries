#if _WIN32

// To avoid annoyances with Windows min/max #defines.
#define NOMINMAX

#include "Graphics/CpuRendering/CpuRasterizationAlgorithm.h"
#include "Graphics/Shading/Shading.h"
#include "Graphics/Viewing/ViewingTransformations.h"
#include "Math/Number.h"

namespace GRAPHICS::CPU_RENDERING
{
    /// Renders some text onto the render target.
    /// @param[in]  text - The text to render.
    /// @param[in,out]  render_target - The target to render to.
    void CpuRasterizationAlgorithm::Render(const GUI::Text& text, IMAGES::Bitmap& render_target)
    {
        // MAKE SURE A FONT EXISTS.
        if (!text.Font)
        {
            return;
        }

        // RENDER A GLYPH FOR EACH CHARACTER.
        unsigned int current_glyph_left_x_position = static_cast<unsigned int>(text.LeftTopPosition.X);
        unsigned int current_glyph_top_y_position = static_cast<unsigned int>(text.LeftTopPosition.Y);
        for (char character : text.String)
        {
            // RENDER ALL PIXELS FOR THE CURRENT GLYPH.
            const GUI::Glyph& glyph = text.Font->GlyphsByCharacter[static_cast<unsigned char>(character)];
            for (unsigned int glyph_local_pixel_y_position = 0; glyph_local_pixel_y_position < glyph.HeightInPixels; ++glyph_local_pixel_y_position)
            {
                for (unsigned int glyph_local_pixel_x_position = 0; glyph_local_pixel_x_position < glyph.WidthInPixels; ++glyph_local_pixel_x_position)
                {
                    // ONLY WRITE THE PIXEL IF IT IS VISIBLE.
                    /// @todo   Fancier alpha blending?
                    GRAPHICS::Color pixel_color = glyph.GetPixelColor(glyph_local_pixel_x_position, glyph_local_pixel_y_position);
                    bool pixel_visible = (pixel_color.Alpha > 0);
                    if (pixel_visible)
                    {
                        unsigned int glyph_destination_x_position = current_glyph_left_x_position + glyph_local_pixel_x_position;
                        unsigned int glyph_destination_y_position = current_glyph_top_y_position + glyph_local_pixel_y_position;
                        render_target.WritePixel(glyph_destination_x_position, glyph_destination_y_position, pixel_color);
                    }
                }
            }

            // MOVE TO THE NEXT GLYPH.
            current_glyph_left_x_position += glyph.WidthInPixels;
        }
    }

    /// Renders an entire 3D scene.
    /// @param[in]  scene - The scene to render.
    /// @param[in]  camera - The camera to use to view the scene.
    /// @param[in]  cull_backfaces - True if backfaces should be culled; false otherwise.
    /// @param[in,out]  output_bitmap - The bitmap to render to.
    /// @param[in,out]  depth_buffer - The depth buffer to use for any depth buffering.
    void CpuRasterizationAlgorithm::Render(
        const Scene& scene, 
        const VIEWING::Camera& camera,
        const bool cull_backfaces, 
        IMAGES::Bitmap& output_bitmap,
        DepthBuffer* depth_buffer)
    {
        // CLEAR THE BACKGROUND.
        output_bitmap.FillPixels(scene.BackgroundColor);
        if (depth_buffer)
        {
            depth_buffer->ClearToDepth(DepthBuffer::MAX_DEPTH);
        }

        // RENDER EACH OBJECT IN THE SCENE.
        for (const auto& object_3D : scene.Objects)
        {
            Render(object_3D, scene.PointLights, camera, cull_backfaces, output_bitmap, depth_buffer);
        }
    }

    /// Renders a 3D object to the render target.
    /// @param[in]  object_3D - The object to render.
    /// @param[in]  lights - Any lights that should illuminate the object.
    /// @param[in]  camera - The camera to use to view the object.
    /// @param[in,out]  output_bitmap - The bitmap to render to.
    /// @param[in,out]  depth_buffer - The depth buffer to use for any depth buffering.
    void CpuRasterizationAlgorithm::Render(
        const Object3D& object_3D, 
        const std::optional<std::vector<LIGHTING::Light>>& lights,
        const VIEWING::Camera& camera,
        const bool cull_backfaces, 
        IMAGES::Bitmap& output_bitmap,
        DepthBuffer* depth_buffer)
    {
        // GET RE-USED TRANSFORMATIONS.
        // This is done before the loop to avoid performance hits for repeatedly calculating these matrices.
        MATH::Matrix4x4f object_world_transform = object_3D.WorldTransform();
        VIEWING::ViewingTransformations viewing_transformations(camera, output_bitmap);

        // RENDER EACH MESH OF THE OBJECT.
        for (const auto& [mesh_name, mesh] : object_3D.Model.MeshesByName)
        {
            // SKIP OVER INVISIBLE MESHES.
            if (!mesh.Visible)
            {
                continue;
            }

            // RENDER EACH TRIANGLE OF THE MESH.
            for (const auto& local_triangle : mesh.Triangles)
            {
                // TRANSFORM THE TRIANGLE INTO WORLD SPACE.
                Triangle world_space_triangle = TransformLocalToWorld(local_triangle, object_world_transform);

                // CULL BACKFACES IF APPLICABLE.
                MATH::Vector3f unit_surface_normal = world_space_triangle.SurfaceNormal();
                if (cull_backfaces)
                {
                    // If the surface normal is facing opposite of the camera's view direction (negative dot product),
                    // then the surface normal should be facing the camera.
                    MATH::Vector3f view_direction = -camera.CoordinateFrame.Forward;
                    float surface_normal_camera_view_direction_dot_product = MATH::Vector3f::DotProduct(unit_surface_normal, view_direction);
                    bool triangle_facing_toward_camera = (surface_normal_camera_view_direction_dot_product < 0.0f);
                    if (!triangle_facing_toward_camera)
                    {
                        continue;
                    }
                }

                // TRANSFORM THE TRIANGLE FOR PROPER CAMERA VIEWING.
                std::optional<ScreenSpaceTriangle> screen_space_triangle = viewing_transformations.Apply(world_space_triangle);
                if (!screen_space_triangle)
                {
                    continue;
                }

                // COMPUTE VERTEX COLORS.
                for (std::size_t vertex_index = 0; vertex_index < Triangle::VERTEX_COUNT; ++vertex_index)
                {
                    // SHADE THE CURRENT VERTEX.
                    const VertexWithAttributes& current_world_vertex = world_space_triangle.Vertices[vertex_index];

                    Color final_vertex_color = SHADING::Shading::Compute(
                        current_world_vertex,
                        unit_surface_normal,
                        *screen_space_triangle->Material,
                        camera.WorldPosition,
                        lights);

                    screen_space_triangle->Vertices[vertex_index].Color = final_vertex_color;
                }

                // RENDER THE FINAL SCREEN SPACE TRIANGLE.
                Render(*screen_space_triangle, output_bitmap, depth_buffer);
            }
        }
    }

    /// Transforms a triangle from local coordinates to world coordinates.
    /// @param[in]  local_triangle - The local triangle to transform.
    /// @param[in]  world_transform - The world transformation for the triangle.
    /// @return The world space triangle.
    Triangle CpuRasterizationAlgorithm::TransformLocalToWorld(const Triangle& local_triangle, const MATH::Matrix4x4f& world_transform)
    {
        // TRANSFORM EACH VERTEX OF THE TRIANGLE.
        Triangle world_space_triangle = local_triangle;

        std::size_t triangle_vertex_count = world_space_triangle.Vertices.size();
        for (std::size_t vertex_index = 0; vertex_index < triangle_vertex_count; ++vertex_index)
        {
            // TRANFORM THE CURRENT LOCAL VERTEX INTO WORLD SPACE.
            const MATH::Vector3f& local_vertex = local_triangle.Vertices[vertex_index].Position;
            MATH::Vector4f local_homogeneous_vertex = MATH::Vector4f::HomogeneousPositionVector(local_vertex);

            MATH::Vector4f world_homogeneous_vertex = world_transform * local_homogeneous_vertex;
            VertexWithAttributes& world_vertex = world_space_triangle.Vertices[vertex_index];
            world_vertex.Position = MATH::Vector3f(world_homogeneous_vertex.X, world_homogeneous_vertex.Y, world_homogeneous_vertex.Z);
        }

        return world_space_triangle;
    }

    /// Renders a single triangle to the render target.
    /// @param[in]  triangle - The triangle to render.
    /// @param[in,out]  render_target - The target to render to.
    /// @param[in,out]  depth_buffer - The depth buffer to use for any depth buffering.
    void CpuRasterizationAlgorithm::Render(
        const ScreenSpaceTriangle& triangle, 
        IMAGES::Bitmap& render_target,
        DepthBuffer* depth_buffer)
    {
        // GET THE VERTICES.
        // They're needed for all kinds of shading.
        const VertexWithAttributes& first_vertex = triangle.Vertices[0];
        const VertexWithAttributes& second_vertex = triangle.Vertices[1];
        const VertexWithAttributes& third_vertex = triangle.Vertices[2];

        // RENDER THE TRIANGLE BASED ON SHADING TYPE.
        switch (triangle.Material->Shading)
        {
            case ShadingType::WIREFRAME:
            {
                // DRAW THE FIRST EDGE.
                DrawLineWithInterpolatedColor(
                    first_vertex,
                    second_vertex,
                    render_target,
                    depth_buffer);

                // DRAW THE SECOND EDGE.
                DrawLineWithInterpolatedColor(
                    second_vertex,
                    third_vertex,
                    render_target,
                    depth_buffer);

                // DRAW THE THIRD EDGE.
                DrawLineWithInterpolatedColor(
                    third_vertex,
                    first_vertex,
                    render_target,
                    depth_buffer);
                break;
            }
            case ShadingType::FLAT:
            {
                // COMPUTE THE BARYCENTRIC COORDINATES OF THE TRIANGLE VERTICES.
                float top_vertex_signed_distance_from_bottom_edge = (
                    ((second_vertex.Position.Y - third_vertex.Position.Y) * first_vertex.Position.X) +
                    ((third_vertex.Position.X - second_vertex.Position.X) * first_vertex.Position.Y) +
                    (second_vertex.Position.X * third_vertex.Position.Y) -
                    (third_vertex.Position.X * second_vertex.Position.Y));
                float right_vertex_signed_distance_from_left_edge = (
                    ((second_vertex.Position.Y - first_vertex.Position.Y) * third_vertex.Position.X) +
                    ((first_vertex.Position.X - second_vertex.Position.X) * third_vertex.Position.Y) +
                    (second_vertex.Position.X * first_vertex.Position.Y) -
                    (first_vertex.Position.X * second_vertex.Position.Y));

                // GET THE BOUNDING RECTANGLE OF THE TRIANGLE.
                /// @todo   Create rectangle class.
                float min_x = std::min({ first_vertex.Position.X, second_vertex.Position.X, third_vertex.Position.X });
                float max_x = std::max({ first_vertex.Position.X, second_vertex.Position.X, third_vertex.Position.X });
                float min_y = std::min({ first_vertex.Position.Y, second_vertex.Position.Y, third_vertex.Position.Y });
                float max_y = std::max({ first_vertex.Position.Y, second_vertex.Position.Y, third_vertex.Position.Y });

                // Endpoints are clamped to avoid trying to draw really huge lines off-screen.
                constexpr float MIN_BITMAP_COORDINATE = 1.0f;

                float max_x_position = static_cast<float>(render_target.GetWidthInPixels() - 1);
                float clamped_min_x = MATH::Number::Clamp<float>(min_x, MIN_BITMAP_COORDINATE, max_x_position);
                float clamped_max_x = MATH::Number::Clamp<float>(max_x, MIN_BITMAP_COORDINATE, max_x_position);

                float max_y_position = static_cast<float>(render_target.GetHeightInPixels() - 1);
                float clamped_min_y = MATH::Number::Clamp<float>(min_y, MIN_BITMAP_COORDINATE, max_y_position);
                float clamped_max_y = MATH::Number::Clamp<float>(max_y, MIN_BITMAP_COORDINATE, max_y_position);

                // COLOR PIXELS WITHIN THE TRIANGLE.
                constexpr float ONE_PIXEL = 1.0f;
                for (float y = clamped_min_y; y <= clamped_max_y; y += ONE_PIXEL)
                {
                    for (float x = clamped_min_x; x <= clamped_max_x; x += ONE_PIXEL)
                    {
                        // COMPUTE THE BARYCENTRIC COORDINATES OF THE CURRENT PIXEL POSITION.
                        // The following diagram shows the order of the vertices:
                        //             first_vertex
                        //                 /\
                        //                /  \
                        // second_vertex /____\ third_vertex
                        float current_pixel_signed_distance_from_bottom_edge = (
                            ((second_vertex.Position.Y - third_vertex.Position.Y) * x) +
                            ((third_vertex.Position.X - second_vertex.Position.X) * y) +
                            (second_vertex.Position.X * third_vertex.Position.Y) -
                            (third_vertex.Position.X * second_vertex.Position.Y));
                        float scaled_signed_distance_of_current_pixel_relative_to_bottom_edge = (current_pixel_signed_distance_from_bottom_edge / top_vertex_signed_distance_from_bottom_edge);

                        float current_pixel_signed_distance_from_left_edge = (
                            ((second_vertex.Position.Y - first_vertex.Position.Y) * x) +
                            ((first_vertex.Position.X - second_vertex.Position.X) * y) +
                            (second_vertex.Position.X * first_vertex.Position.Y) -
                            (first_vertex.Position.X * second_vertex.Position.Y));
                        float scaled_signed_distance_of_current_pixel_relative_to_left_edge = (current_pixel_signed_distance_from_left_edge / right_vertex_signed_distance_from_left_edge);

                        float scaled_signed_distance_of_current_pixel_relative_to_right_edge = (
                            1.0f -
                            scaled_signed_distance_of_current_pixel_relative_to_left_edge -
                            scaled_signed_distance_of_current_pixel_relative_to_bottom_edge);

                        // CHECK IF THE PIXEL IS WITHIN THE TRIANGLE.
                        // It's allowed to be on the borders too.
                        constexpr float MIN_SIGNED_DISTANCE_TO_BE_ON_EDGE = 0.0f;
                        constexpr float MAX_SIGNED_DISTANCE_TO_BE_ON_VERTEX = 1.0f;
                        bool pixel_between_bottom_edge_and_top_vertex = (
                            (MIN_SIGNED_DISTANCE_TO_BE_ON_EDGE <= scaled_signed_distance_of_current_pixel_relative_to_bottom_edge) &&
                            (scaled_signed_distance_of_current_pixel_relative_to_bottom_edge <= MAX_SIGNED_DISTANCE_TO_BE_ON_VERTEX));
                        bool pixel_between_left_edge_and_right_vertex = (
                            (MIN_SIGNED_DISTANCE_TO_BE_ON_EDGE <= scaled_signed_distance_of_current_pixel_relative_to_left_edge) &&
                            (scaled_signed_distance_of_current_pixel_relative_to_left_edge <= MAX_SIGNED_DISTANCE_TO_BE_ON_VERTEX));
                        bool pixel_between_right_edge_and_left_vertex = (
                            (MIN_SIGNED_DISTANCE_TO_BE_ON_EDGE <= scaled_signed_distance_of_current_pixel_relative_to_right_edge) &&
                            (scaled_signed_distance_of_current_pixel_relative_to_right_edge <= MAX_SIGNED_DISTANCE_TO_BE_ON_VERTEX));
                        bool pixel_in_triangle = (
                            pixel_between_bottom_edge_and_top_vertex &&
                            pixel_between_left_edge_and_right_vertex &&
                            pixel_between_right_edge_and_left_vertex);
                        if (pixel_in_triangle)
                        {
                            float interpolated_z = (
                                (scaled_signed_distance_of_current_pixel_relative_to_right_edge * third_vertex.Position.Z) +
                                (scaled_signed_distance_of_current_pixel_relative_to_left_edge * second_vertex.Position.Z) +
                                (scaled_signed_distance_of_current_pixel_relative_to_bottom_edge * first_vertex.Position.Z));

                            // Apply depth buffering filtering if applicable.
                            unsigned int current_pixel_x = static_cast<unsigned int>(std::round(x));
                            unsigned int current_pixel_y = static_cast<unsigned int>(std::round(y));
                            if (depth_buffer)
                            {
                                float current_pixel_depth = depth_buffer->GetDepth(current_pixel_x, current_pixel_y);
                                bool current_pixel_in_front_of_old_pixels = (interpolated_z <= current_pixel_depth);
                                if (!current_pixel_in_front_of_old_pixels)
                                {
                                    // Continue to the next iteration of the loop in
                                    // case there is another pixel to draw.
                                    continue;
                                }
                            }

                            // GET THE COLOR.
                            /// @todo   Assuming all vertices have the same color here.
                            Color face_color = triangle.Vertices[0].Color;

                            // DRAW THE COLORED PIXEL.
                            // The coordinates need to be rounded to integer in order
                            // to plot a pixel on a fixed grid.
                            render_target.WritePixel(
                                current_pixel_x,
                                current_pixel_y,
                                face_color);
                            if (depth_buffer)
                            {
                                depth_buffer->WriteDepth(current_pixel_x, current_pixel_y, interpolated_z);
                            }
                        }
                    }
                }
                break;
            }
            case ShadingType::FACE_VERTEX_COLOR_INTERPOLATION:
            case ShadingType::GOURAUD: /// @todo    This should be the same?
            case ShadingType::TEXTURED: /// @todo    This should be the same?
            case ShadingType::MATERIAL: /// @todo    This should be the same?
            {
                // COMPUTE THE BARYCENTRIC COORDINATES OF THE TRIANGLE VERTICES.
                float top_vertex_signed_distance_from_bottom_edge = (
                    ((second_vertex.Position.Y - third_vertex.Position.Y) * first_vertex.Position.X) +
                    ((third_vertex.Position.X - second_vertex.Position.X) * first_vertex.Position.Y) +
                    (second_vertex.Position.X * third_vertex.Position.Y) -
                    (third_vertex.Position.X * second_vertex.Position.Y));
                float right_vertex_signed_distance_from_left_edge = (
                    ((second_vertex.Position.Y - first_vertex.Position.Y) * third_vertex.Position.X) +
                    ((first_vertex.Position.X - second_vertex.Position.X) * third_vertex.Position.Y) +
                    (second_vertex.Position.X * first_vertex.Position.Y) -
                    (first_vertex.Position.X * second_vertex.Position.Y));

                // GET THE BOUNDING RECTANGLE OF THE TRIANGLE.
                /// @todo   Create rectangle class.
                float min_x = std::min({ first_vertex.Position.X, second_vertex.Position.X, third_vertex.Position.X });
                float max_x = std::max({ first_vertex.Position.X, second_vertex.Position.X, third_vertex.Position.X });
                float min_y = std::min({ first_vertex.Position.Y, second_vertex.Position.Y, third_vertex.Position.Y });
                float max_y = std::max({ first_vertex.Position.Y, second_vertex.Position.Y, third_vertex.Position.Y });

                // Endpoints are clamped to avoid trying to draw really huge lines off-screen.
                constexpr float MIN_BITMAP_COORDINATE = 1.0f;

                float max_x_position = static_cast<float>(render_target.GetWidthInPixels() - 1);
                float clamped_min_x = MATH::Number::Clamp<float>(min_x, MIN_BITMAP_COORDINATE, max_x_position);
                float clamped_max_x = MATH::Number::Clamp<float>(max_x, MIN_BITMAP_COORDINATE, max_x_position);

                float max_y_position = static_cast<float>(render_target.GetHeightInPixels() - 1);
                float clamped_min_y = MATH::Number::Clamp<float>(min_y, MIN_BITMAP_COORDINATE, max_y_position);
                float clamped_max_y = MATH::Number::Clamp<float>(max_y, MIN_BITMAP_COORDINATE, max_y_position);

                // COLOR PIXELS WITHIN THE TRIANGLE.
                constexpr float ONE_PIXEL = 1.0f;
                for (float y = clamped_min_y; y <= clamped_max_y; y += ONE_PIXEL)
                {
                    for (float x = clamped_min_x; x <= clamped_max_x; x += ONE_PIXEL)
                    {
                        // COMPUTE THE BARYCENTRIC COORDINATES OF THE CURRENT PIXEL POSITION.
                        // The following diagram shows the order of the vertices:
                        //             first_vertex
                        //                 /\
                        //                /  \
                        // second_vertex /____\ third_vertex
                        float current_pixel_signed_distance_from_bottom_edge = (
                            ((second_vertex.Position.Y - third_vertex.Position.Y) * x) +
                            ((third_vertex.Position.X - second_vertex.Position.X) * y) +
                            (second_vertex.Position.X * third_vertex.Position.Y) -
                            (third_vertex.Position.X * second_vertex.Position.Y));
                        float scaled_signed_distance_of_current_pixel_relative_to_bottom_edge = (current_pixel_signed_distance_from_bottom_edge / top_vertex_signed_distance_from_bottom_edge);

                        float current_pixel_signed_distance_from_left_edge = (
                            ((second_vertex.Position.Y - first_vertex.Position.Y) * x) +
                            ((first_vertex.Position.X - second_vertex.Position.X) * y) +
                            (second_vertex.Position.X * first_vertex.Position.Y) -
                            (first_vertex.Position.X * second_vertex.Position.Y));
                        float scaled_signed_distance_of_current_pixel_relative_to_left_edge = (current_pixel_signed_distance_from_left_edge / right_vertex_signed_distance_from_left_edge);

                        float scaled_signed_distance_of_current_pixel_relative_to_right_edge = (
                            1.0f -
                            scaled_signed_distance_of_current_pixel_relative_to_left_edge -
                            scaled_signed_distance_of_current_pixel_relative_to_bottom_edge);

                        // CHECK IF THE PIXEL IS WITHIN THE TRIANGLE.
                        float interpolated_z = DepthBuffer::MAX_DEPTH;
                        // It's allowed to be on the borders too.
                        constexpr float MIN_SIGNED_DISTANCE_TO_BE_ON_EDGE = 0.0f;
                        constexpr float MAX_SIGNED_DISTANCE_TO_BE_ON_VERTEX = 1.0f;
                        bool pixel_between_bottom_edge_and_top_vertex = (
                            (MIN_SIGNED_DISTANCE_TO_BE_ON_EDGE <= scaled_signed_distance_of_current_pixel_relative_to_bottom_edge) &&
                            (scaled_signed_distance_of_current_pixel_relative_to_bottom_edge <= MAX_SIGNED_DISTANCE_TO_BE_ON_VERTEX));
                        bool pixel_between_left_edge_and_right_vertex = (
                            (MIN_SIGNED_DISTANCE_TO_BE_ON_EDGE <= scaled_signed_distance_of_current_pixel_relative_to_left_edge) &&
                            (scaled_signed_distance_of_current_pixel_relative_to_left_edge <= MAX_SIGNED_DISTANCE_TO_BE_ON_VERTEX));
                        bool pixel_between_right_edge_and_left_vertex = (
                            (MIN_SIGNED_DISTANCE_TO_BE_ON_EDGE <= scaled_signed_distance_of_current_pixel_relative_to_right_edge) &&
                            (scaled_signed_distance_of_current_pixel_relative_to_right_edge <= MAX_SIGNED_DISTANCE_TO_BE_ON_VERTEX));
                        bool pixel_in_triangle = (
                            pixel_between_bottom_edge_and_top_vertex &&
                            pixel_between_left_edge_and_right_vertex &&
                            pixel_between_right_edge_and_left_vertex);
                        if (pixel_in_triangle)
                        {
                            // The color needs to be interpolated with this kind of shading.
                            Color interpolated_color = GRAPHICS::Color::BLACK;

                            const Color& first_vertex_color = triangle.Vertices[0].Color;
                            const Color& second_vertex_color = triangle.Vertices[1].Color;
                            const Color& third_vertex_color = triangle.Vertices[2].Color;
                            interpolated_color.Red = (
                                (scaled_signed_distance_of_current_pixel_relative_to_right_edge * third_vertex_color.Red) +
                                (scaled_signed_distance_of_current_pixel_relative_to_left_edge * second_vertex_color.Red) +
                                (scaled_signed_distance_of_current_pixel_relative_to_bottom_edge * first_vertex_color.Red));
                            interpolated_color.Green = (
                                (scaled_signed_distance_of_current_pixel_relative_to_right_edge * third_vertex_color.Green) +
                                (scaled_signed_distance_of_current_pixel_relative_to_left_edge * second_vertex_color.Green) +
                                (scaled_signed_distance_of_current_pixel_relative_to_bottom_edge * first_vertex_color.Green));
                            interpolated_color.Blue = (
                                (scaled_signed_distance_of_current_pixel_relative_to_right_edge * third_vertex_color.Blue) +
                                (scaled_signed_distance_of_current_pixel_relative_to_left_edge * second_vertex_color.Blue) +
                                (scaled_signed_distance_of_current_pixel_relative_to_bottom_edge * first_vertex_color.Blue));
                            interpolated_color.Clamp();

                            interpolated_z = (
                                (scaled_signed_distance_of_current_pixel_relative_to_right_edge * third_vertex.Position.Z) +
                                (scaled_signed_distance_of_current_pixel_relative_to_left_edge * second_vertex.Position.Z) +
                                (scaled_signed_distance_of_current_pixel_relative_to_bottom_edge * first_vertex.Position.Z));

                            if (ShadingType::TEXTURED == triangle.Material->Shading)
                            {
                                // INTERPOLATE THE TEXTURE COORDINATES.
                                const MATH::Vector2f& first_texture_coordinate = first_vertex.TextureCoordinates;
                                const MATH::Vector2f& second_texture_coordinate = second_vertex.TextureCoordinates;
                                const MATH::Vector2f& third_texture_coordinate = third_vertex.TextureCoordinates;

                                MATH::Vector2f interpolated_texture_coordinate;
                                interpolated_texture_coordinate.X = (
                                    (scaled_signed_distance_of_current_pixel_relative_to_right_edge * third_texture_coordinate.X) +
                                    (scaled_signed_distance_of_current_pixel_relative_to_left_edge * second_texture_coordinate.X) +
                                    (scaled_signed_distance_of_current_pixel_relative_to_bottom_edge * first_texture_coordinate.X));
                                interpolated_texture_coordinate.Y = (
                                    (scaled_signed_distance_of_current_pixel_relative_to_right_edge * third_texture_coordinate.Y) +
                                    (scaled_signed_distance_of_current_pixel_relative_to_left_edge * second_texture_coordinate.Y) +
                                    (scaled_signed_distance_of_current_pixel_relative_to_bottom_edge * first_texture_coordinate.Y));
                                // Clamping.
                                if (interpolated_texture_coordinate.X < 0.0f)
                                {
                                    interpolated_texture_coordinate.X = 0.0f;
                                }
                                else if (interpolated_texture_coordinate.X > 1.0f)
                                {
                                    interpolated_texture_coordinate.X = 1.0f;
                                }
                                if (interpolated_texture_coordinate.Y < 0.0f)
                                {
                                    interpolated_texture_coordinate.Y = 0.0f;
                                }
                                else if (interpolated_texture_coordinate.Y > 1.0f)
                                {
                                    interpolated_texture_coordinate.Y = 1.0f;
                                }

                                // LOOK UP THE TEXTURE COLOR AT THE COORDINATES.
                                unsigned int texture_width_in_pixels = triangle.Material->Texture->GetWidthInPixels();
                                unsigned int texture_pixel_x_coordinate = static_cast<unsigned int>(texture_width_in_pixels * interpolated_texture_coordinate.X);

                                unsigned int texture_height_in_pixels = triangle.Material->Texture->GetHeightInPixels();
                                unsigned int texture_pixel_y_coordinate = static_cast<unsigned int>(texture_height_in_pixels * interpolated_texture_coordinate.Y);

                                Color texture_color = triangle.Material->Texture->GetPixel(texture_pixel_x_coordinate, texture_pixel_y_coordinate);

                                interpolated_color = Color::ComponentMultiplyRedGreenBlue(interpolated_color, texture_color);
                                interpolated_color.Clamp();
                            }

                            // Apply depth buffering filtering if applicable.
                            unsigned int current_pixel_x = static_cast<unsigned int>(std::round(x));
                            unsigned int current_pixel_y = static_cast<unsigned int>(std::round(y));
                            if (depth_buffer)
                            {
                                float current_pixel_depth = depth_buffer->GetDepth(current_pixel_x, current_pixel_y);
                                bool current_pixel_in_front_of_old_pixels = (interpolated_z >= current_pixel_depth);
                                if (!current_pixel_in_front_of_old_pixels)
                                {
                                    // Continue to the next iteration of the loop in
                                    // case there is another pixel to draw.
                                    continue;
                                }
                            }

                            // The coordinates need to be rounded to integer in order
                            // to plot a pixel on a fixed grid.
                            render_target.WritePixel(
                                current_pixel_x,
                                current_pixel_y,
                                interpolated_color);
                            if (depth_buffer)
                            {
                                depth_buffer->WriteDepth(current_pixel_x, current_pixel_y, interpolated_z);
                            }
                        }
                    }
                }
                break;
            }
        }
    }

    /// Renders a line with the specified endpoints (in screen coordinates).
    /// @param[in]  start_vertex - The starting coordinate of the line.
    /// @param[in]  end_vertex - The ending coordinate of the line.
    /// @param[in]  color - The color of the line to draw.
    /// @param[in,out]  render_target - The target to render to.
    /// @param[in,out]  depth_buffer - The depth buffer to use for any depth buffering.
    void CpuRasterizationAlgorithm::DrawLine(
        const MATH::Vector3f& start_vertex,
        const MATH::Vector3f& end_vertex,
        const Color& color,
        IMAGES::Bitmap& render_target,
        DepthBuffer* depth_buffer)
    {
        // EXTRACT COMPONENTS OF THE VERTEX.
        float start_x = start_vertex.X;
        float start_y = start_vertex.Y;
        float start_z = start_vertex.Z;
        float end_x = end_vertex.X;
        float end_y = end_vertex.Y;
        float end_z = end_vertex.Z;

        // CLAMP ENDPOINTS TO AVOID TRYING TO DRAW REALLY HUGE LINES OFF-SCREEN.
        constexpr float MIN_BITMAP_COORDINATE = 1.0f;

        float max_x_position = static_cast<float>(render_target.GetWidthInPixels() - 1);
        float clamped_start_x = MATH::Number::Clamp<float>(start_x, MIN_BITMAP_COORDINATE, max_x_position);
        float clamped_end_x = MATH::Number::Clamp<float>(end_x, MIN_BITMAP_COORDINATE, max_x_position);

        float max_y_position = static_cast<float>(render_target.GetHeightInPixels() - 1);
        float clamped_start_y = MATH::Number::Clamp<float>(start_y, MIN_BITMAP_COORDINATE, max_y_position);
        float clamped_end_y = MATH::Number::Clamp<float>(end_y, MIN_BITMAP_COORDINATE, max_y_position);

        // COMPUTE THE INCREMENTS ALONG EACH AXIS FOR EACH PIXEL.
        // Each time we draw a pixel, we need to move slightly further along the axes.
        float delta_x = clamped_end_x - clamped_start_x;
        float delta_y = clamped_end_y - clamped_start_y;
        float length = std::max(std::abs(delta_x), std::abs(delta_y));
        float x_increment = delta_x / length;
        float y_increment = delta_y / length;

        float delta_z = end_z - start_z;
        float z_increment = delta_z / length;

        // HAVE THE LINE START BEING DRAWN AT THE STARTING COORDINATES.
        float x = clamped_start_x;
        float y = clamped_start_y;
        float z = start_z;

        // DRAW PIXELS FOR THE LINE.
        for (float pixel_index = 0.0f; pixel_index <= length; ++pixel_index, x += x_increment, y += y_increment, z += z_increment)
        {
            // PREVENT WRITING BEYOND THE BOUNDARIES OF THE RENDER TARGET.
            bool x_boundary_exceeded = (
                (x < 0.0f) ||
                (x > max_x_position));
            bool y_boundary_exceeded = (
                (y < 0.0f) ||
                (y > max_y_position));
            bool boundary_exceeded = (x_boundary_exceeded || y_boundary_exceeded);
            if (boundary_exceeded)
            {
                // Continue to the next iteration of the loop in
                // case there is another pixel to draw.
                continue;
            }

            // DETERMINE IF THE NEW Z IS IN FRONT.
            unsigned int current_pixel_x = static_cast<unsigned int>(std::round(x));
            unsigned int current_pixel_y = static_cast<unsigned int>(std::round(y));
            if (depth_buffer)
            {
                float current_pixel_depth = depth_buffer->GetDepth(current_pixel_x, current_pixel_y);
                bool current_pixel_in_front_of_old_pixels = (z >= current_pixel_depth);
                if (!current_pixel_in_front_of_old_pixels)
                {
                    // Continue to the next iteration of the loop in
                    // case there is another pixel to draw.
                    continue;
                }
            }
            
            // DRAW A PIXEL AT THE CURRENT POSITION.
            // The coordinates need to be rounded to integer in order
            // to plot a pixel on a fixed grid.
            render_target.WritePixel(
                current_pixel_x,
                current_pixel_y,
                color);
            if (depth_buffer)
            {
                depth_buffer->WriteDepth(current_pixel_x, current_pixel_y, z);
            }
        }
    }

    /// Renders a line with the specified endpoints (in screen coordinates) and interpolated color from the vertices.
    /// @param[in]  start_vertex - The starting vertex of the line.
    /// @param[in]  end_vertex - The ending vertex of the line.
    /// @param[in,out]  render_target - The target to render to.
    /// @param[in,out]  depth_buffer - The depth buffer to use for any depth buffering.
    void CpuRasterizationAlgorithm::DrawLineWithInterpolatedColor(
        const VertexWithAttributes& start_vertex,
        const VertexWithAttributes& end_vertex,
        IMAGES::Bitmap& render_target,
        DepthBuffer* depth_buffer)
    {
        // EXTRACT COMPONENTS OF THE VERTEX.
        float start_x = start_vertex.Position.X;
        float start_y = start_vertex.Position.Y;
        float start_z = start_vertex.Position.Z;
        float end_x = end_vertex.Position.X;
        float end_y = end_vertex.Position.Y;
        float end_z = end_vertex.Position.Z;

        // CLAMP ENDPOINTS TO AVOID TRYING TO DRAW REALLY HUGE LINES OFF-SCREEN.
        constexpr float MIN_BITMAP_COORDINATE = 1.0f;

        float max_x_position = static_cast<float>(render_target.GetWidthInPixels() - 1);
        float clamped_start_x = MATH::Number::Clamp<float>(start_x, MIN_BITMAP_COORDINATE, max_x_position);
        float clamped_end_x = MATH::Number::Clamp<float>(end_x, MIN_BITMAP_COORDINATE, max_x_position);

        float max_y_position = static_cast<float>(render_target.GetHeightInPixels() - 1);
        float clamped_start_y = MATH::Number::Clamp<float>(start_y, MIN_BITMAP_COORDINATE, max_y_position);
        float clamped_end_y = MATH::Number::Clamp<float>(end_y, MIN_BITMAP_COORDINATE, max_y_position);

        // COMPUTE THE LENGTH OF THE ENTIRE LINE.
        MATH::Vector2f vector_from_start_to_end(clamped_end_x - clamped_start_x, clamped_end_y - clamped_start_y);
        float line_length = vector_from_start_to_end.Length();

        // COMPUTE THE INCREMENTS ALONG EACH AXIS FOR EACH PIXEL.
        // Each time we draw a pixel, we need to move slightly
        // further along the axes.
        float delta_x = clamped_end_x - clamped_start_x;
        float delta_y = clamped_end_y - clamped_start_y;
        float length = std::max(std::abs(delta_x), std::abs(delta_y));
        float x_increment = delta_x / length;
        float y_increment = delta_y / length;

        float delta_z = end_z - start_z;
        float z_increment = delta_z / length;

        // HAVE THE LINE START BEING DRAWN AT THE STARTING COORDINATES.
        float x = clamped_start_x;
        float y = clamped_start_y;
        float z = start_z;

        // DRAW PIXELS FOR THE LINE.
        for (float pixel_index = 0.0f; pixel_index <= length; ++pixel_index, x += x_increment, y += y_increment, z += z_increment)
        {
            // PREVENT WRITING BEYOND THE BOUNDARIES OF THE RENDER TARGET.
            bool x_boundary_exceeded = (
                (x < 0.0f) ||
                (x > max_x_position));
            bool y_boundary_exceeded = (
                (y < 0.0f) ||
                (y > max_y_position));
            bool boundary_exceeded = (x_boundary_exceeded || y_boundary_exceeded);
            if (boundary_exceeded)
            {
                // Continue to the next iteration of the loop in
                // case there is another pixel to draw.
                continue;
            }

            // DETERMINE IF THE NEW Z IS IN FRONT.
            unsigned int current_pixel_x = static_cast<unsigned int>(std::round(x));
            unsigned int current_pixel_y = static_cast<unsigned int>(std::round(y));
            if (depth_buffer)
            {
                float current_pixel_depth = depth_buffer->GetDepth(current_pixel_x, current_pixel_y);
                bool current_pixel_in_front_of_old_pixels = (z >= current_pixel_depth);
                if (!current_pixel_in_front_of_old_pixels)
                {
                    // Continue to the next iteration of the loop in
                    // case there is another pixel to draw.
                    continue;
                }
            }

            // CALCULATE THE COLOR AT THE CURRENT POINT.
            MATH::Vector2f vector_to_current_pixel(x - start_x, y - start_y);
            float length_to_current_pixel_from_line_start = vector_to_current_pixel.Length();
            float ratio_toward_end_of_line = (length_to_current_pixel_from_line_start / line_length);
            Color interpolated_color = Color::InterpolateRedGreenBlue(start_vertex.Color, end_vertex.Color, ratio_toward_end_of_line);

            // DRAW A PIXEL AT THE CURRENT POSITION.
            // The coordinates need to be rounded to integer in order
            // to plot a pixel on a fixed grid.
            render_target.WritePixel(
                current_pixel_x,
                current_pixel_y,
                interpolated_color);
            if (depth_buffer)
            {
                depth_buffer->WriteDepth(current_pixel_x, current_pixel_y, z);
            }
        }
    }
}

#endif
