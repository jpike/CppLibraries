#if _WIN32

// To avoid annoyances with Windows min/max #defines.
#define NOMINMAX

#include "Debugging/Timer.h"
#include "Graphics/CpuRendering/CpuRasterizationAlgorithm.h"
#include "Graphics/Geometry/TriangleSimd8x.h"
#include "Graphics/Shading/WorldSpaceShading.h"
#include "Graphics/TextureMappingAlgorithm.h"
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
    /// @param[in]  camera - The camera through which the scene is being viewed.
    /// @param[in]  rendering_settings - The settings to use for rendering.
    /// @param[in,out]  output_bitmap - The bitmap to render to.
    /// @param[in,out]  depth_buffer - The depth buffer to use for any depth buffering.
    void CpuRasterizationAlgorithm::Render(
        const Scene& scene, 
        const VIEWING::Camera& camera,
        const GRAPHICS::RenderingSettings& rendering_settings,
        IMAGES::Bitmap& output_bitmap,
        DepthBuffer* depth_buffer)
    {
        /// @todo DEBUGGING::HighResolutionTimer timer("CpuRasterizationAlgorithm::Render(scene)");

        // CLEAR THE BACKGROUND.
        output_bitmap.FillPixels(scene.BackgroundColor);
        if (depth_buffer)
        {
            /// @todo DEBUGGING::HighResolutionTimer timer("CpuRasterizationAlgorithm::Render(scene) - ClearToDepth");
            depth_buffer->ClearToDepth(DepthBuffer::MAX_DEPTH);
        }

        // RENDER EACH OBJECT IN THE SCENE.
        for (const auto& object_3D : scene.Objects)
        {
            /// @todo DEBUGGING::HighResolutionTimer timer("CpuRasterizationAlgorithm::Render(scene) - Render object");
            Render(object_3D, scene.Lights, camera, rendering_settings, output_bitmap, depth_buffer);
        }
    }

    /// Renders a 3D object to the render target.
    /// @param[in]  object_3D - The object to render.
    /// @param[in]  lights - Any lights that should illuminate the object.
    /// @param[in]  camera - The camera through which the object is being viewed.
    /// @param[in]  rendering_settings - The settings to use for rendering.
    /// @param[in,out]  output_bitmap - The bitmap to render to.
    /// @param[in,out]  depth_buffer - The depth buffer to use for any depth buffering.
    void CpuRasterizationAlgorithm::Render(
        const Object3D& object_3D, 
        const std::vector<SHADING::LIGHTING::Light>& lights,
        const VIEWING::Camera& camera,
        const RenderingSettings& rendering_settings,
        IMAGES::Bitmap& output_bitmap,
        DepthBuffer* depth_buffer)
    {
        /// @todo DEBUGGING::HighResolutionTimer timer("CpuRasterizationAlgorithm::Render(object_3D)");

        // GET RE-USED TRANSFORMATIONS.
        // This is done before the loop to avoid performance hits for repeatedly calculating these matrices.
        MATH::Matrix4x4f object_world_transform = object_3D.WorldTransform();
        VIEWING::ViewingTransformations viewing_transformations(camera, output_bitmap);

        // RENDER EACH MESH OF THE OBJECT.
        for (const auto& [mesh_name, mesh] : object_3D.Model.MeshesByName)
        {
            /// @todo DEBUGGING::HighResolutionTimer timer("CpuRasterizationAlgorithm::Render(object_3D) - Mesh loop");

            // SKIP OVER INVISIBLE MESHES.
            if (!mesh.Visible)
            {
                continue;
            }

            // RENDER EACH TRIANGLE OF THE MESH.
            for (const auto& local_triangle : mesh.Triangles)
            {
                /// @todo DEBUGGING::HighResolutionTimer timer("CpuRasterizationAlgorithm::Render(object_3D) - Triangle loop");

                // TRANSFORM THE TRIANGLE INTO WORLD SPACE.
                GEOMETRY::Triangle world_space_triangle = TransformLocalToWorld(local_triangle, object_world_transform);

                // CULL BACKFACES IF APPLICABLE.
                MATH::Vector3f unit_surface_normal = world_space_triangle.SurfaceNormal();
                if (rendering_settings.CullBackfaces)
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
                std::optional<GEOMETRY::Triangle> screen_space_triangle = viewing_transformations.Apply(world_space_triangle);
                if (!screen_space_triangle)
                {
                    continue;
                }

                // COMPUTE VERTEX COLORS.
                for (std::size_t vertex_index = 0; vertex_index < GEOMETRY::Triangle::VERTEX_COUNT; ++vertex_index)
                {
                    /// @todo DEBUGGING::HighResolutionTimer timer("CpuRasterizationAlgorithm::Render(object_3D) - Vertex loop");

                    // SHADE THE CURRENT VERTEX.
                    const VertexWithAttributes& current_world_vertex = world_space_triangle.Vertices[vertex_index];

                    /// @todo   Think about whether we want a triangle-only version of this.
                    Surface surface = { .Shape = &world_space_triangle };
                    SHADING::ShadingSettings vertex_shading_settings = rendering_settings.Shading;
                    vertex_shading_settings.TextureMappingEnabled = false;
                    const std::vector<float> NO_SHADOWING;
                    Color final_vertex_color = SHADING::WorldSpaceShading::ComputeMaterialShading(
                        current_world_vertex.Position,
                        surface,
                        camera.WorldPosition,
                        lights,
                        NO_SHADOWING,
                        vertex_shading_settings);

                    screen_space_triangle->Vertices[vertex_index].Color = final_vertex_color;
                }

                // RENDER THE FINAL SCREEN SPACE TRIANGLE.
                Render(*screen_space_triangle, rendering_settings, output_bitmap, depth_buffer);
            }
        }
    }

    /// Transforms a triangle from local coordinates to world coordinates.
    /// @param[in]  local_triangle - The local triangle to transform.
    /// @param[in]  world_transform - The world transformation for the triangle.
    /// @return The world space triangle.
    GEOMETRY::Triangle CpuRasterizationAlgorithm::TransformLocalToWorld(const GEOMETRY::Triangle& local_triangle, const MATH::Matrix4x4f& world_transform)
    {
        // TRANSFORM EACH VERTEX OF THE TRIANGLE.
        GEOMETRY::Triangle world_space_triangle = local_triangle;

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
    /// @param[in]  rendering_settings - The settings to use for rendering.
    /// @param[in,out]  render_target - The target to render to.
    /// @param[in,out]  depth_buffer - The depth buffer to use for any depth buffering.
    void CpuRasterizationAlgorithm::Render(
        const GEOMETRY::Triangle& triangle,
        const RenderingSettings& rendering_settings,
        IMAGES::Bitmap& render_target,
        DepthBuffer* depth_buffer)
    {
        /// @todo DEBUGGING::HighResolutionTimer timer("CpuRasterizationAlgorithm::Render(triangle)");

        // GET THE VERTICES.
        // They're needed for all kinds of shading.
        const VertexWithAttributes& first_vertex = triangle.Vertices[0];
        const VertexWithAttributes& second_vertex = triangle.Vertices[1];
        const VertexWithAttributes& third_vertex = triangle.Vertices[2];

        // RENDER THE TRIANGLE AS A WIREFRAME IF THERE IS NO MATERIAL.
        // This allows viewing at least something for a triangle.
        if (!triangle.Material)
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

            return;
        }

        // RENDER THE TRIANGLE BASED ON SHADING TYPE.
        /// @todo   Enable this to be an override of material only if set?
        SHADING::ShadingType shading_type = rendering_settings.Shading.ShadingType;
        switch (shading_type)
        {
            case SHADING::ShadingType::WIREFRAME:
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
            // Flat and material-based shading are nearly the same, with only differences in color computation.
            case SHADING::ShadingType::FLAT:
            case SHADING::ShadingType::MATERIAL:
            {
                /// @todo DEBUGGING::HighResolutionTimer timer("CpuRasterizationAlgorithm::Render(triangle) - Material shading");

                // GET THE BOUNDING RECTANGLE OF THE TRIANGLE.
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

#if SIMD
                // LOAD THE TRIANGLE INTO SIMD FORMAT.
                GEOMETRY::TriangleSimd8x simd_triangle = GEOMETRY::TriangleSimd8x::Load(triangle);

                // COLOR PIXELS WITHIN THE TRIANGLE.
                constexpr float ONE_PIXEL = 1.0f;
                for (float y = clamped_min_y; y <= clamped_max_y; y += ONE_PIXEL)
                {
                    constexpr float SIMD_AVX_REGISTER_ELEMENT_COUNT = 8.0f;
                    for (float x = clamped_min_x; x <= clamped_max_x; x += SIMD_AVX_REGISTER_ELEMENT_COUNT)
                    {
                        // LOAD THE CURRENT POINT INTO SIMD FORMAT.
                        // r version maps to more intuitive memory order.
                        __m256 current_point_x_coordinates = _mm256_setr_ps(
                            x,
                            x + 1.0f,
                            x + 2.0f,
                            x + 3.0f,
                            x + 4.0f,
                            x + 5.0f,
                            x + 6.0f,
                            x + 7.0f);
                        __m256 current_point_y_coordinates = _mm256_set1_ps(y);
                        MATH::Vector2<__m256> current_points(current_point_x_coordinates, current_point_y_coordinates);

                        // COMPUTE THE BARYCENTRIC COORDINATES OF THE CURRENT POINTS.
                        MATH::Vector3Simd8x current_point_barycentric_coordinates = simd_triangle.BarycentricCoordinates2DOf(current_points);

                        // CHECK IF THE POINTS ARE WITHIN THE TRIANGLE.
                        const __m256 MIN_SIGNED_DISTANCE_TO_BE_ON_EDGE = _mm256_set1_ps(0.0f);
                        const __m256 MAX_SIGNED_DISTANCE_TO_BE_ON_VERTEX = _mm256_set1_ps(1.0f);
                        __m256 pixels_x_on_inner_side_of_edge = _mm256_cmp_ps(MIN_SIGNED_DISTANCE_TO_BE_ON_EDGE, current_point_barycentric_coordinates.X, _CMP_LE_OS);
                        __m256 pixels_x_on_inner_side_of_vertex = _mm256_cmp_ps(current_point_barycentric_coordinates.X, MAX_SIGNED_DISTANCE_TO_BE_ON_VERTEX, _CMP_LE_OS);
                        __m256 pixels_x_in_triangle = _mm256_and_ps(pixels_x_on_inner_side_of_edge, pixels_x_on_inner_side_of_vertex);

                        __m256 pixels_y_on_inner_side_of_edge = _mm256_cmp_ps(MIN_SIGNED_DISTANCE_TO_BE_ON_EDGE, current_point_barycentric_coordinates.Y, _CMP_LE_OS);
                        __m256 pixels_y_on_inner_side_of_vertex = _mm256_cmp_ps(current_point_barycentric_coordinates.Y, MAX_SIGNED_DISTANCE_TO_BE_ON_VERTEX, _CMP_LE_OS);
                        __m256 pixels_y_in_triangle = _mm256_and_ps(pixels_y_on_inner_side_of_edge, pixels_y_on_inner_side_of_vertex);

                        __m256 pixels_z_on_inner_side_of_edge = _mm256_cmp_ps(MIN_SIGNED_DISTANCE_TO_BE_ON_EDGE, current_point_barycentric_coordinates.Z, _CMP_LE_OS);
                        __m256 pixels_z_on_inner_side_of_vertex = _mm256_cmp_ps(current_point_barycentric_coordinates.Z, MAX_SIGNED_DISTANCE_TO_BE_ON_VERTEX, _CMP_LE_OS);
                        __m256 pixels_z_in_triangle = _mm256_and_ps(pixels_z_on_inner_side_of_edge, pixels_z_on_inner_side_of_vertex);

                        __m256 pixels_x_y_in_triangle = _mm256_and_ps(pixels_x_in_triangle, pixels_y_in_triangle);
                        __m256 pixels_in_triangle = _mm256_and_ps(pixels_x_y_in_triangle, pixels_z_in_triangle);;

#if SIMD_DEBUG
                        float pixel_indices_in_triangle[static_cast<int>(SIMD_AVX_REGISTER_ELEMENT_COUNT)] = {};
                        _mm256_store_ps(pixel_indices_in_triangle, pixels_in_triangle);

                        for (std::size_t pixel_x_offset = 0; pixel_x_offset < static_cast<int>(SIMD_AVX_REGISTER_ELEMENT_COUNT); ++pixel_x_offset)
                        {
                            unsigned int current_pixel_x = static_cast<unsigned int>(std::round(x + static_cast<float>(pixel_x_offset)));
                            unsigned int current_pixel_y = static_cast<unsigned int>(std::round(y));

                            float current_pixel_in_triangle = pixel_indices_in_triangle[pixel_x_offset];
                            if (std::isnan(current_pixel_in_triangle))
                            {
                                render_target.WritePixel(current_pixel_x, current_pixel_y, Color::RED);
                            }
                            else if (0.0f == current_pixel_in_triangle)
                            {
                                //render_target.WritePixel(current_pixel_x, current_pixel_y, Color::GREEN);
                            }
                            else if (1.0f == current_pixel_in_triangle)
                            {
                                //render_target.WritePixel(current_pixel_x, current_pixel_y, Color::BLUE);
                            }
                            else
                            {
                                //render_target.WritePixel(current_pixel_x, current_pixel_y, Color::WHITE);
                            }
                        }
#endif
                        // Interpolating z for depth-buffering.
                        __m256 interpolated_z_barycentric_x_times_second_z = _mm256_mul_ps(current_point_barycentric_coordinates.X, simd_triangle.CenterVertexPosition.Z);
                        __m256 interpolated_z_barycentric_y_times_third_z = _mm256_mul_ps(current_point_barycentric_coordinates.Y, simd_triangle.RightVertexPosition.Z);
                        __m256 interpolated_z_barycentric_z_times_first_z = _mm256_mul_ps(current_point_barycentric_coordinates.Z, simd_triangle.LeftVertexPosition.Z);
                        __m256 interpolated_z_coordinates = _mm256_add_ps(interpolated_z_barycentric_x_times_second_z, interpolated_z_barycentric_y_times_third_z);
                        interpolated_z_coordinates = _mm256_add_ps(interpolated_z_coordinates, interpolated_z_barycentric_z_times_first_z);

                        __m256 pixel_reds_barycentric_x_times_second_red = _mm256_mul_ps(current_point_barycentric_coordinates.X, simd_triangle.SecondVertexColorRed);
                        __m256 pixel_reds_barycentric_y_times_third_red = _mm256_mul_ps(current_point_barycentric_coordinates.Y, simd_triangle.ThirdVertexColorRed);
                        __m256 pixel_reds_barycentric_z_times_first_red = _mm256_mul_ps(current_point_barycentric_coordinates.Z, simd_triangle.FirstVertexColorRed);
                        __m256 pixel_reds = _mm256_add_ps(pixel_reds_barycentric_x_times_second_red, pixel_reds_barycentric_y_times_third_red);
                        pixel_reds = _mm256_add_ps(pixel_reds, pixel_reds_barycentric_z_times_first_red);

                        __m256 pixel_greens_barycentric_x_times_second_green = _mm256_mul_ps(current_point_barycentric_coordinates.X, simd_triangle.SecondVertexColorGreen);
                        __m256 pixel_greens_barycentric_y_times_third_green = _mm256_mul_ps(current_point_barycentric_coordinates.Y, simd_triangle.ThirdVertexColorGreen);
                        __m256 pixel_greens_barycentric_z_times_first_green = _mm256_mul_ps(current_point_barycentric_coordinates.Z, simd_triangle.FirstVertexColorGreen);
                        __m256 pixel_greens = _mm256_add_ps(pixel_greens_barycentric_x_times_second_green, pixel_greens_barycentric_y_times_third_green);
                        pixel_greens = _mm256_add_ps(pixel_greens, pixel_greens_barycentric_z_times_first_green);

                        __m256 pixel_blues_barycentric_x_times_second_blue = _mm256_mul_ps(current_point_barycentric_coordinates.X, simd_triangle.SecondVertexColorBlue);
                        __m256 pixel_blues_barycentric_y_times_third_blue = _mm256_mul_ps(current_point_barycentric_coordinates.Y, simd_triangle.ThirdVertexColorBlue);
                        __m256 pixel_blues_barycentric_z_times_first_blue = _mm256_mul_ps(current_point_barycentric_coordinates.Z, simd_triangle.FirstVertexColorBlue);
                        __m256 pixel_blues = _mm256_add_ps(pixel_blues_barycentric_x_times_second_blue, pixel_blues_barycentric_y_times_third_blue);
                        pixel_blues = _mm256_add_ps(pixel_blues, pixel_blues_barycentric_z_times_first_blue);

                        float pixel_red_indices[static_cast<int>(SIMD_AVX_REGISTER_ELEMENT_COUNT)] = {};
                        _mm256_store_ps(pixel_red_indices, pixel_reds);

                        float pixel_blue_indices[static_cast<int>(SIMD_AVX_REGISTER_ELEMENT_COUNT)] = {};
                        _mm256_store_ps(pixel_blue_indices, pixel_blues);

                        float pixel_green_indices[static_cast<int>(SIMD_AVX_REGISTER_ELEMENT_COUNT)] = {};
                        _mm256_store_ps(pixel_green_indices, pixel_greens);

                        float interpolated_z_by_index[static_cast<int>(SIMD_AVX_REGISTER_ELEMENT_COUNT)] = {};
                        _mm256_store_ps(interpolated_z_by_index, interpolated_z_coordinates);

                        float pixel_indices_in_triangle[static_cast<int>(SIMD_AVX_REGISTER_ELEMENT_COUNT)] = {};
                        _mm256_store_ps(pixel_indices_in_triangle, pixels_in_triangle);

                        // There may be a slight speedup from SIMD here, but we probably need to SIMD-ize more of the below to realize significant benefits.
                        for (std::size_t pixel_x_offset = 0; pixel_x_offset < static_cast<int>(SIMD_AVX_REGISTER_ELEMENT_COUNT); ++pixel_x_offset)
                        {
                            float current_pixel_in_triangle = pixel_indices_in_triangle[pixel_x_offset];
                            if (std::isnan(current_pixel_in_triangle))
                            {
                                // GET THE CURRENT PIXEL COORDINATES.
                                unsigned int current_pixel_x = static_cast<unsigned int>(std::round(x + static_cast<float>(pixel_x_offset)));
                                unsigned int current_pixel_y = static_cast<unsigned int>(std::round(y));

                                // SKIP WRITING PIXELS IF NEW PIXEL IS BEHIND ALREADY WRITTEN ONE.
                                float interpolated_z = interpolated_z_by_index[pixel_x_offset];
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

                                // GET THE FINAL PIXEL COLOR.
                                float current_pixel_red = pixel_red_indices[pixel_x_offset];
                                float current_pixel_green = pixel_red_indices[pixel_x_offset];
                                float current_pixel_blue = pixel_red_indices[pixel_x_offset];
                                Color pixel_color(current_pixel_red, current_pixel_green, current_pixel_blue, Color::MAX_FLOAT_COLOR_COMPONENT);

                                // ADD TEXTURING IF APPLICABLE.
                                if (rendering_settings.Shading.TextureMappingEnabled)
                                {
                                    MATH::Vector2f current_point(x + static_cast<float>(pixel_x_offset), y);

                                    Color texture_color = Color::BLACK;

                                    // ADD AMBIENT TEXTURING IF APPLICABLE.
                                    if (rendering_settings.Shading.Lighting.AmbientLightingEnabled)
                                    {
                                        bool ambient_texture_exists = (nullptr != triangle.Material->AmbientProperties.Texture);
                                        if (ambient_texture_exists)
                                        {
                                            Color ambient_texture_color = TextureMappingAlgorithm::LookupTexel(
                                                triangle,
                                                current_point,
                                                *triangle.Material->AmbientProperties.Texture);
                                            texture_color += ambient_texture_color;
                                        }
                                    }

                                    // ADD DIFFUSE TEXTURING IF APPLICABLE.
                                    if (rendering_settings.Shading.Lighting.DiffuseLightingEnabled)
                                    {
                                        bool diffuse_texture_exists = (nullptr != triangle.Material->DiffuseProperties.Texture);
                                        if (diffuse_texture_exists)
                                        {
                                            Color diffuse_texture_color = TextureMappingAlgorithm::LookupTexel(
                                                triangle,
                                                current_point,
                                                *triangle.Material->DiffuseProperties.Texture);
                                            texture_color += diffuse_texture_color;
                                        }
                                    }

                                    // ADD SPECULAR TEXTURING IF APPLICABLE.
                                    if (rendering_settings.Shading.Lighting.SpecularLightingEnabled)
                                    {
                                        bool specular_texture_exists = (nullptr != triangle.Material->SpecularProperties.Texture);
                                        if (specular_texture_exists)
                                        {
                                            Color specular_texture_color = TextureMappingAlgorithm::LookupTexel(
                                                triangle,
                                                current_point,
                                                *triangle.Material->SpecularProperties.Texture);
                                            texture_color += specular_texture_color;
                                        }
                                    }

                                    // ADD THE FINAL COMPUTED TEXTURE COLOR IF IT EXISTS.
                                    // If no textures exist, the texture color would be left black, which would cancel out normal coloring
                                    // (which is not desirable).
                                    bool texture_coloring_exists = (Color::BLACK != texture_color);
                                    if (texture_coloring_exists)
                                    {
                                        pixel_color = Color::ComponentMultiplyRedGreenBlue(pixel_color, texture_color);
                                    }
                                }

                                // ENSURE THE COLOR IS WITHIN THE PROPER RANGE
                                pixel_color.Clamp();

                                // WRITE THE FINAL COLOR AND DEPTH VALUES.
                                render_target.WritePixel(current_pixel_x, current_pixel_y, pixel_color);
                                if (depth_buffer)
                                {
                                    depth_buffer->WriteDepth(current_pixel_x, current_pixel_y, interpolated_z);
                                }
                            }
                        }
                    }
                }
#else
                // COLOR PIXELS WITHIN THE TRIANGLE.
                constexpr float ONE_PIXEL = 1.0f;
                for (float y = clamped_min_y; y <= clamped_max_y; y += ONE_PIXEL)
                {
                    for (float x = clamped_min_x; x <= clamped_max_x; x += ONE_PIXEL)
                    {
                        /// @todo   If we do this in SIMD, this is probably the biggest potential for speedups.
                        /// @todo DEBUGGING::HighResolutionTimer timer("CpuRasterizationAlgorithm::Render(triangle) - Material pixel single loop");

                        // CHECK IF THE CURRENT PIXEL IS WITHIN THE TRIANGLE.
                        MATH::Vector2f current_point(x, y);
                        MATH::Vector3f current_point_barycentric_coordinates = triangle.BarycentricCoordinates2DOf(current_point);

                        constexpr float MIN_SIGNED_DISTANCE_TO_BE_ON_EDGE = 0.0f;
                        constexpr float MAX_SIGNED_DISTANCE_TO_BE_ON_VERTEX = 1.0f;
                        bool pixel_between_opposite_edge_and_center_vertex = (
                            (MIN_SIGNED_DISTANCE_TO_BE_ON_EDGE <= current_point_barycentric_coordinates.X) &&
                            (current_point_barycentric_coordinates.X <= MAX_SIGNED_DISTANCE_TO_BE_ON_VERTEX));
                        bool pixel_between_left_edge_and_right_vertex = (
                            (MIN_SIGNED_DISTANCE_TO_BE_ON_EDGE <= current_point_barycentric_coordinates.Y) &&
                            (current_point_barycentric_coordinates.Y <= MAX_SIGNED_DISTANCE_TO_BE_ON_VERTEX));
                        bool pixel_between_right_edge_and_left_vertex = (
                            (MIN_SIGNED_DISTANCE_TO_BE_ON_EDGE <= current_point_barycentric_coordinates.Z) &&
                            (current_point_barycentric_coordinates.Z <= MAX_SIGNED_DISTANCE_TO_BE_ON_VERTEX));
                        bool pixel_in_triangle = (
                            pixel_between_opposite_edge_and_center_vertex &&
                            pixel_between_left_edge_and_right_vertex &&
                            pixel_between_right_edge_and_left_vertex);
                        if (pixel_in_triangle)
                        {
                            // COMPUTE THE PIXEL COLOR BASED ON THE TYPE OF SHADING.
                            // If flat shading is not specified, then regular interpolation is assumed.
                            Color pixel_color = Color::BLACK;
                            bool is_flat_shading = (SHADING::ShadingType::FLAT == shading_type);
                            if (is_flat_shading)
                            {
                                // AVERAGE THE VERTEX COLORS.
                                const Color& first_vertex_color = triangle.Vertices[0].Color;
                                const Color& second_vertex_color = triangle.Vertices[1].Color;
                                const Color& third_vertex_color = triangle.Vertices[2].Color;

                                constexpr float VERTEX_COUNT = static_cast<float>(GEOMETRY::Triangle::VERTEX_COUNT);
                                float average_red = (first_vertex_color.Red + second_vertex_color.Red + third_vertex_color.Red) / VERTEX_COUNT;
                                float average_green = (first_vertex_color.Green + second_vertex_color.Green + third_vertex_color.Green) / VERTEX_COUNT;
                                float average_blue = (first_vertex_color.Blue + second_vertex_color.Blue + third_vertex_color.Blue) / VERTEX_COUNT;
                                float average_alpha = (first_vertex_color.Alpha + second_vertex_color.Alpha + third_vertex_color.Alpha) / VERTEX_COUNT;

                                /// @todo   Should we try some kind of texture mapping here?

                                pixel_color = Color(average_red, average_green, average_blue, average_alpha);
                            }
                            else
                            {
                                /// @todo   Note - this is mainly the bottleneck in this method.  About 5000 ticks per pixel.
                                /// @todo DEBUGGING::HighResolutionTimer timer("CpuRasterizationAlgorithm::Render(triangle) - Material pixel shading");

                                // INTERPOLATE THE VERTEX COLORS.
                                // The color needs to be interpolated for other kinds of shading.
                                const Color& first_vertex_color = triangle.Vertices[0].Color;
                                const Color& second_vertex_color = triangle.Vertices[1].Color;
                                const Color& third_vertex_color = triangle.Vertices[2].Color;

                                pixel_color.Red = (
                                    (current_point_barycentric_coordinates.X * second_vertex_color.Red) +
                                    (current_point_barycentric_coordinates.Y * third_vertex_color.Red) +
                                    (current_point_barycentric_coordinates.Z * first_vertex_color.Red));
                                pixel_color.Green = (
                                    (current_point_barycentric_coordinates.X * second_vertex_color.Green) +
                                    (current_point_barycentric_coordinates.Y * third_vertex_color.Green) +
                                    (current_point_barycentric_coordinates.Z * first_vertex_color.Green));
                                pixel_color.Blue = (
                                    (current_point_barycentric_coordinates.X * second_vertex_color.Blue) +
                                    (current_point_barycentric_coordinates.Y * third_vertex_color.Blue) +
                                    (current_point_barycentric_coordinates.Z * first_vertex_color.Blue));
                                
                                // ADD TEXTURING IF APPLICABLE.
                                if (rendering_settings.Shading.TextureMappingEnabled)
                                {
                                    Color texture_color = Color::BLACK;

                                    // ADD AMBIENT TEXTURING IF APPLICABLE.
                                    if (rendering_settings.Shading.Lighting.AmbientLightingEnabled)
                                    {
                                        bool ambient_texture_exists = (nullptr != triangle.Material->AmbientProperties.Texture);
                                        if (ambient_texture_exists)
                                        {
                                            Color ambient_texture_color = TextureMappingAlgorithm::LookupTexel(
                                                triangle,
                                                current_point,
                                                *triangle.Material->AmbientProperties.Texture);
                                            texture_color += ambient_texture_color;
                                        }
                                    }

                                    // ADD DIFFUSE TEXTURING IF APPLICABLE.
                                    if (rendering_settings.Shading.Lighting.DiffuseLightingEnabled)
                                    {
                                        bool diffuse_texture_exists = (nullptr != triangle.Material->DiffuseProperties.Texture);
                                        if (diffuse_texture_exists)
                                        {
                                            Color diffuse_texture_color = TextureMappingAlgorithm::LookupTexel(
                                                triangle,
                                                current_point,
                                                *triangle.Material->DiffuseProperties.Texture);
                                            texture_color += diffuse_texture_color;
                                        }
                                    }

                                    // ADD SPECULAR TEXTURING IF APPLICABLE.
                                    if (rendering_settings.Shading.Lighting.SpecularLightingEnabled)
                                    {
                                        bool specular_texture_exists = (nullptr != triangle.Material->SpecularProperties.Texture);
                                        if (specular_texture_exists)
                                        {
                                            Color specular_texture_color = TextureMappingAlgorithm::LookupTexel(
                                                triangle,
                                                current_point,
                                                *triangle.Material->SpecularProperties.Texture);
                                            texture_color += specular_texture_color;
                                        }
                                    }

                                    // ADD THE FINAL COMPUTED TEXTURE COLOR IF IT EXISTS.
                                    // If no textures exist, the texture color would be left black, which would cancel out normal coloring
                                    // (which is not desirable).
                                    bool texture_coloring_exists = (Color::BLACK != texture_color);
                                    if (texture_coloring_exists)
                                    {
                                        pixel_color = Color::ComponentMultiplyRedGreenBlue(pixel_color, texture_color);
                                    }
                                }

                                // ENSURE THE COLOR IS WITHIN THE PROPER RANGE
                                pixel_color.Clamp();
                            }

                            // AVOID WRITING THE PIXEL IF ANOTHER PIXEL IS ALREADY IN FRONT OF IT.
                            // The z-coordinate needs to be properly interpolated first.
                            float interpolated_z = (
                                (current_point_barycentric_coordinates.X * second_vertex.Position.Z) +
                                (current_point_barycentric_coordinates.Y * third_vertex.Position.Z) +
                                (current_point_barycentric_coordinates.Z * first_vertex.Position.Z));
                            // The coordinates need to be rounded to integer in order to plot a pixel on a fixed grid.
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

                            // WRITE THE FINAL COLOR AND DEPTH VALUES.
                            render_target.WritePixel(current_pixel_x, current_pixel_y, pixel_color);
                            if (depth_buffer)
                            {
                                depth_buffer->WriteDepth(current_pixel_x, current_pixel_y, interpolated_z);
                            }
                        }
                    }
                }
#endif
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
